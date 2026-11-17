#if defined(DCN_SPECTRUM_UI) && DCN_SPECTRUM_UI

#include "DcnSpectrumService.h"
#include "MeshService.h"
#include "configuration.h"
#include "main.h"
#include "mesh/RadioLibInterface.h"
#include <Arduino.h>

static DcnSpectrumSnapshot snap;
static bool sweepRequested = false;
static float reqStart = 863, reqEnd = 870, reqStep = 0.25f;

void dcnSpectrumClampRange(float &startMhz, float &endMhz)
{
    if (startMhz < kSx1262MinMhz)
        startMhz = kSx1262MinMhz;
    if (endMhz > kSx1262MaxMhz)
        endMhz = kSx1262MaxMhz;
    if (endMhz < startMhz)
        endMhz = startMhz;
}

bool dcnSpectrumRequestSweep(float startMhz, float endMhz, float stepMhz)
{
    if (snap.busy)
        return false;
    dcnSpectrumClampRange(startMhz, endMhz);
    if (stepMhz < 0.05f)
        stepMhz = 0.05f;
    reqStart = startMhz;
    reqEnd = endMhz;
    reqStep = stepMhz;
    sweepRequested = true;
    snap.live = false;
    return true;
}

bool dcnSpectrumSetLive(bool enable, float centerMhz, float spanMhz, float stepMhz)
{
    (void)centerMhz;
    (void)spanMhz;
    (void)stepMhz;
    snap.live = enable; // reservado para v3 (LVGL)
    return !snap.busy || !enable;
}

void dcnSpectrumSetRxBoost(uint8_t level)
{
    snap.rxBoost = level ? 1 : 0;
    config.lora.sx126x_rx_boosted_gain = level != 0;
    if (service)
        service->configChanged.notifyObservers(nullptr);
}

const DcnSpectrumSnapshot &dcnSpectrumGetSnapshot()
{
    return snap;
}

static void runSweep(float startMhz, float endMhz, float stepMhz)
{
    RadioLibInterface *ri = RadioLibInterface::instance;
    if (!ri) {
        snap.busy = false;
        return;
    }
    snap.busy = true;
    meshtastic_Config_LoRaConfig backup = config.lora;
    snap.count = 0;
    snap.startMhz = startMhz;
    snap.stepMhz = stepMhz;

    for (float f = startMhz; f <= endMhz + 0.001f && snap.count < kSpectrumMaxBins; f += stepMhz) {
        config.lora.override_frequency = f;
        config.lora.bandwidth = 125;
        config.lora.spread_factor = 7;
        config.lora.tx_enabled = false;
        if (service)
            service->configChanged.notifyObservers(nullptr);
        delay(18);
        ri->updateNoiseFloor();
        snap.bins[snap.count++] = (float)ri->getAverageNoiseFloor();
    }

    snap.minDbm = 0;
    snap.maxDbm = -200;
    for (int i = 0; i < snap.count; i++) {
        if (snap.bins[i] > snap.maxDbm)
            snap.maxDbm = snap.bins[i];
        if (snap.bins[i] < snap.minDbm)
            snap.minDbm = snap.bins[i];
    }

    config.lora = backup;
    if (service)
        service->configChanged.notifyObservers(nullptr);
    snap.busy = false;
}

void dcnSpectrumServiceTick()
{
    if (sweepRequested && !snap.busy) {
        sweepRequested = false;
        runSweep(reqStart, reqEnd, reqStep);
    }
}

#else
#include "DcnSpectrumService.h"
void dcnSpectrumClampRange(float &, float &) {}
bool dcnSpectrumRequestSweep(float, float, float) { return false; }
bool dcnSpectrumSetLive(bool, float, float, float) { return false; }
void dcnSpectrumSetRxBoost(uint8_t) {}
const DcnSpectrumSnapshot &dcnSpectrumGetSnapshot()
{
    static DcnSpectrumSnapshot s;
    return s;
}
void dcnSpectrumServiceTick() {}
#endif