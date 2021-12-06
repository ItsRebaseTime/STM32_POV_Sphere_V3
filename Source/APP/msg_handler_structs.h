#ifndef __MSG_HANDLER_STRUCTS__H__
#define __MSG_HANDLER_STRUCTS__H__
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "modem_enums.h"
/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/
#define MSG_MAX_ARGUMENT_COUNT 5
/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/
typedef struct sMsgHandlerArgs_t {
    char *msg_args[MSG_MAX_ARGUMENT_COUNT];
    char *response_buffer;
    uint16_t response_buffer_size;
    bool print_response;
    bool start_task;
} sMsgHandlerArgs_t;

typedef struct sMsgDesc_t {
    char *msg_name;
    uint16_t param_count;
    void (*fun_ptr)(sMsgHandlerArgs_t*);
} sMsgDesc_t;

typedef struct sMsgLauncherArgs_t {
    char *msg_raw;
    char *response_buffer;
    uint16_t response_buffer_size;
    char* filter_chars;
    char* delimiter;
    const sMsgDesc_t* msg_lut;
    uint16_t msg_lut_elements;
    bool print_response;
    bool start_task;
    eModemStateEnum_t *state;
} sMsgLauncherArgs_t;
/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/
#endif /* __MSG_HANDLER_STRUCTS__H__ */
