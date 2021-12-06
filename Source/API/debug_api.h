#ifndef __DEBUG_API__H__
#define __DEBUG_API__H__

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "cmsis_os.h"
#include "stdio.h"
#include "string.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "uart_api.h"
#include "vcp_driver.h"
#include "vcp_api.h"
/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/
#define SUPER_DEBUG
#define API_DEBUG

#define CREATE_DEBUG_MODULE(modname)                                                        \
    struct debug_module debug_module_##modname = { .name = #modname, .registered = false }; \
    static struct debug_module *local_debug_module = &debug_module_##modname;

#define DECLARE_DEBUG_MODULE(modname)                  \
    extern struct debug_module debug_module_##modname; \
    static struct debug_module *local_debug_module = &debug_module_##modname;

#ifdef API_DEBUG
#define debug(...) Debug_API_Log(local_debug_module, __VA_ARGS__)
#else
#define debug(...) ((void)__VA_ARGS__)
#endif

#ifdef SUPER_DEBUG
#define super_debug(message) VCP_Driver_SendString(message)
#else
#define super_debug(message) ((void)message)
#endif

struct debug_module {
    const char *name;
    bool registered;
};
/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/
void Debug_API_Init (void);
void Debug_API_Log (struct debug_module *module, const char *format, ...);
eUartEnum_t Debug_API_ReturnUart();

#endif /* __DEBUG_API__H__ */
