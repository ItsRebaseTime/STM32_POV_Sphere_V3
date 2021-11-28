/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "vcp_api.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define VCP_API_MESSAGE_QUEUE_SIZE 70
#define VCP_API_MESSAGE_QUEUE_COUNT 2
#define VCP_API_DEFAULT_DELIMITER '\n'
/**********************************************************************************************************************
 * Private types
 *********************************************************************************************************************/
// @formatter:off
typedef enum eStatesEnum_t {
    eVcpApiSetupState = 0,
    eVcpApiReceiveState,
    eVcpApiFlushState,
    eVcpApiStateFirst = eVcpApiSetupState,
    eVcpApiStateLast = eVcpApiFlushState,
} eStatesEnum_t;
// @formatter:on
typedef struct sVcpApiVar_t {
    osMessageQueueId_t msg_queue;
    osMutexId_t tx_mutex_id;
    eStatesEnum_t state;
    bool initialised;
    char *buffer;
    uint16_t index;
    char delimiter;
} sVcpApiVar_t;

typedef struct sVcpApiDesc_t {
    uint16_t msg_queue_buffer_size;
    uint8_t msg_queue_count;
} sVcpApiDesc_t;
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const static osThreadAttr_t vcp_api_task_attributes = {
    .name = "vcp_api_task",
    .stack_size = VCP_API_TASK_STACK_SIZE,
    .priority = (osPriority_t) VCP_API_TASK_PRIORITY, };

const static sVcpApiDesc_t static_vcp_api_lut = {
    .msg_queue_buffer_size = VCP_API_MESSAGE_QUEUE_SIZE, .msg_queue_count = VCP_API_MESSAGE_QUEUE_COUNT
};

const static osMutexAttr_t vcp_api_mutex_attr = {
    "vcp_api_mutex",
    osMutexRecursive | osMutexPrioInherit,
    NULL,
    0U
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static osThreadId_t vcp_api_task_handle = NULL;

static sVcpApiVar_t dynamic_vcp_api_lut = {
    .state = eVcpApiSetupState, .buffer = NULL, .initialised = false, .index = 0
};
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
static void VCP_API_Task (void *argument);
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void VCP_API_Task (void *argument) {
    while (true) {
        switch (dynamic_vcp_api_lut.state) {

            case eVcpApiSetupState: {
                dynamic_vcp_api_lut.buffer = calloc(static_vcp_api_lut.msg_queue_buffer_size, sizeof(uint8_t));
                if (dynamic_vcp_api_lut.buffer != NULL) {
                    dynamic_vcp_api_lut.index = 0;
                    dynamic_vcp_api_lut.state = eVcpApiReceiveState;
                }
                else {
                    if (dynamic_vcp_api_lut.initialised) {
                        super_debug("MEMORY ALLOCATION FAILURE in UART_API_Task\r\n");
                    }
                    break;
                }
            }

            case eVcpApiReceiveState: {
                uint8_t received_byte = 0;
                while (VCP_Driver_ReadByte(&received_byte) == eVcpOK) {
                    if (dynamic_vcp_api_lut.index >= static_vcp_api_lut.msg_queue_buffer_size - 1) {
                        free(dynamic_vcp_api_lut.buffer);
                        dynamic_vcp_api_lut.state = eVcpApiSetupState;
                        if (dynamic_vcp_api_lut.initialised) {
                            super_debug("BUFFER OVERFLOW in UART_API_Task\r\n");
                        }
                        break;
                    }
                    if (received_byte == dynamic_vcp_api_lut.delimiter) {
                        dynamic_vcp_api_lut.state = eVcpApiFlushState;
                    }
                    else {
                        dynamic_vcp_api_lut.buffer[dynamic_vcp_api_lut.index++] = received_byte;
                        break;
                    }
                }
                if (dynamic_vcp_api_lut.state != eVcpApiFlushState) {
                    break;
                }
            }

            case eVcpApiFlushState: {
                if (osMessageQueuePut(dynamic_vcp_api_lut.msg_queue, dynamic_vcp_api_lut.buffer, 0U, osWaitForever) == osOK) {
                    dynamic_vcp_api_lut.state = eVcpApiSetupState;
                    free(dynamic_vcp_api_lut.buffer);
                }
                else {
                    if (dynamic_vcp_api_lut.initialised) {
                        super_debug("FAILED TO PUT MESSAGE TO QUEUE in UART_API_Task\r\n");
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
bool VCP_API_Init () {
    if (VCP_Driver_Init() == eVcpOK) {
        dynamic_vcp_api_lut.index = 0;
        dynamic_vcp_api_lut.delimiter = VCP_API_DEFAULT_DELIMITER;
        dynamic_vcp_api_lut.tx_mutex_id = osMutexNew(&vcp_api_mutex_attr);
        if (dynamic_vcp_api_lut.tx_mutex_id == NULL) {
            return false;
        }
        dynamic_vcp_api_lut.msg_queue = osMessageQueueNew(static_vcp_api_lut.msg_queue_count, static_vcp_api_lut.msg_queue_buffer_size, NULL);
        if (dynamic_vcp_api_lut.msg_queue == NULL) {
            return false;
        }
        vcp_api_task_handle = osThreadNew(VCP_API_Task, NULL, &vcp_api_task_attributes);
        if (vcp_api_task_handle == NULL) {
            return false;
        }
        dynamic_vcp_api_lut.initialised = true;
    }
    else {
        return false;
    }
    return true;
}

void VCP_API_SendString (char *string) {
    if (string == NULL || !dynamic_vcp_api_lut.initialised) {
//        if (dynamic_vcp_api_lut.initialised) {
//            super_debug("ERROR in VCP_API_SendString\r\n");
//        }
        return;
    }
    osMutexAcquire(dynamic_vcp_api_lut.tx_mutex_id, osWaitForever);
    VCP_Driver_SendString(string);
    osMutexRelease(dynamic_vcp_api_lut.tx_mutex_id);
}

bool VCP_API_Receive (char *data, uint16_t buffer_size, uint32_t timeout) {
    if (data == NULL || buffer_size < static_vcp_api_lut.msg_queue_buffer_size || !dynamic_vcp_api_lut.initialised) {
//        if (dynamic_vcp_api_lut.initialised) {
//            super_debug("ERROR in VCP_API_Receive\r\n");
//        }
        return false;
    }
    return (osMessageQueueGet(dynamic_vcp_api_lut.msg_queue, data, NULL, timeout) == osOK) ? true : false;
}

bool VCP_API_IsInitialised () {
    return dynamic_vcp_api_lut.initialised;
}
