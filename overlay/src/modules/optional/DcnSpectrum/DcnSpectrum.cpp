/**
 * DCN EU868 spectrum sweep (863–870 MHz) + texto en pantalla MUI.
 * Activar: mensaje de texto "DCN SPECTRUM" (canal primario o local).
 */
#if defined(DCN_SPECTRUM_UI) && DCN_SPECTRUM_UI

#include "DcnSpectrum.h"
#include "DcnSpectrumUi.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "Router.h"
#include "Throttle.h"
#include "configuration.h"
#include "concurrency/OSThread.h"
#include "UptimeClock.h"
#include "main.h"
#include "mesh/RadioLibInterface.h"
#include "mesh/SinglePortModule.h"
#include "mesh/generated/meshtastic/mesh.pb.h"
#include <Arduino.h>
#include <cctype>
#include <cmath>
#include <cstring>

constexpr float kFreqStartMhz = 863.0f;
constexpr float kFreqEndMhz = 870.0f;
constexpr float kStepMhz = 0.25f;
constexpr int kMaxBins = 32;
constexpr uint32_t kSettleMs = 18;

static float bins[kMaxBins];
static int binCount = 0;
static meshtastic_Config_LoRaConfig loraBackup{};
static bool radioBusy = false;
static bool scanRequested = false;
static bool uiVisible = false;
static uint32_t uiUntil = 0;

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
        setIntervalFromNow(200);
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
        for (char *p = buf; *p; p++)
            if (isupper((unsigned char)*p))
                *p = (char)tolower((unsigned char)*p);
        if (strcmp(buf, "dcn spectrum") != 0 && strstr(buf, "dcn spectrum") == nullptr)
            return ProcessMessage::CONTINUE;
        if (radioBusy) {
            replyText(mp, "DCN: radio ocupada (escaneo en curso).");
            return ProcessMessage::STOP;
        }
        scanRequested = true;
        dcnSpectrumNotifyScanStart();
        replyText(mp, "DCN: espectro EU 863-870 MHz…");
        return ProcessMessage::STOP;
    }

  protected:
    int32_t runOnce() override
    {
        if (scanRequested && !radioBusy) {
            scanRequested = false;
            runScan();
        }
        if (uiVisible && Throttle::deadlinePassedAt(Time::getMillis(), uiUntil)) {
            uiVisible = false;
        }
        return 250;
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

    void runScan()
    {
        RadioLibInterface *ri = RadioLibInterface::instance;
        if (!ri) {
            LOG_WARN("DCN spectrum: sin radio");
            return;
        }
        radioBusy = true;
        loraBackup = config.lora;
        binCount = 0;

        const int totalBins =
            (int)((kFreqEndMhz - kFreqStartMhz) / kStepMhz + 1.0f);
        for (float f = kFreqStartMhz; f <= kFreqEndMhz + 0.001f && binCount < kMaxBins; f += kStepMhz) {
            config.lora.override_frequency = f;
            config.lora.bandwidth = 125;
            config.lora.spread_factor = 7;
            config.lora.tx_enabled = false;
            if (service)
                service->configChanged.notifyObservers(nullptr);
            delay(kSettleMs);
            ri->updateNoiseFloor();
            bins[binCount++] = (float)ri->getAverageNoiseFloor();
            dcnSpectrumNotifyScanProgress(binCount, totalBins);
        }

        config.lora = loraBackup;
        if (service)
            service->configChanged.notifyObservers(nullptr);

        publishResults();
        radioBusy = false;
        uiVisible = true;
        uiUntil = Time::timerEndsAtMillis(120000);
    }

    void publishResults()
    {
        float minR = 0, maxR = -200;
        for (int i = 0; i < binCount; i++) {
            if (bins[i] > maxR)
                maxR = bins[i];
            if (bins[i] < minR)
                minR = bins[i];
        }
        dcnSpectrumShowGraph(bins, binCount, minR, maxR);

        char body[512];
        char *w = body;
        const size_t cap = sizeof(body);
        int n = snprintf(w, cap, "DCN Spectrum 863-870 MHz (%d puntos)\n", binCount);
        w += (n > 0 ? n : 0);

        float span = maxR - minR;
        if (span < 6)
            span = 6;

        for (int i = 0; i < binCount && (size_t)(w - body) < cap - 40; i++) {
            int bars = (int)((bins[i] - minR) * 8.0f / span);
            if (bars < 0)
                bars = 0;
            if (bars > 8)
                bars = 8;
            n = snprintf(w, cap - (size_t)(w - body), "%.2f:", kFreqStartMhz + i * kStepMhz);
            if (n > 0)
                w += n;
            for (int b = 0; b < bars; b++)
                *w++ = '#';
            *w++ = '\n';
            *w = 0;
        }
        n = snprintf(w, cap - (size_t)(w - body), "RSSI %.0f..%.0f dBm\n(Ver hilo local en pantalla).", minR, maxR);
        (void)n;

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
    LOG_INFO("DCN Spectrum EU868 listo (DCN SPECTRUM)");
}

#else
#include "DcnSpectrum.h"
void setupDcnSpectrum() {}
#endif