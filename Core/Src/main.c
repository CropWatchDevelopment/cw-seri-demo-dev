/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "sensirion/sensirion.h"
#include "sensirion/sht4x_i2c.h"
#include "sensirion/sensirion_common.h"
#include "ssd1306.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static volatile bool heater_trigger_requested = false;
static volatile bool heater_active = false;
static uint32_t heater_active_until_ms = 0;
static volatile uint32_t heater_next_allowed_ms = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
static int run_heater_cycle(void);
static bool tick_elapsed(uint32_t now, uint32_t target);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  char sensor_serial_1[9] = {0};
  char sensor_serial_2[9] = {0};

  scan_i2c_bus();

  if (has_sensor_1) {
    sensirion_get_serial_string(SHT43_I2C_ADDR_44,
                                sensor_serial_1,
                                sizeof(sensor_serial_1));
  }

  if (has_sensor_2) {
    sensirion_get_serial_string(SHT40_I2C_ADDR_46,
                                sensor_serial_2,
                                sizeof(sensor_serial_2));
  }

  int i2c_success = sensor_init_and_read();
  (void)i2c_success;
  (void)sensor_serial_1;
  (void)sensor_serial_2;

  ssd1306_init(&hi2c1);
  if (ssd1306_is_initialized()) {
    ssd1306_clear();
    ssd1306_draw_string_scaled(8, 16, "CROPWATCH", 2U);
    ssd1306_update();
    HAL_Delay(5000);
    ssd1306_clear();

    char serial_display[32] = {0};
    if (has_sensor_1) {
      snprintf(serial_display, sizeof(serial_display), "S1 %s", sensor_serial_1);
      uint8_t line_width = ssd1306_measure_text_width(serial_display, 1U);
      uint8_t line_x = (line_width < SSD1306_WIDTH) ? (uint8_t)((SSD1306_WIDTH - line_width) / 2U) : 0U;
      ssd1306_draw_string_scaled(line_x, 16, serial_display, 1U);
    } else {
      ssd1306_draw_string_scaled(12, 16, "S1 MISSING", 1U);
    }

    if (has_sensor_2) {
      snprintf(serial_display, sizeof(serial_display), "S2 %s", sensor_serial_2);
      uint8_t line_width = ssd1306_measure_text_width(serial_display, 1U);
      uint8_t line_x = (line_width < SSD1306_WIDTH) ? (uint8_t)((SSD1306_WIDTH - line_width) / 2U) : 0U;
      ssd1306_draw_string_scaled(line_x, 32, serial_display, 1U);
    } else {
      ssd1306_draw_string_scaled(12, 32, "S2 MISSING", 1U);
    }

    ssd1306_update();
    HAL_Delay(10000);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    scan_i2c_bus();
    uint32_t now = HAL_GetTick();

    if (heater_trigger_requested) {
      heater_trigger_requested = false;
      heater_next_allowed_ms = now + 60000U;
      if (has_sensor_1 || has_sensor_2) {
        heater_active = true;
        heater_active_until_ms = now + 20000U;
      }
    }

    if (heater_active && tick_elapsed(now, heater_active_until_ms)) {
      heater_active = false;
    }

    int read_status = 0;
    if (heater_active) {
      read_status = run_heater_cycle();
      if (read_status != 0) {
        heater_active = false;
      }
    } else {
      read_status = sensor_init_and_read();
    }

    char temp_line[20] = {0};
    char hum_line[20] = {0};
    bool show_humidity = false;

    if (!has_sensor_1) {
      snprintf(temp_line, sizeof(temp_line), "NO SENSOR");
    } else if (!heater_active && read_status == 4) {
      snprintf(temp_line, sizeof(temp_line), "TEMP DIFF");
    } else if (read_status != 0) {
      snprintf(temp_line, sizeof(temp_line), "I2C ERR %d", (int)i2c_error_code);
    } else {
      int16_t temp_centi = sht4x_temp_centi_from_ticks(temp_ticks_1);
      int32_t abs_centi = (temp_centi < 0) ? -(int32_t)temp_centi : (int32_t)temp_centi;
      int32_t whole = abs_centi / 100;
      int32_t frac = abs_centi % 100;

      const char* sign = (temp_centi < 0) ? "-" : "";
      snprintf(temp_line, sizeof(temp_line), "TEMP %s%ld.%02ldC", sign, (long)whole, (long)frac);

      uint16_t hum_centi = calculated_hum_1;
      uint32_t hum_whole = hum_centi / 100U;
      uint32_t hum_frac = hum_centi % 100U;
      snprintf(hum_line, sizeof(hum_line), "HUM %lu.%02lu%%RH", (unsigned long)hum_whole, (unsigned long)hum_frac);
      show_humidity = true;
    }

    if (ssd1306_is_initialized()) {
      ssd1306_clear();

      if (temp_line[0] != '\0') {
        uint8_t temp_width = ssd1306_measure_text_width(temp_line, 2U);
        uint8_t temp_x = (temp_width < SSD1306_WIDTH) ? (uint8_t)((SSD1306_WIDTH - temp_width) / 2U) : 0U;
        ssd1306_draw_string_scaled(temp_x, 6, temp_line, 2U);
      }

      if (show_humidity && hum_line[0] != '\0') {
        uint8_t hum_width = ssd1306_measure_text_width(hum_line, 2U);
        uint8_t hum_x = (hum_width < SSD1306_WIDTH) ? (uint8_t)((SSD1306_WIDTH - hum_width) / 2U) : 0U;
        ssd1306_draw_string_scaled(hum_x, 38, hum_line, 2U);
      }

      ssd1306_update();
    }

    HAL_Delay(1000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00000608;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static bool tick_elapsed(uint32_t now, uint32_t target)
{
  return ((int32_t)(now - target) >= 0);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == B1_Pin) {
    uint32_t now = HAL_GetTick();
    if (!heater_active && !heater_trigger_requested && tick_elapsed(now, heater_next_allowed_ms)) {
      heater_trigger_requested = true;
    }
  }
}

static int run_heater_cycle(void)
{
  int status = 0;
  int16_t command_result = NO_ERROR;
  bool any_sensor = false;

  if (has_sensor_1) {
    any_sensor = true;
    sht4x_init(SHT43_I2C_ADDR_44);
    command_result = sht4x_activate_highest_heater_power_long_ticks(&temp_ticks_1, &hum_ticks_1);
    if (command_result != NO_ERROR) {
      status = command_result;
      i2c_error_code = command_result;
    } else {
      calculated_temp_1 = sht4x_temp_centi_from_ticks(temp_ticks_1) + 5500;
      calculated_hum_1 = sht4x_rh_centi_from_ticks(hum_ticks_1);
    }
  }

  if (has_sensor_2) {
    any_sensor = true;
    sht4x_init(SHT40_I2C_ADDR_46);
    command_result = sht4x_activate_highest_heater_power_long_ticks(&temp_ticks_2, &hum_ticks_2);
    if (command_result != NO_ERROR) {
      if (status == 0) {
        status = command_result;
        i2c_error_code = command_result;
      }
    } else {
      calculated_temp_2 = sht4x_temp_centi_from_ticks(temp_ticks_2) + 5500;
      calculated_hum_2 = sht4x_rh_centi_from_ticks(hum_ticks_2);
    }
  }

  if (!any_sensor) {
    status = NO_SENSORS_FOUND;
    i2c_error_code = NO_SENSORS_FOUND;
  } else if (status == 0) {
    i2c_error_code = NO_ERROR;
  }

  return status;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
