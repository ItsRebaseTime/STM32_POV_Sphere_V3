#ifndef __WS2812_Driver__H__
#define __WS2812_Driver__H__

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "debug_api.h"
#include "stack_info.h"
#include "tim.h"
/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/
#define PWM_LOW_VALUE 40
#define PWM_HIGH_VALUE 80
#define PWM_STOP_VALUE 0

#define STOP_DATA 50

#define COLORS 3
#define DATA_PER_PIXEL 24


#define PICTURE_DATA_SIZE (PICTURE_HEIGHT * DATA_PER_PIXEL * COLORS)

#define PWM_DATA_SIZE ((WS2812_Driver_LED_COUNT * DATA_PER_PIXEL) + STOP_BYTES)

#define STOP_BYTES_START_INDEX (PWM_DATA_SIZE - STOP_BYTES - 1)
#define STOP_BYTES_STOP_INDEX (PWM_DATA_SIZE - 1)
/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/
typedef struct sRgbColor_t {
    uint8_t R;
    uint8_t G;
    uint8_t B;
} sRgbColor_t;
/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/
void WS2812_Driver_Send(void);
void WS2812_Driver_Init (uint16_t led_count);
void WS2812_Driver_GenerateData (sRgbColor_t *frame_data);

#endif /* __WS2812_Driver__H__ */
