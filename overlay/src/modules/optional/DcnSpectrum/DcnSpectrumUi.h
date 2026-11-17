#pragma once

#if defined(DCN_SPECTRUM_UI) && DCN_SPECTRUM_UI

void dcnSpectrumNotifyScanStart();
void dcnSpectrumNotifyScanProgress(int done, int total);
void dcnSpectrumShowGraph(const float *bins, int count, float minDbm, float maxDbm);

#endif