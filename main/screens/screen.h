#include "esp_adc/adc_oneshot.h"
#include <vector>
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include <memory>

using namespace std;

extern int vcoVal;
extern int lfoVal;
extern int pwm1Val;
extern int pwm2Val;
extern bool updated;
extern adc_oneshot_unit_handle_t adc_handle;
extern int pots_val[2];
extern int jack_val[3];
extern adc_channel_t pwm1_chan, pwm2_chan;

struct position
{
    int x;
    int y;
};

class label
{
public:
    label(lv_obj_t *scrn, lv_group_t *group, position pos, char *name);
};

class arc
{
public:
    int &current;
    arc(lv_obj_t *scrn, lv_group_t *group, position pos, int &current, int min = 0, int max = 255, int step = 1);
};

class title
{
public:
    title(lv_obj_t *scrn, lv_group_t *group, position pos, char *name);
};

class screen
{
public:
    lv_obj_t *scrn;
    lv_group_t *group;
    title *t;
    screen();
    void select();
    void unselect();
    virtual void IRAM_ATTR update();
};

extern vector<std::unique_ptr<screen>> screens;
extern int current_screen;

void title_event_cb(lv_event_t *e);