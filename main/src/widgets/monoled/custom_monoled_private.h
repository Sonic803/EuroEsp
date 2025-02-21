/**
 * @file custom_monoled_private.h
 *
 */

#ifndef CUSTOM_MONOLED_PRIVATE_H
#define CUSTOM_MONOLED_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "custom_monoled.h"

#if LV_USE_LED
#include "core/lv_obj_private.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/** Data of led */
struct _custom_monoled_t {
    lv_obj_t obj;
    uint8_t bright;     /**< Current brightness of the LED (0..255)*/
};


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_LED */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*CUSTOM_MONOLED_PRIVATE_H*/