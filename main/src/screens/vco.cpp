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
#include "vco.h"

#define IQ16toINT(val) ((val) >> 16)

using namespace std;

static const char *TAG = "vco";

// TODO
//  vedere https://docs.lvgl.io/master/details/widgets/scale.html

vcoScreen::vcoScreen()
{
    screen::enableadc = enableadc;
    screen::enableout = enableout;

    t = new title(scrn, group, {20, 20}, "VCO");

    sampling = 100000;
    // todo can be an issue
    arc t_arcs[] = {
        // arc(scrn, group, {35, 25}, shape, 0, 3, 1),
        // arc(scrn, group, {20, 50}, sampling, 1, 1000, 10),
        arc(scrn, group, {64, 25}, freq)};

    freq_label_text[0] = '\0';

    led_obj = new led(scrn, group, {35, 25}, 255,16);
    roller *roller_obj = new roller(scrn,
                                    group,
                                    {93, 25},
                                    "Sin\n"
                                    "Sqr\n"
                                    "Saw",
                                    1,
                                    30);

    lvgl_port_lock(0);

    freq_label = lv_label_create(scrn);
    lv_label_set_text_static(freq_label, freq_label_text);
    lv_obj_align(freq_label, LV_ALIGN_TOP_LEFT, 40, 50);
    lvgl_port_unlock();

    pitchTable = static_cast<float *>(heap_caps_malloc(PITCH_TABLE_SIZE * sizeof(float), MALLOC_CAP_INTERNAL));

    float curPitch = 1;
    for (int i = 0; i < PITCH_TABLE_SIZE; i++)
    {
        // Pitch table is 2**k with k from 0 to 6, evenly spaced in the 0-PITCH_TABLE_SIZE
        pitchTable[i] = curPitch;
        curPitch *= pow(2., 6.0 / PITCH_TABLE_SIZE);
    }
}

void IRAM_ATTR vcoScreen::update()
{
    int64_t start_time = esp_timer_get_time();

    frequency = (float)8 * pots_val[0] / MAX_ACTUAL_POTS_VAL;

    frequency = (float)frequency * pitchTable[pots_val[1] * PITCH_TABLE_SIZE / 6000];

    phase += frequency * sampling * TIMER_PERIOD_SEC;

    int64_t stop_time = esp_timer_get_time();

    time = (stop_time - start_time);

    if (phase >= sampling)
    {
        phase -= sampling;
    }
    // int val = 120 * sin((float)2 * M_PI * phase / sampling) + 120;
    val = 250 * phase / sampling;
    vcoVal = val;
    lfoVal = val;
    pwm1Val = val;
    pwm2Val = val;
}

void IRAM_ATTR vcoScreen::refresh()
{

    // led_obj->set_brightness(0);
    lvgl_port_lock(0);
    led_obj->set_brightness(val);
    snprintf(freq_label_text, sizeof(freq_label_text), "%.2f", frequency);
    lv_label_set_text(freq_label, freq_label_text);
    lvgl_port_unlock();
    // ESP_LOGI(TAG, "Time: %d", time);
}
