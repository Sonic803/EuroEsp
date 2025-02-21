/**
 * @file custom_monoled.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "custom_monoled_private.h"
#include "core/lv_obj_private.h"
#include "core/lv_obj_class_private.h"

#if LV_USE_LED

#include "misc/lv_assert.h"
#include "themes/lv_theme.h"
#include "misc/lv_color.h"
#include "esp_log.h"


/*********************
 *      DEFINES
 *********************/
#define MY_CLASS (&custom_monoled_class)

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _custom_monoled_t custom_monoled_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void custom_monoled_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void custom_monoled_event(const lv_obj_class_t * class_p, lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/

const lv_obj_class_t custom_monoled_class  = {
    .base_class = &lv_obj_class,
    .constructor_cb = custom_monoled_constructor,
    .width_def = LV_DPI_DEF / 5,
    .height_def = LV_DPI_DEF / 5,
    .event_cb = custom_monoled_event,
    .instance_size = sizeof(custom_monoled_t),
    .name = "led",
};

static const char *TAG = "custom_monoled";

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * custom_monoled_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*=====================
 * Setter functions
 *====================*/

void custom_monoled_set_brightness(lv_obj_t * obj, uint8_t bright)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    custom_monoled_t * led = (custom_monoled_t *)obj;
    if(led->bright == bright) return;

    led->bright = LV_CLAMP(CUSTOM_MONOLED_BRIGHT_MIN, bright, CUSTOM_MONOLED_BRIGHT_MAX);

    /*Invalidate the object there fore it will be redrawn*/
    lv_obj_invalidate(obj);
}

void custom_monoled_on(lv_obj_t * led)
{
    custom_monoled_set_brightness(led, CUSTOM_MONOLED_BRIGHT_MAX);
}

void custom_monoled_off(lv_obj_t * led)
{
    custom_monoled_set_brightness(led, CUSTOM_MONOLED_BRIGHT_MIN);
}

void custom_monoled_toggle(lv_obj_t * obj)
{
    uint8_t bright = custom_monoled_get_brightness(obj);
    if(bright > (CUSTOM_MONOLED_BRIGHT_MIN + CUSTOM_MONOLED_BRIGHT_MAX) >> 1)
        custom_monoled_off(obj);
    else
        custom_monoled_on(obj);
}

/*=====================
 * Getter functions
 *====================*/

uint8_t custom_monoled_get_brightness(const lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    custom_monoled_t * led = (custom_monoled_t *)obj;
    return led->bright;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void custom_monoled_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    custom_monoled_t * led = (custom_monoled_t *)obj;
    led->bright = CUSTOM_MONOLED_BRIGHT_MAX;
}
static void custom_monoled_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);

    lv_result_t res;

    /* Call the ancestor's event handler */
    lv_event_code_t code = lv_event_get_code(e);
    if(code != LV_EVENT_DRAW_MAIN && code != LV_EVENT_DRAW_MAIN_END) {
        res = lv_obj_event_base(MY_CLASS, e);
        if(res != LV_RESULT_OK) return;
    }

    lv_obj_t * obj = lv_event_get_current_target(e);
    if(code == LV_EVENT_DRAW_MAIN) {
        custom_monoled_t * led = (custom_monoled_t *)obj;
        lv_layer_t * layer = lv_event_get_layer(e);

        lv_draw_rect_dsc_t rect_dsc;
        lv_draw_rect_dsc_init(&rect_dsc);
        rect_dsc.base.layer = layer;
        lv_obj_init_draw_rect_dsc(obj, LV_PART_MAIN, &rect_dsc);

        rect_dsc.bg_color = lv_color_black();
        rect_dsc.bg_opa = LV_OPA_COVER;

        int base_size = lv_obj_get_width(obj);  /* Object size */
        // int radius = (base_size / 2) * (led->bright + 1) / (CUSTOM_MONOLED   _BRIGHT_MAX + 1);
        int diameter = (base_size-1) * (led->bright) / (CUSTOM_MONOLED_BRIGHT_MAX + 1);
        // int diameter = radius * 2;

        /* Set radius to make it a circle */
        rect_dsc.radius = LV_RADIUS_CIRCLE;

        /* Define the area for the circle */
        lv_area_t area;
        int32_t center_x, center_y;
        area.x1 = obj->coords.x1 + (lv_obj_get_width(obj) / 2) - diameter/2;
        area.y1 = obj->coords.y1 + (lv_obj_get_height(obj) / 2) - diameter/2;
        area.x2 = area.x1 + diameter;
        area.y2 = area.y1 + diameter;

        /* Draw the filled circle */
        lv_draw_rect(layer, &rect_dsc, &area);
    }
}


#endif