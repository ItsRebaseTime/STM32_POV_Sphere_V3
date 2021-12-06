/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "sms_cli_app.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define SMS_CLI_APP_RESPONSE_BUFFER_SIZE 160
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const static osThreadAttr_t sms_cli_app_task_attributes = {
    .name = "sms_cli_app_task",
    .stack_size = SMS_CLI_APP_TASK_STACK_SIZE,
    .priority = (osPriority_t) SMS_CLI_APP_TASK_PRIORITY
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static osThreadId_t sms_cli_app_task_handle;
static char sms_cli_app_received_message[UART_API_MESSAGE_QUEUE_SIZE];

CREATE_DEBUG_MODULE(SMS_CLI_APP)
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
static void SMS_CLI_APP_Task (void *argument);
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void SMS_CLI_APP_Task (void *argument) {
    while (true) {
        if (SMS_API_ReceiveSMS(sms_cli_app_received_message, UART_API_MESSAGE_QUEUE_SIZE, osWaitForever)) {
            char *response_buffer = calloc(SMS_CLI_APP_RESPONSE_BUFFER_SIZE, sizeof(char));
            if (response_buffer == NULL) {
                debug("Failed to allocate memory in SMS_CLI_APP_Task");
            }
            else {
                sMsgLauncherArgs_t msg_launcher_args = {
                    .response_buffer_size = SMS_CLI_APP_RESPONSE_BUFFER_SIZE,
                    .response_buffer = response_buffer,
                    .msg_raw = sms_cli_app_received_message,
                    .msg_lut = cli_msg_lut,
                    .msg_lut_elements = ARRAY_ELEMENT_COUNT(cli_msg_lut),
                    .delimiter = CLI_DELIMITER,
                    .filter_chars = CLI_FILTER_CHARS
                };
                MSG_HandlerLauncher(&msg_launcher_args);

                SMS_API_SendSMS(response_buffer);
                free(response_buffer);
            }
        }
        osThreadYield();
    }
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
void SMS_CLI_APP_Init () {
    sms_cli_app_task_handle = osThreadNew(SMS_CLI_APP_Task, NULL, &sms_cli_app_task_attributes);
}
