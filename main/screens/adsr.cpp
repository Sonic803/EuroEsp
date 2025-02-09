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
#include "adsr.h"

using namespace std;

static const char *TAG = "adsr";

// TODO
//  vedere https://docs.lvgl.io/master/details/widgets/scale.html

adsrScreen::adsrScreen()
{

    t = new title(scrn, group, {20, 20}, "ADSR");

    state = RELEASE;
    // todo can be an issue
    arc t_arcs[] = {
        arc(scrn, group, {35, 35}, voltages[0]),
        arc(scrn, group, {64, 35}, voltages[1]),
        arc(scrn, group, {93, 35}, voltages[2]),
        arc(scrn, group, {35, 70}, times[0], 1),
        arc(scrn, group, {64, 70}, times[1], 1),
        arc(scrn, group, {93, 70}, times[2], 1)};
    
    label labels[]={
        label(scrn, group, {10, 35}, "LEV"),
        label(scrn, group, {35, 19}, "ATT"),
        label(scrn, group, {64, 19}, "SUS"),
        label(scrn, group, {93, 19}, "REL"),
        label(scrn, group, {10, 70}, "DUR"),
        label(scrn, group, {35, 54}, "ATT"),
        label(scrn, group, {64, 54}, "DEC"),
        label(scrn, group, {93, 54}, "REL"),
    };

    

    // lv_scr_load(scrn);
}

void IRAM_ATTR adsrScreen::update()
{
    int gate = pots_val[0] + jack_val[0];

    if (gate > MAX_ADC_VAL / 3)
    {
        if (state == RELEASE)
        {
            state = ATTACK;
        }
    }
    else
    {
        state = RELEASE;
    }
    float step;
    switch (state)
    {
    case ATTACK:
        step = (float)3 * times[0] / 255; // Maximum 3 seconds
        cur_val += (float)255 * TIMER_PERIOD_SEC / step;
        if (cur_val > voltages[0])
        {
            state = DECAY;
        }
        break;

    case DECAY:
        step = (float)3 * times[1] / 255; // Maximum 3 seconds

        cur_val -= (float)255 * TIMER_PERIOD_SEC / step;
        if (cur_val < voltages[1])
        {
            state = SUSTAIN;
        }
        break;

    case SUSTAIN:
        cur_val = voltages[1];
        break;

    case RELEASE:
        step = (float)3 * times[2] / 255; // Maximum 3 seconds
        cur_val -= (float)255 * TIMER_PERIOD_SEC / step;

        if (cur_val < voltages[2])
        {
            cur_val = voltages[2];
        }
        break;
    default:
        break;
    }
    vcoVal = (int)cur_val;
}
