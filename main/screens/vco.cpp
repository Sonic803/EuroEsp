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

    state = RELEASE;
    todo=1;
    // todo can be an issue
    arc t_arcs[] = {
        arc(scrn, group, {20, 20}, shape, 0, 3, 1),
        arc(scrn, group, {20, 50}, todo, 1, 1000, 10),
        arc(scrn, group, {50, 20}, freq)};
    // lv_scr_load(scrn);
}

void IRAM_ATTR vcoScreen::update()
{
    float frequency = (float)freq*1000/255 + (float) 1000*pots_val[1]/MAX_ADC_VAL;
    phase += frequency * TIMER_PERIOD_SEC * todo;
    if (phase >= todo)
    {
        phase -= todo;
    }
    // int val = 128 * sin(2 * M_PI * phase) + 128;
    int val = 250 * phase / todo;
    vcoVal = val;
    lfoVal = val;
    pwm1Val = val;
    pwm2Val = val;
}
