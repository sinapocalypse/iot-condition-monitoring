/*
 * defines.h
 *
 *  Created on: May 27, 2023
 *      Author: IroTeam
 *
 *
 */

#ifndef INC_DEFINES_H_
#define INC_DEFINES_H_

// ###################################################### Includes #########################################################
// embOS header
#include "RTOS.h"
// standard library headers
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "ctype.h"
// IDE API headers
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "usart.h"
#include "gpio.h"
// #########################################################################################################################

// ###################################################### Application ##############################################

//#define VERSION                         		 "uart1dma_uart2dma"
#define VERSION                         		 "STM32-2024.09.08-v1.6.4-2Sen"
#define MAIN_ID                         		 0x00

// ###################################################### Lengths ##########################################################
#define Str_Len_Set_Rst              			 44				// {SET_RST}:{"id":"--","data":{"stm_rst":"-"}}
#define Str_Len_Set_Wdt              			 44				// {SET_WDT}:{"id":"--","data":{"stm_wdt":"-"}}
#define Str_Len_Set_Log              			 44				// {SET_LOG}:{"id":"--","data":{"set_log":"-"}}

#define Str_Len_Put_Get_Ino       	 			 (31 + 1)		// {GET_INO}:{"id":"--","data":{}}<NUL>
#define Str_Len_Put_Get_All       	 			 (32 + 1)		// {GET_ALL}:{"id":"--","data":{,}}<NUL>
//#define Str_Len_Put_Get_Fot              		 (47 + 1)    	// {GET_FOT}:{Downloading BOOT: 100 of 250(100%)}<NUL>
#define Str_Len_Put_Get_Fot              		 (128)    		// {GET_FOT}:{Downloading BOOT: 100 of 250(100%)}<NUL>


#define	Len_Max_IOX_Buffer_Recv		 			 45				//--:{GET_INO}:{"inp":"------","out":"------"}<NUL>--> len = 45
#define	Max_IOX_NUMBER				 			 3

#define Num_Max_Sen                     		 32
#define Len_Max_Sen_Send                		 25
#define Len_Max_Sen_Recv                		 21

#define Len_Max_Mod_Buffer_Recv         		 128

#define Len_Max_Buffer_Ino              		 (61 + 1)    	// "out":"----------------------","inp":"----------------------"<NUL>
#define Len_Max_Buffer_All              		 (1799 + 1)  	// "tmp":"----","wdt":"--------","ver":"--------------","sen":{"-------------------------":"---------------------", ... ,"-------------------------":"---------------------"}} 97 + 32 * 51 + 31 - 34 = 1757

#define Len_Max_Mod_Ascii_recv          		 9
#define Len_Max_Mod_rtu_recv            		 4

#define Uart_Max_Delay							 500

// #########################################################################################################################

// ###################################################### embOS ##########################################################

// ****************************************************** Tasks **********************************************************

// ---------------------------- Task STACK Sizes --------------------------------

#define STACK_TASK_OPERATOR_SIZE				512
#define STACK_TASK_QUECTEL_SIZE					1024
#define STACK_TASK_SENSOR_SIZE					512
#define STACK_TASK_IO_SIZE						512
#define STACK_TASK_SEP_SIZE						512

// ---------------------------- Task Declarations --------------------------------

///* USER CODE BEGIN Declarations_Task0 */
extern OS_TASK         TCB_Task_Operator;                   					 // Control block for Task
extern OS_STACKPTR int Stack_Task_Operator[STACK_TASK_OPERATOR_SIZE];            // Stack for Task
///* USER CODE END Declarations_Task0 */

///* USER CODE BEGIN Declarations_Task1 */
extern OS_TASK         TCB_Task_Quectel;               					    // Control block for Task
extern OS_STACKPTR int Stack_Task_Quectel[STACK_TASK_QUECTEL_SIZE];         // Stack for Task
///* USER CODE END Declarations_Task1 */

///* USER CODE BEGIN Declarations_Task2 */
extern OS_TASK TCB_Task_Io; 													//Control block for Task
extern OS_STACKPTR int Stack_Task_Io[STACK_TASK_IO_SIZE]; 			// Stack for Task
///* USER CODE END Declarations_Task2 */

///* USER CODE BEGIN Declarations_Task3 */
extern OS_TASK TCB_Task_Sep; 													//Control block for Task
extern OS_STACKPTR int Stack_Task_Sep[STACK_TASK_SEP_SIZE]; 			// Stack for Task
///* USER CODE END Declarations_Task3 */

// ---------------------------- Task Delays --------------------------------

#define TASK_DELAY_IO_MS      		 			 300
#define TASK_DELAY_SENSOR_MS      		 		 225
#define TASK_DELAY_OPERATOR_MS                   150
#define TASK_DELAY_QUECTEL_MS                    75
#define TASK_DELAY_SEP_MS                    	 50

// ---------------------------- Task Counters --------------------------------

#define QUECTEL_CSQ_COUNTER                      ((1000 / TASK_DELAY_QUECTEL_MS) * 30)
#define QUECTEL_TIME_COUNTER					 ((1000 / TASK_DELAY_QUECTEL_MS) * 30)
#define QUECTEL_GET_ALL_COUNTER                  ((1000 / TASK_DELAY_OPERATOR_MS) * 30)

// ---------------------------- Task Variables --------------------------------

extern uint16_t operator_task_counter;

// ****************************************************** Timers *********************************************************

// ---------------------------- Timer Declarations --------------------------------

extern OS_TIMER osTimer_reset_state;
extern OS_TIMER osTimer_uart_quectel;
extern OS_TIMER osTimer_quectel_pub;
extern OS_TIMER osTimer_quectel_state_sentinel;
extern OS_TIMER osTimer_quectel_sms_mode;
extern OS_TIMER osTimer_rs485_sensor;
//extern OS_TIMER osTimer_quectel_power_down;

// ---------------------------- Timer Timeouts --------------------------------

#define Timeout_Modbus_Sensor_Timer_ms      	10
#define Timeout_Recv_IOX_Timer_ms				10
#define Timeout_Quectel_Uart_Recv_Timer_ms      10
#define Timeout_Quectel_Publish_Timer_ms       	(60 * 1000)
#define Timeout_Quectel_SMS_Timer_ms			(1000 * 60 * 3)  // 3 minutes
#define Timeout_Quectel_Sentinel_Timer_ms       (30 * 1000)
#define Timeout_Reset_State_Timer_ms        	(1000 * 60 * 3) // 3 Minutes
#define Timeout_Quectel_Power_down_Timer_ms     (1000 * 2) // 2 Second

// ****************************************************** QUEUEs *********************************************************

// ---------------------------- QUEUE Declarations --------------------------------

extern OS_Q _Queue_Set;
extern OS_Q _Queue_Get;
extern OS_Q _Queue_Sep;
extern OS_Q _Queue_Quectel;

// ---------------------------- QUEUE Sizes --------------------------------

#define Message_Cnt_Set_Queue                    3
#define Message_Cnt_Get_Queue                    2
#define Message_Cnt_Sep_Queue                    3
#define Message_Cnt_Quectel_Queue				 3

#define Extra_Size                               4

#define Queue_Len_Set                         	 1000   // {SET_SEN}:{"id":"--","data":["--","-------------------------","-------------------------", ... ,"-------------------------"]}<NUL> {34 + [32 * (25 + 2) + 31] + 2} + 1 = 932
#define Queue_Len_Get                        	 1800   // {GET_ALL}:{"id":"--","data":{"out":"--------","inp":"--------","tmp":"----","wdt":"--------","ver":"--------------","sen":{"-------------------------":"---------------------","-------------------------":"---------------------", ... ,"-------------------------":"---------------------"}}}<NUL> {123 + [32 * (25 + 2 + 1 + 21 + 2) + 31] + 3} + 1 = 1790
#define Queue_Len_Sep                        	 1000
#define Queue_Len_Quectel						 1500

#define Queue_Size_Set                           (Message_Cnt_Set_Queue * (Queue_Len_Set + OS_Q_SIZEOF_HEADER + Extra_Size))
#define Queue_Size_Get                           (Message_Cnt_Get_Queue * (Queue_Len_Get + OS_Q_SIZEOF_HEADER + Extra_Size))
#define Queue_Size_Sep                           (Message_Cnt_Sep_Queue * (Queue_Len_Sep + OS_Q_SIZEOF_HEADER + Extra_Size))
#define Queue_Size_Quectel						 (Message_Cnt_Quectel_Queue * (Queue_Len_Quectel + OS_Q_SIZEOF_HEADER + Extra_Size))

// ---------------------------- QUEUE Buffers --------------------------------

extern char _QBuffer_Queue_Set[Queue_Size_Set];
extern char _QBuffer_Queue_Get[Queue_Size_Get];
extern char _QBuffer_Queue_Sep[Queue_Size_Sep];
extern char _QBuffer_Queue_Quectel[Queue_Size_Quectel];

// ---------------------------- QUEUE Function Prototypes --------------------------------

extern uint8_t q_put_message(OS_QUEUE* queue, char* message, int lng);
extern void* q_get_message(OS_QUEUE* queue);
extern void* q_get_message2(OS_QUEUE* queue, uint16_t* size);
// ****************************************************** NTP *********************************************************

#define NTP_SERVER_ADDR                           "ir.pool.ntp.org"

// ****************************************************** MQTT *********************************************************

// ---------------------------- MQTT Server Configurations --------------------------------

#define EMQ_SERVER_ADDR                           "emq.iroteam.com"
#define EMQ_SERVER_PORT                           31536
//#define EMQ_SERVER_PORT                             30348		// use for SSL PORT!!!!

#define EMQ_PASSWD                                "43a35b9326ea46e"

#define EMQ_PASS_MAX_LEN                          20

#define PUB_MAIN_TOPIC                            "Server"
#define SUB_MAIN_TOPIC                            "IroTeamZero_V2"

#define TOPIC_WILL                                "M/STATUS"
#define MSG_WILL_DC                               "{\'ConnectStatus\':0}" // FYI >> M95 bug
#define MSG_WILL_CC                               "{\"ConnectStatus\":1}"

#define TOPIC_FOTA_START                          "Start"
#define TOPIC_FOTA_DOWNLOADING                    "Downloading files..."
#define TOPIC_FOTA_WRITING                        "Writing..."
#define TOPIC_FOTA_INSTALLING                     "Installing..."
#define TOPIC_FOTA_CALCULATING_CRC                "Calculating and Checking CRC..."
#define TOPIC_FOTA_FOLDER_LEN_FAILED			  "failed >> Invalid Folder name Length!"
#define TOPIC_FOTA_INVALID_CONFIG_FAILED          "failed >> Invalid Configuration from Server!"
#define TOPIC_FOTA_GET_FILE_FAILED                "failed >> Getting file from FTP!"
#define TOPIC_FOTA_LOCK_FLASH_FAILED			  "failed >> Locking Internal FLASH!"
#define TOPIC_FOTA_WRITE_TO_FLASH_FAILED		  "failed >> Writing to Internal FLASH!"
#define TOPIC_FOTA_ERASE_FLASH_FAILED             "failed >> Erasing Internal FLASH!"
#define TOPIC_FOTA_FTP_PATH_FAILED                "failed >> Setting FTP Path!"
#define TOPIC_FOTA_STOPPED                        "Stopped!"

// ---------------------------- MQTT Server Variables --------------------------------
extern char emq_server_ip[20+1];
extern char emq_server_port[20+1];
extern char emq_server_pass[20+1]; // user >> IMEI

// ****************************************************** FTP *********************************************************

// ---------------------------- FTP Server Configurations -------------------------------------------------------
// ---------------------------- OLD FTP
//#define FTP_SERVER_IP							  "5.160.247.230"
//#define FTP_SERVER_PORT							  "21"
//#define FTP_SERVER_USER                           "acsftp"
//#define FTP_SERVER_PASS                           "ftpACSIroTeam1926"

//#define FTP_SERVER_IP							  "2.188.210.4"
//#define FTP_SERVER_PORT							  "21"
//#define FTP_SERVER_USER                           "IroTeam1"
//#define FTP_SERVER_PASS                           "IroTeam@1324"

#define FTP_SERVER_ADMIN_USER                     "ftp-user1"
#define FTP_SERVER_ADMIN_PASS                     "ftpIroTeam1435"

#define FTP_SERVER_GET_TRY_COUNT				  10
#define FTP_SERVER_PUT_TRY_COUNT				  5
#define FTP_SERVER_PATH_TRY_COUNT				  3

#define FTP_FILE_SIZE							  1000

// ****************************************************** GSM (Quectel) *********************************************************

// ---------------------------- Quectel Defines --------------------------------

#define Uart_Gsm_Buffer_Len_Recv                  1500      // Supports FOTA Data too!!!

// ---------------------------- Quectel structure --------------------------------

struct quectl_recv_pack
{
    uint8_t buff[Uart_Gsm_Buffer_Len_Recv];
    uint16_t index;
//    uint8_t start;
};
extern struct quectl_recv_pack quectl_recv_pack;

// ---------------------------- Quectel Variables --------------------------------

extern uint8_t qctl_rx_buf[1];
extern uint8_t qctl_pub_resp_failure_counter;
extern uint8_t qctl_pub_resp_fatal_failure_counter;

// ****************************************************** ADC & DMA (for Chip Temperature) **************************************

// ---------------------------- ADC & DMA Defines ----------------------------

#define V25	1.43f
#define AVG_Slope 0.0043f
#define VREF 1.20f
#define ADCMAX 4095

// ---------------------------- ADC & DMA structure ----------------------------

typedef struct AdcValues{
	uint16_t Raw[2]; /* Raw values from ADC */
	double IntSensTmp; /* Temperature */
}adcval_t;

// ---------------------------- ADC & DMA Variables ----------------------------

extern uint8_t flag_modbus_write;

extern float V_supply;
extern float T_sense;
extern short int chip_temp;

extern uint16_t adc1;
extern uint16_t adc2;

extern adcval_t Adc;
extern uint8_t adc_flag;
// ****************************************************** FOTA **************************************

// ---------------------------- FOTA Defines ----------------------------

#define FOTA_STOP                           	 0
#define FOTA_START                          	 1
#define FOTA_ROW_SIZE                       	 32 // FOR STM .hex file ! JUST DATA BYTES
#define FOTA_MAX_FILE_LINE                  	 30 // FOR STM .hex file

// ---------------------------- FOTA Variables ----------------------------
// --------------------------- FOTA SERVER VARIABLES ---------------
extern char* ftp_server_ip;
extern char* ftp_server_port;
extern char* ftp_server_user;
extern char* ftp_server_pass;

extern char* ftp_server_admin_user;
extern char* ftp_server_admin_pass;
// --------------------------- FOTA FLASH TEMP VARIABLES ---------------
extern uint8_t flash_fota_flag;
extern uint32_t flash_fota_app_size;
// --------------------------- FOTA FLASH TEMP VARIABLES ---------------

extern uint8_t fota_flag_start;

extern uint16_t fota_file_numbers;
extern uint16_t fota_row_counter;
extern uint16_t fota_row_sequence;
extern uint16_t fota_recv_byte_count;

extern uint32_t server_fota_App_size;
extern uint32_t server_fota_App_Address;
extern char server_fota_folder_name[30];
extern uint16_t server_fota_file_numbers;
extern uint16_t server_fota_App_lines;



extern uint8_t int_byte;
extern uint8_t int_byte_ctr;

// ****************************************************** IO Expanders **************************************

// ---------------------------- IO Expander structure ----------------------------

struct iox_pack
{
	char buff[Len_Max_IOX_Buffer_Recv];
	uint8_t busy;
	uint16_t index;
	uint8_t id[3];
};
extern struct iox_pack iox_recv_pack;

// ---------------------------- IO Expander Variables ----------------------------

extern uint8_t iox_num;
extern uint8_t iox_rx_buf[1];

extern uint8_t expander_2_Outs_Read[Max_IOX_NUMBER];
extern uint8_t expander_2_Outs_Write[Max_IOX_NUMBER];
extern uint8_t expander_2_Outs_Old_Write[Max_IOX_NUMBER];

extern uint16_t expander_3_Outs_Read[Max_IOX_NUMBER];
extern uint16_t expander_3_Outs_Write[Max_IOX_NUMBER];
extern uint16_t expander_3_Outs_Old_Write[Max_IOX_NUMBER];

extern uint16_t expander_1_Inps_Read[Max_IOX_NUMBER];
extern uint8_t expander_2_Inps_Read[Max_IOX_NUMBER];

// ****************************************************** Modbus (Sensor) **************************************

// ---------------------------- Modbus structure ----------------------------

struct modbus_pack
{
    uint8_t buff[Len_Max_Mod_Buffer_Recv];
    uint8_t busy;
    uint8_t index;
    uint8_t id[5];
};
extern struct modbus_pack modbus_recv_pack;

struct modbus_str_pack
{
    uint8_t send[Num_Max_Sen][Len_Max_Sen_Send + 1];
    uint8_t recv[Num_Max_Sen][Len_Max_Sen_Recv + 1];
};
extern struct modbus_str_pack modbus_str_pack;

// ---------------------------- Modbus Variables ----------------------------

extern uint8_t modbus_rx_buf[1];
extern uint8_t modbus_num;
extern uint8_t modbus_curr;
extern uint8_t modbus_curr_state;
extern uint8_t modbus_default_sen_stat;

// ****************************************************** IO **************************************

// ---------------------------- IO Variables --------------------------------
extern uint8_t io_enable;

extern uint32_t Outs_Write;
extern uint32_t Outs_Old_Write;

extern uint16_t Outs_Read;
extern uint32_t Inps_Read;

extern uint8_t io_buff_sent[Len_Max_Buffer_Ino];
extern uint8_t io_buff_curr[Len_Max_Buffer_Ino];

extern uint8_t Out_Array[32];
extern uint8_t Out_Array_Old[32];
extern uint8_t outs_num;

// ****************************************************** Application **************************************

// ---------------------------- Application Variables ----------------------------

extern uint8_t date_year;
extern uint8_t date_month;
extern uint8_t date_day;
extern uint8_t date_hour;
extern uint8_t date_minute;
extern uint8_t date_second;

extern uint8_t is_log_service_activated;
extern uint8_t all_buffer[Len_Max_Buffer_All];
extern char fota_topic_buffer[Str_Len_Put_Get_Fot];

extern uint8_t global_internal_flash_flag;
extern uint8_t manual_reset;

// ---------------------------- Application Function Prototypes ----------------------------

extern void df_init_vars_all();
extern void df_delay_ms(uint16_t num, uint16_t unit);

extern uint8_t df_hexstring_hex(char character);
extern char df_hex_hexstring(uint8_t data);

extern void df_byte_string (uint32_t *Data, char *Buf);
extern void df_byte_all_string(uint32_t *Data, char *Buf);

extern char* strremove(char *str, const char *sub);

extern void led_set_color(uint8_t color_code);

extern void uart_debug_print(const char* msg);
extern void uart_debug_print_len(const char* msg, uint16_t len);

extern uint16_t crc(char* buf, uint16_t len);

// SORTING ALGORITHM
extern void swap(uint8_t* xp, uint8_t* yp);
extern	void selectionSort(uint8_t arr[], uint8_t arr_len);


// #########################################################################################################################
// #########################################################################################################################

#endif /* INC_DEFINES_H_ */
