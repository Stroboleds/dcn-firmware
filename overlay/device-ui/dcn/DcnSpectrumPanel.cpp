#ifdef DCN_BRAND

#include "DcnSpectrumPanel.h"
#include "graphics/view/TFT/TFTView_320x240.h"
#include "screens.h"

#include <stdio.h>

extern "C" {
int dcn_spectrum_is_busy(void);
int dcn_spectrum_request_sweep(float start_mhz, float end_mhz, float step_mhz);
void dcn_spectrum_set_rx_boost(int on);
int dcn_spectrum_bin_count(void);
float dcn_spectrum_bin_mhz(int index);
float dcn_spectrum_bin_dbm(int index);
float dcn_spectrum_min_dbm(void);
float dcn_spectrum_max_dbm(void);
}

namespace
{
TFTView_320x240 *gView = nullptr;
lv_obj_t *gToolsBtn = nullptr;
lv_obj_t *gPanel = nullptr;
lv_obj_t *gTopBar = nullptr;
lv_obj_t *gGraphLbl = nullptr;
lv_obj_t *gStartSlider = nullptr;
lv_obj_t *gEndSlider = nullptr;
lv_obj_t *gBoostSw = nullptr;
lv_obj_t *gLiveSw = nullptr;
lv_obj_t *gStatusLbl = nullptr;
lv_timer_t *gTimer = nullptr;
bool gLive = false;

float sliderToMhz(int32_t v)
{
    return 850.0f + (v / 10.0f);
}

void refreshGraph()
{
    if (!gGraphLbl)
        return;
    const int n = dcn_spectrum_bin_count();
    if (n <= 0) {
        lv_label_set_text(gGraphLbl, "(sin datos)");
        return;
    }
    const float ymin = dcn_spectrum_min_dbm();
    const float ymax = dcn_spectrum_max_dbm();
    float span = ymax - ymin;
    if (span < 6)
        span = 6;
    char block[720];
    char *w = block;
    const size_t cap = sizeof(block);
    for (int i = 0; i < n && i < 18 && (size_t)(w - block) < cap - 24; i++) {
        int bars = (int)((dcn_spectrum_bin_dbm(i) - ymin) * 10.0f / span);
        if (bars < 0)
            bars = 0;
        if (bars > 10)
            bars = 10;
        int m = snprintf(w, cap - (size_t)(w - block), "%.1f ", dcn_spectrum_bin_mhz(i));
        if (m > 0)
            w += m;
        for (int b = 0; b < bars; b++)
            *w++ = '#';
        *w++ = '\n';
        *w = 0;
    }
    lv_label_set_text(gGraphLbl, block);
}

void timerCb(lv_timer_t *)
{
    if (dcn_spectrum_is_busy()) {
        lv_label_set_text(gStatusLbl, "Escaneando…");
        return;
    }
    refreshGraph();
    if (gLive) {
        int32_t a = lv_slider_get_value(gStartSlider);
        int32_t b = lv_slider_get_value(gEndSlider);
        float start = sliderToMhz(a < b ? a : b);
        float end = sliderToMhz(a < b ? b : a);
        if (dcn_spectrum_request_sweep(start, end, 0.25f))
            lv_label_set_text(gStatusLbl, "Live…");
    } else {
        lv_label_set_text(gStatusLbl, "Listo");
    }
}

void onStart(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    gLive = lv_obj_has_state(gLiveSw, LV_STATE_CHECKED);
    dcn_spectrum_set_rx_boost(lv_obj_has_state(gBoostSw, LV_STATE_CHECKED) ? 1 : 0);
    int32_t a = lv_slider_get_value(gStartSlider);
    int32_t b = lv_slider_get_value(gEndSlider);
    float start = sliderToMhz(a < b ? a : b);
    float end = sliderToMhz(a < b ? b : a);
    if (dcn_spectrum_request_sweep(start, end, 0.25f))
        lv_label_set_text(gStatusLbl, "Iniciado…");
    else
        lv_label_set_text(gStatusLbl, "Radio ocupada");
}

void openPanel(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED || !gView)
        return;
    gView->ui_set_active(gToolsBtn, gPanel, gTopBar);
}

void buildPanel()
{
    lv_obj_t *root = objects.main_screen ? objects.main_screen : lv_screen_active();
    gPanel = lv_obj_create(root);
    lv_obj_add_flag(gPanel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(gPanel, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(gPanel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(gPanel, 6, 0);

    gTopBar = lv_obj_create(gPanel);
    lv_obj_set_width(gTopBar, LV_PCT(100));
    lv_obj_set_height(gTopBar, 24);
    lv_obj_t *title = lv_label_create(gTopBar);
    lv_label_set_text(title, "DCN Espectro");

    lv_label_create(gPanel);
    lv_obj_t *ls = lv_obj_get_child(gPanel, -1);
    lv_label_set_text(ls, "Inicio MHz");
    gStartSlider = lv_slider_create(gPanel);
    lv_obj_set_width(gStartSlider, LV_PCT(92));
    lv_slider_set_range(gStartSlider, 0, 800);
    lv_slider_set_value(gStartSlider, 130, LV_ANIM_OFF);

    lv_label_create(gPanel);
    lv_obj_t *le = lv_obj_get_child(gPanel, -1);
    lv_label_set_text(le, "Fin MHz");
    gEndSlider = lv_slider_create(gPanel);
    lv_obj_set_width(gEndSlider, LV_PCT(92));
    lv_slider_set_range(gEndSlider, 0, 800);
    lv_slider_set_value(gEndSlider, 200, LV_ANIM_OFF);

    lv_obj_t *row = lv_obj_create(gPanel);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, 36);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    gBoostSw = lv_switch_create(row);
    lv_obj_add_state(gBoostSw, LV_STATE_CHECKED);
    lv_label_create(row);
    lv_label_set_text(lv_obj_get_child(row, -1), "Boost");
    gLiveSw = lv_switch_create(row);
    lv_label_create(row);
    lv_label_set_text(lv_obj_get_child(row, -1), "Live");

    lv_obj_t *btn = lv_button_create(gPanel);
    lv_obj_add_event_cb(btn, onStart, LV_EVENT_CLICKED, NULL);
    lv_label_create(btn);
    lv_label_set_text(lv_obj_get_child(btn, 0), "Escanear");

    gGraphLbl = lv_label_create(gPanel);
    lv_obj_set_width(gGraphLbl, LV_PCT(95));
    lv_label_set_long_mode(gGraphLbl, LV_LABEL_LONG_WRAP);
    lv_label_set_text(gGraphLbl, "---");

    gStatusLbl = lv_label_create(gPanel);
    lv_label_set_text(gStatusLbl, "850-930 MHz (filtro EU ~868)");

    gTimer = lv_timer_create(timerCb, 300, NULL);
}

void buildToolsButton()
{
    if (!objects.tools_signal_scanner_button)
        return;
    lv_obj_t *parent = lv_obj_get_parent(objects.tools_signal_scanner_button);
    gToolsBtn = lv_button_create(parent);
    lv_obj_set_size(gToolsBtn, lv_obj_get_width(objects.tools_signal_scanner_button), lv_obj_get_height(objects.tools_signal_scanner_button));
    lv_obj_t *lbl = lv_label_create(gToolsBtn);
    lv_label_set_text(lbl, "DCN\nEspectro");
    lv_obj_center(lbl);
    lv_obj_add_event_cb(gToolsBtn, openPanel, LV_EVENT_CLICKED, NULL);
}

} // namespace

void dcnSpectrumPanelInstall(TFTView_320x240 *view)
{
    gView = view;
    buildPanel();
    buildToolsButton();
}

#endif