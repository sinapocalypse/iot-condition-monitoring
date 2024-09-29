/*
 * microchipMonitoring.c
 *
 *  Created on: Jul 22, 2023
 *      Author: IroTeam
 */


#include "microchipMonitoring.h"

uint16_t adc_read = 0;
float rawC = 0.0f;
float tempC = 0.0f;

float cm_temperature = 0;
uint32_t cm_wtd_cnt = 0;
uint32_t cm_swt_cnt = 0;

uint8_t reset_status = 0;
uint8_t reset_cause = 0;

char reset_cause_msg[128];

// ###################################################### Temperature Functions ############################################
void cm_get_chip_temp()
{
	if(adc_flag == 1){
		adc1 = Adc.Raw[0];
		adc2 = Adc.Raw[1];

		V_supply = ((VREF) * ((float)(ADCMAX)) / ((float)(adc1)));
		T_sense = ( ( V25 - V_supply / ADCMAX * adc2 ) / AVG_Slope) + 25;

		chip_temp = T_sense * 10.0f;

		Adc.Raw[0] = 0;
		Adc.Raw[1] = 0;

		adc_flag = 0;
	}

	return;
}
// #########################################################################################################################

// ###################################################### Watchdog Functions ###############################################

uint8_t cm_get_last_wtd()
{
	memset(reset_cause_msg, '\0', sizeof(reset_cause_msg));
    uint8_t ret = 0;
	sprintf(reset_cause_msg, "App Reset Cause >> ");
    switch(reset_status){
		case RESET_CAUSE_UNKNOWN:{
			strcat(reset_cause_msg, "RESET_CAUSE_UNKNOWN");
			break;
		}
		case RESET_CAUSE_LOW_POWER_RESET:{
			strcat(reset_cause_msg, "RESET_CAUSE_LOW_POWER_RESET");
			break;
		}
		case RESET_CAUSE_WINDOW_WATCHDOG_RESET:{
			strcat(reset_cause_msg, "RESET_CAUSE_WINDOW_WATCHDOG_RESET");
			break;
		}
        case RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET:{
            strcat(reset_cause_msg, "RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET");
            ret = 1;
            break;
        }
        case RESET_CAUSE_SOFTWARE_RESET:{
            ret = 2;
			cm_read_reset_code();
            break;
        }
        case RESET_CAUSE_POWER_ON_POWER_DOWN_RESET:{
            strcat(reset_cause_msg, "RESET_CAUSE_POWER_ON_POWER_DOWN_RESET");
			break;
        }
        case RESET_CAUSE_EXTERNAL_RESET_PIN_RESET:{
			strcat(reset_cause_msg, "RESET_CAUSE_EXTERNAL_RESET_PIN_RESET");
			break;
		}
        default:{
            break;
        }
    }

	uart_debug_print(reset_cause_msg);
	uart_debug_print("\r\n");

//    file_fill_log_buff(reset_cause_msg);

	qctl_mqtt_fota_stat(reset_cause_msg);

    reset_status = 0;

    return ret;
}

void cm_read_reset_code()
{
    switch(reset_cause){
        case Reset_Code_Server:
            strcat(reset_cause_msg, "Server Reset");
            break;
        case Reset_Code_FOTA_Fail:
            strcat(reset_cause_msg, "FOTA >> Failed");
            break;
        case Reset_Code_FOTA_Done:
            strcat(reset_cause_msg, "FOTA >> Successfully done");
            break;
        case Reset_Code_Invalid_SiliconId:
            strcat(reset_cause_msg, "FOTA >> failed >> invalid SiliconID");
            break;
        case Reset_Code_Mqtt_Open_Fail:
            strcat(reset_cause_msg, "MQTT >> failed to open");
            break;
        case Reset_Code_Mqtt_Open_Resp_Fail:
            strcat(reset_cause_msg, "MQTT >> failed to get Open Resp");
            break;
        case Reset_Code_Mqtt_Conn_Resp_Fail:
            strcat(reset_cause_msg, "MQTT >> failed to get Conn Resp");
            break;
        case Reset_Code_Mqtt_Sub_Resp_Fail:
            strcat(reset_cause_msg, "MQTT >> failed to get Subs Resp");
            break;
        case Reset_Code_Mqtt_Pub_Resp_Fail:
			strcat(reset_cause_msg, "MQTT >> failed to get Pub Resp");
			break;
        case Reset_Code_Quectel_Lost:
            strcat(reset_cause_msg, "Quectel >> failed to get Resp");
            break;
        case Reset_Code_I2C_Lost:
            strcat(reset_cause_msg, "Microchip >> failed to get I2C sensor data");
            break;
        default:
            strcat(reset_cause_msg, "Unknown/Hardware Reset");
            break;
    }

	reset_cause = 0;

    return;
}
// #########################################################################################################################

// ###################################################### Buffer Functions #################################################
void cm_create_io_buffer()
{
    memset(io_buff_curr, '\0', sizeof(io_buff_curr));

    uint8_t io_exp_outs_all[3 * Max_IOX_NUMBER];
    memset(io_exp_outs_all, '\0', sizeof(io_exp_outs_all));

    for(int i = 0; i < iox_num; i++){
        io_exp_outs_all[0 + (i * 3)] = expander_3_Outs_Read[i] >> 8;
        io_exp_outs_all[1 + (i * 3)] = expander_3_Outs_Read[i] >> 0;
        io_exp_outs_all[2 + (i * 3)] = expander_2_Outs_Read[i] >> 0;
    }

	uint8_t io_exp_inps_all[3 * Max_IOX_NUMBER];
    memset(io_exp_inps_all, '\0', sizeof(io_exp_inps_all));

    for(int i = 0; i < iox_num; i++){
        io_exp_inps_all[0 + (i * 3)] = expander_1_Inps_Read[i] >> 8;
        io_exp_inps_all[1 + (i * 3)] = expander_1_Inps_Read[i] >> 0;
        io_exp_inps_all[2 + (i * 3)] = expander_2_Inps_Read[i] >> 0;
    }

    uint16_t inputs = 0;
    inputs = (uint16_t)(Inps_Read >> 16);
    inputs = inputs & 0xFF00;

    uint16_t outputs = 0;
    outputs = (uint16_t)(Outs_Read >> 0);
    outputs = outputs & 0xFFFF;

    // "out":"----------------------------------"
    char out_buf[8 + 4 + 6 * Max_IOX_NUMBER];
    memset(out_buf, '\0', sizeof(out_buf));
    // "inp":"----------------------------------"
    char inp_buf[8 + 4 + 6 * Max_IOX_NUMBER];
    memset(inp_buf, '\0', sizeof(inp_buf));

    // "out":"----------------------------------","inp":"----------------------------------"

    sprintf(out_buf, "\"out\":\"");
    char byte_buf[3] = {'\0', '\0', '\0'};

//    char buf[100];
//    sprintf(buf, "outs_num = %d, outs_bytes = %d\r\n", outs_num, (outs_num / 4));
//    usb_debug_print(buf, 1);

    if(outs_num == 0){
        // ignore
    }
    else if(outs_num > 0 && outs_num <= 4){
        sprintf(byte_buf, "%01X", (uint8_t)((Outs_Read >> 12) & 0x0F));
        strcat(out_buf, byte_buf);
    }
    else if(outs_num > 4 && outs_num <= 8){
        sprintf(byte_buf, "%02X", (uint8_t)((Outs_Read >> 8) & 0xFF));
        strcat(out_buf, byte_buf);
    } // FYI >> stm32 is different
    else if(outs_num > 8 && outs_num <= 12){
		sprintf(byte_buf, "%02X", (uint8_t)((Outs_Read >> 8) & 0xFF));
		strcat(out_buf, byte_buf);
		sprintf(byte_buf, "%01X", (uint8_t)((Outs_Read >> 4) & 0x0F));
		strcat(out_buf, byte_buf);
	} // FYI >> stm32 is different
	else if(outs_num > 12 && outs_num <= 16){
		sprintf(byte_buf, "%02X", (uint8_t)((Outs_Read >> 8) & 0xFF));
		strcat(out_buf, byte_buf);
		sprintf(byte_buf, "%02X",  (uint8_t)((Outs_Read >> 0 ) & 0xFF));
		strcat(out_buf, byte_buf);
	}
    else if(outs_num > 16){

    	sprintf(byte_buf, "%02X", (uint8_t)((Outs_Read >> 8) & 0xFF));
		strcat(out_buf, byte_buf);
		sprintf(byte_buf, "%02X",  (uint8_t)((Outs_Read >> 0 ) & 0xFF));
		strcat(out_buf, byte_buf);

//        RS232_Debug_PutString("out_buf1:\r\n");
//        RS232_Debug_PutString(out_buf);
//        RS232_Debug_PutString("\r\n");

        // fill IOX outputs
        uint8_t num = outs_num - 16;
        uint8_t expander_num = num / 24;
        uint8_t expander_diff = num % 24;
//        if(expander_num == 0 && expander_diff > 0){
//            expander_num = 1;
//        }
//        else
        if(expander_num >= 0 && expander_diff > 0){
            expander_num++;
        }
        iox_num = expander_num;  // is inputs OK?????????????????????????????

//        else if(expander_num > A_ioxNum){
//            A_ioxNum = expander_num;
//        }

        uint8_t nnum_d = num % 24;
        for(uint8_t i = 0; i < expander_num; i++){

//            RS232_Debug_PutString("out_buf2:\r\n");
//            RS232_Debug_PutString(out_buf);
//            RS232_Debug_PutString("\r\n");

            if(nnum_d == 0 && num >= 24){
                nnum_d = 24;
            }

            if(nnum_d > 0 && nnum_d <= 4){
                sprintf(byte_buf, "%01X", (uint8_t)((expander_3_Outs_Read[i] >> 12) & 0x0F));
                strcat(out_buf, byte_buf);
                num = num - 4;
            }
            else if(nnum_d > 4 && nnum_d <= 8){
                sprintf(byte_buf, "%02X", (uint8_t)((expander_3_Outs_Read[i] >> 8) & 0xFF));
                strcat(out_buf, byte_buf);
                num = num - 8;
            }
            else if(nnum_d > 8 && nnum_d <= 12){
                sprintf(byte_buf, "%02X", (uint8_t)((expander_3_Outs_Read[i] >> 8) & 0xFF));
                strcat(out_buf, byte_buf);

                sprintf(byte_buf, "%01X", (uint8_t)((expander_3_Outs_Read[i] >> 0) & 0x0F));
                strcat(out_buf, byte_buf);

//                RS232_Debug_PutString("out_buf3:\r\n");
//                RS232_Debug_PutString(out_buf);
//                RS232_Debug_PutString("\r\n");

                num = num - 12;
            }
            else if(nnum_d > 12 && nnum_d <= 16){
                sprintf(byte_buf, "%02X", (uint8_t)((expander_3_Outs_Read[i] >> 8) & 0xFF));
                strcat(out_buf, byte_buf);

                sprintf(byte_buf, "%02X", (uint8_t)((expander_3_Outs_Read[i] >> 0) & 0xFF));
                strcat(out_buf, byte_buf);

                num = num - 16;
            }
            else if(nnum_d > 16 && nnum_d <= 20){
                sprintf(byte_buf, "%02X", (uint8_t)((expander_3_Outs_Read[i] >> 8) & 0xFF));
                strcat(out_buf, byte_buf);

                sprintf(byte_buf, "%02X", (uint8_t)((expander_3_Outs_Read[i] >> 0) & 0xFF));
                strcat(out_buf, byte_buf);

                sprintf(byte_buf, "%01X", (uint8_t)((expander_2_Outs_Read[i] >> 4) & 0x0F));
                strcat(out_buf, byte_buf);

                num = num - 20;
            }
            else if(nnum_d > 20 && nnum_d <= 24){
                sprintf(byte_buf, "%02X", (uint8_t)((expander_3_Outs_Read[i] >> 8) & 0xFF));
                strcat(out_buf, byte_buf);

                sprintf(byte_buf, "%02X", (uint8_t)((expander_3_Outs_Read[i] >> 0) & 0xFF));
                strcat(out_buf, byte_buf);

                sprintf(byte_buf, "%02X", (uint8_t)((expander_2_Outs_Read[i] >> 0) & 0xFF));
                strcat(out_buf, byte_buf);

                num = num - 24;
            }

            nnum_d = num % 24;
        }
    }

//    sprintf(out_buf, "\"out\":\"%04X", outputs);

//    for(int i = 0; i < A_ioxNum; i++){
//        sprintf(byte_buf, "%02X", io_exp_outs_all[0 + (i * 3)]);
//        strcat(out_buf, byte_buf);
//        sprintf(byte_buf, "%02X", io_exp_outs_all[1 + (i * 3)]);
//        strcat(out_buf, byte_buf);
//        sprintf(byte_buf, "%02X", io_exp_outs_all[2 + (i * 3)]);
//        strcat(out_buf, byte_buf);
//    }
    strcat((char*)io_buff_curr, out_buf);
    strcat((char*)io_buff_curr, "\"");
    strcat((char*)io_buff_curr, ",");

    sprintf(inp_buf, "\"inp\":\"%04X", inputs);
    for(int i = 0; i < iox_num; i++){
        sprintf(byte_buf, "%02X", io_exp_inps_all[0 + (i * 3)]);
        strcat(inp_buf, byte_buf);
        sprintf(byte_buf, "%02X", io_exp_inps_all[1 + (i * 3)]);
        strcat(inp_buf, byte_buf);
        sprintf(byte_buf, "%02X", io_exp_inps_all[2 + (i * 3)]);
        strcat(inp_buf, byte_buf);
    }
    strcat((char*)io_buff_curr, inp_buf);
    strcat((char*)io_buff_curr, "\"");

    return;
}



void cm_create_all_buffer()
{

    // "tmp":"----","wdt":"--------","ver":"--------------","sen":{"-------------------------":"---------------------", ... ,"-------------------------":"---------------------"}

    uint16_t cur = 0;
    char bff[200];
    memset(bff, '\0', sizeof(bff));
    sprintf(bff, "\"tmp\":\"%04X\",\"wdt\":\"%08lX\",\"swt\":\"%08lX\",\"ver\":\"", chip_temp, cm_wtd_cnt, cm_swt_cnt);
    strcat(bff, VERSION);
    strcat(bff, "\",");

    uint8_t buf_index = 0;
    while(bff[buf_index] != '\0'){
        all_buffer[cur] = bff[buf_index];
        cur++;
        buf_index++;
    }

    memset(bff, '\0', sizeof(bff));
    sprintf(bff, "\"sen\":{");
    buf_index = 0;
    while(bff[buf_index] != '\0'){
        all_buffer[cur] = bff[buf_index];
        cur++;
        buf_index++;
    }

  for(uint8_t i = 0; i < modbus_num; i++){
	  	HAL_IWDG_Refresh(&hiwdg);
		all_buffer[cur] ='\"';
		cur++;

		buf_index = 0;
		while(modbus_str_pack.send[i][buf_index] != '\0'){
			all_buffer[cur] = modbus_str_pack.send[i][buf_index];
			cur++;
			buf_index++;
		}

		all_buffer[cur] ='\"';
		cur++;

		all_buffer[cur] =':';
		cur++;

		all_buffer[cur] ='\"';
		cur++;

		buf_index = 0;
		while(modbus_str_pack.recv[i][buf_index] != '\0'){
			all_buffer[cur] = modbus_str_pack.recv[i][buf_index];
			cur++;
			buf_index++;
		}

		all_buffer[cur] ='\"';
		cur++;

		if(i < (modbus_num - 1)){
			all_buffer[cur] = ',';
			cur++;
		}

	}


    all_buffer[cur] ='}';
    cur++;

    all_buffer[cur] ='\0';
    cur++;


    return;
}

// #########################################################################################################################
