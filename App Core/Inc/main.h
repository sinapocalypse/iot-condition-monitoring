/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "proc.h"
#include "modbus.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
//RGB colors
#define OFF	0b00000000
#define WHT	0b00000111

#define RED	0b00000001
#define GRE 0b00000100
#define BLU 0b00000010
#define YEL 0b00000011
#define PUR 0b00000101
#define CYA	0b00000110
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GPIO_RGB_1_Pin GPIO_PIN_13
#define GPIO_RGB_1_GPIO_Port GPIOC
#define IN_5_Pin GPIO_PIN_0
#define IN_5_GPIO_Port GPIOC
#define IN_6_Pin GPIO_PIN_1
#define IN_6_GPIO_Port GPIOC
#define IN_7_Pin GPIO_PIN_2
#define IN_7_GPIO_Port GPIOC
#define IN_8_Pin GPIO_PIN_3
#define IN_8_GPIO_Port GPIOC
#define RS485_Sensor_RE_DE_Pin GPIO_PIN_0
#define RS485_Sensor_RE_DE_GPIO_Port GPIOA
#define RELAY_1_Pin GPIO_PIN_1
#define RELAY_1_GPIO_Port GPIOA
#define RS485_Sensor_TX_Pin GPIO_PIN_2
#define RS485_Sensor_TX_GPIO_Port GPIOA
#define RS485_Sensor_RX_Pin GPIO_PIN_3
#define RS485_Sensor_RX_GPIO_Port GPIOA
#define OUT_8_Pin GPIO_PIN_5
#define OUT_8_GPIO_Port GPIOC
#define OUT_7_Pin GPIO_PIN_0
#define OUT_7_GPIO_Port GPIOB
#define OUT_6_Pin GPIO_PIN_1
#define OUT_6_GPIO_Port GPIOB
#define OUT_5_Pin GPIO_PIN_2
#define OUT_5_GPIO_Port GPIOB
#define OUT_3_Pin GPIO_PIN_10
#define OUT_3_GPIO_Port GPIOB
#define OUT_4_Pin GPIO_PIN_11
#define OUT_4_GPIO_Port GPIOB
#define Quectel_PWRKEY_Pin GPIO_PIN_12
#define Quectel_PWRKEY_GPIO_Port GPIOB
#define IOX_RE_DE_Pin GPIO_PIN_13
#define IOX_RE_DE_GPIO_Port GPIOB
#define OUT_2_Pin GPIO_PIN_14
#define OUT_2_GPIO_Port GPIOB
#define OUT_1_Pin GPIO_PIN_15
#define OUT_1_GPIO_Port GPIOB
#define GPIO_SIMSELECT_Pin GPIO_PIN_6
#define GPIO_SIMSELECT_GPIO_Port GPIOC
#define UART_Quectel_TX_Pin GPIO_PIN_9
#define UART_Quectel_TX_GPIO_Port GPIOA
#define UART_Quectel_RX_Pin GPIO_PIN_10
#define UART_Quectel_RX_GPIO_Port GPIOA
#define IN_1_Pin GPIO_PIN_15
#define IN_1_GPIO_Port GPIOA
#define IN_2_Pin GPIO_PIN_3
#define IN_2_GPIO_Port GPIOB
#define IN_3_Pin GPIO_PIN_4
#define IN_3_GPIO_Port GPIOB
#define IN_4_Pin GPIO_PIN_5
#define IN_4_GPIO_Port GPIOB
#define GPIO_RGB_3_Pin GPIO_PIN_8
#define GPIO_RGB_3_GPIO_Port GPIOB
#define GPIO_RGB_2_Pin GPIO_PIN_9
#define GPIO_RGB_2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

extern void create_queues();
extern void create_callbacks();

extern void Timer_Read_Modbus_Callback();

extern void Timer_Read_Quectel_Callback();
extern void Timer_RESET_STATE_Callback();
extern void Timer_Read_Quectel_Publish_Callback();
//extern void Timer_Quectel_Power_down_Callback();
extern void Timer_Quectel_State_Sentinel_Callback();
extern void Timer_Quectel_SMS_Callback();
extern void Read_IOX_Callback();

extern void separate_gsm_resp();

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
