/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "uart_driver.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
typedef struct sUartDesc_t {
    USART_TypeDef *uart;
    uint32_t clock;
    uint32_t baud;
    uint32_t width;
    uint32_t bits;
    uint32_t parity;
    uint32_t direction;
    uint32_t flow;
    uint32_t oversampling;
    uint32_t IRQn;
    uint16_t buffer_size;
} sUartDesc_t;

typedef struct sUartVar_t {
    rb_handle_t rb_handle;
} sUartVar_t;
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
// @formatter:off
static const sUartDesc_t static_uart_lut[] = {
    //[eUartDriverUart5] =  {.buffer_size = 1024, .uart = UART5, .clock = LL_APB1_GRP1_PERIPH_UART5,   .width = LL_USART_DATAWIDTH_8B, .bits = LL_USART_STOPBITS_1, .parity = LL_USART_PARITY_NONE, .direction = LL_USART_DIRECTION_TX_RX, .flow = LL_USART_HWCONTROL_NONE,    .oversampling = LL_USART_OVERSAMPLING_16, .IRQn = UART5_IRQn, },
    //[eUartDriverUart7] =  {.buffer_size = 1024, .uart = UART7, .clock = LL_APB1_GRP1_PERIPH_UART7,   .width = LL_USART_DATAWIDTH_8B, .bits = LL_USART_STOPBITS_1, .parity = LL_USART_PARITY_NONE, .direction = LL_USART_DIRECTION_TX_RX, .flow = LL_USART_HWCONTROL_NONE,    .oversampling = LL_USART_OVERSAMPLING_16, .IRQn = UART7_IRQn, },
    //[eUartDriverUart8] =  {.buffer_size = 1024, .uart = UART8, .clock = LL_APB1_GRP1_PERIPH_UART8,   .width = LL_USART_DATAWIDTH_8B, .bits = LL_USART_STOPBITS_1, .parity = LL_USART_PARITY_NONE, .direction = LL_USART_DIRECTION_TX_RX, .flow = LL_USART_HWCONTROL_NONE,    .oversampling = LL_USART_OVERSAMPLING_16, .IRQn = UART8_IRQn, },
    //[eUartDriverUart9] =  {.buffer_size = 1024, .uart = UART9, .clock = LL_APB2_GRP1_PERIPH_UART9,   .width = LL_USART_DATAWIDTH_8B, .bits = LL_USART_STOPBITS_1, .parity = LL_USART_PARITY_NONE, .direction = LL_USART_DIRECTION_TX_RX, .flow = LL_USART_HWCONTROL_NONE,    .oversampling = LL_USART_OVERSAMPLING_16, .IRQn = UART9_IRQn, },
    [eUartDriverUsart1] = {.buffer_size = 1024, .uart = USART1, .clock = LL_APB2_GRP1_PERIPH_USART1, .width = LL_USART_DATAWIDTH_8B, .bits = LL_USART_STOPBITS_1, .parity = LL_USART_PARITY_NONE, .direction = LL_USART_DIRECTION_TX_RX, .flow = LL_USART_HWCONTROL_NONE, .oversampling = LL_USART_OVERSAMPLING_16, .IRQn = USART1_IRQn, },
    [eUartDriverUsart2] = {.buffer_size = 1024, .uart = USART2, .clock = LL_APB1_GRP1_PERIPH_USART2, .width = LL_USART_DATAWIDTH_8B, .bits = LL_USART_STOPBITS_1, .parity = LL_USART_PARITY_NONE, .direction = LL_USART_DIRECTION_TX_RX, .flow = LL_USART_HWCONTROL_NONE, .oversampling = LL_USART_OVERSAMPLING_16, .IRQn = USART2_IRQn, },
    //[eUartDriverUsart3] = {.buffer_size = 1024, .uart = USART3, .clock = LL_APB1_GRP1_PERIPH_USART3, .width = LL_USART_DATAWIDTH_8B, .bits = LL_USART_STOPBITS_1, .parity = LL_USART_PARITY_NONE, .direction = LL_USART_DIRECTION_TX_RX, .flow = LL_USART_HWCONTROL_RTS_CTS, .oversampling = LL_USART_OVERSAMPLING_16, .IRQn = USART3_IRQn, },
    //[eUartDriverUsart6] = {.buffer_size = 1024, .uart = USART6, .clock = LL_APB2_GRP1_PERIPH_USART6, .width = LL_USART_DATAWIDTH_8B, .bits = LL_USART_STOPBITS_1, .parity = LL_USART_PARITY_NONE, .direction = LL_USART_DIRECTION_TX_RX, .flow = LL_USART_HWCONTROL_NONE,    .oversampling = LL_USART_OVERSAMPLING_16, .IRQn = USART6_IRQn, },
};
// @formatter:on
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static sUartVar_t dynamic_uart_lut[] = {
    [eUartDriverUartFirst ... eUartDriverUartLast] = {.rb_handle = NULL}
};
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
void USART1_IRQHandler (void);
void USART2_IRQHandler (void);
//void USART3_IRQHandler (void);
//void UART5_IRQHandler (void);
//void USART6_IRQHandler (void);
//void UART7_IRQHandler (void);
//void UART8_IRQHandler (void);
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void UART_Driver_UARTx_IRQ_Handler (eUartEnum_t uart) {
    if (uart <= eUartDriverUartLast) {
        if (LL_USART_IsActiveFlag_RXNE(static_uart_lut[uart].uart)) {
            uint8_t byte_received = LL_USART_ReceiveData8(static_uart_lut[uart].uart);
            RB_Push(dynamic_uart_lut[uart].rb_handle, byte_received);
        }
    }
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
eUartErrorEnum_t UART_Driver_Init (eUartEnum_t uart, uint32_t baudrate) {
    if (uart > eUartDriverUartLast || baudrate == 0) {
        return eUartFAIL;
    }
    LL_USART_InitTypeDef USART_InitStruct = {0};
    if (static_uart_lut[uart].clock == LL_APB2_GRP1_PERIPH_USART1) {
        LL_APB2_GRP1_EnableClock(static_uart_lut[uart].clock);
    }
    if (static_uart_lut[uart].clock == LL_APB1_GRP1_PERIPH_USART2) {
        LL_APB1_GRP1_EnableClock(static_uart_lut[uart].clock);
    }
    USART_InitStruct.BaudRate = baudrate;
    USART_InitStruct.DataWidth = static_uart_lut[uart].width;
    USART_InitStruct.StopBits = static_uart_lut[uart].bits;
    USART_InitStruct.Parity = static_uart_lut[uart].parity;
    USART_InitStruct.TransferDirection = static_uart_lut[uart].direction;
    USART_InitStruct.HardwareFlowControl = static_uart_lut[uart].flow;
    USART_InitStruct.OverSampling = static_uart_lut[uart].oversampling;
    LL_USART_Init(static_uart_lut[uart].uart, &USART_InitStruct);
    LL_USART_ConfigAsyncMode(static_uart_lut[uart].uart);
    LL_USART_Enable(static_uart_lut[uart].uart);
    LL_USART_EnableIT_RXNE(static_uart_lut[uart].uart);
    NVIC_SetPriority(static_uart_lut[uart].IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(static_uart_lut[uart].IRQn);
    dynamic_uart_lut[uart].rb_handle = RB_Init(static_uart_lut[uart].buffer_size);
    if (dynamic_uart_lut[uart].rb_handle == NULL) {
        return eUartFAIL;
    }
    return eUartOK;
}

void UART_Driver_SendByte (eUartEnum_t uart, uint8_t byte) {
    while (!LL_USART_IsActiveFlag_TXE(static_uart_lut[uart].uart)) {
    };
    LL_USART_TransmitData8(static_uart_lut[uart].uart, byte);
}

eUartErrorEnum_t UART_Driver_SendBytes (eUartEnum_t uart, uint8_t *bytes, uint8_t len) {
    if (bytes == NULL || len == 0 || uart > eUartDriverUartLast) {
        return eUartFAIL;
    }
    uint16_t index = 0;
    while (index < len) {
        UART_Driver_SendByte(uart, bytes[index++]);
    }
    return eUartOK;
}

eUartErrorEnum_t UART_Driver_SendString (eUartEnum_t uart, char *string) {
    if (string == NULL || uart > eUartDriverUartLast) {
        return eUartFAIL;
    }
    while (*string) {
        UART_Driver_SendByte(uart, (uint8_t) *string);
        string++;
    }
    return eUartOK;
}

eUartErrorEnum_t UART_Driver_ReadByte (eUartEnum_t uart, uint8_t *byte) {
    NVIC_DisableIRQ(static_uart_lut[uart].IRQn);
    eUartErrorEnum_t read_status = eUartFAIL;
    if (RB_Pop(dynamic_uart_lut[uart].rb_handle, byte)) {
        read_status = eUartOK;
    }
    NVIC_EnableIRQ(static_uart_lut[uart].IRQn);
    return read_status;
}

uint16_t UART_Driver_ReadBytes (eUartEnum_t uart, uint8_t *bytes, uint16_t number_of_bytes) {
    uint16_t index = 0;
    if (number_of_bytes <= 0 || bytes == NULL || uart > eUartDriverUartLast) {
        return 0;
    }
    while (index < number_of_bytes) {
        if (UART_Driver_ReadByte(uart, &bytes[index++]) == eUartFAIL) {
            break;
        }
    }
    return index;
}

//void UART5_IRQHandler (void) {
//    UART_Driver_UARTx_IRQ_Handler(eUartDriverUart5);
//}
//
//void UART7_IRQHandler (void) {
//    UART_Driver_UARTx_IRQ_Handler(eUartDriverUart7);
//}
//
//void UART8_IRQHandler (void) {
//    UART_Driver_UARTx_IRQ_Handler(eUartDriverUart8);
//}
//
//void UART9_IRQHandler (void) {
//    UART_Driver_UARTx_IRQ_Handler(eUartDriverUart9);
//}

void USART1_IRQHandler (void) {
    UART_Driver_UARTx_IRQ_Handler(eUartDriverUsart1);
}

void USART2_IRQHandler (void) {
    UART_Driver_UARTx_IRQ_Handler(eUartDriverUsart2);
}

//void USART3_IRQHandler (void) {
//    UART_Driver_UARTx_IRQ_Handler(eUartDriverUsart3);
//}
//
//void USART6_IRQHandler (void) {
//    UART_Driver_UARTx_IRQ_Handler(eUartDriverUsart6);
//}
