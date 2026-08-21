#include "app_tmc.h"

#include "tmc6460.h"

#include "FreeRTOS.h"
#include "task.h"


/*
 * Keep these non-static initially so that you can inspect them
 * easily in IAR Watch window.
 */
volatile uint32_t g_tmc6460_chip_id = 0U;

volatile tmc6460_status_t g_tmc6460_status =
    TMC6460_ERROR;


/* -------------------------------------------------------------------------- */

void APP_TMC_Task(void)
{
    uint32_t chipId;

    /*
     * Give TMC some startup time.
     */
    vTaskDelay(pdMS_TO_TICKS(100U));


    for (;;)
    {
//      TMC_SendData();
      
        chipId = 0U;

        g_tmc6460_status =
            TMC6460_ReadChipID(&chipId);


        if (g_tmc6460_status == TMC6460_OK)
        {
            g_tmc6460_chip_id = chipId;


            if (chipId == TMC6460_EXPECTED_CHIP_ID)
            {
                /*
                 * Communication OK.
                 *
                 * Expected:
                 *
                 * 0x36343630
                 */
            }
            else
            {
                g_tmc6460_status =
                    TMC6460_INVALID_CHIP_ID;
            }
        }
        else
        {
            g_tmc6460_chip_id = 0U;
        }

        /*
         * Read once per second during bring-up.
         */
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}