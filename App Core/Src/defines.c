/*
 * defines.c
 *
 *  Created on: May 27, 2023
 *      Author: IroTeam
 */

#include "defines.h"
//#include "usbd_cdc_if.h"
#include "usart.h"
#include "iwdg.h"

// ---------------------------- Task Declarations --------------------------------

///* USER CODE BEGIN Declarations_Task0 */
OS_TASK         TCB_Task_Operator;                   					 // Control block for Task
OS_STACKPTR int Stack_Task_Operator[STACK_TASK_OPERATOR_SIZE];            // Stack for Task
///* USER CODE END Declarations_Task0 */

///* USER CODE BEGIN Declarations_Task1 */
OS_TASK         TCB_Task_Quectel;               					    // Control block for Task
OS_STACKPTR int Stack_Task_Quectel[STACK_TASK_QUECTEL_SIZE];         // Stack for Task
///* USER CODE END Declarations_Task1 */

///* USER CODE BEGIN Declarations_Task2 */
OS_TASK TCB_Task_Io; 													//Control block for Task
OS_STACKPTR int Stack_Task_Io[STACK_TASK_IO_SIZE]; 			// Stack for Task
///* USER CODE END Declarations_Task2 */

///* USER CODE BEGIN Declarations_Task3 */
OS_TASK TCB_Task_Sep; 													//Control block for Task
OS_STACKPTR int Stack_Task_Sep[STACK_TASK_SEP_SIZE]; 			// Stack for Task
///* USER CODE END Declarations_Task3 */


// ---------------------------- Task Variables --------------------------------

uint16_t operator_task_counter = 0;

// ****************************************************** Timers *********************************************************

// ---------------------------- Timer Declarations --------------------------------

OS_TIMER osTimer_reset_state;
OS_TIMER osTimer_uart_quectel;
OS_TIMER osTimer_quectel_pub;
OS_TIMER osTimer_quectel_state_sentinel;
OS_TIMER osTimer_quectel_sms_mode;
OS_TIMER osTimer_rs485_sensor;
//OS_TIMER osTimer_quectel_power_down;
//OS_TIMER osTimer_rs485_iox;

// ****************************************************** QUEUEs *********************************************************

// ---------------------------- QUEUE Declarations --------------------------------

OS_Q _Queue_Set;
OS_Q _Queue_Get;
OS_Q _Queue_Sep;
OS_Q _Queue_Quectel;

// ---------------------------- QUEUE Buffers --------------------------------

char _QBuffer_Queue_Set[Queue_Size_Set];
char _QBuffer_Queue_Get[Queue_Size_Get];
char _QBuffer_Queue_Sep[Queue_Size_Sep];
char _QBuffer_Queue_Quectel[Queue_Size_Quectel];

// ****************************************************** MQTT *********************************************************

// ---------------------------- MQTT Server Variables --------------------------------

char emq_server_ip[20+1] = "emq.iroteam.com";
char emq_server_port[20+1] = "31536";
char emq_server_pass[20+1] = "43a35b9326ea46e"; // user >> IMEI

// ****************************************************** GSM (Quectel) *********************************************************

// ---------------------------- Quectel structure --------------------------------

struct quectl_recv_pack quectl_recv_pack;

// ---------------------------- Quectel Variables --------------------------------

uint8_t qctl_rx_buf[1];
uint8_t qctl_pub_resp_failure_counter = 0;
uint8_t qctl_pub_resp_fatal_failure_counter = 0;

// ****************************************************** ADC & DMA (for Chip Temperature) **************************************

// ---------------------------- ADC & DMA Variables ----------------------------

uint8_t flag_modbus_write = 0;

float V_supply = 0.0f;
float T_sense = 0.0f;
short int chip_temp = 0;

uint16_t adc1 = 0;
uint16_t adc2 = 0;

adcval_t Adc;

uint8_t adc_flag = 0;
// ****************************************************** FOTA **************************************

// ---------------------------- FOTA Variables ----------------------------

// ---------------------------- FOTA Server Variables ----------------------------
char* ftp_server_ip;// = "2.188.210.4";
char* ftp_server_port;//	= "21";
char* ftp_server_user;// = "IroTeam1";
char* ftp_server_pass;// = "IroTeam@1324";

// for Log
char* ftp_server_admin_user = "ftp-user1";
char* ftp_server_admin_pass = "ftpIroTeam1435";

// --------------------------- FOTA FLASH TEMP VARIABLES ---------------
uint8_t flash_fota_flag = 0;
uint32_t flash_fota_app_size = 0;
// --------------------------- FOTA FLASH TEMP VARIABLES ---------------

uint8_t fota_flag_start = 0;


uint16_t fota_file_numbers = 1;
uint16_t fota_row_counter = 0;
uint16_t fota_row_sequence = 0;
uint16_t fota_recv_byte_count = 0;

uint32_t server_fota_App_size = 0;
uint32_t server_fota_App_Address = 0;
char server_fota_folder_name[30];
uint16_t server_fota_file_numbers = 0;
uint16_t server_fota_App_lines = 0;
char server_fota_data[1500];

//uint8_t fota_buff[Len_Max_Buffer_Fota];

uint8_t int_byte = 0;
uint8_t int_byte_ctr = 0;

// ****************************************************** IO Expanders **************************************

// ---------------------------- IO Expander structure ----------------------------

struct iox_pack iox_recv_pack;

// ---------------------------- IO Expander Variables ----------------------------

uint8_t iox_num = 0;
//uint iox_req_index = 0;
uint8_t iox_rx_buf[1];

uint8_t expander_2_Outs_Read[Max_IOX_NUMBER];
uint8_t expander_2_Outs_Write[Max_IOX_NUMBER];
uint8_t expander_2_Outs_Old_Write[Max_IOX_NUMBER];

uint16_t expander_3_Outs_Read[Max_IOX_NUMBER];
uint16_t expander_3_Outs_Write[Max_IOX_NUMBER];
uint16_t expander_3_Outs_Old_Write[Max_IOX_NUMBER];

uint16_t expander_1_Inps_Read[Max_IOX_NUMBER];
uint8_t expander_2_Inps_Read[Max_IOX_NUMBER];

// ****************************************************** Modbus (Sensor) **************************************

// ---------------------------- Modbus structure ----------------------------

struct modbus_pack modbus_recv_pack;

struct modbus_str_pack modbus_str_pack;

// ---------------------------- Modbus Variables ----------------------------

uint8_t modbus_rx_buf[1];
uint8_t modbus_num = 0;
uint8_t modbus_curr = 0;
uint8_t modbus_curr_state = 0;
uint8_t modbus_default_sen_stat = 0;

// ****************************************************** IO **************************************

// ---------------------------- IO Variables --------------------------------
uint8_t io_enable = 0;

uint32_t Outs_Write = 0;
uint32_t Outs_Old_Write = 0;

uint16_t Outs_Read = 0;
uint32_t Inps_Read = 0;

uint8_t io_buff_sent[Len_Max_Buffer_Ino];
uint8_t io_buff_curr[Len_Max_Buffer_Ino];

uint8_t Out_Array[32];
uint8_t Out_Array_Old[32];
uint8_t outs_num = 0;

// ****************************************************** Application **************************************

// ---------------------------- Application Variables ----------------------------

uint8_t date_year = 1;
uint8_t date_month = 1;
uint8_t date_day = 1;
uint8_t date_hour = 0;
uint8_t date_minute = 0;
uint8_t date_second = 0;

uint8_t is_log_service_activated = 0;
uint8_t all_buffer[Len_Max_Buffer_All];
char fota_topic_buffer[Str_Len_Put_Get_Fot];

uint8_t global_internal_flash_flag = 0;
uint8_t manual_reset = 0;

// ---------------------------- Application Function Prototypes ----------------------------

void df_init_vars_all()
{
	HAL_IWDG_Refresh(&hiwdg);

	io_init();

	Adc.IntSensTmp = 0;
	Adc.Raw[0] = 0;
	Adc.Raw[1] = 0;

	chip_temp = 0;

    memset(iox_recv_pack.buff, '\0', sizeof(iox_recv_pack.buff));
    iox_recv_pack.busy = 0;
    iox_recv_pack.index = 0;
    iox_recv_pack.id[0] = 255;
    iox_recv_pack.id[1] = 255;
    iox_recv_pack.id[2] = 255;

    memset(server_fota_folder_name, '\0', sizeof(server_fota_folder_name));

    memset(server_fota_data, '\0', sizeof(server_fota_data));

    memset(_QBuffer_Queue_Set, '\0', sizeof(_QBuffer_Queue_Set));
	memset(_QBuffer_Queue_Get, '\0', sizeof(_QBuffer_Queue_Get));
	memset(_QBuffer_Queue_Sep, '\0', sizeof(_QBuffer_Queue_Sep));
	memset(_QBuffer_Queue_Quectel, '\0', sizeof(_QBuffer_Queue_Quectel));

	quectl_recv_pack.index = 0;
//	quectl_recv_pack.lf_start = 0;
//	quectl_recv_pack.cr_start = 0;
	memset(quectl_recv_pack.buff, '\0', sizeof(quectl_recv_pack.buff));


	//init sensor struct
	memset(modbus_str_pack.recv, '\0', sizeof(modbus_str_pack.recv));
	memset(modbus_str_pack.send, '\0', sizeof(modbus_str_pack.send));
	//init io and all buffer
	memset(all_buffer, '\0', sizeof(all_buffer));
	memset(io_buff_sent, '\0', sizeof(io_buff_sent));
	memset(io_buff_curr, '\0', sizeof(io_buff_curr));
	memset(fota_topic_buffer, '\0', sizeof(fota_topic_buffer));
	//init mod recv struct
	memset(modbus_recv_pack.buff, '\0', sizeof(modbus_recv_pack.buff));
	memset(modbus_recv_pack.id, '\0', sizeof(modbus_recv_pack.id));
	modbus_recv_pack.busy = 0;
	modbus_recv_pack.index = 0;
	// init io expander
	for(int i = 0; i < Max_IOX_NUMBER; i++){
		expander_2_Outs_Read[i] = 0;
		expander_2_Outs_Write[i] = 0;
		expander_2_Outs_Old_Write[i] = 0;

		expander_3_Outs_Read[i] = 0;
		expander_3_Outs_Write[i] = 0;
		expander_3_Outs_Old_Write[i] = 0;

		expander_1_Inps_Read[i] = 0;
		expander_2_Inps_Read[i] = 0;
	}

	for(int i = 0; i < 32 ; i++){
		Out_Array[i] = 0;
		Out_Array_Old[i] = 0;
	}

    return;
}

void df_delay_ms(uint16_t num, uint16_t unit)
{
    if(unit > 250){
        unit = 250;
    }

    for(uint16_t i = 0; i < num; i++){
    	HAL_IWDG_Refresh(&hiwdg);
        OS_TASK_Delay(unit);
    }

    return;
}

uint8_t q_put_message(OS_QUEUE* queue, char* message, int lng)
{
    HAL_IWDG_Refresh(&hiwdg);

    int res = OS_QUEUE_Put(queue, (void*)message, lng);
    if(res == 0){
        return 1;
    }
    else{
//    	char msg[] = "q_put_message >> failed to put message";
//    	file_fill_log_buff(msg);

        return 0;
    }
}

void* q_get_message(OS_QUEUE* queue)
{
    void* q_output = NULL;// must be static to return pointer!!!
    if(OS_QUEUE_GetPtr(queue, (void**)(&q_output))){
//        OS_QUEUE_Purge(queue);
    }
    else{
        q_output = NULL;
    }

    return q_output;
}

void* q_get_message2(OS_QUEUE* queue, uint16_t* size)
{
	void* q_output = NULL;// must be static to return pointer!!!
	if(OS_QUEUE_GetPtr(queue, (void**)(&q_output))){
		*size = OS_QUEUE_GetMessageSize(queue);
		OS_QUEUE_Purge(queue);
	}
	else{
		q_output = NULL;
	}

	return q_output;
}

uint8_t df_hexstring_hex(char character)
{
    uint8_t data = 255;

    if(
        (character == '0') ||
        (character == '1') ||
        (character == '2') ||
        (character == '3') ||
        (character == '4') ||
        (character == '5') ||
        (character == '6') ||
        (character == '7') ||
        (character == '8') ||
        (character == '9') ||
        (character == 'a') ||
        (character == 'A') ||
        (character == 'b') ||
        (character == 'B') ||
        (character == 'c') ||
        (character == 'C') ||
        (character == 'd') ||
        (character == 'D') ||
        (character == 'e') ||
        (character == 'E') ||
        (character == 'f') ||
        (character == 'F')
    ){
        switch(character){
            case '0':
                data = 0;
                break;
            case '1':
                data = 1;
                break;
            case '2':
                data = 2;
                break;
            case '3':
                data = 3;
                break;
            case '4':
                data = 4;
                break;
            case '5':
                data = 5;
                break;
            case '6':
                data = 6;
                break;
            case '7':
                data = 7;
                break;
            case '8':
                data = 8;
                break;
            case '9':
                data = 9;
                break;
            case 'a':
                data = 10;
                break;
            case 'A':
                data = 10;
                break;
            case 'b':
                data = 11;
                break;
            case 'B':
                data = 11;
                break;
            case 'c':
                data = 12;
                break;
            case 'C':
                data = 12;
                break;
            case 'd':
                data = 13;
                break;
            case 'D':
                data = 13;
                break;
            case 'e':
                data = 14;
                break;
            case 'E':
                data = 14;
                break;
            case 'f':
                data = 15;
                break;
            case 'F':
                data = 15;
                break;
            default:
                break;
        }
    }
    return data;
}

char df_hex_hexstring(uint8_t data)
{
    char character = 0;


    if((data >= 0) && (data <= 15)){
        switch(data){
            case 0:
                character = '0';
                break;
            case 1:
                character = '1';
                break;
            case 2:
                character = '2';
                break;
            case 3:
                character = '3';
                break;
            case 4:
                character = '4';
                break;
            case 5:
                character = '5';
                break;
            case 6:
                character = '6';
                break;
            case 7:
                data = '7';
                break;
            case 8:
                character = '8';
                break;
            case 9:
                character = '9';
                break;
            case 10:
                character = 'A';
                break;
            case 11:
                character = 'B';
                break;
            case 12:
                character = 'C';
                break;
            case 13:
                character = 'D';
                break;
            case 14:
                character = 'E';
                break;
            case 15:
                character = 'F';
                break;
            default:
                break;
        }
    }

    return character;
}

void df_byte_string(uint32_t *Data, char *Buf)
{
	int numberofbytes = ((strlen((char *)Data) / 4) + ((strlen((char *)Data) % 4) != 0)) * 4;
	uint32_t sen_buf_index = 0;
	for (int i = 0; i < numberofbytes; i++)
	{
		char c = Data[i / 4] >> (8 * (i % 4));
		if(isalnum(c) || c == ','){
			Buf[sen_buf_index] = c;
			sen_buf_index++;
		}
	}
	Buf[sen_buf_index] = '\0';

	return;
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

char* strremove(char *str, const char *sub)
{
    size_t len = strlen(sub);
    if(len > 0){
        char *p = str;
        while((p = strstr(p, sub)) != NULL){
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }

    return str;
}

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

void uart_debug_print(const char* msg)
{
	HAL_IWDG_Refresh(&hiwdg);
	HAL_UART_Transmit(&huart5, (const uint8_t*)msg, strlen(msg), 500);

	return;
}

void uart_debug_print_len(const char* msg, uint16_t len)
{
	HAL_IWDG_Refresh(&hiwdg);
	HAL_UART_Transmit(&huart5, (const uint8_t*)msg, len, 500);

	return;
}

extern uint16_t crc(char* buf, uint16_t len)
{
	uint16_t crc = 0xFFFF;
	for(uint16_t pos = 0; pos < len; pos++){
		crc ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc
		for(uint8_t i = 8; i != 0; i--) {    // Loop over each bit
			if((crc & 0x0001) != 0){      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else{                           // Else LSB is not set
				crc >>= 1;                    // Just shift right
			}
		}
	}
	// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
	return crc;
}

// SORTING ALGORITHM
void swap(uint8_t* xp, uint8_t* yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// Function to perform Selection Sort
void selectionSort(uint8_t arr[], uint8_t arr_len)
{
    int i, j, min_idx;

    // One by one move boundary of
    // unsorted subarray
    for (i = 0; i < arr_len - 1; i++){
    	HAL_IWDG_Refresh(&hiwdg);

        // Find the minimum element in
        // unsorted array
        min_idx = i;
        for (j = i + 1; j < arr_len; j++)
            if (arr[j] < arr[min_idx])
                min_idx = j;

        // Swap the found minimum element
        // with the first element
        swap(&arr[min_idx], &arr[i]);
    }
}

// #########################################################################################################################
// #########################################################################################################################
// #########################################################################################################################
