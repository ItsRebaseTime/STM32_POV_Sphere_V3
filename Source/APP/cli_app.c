/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "cli_app.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define CLI_APP_MESSAGE_QUEUE_BUFFER_SIZE 70
#define CLI_APP_RESPONSE_BUFFER_SIZE 50

CREATE_DEBUG_MODULE(CLI_APP)
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const static osThreadAttr_t cli_app_task_attributes = {
    .name = "cli_app_task",
    .stack_size = CLI_APP_TASK_STACK_SIZE,
    .priority = (osPriority_t) CLI_APP_TASK_PRIORITY
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static osThreadId_t cli_app_task_handle;
static char received_message[CLI_APP_MESSAGE_QUEUE_BUFFER_SIZE];
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
static void CLI_APP_Task (void *argument);
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void CLI_APP_Task (void *argument) {
    while (true) {
        if (VCP_API_Receive(received_message, CLI_APP_MESSAGE_QUEUE_BUFFER_SIZE, osWaitForever)) {
            char *response_buffer = calloc(CLI_APP_RESPONSE_BUFFER_SIZE, sizeof(char));
            if (response_buffer == NULL) {
                debug("Failed to allocate memory in CLI_APP_Task");
            }
            else {
                sCmdLauncherArgs_t cmd_launcher_args = {
                    .response_buffer_size = CLI_APP_RESPONSE_BUFFER_SIZE,
                    .response_buffer = response_buffer,
                    .cmd_raw = received_message
                };
                CMD_Handler_Launcher(&cmd_launcher_args);

                debug(response_buffer);
                free(response_buffer);
            }
        }
        osThreadYield();
    }
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
void CLI_APP_Init () {
    cli_app_task_handle = osThreadNew(CLI_APP_Task, NULL, &cli_app_task_attributes);
    if (cli_app_task_handle == NULL) {
        debug("Failed to create thread in CLI_APP_Init");
        return;
    }
}
