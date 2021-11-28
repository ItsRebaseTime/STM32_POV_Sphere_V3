/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "debug_api.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define DEBUG_MSG_SIZE 80
#define TEMP_STRING_SIZE 20
//#define DEBUG_MODULE_AMOUNT 5
//#define DEBUG_API_PORT VCP
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const osMutexAttr_t debug_api_mutex_attr = {
    "debug_api_mutex",
    osMutexRecursive | osMutexPrioInherit,
    NULL,
    0U
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
osMutexId_t debug_api_mutex_id;

bool initialised = false;
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void Debug_API_SendString (char *string) {
    if (string == NULL) {
        super_debug("[SD]\tNULL POINTER in Debug_API_Send\r\n");
        return;
    }
    osMutexAcquire(debug_api_mutex_id, osWaitForever);
    VCP_API_SendString(string);
    osMutexRelease(debug_api_mutex_id);
}

static void Debug_API_Print (const char *tag, const char *format, va_list args) {
    char buffer[DEBUG_MSG_SIZE] = {0};
    uint16_t offset = 0;
    offset += snprintf(buffer, DEBUG_MSG_SIZE, "[%s]\tDEBUG:\t", tag);
    if (format) {
        offset += vsnprintf(&buffer[offset], sizeof(buffer) - offset, format, args);
    }
    Debug_API_SendString(buffer);
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
void Debug_API_Init (void) {
    debug_api_mutex_id = osMutexNew(&debug_api_mutex_attr);
    if (debug_api_mutex_id == NULL) {
        super_debug("[SD]\tMUTEX CREATION FAILURE in Debug_API_Init\r\n");
    }
    if (VCP_API_Init()) {
        initialised = true;
    }
}

void Debug_API_Log (struct debug_module *module, const char *format, ...) {
    va_list args;
    va_start(args, format);
    Debug_API_Print(module->name, format, args);
    va_end(args);
}

bool Debug_API_IsInitialised() {
    return initialised;
}
