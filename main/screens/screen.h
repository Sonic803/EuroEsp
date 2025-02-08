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

class arc
{
public:
    int min;
    int max;
    int step;
    int &current;
    position pos;
    lv_obj_t *nome;
    lv_obj_t *valore;
    arc(lv_obj_t *scrn, lv_group_t *group, std::string nome, position pos, int &current, int min = 0, int max = 255, int step = 1);
    ~arc();
};

class title
{
public:
    title(lv_obj_t *scrn, lv_group_t *group, char* name, position pos);
    ~title();
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