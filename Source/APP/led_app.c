/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "led_app.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
typedef struct sLedAppVar_t {
    uint8_t blinks;
    uint16_t interval;
    uint8_t id;
} sLedApiVar_t;
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const osThreadAttr_t led_app_task_attributes = {
    .name = "led_app_task",
    .stack_size = LED_APP_TASK_STACK_SIZE,
    .priority = (osPriority_t) LED_APP_TASK_PRIORITY
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
CREATE_DEBUG_MODULE(LED_APP)

static osThreadId_t led_app_task_handles[eLedAppLedLast + 1];

static sLedApiVar_t dynamic_led_app_lut[] = {
    [eLedAppBlueLed] = {.blinks = 255, .interval = 1000},
};
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
void LED_APP_LedBlinkTask (void *index) {
    uint8_t led = *(uint8_t*) index;
    while (true) {
        if (dynamic_led_app_lut[led].blinks > 0) {
            LED_API_SetLed(led);
            osDelay(dynamic_led_app_lut[led].interval / 2);
            LED_API_ResetLed(led);
            osDelay(dynamic_led_app_lut[led].interval / 2);
            if (dynamic_led_app_lut[led].blinks != eLedInfiniteBlinks) {
                dynamic_led_app_lut[led].blinks--;
            }
            debug("BLINK\r\n");
        }
    }
}

void LED_APP_BlinkLed (eLedAppLedEnum_t led, uint8_t blinks, uint16_t interval) {
    if (led > eLedAppLedLast || blinks == 0 || interval == 0) {
        return;
    }
    dynamic_led_app_lut[led].blinks = blinks;
    dynamic_led_app_lut[led].interval = interval;
}

void LED_APP_Init (void) {
    LED_API_Init();
    for (uint8_t i = 0; i <= eLedAppLedLast; i++) {
        dynamic_led_app_lut[i].id = i;
        led_app_task_handles[i] = osThreadNew(LED_APP_LedBlinkTask, (void*) &dynamic_led_app_lut[i].id, &led_app_task_attributes);
        if (led_app_task_handles[i] == NULL) {
            debug("THREAD CREATION FAILURE in LED_APP_Init");
        }
    }
}
