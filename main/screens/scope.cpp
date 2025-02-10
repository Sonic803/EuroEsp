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
#include "scope.h"

#define COLOR_FG lv_color_black()
#define COLOR_BG lv_color_white()

using namespace std;

static const char *TAG = "vco";

// TODO
//  vedere https://docs.lvgl.io/master/details/widgets/scale.html

static DRAM_ATTR uint8_t *buffer;

void canvas_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_current_target(e);
    scopeScreen *scope = (scopeScreen *)lv_event_get_user_data(e); // Get instance

    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_KEY)
    {
        uint32_t key = lv_event_get_key(e);
        if (key == LV_KEY_LEFT)
        {
            scope->window_us = min(100000., scope->window_us * 1.3); // Zoom out, ensure div is at least 1
        }
        else if (key == LV_KEY_RIGHT)
        {
            scope->window_us = max(500., scope->window_us / 1.3); // Zoom out, ensure div is at least 1
        }
    }
    else if (code == LV_EVENT_CLICKED)
    {

        lv_group_t *g = lv_obj_get_group(obj);
        bool editing = lv_group_get_editing(g);
        lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_active());
        if (indev_type == LV_INDEV_TYPE_ENCODER)
        {
            if (editing)
                lv_group_set_editing(g, false);
        }
    }
}

scopeScreen::scopeScreen()
{

    enable_out = {true, false, {false, false}, {false, false}};
    configAdcEnabled((struct enableAdc){.pots = {true, false}, .jacks = {true, false, false}});

    for (int i = 0; i < WIDTH; i++)
    {
        values[i] = 0;
    }

    lv_obj_set_scrollbar_mode(scrn, LV_SCROLLBAR_MODE_OFF);

    t = new title(scrn, group, {20, 20}, "Scope");

    lvgl_port_lock(0);
    canvas = lv_canvas_create(scrn);
    // TODO malloc
    // buffer = static_cast<uint8_t *>(lv_malloc(WIDTH * HEIGHT * 2));
    buffer = static_cast<uint8_t *>(heap_caps_malloc(WIDTH * HEIGHT * 2, MALLOC_CAP_INTERNAL));

    lv_canvas_set_buffer(canvas, buffer, WIDTH, HEIGHT, LV_COLOR_FORMAT_RGB565);
    lv_obj_align(canvas, LV_ALIGN_TOP_MID, 0, 15);
    lv_canvas_fill_bg(canvas, COLOR_BG, LV_OPA_COVER);

    lv_obj_add_flag(canvas, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_add_event_cb(canvas, canvas_event_cb, LV_EVENT_ALL, this); // Handle rotation
    lv_group_add_obj(group, canvas);

    lvgl_port_unlock();
}

void IRAM_ATTR scopeScreen::update()
{
    float frequency = 1000 * pots_val[0] / MAX_ADC_VAL;
    int sampling = 100000;
    static long phase = 0;
    phase += frequency * sampling * TIMER_PERIOD_SEC;
    if (phase >= sampling)
    {
        phase -= sampling;
    }
    int val = 120 * sin((float)2 * M_PI * phase / sampling) + 120;
    // int val = 200 * phase / sampling;
    vcoVal = val;
    lfoVal = val;
    pwm1Val = val;
    pwm2Val = val;

    if (updated)
    {
        time+=TIMER_PERIOD_MICRO;
        int val = jack_val[0];
        while (time > values_time && current != WIDTH)
        {
            values[current] = val;
            values_time += window_us / WIDTH;
            current = current + 1;
        }
        if (current == WIDTH)
        {
            current=0;
            values_time=0;
            time=0;
            updated = false;
        }
    }
}

void IRAM_ATTR scopeScreen::refresh()
{

    // static int64_t last_refresh_time = esp_timer_get_time();
    // int64_t current_time = esp_timer_get_time();
    // int64_t elapsed_time = current_time - last_refresh_time;
    // last_refresh_time = current_time;

    // ESP_EARLY_LOGI(TAG, "Elapsed time refresh: %lld ms", elapsed_time/1000);

    lvgl_port_lock(0);

    // Clear the canvas
    if (!updated)
    {

        lv_canvas_fill_bg(canvas, COLOR_BG, LV_OPA_COVER);

        for (int i = 0; i < WIDTH; i++)
        {
            int y = (values[i] * (HEIGHT - 1)) / MAX_ADC_VAL;
            y = (HEIGHT - 1) - y;
            y= min(HEIGHT-1, max(0, y));
            y = max(0, y);
            lv_canvas_set_px(canvas, i, y, COLOR_FG, LV_OPA_COVER);
        }
        updated = true;
    }

    lvgl_port_unlock();
}
