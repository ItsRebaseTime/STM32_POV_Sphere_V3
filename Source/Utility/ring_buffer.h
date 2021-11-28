#ifndef __RING_BUFFER__H__
#define __RING_BUFFER__H__
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/
// @formatter:off
/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/
typedef struct rbuf_t {
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    bool full;
    uint8_t *buffer;
} rbuf_t;

typedef rbuf_t *rb_handle_t;
/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/
// @formatter:on
/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/
rb_handle_t RB_Init (uint16_t size);
void RB_Free (rb_handle_t cbuf);
void RB_Reset (rb_handle_t cbuf);
void RB_Push (rb_handle_t cbuf, uint8_t data);
bool RB_Pop (rb_handle_t cbuf, uint8_t *data);
bool RB_IsEmpty (rb_handle_t cbuf);
bool RB_IsFull (rb_handle_t cbuf);
uint32_t RB_GetSize (rb_handle_t cbuf);
uint32_t RB_GetFreeSpace (rb_handle_t cbuf);
#endif /* __RING_BUFFER__H__ */
