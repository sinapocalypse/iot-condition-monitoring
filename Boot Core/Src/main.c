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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <core_cm3.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define VERSION                         	"STM32-2024.09.08-v1.6.2-2Sen"

#define FOTA_START                          1
#define FOTA_STOP                           0
#define FOTA_DONE                           2

#define FOTA_ROW_SIZE                       42 // ??????????????????????????
// external e2prom
#define EE2_1_CHIP_ADDR                     0x56 << 1
#define EE2_2_CHIP_ADDR                     0x50


#define	EE2_METADATA_START_ADDRESS			0
#define	IE2_Loc_FOTA_ROW_START				0x08040400

#define EE2_Loc_Reset_Status             	(EE2_METADATA_START_ADDRESS + 38)
#define EE2_Loc_Reset_Code_Address         	(EE2_METADATA_START_ADDRESS + 39)

// FOTA METADATA
#define EE2_Loc_FOTA_STATUS                 (EE2_METADATA_START_ADDRESS + 20)
#define EE2_Loc_FOTA_CRC_BYTE0              (EE2_METADATA_START_ADDRESS + 21)
#define EE2_Loc_FOTA_CRC_BYTE1              (EE2_METADATA_START_ADDRESS + 22)
#define EE2_Loc_FOTA_CRC_BYTE2              (EE2_METADATA_START_ADDRESS + 23)
#define EE2_Loc_FOTA_CRC_BYTE3              (EE2_METADATA_START_ADDRESS + 24)
#define EE2_Loc_FOTA_App_SIZE_BYTE0         (EE2_METADATA_START_ADDRESS + 25)
#define EE2_Loc_FOTA_App_SIZE_BYTE1         (EE2_METADATA_START_ADDRESS + 26)
#define EE2_Loc_FOTA_App_SIZE_BYTE2         (EE2_METADATA_START_ADDRESS + 27)
#define EE2_Loc_FOTA_App_SIZE_BYTE3         (EE2_METADATA_START_ADDRESS + 28)
#define EE2_Loc_FOTA_ROW_START              (EE2_METADATA_START_ADDRESS + 29)

#define MAX_BLOCK_SIZE          			( 1024 )                  //1KB
#define ETX_APP_START_ADDRESS   			0x08008800

#define	IE2_METADATA_START_ADDRESS			0x08007800
#define METADATA_STRING_LENGTH				200

#define OFF	0b00000000
#define WHT	0b00000111

#define RED	0b00000001
#define GRE 0b00000100
#define BLU 0b00000010
#define YEL 0b00000011
#define PUR 0b00000101
#define CYA	0b00000110

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint8_t iox_num = 0;
uint8_t iox_id_array[3] = {0, 0, 0};
uint8_t qctl_clk_str[32] = "01/01/01,00:00:00+00";

uint8_t MAIN_ID = 0;
uint8_t date_year = 1;
uint8_t date_month = 1;
uint8_t date_day = 1;
uint8_t date_hour = 0;
uint8_t date_minute = 0;
uint8_t date_second = 0;
uint8_t reset_status = 0;
uint8_t reset_cause = 0;
uint8_t is_log_service_activated = 0;
uint8_t qctl_ftp_is_dir_exist = 0;
uint8_t modbus_default_sen_stat = 0;
uint32_t cm_wtd_cnt = 0;
uint32_t cm_swt_cnt = 0;
uint8_t io_enable = 0;
uint8_t outs_num = 0;
uint8_t flash_fota_flag = 0;
uint32_t flash_fota_app_size = 0;

char emq_server_ip[20+1] = "emq.iroteam.com";
char emq_server_port[20+1] = "31536";
char emq_server_pass[20+1] = "43a35b9326ea46e"; // user >> IMEI

char log_server_ip[20+1] = "2.188.210.4";
char log_server_port[20+1] = "21";
char log_server_user[20+1] = "IroTeam1";
char log_server_pass[20+1] = "IroTeam@1324";
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart5;

/* USER CODE BEGIN PV */
//uint8_t BL_Version[2] = { MAJOR, MINOR };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_UART5_Init(void);
/* USER CODE BEGIN PFP */
void update_firmware();
static void jump_to_app(uint8_t flag);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void uart_debug_print(char* msg)
{
	HAL_UART_Transmit(&huart5,(const uint8_t*)(msg), strlen((const char*)(msg)), 500);

	return;
}

// #########################################################################################################################
uint32_t internal_flash_get_page(uint32_t Address)
{
	for (int indx=0; indx<256; indx++){
		if((Address < (0x08000000 + (FLASH_PAGE_SIZE *(indx+1))) ) && (Address >= (0x08000000 + FLASH_PAGE_SIZE*indx))){
			return (0x08000000 + FLASH_PAGE_SIZE*indx);
		}
	}

	return 0;
}

void df_byte_all_string(uint32_t *Data, char *Buf)
{
	int numberofbytes = ((strlen((char *)Data) / 4) + ((strlen((char *)Data) % 4) != 0)) * 4;
	uint32_t sen_buf_index = 0;
	for (int i=0; i < numberofbytes; i++)
	{
		char c = Data[i / 4] >> (8 * (i % 4));
		if(isalnum(c) || c == ',' || c == '/' || c == ':' || c == '+' || c == '.' || c == '@' || c == '-'){
			Buf[sen_buf_index] = c;
			sen_buf_index++;
		}
	}
	Buf[sen_buf_index] = '\0';

	return;
}

uint32_t internal_flash_write_metadata()
{
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;
	uint32_t StartPageAddress = IE2_METADATA_START_ADDRESS;
	int sofar = 0;

//  data pattern: METADATA,[id],[DATE],[TIME],[Reset_Status],[Reset_Cause],[LOG_STATUS],[DEFAULT_SENSORS],[wdt],[swt],[io_enable],[out],[iox_num],[iox_IDs],[FOTA_STATUS],[FOTA_SIZE],[EMQ_IP],[EMQ_PORT],[EMQ_PASS],[FTP_ADMIN_IP],[FTP_ADMIN_PORT],[FTP_ADMIN_USERNAME],[FTP_ADMIN_PASS]
//	sample data: METADATA,00,24/02/06,15:27:25+14,00,00,00,00,00000007,00000012,00,02,03,05,02,15,00,00016738,emq.iroteam.com,31536,XXXXXXXXXXXXXXX,2.188.210.4,21,IroTeam1,IroTeam@1324,
//	default data: "00,,00,00,00,00,00,00000000,00000000,00,00,00,00000000,emq.iroteam.com,31536,XXXXXXXXXXXXXXX,2.188.210.4,21,IroTeam1,IroTeam@1324,;

	char dd[256];

	if(iox_num == 0){
		sprintf(dd, "METADATA,%02u,%02u/%02u/%02u,%02u:%02u:%02u+00,%02u,%02u,%02u,%02u,%08lX,%08lX,%02u,%02u,00,%02u,%08lX,",
				MAIN_ID,
				date_year,
				date_month,
				date_day,
				date_hour,
				date_minute,
				date_second,
				reset_status,
				reset_cause,
				is_log_service_activated,
				modbus_default_sen_stat,
				cm_wtd_cnt,
				cm_swt_cnt,
				io_enable,
				outs_num,
				flash_fota_flag,
				flash_fota_app_size
				);

		strncat(dd, emq_server_ip, strlen(emq_server_ip));
		strcat(dd, ",");
		strncat(dd, emq_server_port, strlen(emq_server_port));
		strcat(dd, ",");
		strncat(dd, emq_server_pass, strlen(emq_server_pass));
		strcat(dd, ",");

		strncat(dd, log_server_ip, strlen(log_server_ip));
		strcat(dd, ",");
		strncat(dd, log_server_port, strlen(log_server_port));
		strcat(dd, ",");
		strncat(dd, log_server_user, strlen(log_server_user));
		strcat(dd, ",");
		strncat(dd, log_server_pass, strlen(log_server_pass));
		strcat(dd, ",");
	}
	else if(iox_num == 1){
		sprintf(dd, "METADATA,%02u,%02u/%02u/%02u,%02u:%02u:%02u+00,%02u,%02u,%02u,%02u,%08lX,%08lX,%02u,%02u,01,%02u,%02u,%08lX,",
				MAIN_ID,
				date_year,
				date_month,
				date_day,
				date_hour,
				date_minute,
				date_second,
				reset_status,
				reset_cause,
				is_log_service_activated,
				modbus_default_sen_stat,
				cm_wtd_cnt,
				cm_swt_cnt,
				io_enable,
				outs_num,
				iox_id_array[0],
				flash_fota_flag,
				flash_fota_app_size);

		strncat(dd, emq_server_ip, strlen(emq_server_ip));
		strcat(dd, ",");
		strncat(dd, emq_server_port, strlen(emq_server_port));
		strcat(dd, ",");
		strncat(dd, emq_server_pass, strlen(emq_server_pass));
		strcat(dd, ",");

		strncat(dd, log_server_ip, strlen(log_server_ip));
		strcat(dd, ",");
		strncat(dd, log_server_port, strlen(log_server_port));
		strcat(dd, ",");
		strncat(dd, log_server_user, strlen(log_server_user));
		strcat(dd, ",");
		strncat(dd, log_server_pass, strlen(log_server_pass));
		strcat(dd, ",");
	}
	else if(iox_num == 2){
		sprintf(dd, "METADATA,%02u,%02u/%02u/%02u,%02u:%02u:%02u+00,%02u,%02u,%02u,%02u,%08lX,%08lX,%02u,%02u,02,%02u,%02u,%02u,%08lX,",
				MAIN_ID,
				date_year,
				date_month,
				date_day,
				date_hour,
				date_minute,
				date_second,
				reset_status,
				reset_cause,
				is_log_service_activated,
				modbus_default_sen_stat,
				cm_wtd_cnt,
				cm_swt_cnt,
				io_enable,
				outs_num,
				iox_id_array[0],
				iox_id_array[1],
				flash_fota_flag,
				flash_fota_app_size);

		strncat(dd, emq_server_ip, strlen(emq_server_ip));
		strcat(dd, ",");
		strncat(dd, emq_server_port, strlen(emq_server_port));
		strcat(dd, ",");
		strncat(dd, emq_server_pass, strlen(emq_server_pass));
		strcat(dd, ",");

		strncat(dd, log_server_ip, strlen(log_server_ip));
		strcat(dd, ",");
		strncat(dd, log_server_port, strlen(log_server_port));
		strcat(dd, ",");
		strncat(dd, log_server_user, strlen(log_server_user));
		strcat(dd, ",");
		strncat(dd, log_server_pass, strlen(log_server_pass));
		strcat(dd, ",");
	}
	else if(iox_num == 3){
		sprintf(dd, "METADATA,%02u,%02u/%02u/%02u,%02u:%02u:%02u+00,%02u,%02u,%02u,%02u,%08lX,%08lX,%02u,%02u,03,%02u,%02u,%02u,%02u,%08lX,",
				MAIN_ID,
				date_year,
				date_month,
				date_day,
				date_hour,
				date_minute,
				date_second,
				reset_status,
				reset_cause,
				is_log_service_activated,
				modbus_default_sen_stat,
				cm_wtd_cnt,
				cm_swt_cnt,
				io_enable,
				outs_num,
				iox_id_array[0],
				iox_id_array[1],
				iox_id_array[2],
				flash_fota_flag,
				flash_fota_app_size);

		strncat(dd, emq_server_ip, strlen(emq_server_ip));
		strcat(dd, ",");
		strncat(dd, emq_server_port, strlen(emq_server_port));
		strcat(dd, ",");
		strncat(dd, emq_server_pass, strlen(emq_server_pass));
		strcat(dd, ",");

		strncat(dd, log_server_ip, strlen(log_server_ip));
		strcat(dd, ",");
		strncat(dd, log_server_port, strlen(log_server_port));
		strcat(dd, ",");
		strncat(dd, log_server_user, strlen(log_server_user));
		strcat(dd, ",");
		strncat(dd, log_server_pass, strlen(log_server_pass));
		strcat(dd, ",");
	}


	uint8_t numofwords = (strlen(dd) / 4) + ((strlen(dd) % 4) != 0);

	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();

	/* Erase the user Flash area*/
	uint32_t StartPage = internal_flash_get_page(StartPageAddress);
	uint32_t EndPageAdress = StartPageAddress + (numofwords * 4);
	uint32_t EndPage = internal_flash_get_page(EndPageAdress);

	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = StartPage;
	EraseInitStruct.NbPages     = ((EndPage - StartPage) / FLASH_PAGE_SIZE) + 1;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK){
	/*Error occurred while page erase.*/
		return HAL_FLASH_GetError();
	}

	/* Program the user Flash area word by word*/
	uint32_t* new_data = (uint32_t *)(dd);
	while (sofar < numofwords){
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, StartPageAddress, new_data[sofar]) == HAL_OK){
			StartPageAddress += 4;  // use StartPageAddress += 2 for half word and 8 for double word
			sofar++;
		}else{
			/* Error occurred while writing data in Flash memory*/
			return HAL_FLASH_GetError ();
		}
	}

	/* Lock the Flash to disable the flash control register access (recommended
	to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock();

	uart_debug_print("metadata wrote: \r\n");
	uart_debug_print(dd);
	uart_debug_print("\r\n");

	return 0;
}
void internal_flash_read_metadata()
{

	uart_debug_print("++++++++++START METADATA+++++++++\r\n");

	uint32_t received_data[130];
	uint8_t received_data_index = 0;
	uint32_t StartPageAddress = IE2_METADATA_START_ADDRESS;
	char string_data[256];

	uint32_t numofwords = (METADATA_STRING_LENGTH / 4) + ((METADATA_STRING_LENGTH % 4) != 0);


	for(uint8_t i = 0; i < numofwords; i++){
		received_data[received_data_index] = *(__IO uint32_t *)StartPageAddress;
		StartPageAddress += 4;
		received_data_index++;
	}

	df_byte_all_string(received_data, string_data);

	if(memcmp(string_data, "METADATA", 8) == 0){
		// valid data to read
		// Skip "METADATA" tag...
		strtok((char*)string_data, ",");

		char* id_token = strtok(NULL, ",");
		uart_debug_print("Board ID: ");
		uart_debug_print(id_token);
		uart_debug_print("\r\n");

		char* date_year_token = strtok(NULL, "/");
		date_year = strtoul(date_year_token, NULL, 10);

		char* date_month_token = strtok(NULL, "/");
		date_month = strtoul(date_month_token, NULL, 10);

		char* date_day_token = strtok(NULL, ",");
		date_day = strtoul(date_day_token, NULL, 10);

		char* date_hour_token = strtok(NULL, ":");
		date_hour = strtoul(date_hour_token, NULL, 10);

		char* date_minute_token = strtok(NULL, ":");
		date_minute = strtoul(date_minute_token, NULL, 10);

		char* date_second_token = strtok(NULL, "+");
		date_second = strtoul(date_second_token, NULL, 10);

		// skip "+00" in datetime
		strtok(NULL, ",");

		sprintf((char*)qctl_clk_str, "%02u/%02u/%02u,%02u:%02u:%02u+00",
				date_year,
				date_month,
				date_day,
				date_hour,
				date_minute,
				date_second);

		uart_debug_print("DateTime: ");
		uart_debug_print((char*)qctl_clk_str);
		uart_debug_print("\r\n");

		char bf[50];

		char* reset_status_token = strtok(NULL, ",");
		reset_status = strtoul(reset_status_token, NULL, 10);
		sprintf(bf,"reset_status: %d\r\n", reset_status);
		uart_debug_print(bf);

		char* reset_cause_token = strtok(NULL, ",");
		reset_cause = strtoul(reset_cause_token, NULL, 10);
		sprintf(bf,"cm_reset_flag: %d\r\n", reset_cause);
		uart_debug_print(bf);

		char* log_status_token = strtok(NULL, ",");
		is_log_service_activated = strtoul(log_status_token, NULL, 10);
		sprintf(bf,"is_log_service_activated: %d\r\n", is_log_service_activated);
		uart_debug_print(bf);

//		char* log_directory_token = strtok(NULL, ",");
//		qctl_ftp_is_dir_exist = strtoul(log_directory_token, NULL, 10);
//		sprintf(bf,"qctl_ftp_is_dir_exist: %d\r\n", qctl_ftp_is_dir_exist);
//		uart_debug_print(bf);

		char* default_sen_token = strtok(NULL, ",");
		modbus_default_sen_stat = strtoul(default_sen_token, NULL, 10);
		sprintf(bf,"modbus_default_sen_stat: %d\r\n", modbus_default_sen_stat);
		uart_debug_print(bf);

		char* wdt_token = strtok(NULL, ",");
		cm_wtd_cnt = strtoul(wdt_token, NULL, 16);
		sprintf(bf,"cm_wtd_cnt: %08lX\r\n", cm_wtd_cnt);
		uart_debug_print(bf);

		char* swt_token = strtok(NULL, ",");
		cm_swt_cnt = strtoul(swt_token, NULL, 16);
		sprintf(bf,"cm_swt_cnt: %08lX\r\n", cm_swt_cnt);
		uart_debug_print(bf);

		char* io_enable_token = strtok(NULL, ",");
		io_enable = strtoul(io_enable_token , NULL, 10);
		sprintf(bf,"io_enable: %d\r\n", io_enable);
		uart_debug_print(bf);

		char* out_token = strtok(NULL, ",");
		outs_num = strtoul(out_token, NULL, 10);
		sprintf(bf,"outs_num: %d\r\n", outs_num);
		uart_debug_print(bf);

		char* iox_token = strtok(NULL, ",");
		iox_num = strtoul(iox_token, NULL, 10);
		sprintf(bf,"iox_num: %d ", iox_num);
		uart_debug_print(bf);
		if(iox_num){
			uart_debug_print(" >> IDs: ");

			for (uint8_t i = 0; i < iox_num; i++) {
				char* id = strtok(NULL, ",");
				iox_id_array[i] = strtoul(id, NULL, 10);
				sprintf(bf,"%d ", iox_id_array[i]);
				uart_debug_print(bf);
				if(i != 2){
					uart_debug_print(" - ");
				}
			}
		}
		uart_debug_print("\r\n");

		char* fota_status_token = strtok(NULL, ",");
		flash_fota_flag = strtoul(fota_status_token, NULL, 10);
		sprintf(bf,"flash_fota_flag: %d\r\n", flash_fota_flag);
		uart_debug_print(bf);

		char* fota_size_token = strtok(NULL, ",");
		flash_fota_app_size = strtoul(fota_size_token, NULL, 16);
		sprintf(bf,"flash_fota_app_size: %08lX\r\n", flash_fota_app_size);
		uart_debug_print(bf);

		uart_debug_print("\r\n");

		//[EMQ_IP],[EMQ_PORT],[EMQ_PASS],[FTP_ADMIN_IP],[FTP_ADMIN_PORT],[FTP_ADMIN_USERNAME],[FTP_ADMIN_PASS]
		char* emq_ip_token = strtok(NULL, ",");
		strncpy(emq_server_ip, emq_ip_token, strlen(emq_ip_token));
		emq_server_ip[strlen(emq_ip_token)]='\0';
		sprintf(bf,"emq_server_ip:\t\t");
		strncat(bf, emq_server_ip, strlen(emq_server_ip));
		uart_debug_print(bf);
		uart_debug_print("\r\n");


		char* emq_port_token = strtok(NULL, ",");
		strncpy(emq_server_port, emq_port_token, strlen(emq_port_token));
		emq_server_port[strlen(emq_port_token)]='\0';
		sprintf(bf,"emq_server_port:\t");
		strncat(bf, emq_server_port, strlen(emq_server_port));
		uart_debug_print(bf);
		uart_debug_print("\r\n");

		char* emq_pass_token = strtok(NULL, ",");
		strncpy(emq_server_pass, emq_pass_token, strlen(emq_pass_token));
		emq_server_pass[strlen(emq_pass_token)]='\0';
		sprintf(bf,"emq_server_pass:\t");
		strncat(bf, emq_server_pass, strlen(emq_server_pass));
		uart_debug_print(bf);
		uart_debug_print("\r\n");

		char* log_server_ip_token = strtok(NULL, ",");
		strncpy(log_server_ip, log_server_ip_token, strlen(log_server_ip_token));
		log_server_ip[strlen(log_server_ip_token)]='\0';
		sprintf(bf,"log_server_ip:\t\t");
		strncat(bf, log_server_ip, strlen(log_server_ip));
		uart_debug_print(bf);
		uart_debug_print("\r\n");

		char* log_server_port_token = strtok(NULL, ",");
		strncpy(log_server_port, log_server_port_token, strlen(log_server_port_token));
		log_server_port[strlen(log_server_port_token)]='\0';
		sprintf(bf,"log_server_port:\t");
		strncat(bf, log_server_port, strlen(log_server_port));
		uart_debug_print(bf);
		uart_debug_print("\r\n");

		char* log_server_username_token = strtok(NULL, ",");
		strncpy(log_server_user, log_server_username_token, strlen(log_server_username_token));
		log_server_user[strlen(log_server_username_token)]='\0';
		sprintf(bf,"log_server_user:\t");
		strncat(bf, log_server_user, strlen(log_server_user));
		uart_debug_print(bf);
		uart_debug_print("\r\n");

		char* log_server_pass_token = strtok(NULL, ",");
		strncpy(log_server_pass, log_server_pass_token, strlen(log_server_pass_token));
		log_server_pass[strlen(log_server_pass_token)]='\0';
		sprintf(bf,"log_server_pass:\t");
		strncat(bf, log_server_pass, strlen(log_server_pass));
		uart_debug_print(bf);
		uart_debug_print("\r\n");
	}
	else{
		// invalid data in flash memory
		uart_debug_print("internal_flash_read_metadata >> No Valid Data found...writing default values...\r\n");
		internal_flash_write_metadata();
	}

	uart_debug_print("++++++++++END METADATA++++++++++\r\n");

	return;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! use INTERNAL FLASH as Internal EEPROM
HAL_StatusTypeDef internal_flash_write_WORD(uint32_t Flash_Address, uint32_t data){
	HAL_StatusTypeDef internal_flash_ret = HAL_ERROR;
	internal_flash_ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Flash_Address, (uint64_t)data);

    return internal_flash_ret;
}

uint16_t internal_flash_read_HALFWORD(uint32_t Flash_Address){
	__IO uint32_t read_data = *(__IO uint32_t *)Flash_Address;

	return (uint16_t)read_data;
}

// SOURCE: https://stackoverflow.com/questions/34196663/stm32-how-to-get-last-reset-status
typedef enum reset_cause_e
{
    RESET_CAUSE_UNKNOWN = 0,
    RESET_CAUSE_LOW_POWER_RESET,
    RESET_CAUSE_WINDOW_WATCHDOG_RESET,
    RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET,
    RESET_CAUSE_SOFTWARE_RESET,
    RESET_CAUSE_POWER_ON_POWER_DOWN_RESET,
    RESET_CAUSE_EXTERNAL_RESET_PIN_RESET
    //RESET_CAUSE_BROWNOUT_RESET,
} reset_cause_t;

/// @brief      Obtain the STM32 system reset cause
/// @param      None
/// @return     The system reset cause
reset_cause_t reset_cause_get(void)
{
    reset_cause_t reset_cause;

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
    {
        reset_cause = RESET_CAUSE_LOW_POWER_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        reset_cause = RESET_CAUSE_WINDOW_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        reset_cause = RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
        // This reset is induced by calling the ARM CMSIS
        // `NVIC_SystemReset()` function!
        reset_cause = RESET_CAUSE_SOFTWARE_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
    {
        reset_cause = RESET_CAUSE_POWER_ON_POWER_DOWN_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
    {
        reset_cause = RESET_CAUSE_EXTERNAL_RESET_PIN_RESET;
    }
    // Needs to come *after* checking the `RCC_FLAG_PORRST` flag in order to
    // ensure first that the reset cause is NOT a POR/PDR reset. See note
    // below.
//    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST))
//    {
//        reset_cause = RESET_CAUSE_BROWNOUT_RESET;
//    }
    else
    {
        reset_cause = RESET_CAUSE_UNKNOWN;
    }

    // Clear all the reset flags or else they will remain set during future
    // resets until system power is fully removed.
    __HAL_RCC_CLEAR_RESET_FLAGS();

    return reset_cause;
}

// Note: any of the STM32 Hardware Abstraction Layer (HAL) Reset and Clock
// Controller (RCC) header files, such as
// "STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_rcc.h",
// "STM32Cube_FW_F2_V1.7.0/Drivers/STM32F2xx_HAL_Driver/Inc/stm32f2xx_hal_rcc.h",
// etc., indicate that the brownout flag, `RCC_FLAG_BORRST`, will be set in
// the event of a "POR/PDR or BOR reset". This means that a Power-On Reset
// (POR), Power-Down Reset (PDR), OR Brownout Reset (BOR) will trip this flag.
// See the doxygen just above their definition for the
// `__HAL_RCC_GET_FLAG()` macro to see this:
//      "@arg RCC_FLAG_BORRST: POR/PDR or BOR reset." <== indicates the Brownout
//      Reset flag will *also* be set in the event of a POR/PDR.
// Therefore, you must check the Brownout Reset flag, `RCC_FLAG_BORRST`, *after*
// first checking the `RCC_FLAG_PORRST` flag in order to ensure first that the
// reset cause is NOT a POR/PDR reset.


/// @brief      Obtain the system reset cause as an ASCII-printable name string
///             from a reset cause type
/// @param[in]  reset_cause     The previously-obtained system reset cause
/// @return     A null-terminated ASCII name string describing the system
///             reset cause
const char * reset_cause_get_name(reset_cause_t cause)
{
    const char * reset_cause_name = "TBD";

    switch (cause)
    {
        case RESET_CAUSE_UNKNOWN:
            reset_cause_name = "UNKNOWN";
            reset_status = 0;
            break;
        case RESET_CAUSE_LOW_POWER_RESET:
            reset_cause_name = "LOW_POWER_RESET";
            reset_status = 1;
            break;
        case RESET_CAUSE_WINDOW_WATCHDOG_RESET:
            reset_cause_name = "WINDOW_WATCHDOG_RESET";
            reset_status = 2;
            break;
        case RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET:
            reset_cause_name = "INDEPENDENT_WATCHDOG_RESET";
            reset_status = 3;
            break;
        case RESET_CAUSE_SOFTWARE_RESET:
            reset_cause_name = "SOFTWARE_RESET";
            reset_status = 4;
            break;
        case RESET_CAUSE_POWER_ON_POWER_DOWN_RESET:
            reset_cause_name = "POWER-ON_RESET (POR) / POWER-DOWN_RESET (PDR)";
            reset_status = 5;
            break;
        case RESET_CAUSE_EXTERNAL_RESET_PIN_RESET:
            reset_cause_name = "EXTERNAL_RESET_PIN_RESET";
            reset_status = 6;
            break;
//        case RESET_CAUSE_BROWNOUT_RESET:
//            reset_cause_name = "BROWNOUT_RESET (BOR)";
//            cm_reset_flag = 7;
//            break;
    }

    return reset_cause_name;
}
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! use INTERNAL FLASH as Internal EEPROM
void led_set_color(uint8_t color_code)
{
	if(color_code < 8){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, (~((color_code & 0b00000001) >> 0) & 0b00000001)); //red
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (~((color_code & 0b00000010) >> 1) & 0b00000001)); //green
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, (~((color_code & 0b00000100) >> 2) & 0b00000001)); //blue
	}


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
  MX_I2C1_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */
  uart_debug_print("\r\n############################ START BOOTLOADER ############################\r\n");
  char ver[100];
  sprintf(ver, "\r\n############################ BOOT VERSION: ");
  strcat(ver, (char*)VERSION);
  strcat(ver, " ############################\r\n");
  uart_debug_print(ver);

  internal_flash_read_metadata();

  reset_cause_t reset_cause = reset_cause_get();
  char buf[64];
  sprintf(buf, "\r\n############################ CHIP RESET CAUSE >> ");
  strcat(buf, "\"");
  strcat(buf, reset_cause_get_name(reset_cause));
  strcat(buf, "\" ############################\r\n");
  uart_debug_print(buf);

//  ee2_write_byte(EE2_1_CHIP_ADDR, cm_reset_flag, EE2_Loc_Reset_Status);

  update_firmware();

//  jump_to_app(0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  uart_debug_print(">> BootLoader failed >> FATAL ERROR...\r\n");
	  HAL_Delay(1000);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 57600;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIO_RGB_1_GPIO_Port, GPIO_RGB_1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_RGB_3_Pin|GPIO_RGB_2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : GPIO_RGB_1_Pin */
  GPIO_InitStruct.Pin = GPIO_RGB_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIO_RGB_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_RGB_3_Pin GPIO_RGB_2_Pin */
  GPIO_InitStruct.Pin = GPIO_RGB_3_Pin|GPIO_RGB_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void update_firmware()
{
//	ee2_writeByte(EE2_1_CHIP_ADDR, FOTA_START, EE2_Loc_FOTA_STATUS);
	/* ***************************** CHECK METADATA FROM External EEPROM ***************************** */
//	HAL_StatusTypeDef status = ee2_read_byte(EE2_1_CHIP_ADDR, &byte, EE2_Loc_FOTA_STATUS);
//
//
//	uint8_t byte1 = 0;
//	uint8_t byte2 = 0;
//	uint8_t byte3 = 0;
//	uint8_t byte4 = 0;
//
//	HAL_StatusTypeDef s1 = ee2_read_byte(EE2_1_CHIP_ADDR, &byte1, EE2_Loc_FOTA_App_SIZE_BYTE3);
//	HAL_StatusTypeDef s2 = ee2_read_byte(EE2_1_CHIP_ADDR, &byte2, EE2_Loc_FOTA_App_SIZE_BYTE2);
//	HAL_StatusTypeDef s3 = ee2_read_byte(EE2_1_CHIP_ADDR, &byte3, EE2_Loc_FOTA_App_SIZE_BYTE1);
//	HAL_StatusTypeDef s4 = ee2_read_byte(EE2_1_CHIP_ADDR, &byte4, EE2_Loc_FOTA_App_SIZE_BYTE0);

//	unsigned int appSize = (uint32_t)byte1 << 24 | (uint32_t)byte2 << 16 | (uint32_t)byte3 << 8 | (uint32_t)byte4 << 0;
	/* ************************************************************************************************ */

	uint8_t fail = 0;

	if(flash_fota_flag == FOTA_START){
		HAL_StatusTypeDef ret = HAL_FLASH_Unlock();
		if(ret != HAL_OK){
			uart_debug_print("\r\n############################ HAL_FLASH_Unlock >> FAILED! Please try again later...	############################\r\n");
			jump_to_app(0);
		}
		else{
			uart_debug_print("	*****	HAL_FLASH_Unlock >> OK	*****	\r\n");

			uart_debug_print("	*****	Erasing Flash Memory...	*****	\r\n");
			//Erase the Flash
			FLASH_EraseInitTypeDef EraseInitStruct;
			uint32_t SectorError;

			EraseInitStruct.TypeErase     = FLASH_TYPEERASE_PAGES;
			EraseInitStruct.PageAddress   = ETX_APP_START_ADDRESS;
			EraseInitStruct.NbPages       = 110;                     // from 0x08008800 to 0x08040000 (2k Pages)

			ret = HAL_FLASHEx_Erase( &EraseInitStruct, &SectorError );
			if( ret != HAL_OK ){
				uart_debug_print("	*****	Erasing Flash FAILED!	*****	\r\n");
				jump_to_app(0);
			}
			else {
				uart_debug_print("	*****	Erasing Flash Done!	*****	\r\n");
			}

			char buf[100];
			sprintf(buf, ">> Application size: %08lu\r\n", flash_fota_app_size);
			uart_debug_print(buf);

			int flash_write_progress = 0;
			int percent = 0;
			int former_percent = 0;

			for (int var = 0; var < flash_fota_app_size; var += 4) {

				uint32_t four_bytes = internal_flash_read_HALFWORD(IE2_Loc_FOTA_ROW_START + ( var + 3 )) << 24
									| internal_flash_read_HALFWORD(IE2_Loc_FOTA_ROW_START + ( var + 2 )) << 16
									| internal_flash_read_HALFWORD(IE2_Loc_FOTA_ROW_START + ( var + 1 )) << 8
									| internal_flash_read_HALFWORD(IE2_Loc_FOTA_ROW_START + ( var + 0 )) << 0;

				ret = internal_flash_write_WORD(ETX_APP_START_ADDRESS + var, four_bytes);

				flash_write_progress++;


				percent = (((float)flash_write_progress) / flash_fota_app_size) * 100;
				if( percent % 5 == 0 && percent != former_percent){
					led_set_color(YEL);
					former_percent = percent;
					sprintf(buf, ">> Flash Writing Progress >> %d%% \r\n", (percent * 4));
					uart_debug_print(buf);
					led_set_color(OFF);
				}

				if(ret != HAL_OK){
					uart_debug_print(">> FLASH_WRITE_NOT_OK!\r\n");
					fail = 1;
					break;
				}
			}

			ret = HAL_FLASH_Lock();

			if(ret != HAL_OK){
				fail = 2;
			}
			else{
				uart_debug_print("	*****	HAL_FLASH_Lock => OK	*****	\r\n");
			}

			if(fail == 0){
//				ee2_write_byte(EE2_1_CHIP_ADDR, FOTA_STOP, EE2_Loc_FOTA_STATUS);
//				ee2_write_byte(EE2_1_CHIP_ADDR, FOTA_DONE, EE2_Loc_Reset_Code_Address);
				flash_fota_flag = FOTA_STOP;
				reset_cause = FOTA_DONE;
				uart_debug_print("\r\n############################ Programming FLASH finished successfully... ############################\r\n");
				// Reset Watchdog and Software counts
				cm_wtd_cnt = 0;
				cm_swt_cnt = 0;
				internal_flash_write_metadata();
				jump_to_app(1);
			}
			else if(fail == 1){
//				ee2_write_byte(EE2_1_CHIP_ADDR, FOTA_STOP, EE2_Loc_FOTA_STATUS);
				flash_fota_flag = FOTA_STOP;
				uart_debug_print("\r\n############################ Programming FLASH FAILED! Error occurred in writing into the FLASH Memory. ############################\r\n");
				internal_flash_write_metadata();
				HAL_Delay(1000);
				NVIC_SystemReset();
			}
			else if(fail == 2){
//				ee2_write_byte(EE2_1_CHIP_ADDR, FOTA_STOP, EE2_Loc_FOTA_STATUS);
				flash_fota_flag = FOTA_STOP;
				uart_debug_print("\r\n############################ Programming FLASH FAILED! Error occurred in Locking the FLASH Memory. ############################\r\n");
				internal_flash_write_metadata();
				HAL_Delay(1000);
				NVIC_SystemReset();
			}
		}

	}
	else{
		uart_debug_print("\r\n############################ BOOTLOADER >> No New Program to write! ############################\r\n");
		internal_flash_write_metadata();
		jump_to_app(0);
	}

	return;
}

static void jump_to_app(uint8_t flag)
{
	if(flag){
		uart_debug_print("\r\n############################ Jumping to NEW Application ############################\r\n");
	}
	else{
		uart_debug_print("\r\n############################ Jumping to Application ############################\r\n");
	}

//	void (*app_reset_handler)(void) = (void*)(*((volatile uint32_t*)(ETX_APP_START_ADDRESS)));
//
//	__set_MSP(*(volatile uint32_t*) ETX_APP_START_ADDRESS);
//
//	// Turn OFF the Led to tell the user that Bootloader is not running
////	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET );
//
//	app_reset_handler();    //call the app reset handler

//////////////////////////	CORRECT	////////////////////////////////////

	HAL_RCC_DeInit();

	__disable_irq();

	for (int i = 0; i < (sizeof(NVIC->ICPR) / sizeof(NVIC->ICPR[0])); i++) {
	  NVIC_ClearPendingIRQ(i);
	}

	__set_CONTROL(0x00);

	typedef void (*jump_app)(void);

	volatile uint32_t *_vectable = (volatile uint32_t *)(__IO uint32_t*)(ETX_APP_START_ADDRESS);  // point _vectable to the start of the application at 0x08005000

	jump_app app_jump = (jump_app) *(_vectable + 1);   // get the address of the application's reset handler by loading the 2nd entry in the table

	SCB->VTOR = *_vectable;   // point VTOR to the start of the application's vector table

	__set_MSP(*_vectable);   // setup the initial stack pointer using the RAM address contained at the start of the vector table

	app_jump();   // call the application's reset handler


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
