/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "ws2812_api.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define MOTOR_DRIVER_DEFAULT_SPEED 50
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
sRgbColor_t picture_data[WS2812_API_PICTURE_WIDTH][WS2812_API_PICTURE_HEIGHT] = {0};
sRgbColor_t frame_data[WS2812_API_FRAME_WIDTH][WS2812_API_FRAME_HEIGHT] = {0};

const osThreadAttr_t ws2812_api_task_attributes = {
    .name = "ws2812_api_task",
    .stack_size = WS2812_API_TASK_STACK_SIZE,
    .priority = (osPriority_t) WS2812_API_TASK_PRIORITY
};

sRgbColor_t black = {
    .R = 0,
    .G = 0,
    .B = 0
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
CREATE_DEBUG_MODULE(WS2812_API)

static osThreadId_t ws2812_api_task_handle;

static uint32_t ws2812_api_frame_position = 0;
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
void WS2812_API_LedUpdateTask (void *argument);
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
void WS2812_API_LedUpdateTask (void *argument) {
    while (true) {
        if ((Motor_Driver_GetReferenceTime() + (Motor_Driver_GetInterval() * ws2812_api_frame_position)) <= Timer_Driver_GetCurrentTime()) {
            WS2812_Driver_Send();
            ws2812_api_frame_position++;
            if (ws2812_api_frame_position >= WS2812_API_FRAME_WIDTH) {
                ws2812_api_frame_position = 0;
            }
            WS2812_Driver_GenerateData(frame_data[ws2812_api_frame_position]);
        }
        osThreadYield();
    }
}

void WS2812_API_SetPixel (uint16_t x, uint16_t y, sRgbColor_t color) {
    if (x >= WS2812_API_FRAME_WIDTH || y >= WS2812_API_FRAME_HEIGHT) {
        return;
    }
    frame_data[x][y] = color;
}

void WS2812_API_FillFrame (sRgbColor_t color) {
    for (uint16_t x = 0; x < WS2812_API_FRAME_WIDTH; x++) {
        for (uint16_t y = 0; y < WS2812_API_FRAME_HEIGHT; y++) {
            WS2812_API_SetPixel(x, y, color);
        }
    }
}

void WS8212_API_DrawPicture (uint16_t x, uint16_t y, sRgbColor_t picture[WS2812_API_PICTURE_WIDTH][WS2812_API_PICTURE_HEIGHT]) {
    if (x >= WS2812_API_FRAME_WIDTH || y >= WS2812_API_FRAME_HEIGHT) {
        return;
    }
    for (uint16_t i = 0; i < WS2812_API_PICTURE_WIDTH; i++) {
        uint16_t x_offset = x + i;
        while (x_offset >= WS2812_API_FRAME_WIDTH) {
            x_offset -= WS2812_API_FRAME_WIDTH;
        }
        for (uint16_t j = 0; j < WS2812_API_PICTURE_HEIGHT; j++) {
            uint16_t y_offset = y + j;
            while (y_offset >= WS2812_API_FRAME_HEIGHT) {
                y_offset -= WS2812_API_FRAME_HEIGHT;
            }
            WS2812_API_SetPixel(x_offset, y_offset, picture_data[i][j]);
        }
    }
}

void WS2812_API_ResetFrame (void) {
    WS2812_API_FillFrame(black);
}

void WS2812_API_Init (void) {
    WS2812_Driver_Init(WS2812_API_LED_COUNT);
    Timer_Driver_Init_TIM2();
    Motor_Driver_Init(MOTOR_DRIVER_DEFAULT_SPEED);
    Motor_Driver_Start();

    WS2812_API_ResetFrame();
    WS2812_Driver_Send();

    ws2812_api_task_handle = osThreadNew(WS2812_API_LedUpdateTask, NULL, &ws2812_api_task_attributes);
    if (ws2812_api_task_handle == NULL) {
        debug("Failed to create thread in WS2812_API_Init\r\n");
    }
}
