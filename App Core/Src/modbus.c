/*
 * modbus.c
 *
 *  Created on: Jul 22, 2023
 *      Author: IroTeam
 */
#include "modbus.h"

// ###################################################### Modbus Functions #################################################
void modbus_proc(uint8_t number)
{
	switch(modbus_str_pack.send[number][0]){
        case 'A':{
            modbus_send_ascii(modbus_str_pack.send[number]);
            break;
        }
        case 'R':{
            modbus_send_rtu(modbus_str_pack.send[number]);
            break;
        }
        default:{
            break;
        }
    }

    return;
}

void modbus_send_ascii(uint8_t send_modbus[Len_Max_Sen_Send + 1])
{
	uint8_t send[Len_Max_Sen_Send + 3  - 1 + 1]; // : \r \n   // -A or -R
    memset(send, '\0', sizeof(send));
    uint8_t j = 0;
    send[0] = ':';
    j++;

    uint8_t i = 1;
    while(send_modbus[i] != '\0'){
    	HAL_IWDG_Refresh(&hiwdg);

        send[j] = send_modbus[i];
        i++;
        j++;
    }

    send[j] = '\r';
    j++;

    send[j] = '\n';
    j++;

    HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 1);		// set pin high to enable Transmitting
	HAL_UART_Transmit(&huart2, send, j, 50);
	HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 0);		// set pin low to enable Receiving

    return;
}

void modbus_send_rtu(uint8_t send_modbus[Len_Max_Sen_Send + 1])
{
    uint8_t send[Len_Max_Sen_Send / 2 + 1];   // 13
    memset(send, '\0', sizeof(send));

    uint8_t i = 1;
    uint8_t j = 0;
    while(send_modbus[i] != '\0'){
    	HAL_IWDG_Refresh(&hiwdg);
        j++;
        i++;
    }

    uint8_t a = 0;
    if(j % 2 == 0){
        for(i = 0; i < j; i = i + 2){
            send[a] =  df_hexstring_hex(send_modbus[1 + i + 0]) * 16 +  df_hexstring_hex(send_modbus[1 + i + 1]);
            a++;
        }

        HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 1);		// set pin high to enable Transmitting
		HAL_UART_Transmit(&huart2, send, a, 50);
		HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 0);		// set pin low to enable Receiving
    }
    else{
        uart_debug_print("modbus_send_rtu: invalid sensor data\r\n");
    }

    return;
}
// #########################################################################################################################

// ###################################################### Functional Functions #############################################
void modbus_find_row(uint8_t type)
{
    switch(type){
        case 'A':{
            if(memcmp((char*)modbus_recv_pack.id, (char*)&modbus_str_pack.send[modbus_curr][1], 4) == 0){
                if(modbus_curr_state == 1){
                    modbus_curr_state = 2;
                }
                memset(modbus_str_pack.recv[modbus_curr], '\0', sizeof(modbus_str_pack.recv[modbus_curr]));
                for(uint8_t j = 5; j < (modbus_recv_pack.index - 4); j++){
                    modbus_str_pack.recv[modbus_curr][j - 5] = modbus_recv_pack.buff[j];

                    if(j == (modbus_recv_pack.index - 4) - 1){
                        modbus_str_pack.recv[modbus_curr][j + 1] = '\0';
//                        uart_debug_print((char*)modbus_str_pack.recv[modbus_curr], 0);
//                        uart_debug_print("\r\n", 0);
                        break;
                    }
                }

                break;
            }

            break;
        }
        case 'R':{
                if(memcmp((char*)modbus_recv_pack.id, (char*)&modbus_str_pack.send[modbus_curr][1], 4) == 0){
                    if(modbus_curr_state == 1){
                        modbus_curr_state = 2;
                    }
                    memset(modbus_str_pack.recv[modbus_curr], '\0', sizeof(modbus_str_pack.recv[modbus_curr]));
                    for(uint8_t j = 2; j < (modbus_recv_pack.index - 2); j++){
                        char data[3];
                        uint8_t t = (j - 2) * 2;
                        sprintf(data, "%02X", modbus_recv_pack.buff[j]);
                        modbus_str_pack.recv[modbus_curr][t] = data[0];
                        modbus_str_pack.recv[modbus_curr][t + 1] = data[1];

                        if(j == (modbus_recv_pack.index - 2)){
                            modbus_str_pack.recv[modbus_curr][j + 1] = '\0';
//                            uart_debug_print((char*)modbus_str_pack.recv[modbus_curr], 0);
//                            uart_debug_print("\r\n", 0);
                            break;
                        }
                    }

                    break;
                }

            break;
        }
        default:
            break;
    }

    return;
}

void modbus_set_sen(char* msg)
{
    // "{\"id\":\"--\",\"data\":[\""
    msg = &msg[20];

    msg = strremove(msg, "]");
    msg = strremove(msg, "}");
    msg = strremove(msg, "\"");

    char* flash_data = msg;
	flash_data[strlen(msg)] = ',';
	flash_data[strlen(msg)+1] = '\0';

    uint32_t numofwords = (strlen(flash_data) / 4) + ((strlen(flash_data) % 4) != 0);
    if(fota_flag_start == 0){
		internal_flash_write_sensor((uint32_t *)(flash_data), numofwords);
	}

    char* snum = NULL;
	snum = strtok(msg, ",");
	uint8_t num = strtol(snum, NULL, 10);

    char* token = strtok(NULL, ",");
    uint8_t i = 0;

    while((i < num) && (token) && (num != 0)){
    	HAL_IWDG_Refresh(&hiwdg);
    	uint8_t len = strlen(token);

        if(len > Len_Max_Sen_Send){
            len = Len_Max_Sen_Send;
        }

        uint16_t j = 0;
        for(; j < len; j++){
        	HAL_IWDG_Refresh(&hiwdg);
            modbus_str_pack.send[i][j] = token[j];
        }
        if(j == (Len_Max_Sen_Send - 1)){
            modbus_str_pack.send[i][Len_Max_Sen_Send] = '\0';
        }
        else{
            modbus_str_pack.send[i][j] = '\0';
        }

        i++;
        token = strtok(NULL, ",");
    }

    char buf[100];
    if(num > Num_Max_Sen){
		sprintf(buf, "modbus_set_sen >> Invalid number of Sensors, Set Numbers to %d.\r\n", i);
		uart_debug_print(buf);
	}

    num = i; // correct the numbers based on index iteration (i)

    modbus_num = num;


    flag_modbus_write = 1;




    sprintf(buf, " ***** %d new sensor(s) wrote *****\r\n", modbus_num);
    uart_debug_print(buf);

    return;
}

// #########################################################################################################################
