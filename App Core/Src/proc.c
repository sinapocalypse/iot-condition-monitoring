/*
 * proc.c
 *
 *  Created on: Jul 22, 2023
 *      Author: IroTeam
 */
#include "proc.h"

uint16_t line_num = 0;
uint16_t sht3_err_counter = 0;
uint16_t modbus_err_counter = 0;

unsigned int data_length = 0;
uint8_t fota_data_bytes[16 * FOTA_MAX_FILE_LINE];
uint16_t fota_data_index = 0;
int	data_index = 0;

uint8_t global_flag = 0;
uint16_t global_counter = 0;

// ###################################################### Init Functions ###################################################
void proc_init()
{
    df_init_vars_all();

	internal_flash_read_metadata();

//	proc_file_init();

	proc_wdt();

    proc_init_modbus();

    uart_debug_print("\r\n ############################ Configurations Set ############################\r\n");

//    proc_init_log_file();

    internal_flash_write_metadata();

    led_set_color(RED);

    return;
}

void proc_file_init()
{
    file_init();

    return;
}

void proc_wdt()
{

//	if(ee2_read_wdt() == 1 && ee2_read_swt() == 1){

		uint8_t res = cm_get_last_wtd();

		if(res == 1){
			cm_wtd_cnt++;

			char bf[100];
			sprintf(bf, "proc_wdt: watchdog counter = %lu \r\n", cm_wtd_cnt);
			uart_debug_print(bf);

//			ee2_write_wdt();
		}
		else if(res == 2){
			cm_swt_cnt++;

			char bf[100];
			sprintf(bf, "proc_swt: software counter = %lu \r\n", cm_swt_cnt);
			uart_debug_print(bf);

//			ee2_write_swt();
		}
//	}
	global_internal_flash_flag = 1;

    return;
}

void proc_init_modbus()
{
    //"A420400000001","A420400010001" example without LRC
//	if(ee2_read_default_sensor_status() == 1){
		if(modbus_default_sen_stat == 0){
			modbus_default_sen_stat = 1;
			internal_flash_write_metadata();
//			ee2_write_default_sensor_status();
			proc_init_default_sensor();
		}
		else{
			uart_debug_print("\r\nproc_init_modbus: already default sensors has been written!\r\n");
		}
//	}

	internal_flash_read_sensor();

    return;
}

void proc_init_default_sensor()
{
	modbus_num = 4;

	sprintf((char*)modbus_str_pack.send[0], "A820400000001");
	modbus_str_pack.send[0][13] = '\0';


	sprintf((char*)modbus_str_pack.send[1], "A820400010001");
	modbus_str_pack.send[1][13] = '\0';


	sprintf((char*)modbus_str_pack.send[2], "AA20400000001");
	modbus_str_pack.send[2][13] = '\0';


	sprintf((char*)modbus_str_pack.send[3], "AA20400010001");
	modbus_str_pack.send[3][13] = '\0';

	///////////////////////////////////////////////////////////////////////
	const char sensor_data[] = "04,A820400000001,A820400010001,AA20400000001,AA20400010001";

	uint32_t numofwords = (strlen(sensor_data) / 4) + ((strlen(sensor_data) % 4) != 0);
	internal_flash_write_sensor((uint32_t *)(sensor_data), numofwords);

	return;
}

void proc_init_log_file()
{
	/*
	if(!is_log_service_activated){
		char sen_num_bf[6];
		sprintf(sen_num_bf, "%02u", modbus_num);
		char iox_id_bf[32];
		sprintf(iox_id_bf, "[%02u, %02u, %02u]", iox_id_array[0], iox_id_array[1], iox_id_array[2]);

		char bf[256];
		sprintf(bf, "board id = %d", MAIN_ID);
		file_fill_log_buff(bf);

		sprintf(bf, "code version: ");
		strcat(bf, (char*)VERSION);
		file_fill_log_buff(bf);

		sprintf(bf, "number of sensors: ");
		strcat(bf, sen_num_bf);
		file_fill_log_buff(bf);

		sprintf(bf, "io expanders IDs: ");
		strcat(bf, iox_id_bf);
		file_fill_log_buff(bf);
	}
	*/

	return;
}

// #########################################################################################################################
// #########################################################################################################################
// ###################################################### For DAS ##########################################################
// ------------------------------------------------------ COMMUNICATOR proc ------------------------------------------------------
void proc_task_io()
{
	// check Board IO & IO Expanders
	if(io_enable){
		io_proc();
	    cm_create_io_buffer();
	}

	// RTC ??
	// rtc_proc();

    // Creating and Storing Buffers
    cm_create_all_buffer();

    // FOTA if needed

    return;
}

void proc_thermal_sensor_1()
{
    // Create the handle for the sensor.
	handle.i2c_handle = &hi2c1;
	handle.device_address = SHT3X_I2C_DEVICE_ADDRESS_ADDR_PIN_HIGH;

	//	   Initialize sensor (tests connection by reading the status register).
	if (!sht3x_init(&handle)) {
	  sht3_err_counter++;
	  if(sht3_err_counter > SHT3_ERR_MAX_COUNTER){
		  sht3_err_counter = 0;
//		  file_fill_log_buff("SHT3x access failed.");
		  uart_debug_print("SHT3x << 1 >> access failed.\r\n");
	  }
	}
	else{
		if(!sht3x_read_temperature_and_humidity(&handle, &sht3x_temperature_1, &sht3x_humidity_1)){
			sht3_err_counter++;
			if(sht3_err_counter > SHT3_ERR_MAX_COUNTER){
				sht3_err_counter = 0;
//				file_fill_log_buff("cm_create_all_buffer: Error reading SHT3 Sensor.");
				uart_debug_print("cm_create_all_buffer: Error reading SHT3 << 1 >> Sensor.\r\n");
			}
			sht3x_temperature_1 = 0;
			sht3x_humidity_1 = 0;
		}
	}

	// create receive buffer for board's humidity and temperature i2c sensor
	if(modbus_num > 0){
		sprintf((char*)modbus_str_pack.recv[0], "02%04X", sht3x_temperature_1);
		modbus_str_pack.recv[0][6] = '\0';
		if(modbus_num > 1){
			sprintf((char*)modbus_str_pack.recv[1], "02%04X", sht3x_humidity_1);
			modbus_str_pack.recv[1][6] = '\0';
		}
		sht3x_temperature_1 = 0;
		sht3x_humidity_1 = 0;
	}

	return;
}

void proc_thermal_sensor_2()
{
    // Create the handle for the sensor.
	handle.i2c_handle = &hi2c1;
	handle.device_address = SHT3X_I2C_DEVICE_ADDRESS_ADDR_PIN_LOW;

	//	   Initialize sensor (tests connection by reading the status register).
	if (!sht3x_init(&handle)) {
	  sht3_err_counter++;
	  if(sht3_err_counter > SHT3_ERR_MAX_COUNTER){
		  sht3_err_counter = 0;
//		  file_fill_log_buff("SHT3x access failed.");
		  uart_debug_print("SHT3x << 2 >> access failed.\r\n");
	  }
	}
	else{
		if(!sht3x_read_temperature_and_humidity(&handle, &sht3x_temperature_2, &sht3x_humidity_2)){
			sht3_err_counter++;
			if(sht3_err_counter > SHT3_ERR_MAX_COUNTER){
				sht3_err_counter = 0;
//				file_fill_log_buff("cm_create_all_buffer: Error reading SHT3 Sensor.");
				uart_debug_print("cm_create_all_buffer: Error reading SHT3 << 2 >> Sensor.\r\n");
			}
			sht3x_temperature_2 = 0;
			sht3x_humidity_2 = 0;
		}
	}

	// create receive buffer for board's humidity and temperature i2c sensor
	if(modbus_num > 2){
		sprintf((char*)modbus_str_pack.recv[2], "02%04X", sht3x_temperature_2);
		modbus_str_pack.recv[2][6] = '\0';
		if(modbus_num > 3){
			sprintf((char*)modbus_str_pack.recv[3], "02%04X", sht3x_humidity_2);
			modbus_str_pack.recv[3][6] = '\0';
		}
		sht3x_temperature_2 = 0;
		sht3x_humidity_2 = 0;
	}

	return;
}

// ------------------------------------------------------ OTHER func ---------------------------------------------------------
void proc_reset_wdt(uint8_t reset)
{
    if(reset){
        cm_wtd_cnt = 0;
        cm_swt_cnt = 0;

        if(fota_flag_start == 0){
			global_internal_flash_flag = 1;
		}

        uart_debug_print(" *** proc_reset_wdt >> Watchdog and Software Reset!! ***\r\n");
//    	internal_flash_write_metadata();

//        ee2_write_swt();
//        ee2_write_wdt();
    }

    return;
}


void proc_reset_chip(uint8_t reset_code)
{
    HAL_IWDG_Refresh(&hiwdg);
//    OS_TASK_Suspend(&TCB_Task_Io);

    reset_cause = reset_code;
    global_internal_flash_flag = 1;

//    qctl_state = QUECTEL_STATE_CHECK_POWER;
//	qctl_state_power = QUECTEL_STATE_POWER_DOWN;

    manual_reset = 1;

    return;
}
// -------------------------------------------------------------------------------------------------------------------------
// #########################################################################################################################
// #########################################################################################################################
// #########################################################################################################################

// #########################################################################################################################
// #########################################################################################################################
// ###################################################### For Gateway ######################################################
// ------------------------------------------------------ OPERATOR proc --------------------------------------------------------
void proc_task_operator()
{
	operator_task_counter++;

	proc_task_operator_QueueSet();
	proc_task_operator_get_ino_or_all();

    return;
}

    void proc_task_operator_QueueSet()
    {
        void* ret = NULL;
        ret = q_get_message(&_Queue_Set);

        while(ret){
        	HAL_IWDG_Refresh(&hiwdg);

        	proc_task_operator_QueueSet_separate((char*)ret);
        	OS_QUEUE_Purge(&_Queue_Set);

            OS_TASK_Delay(TASK_DELAY_OPERATOR_MS);

            ret = NULL;
            ret = q_get_message(&_Queue_Set);
        }

        return;
    }
//-
//--
    //-
    //--
        void proc_task_operator_QueueSet_separate(char* msg)
        {
            // {---_---}:{"id":"--","data":{}}
            // {---_---}:{"id":"--","data":[]}
            // {SET_FOT}:
            // {FOT_FOT}:
            if(memcmp(msg, "{SET_FOT}:", 10) == 0){
                proc_task_operator_QueueSet_for_gateway(msg);
            }
            else if(memcmp(msg, "{FOT_FOT}:", 10) == 0){
                proc_task_operator_QueueSet_for_gateway(msg);
            }
            else if(strlen(msg) > 31){
                char sid[3];
                uint8_t id = 255;
                char* idloc = strstr(msg, "\"id\":\"");
                if((idloc) && (strlen(idloc) >= 6)){
                    sid[0] = idloc[6];
                    sid[1] = idloc[7];
                    sid[2] = '\0';

                    id = strtol(sid, NULL, 10);

                    if(id == MAIN_ID){
						//for gateway
//						uart_debug_print("send to gw\r\n", 0);
						proc_task_operator_QueueSet_for_gateway(msg);
					}
                    else{
                    	uart_debug_print("proc_exe_set_queue: id is wrong! --> ");
//                    	file_fill_log_buff(ret);
                        uart_debug_print(msg);
//                    	file_fill_log_buff(ret);
                        uart_debug_print("\r\n");

                        return;
                    }
                }
                else{
                	uart_debug_print("proc_exe_set_queue: wrong format: no id found! --> ");
//                	file_fill_log_buff("proc_exe_set_queue: wrong format: no id found! --> ");
                    uart_debug_print(msg);
//                    file_fill_log_buff(msg);
                    uart_debug_print("\r\n");

                    return;
                }
            }
            else{
            	uart_debug_print("proc_exe_set_queue: wrong length --> \r\n");
//				file_fill_log_buff("proc_exe_set_queue: wrong length -->");
                uart_debug_print(msg);
//                file_fill_log_buff(msg);
                uart_debug_print("\r\n");

                return;
            }

            return;
        }
//-
//--
    //-
    //--
        //-
        //--
            void proc_task_operator_QueueSet_for_gateway(char* msg)
            {
//            	uart_debug_print(" >>> RECEV: ");
//            	uart_debug_print(msg);
//            	uart_debug_print("\r\n");
                //from IroTeamZero_V2/<IMEI>/M/ALL --> {SET_ALL}:{"id":"00","data":{"set_all":"1"}}
            	if(memcmp((char*)&msg[0], "{SET_ALL}:", 10) == 0){
            		operator_task_counter = QUECTEL_GET_ALL_COUNTER + 10;
				}
            	//from IroTeamZero_V2/<IMEI>/M/LOG --> {SET_LOG}:{"id":"00","data":["1/0","[FTP_SERVER_IP]","[FTP_SERVER_PORT]","[FTP_USER]","[FTP_PASS]"]}
            	else if(memcmp((char*)&msg[0], "{SET_LOG}:", 10) == 0){
					char log[128];
					sprintf(log, "{\"id\":\"%02d\",\"data\":[", (unsigned int)MAIN_ID);
					if((memcmp((char*)&msg[10], log, 19) == 0)){
						proc_update_log_status((char*)&msg[29]);
					}
				}
            	//from IroTeamZero_V2/<IMEI>/E/INO --> {SET_INO}:{"id":"00","data":{"set_ino":"1"}}
				else if((memcmp((char*)&msg[0], "{SET_INO}:", 10) == 0)){
					io_set_enable((char*)&msg[40]);
				}
                //from IroTeamZero_V2/<IMEI>/E/Dout --> {SET_OUT}:{"id":"--","data":{"out":"--------"}}
            	else if((memcmp((char*)&msg[0], "{SET_OUT}:", 10) == 0)){
                    io_set_out((char*)&msg[10]);
                }
                //from IroTeamZero_V2/<IMEI>/E/Sensor --> {SET_SEN}:{"id":"--","data":["--","sssssss","ssssssssssssss"]}
                else if(memcmp((char*)&msg[0], "{SET_SEN}:", 10) == 0){
                    modbus_set_sen((char*)&msg[10]);
                }
                //from IroTeamZero_V2/<IMEI>/E/Reset --> {SET_RST}:{"id":"--","data":{"cyp_rst":"-"}}
                else if((memcmp((char*)&msg[0], "{SET_RST}:", 10) == 0) && (strlen(msg) == Str_Len_Set_Rst)){
                    //check id data format
                    char bf[70];
                    sprintf(bf,"{SET_RST}:{\"id\":\"%02u\",\"data\":{\"stm_rst\":\"1\"}}}", (unsigned int)MAIN_ID);
                    if(memcmp((char*)&msg[0], bf, Str_Len_Set_Rst) == 0){
                        proc_reset_chip(Reset_Code_Server);
                    }
                }
                //from IroTeamZero_V2/<IMEI>/E/Wdt --> {SET_WDT}:{"id":"--","data":{"cyp_wdt":"-"}}
                else if((memcmp((char*)&msg[0], "{SET_WDT}:", 10) == 0) && (strlen(msg) == Str_Len_Set_Wdt)){
                    //check id data format
                    char bf[70];
                    sprintf(bf,"{SET_WDT}:{\"id\":\"%02u\",\"data\":{\"wdt_rst\":\"1\"}}", (unsigned int)MAIN_ID);
                    if(memcmp((char*)&msg[0], bf, Str_Len_Set_Wdt) == 0){
                        proc_reset_wdt(1);
                    }
                }
                //from IroTeamZero_V2/<IMEI>/E/IOX --> {SET_IOX}:{"id":"--","data":["--","--","--",...]}
				else if(memcmp((char*)&msg[0], "{SET_IOX}:", 10) == 0){
					io_set_iox_id((char*)&msg[10]);
				}
                //from IroTeamZero_V2/<IMEI>/M/FOTA --> {SET_FOT}:{"id":"00","data":["-","???","--","--","--------"]}
                else if((memcmp((char*)&msg[0], "{SET_FOT}:", 10) == 0)){
                    char fota[128];
                    sprintf(fota, "{SET_FOT}:{\"id\":\"%02d\",\"data\":[", (unsigned int)MAIN_ID);
                    if((memcmp((char*)&msg[0], fota, 29) == 0)){
                        proc_start_fota(1, &((char*)msg)[29]);
                    }
                }
                //from Interrupt
                else if((memcmp((char*)&msg[0], "{FOT_FOT}:", 10) == 0)){
                	qctl_fota_procedure(&((char*)msg)[10]);
                }
                else{

                }

                return;
            }
//-
//--
    //-
    //--
        //-
        //--
//-
//--
void proc_task_operator_get_ino_or_all()
{
	if(io_enable){
		proc_task_operator_get_ino_for_gateway();
//		OS_TASK_Delay(TASK_DELAY_OPERATOR_MS);
	}

	if(qctl_mqtt_maintain_topics){
		qctl_mqtt_maintain_topics = 0;
		if(qctl_mqtt_maintain_topics_first){
			qctl_mqtt_stat();
//			OS_TASK_Delay(TASK_DELAY_OPERATOR_MS);

			qctl_mqtt_will();
//			OS_TASK_Delay(TASK_DELAY_OPERATOR_MS);
		}
		else{
			qctl_mqtt_maintain_topics_first = 1;
			qctl_mqtt_stat();
		}
	}

	if(fota_stat){
		fota_stat = 0;
		proc_task_operator_get_fota_stat_for_gateway();
//		OS_TASK_Delay(TASK_DELAY_OPERATOR_MS);
	}

	if(operator_task_counter >= QUECTEL_GET_ALL_COUNTER){        // FYI //former condition: Gw_operator_task_counter % QUECTEL_GET_ALL_COUNTER == 0
		operator_task_counter = 0;
		//get all
		//Gateway

		proc_task_operator_get_all_for_gateway();
//        OS_TASK_Delay(TASK_DELAY_OPERATOR_MS);

	}


    return;
}

void proc_task_operator_get_all_for_gateway()
{
//	uart_debug_print("proc_task_operator_get_all_for_gateway >> ");
	strcpy((char*)io_buff_sent, (char*)io_buff_curr);
	uint16_t len = Str_Len_Put_Get_All + strlen((char*)all_buffer);
	if(io_enable){
		len += strlen((char*)io_buff_sent);
	}
	else{
		len += strlen("\"out\":\"\",\"inp\":\"\",");
	}
	char bf[len];
//	memset(bf, '\0', sizeof(bf));
	//            char s[100];
	//            sprintf(s, "len = %d\r\n", len);
	//            uart_debug_print(s);
	sprintf(bf, "{GET_ALL}:{\"id\":\"%02u\",\"data\":{", (unsigned int)MAIN_ID);
	if(io_enable){
		strcat(bf, (char*)io_buff_sent);
		strcat(bf, ",");
	}
	else{
		strcat(bf,"\"out\":\"\",\"inp\":\"\",");
	}
	strcat(bf, (char*)all_buffer);
	strcat(bf, "}}");

//	uart_debug_print("BF >> ");
//	uart_debug_print(bf);
//	uart_debug_print("\r\n");

//	bf[len] = '\0'; // FYI: Added _ 2023/10/07

	//sprintf(bf, "{GET_ALL}:{\"id\":\"%02u\",\"data\":{%s,%s}}", MAIN_ID, (char*)Das_io_buff_sent, (char*)Das_all_buffer);
	//            uart_debug_print("GET_ALL_BUFFER >> ");
	//            uart_debug_print(bf);
	//            uart_debug_print("\r\n");
	if(q_put_message(&_Queue_Get, bf, len)){
		uart_debug_print("proc_task_operator_get_all_for_gateway >> _Queue_Get >> put >> {GET_ALL}\r\n");
	}
	else{
		uart_debug_print("proc_task_operator_get_all_for_gateway >> failed to put to _Queue_Get >> ");
		uart_debug_print(bf);
		uart_debug_print("\r\n");
	}

	return;
}


void proc_task_operator_get_ino_for_gateway()
{
	if(memcmp(io_buff_sent, io_buff_curr, strlen((const char*)io_buff_curr)) != 0){
		strcpy((char*)io_buff_sent, (char*)io_buff_curr);
		uint16_t len = Str_Len_Put_Get_Ino + strlen((char*)io_buff_sent);
		char bf[len];
		sprintf(bf, "{GET_INO}:{\"id\":\"%02u\",\"data\":{", (unsigned int)MAIN_ID);
		strcat(bf, (char*)io_buff_sent);
		strcat(bf, "}}");
		//sprintf(bf, "{GET_INO}:{\"id\":\"%02u\",\"data\":{%s}}", MAIN_ID, (char*)Das_io_buff_sent); // 31 + 33

//		bf[len] = '\0'; // FYI: Added _ 2023/10/07

		if(q_put_message(&_Queue_Get, bf, len)){
			uart_debug_print("proc_task_operator_get_ino_for_gateway >> _Queue_Get >> put >> {GET_INO}\r\n");
		}
		else{
			uart_debug_print("proc_task_operator_get_ino_for_gateway >> failed to put to _Queue_Get >> ");
			uart_debug_print(bf);
			uart_debug_print("\r\n");
		}
	}

	return;
}

void proc_task_operator_get_fota_stat_for_gateway()
{
	char bf[Str_Len_Put_Get_Fot];
	sprintf(bf, "{GET_FOT}:{");
	strcat(bf, fota_topic_buffer);
	strcat(bf, "}");

	if(q_put_message(&_Queue_Get, bf, strlen(bf) + 1)){
		uart_debug_print("proc_task_operator_get_fota_stat_for_gateway >> _Queue_Get >> put >> {GET_FOT}\r\n");
	}
	else{
		uart_debug_print("proc_task_operator_get_fota_stat_for_gateway >> failed to put to _Queue_Get >> ");
		uart_debug_print(bf);
		uart_debug_print("\r\n");
	}

	memset(fota_topic_buffer, '\0', sizeof(fota_topic_buffer));

	return;
}

void proc_qctl_sep()
{
	void* ret = NULL;
	ret = q_get_message(&_Queue_Quectel);
	while(ret){
		HAL_IWDG_Refresh(&hiwdg);

		proc_separate_gsm_resp((char*)ret, strlen(ret) + 1);
		OS_QUEUE_Purge(&_Queue_Quectel);

		OS_TIMER_Restart(&osTimer_reset_state); //reset UART1
		ret = NULL;
		ret = q_get_message(&_Queue_Quectel);
	}

	return;
}

void proc_separate_fota_resp(char* data)
{
    if(data){
    	char fota_buff[10 + (FOTA_ROW_SIZE * FOTA_MAX_FILE_LINE) + 5];	//{FOT_FOT}:.....[dota data byte].....<NUL>

    	sprintf(fota_buff, "{FOT_FOT}:");
    	strcat(fota_buff, data);
    	fota_buff[10 + strlen(data)] = '\0';


    	if(q_put_message(&_Queue_Set, fota_buff, strlen(fota_buff) + 1)){
//			uart_debug_print("proc_separate_fota_resp: _Queue_Set >> put >> {FOT_FOT}\r\n");
	    	qctl_ftp_state = FTP_STATE_IDLE;  // CHANGE FTP_STATE to IDLE
		}
    	else{
    		uart_debug_print("proc_separate_fota_resp >> failed to put to _Queue_Set\r\n");
    		qctl_ftp_state = FTP_STATE_RAM_DOWNLOAD;
    	}

    }

    return;
}

void proc_separate_gsm_resp(char* data, uint16_t lenwith0)
{
	if(memcmp(data, "+CMT: ", 6) == 0){
		uart_debug_print("\r\nsms received\r\n");
		uart_debug_print("-----\r\n");
	}
	else if(memcmp(data, "\r\nCONNECT\r\n", 11) == 0 && fota_flag_start){
		uart_debug_print("\r\npkg received\r\n");
		uart_debug_print("-----\r\n");
		if(!q_put_message(&_Queue_Sep, data, lenwith0)){
			uart_debug_print("proc_separate_gsm_resp >> failed to put to _Queue_Sep\r\n");
			uart_debug_print(data);
			uart_debug_print("\r\n");
		}
	}
	else{
		uart_debug_print(data);
		uart_debug_print("-----\r\n");
		char* token = NULL;
		token = strtok(data, "\r\n");
		while(token){
			if(!q_put_message(&_Queue_Sep, token, strlen(token) + 1)){
				uart_debug_print("proc_separate_gsm_resp >> failed to put to _Queue_Sep\r\n");
				uart_debug_print(token);
				uart_debug_print("\r\n");
			}
			token = strtok(NULL, "\r\n");
		}
	}

	return;
}

// ------------------------------------------------------ SENSOR proc -----------------------------------------------------

void proc_task_io_sensor()
{
	cm_get_chip_temp();

	// Reading Temperature & Humidity Sensors Data
	proc_thermal_sensor_1();
	OS_TASK_Delay(50);
	proc_thermal_sensor_2();

	// Reading ASCII/RTU Sensors Data
	if(modbus_num > 4){
		for(uint8_t i = 4; i < modbus_num; i++){
			HAL_IWDG_Refresh(&hiwdg);

			proc_task_io();
			OS_TASK_Delay(50);

			if((modbus_curr != 255) && (modbus_curr_state != 255) && (modbus_curr_state != 2) && (modbus_curr_state == 1)){
				memset(modbus_str_pack.recv[modbus_curr], '\0', sizeof(modbus_str_pack.recv[modbus_curr]));
			}
			modbus_curr = i;
			modbus_curr_state = 1;
			modbus_proc(i);
			OS_TASK_Delay(100);
		}
	}
	else{
		proc_task_io();
	}

	return;
}
// ------------------------------------------------------ QUECTEL proc -----------------------------------------------------
void proc_task_quectel()
{
	if(qctl_debug_state_flag){
		qctl_debug_state_flag = 0;
		// benevis
		char bf[100];
		sprintf(bf, "qctl_state = %d, qctl_mqtt_state = %d\r\n", qctl_state, qctl_state_mqtt);
		uart_debug_print(bf);
	}

    proc_qctl_get();
    proc_qctl_set();

    if(global_flag){
    	global_counter++;
    	if(global_counter > ((20 * 1000) / TASK_DELAY_QUECTEL_MS)){
    		global_flag = 0;
    		global_counter = 0;

    		// set baudrate to fix 57600
    		qctl_send_cmd("AT+IPR=57600;&W", 1, 300, 1);

    		proc_reset_chip(Reset_Code_FOTA_Done);
    	}
    }

    if(global_internal_flash_flag == 1){
    	global_internal_flash_flag = 0;
    	internal_flash_write_metadata();

        if(manual_reset == 1){
        	df_delay_ms(6, 250);
        	NVIC_SystemReset();
            manual_reset = 0;
        }
    }

    return;
}

void proc_task_sep()
{
	proc_qctl_sep();

    return;
}

void proc_update_log_status(char* log_msg)
{
//	"{SET_LOG}:{"id":"00","data":["1/0","[FTP_SERVER_IP]","[FTP_SERVER_PORT]","[FTP_USER]","[FTP_PASS]"]}";

//	uart_debug_print("log_msg >> ");
//	uart_debug_print(log_msg);
//	uart_debug_print("\r\n");

	strremove(log_msg, "[");
	strremove(log_msg, "\"");
	strremove(log_msg, "]");
	strremove(log_msg, "}");

	char* ret = NULL;

	ret = strtok(log_msg, ",");
	is_log_service_activated = strtol(ret, NULL, 16);
//	uart_debug_print("log_msg >> is_log_service_activated >> ");
//	uart_debug_print(ret);
//	uart_debug_print("\r\n");

	if(is_log_service_activated == 1){

		memset(log_server_ip,'\0',sizeof(log_server_ip));
		memset(log_server_port,'\0',sizeof(log_server_port));
		memset(log_server_user,'\0',sizeof(log_server_user));
		memset(log_server_pass,'\0',sizeof(log_server_pass));

		ret = strtok(NULL, ",");
		strncpy(log_server_ip, ret,strlen(ret));

		uart_debug_print(log_server_ip);
		uart_debug_print("\r\n");

		ret = strtok(NULL, ",");
		strncpy(log_server_port, ret,strlen(ret));

		uart_debug_print(log_server_port);
		uart_debug_print("\r\n");

		ret = strtok(NULL, ",");
		strncpy(log_server_user, ret,strlen(ret));

		uart_debug_print(log_server_user);
		uart_debug_print("\r\n");

		ret = strtok(NULL, ",");
		strncpy(log_server_pass, ret,strlen(ret));

		uart_debug_print(log_server_pass);
		uart_debug_print("\r\n");

		uart_debug_print("\r\n *** LOG Status Activated! *** \r\n");

		if(fota_flag_start == 0){
			global_internal_flash_flag = 1;
		}
	}
	else if(is_log_service_activated == 0){
		uart_debug_print("\r\n *** LOG Status Deactivated! *** \r\n");
		is_log_service_activated = 0;

		if(fota_flag_start == 0){
			global_internal_flash_flag = 1;
		}
	}
	else{
//		file_fill_log_buff("proc_update_log_status >> invalid flag for updating the log status!");
		uart_debug_print("proc_update_log_status >> invalid flag for updating the log status!\r\n");
	}

	return;
}

// -------------------------------------------------------------------------------------------------------------------------

void proc_start_fota(uint8_t fota_flag, char* fota_data)
{
//    [fota],[folder_name],[number_of_file],[number_of_all_lines],[APP/BOOT_START_ADDRESS],[FTP_IP],[FTP_PORT],[FTP_USER],[FTP_PASS]
//    ["-","------------","---","---","--------"]}
	// get FOTA command from server and parse it
	strremove(fota_data, "[");
	strremove(fota_data, "\"");
	strremove(fota_data, "]");
	strremove(fota_data, "}");

	char* ret = NULL;

	ret = strtok(fota_data, ",");
	fota_flag_start = strtol(ret, NULL, 16);

    if(fota_flag_start == 1){

        ret = strtok(NULL, ",");

        if(strlen(ret) <= sizeof(server_fota_folder_name)){
            strcpy(server_fota_folder_name, ret);
        }
        else{
//			file_fill_log_buff("#### Note: FOTA Folder name not valid!!! ####");
			uart_debug_print("#### Note: FOTA Folder name not valid!!! ####\r\n");
			qctl_mqtt_fota_stat((char*)TOPIC_FOTA_FOLDER_LEN_FAILED);

			qctl_ftp_state = FTP_STATE_RESET;

			return;
        }

        ret = strtok(NULL, ",");
        server_fota_file_numbers = strtol(ret, NULL, 10);

        ret = strtok(NULL, ",");
        server_fota_App_size = strtoul(ret, NULL, 10);

        ret = strtok(NULL, ",");

        server_fota_App_Address = strtoul(ret, NULL, 16);

        ret = strtok(NULL, ",");

        ftp_server_ip = ret;

        ret = strtok(NULL, ",");

		ftp_server_port = ret;

        ret = strtok(NULL, ",");

        ftp_server_user = ret;

        ret = strtok(NULL, ",");

		ftp_server_pass = ret;

        if(fota_flag_start &&
        		server_fota_file_numbers &&
				server_fota_App_size &&
				server_fota_App_Address &&
				ftp_server_ip &&
				ftp_server_port &&
				ftp_server_user &&
				ftp_server_pass){
			// uncomment this if FOTA files are already written in Quectel RAM
//        		qctl_ftp_state = FTP_STATE_RAM_DOWNLOAD; // optional

			// ORIGINAL CODE LINE
        	qctl_ftp_state = FTP_STATE_CONFIG;
			qctl_ftp_config_state = FTP_STATE_CONFIG_STATE_RAM_CLEAR;
			qctl_state = QUECTEL_STATE_MQTT_CONNECTION;

			if(qctl_state == QUECTEL_STATE_MQTT_CONNECTION && qctl_state_mqtt > MQTT_STATE_PUBLISH && qctl_state_mqtt != MQTT_STATE_PUBLISHING){
				qctl_state_mqtt = MQTT_STATE_PUBLISH;
			}
			/////////////////////
			uart_debug_print("\r\n #### Note: FOTA Procedure Started!!! ####\r\n");

			qctl_mqtt_fota_stat((char*)TOPIC_FOTA_START);
        }
        else{
			qctl_mqtt_fota_stat((char*)TOPIC_FOTA_INVALID_CONFIG_FAILED);
//        	file_fill_log_buff("#### Note: FOTA Procedure FAILED!!! ####");
        	uart_debug_print("\r\n #### Note: FOTA Procedure FAILED!!! ####\r\n");

			qctl_ftp_state = FTP_STATE_RESET;
        }
    }
    else if(fota_flag_start == 2){
		qctl_mqtt_fota_stat((char*)TOPIC_FOTA_STOPPED);
    	uart_debug_print("\r\n #### Note: FOTA Procedure STOPPED!!! ####\r\n");

		qctl_ftp_state = FTP_STATE_CONNECTION;
		qctl_ftp_connection_state = FTP_STATE_CONNECTION_CLOSE;
    }

    return;
}


// -------------------------------------------------------------------------------------------------------------------------
// #########################################################################################################################
// #########################################################################################################################
// #########################################################################################################################
