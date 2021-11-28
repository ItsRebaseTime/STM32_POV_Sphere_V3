/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "led_api.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define LED_API_LED_COUNT (eLedLast + 1)
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
typedef struct sLedApiVar_t {
    uint8_t blinks;
    uint16_t interval;
} sLedApiVar_t;

typedef struct sLedApiDesc_t {
    eGpioPinEnum_t pin;
    bool inverted;
} sLedApiDesc_t;
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const static sLedApiDesc_t static_led_api_lut[] = {
    [eLedApiBoardLed] = {.pin = eGpioDriverBoardLedPin, .inverted = true},
};

const osThreadAttr_t led_api_task_attributes = {
    .name = "led_api_task",
    .stack_size = LED_API_TASK_STACK_SIZE,
    .priority = (osPriority_t) LED_API_TASK_PRIORITY
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
CREATE_DEBUG_MODULE(LED_API)

static osThreadId_t led_api_task_handles[eLedLast + 1];

static sLedApiVar_t dynamic_led_api_lut[] = {
    [eLedFirst ... eLedLast] = {.blinks = 255, .interval = 1000},
};

uint8_t led_task_registration_index = 0;
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
void LED_API_LedBlinkTask (void *argument) {
    uint8_t led_index = led_task_registration_index++;
    while (true) {
        if (dynamic_led_api_lut[led_index].blinks > 0) {
            LED_API_SetLed(led_index);
            osDelay(dynamic_led_api_lut[led_index].interval / 2);
            LED_API_ResetLed(led_index);
            osDelay(dynamic_led_api_lut[led_index].interval / 2);
            if (dynamic_led_api_lut[led_index].blinks != eLedInfiniteBlinks) {
                dynamic_led_api_lut[led_index].blinks--;
            }
            debug("BLINK\r\n");
        }
    }
}

bool LED_API_BlinkLed (eLedEnum_t led, uint8_t blinks, uint16_t interval) {
    if (led > eLedLast || blinks == 0 || interval == 0) {
        return false;
    }
    dynamic_led_api_lut[led].blinks = blinks;
    dynamic_led_api_lut[led].interval = interval;
    return true;
}

bool LED_API_SetLed (eLedEnum_t led) {
    if (led > eLedLast) {
        return false;
    }
    static_led_api_lut[led].inverted ? GPIO_API_ResetPin(static_led_api_lut[led].pin) : GPIO_API_SetPin(static_led_api_lut[led].pin);
    return true;
}

bool LED_API_ResetLed (eLedEnum_t led) {
    if (led > eLedLast) {
        return false;
    }
    static_led_api_lut[led].inverted ? GPIO_API_SetPin(static_led_api_lut[led].pin) : GPIO_API_ResetPin(static_led_api_lut[led].pin);
    return true;
}

bool LED_API_ToggleLed (eLedEnum_t led) {
    if (led > eLedLast) {
        return false;
    }
    GPIO_API_TogglePin(static_led_api_lut[led].pin);
    return true;
}

void LED_API_Init (void) {
    LED_API_SetLed(eLedApiBoardLed);
    for (uint8_t i = eLedFirst; i <= eLedLast; i++) {
        led_api_task_handles[i] = osThreadNew(LED_API_LedBlinkTask, NULL, &led_api_task_attributes);
        if (led_api_task_handles[i] == NULL) {
            debug("THREAD CREATION FAILURE in LED_API_Init");
        }
    }
}
