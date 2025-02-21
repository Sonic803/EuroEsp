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

#include <string>
#include <algorithm>
#include <vector>

#include "esp_timer.h"

#include "defines.h"
#include "math.h"
#include "screen.h"

#include "peripherals/adc/adc.h"


using namespace std;

static const char *TAG = "screen";

vector<unique_ptr<screen>> screens;
int current_screen = 0;



screen::screen()
{
    scrn = lv_obj_create(NULL);
    group = lv_group_create();
}

void screen::unselect()
{
    lvgl_port_lock(0);
    lvgl_port_unlock();
}

extern lv_indev_t *indev;

void screen::select()
{
    enable_out = enableout;
    configAdcEnabled(enableadc);
    lvgl_port_lock(0);
    lv_scr_load(scrn);
    lv_indev_set_group(indev, group);
    lvgl_port_unlock();
}

void IRAM_ATTR screen::update()
{
}
void IRAM_ATTR screen::refresh()
{
}




void title_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_current_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_KEY)
    {
        uint32_t key = lv_event_get_key(e);
        if (key == LV_KEY_LEFT || key == LV_KEY_RIGHT)
        {
            lvgl_port_lock(0);

            screens[current_screen]->unselect();

            int next_screen = current_screen + (key == LV_KEY_LEFT ? -1 : 1);
            next_screen = (next_screen + screens.size()) % screens.size();

            current_screen = next_screen;
            ESP_LOGI(TAG, "Switching to screen %d", next_screen);

            screens[current_screen]->select();

            // REMOVE THIS LINE to stop forcing title focus
            // lv_group_focus_obj(title_btn);

            lvgl_port_unlock();
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
