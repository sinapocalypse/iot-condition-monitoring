#include "io.h"

uint8_t iox_id = 0;
uint8_t iox_id_array[16];

// ###################################################### Functions ########################################################
void io_init()
{
	for (uint8_t i = 0; i < 16; ++i) {
		HAL_IWDG_Refresh(&hiwdg);
		iox_id_array[i] = 0;
	}
}

void io_read_inputs()
{
	uint8_t inputs = 0;

	inputs |= ((uint8_t)HAL_GPIO_ReadPin(IN_1_GPIO_Port, IN_1_Pin)) << 7;
	inputs |= ((uint8_t)HAL_GPIO_ReadPin(IN_2_GPIO_Port, IN_2_Pin)) << 6;
	inputs |= ((uint8_t)HAL_GPIO_ReadPin(IN_3_GPIO_Port, IN_3_Pin)) << 5;
	inputs |= ((uint8_t)HAL_GPIO_ReadPin(IN_4_GPIO_Port, IN_4_Pin)) << 4;
	inputs |= ((uint8_t)HAL_GPIO_ReadPin(IN_5_GPIO_Port, IN_5_Pin)) << 3;
	inputs |= ((uint8_t)HAL_GPIO_ReadPin(IN_6_GPIO_Port, IN_6_Pin)) << 2;
	inputs |= ((uint8_t)HAL_GPIO_ReadPin(IN_7_GPIO_Port, IN_7_Pin)) << 1;
	inputs |= ((uint8_t)HAL_GPIO_ReadPin(IN_8_GPIO_Port, IN_8_Pin)) << 0;


	uint32_t inp = (((uint32_t)inputs) << 24);
	inp = ~(inp);
	Inps_Read = inp;

	return;
}

void io_read_outputs()
{
	uint16_t outputs = 0;

	uint8_t out1 = 0;
	out1 |= HAL_GPIO_ReadPin(OUT_1_GPIO_Port, OUT_1_Pin) << 7;
	out1 |= HAL_GPIO_ReadPin(OUT_2_GPIO_Port, OUT_2_Pin) << 6;
	out1 |= HAL_GPIO_ReadPin(OUT_3_GPIO_Port, OUT_3_Pin) << 5;
	out1 |= HAL_GPIO_ReadPin(OUT_4_GPIO_Port, OUT_4_Pin) << 4;
	out1 |= HAL_GPIO_ReadPin(OUT_5_GPIO_Port, OUT_5_Pin) << 3;
	out1 |= HAL_GPIO_ReadPin(OUT_6_GPIO_Port, OUT_6_Pin) << 2;
	out1 |= HAL_GPIO_ReadPin(OUT_7_GPIO_Port, OUT_7_Pin) << 1;
	out1 |= HAL_GPIO_ReadPin(OUT_8_GPIO_Port, OUT_8_Pin) << 0;

	outputs = ((uint16_t)(out1 << 8));

	uint8_t out2 = 0;

	out2 = HAL_GPIO_ReadPin(RELAY_1_GPIO_Port, RELAY_1_Pin) << 7;

	outputs |= ((uint16_t)(out2 << 0));

	Outs_Read = outputs;

    return;
}

void io_write_outputs()
{
	if(Out_Array_Old[0] != Out_Array[0] || Out_Array_Old[1] != Out_Array[1]){
		HAL_GPIO_WritePin(OUT_1_GPIO_Port, OUT_1_Pin, (Out_Array[0] & 0b10000000) >> 7);
		HAL_GPIO_WritePin(OUT_2_GPIO_Port, OUT_2_Pin, (Out_Array[0] & 0b01000000) >> 6);
		HAL_GPIO_WritePin(OUT_3_GPIO_Port, OUT_3_Pin, (Out_Array[0] & 0b00100000) >> 5);
		HAL_GPIO_WritePin(OUT_4_GPIO_Port, OUT_4_Pin, (Out_Array[0] & 0b00010000) >> 4);
		HAL_GPIO_WritePin(OUT_5_GPIO_Port, OUT_5_Pin, (Out_Array[0] & 0b00001000) >> 3);
		HAL_GPIO_WritePin(OUT_6_GPIO_Port, OUT_6_Pin, (Out_Array[0] & 0b00000100) >> 2);
		HAL_GPIO_WritePin(OUT_7_GPIO_Port, OUT_7_Pin, (Out_Array[0] & 0b00000010) >> 1);
		HAL_GPIO_WritePin(OUT_8_GPIO_Port, OUT_8_Pin, (Out_Array[0] & 0b00000001) >> 0);

		HAL_GPIO_WritePin(RELAY_1_GPIO_Port, RELAY_1_Pin, (Out_Array[1] & 0b10000000) >> 7);

		Out_Array_Old[0] = Out_Array[0];
		Out_Array_Old[1] = Out_Array[1];

		char buff[100];
		sprintf(buff, "io_set_out: new OUT: %02X%02X, old OUT: %02X%02X\r\n", Out_Array[0], Out_Array[1], Out_Array_Old[0], Out_Array_Old[1]);
		uart_debug_print(buff);
	}

    return;
}

void io_read_io_expanders()
{
    char header_buf[20];

    for(int i = 0; i < iox_num; i++){
    	HAL_IWDG_Refresh(&hiwdg);

        sprintf(header_buf, "%02u:{GET_INO}:", iox_id_array[i]);

        HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 1);		// set pin high to enable Transmitting
        HAL_UART_Transmit(&huart2, (const uint8_t*)header_buf, strlen((const char*)header_buf) + 1, 50);
       	HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 0);		// set pin low to enable Receiving

        df_delay_ms(1, 50);
    }

    return;
}

void io_write_io_expanders()
{
    if(outs_num > 16){
        uint8_t num = outs_num - 16;

        if(num > 0 && num <= 24 && iox_num >= 1
        		&& ((Out_Array[2] != Out_Array_Old[2])
				|| (Out_Array[3] != Out_Array_Old[3])
				|| (Out_Array[4] != Out_Array_Old[4]))){
            char header_buf[50];
            uint32_t output = 0;

            // FYI >>
            output = (Out_Array[2] << 16) | (Out_Array[3] << 8) | (Out_Array[4] << 0);

            sprintf(header_buf, "%02u:{SET_OUT}:{\"out\":\"%06lX\"}", iox_id_array[0], output);
            uart_debug_print("io_write_io_expanders__0 >> ");
            uart_debug_print(header_buf);

            HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 1);		// set pin high to enable Transmitting
            HAL_UART_Transmit(&huart2, (const uint8_t*)header_buf, strlen((const char*)header_buf) + 1, 50);
            HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 0);		// set pin low to enable Receiving

            Out_Array_Old[2] = Out_Array[2];
            Out_Array_Old[3] = Out_Array[3];
            Out_Array_Old[4] = Out_Array[4];
            df_delay_ms(2, 75);
        }
        else if(num > 24 && num <= 48 && iox_num >= 2
        		&& ((Out_Array[5] != Out_Array_Old[5])
				|| (Out_Array[6] != Out_Array_Old[6])
				|| (Out_Array[7] != Out_Array_Old[7]))){
            char header_buf[50];
            uint32_t output = 0;
            // FYI >>
			output = (Out_Array[5] << 16) | (Out_Array[6] << 8) | ((Out_Array[7] << 0));

			sprintf(header_buf, "%02u:{SET_OUT}:{\"out\":\"%06lX\"}", iox_id_array[1], output);
			uart_debug_print("io_write_io_expanders__1 >> ");
			uart_debug_print(header_buf);

            HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 1);		// set pin high to enable Transmitting
            HAL_UART_Transmit(&huart2, (const uint8_t*)header_buf, strlen((const char*)header_buf) + 1, 50);
            HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 0);		// set pin low to enable Receiving

			Out_Array_Old[5] = Out_Array[5];
			Out_Array_Old[6] = Out_Array[6];
			Out_Array_Old[7] = Out_Array[7];

            df_delay_ms(2, 75);
        }
        else if(num > 48 && num < 72 && iox_num >= 3
        		&& ((Out_Array[8] != Out_Array_Old[8])
				|| (Out_Array[9] != Out_Array_Old[9])
				|| (Out_Array[10] != Out_Array_Old[10]))){
            char header_buf[50];
            uint32_t output = 0;

            // FYI >>
			output = (Out_Array[8] << 16) | (Out_Array[9] << 8) | (Out_Array[10] << 0);

			sprintf(header_buf, "%02u:{SET_OUT}:{\"out\":\"%06lX\"}", iox_id_array[2], output);
			uart_debug_print("io_write_io_expanders__2 >> ");
			uart_debug_print(header_buf);

            HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 1);		// set pin high to enable Transmitting
            HAL_UART_Transmit(&huart2, (const uint8_t*)header_buf, strlen((const char*)header_buf) + 1, 50);
            HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 0);		// set pin low to enable Receiving


			Out_Array_Old[8] = Out_Array[8];
			Out_Array_Old[9] = Out_Array[9];
			Out_Array_Old[10] = Out_Array[10];

            df_delay_ms(2, 75);
        }
        else if(num > 72 && num < 96 && iox_num >= 4
        		&& ((Out_Array[11] != Out_Array_Old[11])
				|| (Out_Array[12] != Out_Array_Old[12])
				|| (Out_Array[13] != Out_Array_Old[13]))){
            char header_buf[50];

            uint32_t output = 0;
			output = (Out_Array[11] << 16) | (Out_Array[12] << 8) | (Out_Array[13] << 0);

			sprintf(header_buf, "%02u:{SET_OUT}:{\"out\":\"%06lX\"}", iox_id_array[3], output);
			uart_debug_print("io_write_io_expanders__3 >> ");
			uart_debug_print(header_buf);

            HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 1);		// set pin high to enable Transmitting
            HAL_UART_Transmit(&huart2, (const uint8_t*)header_buf, strlen((const char*)header_buf) + 1, 50);
            HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 0);		// set pin low to enable Receiving

			Out_Array_Old[11] = Out_Array[11];
			Out_Array_Old[12] = Out_Array[12];
			Out_Array_Old[13] = Out_Array[13];

            df_delay_ms(2, 75);
        }
        else if(num > 96 && num < 120 && iox_num >= 5
        		&& ((Out_Array[14] != Out_Array_Old[14])
				|| (Out_Array[15] != Out_Array_Old[15])
				|| (Out_Array[16] != Out_Array_Old[16]))){
            char header_buf[50];
            uint32_t output = 0;

            // FYI >>
			output = (Out_Array[14] << 16) | (Out_Array[15] << 8) | ((Out_Array[16] << 0));

			sprintf(header_buf, "%02u:{SET_OUT}:{\"out\":\"%06lX\"}", iox_id_array[4], output);
			uart_debug_print("io_write_io_expanders__4 >> ");
			uart_debug_print(header_buf);

            HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 1);		// set pin high to enable Transmitting
            HAL_UART_Transmit(&huart2, (const uint8_t*)header_buf, strlen((const char*)header_buf) + 1, 50);
            HAL_GPIO_WritePin(RS485_Sensor_RE_DE_GPIO_Port, RS485_Sensor_RE_DE_Pin, 0);		// set pin low to enable Receiving

			Out_Array_Old[14] = Out_Array[14];
			Out_Array_Old[15] = Out_Array[15];
			Out_Array_Old[16] = Out_Array[16];

            df_delay_ms(2, 75);
        }
    }

    return;
}

// #########################################################################################################################

// ###################################################### Main Functions ###################################################
void io_proc()
{
    io_write_outputs();
    io_write_io_expanders();

    io_read_outputs();
    io_read_inputs();
    io_read_io_expanders();

    return;
}

void io_set_iox_id(char* msg)
{
	char buf[100];
    //{"id":"00","data":["02","XX","15"]}
    char* res = strstr(msg, "["); // ["02","XX","15"]}
    strremove(res, "[");
    strremove(res, "\"");
    strremove(res, "]");
    strremove(res, "}");

    // 16,12,15,...,08 => 17 number
    char* token = NULL;
    uint8_t id = 0;

    token = strtok(res, ",");
    iox_num = strtol(token, NULL, 10);

    token = strtok(NULL,",");        // first id

//    memset(iox_id_array, '\0', sizeof(iox_id_array));

    int i = 0;
    while ((token) && (i < Max_IOX_NUMBER)) {
    	HAL_IWDG_Refresh(&hiwdg);

        id = strtol(token, NULL, 10);
        iox_id_array[i] = id;

        token = strtok(NULL,",");
        i++;
    }

    if(iox_num > Max_IOX_NUMBER){
		sprintf(buf, "iox_set_id_proc => Invalid number of IOX, Set to MAX Number: %d.\r\n", Max_IOX_NUMBER);
		uart_debug_print(buf);
		iox_num = Max_IOX_NUMBER;
	}
    else{
    	iox_num = i;
    }

    selectionSort(iox_id_array, iox_num);

    if(fota_flag_start == 0){
		global_internal_flash_flag = 1;
	}

    return;
}

void io_find_io_expander()
{
    uint8_t iox_id = strtol((char*)modbus_recv_pack.id, NULL, 10);

    for(int i = 0; i < iox_num; i++){
        if(iox_id == iox_id_array[i]){
            // --:{GET_INO}:{"inp":"------","out":"------"}<NUL>
            if(strlen((char*)modbus_recv_pack.buff) > 20){

                char* ret1 = NULL;
                ret1 = strstr((char*)modbus_recv_pack.buff, "}") + 1;

                strremove(ret1, "{");
                strremove(ret1, "\"");
                strremove(ret1, "}");

                char* ret2 = NULL;
                ret2 = strstr(ret1 + 1, ":");

                uint32_t iox_inp = 0;
                iox_inp = strtoul(ret2 + 1, NULL, 16);

                expander_1_Inps_Read[i] = (uint16_t)(iox_inp >> 8);
                expander_2_Inps_Read[i] = (uint8_t)(iox_inp >> 0);

                char* ret3 = NULL;
                ret3 = strstr(ret2 + 1, ":");

                uint32_t iox_out = 0;
                iox_out = strtoul(ret3 + 1, NULL, 16);

                expander_2_Outs_Read[i] = (uint8_t)(iox_out >> 0);
                expander_3_Outs_Read[i] = (uint16_t)(iox_out >> 8);

            }
            else{
//            	file_fill_log_buff("io_find_io_expander >> invalid IOX packet size!");
            }
        }
        else{
			expander_1_Inps_Read[i] = 0;
			expander_2_Inps_Read[i] = 0;
			expander_2_Outs_Read[i] = 0;
			expander_3_Outs_Read[i] = 0;
        }
    }

    return;
}
// #########################################################################################################################

// ###################################################### Sep Functions ####################################################
void io_set_enable(char* msg)
{

	if(msg[0] == '1'){
		uart_debug_print("io_set_enable >> *** ACTIVATED ***!\r\n");
		io_enable = 1;

	    if(fota_flag_start == 0){
	    	global_internal_flash_flag = 1;
	    }
	}
	else if(msg[0] == '0'){
		uart_debug_print("io_set_enable >> *** DEACTIVATED ***!\r\n");
		io_enable = 0;
		memset(io_buff_curr, '\0', sizeof(io_buff_curr));
		sprintf((char*)io_buff_curr, "\"out\":\"\",\"inp\":\"\"");
		proc_task_operator_get_ino_for_gateway();

	    if(fota_flag_start == 0){
	    	global_internal_flash_flag = 1;
	    }
	}
	else{
//		file_fill_log_buff("io_set_enable >> invalid flag for updating the IO status!");
		uart_debug_print("io_set_enable >> invalid flag for updating the IO status!\r\n");
	}

	return;
}
void io_set_out(char* msg)
{
    // {"id":"00","data":{"out":"1024FC32540019AA12B43D1AED3FBB1089"}}
    char* res = msg + 26;

    // res = 1024FC32540019AA12B43D1AED3FBB1089"}}
    strremove(res, "\"");
    strremove(res, "}");

    // res = 1024FC32540019AA12B43D1AED3FBB1089

    size_t length = strlen(res);
    char buf[100];
    sprintf(buf, "length: %d, data: ", length);
    strcat(buf, res);
    strcat(buf, "\r\n");
    uart_debug_print(buf);

    // res = 1024FC32540019AA12B43D1AED3FBB1089

    // result = 02 << 4 : 20

    char niblle_buf[2] = {'\0', '\0'};
    uint8_t tmp = 0;
    uint8_t j = 0;
    outs_num = length * 4; // write to eeprom

    if(length % 2 == 0){
        for(uint8_t i = 0; i < length; i++){
            if(i % 2 == 0){
                tmp = 0;
                niblle_buf[0] = res[i];
                tmp = (strtoul(niblle_buf, NULL, 16)) << 4;
            }
            else{
                niblle_buf[0] = res[i];
                tmp = tmp | (strtoul(niblle_buf, NULL, 16));
                Out_Array[j] = tmp;
                j++;
            }
        }
    }
    else{
        for(uint8_t i = 0; i < length; i++){
            if(i == length - 1){
                tmp = 0;
                niblle_buf[0] = res[i];
                tmp = (strtoul(niblle_buf, NULL, 16)) << 4;
                Out_Array[j] = tmp;
                j++;
            }
            else{
                if(i % 2 == 0){
                    tmp = 0;
                    niblle_buf[0] = res[i];
                    tmp = (strtoul(niblle_buf, NULL, 16)) << 4;
                }
                else{
                    niblle_buf[0] = res[i];
                    tmp = tmp | (strtoul(niblle_buf, NULL, 16));
                    Out_Array[j] = tmp;
                    j++;
                }
            }

        }
    }

    if(fota_flag_start == 0){
    	global_internal_flash_flag = 1;
    }

    return;
}

uint32_t io_hex_to_bin(char* hexString)
{
    uint32_t ret = 0;

    for(uint8_t i = 0; i < 8; i++){
    	HAL_IWDG_Refresh(&hiwdg);

        uint32_t byte = 0;
        byte = df_hexstring_hex(hexString[i]);
        if(byte == 255){
            byte = 0;
        }
        byte = byte << (4 * (7 - i));
        ret |= byte;
    }

    return ret;
}
