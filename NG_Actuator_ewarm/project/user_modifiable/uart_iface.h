#ifndef UART_IF_H
#define UART_IF_H

#include "main.h"
#include <stdint.h>

typedef enum
{
    UART_IF_OK = 0,
    UART_IF_ERROR,
    UART_IF_BUSY,
    UART_IF_TIMEOUT,
    UART_IF_INVALID_PARAM

} uart_if_status_t;


/* Connect wrapper to CubeMX UART handle */
void UART_IF_Init(void);

/* Interrupt based TX */
uart_if_status_t UART_IF_Write(const uint8_t *data,
                               uint32_t length,
                               uint32_t timeout_ms);


/* Interrupt based RX */
uart_if_status_t UART_IF_Read(uint8_t *data,
                              uint32_t length,
                              uint32_t timeout_ms);


/*
 * Start RX first and then TX.
 *
 * Very useful for devices such as TMC6460 where
 * a command is immediately followed by a response.
 */
uart_if_status_t UART_IF_WriteRead(const uint8_t *tx_data,
                                   uint32_t tx_length,
                                   uint8_t *rx_data,
                                   uint32_t rx_length,
                                   uint32_t timeout_ms);
 
 void send_data(void);

#endif