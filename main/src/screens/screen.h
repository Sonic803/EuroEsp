#pragma once

#include "esp_adc/adc_oneshot.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "main.h"
#include "widgets.h"
#include <memory>
#include <vector>

extern "C" {
extern int vcoVal;
extern int lfoVal;
extern int pwm1Val;
extern int pwm2Val;
extern bool updated;
extern adc_oneshot_unit_handle_t adc_handle;
extern int pots_val[2];
extern int jack_val[3];
extern adc_channel_t pwm1_chan, pwm2_chan;
}

class screen {
public:
  enableAdc enableadc = {{true, true}, {true, true, true}};
  enableOut enableout = {true, true, {true, true}, {true, true}};
  lv_obj_t *scrn;
  lv_group_t *group;
  title *t;
  screen();
  virtual void select();
  virtual void unselect();
  virtual void IRAM_ATTR update();
  virtual void IRAM_ATTR refresh();
};

extern std::vector<std::unique_ptr<screen>> screens;
extern int current_screen;

extern struct enableOut enable_out;

void title_event_cb(lv_event_t *e);