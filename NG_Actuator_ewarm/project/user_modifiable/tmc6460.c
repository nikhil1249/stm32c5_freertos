#include "tmc6460.h"
#include "uart_iface.h"

#include <stddef.h>


#define TMC6460_MAX_REGISTER_ADDRESS     0x03FFU

#define TMC6460_UART_TIMEOUT_MS          20U

#define TMC6460_READ_TX_SIZE             2U
#define TMC6460_READ_RX_SIZE             6U

#define TMC6460_WRITE_TX_SIZE            6U
#define TMC6460_WRITE_RX_SIZE            5U


/*
 * UART command byte.
 *
 * CRC disabled.
 *
 * Base read byte = 0x42.
 *
 * Address bits [9:8] are inserted into bits [5:4].
 *
 * WnR bit = bit 3.
 */
#define TMC6460_UART_BASE_HEADER         0x42U
#define TMC6460_UART_WRITE_BIT           0x08U


/* -------------------------------------------------------------------------- */

static uint8_t TMC6460_GetHeader(uint16_t address,
                                 uint8_t write)
{
    uint8_t header;

    header =
        (uint8_t)(TMC6460_UART_BASE_HEADER |
                  ((address >> 4U) & 0x30U));

    if (write != 0U)
    {
        header |= TMC6460_UART_WRITE_BIT;
    }

    return header;
}


/* -------------------------------------------------------------------------- */

static tmc6460_status_t TMC6460_ConvertStatus(uart_if_status_t status)
{
    switch (status)
    {
        case UART_IF_OK:
            return TMC6460_OK;

        case UART_IF_BUSY:
            return TMC6460_BUSY;

        case UART_IF_TIMEOUT:
            return TMC6460_TIMEOUT;

        case UART_IF_INVALID_PARAM:
            return TMC6460_INVALID_PARAM;

        default:
            return TMC6460_ERROR;
    }
}


/* -------------------------------------------------------------------------- */
/* REGISTER READ                                                              */
/* -------------------------------------------------------------------------- */

tmc6460_status_t TMC6460_ReadRegister(uint16_t address,
                                      uint32_t *value)
{
    uint8_t tx[TMC6460_READ_TX_SIZE];
    uint8_t rx[TMC6460_READ_RX_SIZE];

    uart_if_status_t uartStatus;

    if (value == NULL)
    {
        return TMC6460_INVALID_PARAM;
    }

    if (address > TMC6460_MAX_REGISTER_ADDRESS)
    {
        return TMC6460_INVALID_PARAM;
    }


    /*
     * Construct request.
     */
    tx[0] = TMC6460_GetHeader(address, 0U);
    tx[1] = (uint8_t)(address & 0xFFU);


    /*
     * Send request and receive 6-byte TMC reply.
     */
    uartStatus =
        UART_IF_WriteRead(tx,
                          sizeof(tx),
                          rx,
                          sizeof(rx),
                          TMC6460_UART_TIMEOUT_MS);

    if (uartStatus != UART_IF_OK)
    {
        return TMC6460_ConvertStatus(uartStatus);
    }


    /*
     * Response byte 0 and 1 must repeat request.
     */
    if ((rx[0] != tx[0]) ||
        (rx[1] != tx[1]))
    {
        return TMC6460_INVALID_RESPONSE;
    }


    /*
     * Bytes 2..5 contain 32-bit register data.
     */
    *value =
        ((uint32_t)rx[2] << 24U) |
        ((uint32_t)rx[3] << 16U) |
        ((uint32_t)rx[4] << 8U)  |
        ((uint32_t)rx[5]);

    return TMC6460_OK;
}


/* -------------------------------------------------------------------------- */
/* REGISTER WRITE                                                             */
/* -------------------------------------------------------------------------- */

tmc6460_status_t TMC6460_WriteRegister(uint16_t address,
                                       uint32_t value)
{
    uint8_t tx[TMC6460_WRITE_TX_SIZE];
    uint8_t rx[TMC6460_WRITE_RX_SIZE];

    uint32_t responseValue;

    uart_if_status_t uartStatus;


    if (address > TMC6460_MAX_REGISTER_ADDRESS)
    {
        return TMC6460_INVALID_PARAM;
    }


    tx[0] = TMC6460_GetHeader(address, 1U);

    tx[1] = (uint8_t)(address & 0xFFU);

    tx[2] = (uint8_t)((value >> 24U) & 0xFFU);
    tx[3] = (uint8_t)((value >> 16U) & 0xFFU);
    tx[4] = (uint8_t)((value >> 8U)  & 0xFFU);
    tx[5] = (uint8_t)(value & 0xFFU);


    uartStatus =
        UART_IF_WriteRead(tx,
                          sizeof(tx),
                          rx,
                          sizeof(rx),
                          TMC6460_UART_TIMEOUT_MS);

    if (uartStatus != UART_IF_OK)
    {
        return TMC6460_ConvertStatus(uartStatus);
    }


    /*
     * Normal write response:
     *
     * byte 0 = status/write count
     * bytes 1..4 = read-after-write register value.
     */
    responseValue =
        ((uint32_t)rx[1] << 24U) |
        ((uint32_t)rx[2] << 16U) |
        ((uint32_t)rx[3] << 8U) |
        ((uint32_t)rx[4]);


    if (responseValue != value)
    {
        return TMC6460_INVALID_RESPONSE;
    }


    return TMC6460_OK;
}


/* -------------------------------------------------------------------------- */
/* FIELD READ                                                                 */
/* -------------------------------------------------------------------------- */

tmc6460_status_t TMC6460_ReadField(uint16_t address,
                                   uint32_t mask,
                                   uint8_t shift,
                                   uint32_t *value)
{
    uint32_t registerValue;

    tmc6460_status_t status;


    if (value == NULL)
    {
        return TMC6460_INVALID_PARAM;
    }


    status =
        TMC6460_ReadRegister(address,
                             &registerValue);

    if (status != TMC6460_OK)
    {
        return status;
    }


    *value = (registerValue & mask) >> shift;

    return TMC6460_OK;
}


/* -------------------------------------------------------------------------- */
/* FIELD WRITE                                                                */
/* -------------------------------------------------------------------------- */

tmc6460_status_t TMC6460_WriteField(uint16_t address,
                                    uint32_t mask,
                                    uint8_t shift,
                                    uint32_t value)
{
    uint32_t registerValue;

    tmc6460_status_t status;


    /*
     * Read
     */
    status =
        TMC6460_ReadRegister(address,
                             &registerValue);

    if (status != TMC6460_OK)
    {
        return status;
    }


    /*
     * Modify
     */
    registerValue &= ~mask;

    registerValue |=
        ((value << shift) & mask);


    /*
     * Write
     */
    return TMC6460_WriteRegister(address,
                                 registerValue);
}


/* -------------------------------------------------------------------------- */
/* CHIP ID                                                                    */
/* -------------------------------------------------------------------------- */

tmc6460_status_t TMC6460_ReadChipID(uint32_t *chip_id)
{
    return TMC6460_ReadRegister(TMC6460_CHIP_ID_REG,
                                chip_id);
}

/* -------------------------------------------------------------------------- */
/* COMMUNICATION CHECK                                                        */
/* -------------------------------------------------------------------------- */

tmc6460_status_t TMC6460_CheckCommunication(void)
{
    uint32_t chipId;

    tmc6460_status_t status;


    status = TMC6460_ReadChipID(&chipId);

    if (status != TMC6460_OK)
    {
        return status;
    }


    if (chipId != TMC6460_EXPECTED_CHIP_ID)
    {
        return TMC6460_INVALID_CHIP_ID;
    }


    return TMC6460_OK;
}

void TMC_SendData(void)
{
  send_data();
}