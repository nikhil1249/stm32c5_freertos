/**
  ******************************************************************************
  * @file           : mx_freertos_app.c
  * @brief          : FreeRTOS initialization
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the mx_freertos_license.md file
  * in the same directory as the generated code.
  * If no mx_freertos_license.md file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "mx_freertos_app.h"
#include "user_modifiable/tmc6460.h"
#include "user_modifiable/app_tmc.h"

/* Private define ------------------------------------------------------------*/
#define Task1_stack_size  128U
#define Task2_stack_size  128U
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*-------------------- Tasks definition --------------------------------------*/
/* Definitions for Task1 */
static TaskHandle_t Task1_Handle;
/* Definitions for Task2 */
static TaskHandle_t Task2_Handle;

/*-------------------- Mutexes definition -----------------------------------*/
/* Definitions for Mutex1 */
static SemaphoreHandle_t Mutex1_Handle;

/* Private functions prototype -----------------------------------------------*/
/* Tasks entry function ------------------------------------------------------*/
static void function1(void *pvParameters);
static void function2(void *pvParameters);

/**
  * @brief Initializes FreeRTOS kernel objects.
  * @param None
  * @retval int32_t Returns 0 on success, -1 on failure.
  */
int32_t app_synctasks_init (void)
{
  BaseType_t ret;

  /* Task1 creation-------------------------------------*/
  ret = xTaskCreate(function1, "Task1", Task1_stack_size,
                    (void*) NULL, 0, &Task1_Handle);

  if (ret != pdPASS)
  {
      return -1;
  }

  /* Task2 creation-------------------------------------*/
  ret = xTaskCreate(function2, "Task2", Task2_stack_size,
                    (void*) NULL, 0, &Task2_Handle);

  if (ret != pdPASS)
  {
      return -1;
  }

  /* Mutex1 creation-----------------------------------*/
  Mutex1_Handle = xSemaphoreCreateMutex();

  if (Mutex1_Handle == NULL)
  {
      return -1;
  }

  return 0;
}

/* Tasks entry function ------------------------------------------------------*/
/**
  * @brief Function implementing the Task1 thread.
  * @param pvParameters: A pointer to the parameters passed to the task.
  * @retval None
  */
static void function1(void *pvParameters)
{
  ( void ) pvParameters;
 //  static const uint8_t message[] = "Hello\r";

 //   hal_uart_handle_t *uart;
 //    hal_status_t status;

    (void)pvParameters;

//    uart = mx_usart2_uart_gethandle();

    for (;;)
    {
        /* Toggle onboard LED PA5 */
        HAL_GPIO_TogglePin(HAL_GPIOA, HAL_GPIO_PIN_5);

/*        status = HAL_UART_Transmit_IT(uart, message, sizeof(message) - 1U);
        
        if (status == HAL_OK)
        {
            // Transmission started successfully 
        }
        else if (status == HAL_BUSY)
        {
           //  Previous transmission is still active 
        }
        else
        {
           //  UART error 
        }

*/        
         /* Infinite loop executing Task1 functionality. */

        vTaskDelay(pdMS_TO_TICKS(100));
    }
 
}

/**
  * @brief Function implementing the Task2 thread.
  * @param pvParameters: A pointer to the parameters passed to the task.
  * @retval None
  */
static void function2(void *pvParameters)
{
  ( void ) pvParameters;
 
  for(;;)
  {
    /* Infinite loop executing Task2 functionality. */
    APP_TMC_Task();
    vTaskDelay(pdMS_TO_TICKS(1000U));
  }
}

