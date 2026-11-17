#pragma once

/** Puente C para pantalla LVGL (device-ui v3). Enlazado desde firmware DCN. */
#ifdef __cplusplus
extern "C" {
#endif

int dcn_spectrum_is_busy(void);
int dcn_spectrum_request_sweep(float start_mhz, float end_mhz, float step_mhz);
void dcn_spectrum_set_rx_boost(int on);
int dcn_spectrum_bin_count(void);
float dcn_spectrum_bin_mhz(int index);
float dcn_spectrum_bin_dbm(int index);
float dcn_spectrum_min_dbm(void);
float dcn_spectrum_max_dbm(void);

#ifdef __cplusplus
}
#endif