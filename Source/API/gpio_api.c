/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "gpio_api.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
typedef struct sGpioDesc_t {
    GPIO_TypeDef *port;
    uint32_t pin;
    uint32_t mode;
    uint32_t speed;
    uint32_t output;
    uint32_t pull;
    uint32_t clock;
    uint32_t alternate;
} sGpioDesc_t;
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
static sGpioDesc_t static_gpio_lut[] = {
    [eGpioApiLedPin] = {.port = GPIOC, .pin = LL_GPIO_PIN_13, .mode = LL_GPIO_MODE_OUTPUT, .speed = LL_GPIO_SPEED_FREQ_LOW, .output = LL_GPIO_OUTPUT_PUSHPULL, .pull = LL_GPIO_PULL_NO, .clock = LL_AHB1_GRP1_PERIPH_GPIOC, .alternate = LL_GPIO_AF_0},
    [eGpioApiTim3Ch1Pin] = {.port = GPIOA, .pin = LL_GPIO_PIN_6, .mode = LL_GPIO_MODE_ALTERNATE, .speed = LL_GPIO_SPEED_FREQ_LOW, .output = LL_GPIO_OUTPUT_PUSHPULL, .pull = LL_GPIO_PULL_NO, .clock = LL_AHB1_GRP1_PERIPH_GPIOA, .alternate = LL_GPIO_AF_2},
    [eGpioApiTim3Ch2Pin] = {.port = GPIOA, .pin = LL_GPIO_PIN_7, .mode = LL_GPIO_MODE_ALTERNATE, .speed = LL_GPIO_SPEED_FREQ_LOW, .output = LL_GPIO_OUTPUT_PUSHPULL, .pull = LL_GPIO_PULL_NO, .clock = LL_AHB1_GRP1_PERIPH_GPIOA, .alternate = LL_GPIO_AF_2},
    [eGpioApiTim1Ch3Pin] = {.port = GPIOA, .pin = LL_GPIO_PIN_10, .mode = LL_GPIO_MODE_ALTERNATE, .speed = LL_GPIO_SPEED_FREQ_LOW, .output = LL_GPIO_OUTPUT_PUSHPULL, .pull = LL_GPIO_PULL_NO, .clock = LL_AHB1_GRP1_PERIPH_GPIOA, .alternate = LL_GPIO_AF_1},
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void GPIO_API_InitExtra (void) {
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE0);
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_0, LL_GPIO_PULL_NO);
    NVIC_SetPriority(EXTI0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI0_IRQn);
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
void GPIO_API_Init (void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    for (uint8_t i = eGpioApiPinFirst; i <= eGpioApiPinLast; i++) {
        LL_AHB1_GRP1_EnableClock(static_gpio_lut[i].clock);
        GPIO_InitStruct.Mode = static_gpio_lut[i].mode;
        GPIO_InitStruct.Speed = static_gpio_lut[i].speed;
        GPIO_InitStruct.OutputType = static_gpio_lut[i].output;
        GPIO_InitStruct.Pull = static_gpio_lut[i].pull;
        GPIO_InitStruct.Alternate = static_gpio_lut[i].alternate;
        GPIO_InitStruct.Pin = static_gpio_lut[i].pin;
        LL_GPIO_Init(static_gpio_lut[i].port, &GPIO_InitStruct);
    }

    GPIO_API_InitExtra();
}

void GPIO_API_SetPin (eGpioApiPinEnum_t gpio) {
    LL_GPIO_SetOutputPin(static_gpio_lut[gpio].port, static_gpio_lut[gpio].pin);
}

void GPIO_API_ResetPin (eGpioApiPinEnum_t gpio) {
    LL_GPIO_ResetOutputPin(static_gpio_lut[gpio].port, static_gpio_lut[gpio].pin);
}

void GPIO_API_TogglePin (eGpioApiPinEnum_t gpio) {
    LL_GPIO_TogglePin(static_gpio_lut[gpio].port, static_gpio_lut[gpio].pin);
}

bool GPIO_API_IsPinSet (eGpioApiPinEnum_t gpio) {
    return (LL_GPIO_IsOutputPinSet(static_gpio_lut[gpio].port, static_gpio_lut[gpio].pin) ? true : false);
}

void EXTI0_IRQHandler (void) {
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
        void Motor_Driver_Interrupt ();
    }
}
