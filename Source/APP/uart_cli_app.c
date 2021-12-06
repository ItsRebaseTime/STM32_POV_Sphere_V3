/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "uart_cli_app.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define UART_CLI_APP_RESPONSE_BUFFER_SIZE 512

#define UART_CLI_APP_MESSAGE_DELIMITER ":"
#define UART_CLI_APP_FILTER_CHARS "\r\n"
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const static osThreadAttr_t uart_cli_app_task_attributes = {
    .name = "uart_cli_app_task",
    .stack_size = UART_CLI_APP_TASK_STACK_SIZE,
    .priority = (osPriority_t) UART_CLI_APP_TASK_PRIORITY
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static osThreadId_t uart_cli_app_task_handle;
static char uart_cli_app_received_message[UART_API_MESSAGE_QUEUE_SIZE];

CREATE_DEBUG_MODULE(UART_CLI)
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
static void UART_CLI_APP_Task (void *argument);
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void UART_CLI_APP_Task (void *argument) {
    while (true) {
        if (UART_API_Receive(Debug_API_ReturnUart(), uart_cli_app_received_message, osWaitForever)) {
            char *response_buffer = calloc(UART_CLI_APP_RESPONSE_BUFFER_SIZE, sizeof(char));
            if (response_buffer == NULL) {
                debug("Failed to allocate memory in UART_CLI_APP_Task");
            } else {
                sMsgLauncherArgs_t msg_launcher_args = {
                    .response_buffer_size = UART_CLI_APP_RESPONSE_BUFFER_SIZE,
                    .response_buffer = response_buffer,
                    .msg_raw = uart_cli_app_received_message,
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
void UART_CLI_APP_Init (void) {
    Debug_API_Init();

    uart_cli_app_task_handle = osThreadNew(UART_CLI_APP_Task, NULL, &uart_cli_app_task_attributes);
    if (uart_cli_app_task_handle == NULL) {
        return;
    }
}
