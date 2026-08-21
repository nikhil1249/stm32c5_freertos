#include "uart_iface.h"
#include <stddef.h>


static hal_uart_handle_t *g_uart = NULL;

static volatile uint8_t g_tx_complete = 0U;
static volatile uint8_t g_rx_complete = 0U;
static volatile uint8_t g_uart_error  = 0U;


/* -------------------------------------------------------------------------- */

void UART_IF_Init(void)
{
    g_uart = mx_usart1_uart_gethandle();

    g_tx_complete = 0U;
    g_rx_complete = 0U;
    g_uart_error  = 0U;
}


/* -------------------------------------------------------------------------- */

static uart_if_status_t UART_IF_Wait(volatile uint8_t *flag,
                                     uint32_t timeout_ms)
{
    TickType_t start;
    TickType_t timeoutTicks;

    start = xTaskGetTickCount();

    timeoutTicks = pdMS_TO_TICKS(timeout_ms);

    if (timeoutTicks == 0U)
    {
        timeoutTicks = 1U;
    }

    while (*flag == 0U)
    {
        if (g_uart_error != 0U)
        {
            (void)HAL_UART_Abort(g_uart);

            return UART_IF_ERROR;
        }

        if ((xTaskGetTickCount() - start) >= timeoutTicks)
        {
            (void)HAL_UART_Abort(g_uart);

            return UART_IF_TIMEOUT;
        }

        /*
         * Do NOT busy-spin.
         * Give CPU to other FreeRTOS tasks.
         */
        vTaskDelay(pdMS_TO_TICKS(1U));
    }

    return UART_IF_OK;
}


/* -------------------------------------------------------------------------- */
/* UART WRITE                                                                 */
/* -------------------------------------------------------------------------- */

uart_if_status_t UART_IF_Write(const uint8_t *data,
                               uint32_t length,
                               uint32_t timeout_ms)
{
    hal_status_t status;

    if ((g_uart == NULL) ||
        (data == NULL) ||
        (length == 0U))
    {
        return UART_IF_INVALID_PARAM;
    }

    g_tx_complete = 0U;
    g_uart_error  = 0U;

    status = HAL_UART_Transmit_IT(g_uart,
                                  data,
                                  length);

    if (status == HAL_BUSY)
    {
        return UART_IF_BUSY;
    }

    if (status != HAL_OK)
    {
        return UART_IF_ERROR;
    }

    return UART_IF_Wait(&g_tx_complete,
                        timeout_ms);
}


/* -------------------------------------------------------------------------- */
/* UART READ                                                                  */
/* -------------------------------------------------------------------------- */

uart_if_status_t UART_IF_Read(uint8_t *data,
                              uint32_t length,
                              uint32_t timeout_ms)
{
    hal_status_t status;

    if ((g_uart == NULL) ||
        (data == NULL) ||
        (length == 0U))
    {
        return UART_IF_INVALID_PARAM;
    }

    g_rx_complete = 0U;
    g_uart_error  = 0U;

    status = HAL_UART_Receive_IT(g_uart,
                                 data,
                                 length);

    if (status == HAL_BUSY)
    {
        return UART_IF_BUSY;
    }

    if (status != HAL_OK)
    {
        return UART_IF_ERROR;
    }

    return UART_IF_Wait(&g_rx_complete,
                        timeout_ms);
}


/* -------------------------------------------------------------------------- */
/* UART WRITE + READ                                                          */
/* -------------------------------------------------------------------------- */

uart_if_status_t UART_IF_WriteRead(const uint8_t *tx_data,
                                   uint32_t tx_length,
                                   uint8_t *rx_data,
                                   uint32_t rx_length,
                                   uint32_t timeout_ms)
{
    hal_status_t status;
    uart_if_status_t result;

    if ((g_uart == NULL) ||
        (tx_data == NULL) ||
        (rx_data == NULL) ||
        (tx_length == 0U) ||
        (rx_length == 0U))
    {
        return UART_IF_INVALID_PARAM;
    }

    g_tx_complete = 0U;
    g_rx_complete = 0U;
    g_uart_error  = 0U;


    /*
     * IMPORTANT:
     *
     * Enable reception BEFORE transmitting.
     *
     * TMC6460 can respond immediately after the command.
     */
    status = HAL_UART_Receive_IT(g_uart,
                                 rx_data,
                                 rx_length);

    if (status == HAL_BUSY)
    {
        return UART_IF_BUSY;
    }

    if (status != HAL_OK)
    {
        return UART_IF_ERROR;
    }


    /*
     * Start TX.
     */
    status = HAL_UART_Transmit_IT(g_uart,
                                  tx_data,
                                  tx_length);

    if (status != HAL_OK)
    {
        (void)HAL_UART_Abort(g_uart);

        if (status == HAL_BUSY)
        {
            return UART_IF_BUSY;
        }

        return UART_IF_ERROR;
    }


    /*
     * Wait for TMC response.
     */
    result = UART_IF_Wait(&g_rx_complete,
                          timeout_ms);

    if (result != UART_IF_OK)
    {
        return result;
    }


    /*
     * Normally TX has already completed before the response
     * arrives, but explicitly verify it.
     */
    result = UART_IF_Wait(&g_tx_complete,
                          timeout_ms);

    return result;
}


/* ========================================================================== */
/* STM32 HAL UART CALLBACKS                                                   */
/* ========================================================================== */

void HAL_UART_TxCpltCallback(hal_uart_handle_t *huart)
{
    if (huart == g_uart)
    {
        g_tx_complete = 1U;
    }
}



void send_data(void)
{
  static char arry[] = "Hello\r";
  HAL_UART_Transmit_IT(g_uart, arry,sizeof(arry));
}  

/* -------------------------------------------------------------------------- */

void HAL_UART_RxCpltCallback(hal_uart_handle_t *huart,
                             uint32_t size_byte,
                             hal_uart_rx_event_types_t rx_event)
{
    (void)size_byte;
    (void)rx_event;

    if (huart == g_uart)
    {
        g_rx_complete = 1U;
    }
}


/* -------------------------------------------------------------------------- */

void HAL_UART_ErrorCallback(hal_uart_handle_t *huart)
{
    if (huart == g_uart)
    {
        g_uart_error = 1U;
    }
}