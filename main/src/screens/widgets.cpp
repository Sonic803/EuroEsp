#include "widgets.h"
#include "esp_log.h"

#include "esp_lvgl_port.h"
#include "widgets/monoled/custom_monoled.h"

static const char *TAG = "widgets";

static void value_changed_event_cb(lv_event_t *e)
{
    lv_obj_t *arc = (lv_obj_t *)lv_event_get_target(e);
    int *value = static_cast<int *>(lv_event_get_user_data(e));
    *value = lv_arc_get_value(arc);
    // ESP_LOGI(TAG, "Value changed to %d", *value);
}

arc::arc(lv_obj_t *scrn, lv_group_t *group, position pos, int &current, int min, int max, int step) : current(current)
{
    lvgl_port_lock(0);

    lv_obj_t *arc = lv_arc_create(scrn);
    int arc_size = 20; // Size of the arc
    lv_obj_set_size(arc, arc_size, arc_size);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_obj_align(arc, LV_ALIGN_TOP_LEFT, pos.x - (arc_size / 2), pos.y - (arc_size / 2));

    lv_group_add_obj(group, arc); // Add the arc to the group

    lv_arc_set_range(arc, min, max);
    lv_arc_set_value(arc, current);
    lv_arc_set_change_rate(arc, step);

    lv_obj_add_flag(arc, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_obj_add_event_cb(arc, value_changed_event_cb, LV_EVENT_VALUE_CHANGED, &current);

    lvgl_port_unlock();

    // ESP_LOGI(TAG, "Finished creating arc");
}

led::led(lv_obj_t *scrn, lv_group_t *group, position pos, int brightness, int size)
{
    lvgl_port_lock(0);

    led_obj = custom_monoled_create(scrn);
    custom_monoled_set_brightness(led_obj, brightness);
    lv_obj_set_size(led_obj, size, size);
    lv_obj_align(led_obj, LV_ALIGN_TOP_LEFT, pos.x - size / 2, pos.y - size / 2);

    lvgl_port_unlock();
}

void led::set_brightness(int brightness)
{
    lvgl_port_lock(0);
    custom_monoled_set_brightness(led_obj, brightness);
    lvgl_port_unlock();
}
static void roller_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    // if (code == LV_EVENT_VALUE_CHANGED)
    // {
    //     char buf[32];
    //     lv_roller_get_selected_str(obj, buf, sizeof(buf));
    //     ESP_LOGI(TAG, "Selected: %s\n", buf);
    // }
    if (code == LV_EVENT_KEY)
    {
        uint32_t key = lv_event_get_key(e);
        if (key == LV_KEY_LEFT || key == LV_KEY_RIGHT)
        {
            char buf[32];
            lv_roller_get_selected_str(obj, buf, sizeof(buf));
            ESP_LOGI(TAG, "Selected: %s", buf);
        }
    }
}

roller::roller(lv_obj_t *scrn, lv_group_t *group, position pos, char *options, int num_show, int width)
{
    lvgl_port_lock(0);

    lv_obj_t *roller_obj = lv_roller_create(scrn);

    lv_roller_set_options(roller_obj, options, LV_ROLLER_MODE_INFINITE);
    // lv_obj_center(roller_obj);
    // lv_obj_set_size(led_obj, size, size);
    lv_obj_set_width(roller_obj, width);
    lv_roller_set_visible_row_count(roller_obj, num_show);

    lv_obj_update_layout(roller_obj);

    int height = lv_obj_get_height(roller_obj);

    lv_obj_align(roller_obj, LV_ALIGN_TOP_LEFT, pos.x - width / 2, pos.y - height / 2);

    lv_group_add_obj(group, roller_obj); // Add the arc to the group

    lv_obj_add_event_cb(roller_obj, roller_event_handler, LV_EVENT_ALL, NULL);

    lvgl_port_unlock();
}

label::label(lv_obj_t *scrn, lv_group_t *group, position pos, char *name)
{
    lvgl_port_lock(0);
    lv_obj_t *name_label = lv_label_create(scrn);
    lv_label_set_text(name_label, name);
    // Set text alignment to center
    lv_obj_set_style_text_align(name_label, LV_TEXT_ALIGN_CENTER, 0);

    // Refresh the layout to get correct width and height
    lv_obj_update_layout(name_label);

    // Get label size
    int label_w = lv_obj_get_width(name_label);
    int label_h = lv_obj_get_height(name_label);

    // Adjust position so the center of the label is at pos.x, pos.y
    lv_obj_align(name_label, LV_ALIGN_TOP_LEFT, pos.x - (label_w / 2), pos.y - (label_h / 2));

    lvgl_port_unlock();
}

title::title(lv_obj_t *scrn, lv_group_t *group, position pos, char *nome)
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
    lv_obj_add_flag(title_label, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_obj_add_event_cb(title_label, title_event_cb, LV_EVENT_ALL, NULL); // Handle rotation

    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 0);

    lv_group_add_obj(group, title_label);

    lv_group_set_editing(group, true);

    lvgl_port_unlock();
}
