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
#include "screen.h"
#include "vco.h"

using namespace std;

static const char *TAG = "vco";

// TODO
//  vedere https://docs.lvgl.io/master/details/widgets/scale.html

vcoScreen::vcoScreen()
{

    t = new title(scrn, group, {20, 20}, "VCO");

    sampling = 100000;
    // todo can be an issue
    arc t_arcs[] = {
        arc(scrn, group, {20, 20}, shape, 0, 3, 1),
        // arc(scrn, group, {20, 50}, sampling, 1, 1000, 10),
        arc(scrn, group, {50, 20}, freq)};

    freq_label_text[0] = '\0';
    lvgl_port_lock(0);

    freq_label = lv_label_create(scrn);
    lv_label_set_text_static(freq_label, freq_label_text);
    lv_obj_align(freq_label, LV_ALIGN_TOP_LEFT, 40, 50);
    lvgl_port_unlock();

}

void IRAM_ATTR vcoScreen::update()
{
    frequency = (float)freq * 1000 / 255 + (float) 500*pots_val[0]/MAX_ADC_VAL+ (float) 500*pots_val[1]/MAX_ADC_VAL;
    phase += frequency * sampling * TIMER_PERIOD_SEC;
    if (phase >= sampling)
    {
        phase -= sampling;
    }
    // int val = 120 * sin((float)2 * M_PI * phase / sampling) + 120;
    int val = 250 * phase / sampling;
    vcoVal = val;
    lfoVal = val;
    pwm1Val = val;
    pwm2Val = val;
}

void IRAM_ATTR vcoScreen::refresh()
{
    lvgl_port_lock(0);

    snprintf(freq_label_text, sizeof(freq_label_text), "%.2f", frequency);
    lv_label_set_text(freq_label, freq_label_text);
    lvgl_port_unlock();

}
