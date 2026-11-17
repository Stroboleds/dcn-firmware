#pragma once

#include "lvgl.h"

class TFTView_320x240;

#ifdef DCN_BRAND

/** Registra botón en Herramientas y panel LVGL (v3). */
void dcnSpectrumPanelInstall(TFTView_320x240 *view);

#endif