/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "uart_api.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define UART_API_MESSAGE_QUEUE_SIZE 100
#define UART_API_MESSAGE_QUEUE_COUNT 3
/**********************************************************************************************************************
 * Private types
 *********************************************************************************************************************/
// @formatter:off
typedef enum eStatesEnum_t {
    eUartApiSetupState = 0,
    eUartApiReceiveState,
    eUartApiFlushState,
    eUartApiStateFirst = eUartApiSetupState,
    eUartApiStateLast = eUartApiFlushState,
} eStatesEnum_t;
// @formatter:on
typedef struct sUartApiVar_t {
    osMessageQueueId_t msg_queue;
    osMutexId_t tx_mutex_id;
    eStatesEnum_t state;
    uint32_t baudrate;
    eUartEnum_t uart;
    bool initialised;
    char *buffer;
    uint16_t index;
    char delimiter;
} sUartApiVar_t;

typedef struct sUartApiDesc_t {
    uint16_t msg_queue_buffer_size;
    uint8_t msg_queue_count;
} sUartApiDesc_t;
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const static osThreadAttr_t uart_api_task_attributes = {
    .name = "uart_api_task",
    .stack_size = UART_API_TASK_STACK_SIZE,
    .priority = (osPriority_t) UART_API_TASK_PRIORITY, };

const static sUartApiDesc_t static_uart_api_lut[eUartDriverUartLast + 1] = {
    [eUartDriverUartFirst ... eUartDriverUartLast] = {.msg_queue_buffer_size = UART_API_MESSAGE_QUEUE_SIZE, .msg_queue_count = UART_API_MESSAGE_QUEUE_COUNT}
};

const static osMutexAttr_t uart_api_mutex_attr = {
    "uart_api_mutex",
    osMutexRecursive | osMutexPrioInherit,
    NULL,
    0U
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static osThreadId_t uart_api_task_handle;

static bool uart_api_task_started = false;

static sUartApiVar_t dynamic_uart_api_lut[eUartDriverUartLast + 1] = {
    [eUartDriverUartFirst ... eUartDriverUartLast] = {.state = eUartApiSetupState, .buffer = NULL, .initialised = false, .index = 0}
};
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
static void UART_API_Task (void *argument);
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void UART_API_Task (void *argument) {
    while (true) {
        for (uint8_t uart = eUartDriverUartFirst; uart <= eUartDriverUartLast; uart++) {
            if (!dynamic_uart_api_lut[uart].initialised) {
                continue;
            }
            switch (dynamic_uart_api_lut[uart].state) {

                case eUartApiSetupState: {
                    dynamic_uart_api_lut[uart].buffer = calloc(static_uart_api_lut[uart].msg_queue_buffer_size, sizeof(uint8_t));
                    if (dynamic_uart_api_lut[uart].buffer != NULL) {
                        dynamic_uart_api_lut[uart].index = 0;
                        dynamic_uart_api_lut[uart].state = eUartApiReceiveState;
                    }
                    else {
                        if (Debug_API_IsInitialised()) {
                            super_debug("[SD] MEMORY ALLOCATION FAILURE in UART_API_Task\r\n");
                        }
                        break;
                    }
                }

                case eUartApiReceiveState: {
                    uint8_t received_byte = 0;
                    while (UART_Driver_ReadByte(dynamic_uart_api_lut[uart].uart, &received_byte) == eUartOK) {
                        if (dynamic_uart_api_lut[uart].index >= static_uart_api_lut[uart].msg_queue_buffer_size - 1) {
                            free(dynamic_uart_api_lut[uart].buffer);
                            dynamic_uart_api_lut[uart].state = eUartApiSetupState;
                            if (Debug_API_IsInitialised()) {
                                super_debug("[SD] BUFFER OVERFLOW in UART_API_Task\r\n");
                            }
                            break;
                        }
                        if (received_byte == dynamic_uart_api_lut[uart].delimiter) {
                            dynamic_uart_api_lut[uart].state = eUartApiFlushState;
                        }
                        else {
                            dynamic_uart_api_lut[uart].buffer[dynamic_uart_api_lut[uart].index++] = received_byte;
                            break;
                        }
                    }
                    if (dynamic_uart_api_lut[uart].state != eUartApiFlushState) {
                        break;
                    }
                }

                case eUartApiFlushState: {
                    if (osMessageQueuePut(dynamic_uart_api_lut[uart].msg_queue, dynamic_uart_api_lut[uart].buffer, 0U, osWaitForever) == osOK) {
                        dynamic_uart_api_lut[uart].state = eUartApiSetupState;
                        free(dynamic_uart_api_lut[uart].buffer);
                    }
                    else {
                        if (Debug_API_IsInitialised()) {
                            super_debug("[SD] FAILED TO PUT MESSAGE TO QUEUE in UART_API_Task\r\n");
                        }
                    }
                }
            }
        }
        osThreadYield();
    }
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
void UART_API_Init (eUartEnum_t uart, uint32_t baudrate, char delimiter) {
    if (uart > eUartDriverUartLast || baudrate == 0) {
        if (Debug_API_IsInitialised()) {
            super_debug("[SD] ERROR in UART_API_Init\r\n");
        }
        return;
    }
    if (UART_Driver_Init(uart, baudrate) == eUartOK) {
        dynamic_uart_api_lut[uart].index = 0;
        dynamic_uart_api_lut[uart].uart = uart;
        dynamic_uart_api_lut[uart].baudrate = baudrate;
        dynamic_uart_api_lut[uart].delimiter = delimiter;
        dynamic_uart_api_lut[uart].tx_mutex_id = osMutexNew(&uart_api_mutex_attr);
        if (dynamic_uart_api_lut[uart].tx_mutex_id == NULL) {
            if (Debug_API_IsInitialised()) {
                super_debug("[SD] TX MUTEX CREATION FAILURE in UART_API_Init\r\n");
            }
            return;
        }
        dynamic_uart_api_lut[uart].msg_queue = osMessageQueueNew(static_uart_api_lut[uart].msg_queue_count, static_uart_api_lut[uart].msg_queue_buffer_size, NULL);
        if (dynamic_uart_api_lut[uart].msg_queue == NULL) {
            if (Debug_API_IsInitialised()) {
                super_debug("[SD] MESSAGE QUEUE CREATION FAILURE in UART_API_Init\r\n");
            }
            return;
        }
        dynamic_uart_api_lut[uart].initialised = true;
        if (!uart_api_task_started) {
            uart_api_task_handle = osThreadNew(UART_API_Task, NULL, &uart_api_task_attributes);
            if (uart_api_task_handle == NULL) {
                if (Debug_API_IsInitialised()) {
                    super_debug("[SD] THREAD CREATION FAILURE in UART_API_Init\r\n");
                }
                return;
            }
            uart_api_task_started = true;
        }
    }
    else {
        if (Debug_API_IsInitialised()) {
            super_debug("[SD] UART INIT FAILURE in UART_API_Init\r\n");
        }
        return;
    }
}

void UART_API_SendString (eUartEnum_t uart, char *string) {
    if (string == NULL || uart > eUartDriverUartLast || !dynamic_uart_api_lut[uart].initialised) {
        if (Debug_API_IsInitialised()) {
            super_debug("[SD] ERROR in UART_API_SendString\r\n");
        }
        return;
    }
    osMutexAcquire(dynamic_uart_api_lut[uart].tx_mutex_id, osWaitForever);
    UART_Driver_SendString(uart, string);
    osMutexRelease(dynamic_uart_api_lut[uart].tx_mutex_id);
}

bool UART_API_Receive (eUartEnum_t uart, char *data, uint16_t buffer_size, uint32_t timeout) {
    if (data == NULL || uart > eUartDriverUartLast || buffer_size < static_uart_api_lut[uart].msg_queue_buffer_size || !dynamic_uart_api_lut[uart].initialised) {
        if (Debug_API_IsInitialised()) {
            super_debug("[SD] ERROR in UART_API_Receive\r\n");
        }
        return false;
    }
    return (osMessageQueueGet(dynamic_uart_api_lut[uart].msg_queue, data, NULL, timeout) == osOK) ? true : false;
}
