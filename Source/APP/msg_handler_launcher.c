/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "msg_handler_launcher.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
static const osMutexAttr_t msg_handler_mutex_attr = {
    "msg_handler_mutex",
    osMutexRecursive | osMutexPrioInherit,
    NULL,
    0U
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static osMutexId_t msg_handler_mutex_id = NULL;
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
void MSG_HandlerLauncher (sMsgLauncherArgs_t *launcher_args) {
    osMutexAcquire(msg_handler_mutex_id, osWaitForever);
    sMsgHandlerArgs_t handler_args = {0};
    handler_args.response_buffer = launcher_args->response_buffer;
    handler_args.response_buffer_size = launcher_args->response_buffer_size;
    handler_args.print_response = true;

    uint8_t token_index = 0;
    uint16_t msg_id = 0;
    bool msg_recognized = false;
    char *token = NULL;

    remchar(launcher_args->msg_raw, launcher_args->filter_chars, strlen(launcher_args->msg_raw));

    char *msg_copy = calloc(strlen(launcher_args->msg_raw), sizeof(char));
    strcpy(msg_copy, launcher_args->msg_raw);

    if (strlen(msg_copy) == 0) {
        launcher_args->print_response = false;
    } else {
        token = strtok(msg_copy, launcher_args->delimiter);
        for (uint16_t msg = 0; msg < launcher_args->msg_lut_elements; msg++) {
            if (strlcmp(token, launcher_args->msg_lut[msg].msg_name) == 0) {
                msg_id = msg;
                if (launcher_args->msg_lut[msg].param_count > 0) {
                    while (token != NULL) {
                        token = strtok(NULL, launcher_args->delimiter);
                        if (token != NULL) {
                            handler_args.msg_args[token_index++] = token;
                            if (token_index >= MSG_MAX_ARGUMENT_COUNT) {
                                break;
                            }
                        }
                    }
                }

                if (token_index == launcher_args->msg_lut[msg].param_count) {
                    msg_recognized = true;
                }
                break;
            }
        }

        if (msg_recognized) {
            launcher_args->msg_lut[msg_id].fun_ptr(&handler_args);
            launcher_args->print_response = handler_args.print_response;
            launcher_args->start_task = handler_args.start_task;
        } else {
            snprintf(launcher_args->response_buffer, launcher_args->response_buffer_size, "Got: %s\r\n", launcher_args->msg_raw);
        }
    }
    free(msg_copy);
    osMutexRelease(msg_handler_mutex_id);
}

void MSG_HandlerLouncher_Init (void) {
    msg_handler_mutex_id = osMutexNew(&msg_handler_mutex_attr);
}
