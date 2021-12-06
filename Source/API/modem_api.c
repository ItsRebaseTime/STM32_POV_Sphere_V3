/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "modem_api.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define MODEM_API_UART eUartDriverUsart2
#define MODEM_API_BAUDRATE 115200

#define MODEM_API_MSG_QUEUE_COUNT 3
#define MODEM_API_MSG_QUEUE_BUFFER_SIZE 1024
#define MODEM_API_RESPONSE_BUFFER_SIZE 1024

#define MODEM_API_MESSAGE_DELIMITER ":"
#define MODEM_API_UART_DELIMITER "\n"
#define MODEM_API_FILTER_CHARS "\r\n "

#define MODEM_API_EVENT_FLAG 0x00000001U
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const static osThreadAttr_t modem_api_task_attributes = {
    .name = "modem_api_task",
    .stack_size = MODEM_API_TASK_STACK_SIZE,
    .priority = (osPriority_t) MODEM_API_TASK_PRIORITY
};

const static sMsgDesc_t modem_api_msg_lut[] = {
    [eModemMsgEIND] =   {.msg_name = "+EIND",   .param_count = 1, .fun_ptr = &MSG_EIND},
    [eModemMsgEUSIM] =  {.msg_name = "+EUSIM",  .param_count = 1, .fun_ptr = &MSG_EUSIM},
    [eModemMsgOK] =     {.msg_name = "OK",      .param_count = 0, .fun_ptr = &MSG_OK},
    [eModemMsgCOPS] =   {.msg_name = "+COPS",   .param_count = 1, .fun_ptr = &MSG_COPS},
    [eModemMsgIPR] =    {.msg_name = "+IPR",    .param_count = 1, .fun_ptr = &MSG_IPR},
};

//const static osMutexAttr_t modem_api_mutex_attr = {
//    "uart_api_mutex",
//    osMutexRecursive | osMutexPrioInherit,
//    NULL,
//    0U
//};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
CREATE_DEBUG_MODULE(MODEM)
static osThreadId_t modem_api_task_handle = NULL;
static char modem_api_received_message[UART_API_MESSAGE_QUEUE_SIZE] = {0};
static osMessageQueueId_t modem_api_msg_queue = NULL;
eModemStateEnum_t modem_api_state = eModemStateSetup;
//static osMutexId_t modem_api_mutex_id = NULL;

static bool modem_api_locked = false;



osEventFlagsId_t modem_api_event_flag_id = NULL;
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
static void Modem_API_Task (void *argument);
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void Modem_API_Task (void *argument) {
    while (true) {
        if (UART_API_Receive(MODEM_API_UART, modem_api_received_message, osWaitForever)) {
            char *response_buffer = calloc(MODEM_API_RESPONSE_BUFFER_SIZE, sizeof(char));
            if (response_buffer == NULL) {
                debug("Failed to allocate memory in Modem_API_Task");
            } else {
                sMsgLauncherArgs_t msg_launcher_args = {
                    .response_buffer_size = MODEM_API_RESPONSE_BUFFER_SIZE,
                    .response_buffer = response_buffer,
                    .msg_raw = modem_api_received_message,
                    .msg_lut = modem_api_msg_lut,
                    .msg_lut_elements = ARRAY_ELEMENT_COUNT(modem_api_msg_lut),
                    .delimiter = MODEM_API_MESSAGE_DELIMITER,
                    .filter_chars = MODEM_API_FILTER_CHARS,
                    .print_response = true,
                    .state = &modem_api_state
                };

                MSG_HandlerLauncher(&msg_launcher_args);

                if (msg_launcher_args.print_response) {
                    debug("%s\r\n", response_buffer);
                }

                //osMessageQueuePut(mq_id, msg_ptr, msg_prio, timeout)

                free(response_buffer);
            }
        }
        osThreadYield();
    }
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
void Modem_API_SendAndWaitAnyResponse (char *string, uint32_t timeout) {
    if (string == NULL) {
        return;
    }
    Modem_API_Send(string);
}

char* Modem_API_SendAndGetResponse (char *string, uint32_t timeout) {
    if (string == NULL) {
        return NULL;
    }
    Modem_API_Send(string);

    return NULL;
}

bool Modem_API_SendAndWaitSpecificResponse (char *string, const char *response, uint32_t timeout) {
    if (string == NULL || response == NULL) {
        return false;
    }
    return true;
}

void Modem_API_Init (void) {
    UART_API_Init(MODEM_API_UART, MODEM_API_BAUDRATE, MODEM_API_UART_DELIMITER);

//    modem_api_event_flag_id = osEventFlagsNew(NULL);
//    if (modem_api_event_flag_id == NULL) {
//        debug("Failed to create event flag in Modem_API_Init");
//    }

//    modem_api_mutex_id = osMutexNew(&modem_api_mutex_attr);
//    if (modem_api_mutex_id == NULL) {
//        debug("Failed to create mutex in Modem_API_Init\r\n");
//        return;
//    }

    modem_api_msg_queue = osMessageQueueNew(MODEM_API_MSG_QUEUE_COUNT, MODEM_API_MSG_QUEUE_BUFFER_SIZE, NULL);
    if (modem_api_msg_queue == NULL) {
        debug("Failed to create message queue in MODEM_API_Init\r\n");
        return;
    }

    modem_api_task_handle = osThreadNew(Modem_API_Task, NULL, &modem_api_task_attributes);
    if (modem_api_task_handle == NULL) {
        debug("Failed to create thread in Modem_API_Init");
        return;
    }

    GPIO_API_SetPin(eGpioDriverGnssOnPin);
    GPIO_API_SetPin(eGpioDriverModemPowerOffPin);
    GPIO_API_SetPin(eGpioDriverModemOnPin);
}

bool Modem_API_Send (char *string) {
    if (string == NULL) {
        return false;
    }
    UART_API_SendString(MODEM_API_UART, string);
    return true;
}

void Modem_API_Reset (void) {
    GPIO_API_ResetPin(eGpioDriverGnssOnPin);
    GPIO_API_ResetPin(eGpioDriverModemPowerOffPin);
    GPIO_API_ResetPin(eGpioDriverModemOnPin);

    osDelay(1000);

    GPIO_API_SetPin(eGpioDriverGnssOnPin);
    GPIO_API_SetPin(eGpioDriverModemPowerOffPin);
    GPIO_API_SetPin(eGpioDriverModemOnPin);
}

bool Modem_API_Lock (void) {
    if (modem_api_locked) {
        return false;
    }
    return true;
}

void Modem_API_Unlock (void) {
    modem_api_locked = false;
}
