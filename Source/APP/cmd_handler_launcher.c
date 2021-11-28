/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "cmd_handler_launcher.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define CMD_NAME_LENGHT 1
#define DEFAULT_DELIM ":"
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const static sCliAppCmdDesc_t static_cli_app_cmd_lut[] = {
    {.cmd_name = "BLINK", .param_count = 3, .fun_ptr = &CMD_LedBlink},
    {.cmd_name = "TOGGLE", .param_count = 1, .fun_ptr = &CMD_LedToggle}
};

const char bad_chars[] = "\n\r";
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

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
bool CMD_Handler_Launcher (sCmdLauncherArgs_t *launcher_args) {
    sCmdHandlerArgs_t handler_args = {0};
    handler_args.response_buffer = launcher_args->response_buffer;
    handler_args.response_buffer_size = launcher_args->response_buffer_size;
    uint8_t token_index = 0;
    uint16_t cmd_id = 0;
    bool cmd_recognized = false;
    char *token = NULL;
    remchar(launcher_args->cmd_raw, bad_chars, strlen(launcher_args->cmd_raw));

    token = strtok(launcher_args->cmd_raw, DEFAULT_DELIM);
    for (uint16_t cmd = 0; cmd < ARRAY_ELEMENT_COUNT(static_cli_app_cmd_lut); cmd++) {
        if (strlcmp(token, static_cli_app_cmd_lut[cmd].cmd_name) == 0) {
            cmd_id = cmd;

            while (token != NULL) {
                token = strtok(NULL, DEFAULT_DELIM);
                if (token != NULL) {
                    handler_args.cmd_args[token_index++] = token;
                    if (token_index >= CMD_MAX_ARGUMENT_COUNT) {
                        break;
                    }
                }
            }

            if (token_index == static_cli_app_cmd_lut[cmd].param_count) {
                cmd_recognized = true;
            }
            break;
        }
    }

    if (cmd_recognized) {
        static_cli_app_cmd_lut[cmd_id].fun_ptr(handler_args);
    }
    else {
        snprintf(launcher_args->response_buffer, launcher_args->response_buffer_size, "CMD recognition failed\r\n");
    }
    return cmd_recognized;
}
