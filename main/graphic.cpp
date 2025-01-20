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

using namespace std;

static const char *TAG = "graphic";

// TODO
//  vedere https://docs.lvgl.io/master/details/widgets/scale.html

extern int vcoVal;
extern int lfoVal;
extern int pwm1Val;
extern int pwm2Val;
extern bool updated;
extern adc_oneshot_unit_handle_t adc_handle;
extern int pots_val[2];
extern int jack_val[3];
extern adc_channel_t pwm1_chan, pwm2_chan;

extern "C" void getEncoderValue(int *ret_val, bool *pressed);

static lv_group_t *group;

struct position
{
    int x;
    int y;
};

static void value_changed_event_cb(lv_event_t *e)
{
    lv_obj_t *arc = (lv_obj_t *)lv_event_get_target(e);
    int *value = static_cast<int *>(lv_event_get_user_data(e));
    *value = lv_arc_get_value(arc);
    ESP_LOGI(TAG, "Value changed to %d", *value);
}

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
    arc(std::string nome, position pos, int &current, int min = 0, int max = 255, int step = 1) : current(current)
    {
        this->min = min;
        this->max = max;
        this->step = step;
        this->pos = pos;

        lvgl_port_lock(0);

        lv_obj_t *arc = lv_arc_create(lv_scr_act());
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

        ESP_LOGI(TAG, "Finished creating arc");
    }
    ~arc()
    {
    }
};

class screen
{
public:
    vector<arc> arcs;
    bool selected = false;
    int hoveredBlocco = 0;
    screen()
    {
    }
    void addArc(arc a)
    {
        arcs.push_back(a);
    }
};

class adsrScreen : public screen
{
public:
    enum
    {
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE
    } state;
    int voltages[3] = {200, 170, 0};
    int times[3] = {100, 100, 100};
    float current = 0;
    adsrScreen()
    {
        state = RELEASE;
        arc t_arcs[] = {
            arc("A", {20, 10}, voltages[0]),
            arc("D", {50, 10}, voltages[1]),
            arc("S", {80, 10}, voltages[2]),
            arc("A", {20, 30}, times[0], 1),
            arc("D", {50, 30}, times[1], 1),
            arc("S", {80, 30}, times[2], 1)
            };

        // for (int i = 0; i < 4; i++)
        // {
        //     addArc(t_arcs[i]);
        // }
    }
    void IRAM_ATTR update()
    {
        int gate = pots_val[0] + jack_val[0];
        // // ESP_EARLY_LOGI(TAG, "GATE: %d", gate);

        if (gate > MAX_ADC_VAL / 3)
        {
            if (state == RELEASE)
            {
                state = ATTACK;
            }
        }
        else
        {
            state = RELEASE;
        }
        float step;
        switch (state)
        {
        case ATTACK:
            step = (float)3 * times[0] / 255;
            current += (float)255 * TIMER_PERIOD / 1000000 / step;
            if (current > voltages[0])
            {
                state = DECAY;
            }
            break;

        case DECAY:
            step = (float)3 * times[1] / 255;

            current -= (float)255 * TIMER_PERIOD / 1000000 / step;
            if (current < voltages[1])
            {
                state = SUSTAIN;
            }
            break;

        case SUSTAIN:
            current = voltages[1];
            break;

        case RELEASE:
            step = (float)3 * times[2] / 255;
            current -= (float)255 * TIMER_PERIOD / 1000000 / step;

            if (current < voltages[2])
            {
                current = voltages[2];
            }
            break;
        default:
            break;
        }
        vcoVal = (int)current;
        pwm1Val = (int)current + voltages[1] + pots_val[0];

        static float phase = 0;
        float freq = 10. + 10000 * pots_val[0] / MAX_ADC_VAL;
        phase += freq * TIMER_PERIOD / 1000000;
        if (phase > 1)
        {
            phase -= 1;
        }
        // int val = 128 * sin(2 * M_PI * phase) + 128;
        int val = 128 * phase + 128;
        vcoVal = val;
        lfoVal = val;
        pwm2Val = val;
    }
};

adsrScreen *a;
typedef void (*FunctionPointer)();

FunctionPointer updateFunction = NULL;

// Simulated encoder state variables
static int32_t encoder_diff = 0;     // Tracks encoder rotation steps
static bool encoder_pressed = false; // Tracks button press state

// Callback function for the encoder input device
static void encoder_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
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

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(indev, encoder_read_cb);

    // (Optional) Assign a group to the input device for focus management
    group = lv_group_create();
    lv_indev_set_group(indev, group);
    // lv_group_set_focus_cb(group, focus_cb);

    // lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);

    ESP_LOGI(TAG, "Finished encoder group");
}

extern "C" void IRAM_ATTR slow_lfo()
{
    static float i = 0;
    i += (float)255 * TIMER_PERIOD / 1000000 / 1;
    if (i > 255)
    {
        i = 0;
    }
    // lfoVal=int(i);
}

extern "C" void IRAM_ATTR update()
{
    a->update();
    // slow_lfo();
}

extern "C" void startGraphic()
{
    ESP_LOGI(TAG, "Starting graphic");
    encoder_init();
    a = new adsrScreen();
    updateFunction = update;
}

extern "C" void runGraphic()
{

    int rotation;
    bool pressed;
    getEncoderValue(&rotation, &pressed);
    encoder_diff += rotation;
    encoder_pressed = pressed;

}
