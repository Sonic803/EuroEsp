#pragma once

#include "utils/libs.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_filter.h"

#include "esp_lcd_panel_vendor.h"
// #include "main.h"

#include "esp_timer.h"

#include "defines.h"
#include "math.h"

using namespace std;

#define WIDTH 128
#define HEIGHT 64
#define VALUES_SIZE 256 // The more time refresh takes, the bigger it needs to be

class scopeScreen : public screen
{
public:
    enableAdc enableadc = {true, false, true, false, false};
    enableOut enableout = {true, false, {false, false}, {false, false}};
    lv_obj_t *canvas;
    int *values;
    int *values_copy;
    float window_us = 10000;
    float time = 0;
    float values_time = 0;
    int current = 0;
    int trigger = 100;
    int last = 0;
    int direction = 0;
    bool rolling = true;
    bool full = false;
    scopeScreen();
    void switchMode();
    void IRAM_ATTR update() override;
    void IRAM_ATTR refresh() override;

};
