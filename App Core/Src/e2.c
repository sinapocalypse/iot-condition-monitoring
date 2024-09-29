/*
 * IE2.c
 *
 *  Created on: Jun 6, 2023
 *      Author: IroTeam
 */

#include "e2.h"

// ###################################################### EEProm Functional Functions ######################################
// use INTERNAL FLASH as Internal EEPROM
HAL_StatusTypeDef internal_flash_ret = HAL_ERROR;

HAL_StatusTypeDef internal_flash_write_word(uint32_t Flash_Address, uint32_t data)
{
	// clear all flags before you write it to flash
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);

	HAL_StatusTypeDef ret_write = HAL_ERROR;
	HAL_StatusTypeDef ret_lock = HAL_ERROR;

	ret_lock = HAL_FLASH_Unlock();
	if(ret_lock == HAL_OK){
		ret_write = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Flash_Address, (uint64_t)data);

		ret_lock = HAL_FLASH_Lock();

		if(ret_lock != HAL_OK){
//			file_fill_log_buff("internal_flash_write_byte >> HAL_FLASH_Lock >> FAILED	*****");
			uart_debug_print("internal_flash_write_byte >> HAL_FLASH_Lock >> FAILED	*****\r\n");
		}
	}
	else{
//		file_fill_log_buff("internal_flash_write_byte >> HAL_FLASH_Unlock >> FAILED");
		uart_debug_print("internal_flash_write_byte >> HAL_FLASH_Unlock >> FAILED\r\n");
	}

	return ret_write;
}

uint16_t internal_flash_read_byte(uint32_t Flash_Address){
	__IO uint32_t read_data = *(__IO uint32_t *)Flash_Address;
	return (uint16_t)read_data;
}

uint8_t internal_flash_erase()
{

	uint8_t ret = 1;
	HAL_StatusTypeDef status;
	status = HAL_FLASH_Unlock();

	if(status == HAL_OK){
		FLASH_EraseInitTypeDef EraseInitStruct;

		uart_debug_print("\r\n ***** internal_flash_erase >> Erasing FLASH Section, Please wait... ***** \r\n");
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.PageAddress = IE2_FOTA_START_APP_ADDRESS; // first address of the SECOND half of INTERNAL flash (for FOTA data)
		EraseInitStruct.NbPages = 100;

		uint32_t PageError;
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK){		//Erase the Page Before a Write Operation
//			file_fill_log_buff("internal_flash_erase >> ERASE FLASH >> FAILED");
			uart_debug_print("internal_flash_erase >> ERASE FLASH >> FAILED\r\n");

			ret = 0;
		}
		else{
			uart_debug_print("internal_flash_erase: ERASE FLASH >> DONE\r\n");
		}
	}

	status = HAL_FLASH_Lock();

	if(status != HAL_OK){
//		file_fill_log_buff("internal_flash_erase >> ERASE FLASH >> FAILED TO LOCK FLASH AGAIN");
		uart_debug_print("internal_flash_erase >> ERASE FLASH >> FAILED TO LOCK FLASH AGAIN\r\n");
		ret = 0;
	}

	return ret;
}

uint32_t internal_flash_get_page(uint32_t Address)
{
	for (int indx=0; indx<256; indx++){
		if((Address < (0x08000000 + (FLASH_PAGE_SIZE *(indx+1))) ) && (Address >= (0x08000000 + FLASH_PAGE_SIZE*indx))){
			return (0x08000000 + FLASH_PAGE_SIZE*indx);
		}
	}

	return 0;
}

uint32_t internal_flash_write_sensor (uint32_t *Data, uint32_t numberofwords)
{
	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;
	uint32_t StartPageAddress = IE2_SENSOR_CMD_START_ADDRESS;
	int sofar = 0;

	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();

	/* Erase the user Flash area*/
	uint32_t StartPage = internal_flash_get_page(StartPageAddress);
	uint32_t EndPageAdress = StartPageAddress + numberofwords*4;
	uint32_t EndPage = internal_flash_get_page(EndPageAdress);

	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = StartPage;
	EraseInitStruct.NbPages     = ((EndPage - StartPage)/FLASH_PAGE_SIZE) +1;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK){
	/*Error occurred while page erase.*/
		return HAL_FLASH_GetError();
	}

	/* Program the user Flash area word by word*/
	while (sofar < numberofwords){
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, StartPageAddress, Data[sofar]) == HAL_OK){
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

	return 0;
}

void internal_flash_read_sensor()
{

	uart_debug_print("\r\n++++++++++START SENSORS_LIST++++++++++\r\n");

	uint32_t received_data[130];
	uint8_t received_data_index = 0;
	uint32_t StartPageAddress = IE2_SENSOR_CMD_START_ADDRESS;
	char string_data[512];

//	char bf[100];

	while (1){
		HAL_IWDG_Refresh(&hiwdg);

		received_data[received_data_index] = *(__IO uint32_t *)StartPageAddress;
		StartPageAddress += 4;

//		sprintf(bf, "received_data[%d]: %08lX\r\n", received_data_index, received_data[received_data_index]);
//		uart_debug_print(bf);

		if (received_data[received_data_index] == 0xFFFFFFFF || received_data[received_data_index] == 0x00000000){
			df_byte_string(received_data, string_data);
//			const char s[2] = ",";

			char* sensor_token = strtok((char*)string_data, ",");
			modbus_num = strtol(sensor_token, NULL, 10);
			sensor_token = strtok(NULL, ",");

			char bf[32];
//			int seq = 0;
//			for(; sensor_token != NULL; seq++){
			for(int seq = 0; seq < modbus_num; seq++){
				HAL_IWDG_Refresh(&hiwdg);

//				uart_debug_print("sensor_token >> ");
//				uart_debug_print(sensor_token);
//				uart_debug_print("\r\n");

				strncpy((char*)modbus_str_pack.send[seq], sensor_token, strlen(sensor_token));
				modbus_str_pack.send[seq][strlen(sensor_token)] = '\0';

				sprintf(bf, "modbus_sensor.send[%.2d] >> ", seq);
				strcat(bf, (char*)modbus_str_pack.send[seq]);
				strcat(bf, "\r\n");
				uart_debug_print(bf);

				sensor_token = strtok(NULL, ",");
			}
//			modbus_str_pack.send[seq][Len_Max_Sen_Send] = '\0';

			break;
		}
		received_data_index++;

//		uart_debug_print("checking!...\r\n");
//		HAL_Delay(50);
	}

	uart_debug_print("++++++++++END SENSORS_LIST++++++++++\r\n");

	return;
}
// #########################################################################################################################
uint32_t internal_flash_write_metadata()
{
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;
	uint32_t StartPageAddress = IE2_METADATA_START_ADDRESS;
	int sofar = 0;

//  data pattern: [id],[DATE],[TIME],[Reset_Status],[Reset_Cause],[LOG_STATUS],[LOG_DIRECTORY],[DEFAULT_SENSORS],[wdt],[swt],[out],[iox_num],[iox_IDs],[FOTA_STATUS],[FOTA_SIZE]
//	sample data: "00,24/02/06,15:27:25+14,00,00,00,00,00,00000007,00000012,02,03,05,02,15,01,XXXXXXXX";
//	default data: "00,,00,00,00,00,00,00000000,00000000,00,00,00,00000000";

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

		HAL_IWDG_Refresh(&hiwdg);
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

		HAL_IWDG_Refresh(&hiwdg);
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

		HAL_IWDG_Refresh(&hiwdg);
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

		HAL_IWDG_Refresh(&hiwdg);
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
		HAL_IWDG_Refresh(&hiwdg);
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

	uart_debug_print("metadata to write: \r\n");
	uart_debug_print(dd);
	uart_debug_print("\r\n");

	return 0;
}


void internal_flash_read_metadata()
{

//	uart_debug_print("\r\n++++++++++START METADATA+++++++++\r\n");

	uint32_t received_data[130];
	uint8_t received_data_index = 0;
	uint32_t StartPageAddress = IE2_METADATA_START_ADDRESS;
	char string_data[256];

	HAL_IWDG_Refresh(&hiwdg);

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

//		char* id_token = strtok(NULL, ",");
//		uart_debug_print("Board ID: ");
//		uart_debug_print(id_token);
//		uart_debug_print("\r\n");
		strtok(NULL, ",");

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

//		uart_debug_print("DateTime: ");
//		uart_debug_print((char*)qctl_clk_str);
//		uart_debug_print("\r\n");

		char bf[50];

		char* reset_status_token = strtok(NULL, ",");
		reset_status = strtoul(reset_status_token, NULL, 10);
//		sprintf(bf,"reset_status: %d\r\n", reset_status);
//		uart_debug_print(bf);

		char* reset_cause_token = strtok(NULL, ",");
		reset_cause = strtoul(reset_cause_token, NULL, 10);
//		sprintf(bf,"reset_cause: %d\r\n", reset_cause);
//		uart_debug_print(bf);

		char* log_status_token = strtok(NULL, ",");
		is_log_service_activated = strtoul(log_status_token, NULL, 10);
//		sprintf(bf,"is_log_service_activated: %d\r\n", is_log_service_activated);
//		uart_debug_print(bf);

//		char* log_directory_token = strtok(NULL, ",");
//		qctl_ftp_is_dir_exist = strtoul(log_directory_token, NULL, 10);
//		sprintf(bf,"qctl_ftp_is_dir_exist: %d\r\n", qctl_ftp_is_dir_exist);
//		uart_debug_print(bf);

		char* default_sen_token = strtok(NULL, ",");
		modbus_default_sen_stat = strtoul(default_sen_token, NULL, 10);
//		sprintf(bf,"modbus_default_sen_stat: %d\r\n", modbus_default_sen_stat);
//		uart_debug_print(bf);

		char* wdt_token = strtok(NULL, ",");
		cm_wtd_cnt = strtoul(wdt_token, NULL, 16);
//		sprintf(bf,"cm_wtd_cnt: %08lX\r\n", cm_wtd_cnt);
//		uart_debug_print(bf);

		char* swt_token = strtok(NULL, ",");
		cm_swt_cnt = strtoul(swt_token, NULL, 16);
		sprintf(bf,"cm_swt_cnt: %08lX\r\n", cm_swt_cnt);
		uart_debug_print(bf);

		char* io_enable_token = strtok(NULL, ",");
		io_enable = strtoul(io_enable_token, NULL, 10);

		char* out_token = strtok(NULL, ",");
		outs_num = strtoul(out_token, NULL, 10);
//		sprintf(bf,"outs_num: %d\r\n", outs_num);
//		uart_debug_print(bf);

		char* iox_token = strtok(NULL, ",");
		iox_num = strtoul(iox_token, NULL, 10);
//		sprintf(bf,"iox_num: %d ", iox_num);
//		uart_debug_print(bf);
		if(iox_num){
			uart_debug_print(" >> IDs: ");

			for (uint8_t i = 0; i < iox_num; i++) {
				HAL_IWDG_Refresh(&hiwdg);
				char* id = strtok(NULL, ",");
				iox_id_array[i] = strtoul(id, NULL, 10);
//				sprintf(bf,"%d ", iox_id_array[i]);
//				uart_debug_print(bf);
//				if(i != 2){
//					uart_debug_print(" - ");
//				}
			}
		}
//		uart_debug_print("\r\n");

		char* fota_status_token = strtok(NULL, ",");
		flash_fota_flag = strtoul(fota_status_token, NULL, 10);
//		sprintf(bf,"flash_fota_flag: %d\r\n", flash_fota_flag);
//		uart_debug_print(bf);

		char* fota_size_token = strtok(NULL, ",");
		server_fota_App_size = strtoul(fota_size_token, NULL, 16);
//		sprintf(bf,"server_fota_App_size: %08lX\r\n", server_fota_App_size);
//		uart_debug_print(bf);

		uart_debug_print("\r\n");

		//[EMQ_IP],[EMQ_PORT],[EMQ_PASS],[FTP_ADMIN_IP],[FTP_ADMIN_PORT],[FTP_ADMIN_USERNAME],[FTP_ADMIN_PASS]
		char* emq_ip_token = strtok(NULL, ",");
		strncpy(emq_server_ip, emq_ip_token, strlen(emq_ip_token));
		emq_server_ip[strlen(emq_ip_token)]='\0';
//		sprintf(bf,"emq_server_ip:\t\t");
//		strcat(bf, emq_server_ip);
//		uart_debug_print(bf);
//		uart_debug_print("\r\n");


		char* emq_port_token = strtok(NULL, ",");
		strncpy(emq_server_port, emq_port_token, strlen(emq_port_token));
		emq_server_port[strlen(emq_port_token)]='\0';
//		sprintf(bf,"emq_server_port:\t");
//		strcat(bf, emq_server_port);
//		uart_debug_print(bf);
//		uart_debug_print("\r\n");

		char* emq_pass_token = strtok(NULL, ",");
		strncpy(emq_server_pass, emq_pass_token, strlen(emq_pass_token));
		emq_server_pass[strlen(emq_pass_token)]='\0';
//		sprintf(bf,"emq_server_pass:\t");
//		strcat(bf, emq_server_pass);
//		uart_debug_print(bf);
//		uart_debug_print("\r\n");

		char* log_server_ip_token = strtok(NULL, ",");
		strncpy(log_server_ip, log_server_ip_token, strlen(log_server_ip_token));
		log_server_ip[strlen(log_server_ip_token)]='\0';
//		sprintf(bf,"log_server_ip:\t\t");
//		strcat(bf, log_server_ip);
//		uart_debug_print(bf);
//		uart_debug_print("\r\n");

		char* log_server_port_token = strtok(NULL, ",");
		strncpy(log_server_port, log_server_port_token, strlen(log_server_port_token));
		log_server_port[strlen(log_server_port_token)]='\0';
//		sprintf(bf,"log_server_port:\t");
//		strcat(bf, log_server_port);
//		uart_debug_print(bf);
//		uart_debug_print("\r\n");

		char* log_server_username_token = strtok(NULL, ",");
		strncpy(log_server_user, log_server_username_token, strlen(log_server_username_token));
		log_server_user[strlen(log_server_username_token)]='\0';
//		sprintf(bf,"log_server_user:\t");
//		strcat(bf, log_server_user);
//		uart_debug_print(bf);
//		uart_debug_print("\r\n");

		char* log_server_pass_token = strtok(NULL, ",");
		strncpy(log_server_pass, log_server_pass_token, strlen(log_server_pass_token));
		log_server_pass[strlen(log_server_pass_token)]='\0';
//		sprintf(bf,"log_server_pass:\t");
//		strcat(bf, log_server_pass);
//		uart_debug_print(bf);
//		uart_debug_print("\r\n");

	}
	else{
		// invalid data in flash memory
		uart_debug_print("internal_flash_read_metadata >> No Valid Data found...writing default values...\r\n");
		internal_flash_write_metadata();
	}

//	uart_debug_print("\r\n++++++++++END METADATA++++++++++\r\n");

	return;
}
// #######################################################################################################################

/* [] END OF FILE */

