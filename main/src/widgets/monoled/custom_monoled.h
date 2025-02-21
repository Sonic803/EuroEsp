/**
 * @file custom_monoled.h
 *
 */

#ifndef CUSTOM_MONOLED_H
#define CUSTOM_MONOLED_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "core/lv_obj.h"

#if LV_USE_LED

/*********************
 *      DEFINES
 *********************/
/** Brightness when the LED if OFF */
#ifndef CUSTOM_MONOLED_BRIGHT_MIN
# define CUSTOM_MONOLED_BRIGHT_MIN 0
#endif

/** Brightness when the LED if ON */
#ifndef CUSTOM_MONOLED_BRIGHT_MAX
# define CUSTOM_MONOLED_BRIGHT_MAX 255
#endif

/**********************
 *      TYPEDEFS
 **********************/

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t custom_monoled_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a led object
 * @param parent    pointer to an object, it will be the parent of the new led
 * @return          pointer to the created led
 */
lv_obj_t * custom_monoled_create(lv_obj_t * parent);

/**
 * Set the brightness of a LED object
 * @param led       pointer to a LED object
 * @param bright    CUSTOM_MONOLED_BRIGHT_MIN (max. dark) ... CUSTOM_MONOLED_BRIGHT_MAX (max. light)
 */
void custom_monoled_set_brightness(lv_obj_t * led, uint8_t bright);

/**
 * Light on a LED
 * @param led       pointer to a LED object
 */
void custom_monoled_on(lv_obj_t * led);

/**
 * Light off a LED
 * @param led       pointer to a LED object
 */
void custom_monoled_off(lv_obj_t * led);

/**
 * Toggle the state of a LED
 * @param led       pointer to a LED object
 */
void custom_monoled_toggle(lv_obj_t * led);

/**
 * Get the brightness of a LED object
 * @param obj       pointer to LED object
 * @return bright   0 (max. dark) ... 255 (max. light)
 */
uint8_t custom_monoled_get_brightness(const lv_obj_t * obj);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_LED*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*CUSTOM_MONOLED_H*/