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


    t = new title(scrn, group, "VCO", {20, 20});


    state = RELEASE;
    // todo can be an issue
    arc t_arcs[] = {
        arc(scrn, group, "A", {20, 20}, voltages[0]),
        arc(scrn, group, "D", {50, 20}, voltages[1]),
        arc(scrn, group, "S", {80, 20}, voltages[2]),
        arc(scrn, group, "A", {20, 40}, times[0], 1),
        arc(scrn, group, "D", {50, 40}, times[1], 1),
        arc(scrn, group, "S", {80, 40}, times[2], 1)};

    // lv_scr_load(scrn);
}

void IRAM_ATTR vcoScreen::update()
{
    vcoVal = (int)current;
    pwm1Val = (int)current + voltages[1] + pots_val[0];

    static float phase = 0;
    float freq = 10. + 10 * pots_val[0] / MAX_ADC_VAL;
    phase += freq * TIMER_PERIOD_SEC;
    if (phase > 1)
    {
        phase -= 1;
    }
    // int val = 128 * sin(2 * M_PI * phase) + 128;
    int val = 128 * phase + 128;
    vcoVal = val;
    lfoVal = val;
    pwm2Val = val;
}
