/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "vcp_driver.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
typedef struct sVcpDesc_t {
    uint16_t buffer_size;
} sVcpDesc_t;

typedef struct sVcpVar_t {
    rb_handle_t rb_handle;
} sVcpVar_t;
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
static const sVcpDesc_t static_vcp_driver_lut = {
    .buffer_size = 7000
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static sVcpVar_t dynamic_vcp_driver_lut = {
    .rb_handle = NULL
};
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
void VCP_Driver_Receive_Handler (uint8_t byte_received) {
    RB_Push(dynamic_vcp_driver_lut.rb_handle, byte_received);
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
eVcpErrorEnum_t VCP_Driver_Init (void) {
    dynamic_vcp_driver_lut.rb_handle = RB_Init(static_vcp_driver_lut.buffer_size);
    if (dynamic_vcp_driver_lut.rb_handle == NULL) {
        return eVcpFAIL;
    }
    return eVcpOK;
}

void VCP_Driver_SendByte (uint8_t byte) {
    CDC_Transmit_FS(&byte, 1);
}

eVcpErrorEnum_t VCP_Driver_SendBytes (uint8_t *bytes, uint8_t len) {
    if (bytes == NULL || len == 0) {
        return eVcpFAIL;
    }
    return (CDC_Transmit_FS(bytes, len) == USBD_OK) ? eVcpOK : eVcpFAIL;
}

eVcpErrorEnum_t VCP_Driver_SendString (char *string) {
    if (string == NULL) {
        return eVcpFAIL;
    }
    return (CDC_Transmit_FS((uint8_t*) string, strlen(string)) == USBD_OK) ? eVcpOK : eVcpFAIL;
}

eVcpErrorEnum_t VCP_Driver_ReadByte (uint8_t *byte) {
    eVcpErrorEnum_t read_status = eVcpFAIL;
    if (RB_Pop(dynamic_vcp_driver_lut.rb_handle, byte)) {
        read_status = eVcpOK;
    }
    return read_status;
}

uint16_t VCP_Driver_ReadBytes (uint8_t *bytes, uint16_t number_of_bytes) {
    uint16_t index = 0;
    if (number_of_bytes <= 0 || bytes == NULL) {
        return 0;
    }
    while (index < number_of_bytes) {
        if (VCP_Driver_ReadByte(&bytes[index++]) == eVcpFAIL) {
            break;
        }
    }
    return index;
}

void VCP_Driver_ResetBuffer () {
    RB_Reset(dynamic_vcp_driver_lut.rb_handle);
}
