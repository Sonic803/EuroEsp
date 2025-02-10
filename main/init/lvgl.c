#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"

#include "esp_lcd_panel_vendor.h"

#include <unistd.h>
#include <sys/lock.h>
#include "esp_timer.h"

static const char *TAG = "espvco lvgl";


extern int encoder_diff;
extern bool encoder_pressed;

lv_indev_t *indev;


// Callback function for the encoder input device
static void encoder_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    // Report rotation (diff) to LVGL
    data->enc_diff = encoder_diff;
    encoder_diff = 0; // Clear the diff after reporting

    // Report button state to LVGL
    data->state = encoder_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

// Initialize the encoder as an LVGL input device
void encoder_init(void)
{
    // Initialize the LVGL input device driver

    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(indev, encoder_read_cb);

    // (Optional) Assign a group to the input device for focus management

    // lv_group_set_focus_cb(group, focus_cb);

    // lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_ACTIVE);

    ESP_LOGI(TAG, "Finished encoder group");
}

void configLvgl(void)
{
    encoder_init();
}
