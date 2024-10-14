#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "driver/dac_oneshot.h"
#include "main.h"

#define EXAMPLE_TIMER_RESOLUTION 2000000 // 1MHz, 1 tick = 1us
#define EXAMPLE_TIMER_ALARM_COUNT 60     // The count value that trigger the timer alarm callback

static dac_oneshot_handle_t chan0_handle;
static dac_oneshot_handle_t chan1_handle;

extern uint8_t sin_wav[EXAMPLE_ARRAY_LEN]; // Used to store sine wave values
extern uint8_t tri_wav[EXAMPLE_ARRAY_LEN]; // Used to store triangle wave values
extern uint8_t saw_wav[EXAMPLE_ARRAY_LEN]; // Used to store sawtooth wave values
extern uint8_t squ_wav[EXAMPLE_ARRAY_LEN]; // Used to store square wave values

extern float f;

/* Timer interrupt service routine */
static bool IRAM_ATTR on_timer_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    static float phase = 0;
    static float freq = 20;
    // freq*=1.00001;
    if (freq > 10000)
    {
        freq = 1;
    }
    int point = tri_wav[(int)phase];
    phase += (f / 20) / 10;
    phase = fmod(phase, EXAMPLE_ARRAY_LEN);
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan0_handle, point));
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan1_handle, 255 - point));
    return false;
}

void configDac(void)
{
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = EXAMPLE_TIMER_RESOLUTION, // 1MHz, 1 tick = 1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    dac_oneshot_config_t dac0_cfg = {
        .chan_id = DAC_CHAN_0,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&dac0_cfg, &chan0_handle));
    dac_oneshot_config_t dac1_cfg = {
        .chan_id = DAC_CHAN_1,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&dac1_cfg, &chan1_handle));

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = EXAMPLE_TIMER_ALARM_COUNT,
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_event_callbacks_t cbs = {
        .on_alarm = on_timer_alarm_cb,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}