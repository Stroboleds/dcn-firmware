/**
 * DCN espectro EU / SX1262 — comando texto + servicio para UI LVGL (v3).
 * Comandos: "DCN SPECTRUM" | "dcn spectrum 865 875" | "dcn gain on"
 */
#if defined(DCN_SPECTRUM_UI) && DCN_SPECTRUM_UI

#include "DcnSpectrum.h"
#include "DcnSpectrumService.h"
#include "DcnSpectrumUi.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "Router.h"
#include "configuration.h"
#include "concurrency/OSThread.h"
#include "main.h"
#include "mesh/SinglePortModule.h"
#include "mesh/generated/meshtastic/mesh.pb.h"
#include <cctype>
#include <cstdio>
#include <cstring>

static void setPayloadText(meshtastic_MeshPacket *p, const char *text)
{
    size_t len = strlen(text);
    if (len > sizeof(p->decoded.payload.bytes))
        len = sizeof(p->decoded.payload.bytes);
    memcpy(p->decoded.payload.bytes, text, len);
    p->decoded.payload.size = len;
}

class DcnSpectrumModule : public SinglePortModule, private concurrency::OSThread
{
  public:
    DcnSpectrumModule()
        : SinglePortModule("dcn-spectrum", meshtastic_PortNum_TEXT_MESSAGE_APP), OSThread("DcnSpectrum")
    {
        loopbackOk = true; // mensajes escritos en el propio T-Deck (RX_SRC_LOCAL)
        setIntervalFromNow(150);
    }

    ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override
    {
        if (!isBroadcastOrLocal(mp))
            return ProcessMessage::CONTINUE;
        const auto &decoded = mp.decoded;
        if (!decoded.payload.size)
            return ProcessMessage::CONTINUE;

        char buf[160];
        size_t n = decoded.payload.size;
        if (n >= sizeof(buf))
            n = sizeof(buf) - 1;
        memcpy(buf, decoded.payload.bytes, n);
        buf[n] = 0;

        if (!handleCommand(buf, mp))
            return ProcessMessage::CONTINUE;
        return ProcessMessage::STOP;
    }

  protected:
    int32_t runOnce() override
    {
        const bool wasBusy = dcnSpectrumGetSnapshot().busy;
        dcnSpectrumServiceTick();
        const auto &s = dcnSpectrumGetSnapshot();
        if (wasBusy && !s.busy && s.count > 0)
            publishResults(s);
        return 150;
    }

  private:
    static bool isBroadcastOrLocal(const meshtastic_MeshPacket &mp)
    {
        return mp.to == NODENUM_BROADCAST || mp.to == nodeDB->getNodeNum();
    }

    static void replyText(const meshtastic_MeshPacket &rx, const char *text)
    {
        meshtastic_MeshPacket *p = router->allocForSending();
        if (!p)
            return;
        p->to = rx.from;
        p->decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
        p->want_ack = false;
        p->channel = rx.channel;
        setPayloadText(p, text);
        service->sendToMesh(p, RX_SRC_LOCAL, true);
    }

    bool handleCommand(char *buf, const meshtastic_MeshPacket &mp)
    {
        for (char *p = buf; *p; p++)
            if (isupper((unsigned char)*p))
                *p = (char)tolower((unsigned char)*p);

        if (strstr(buf, "dcn gain on") || strstr(buf, "dcn gain 1")) {
            dcnSpectrumSetRxBoost(1);
            replyText(mp, "DCN: RX boost ON");
            return true;
        }
        if (strstr(buf, "dcn gain off") || strstr(buf, "dcn gain 0")) {
            dcnSpectrumSetRxBoost(0);
            replyText(mp, "DCN: RX boost OFF");
            return true;
        }

        const char *key = strstr(buf, "dcn spectrum");
        if (!key)
            return false;

        float start = 863.0f, end = 870.0f, step = 0.25f;
        float a = 0, b = 0;
        if (sscanf(key + strlen("dcn spectrum"), "%f %f", &a, &b) == 2) {
            start = a;
            end = b;
        }

        if (dcnSpectrumGetSnapshot().busy) {
            replyText(mp, "DCN: radio ocupada (escaneo en curso).");
            return true;
        }
        if (!dcnSpectrumRequestSweep(start, end, step)) {
            replyText(mp, "DCN: no se pudo iniciar barrido.");
            return true;
        }
        dcnSpectrumNotifyScanStart();
        char ack[80];
        snprintf(ack, sizeof(ack), "DCN: barrido %.1f-%.1f MHz…", start, end);
        replyText(mp, ack);
        return true;
    }

    void publishResults(const DcnSpectrumSnapshot &s)
    {
        dcnSpectrumShowGraph(s.bins, s.count, s.minDbm, s.maxDbm);

        char body[512];
        char *w = body;
        const size_t cap = sizeof(body);
        int n = snprintf(w, cap, "DCN Spectrum %.1f-%.1f MHz (%d pts)\n", s.startMhz,
                         s.startMhz + (s.count > 0 ? (s.count - 1) * s.stepMhz : 0), s.count);
        w += (n > 0 ? n : 0);

        float span = s.maxDbm - s.minDbm;
        if (span < 6)
            span = 6;
        for (int i = 0; i < s.count && (size_t)(w - body) < cap - 32; i++) {
            int bars = (int)((s.bins[i] - s.minDbm) * 8.0f / span);
            if (bars < 0)
                bars = 0;
            if (bars > 8)
                bars = 8;
            n = snprintf(w, cap - (size_t)(w - body), "%.2f:", s.startMhz + i * s.stepMhz);
            if (n > 0)
                w += n;
            for (int b = 0; b < bars; b++)
                *w++ = '#';
            *w++ = '\n';
            *w = 0;
        }
        snprintf(w, cap - (size_t)(w - body), "RSSI %.0f..%.0f dBm (v3 LVGL en Tools)", s.minDbm, s.maxDbm);
        LOG_INFO("%s", body);

        meshtastic_MeshPacket *p = router->allocForSending();
        if (p) {
            p->to = NODENUM_BROADCAST;
            p->decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
            p->want_ack = false;
            setPayloadText(p, body);
            service->sendToMesh(p, RX_SRC_LOCAL, true);
        }
    }
};

static DcnSpectrumModule *dcnSpectrumModule;

void setupDcnSpectrum()
{
    dcnSpectrumModule = new DcnSpectrumModule();
    LOG_INFO("DCN Spectrum (SX1262, loopback OK)");
}

#else
#include "DcnSpectrum.h"
void setupDcnSpectrum() {}
#endif