#include "esp_log.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"


static const char *TAG = "espvco lvgl";


extern int encoder_diff;
extern bool encoder_pressed;

lv_indev_t *indev;


// Callback function for the encoder input device
static void encoder_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    data->enc_diff = encoder_diff;
    encoder_diff = 0;
    data->state = encoder_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

// Initialize the encoder as an LVGL input device
void encoder_init(void)
{
    // Initialize the LVGL input device driver

    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_mode(indev, LV_INDEV_MODE_EVENT);
    lv_indev_set_scroll_throw(indev, 1);
    lv_indev_set_read_cb(indev, encoder_read_cb);
    // lv_group_set_focus_cb(group, focus_cb);
    // lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_ACTIVE);

    ESP_LOGI(TAG, "Finished encoder group");
}

void configLvgl(void)
{
    encoder_init();
}
