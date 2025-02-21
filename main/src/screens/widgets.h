#pragma once

#include "esp_adc/adc_oneshot.h"
#include <vector>
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include <memory>

struct position
{
    int x;
    int y;
};

class arc
{
public:
    int &current;
    arc(lv_obj_t *scrn, lv_group_t *group, position pos, int &current, int min = 0, int max = 255, int step = 1);
};

class led
{
public:
    lv_obj_t *led_obj;
    led(lv_obj_t *scrn, lv_group_t *group, position pos, int brightness = 255, int size = 8);
    void set_brightness(int brightness);
};

class roller
{
public:
    roller(lv_obj_t *scrn, lv_group_t *group, position pos, char* options, int num_show,int width);
};

class label
{
public:
    label(lv_obj_t *scrn, lv_group_t *group, position pos, char *name);
};

void title_event_cb(lv_event_t *e);

class title
{
public:
    title(lv_obj_t *scrn, lv_group_t *group, position pos, char *name);
};
