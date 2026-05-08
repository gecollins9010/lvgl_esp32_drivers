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

#include <esp_log.h>
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include <lvgl.h>
#else
#include <lvgl/lvgl.h>
#endif
#include "tsc2007.h"
#include "lvgl_i2c/i2c_manager.h"

#define TAG "TSC2007"


static tsc2007_status_t tsc2007_status;
static uint8_t current_dev_addr;       // set during init
//static ft6x36_touch_t touch_inputs = { -1, -1, LV_INDEV_STATE_REL };    // -1 coordinates to designate it was never touched
//#if CONFIG_LV_FT6X36_COORDINATES_QUEUE
//QueueHandle_t ft6x36_touch_queue_handle;
//#endif

/**
  * @brief  Perform the I2C read of the TSC2007
  * @param  dev_addr: I2C TSC2007 Slave address.
  * @param  function: Converter Function Select value
  * @retval int16 12bit value of the measure returned by the TSC2007.  -1 if the read fails.
  *
  */

/* Original
  static esp_err_t tsc2007_i2c_read(uint8_t slave_addr, uint8_t function, uint8_t *data_buf) {
    return lvgl_i2c_read(CONFIG_LV_I2C_TOUCH_PORT, slave_addr, function, data_buf, 2);

    // TODO: Should the two bytes be reassembled here to a uint16_t value?
}
*/
uint16_t tsc2007_i2c_read(uint8_t slave_addr, uint8_t function) {

    uint8_t data_buf[2];    // TSC2007 returns 2 bytes.

    esp_err_t ret = lvgl_i2c_read(CONFIG_LV_I2C_TOUCH_PORT, slave_addr, function, data_buf, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error talking to touch IC: %s", esp_err_to_name(ret));
        return -1;
    }

    // Assemble the two bytes returned to a 12bit value.
    return ((uint16_t)data_buf[0] << 4) | (data_buf[1] >> 4); // 12 bits
}


/**
  * @brief  Build the command to send
  * @param  function: TSC2007 Converter Function Select
  * @param  pwr: A/D Converter Power and IRQ enable/disable
  * @param  res: resolution mode 12 bit or 8 bit
  * @retval uint8_t command byte to send.
  */
uint8_t tsc2007_command(uint8_t function, uint8_t pwr, uint8_t res){
    uint8_t cmd = function;
    cmd |= pwr;
    cmd |= res;
    return cmd;
}


/**
  * @brief  Initialize for TSC2007 communication via I2C
  * @param  dev_addr: Device address on communication Bus (I2C slave address of TSC2007).
  * @retval None
  */
void tsc2007_init(uint16_t dev_addr) {

    
    current_dev_addr = dev_addr;
    uint8_t data_buf[2];
    esp_err_t ret;
    ESP_LOGI(TAG, "Found touch panel controller");

    // Per datasheet, when the TSC2007 powers up the power down bits must be written to ensure it is placed in mode to acheive
    // lowest power use.
    if (-1 == tsc2007_i2c_read(dev_addr, tsc2007_command(TSC2007_FNC_MEASURE_TEMP0,
                                                             TSC2007_POWERDOWN_IRQON, 
                                                            TSC2007_ADC_12BIT)))
        ESP_LOGE(TAG, "Error reading from touch device: 0x%X",
                 dev_addr);    // Only show error the first time
    
    tsc2007_status.inited = true;
}

/**
  * @brief  Read touch screen to detect if it is touched
  * @param  drv:
  * @param  data: Store data here
  * @retval Always false
  */
bool tsc2007_touch_detected() {
    // TODO: Write this
    // AdaFruit example may help: https://learn.adafruit.com/adafruit-3-5-tft-featherwing/resistive-touch-screen-v2
    
    // check IRQ pin if we IRQ or IRQ and preessure
#if TSC2007_TOUCH_IRQ ||  TSC2007_TOUCH_IRQ_PRESS
    uint8_t irq = gpio_get_level(TSC2007_IRQ);

    if (irq != 0) {
        return false;
    }
#endif

    uint16_t z1,z2;

    // Read the touch pressures Z1 and Z2
    z1 = tsc2007_i2c_read(current_dev_addr, 
        tsc2007_command(TSC2007_FNC_MEASURE_Z1,TSC2007_ADON_IRQOFF, TSC2007_ADC_12BIT));

        // Read the Z2 value
    z2 = tsc2007_i2c_read(current_dev_addr, 
        tsc2007_command(TSC2007_FNC_MEASURE_Z2,TSC2007_ADON_IRQOFF, TSC2007_ADC_12BIT));

#if (CONFIG_LV_TSC2007_TOUCH_LOG)        
    ESP_LOGV(TAG, "Pressure values read: z1 = %d, z2 = %d", z1, z2);
#endif

    // Evaluate the values to determine if a press occured

    /*  Datasheet: For touich pressure, datasheet says to use this formula.  But I need to know the RX-plate resistance value which I don't.
        PTOUCH = Rx−plate x (x/4096) x (z2/z1 -1)

        AdaFruit example checks the z1 value to be less than 10.  It also checks x and y to be 0 for no touch.
        I could read all three values and run the same logic AdaFruit does.
        if (((p.x == 0) && (p.y == 0)) || (p.z < 10)) return; // no pressure, no touch

        XP2046 code uses this formula which seems to work well on my ESP32R32 board: int16_t z = z1 + 4096 - z2
    */

    // AdaFruit example checks the z1 value to be less than 10.  I'll do that too.
    if (z1 > TSC2007_TOUCH_THRESHOLD)
    {
        ESP_LOGD(TAG, "Touched pressure: z1 = %d", z1);
        return true;
    }
    return false;
}

/**
  * @brief  Adjust the coordinates as needed from KConfig
  * @param  x: pointer
  * @param  y: pointer
  *
  */
void tsc2007_corr(int16_t * x, int16_t * y)
{

#if TSC2007_XY_SWAP != 0
    // Swap values - configured in KConfig
	int16_t swap_tmp;
    swap_tmp = *x;
    *x = *y;
    *y = swap_tmp;
#endif

    if((*x) > TSC2007_X_MIN)(*x) -= TSC2007_X_MIN;
    else(*x) = 0;

    if((*y) > TSC2007_Y_MIN)(*y) -= TSC2007_Y_MIN;
    else(*y) = 0;

    (*x) = (uint32_t)((uint32_t)(*x) * LV_HOR_RES) /
           (TSC2007_X_MAX - TSC2007_X_MIN);

    (*y) = (uint32_t)((uint32_t)(*y) * LV_VER_RES) /
           (TSC2007_Y_MAX - TSC2007_Y_MIN);

#if TSC2007_X_INV != 0
    // Invert X - Configured in KConfig
    (*x) =  LV_HOR_RES - (*x);
#endif

#if TSC2007_Y_INV != 0
    // Invert Y - Configured in KConfig
    (*y) =  LV_VER_RES - (*y);
#endif


}

/**
  * @brief  Get the touch screen X and Y positions values. Ignores multi touch
  * @param  drv:
  * @param  data: Store data here
  * @retval Always false
  */
bool tsc2007_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    if (!tsc2007_status.inited) {
        ESP_LOGE(TAG, "Init before reading!");
        return 0x00;
    }
    
    static int16_t last_x = 0;
    static int16_t last_y = 0;

#if (CONFIG_LV_TSC2007_TOUCH_LOG)
    // values to capture for calibrating the touch screen
    static int16_t low_x = 32000;
    static int16_t low_y = 32000;
    static int16_t high_x = 0;
    static int16_t high_y = 0;
#endif

    bool valid = false;
    //uint8_t data_buf[2];        // TSC2007 returns 2 bytes.
    int16_t x = last_x;         // Start with last values read
    int16_t y = last_y;

    /*
        The TSC2007 has 4 measurement values X, Y, Z1, and Z2
        lvgl uses X and Y coordinates
        Z1 and Z2 are used for touch pressure.  This can be used to distinguish it was touched and if finger and stylus.

        A i2c read function call is sent and read for each coordinate
        Read X
        Read Y
    */

    // TODO: AdaFruit's Arduino library reads it twice for accuracy.  XP2046 averages with last read values.  Do I also want to do that?

    if (tsc2007_touch_detected()) {
        // Touch occured
        // Read the X value
        x = tsc2007_i2c_read(current_dev_addr, 
            tsc2007_command(TSC2007_FNC_MEASURE_X,TSC2007_ADON_IRQOFF, TSC2007_ADC_12BIT));

            // Read the Y value
        y = tsc2007_i2c_read(current_dev_addr, 
            tsc2007_command(TSC2007_FNC_MEASURE_Y,TSC2007_ADON_IRQOFF, TSC2007_ADC_12BIT));

#if (CONFIG_LV_TSC2007_TOUCH_LOG)
        if (x < low_x) low_x = x;
        if (y < low_y) low_y = y;
        if (x > high_x) high_x = x;
        if (y > high_y) high_y = y;
        ESP_LOGD(TAG, "Values read: x=%d, y=%d; Lowest read: x=%d, y=%d; Highest read: x=%d, y=%d", x, y, low_x, low_y, high_x, high_y);
#endif

        valid = true;
    } else {
        // Not touched
        valid = false;
        data->point.x = last_x;
        data->point.y = last_y;
        data->state = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;
        return false;
    }

    // Valid touch

    // Adjust coordinate values returned
    tsc2007_corr(&x, &y);

    last_x = x;
    last_y = y;
   
    data->point.x = last_x;
    data->point.y = last_y;
    data->state = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;
    ESP_LOGD(TAG, "Values adj : X=%u, Y=%u", data->point.x, data->point.y);

    return false;   // false means no more data to read
}
