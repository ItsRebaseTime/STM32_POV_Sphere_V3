/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "usb_driver.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define USB_DRIVER_BAUDRATE 921600
#define USB_DRIVER_RX_BUFFER_SIZE 100
#define USB_DRIVER_TX_BUFFER_SIZE 100
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
USBD_HandleTypeDef hUsbDeviceFS = {0};

uint8_t usb_driver_rx_buffer[USB_DRIVER_RX_BUFFER_SIZE] = {0};
uint8_t usb_driver_tx_buffer[USB_DRIVER_TX_BUFFER_SIZE] = {0};

static uint8_t lcBuffer[7];
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
static int8_t CDC_Init_FS (void);
static int8_t CDC_DeInit_FS (void);
static int8_t CDC_Control_FS (uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_Receive_FS (uint8_t *pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS (uint8_t *pbuf, uint32_t *Len, uint8_t epnum);
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static int8_t CDC_Init_FS (void) {
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, usb_driver_tx_buffer, 0);
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, usb_driver_rx_buffer);
    uint32_t baudrate = USB_DRIVER_BAUDRATE;
    lcBuffer[0] = (uint8_t) (baudrate);
    lcBuffer[1] = (uint8_t) (baudrate >> 8);
    lcBuffer[2] = (uint8_t) (baudrate >> 16);
    lcBuffer[3] = (uint8_t) (baudrate >> 24);
    lcBuffer[4] = 0;
    lcBuffer[5] = 0;
    lcBuffer[6] = 8;
    return (USBD_OK);
}

static int8_t CDC_DeInit_FS (void) {
    return (USBD_OK);
}

static int8_t CDC_Control_FS (uint8_t cmd, uint8_t *pbuf, uint16_t length) {
    switch (cmd) {
        case CDC_SEND_ENCAPSULATED_COMMAND:

            break;

        case CDC_GET_ENCAPSULATED_RESPONSE:

            break;

        case CDC_SET_COMM_FEATURE:

            break;

        case CDC_GET_COMM_FEATURE:

            break;

        case CDC_CLEAR_COMM_FEATURE:

            break;

        case CDC_SET_LINE_CODING:
            lcBuffer[0] = pbuf[0];
            lcBuffer[1] = pbuf[1];
            lcBuffer[2] = pbuf[2];
            lcBuffer[3] = pbuf[3];
            lcBuffer[4] = pbuf[4];
            lcBuffer[5] = pbuf[5];
            lcBuffer[6] = pbuf[6];
            break;

        case CDC_GET_LINE_CODING:
            pbuf[0] = lcBuffer[0];
            pbuf[1] = lcBuffer[1];
            pbuf[2] = lcBuffer[2];
            pbuf[3] = lcBuffer[3];
            pbuf[4] = lcBuffer[4];
            pbuf[5] = lcBuffer[5];
            pbuf[6] = lcBuffer[6];

            VCP_Driver_ResetBuffer();
            break;

        case CDC_SET_CONTROL_LINE_STATE:

            break;

        case CDC_SEND_BREAK:

            break;

        default:
            break;
    }
    return (USBD_OK);
}

static int8_t CDC_Receive_FS (uint8_t *Buf, uint32_t *Len) {
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
    uint8_t len = (uint8_t) *Len;
    for (uint32_t i = 0; i < len; i++) {
        VCP_Driver_Receive_Handler(Buf[i]);
    }
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return (USBD_OK);
}

static int8_t CDC_TransmitCplt_FS (uint8_t *Buf, uint32_t *Len, uint8_t epnum) {
    uint8_t result = USBD_OK;
    UNUSED(Buf);
    UNUSED(Len);
    UNUSED(epnum);
    return result;
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
void USB_Driver_Init (void) {
    if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK) {
        Error_Handler();
    }
    if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK) {
        Error_Handler();
    }
    if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK) {
        Error_Handler();
    }
    if (USBD_Start(&hUsbDeviceFS) != USBD_OK) {
        Error_Handler();
    }
}

uint8_t CDC_Transmit_FS (uint8_t *Buf, uint16_t Len) {
    uint8_t result = USBD_OK;
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*) hUsbDeviceFS.pClassData;
    if (hcdc->TxState != 0) {
        return USBD_BUSY;
    }
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
    result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
    return result;
}

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = {
    CDC_Init_FS,
    CDC_DeInit_FS,
    CDC_Control_FS,
    CDC_Receive_FS,
    CDC_TransmitCplt_FS
};
