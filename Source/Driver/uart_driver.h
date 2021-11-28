#ifndef __UART_DRIVER__H__
#define __UART_DRIVER__H__

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "stm32f4xx_ll_bus.h"
//#include "stm32f4xx_ll_usart.h"
#include "stdbool.h"
#include "ring_buffer.h"
/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/
// @formatter:off
typedef enum eUartEnum_t {
    eUartDriverUart5 = 0,
    eUartDriverUart7,
    eUartDriverUart8,
    eUartDriverUart9,
    eUartDriverUsart1,
    eUartDriverUsart2,
    eUartDriverUsart3,
    eUartDriverUsart6,
    eUartDriverUartFirst = eUartDriverUart5,
    eUartDriverUartLast = eUartDriverUsart6,
} eUartEnum_t;

//typedef enum eBaudrateEnum_t {
//    eUartBaudrate115200 = 0,
//    eUartBaudrate230400,
//    eUartBaudrate460800,
//    eUartBaudrate576000,
//    eUartBaudrate921600,
//    eUartBaudrateFirst = eUartBaudrate115200,
//    eUartBaudrateLast = eUartBaudrate921600,
//} eBaudrateEnum_t;

typedef enum eUartErrorEnum_t {
    eUartFAIL,
    eUartOK,
} eUartErrorEnum_t;
// @formatter:on
/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/
eUartErrorEnum_t UART_Driver_Init (eUartEnum_t uart, uint32_t baudrate);
void UART_Driver_SendByte (eUartEnum_t uart, uint8_t byte);
eUartErrorEnum_t UART_Driver_SendBytes (eUartEnum_t uart, uint8_t *bytes, uint8_t len);
eUartErrorEnum_t UART_Driver_SendString (eUartEnum_t uart, char *string);
eUartErrorEnum_t UART_Driver_ReadByte (eUartEnum_t uart, uint8_t *byte);
uint16_t UART_Driver_ReadBytes (eUartEnum_t uart, uint8_t *bytes, uint16_t number_of_bytes);

#endif /*__UART_DRIVER__H__*/
