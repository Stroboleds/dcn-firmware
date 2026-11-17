#if defined(DCN_SPECTRUM_UI) && DCN_SPECTRUM_UI

#include "DcnSpectrumBridge.h"
#include "DcnSpectrumService.h"

extern "C" int dcn_spectrum_is_busy(void)
{
    return dcnSpectrumGetSnapshot().busy ? 1 : 0;
}

extern "C" int dcn_spectrum_request_sweep(float start_mhz, float end_mhz, float step_mhz)
{
    return dcnSpectrumRequestSweep(start_mhz, end_mhz, step_mhz) ? 1 : 0;
}

extern "C" void dcn_spectrum_set_rx_boost(int on)
{
    dcnSpectrumSetRxBoost(on ? 1 : 0);
}

extern "C" int dcn_spectrum_bin_count(void)
{
    return dcnSpectrumGetSnapshot().count;
}

extern "C" float dcn_spectrum_bin_mhz(int index)
{
    const auto &s = dcnSpectrumGetSnapshot();
    if (index < 0 || index >= s.count)
        return 0;
    return s.startMhz + index * s.stepMhz;
}

extern "C" float dcn_spectrum_bin_dbm(int index)
{
    const auto &s = dcnSpectrumGetSnapshot();
    if (index < 0 || index >= s.count)
        return 0;
    return s.bins[index];
}

extern "C" float dcn_spectrum_min_dbm(void)
{
    return dcnSpectrumGetSnapshot().minDbm;
}

extern "C" float dcn_spectrum_max_dbm(void)
{
    return dcnSpectrumGetSnapshot().maxDbm;
}

#endif