#ifndef __USB_DRIVER__H__
#define __USB_DRIVER__H__
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "usbd_def.h"
#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/
extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;
/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/
void USB_Driver_Init (void);
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
#endif /* __USB_DRIVER__H__ */