#ifndef __VCP_API__H__
#define __VCP_API__H__

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "debug_api.h"
#include "cmsis_os.h"
#include "uart_driver.h"
#include "stack_info.h"
/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/
// @formatter:off
/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/
// @formatter:on
/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/
bool VCP_API_Init ();
void VCP_API_SendString (char *string);
bool VCP_API_Receive (char *data, uint16_t buffer_size, uint32_t timeout);
#endif /* __VCP_API__H__ */
