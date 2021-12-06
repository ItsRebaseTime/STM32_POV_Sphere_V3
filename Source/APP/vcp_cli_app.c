/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "vcp_cli_app.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define VCP_CLI_APP_MESSAGE_QUEUE_BUFFER_SIZE 70
#define VCP_CLI_APP_RESPONSE_BUFFER_SIZE 1024

CREATE_DEBUG_MODULE(CLI_APP)
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const static osThreadAttr_t vcp_cli_app_task_attributes = {
    .name = "cli_app_task",
    .stack_size = VCP_CLI_APP_TASK_STACK_SIZE,
    .priority = (osPriority_t) VCP_CLI_APP_TASK_PRIORITY
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static osThreadId_t vcp_cli_app_task_handle;
static char vcp_cli_app_received_message[VCP_CLI_APP_RESPONSE_BUFFER_SIZE];
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
static void VCP_CLI_APP_Task (void *argument);
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void VCP_CLI_APP_Task (void *argument) {
    while (true) {
        if (VCP_API_Receive(vcp_cli_app_received_message, VCP_CLI_APP_MESSAGE_QUEUE_BUFFER_SIZE, osWaitForever)) {
            char *response_buffer = calloc(VCP_CLI_APP_RESPONSE_BUFFER_SIZE, sizeof(char));
            if (response_buffer == NULL) {
                debug("Failed to allocate memory in VCP_CLI_APP_Task");
            } else {
                sMsgLauncherArgs_t msg_launcher_args = {
                    .response_buffer_size = VCP_CLI_APP_RESPONSE_BUFFER_SIZE,
                    .response_buffer = response_buffer,
                    .msg_raw = vcp_cli_app_received_message,
                    .msg_lut = cli_msg_lut,
                    .msg_lut_elements = ARRAY_ELEMENT_COUNT(cli_msg_lut),
                    .delimiter = CLI_DELIMITER,
                    .filter_chars = CLI_FILTER_CHARS,
                    .print_response = true,
                    .start_task = false
                };

                MSG_HandlerLauncher(&msg_launcher_args);

                debug("%s\r\n", response_buffer);

                free(response_buffer);
            }
        }
        osThreadYield();
    }
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
void VCP_CLI_APP_Init () {
    VCP_API_Init();
    vcp_cli_app_task_handle = osThreadNew(VCP_CLI_APP_Task, NULL, &vcp_cli_app_task_attributes);
    if (vcp_cli_app_task_handle == NULL) {
        debug("Failed to create thread in VCP_CLI_APP_Init");
        return;
    }
}
