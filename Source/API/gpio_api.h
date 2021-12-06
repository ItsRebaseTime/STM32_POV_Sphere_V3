#ifndef __GPIO_API__H__
#define __GPIO_API__H__
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"
#include <stdbool.h>
/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/
typedef enum eGpioApiPinEnum_t {
    eGpioApiPinFirst = 0,
    eGpioApiLedPin = eGpioApiPinFirst,
    eGpioApiTim3Ch1Pin,
    eGpioApiTim3Ch2Pin,
    eGpioApiTim1Ch3Pin,
    eGpioApiPinLast = eGpioApiLedPin,
} eGpioApiPinEnum_t;
/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/
void GPIO_API_Init (void);
void GPIO_API_SetPin (eGpioApiPinEnum_t gpio);
void GPIO_API_ResetPin (eGpioApiPinEnum_t gpio);
void GPIO_API_TogglePin (eGpioApiPinEnum_t gpio);
bool GPIO_API_IsPinSet (eGpioApiPinEnum_t gpio);
#endif /* __GPIO_API__H__ */
