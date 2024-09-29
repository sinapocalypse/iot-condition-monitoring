/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Scheduler includes. */
#include "RTOS.h"

#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
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

/* USER CODE BEGIN PV */
char start_buff[100] = "\r\n############################ START APPLICATION ############################\r\n";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

///* USER CODE BEGIN Prototype_Task0 */
static void Task_Quectel(void);
static void Task_Operator(void);
static void Task_Io(void);

static void Task_Sep(void);

///* USER CODE END Prototype_Task0 */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//void Timer_Quectel_Power_down_Callback()
//{
//	qctl_state = QUECTEL_STATE_CHECK_POWER;
//	qctl_state_power = POWER_STATE_POWER_UP;
//
//	return;
//}

void Timer_Quectel_Publish_Callback()
{

	//HAL_UART_Transmit(&huart1, esc, 1, 1000);

	qctl_pub_resp_failure_counter++;
	if(qctl_pub_resp_failure_counter > 3){
//			file_fill_log_buff("Publish_Callback >> failure_counter = 3");
		uart_debug_print("Publish_Callback >> failure_counter = 3\r\n");

		qctl_pub_resp_failure_counter = 0;
		qctl_state = QUECTEL_STATE_CFUN;

		qctl_pub_resp_fatal_failure_counter++;
		if(qctl_pub_resp_fatal_failure_counter > 2){
//				file_fill_log_buff("Publish_Callback >> fatal_failure_counter = 2 >> reset chip");
			uart_debug_print("Publish_Callback >> fatal_failure_counter = 2 >> reset chip\r\n");
			qctl_pub_resp_fatal_failure_counter = 0;
			proc_reset_chip(Reset_Code_Mqtt_Pub_Resp_Fail);
		}
	}
	else{
		qctl_state_mqtt = MQTT_STATE_PUBLISH;
	}

	return;
}

void Timer_Quectel_SMS_Callback()
{
//    OS_TIMER_Restart(&osTimer_quectel_sms_mode);
//
//    void* ret = NULL;
//    ret = q_get_message(&_Queue_Get);
//
//    while(ret){
//    	HAL_IWDG_Refresh(&hiwdg);
//
//        if(memcmp((char*)ret, "{GET_ALL}:", 10) == 0){
//            strcpy((char*)qctl_sms_send_number, (char*)qctl_sms_numbers[2]);
//
////            uart_debug_print(" >> time to SMS!\r\n");
////			uart_debug_print(&((char*)ret)[strlen("{GET_ALL}:")]);
////			uart_debug_print("\r\n");
//
//            qctl_sms_send(&((char*)ret)[strlen("{GET_ALL}:")], qctl_sms_send_number);
//
//            OS_TIMER_Restart(&osTimer_quectel_state_sentinel);
//        }
//
//        ret = NULL;
//        ret = q_get_message(&_Queue_Get);
//    }


    return;
}

void Timer_Read_Quectel_Callback()
{
	if(!q_put_message(&_Queue_Quectel, (char*)quectl_recv_pack.buff, quectl_recv_pack.index + 1)){
		uart_debug_print("Timer_Read_Quectel_Callback >> failed to put to _Queue_Quectel >> ");
		uart_debug_print((char*)quectl_recv_pack.buff);
		uart_debug_print("\r\n");
	}

	memset(quectl_recv_pack.buff, '\0', quectl_recv_pack.index);
    quectl_recv_pack.index = 0;

    return;
}

void Timer_RESET_STATE_Callback()
{
	uart_debug_print("*** Timer_RESET_STATE_Callback >> no response for 185 seconds!\r\n");
	proc_reset_chip(Reset_Code_Quectel_Lost);

    return;
}

void Timer_Read_Modbus_Callback()
{
	char msg[100];
    if(modbus_recv_pack.buff[0] == ':'){ //ASCII
        //ascii
        if(modbus_recv_pack.index > Len_Max_Mod_Ascii_recv){
            if((modbus_recv_pack.buff[modbus_recv_pack.index - 1] == '\n') && (modbus_recv_pack.buff[modbus_recv_pack.index - 2] == '\r')){
                //find id
                modbus_recv_pack.id[0] = modbus_recv_pack.buff[1];
                modbus_recv_pack.id[1] = modbus_recv_pack.buff[2];
                modbus_recv_pack.id[2] = modbus_recv_pack.buff[3];
                modbus_recv_pack.id[3] = modbus_recv_pack.buff[4];
                modbus_recv_pack.id[4] = '\0';

                //function id
                uint8_t d = (uint8_t)('A');
                modbus_find_row(d);
            }
            else{
                modbus_err_counter++;
            	strcpy(msg ,"Timer_Read_Modbus_Callback: invalid modbus >> ASCII wrong end characters!!!");
                uart_debug_print(msg);
                uart_debug_print("\r\n");
            }
        }
        else{
            modbus_err_counter++;
        	strcpy(msg, "Timer_Read_Modbus_Callback: invalid modbus >> ASCII wrong length received!!!");
            uart_debug_print(msg);
            uart_debug_print("\r\n");
        }
    }
    else if(memcmp(&modbus_recv_pack.buff[2], ":{GET_INO}:{",12) == 0){ /*--:{GET_INO}:{"inp":"XXXXXX","out":"XXXXXX"}<NUL>IOX */
    	Read_IOX_Callback();
    }
    else if(modbus_recv_pack.index > Len_Max_Mod_rtu_recv){ //RTU
        //rtu
        sprintf((char*)modbus_recv_pack.id, "%02X%02X", modbus_recv_pack.buff[0], modbus_recv_pack.buff[1]);
        modbus_recv_pack.id[4] = '\0';

        //find id
        modbus_find_row('R');
    }
    else{
        modbus_err_counter++;
    	strcpy(msg, "Timer_Read_Modbus_Callback: invalid modbus >> minimum data byte received(ASCII/RTU)!!!");
        uart_debug_print(msg);
        uart_debug_print("\r\n");
    }

    modbus_recv_pack.id[0] = '\0';
    modbus_recv_pack.id[1] = '\0';
    modbus_recv_pack.id[2] = '\0';
    modbus_recv_pack.index = 0;
    memset(modbus_recv_pack.buff, '\0', sizeof(modbus_recv_pack.buff));

    if(modbus_err_counter > MODBUS_ERR_MAX_COUNTER){
    	modbus_err_counter = 0;
//    	file_fill_log_buff(msg);
    }

    return;
}

void Read_IOX_Callback()
{
    // read data from iox

	if(strlen((char*)modbus_recv_pack.buff) >= 12){
		//find id
		modbus_recv_pack.id[0] = modbus_recv_pack.buff[0];
		modbus_recv_pack.id[1] = modbus_recv_pack.buff[1];
		modbus_recv_pack.id[2] = '\0';

		//function id
		io_find_io_expander();
	}

    return;
}

void create_queues()
{
    OS_Q_Create(&_Queue_Set, &_QBuffer_Queue_Set, sizeof(_QBuffer_Queue_Set));
    OS_Q_Create(&_Queue_Get, &_QBuffer_Queue_Get, sizeof(_QBuffer_Queue_Get));
    OS_Q_Create(&_Queue_Sep, &_QBuffer_Queue_Sep, sizeof(_QBuffer_Queue_Sep));

    OS_Q_Create(&_Queue_Quectel, &_QBuffer_Queue_Quectel, sizeof(_QBuffer_Queue_Quectel));

    return;
}

void create_timers()
{
	OS_TIMER_Create(&osTimer_rs485_sensor,  Timer_Read_Modbus_Callback, Timeout_Modbus_Sensor_Timer_ms);    // Create : without start.
    OS_TIMER_Create(&osTimer_uart_quectel,  Timer_Read_Quectel_Callback, Timeout_Quectel_Uart_Recv_Timer_ms);    // Create : without start.
    OS_TIMER_Create(&osTimer_quectel_pub,  Timer_Quectel_Publish_Callback, Timeout_Quectel_Publish_Timer_ms);    // Create : without start.
	OS_TIMER_Create(&osTimer_reset_state,  Timer_RESET_STATE_Callback, Timeout_Reset_State_Timer_ms);    // Create without start.

//	OS_TIMER_Create(&osTimer_quectel_power_down,  Timer_Quectel_Power_down_Callback, Timeout_Quectel_Power_down_Timer_ms);    // Create without start.


	//	OS_TIMER_Create(&osTimer_quectel_sms_mode,  Timer_Quectel_SMS_Callback, Timeout_Quectel_SMS_Timer_ms);    // Create without start.

	return;
}

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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_UART5_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  __HAL_AFIO_REMAP_SWJ_NOJTAG(); // FYI : fix inp_2 problem

  /* Initialize the embOS kernel and configure the hardware parameters for embOS */
  OS_Init();
  OS_InitHW();

  // Start The Highest Priority Task first, then start other tasks within this task!
  OS_TASK_Create(&TCB_Task_Io, "Task_IO", 400, Task_Io, Stack_Task_Io, sizeof(Stack_Task_Io), 2);

  OS_Start();
  /* USER CODE END 2 */

  /* Initialize the embOS kernel and configure the hardware parameters for embOS */
  OS_Init();
  OS_InitHW();

  /* Start embOS */
  OS_Start();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	  HAL_IWDG_Refresh(&hiwdg);
	  uart_debug_print("no embOS ... \r\n");
	  led_set_color(RED);
	  HAL_Delay(500);
	  led_set_color(OFF);
	  HAL_Delay(500);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
uint8_t large_buf[2];

void Task_Io()
{
	//peripherals usage
		//1-internal flash (read/write) => only in init
		//2-chip temp adc
		//3- i2c for T&H sensor
		//4- modbus sensors and io-expanders
		//5- io write and pulling read





	// init and start process
	uart_debug_print(start_buff);
	char ver[100];
	sprintf(ver, "\r\n############################ APP VERSION: ");
	strcat(ver, (char*)VERSION);
	strcat(ver, " ############################\r\n\r\n");
	uart_debug_print(ver);

	create_timers();
	create_queues();

	proc_init();

	qctl_uart_baudrate = huart1.Init.BaudRate;
	qctl_qctl_baudrate = qctl_uart_baudrate;
//	HAL_UART_Receive_IT(&huart1, qctl_rx_buf, sizeof(qctl_rx_buf));
//	HAL_UART_Receive_IT(&huart2, modbus_rx_buf, sizeof(modbus_rx_buf));

	HAL_UART_Receive_DMA(&huart1, qctl_rx_buf, sizeof(qctl_rx_buf));
	HAL_UART_Receive_DMA(&huart2, modbus_rx_buf, sizeof(modbus_rx_buf));

    uart_debug_print("	>> Task_IO >> Start!\r\n");

	OS_TASK_Create(&TCB_Task_Operator, "Task_Operator", 200, Task_Operator, Stack_Task_Operator, sizeof(Stack_Task_Operator), 2);  // Create & start

	// TODO: :Last modified by SINA!:/ default sim select is 0
	HAL_GPIO_WritePin(GPIOC, GPIO_SIMSELECT_Pin, GPIO_PIN_RESET);


    int i = 0;
    while(1){
    	HAL_IWDG_Refresh(&hiwdg);

    	i++;
    	if(i == ((30 * 1000) / TASK_DELAY_IO_MS)){
    		i = 0;
			uart_debug_print("	>> Task_IO >> running...\r\n");
    	}

    	proc_task_io_sensor();

        OS_TASK_Delay(TASK_DELAY_IO_MS);
    }

    return;
}

void Task_Operator(void)
{

	//peripherals usage
		//1-put to _Queue_Get (all/ino/fota)
		//2-get form _Queue_Set (all cmds)

    uart_debug_print("	>> Task_Operator >> Start!\r\n");

    int i = 0;

	OS_TASK_Create(&TCB_Task_Quectel, "Task_Quectel", 100, Task_Quectel, Stack_Task_Quectel, sizeof(Stack_Task_Quectel), 2);  // Create & start
	OS_TASK_Create(&TCB_Task_Sep, "Task_Sep", 50, Task_Sep, Stack_Task_Sep, sizeof(Stack_Task_Sep), 2);  // Create & start

	while(1){
    	HAL_IWDG_Refresh(&hiwdg);

		i++;
		if(i == ((30 * 1000) / TASK_DELAY_OPERATOR_MS)){
			i = 0;
			uart_debug_print("	>> Task_Operator >> running...\r\n");
		}

		proc_task_operator();

		OS_TASK_Delay(TASK_DELAY_OPERATOR_MS);
	}
}

void Task_Quectel(void)
{
	//peripherals usage
		//1-internal flash write => in while
		//2-get from _Queue_Sep => quectel pure responses
		//3-get from _Queue_Get => publsih and fota process
		//4-uart quectel
    uart_debug_print("	>> Task_Quectel >> Start!\r\n");

	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)Adc.Raw, 2);
	HAL_TIM_Base_Start(&htim3);

//	qctl_mqtt_will();

	int i = 0;
	while (1){
		HAL_IWDG_Refresh(&hiwdg);

		i++;
		if(i == ((30 * 1000) / TASK_DELAY_QUECTEL_MS)){

			i = 0;
			uart_debug_print("	>> Task_Quectel >> running...\r\n");
		}

		proc_task_quectel();

		OS_TASK_Delay(TASK_DELAY_QUECTEL_MS);
	}

	return;
}

void Task_Sep(void)
{
	//peripherals usage
		//1- get from _Queue_Quectel
	uint32_t i = 0;
	while (1){
		HAL_IWDG_Refresh(&hiwdg);

		i++;
		if(i == ((30 * 1000) / TASK_DELAY_SEP_MS)){
			qctl_debug_state_flag = 1;
			i = 0;
			uart_debug_print("	>> Task_Sep >> running...\r\n");
		}

		proc_task_sep();

		OS_TASK_Delay(TASK_DELAY_SEP_MS);
	}
	return;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	OS_INT_Enter();

	 //USART1 >> for Quectel
	if(huart->Instance == USART1){
		OS_TIMER_Restart(&osTimer_uart_quectel);

		quectl_recv_pack.buff[quectl_recv_pack.index] = huart1.Instance->DR;
		quectl_recv_pack.index++;

		if(quectl_recv_pack.index > Uart_Gsm_Buffer_Len_Recv - 1){
			quectl_recv_pack.index = 0;
		}

//		HAL_UART_Receive_IT(&huart1, qctl_rx_buf, sizeof(qctl_rx_buf));
		HAL_UART_Receive_DMA(&huart1, qctl_rx_buf, sizeof(qctl_rx_buf));
	}

	 //USART2 >> for Modbus_Sensor & IOX
	else if(huart -> Instance == USART2){
		OS_TIMER_Restart(&osTimer_rs485_sensor);

		modbus_recv_pack.buff[modbus_recv_pack.index] = huart2.Instance->DR;
		modbus_recv_pack.index++;

		if(modbus_recv_pack.index > Len_Max_Mod_Buffer_Recv - 1){
			modbus_recv_pack.index = 0;
		}

//		HAL_UART_Receive_IT(&huart2, modbus_rx_buf, sizeof(modbus_rx_buf));
		HAL_UART_Receive_DMA(&huart2, modbus_rx_buf, sizeof(modbus_rx_buf));
	}

	OS_INT_Leave();

	return;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1){
		adc_flag = 1;
    }

	return;
}


/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  NVIC_SystemReset();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
