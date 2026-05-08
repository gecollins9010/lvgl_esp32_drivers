#ifndef __TSC2007_H
/*
* Copyright © 2026 Gregory Collins

* Permission is hereby granted, free of charge, to any person obtaining a copy of this
* software and associated documentation files (the “Software”), to deal in the Software
* without restriction, including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
* to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
* PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
* FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#define __TSC2007_H

#include <stdint.h>
#include <stdbool.h>
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TSC2007_I2C_SLAVE_ADDR   0x48

// Defines for TSC2007 i2c function commands
/* TSC2007 Converter Function Select are bits 7-4 of commands */
#define TSC2007_FNC_MEASURE_TEMP0 0b00000000
#define TSC2007_FNC_MEASURE_AUX 0b00100000
#define TSC2007_FNC_MEASURE_TEMP1 0b01000000
#define TSC2007_FNC_ACTIVATE_X 0b10000000
#define TSC2007_FNC_ACTIVATE_Y 0b10010000
#define TSC2007_FNC_ACTIVATE_YPLUS_X 0b10100000
#define TSC2007_FNC_SETUP_COMMAND 0b10110000
#define TSC2007_FNC_MEASURE_X 0b11000000
#define TSC2007_FNC_MEASURE_Y 0b11010000
#define TSC2007_FNC_MEASURE_Z1 0b11100000
#define TSC2007_FNC_MEASURE_Z2 0b11110000

/* TSC2007 cmd bits 3-2 control the AD converter and IRQ pin*/
#define TSC2007_POWERDOWN_IRQON 0b00000000
#define TSC2007_ADON_IRQOFF 0b00000100
#define TSC2007_ADOFF_IRQON 0b00001000
#define TSC2007_ADOFF_IRQOFF 0b00001100

/* TSC2007 cmd bit 1 12 bit low speed (1) or 8 bit high speed (0) */
#define TSC2007_ADC_12BIT 0b00000010
#define TSC2007_ADC_8BIT 0b00000000

/* TSC2007 cmd bit 0 is does not matter '0' */

// TSC2007 defines for x,y coordinate handling and touch detection

#define TSC2007_IRQ CONFIG_LV_TSC2007_TOUCH_PIN_IRQ

#define XPT2046_AVG             4
#define TSC2007_X_MIN           CONFIG_LV_TSC2007_TOUCH_X_MIN
#define TSC2007_Y_MIN           CONFIG_LV_TSC2007_TOUCH_Y_MIN
#define TSC2007_X_MAX           CONFIG_LV_TSC2007_TOUCH_X_MAX
#define TSC2007_Y_MAX           CONFIG_LV_TSC2007_TOUCH_Y_MAX
#define TSC2007_X_INV           CONFIG_LV_TSC2007_TOUCH_INVERT_X
#define TSC2007_Y_INV           CONFIG_LV_TSC2007_TOUCH_INVERT_Y
#define TSC2007_XY_SWAP 		    CONFIG_LV_TSC2007_TOUCH_XY_SWAP
#define TSC2007_TOUCH_THRESHOLD CONFIG_LV_TSC2007_TOUCH_THRESHOLD // Threshold for touch detection
#define TSC2007_TOUCH_IRQ       CONFIG_LV_TSC2007_TOUCH_DETECT_IRQ
#define TSC2007_TOUCH_IRQ_PRESS CONFIG_LV_TSC2007_TOUCH_DETECT_IRQ_PRESSURE
#define TSC2007_TOUCH_PRESS     CONFIG_LV_TSC2007_TOUCH_DETECT_PRESSURE

typedef struct {
    bool inited;
} tsc2007_status_t;

/**
  * @brief  Initialize for TSC2007 communication via I2C
  * @param  dev_addr: Device address on communication Bus (I2C slave address of TSC2007).
  * @retval None
  */
void tsc2007_init(uint16_t dev_addr);

/**
  * @brief  Get the touch screen X and Y positions values. Ignores multi touch
  * @param  drv:
  * @param  data: Store data here
  * @retval Always false
  */
bool tsc2007_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

#ifdef __cplusplus
}
#endif
#endif /* __FT2007_H */
