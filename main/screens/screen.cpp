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

using namespace std;

static const char *TAG = "screen";

vector<std::unique_ptr<screen>> screens;
int current_screen = 0;

static void value_changed_event_cb(lv_event_t *e)
{
    lv_obj_t *arc = (lv_obj_t *)lv_event_get_target(e);
    int *value = static_cast<int *>(lv_event_get_user_data(e));
    *value = lv_arc_get_value(arc);
    // ESP_LOGI(TAG, "Value changed to %d", *value);
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

arc::arc(lv_obj_t *scrn, lv_group_t *group, std::string nome, position pos, int &current, int min, int max, int step) : current(current)
{
    this->min = min;
    this->max = max;
    this->step = step;
    this->pos = pos;

    lvgl_port_lock(0);

    lv_obj_t *arc = lv_arc_create(scrn);
    lv_obj_set_size(arc, 20, 20);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_obj_align(arc, LV_ALIGN_TOP_LEFT, pos.x, pos.y);
    lv_group_add_obj(group, arc); // Add the arc to the group

    lv_arc_set_range(arc, min, max);
    lv_arc_set_value(arc, current);
    lv_arc_set_change_rate(arc, step);

    lv_obj_add_flag(arc, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_obj_add_event_cb(arc, value_changed_event_cb, LV_EVENT_VALUE_CHANGED, &current);

    lvgl_port_unlock();

    // ESP_LOGI(TAG, "Finished creating arc");
}
arc::~arc()
{
}
title::title(lv_obj_t *scrn, lv_group_t *group, char* nome, position pos)
{
    lvgl_port_lock(0);

    lv_obj_t *title_label = lv_label_create(scrn);
    lv_label_set_text(title_label, nome);
    // lv_obj_set_style_text_color(title_label, lv_color_black(), 0); // Ensure visibility

    static lv_style_t style_title;
    lv_style_init(&style_title);

    static lv_style_t style_title_focus;
    lv_style_init(&style_title_focus);
    lv_style_set_text_decor(&style_title_focus, LV_TEXT_DECOR_UNDERLINE);

    static lv_style_t style_title_edited;
    lv_style_init(&style_title_edited);
    lv_style_set_text_decor(&style_title_edited, LV_TEXT_DECOR_UNDERLINE);
    lv_style_set_bg_opa(&style_title_edited, LV_OPA_COVER);
    lv_style_set_bg_color(&style_title_edited, lv_color_black());
    lv_style_set_text_color(&style_title_edited, lv_color_white());

    lv_obj_add_style(title_label, &style_title, LV_STATE_DEFAULT);
    lv_obj_add_style(title_label, &style_title_focus, LV_STATE_FOCUSED);
    lv_obj_add_style(title_label, &style_title_edited, LV_STATE_EDITED);

    lv_obj_add_flag(title_label, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(title_label, title_event_cb, LV_EVENT_ALL, NULL); // Handle rotation

    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 0);

    lv_group_add_obj(group, title_label);

    lv_group_set_editing(group, true);

    lvgl_port_unlock();
}
title::~title()
{
}

screen::screen()
{
    scrn = lv_obj_create(NULL);
    group = lv_group_create();
}

void screen::unselect()
{
    lvgl_port_lock(0);
    // lv_group_focus_freeze(group, true);
    lvgl_port_unlock();
}

extern lv_indev_t *indev;

void screen::select()
{
    lvgl_port_lock(0);
    lv_scr_load(scrn);
    // lv_group_focus_freeze(group, false);
    // lv_group_focus_obj(lv_group_get_focused(group));
    // lv_group_set_default(group);  // Switch to group 2
    lv_indev_set_group(indev, group);

    lvgl_port_unlock();
}


void IRAM_ATTR screen::update()
{
}
