/**
 * Pantalla T-Deck (MUI/LVGL): no hay API pública estable en device-ui, así que v2 muestra
 * el espectro como mensaje local con barras Unicode (█) que el hilo de mensajes renderiza bien.
 * Fase siguiente: overlay LVGL dedicado en device-ui DCN.
 */
#if defined(DCN_SPECTRUM_UI) && DCN_SPECTRUM_UI

#include "DcnSpectrumUi.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "Router.h"
#include "configuration.h"
#include "mesh/generated/meshtastic/mesh.pb.h"
#include <cstdio>
#include <cstring>

static void sendLocalText(const char *text, bool wake)
{
    meshtastic_MeshPacket *p = router->allocForSending();
    if (!p)
        return;
    p->to = nodeDB->getNodeNum();
    p->from = nodeDB->getNodeNum();
    p->decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
    p->want_ack = false;
    size_t len = strlen(text);
    if (len > sizeof(p->decoded.payload.bytes))
        len = sizeof(p->decoded.payload.bytes);
    memcpy(p->decoded.payload.bytes, text, len);
    p->decoded.payload.size = len;
    service->sendToMesh(p, RX_SRC_LOCAL, true);
    if (wake)
        powerFSM.trigger(EVENT_RECEIVED_MSG);
}

void dcnSpectrumNotifyScanStart()
{
    sendLocalText("\x07 DCN Spectrum\nEscaneando 863-870 MHz…", true);
}

void dcnSpectrumNotifyScanProgress(int done, int total)
{
    if (total <= 0 || done % 7 != 0)
        return;
    char line[64];
    snprintf(line, sizeof(line), "\x07 DCN Spectrum %d/%d", done, total);
    sendLocalText(line, false);
}

void dcnSpectrumShowGraph(const float *bins, int count, float minDbm, float maxDbm)
{
    if (count <= 0 || !bins)
        return;
    float span = maxDbm - minDbm;
    if (span < 4.0f)
        span = 4.0f;

    char body[420];
    char *w = body;
    const size_t cap = sizeof(body);
    int n = snprintf(w, cap, "\x07 DCN Spectrum (EU 863-870)\n%.0f..%.0f dBm\n", minDbm, maxDbm);
    w += (n > 0 ? n : 0);

    const int maxBars = count > 14 ? 14 : count;
    const int step = count > maxBars ? (count + maxBars - 1) / maxBars : 1;
    for (int i = 0, shown = 0; i < count && shown < maxBars && (size_t)(w - body) < cap - 24; i += step, shown++) {
        int bars = (int)((bins[i] - minDbm) * 12.0f / span);
        if (bars < 0)
            bars = 0;
        if (bars > 12)
            bars = 12;
        float mhz = 863.0f + i * 0.25f;
        n = snprintf(w, cap - (size_t)(w - body), "%.1f ", mhz);
        if (n > 0)
            w += n;
        for (int b = 0; b < bars; b++)
            *w++ = 0xE2, *w++ = 0x96, *w++ = 0x88; // UTF-8 █
        *w++ = '\n';
        *w = 0;
    }
    snprintf(w, cap - (size_t)(w - body), "Fin (2 min). Comando: DCN SPECTRUM");

    sendLocalText(body, true);
}

#else
#include "DcnSpectrumUi.h"
void dcnSpectrumNotifyScanStart() {}
void dcnSpectrumNotifyScanProgress(int, int) {}
void dcnSpectrumShowGraph(const float *, int, float, float) {}
#endif