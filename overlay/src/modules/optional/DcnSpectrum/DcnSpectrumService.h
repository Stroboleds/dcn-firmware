#pragma once

#if defined(DCN_SPECTRUM_UI) && DCN_SPECTRUM_UI

#include <stdint.h>

/** Rango útil del chip SX1262 (no el filtro SAW del T-Deck EU). */
constexpr float kSx1262MinMhz = 850.0f;
constexpr float kSx1262MaxMhz = 930.0f;

constexpr int kSpectrumMaxBins = 64;

struct DcnSpectrumSnapshot {
    float bins[kSpectrumMaxBins];
    int count = 0;
    float startMhz = 863.0f;
    float stepMhz = 0.25f;
    float minDbm = 0;
    float maxDbm = -120;
    bool live = false;
    bool busy = false;
    uint8_t rxBoost = 1; /**< 0 estándar, 1 boosted (ganancia RX). */
};

void dcnSpectrumClampRange(float &startMhz, float &endMhz);
bool dcnSpectrumRequestSweep(float startMhz, float endMhz, float stepMhz);
bool dcnSpectrumSetLive(bool enable, float centerMhz, float spanMhz, float stepMhz);
void dcnSpectrumSetRxBoost(uint8_t level);
const DcnSpectrumSnapshot &dcnSpectrumGetSnapshot();
void dcnSpectrumServiceTick();

#endif