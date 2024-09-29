/*
 * file.c
 *
 *  Created on: Nov 21, 2023
 *      Author: IroTeam
 */

#include "file.h"
#include "quectel.h"

struct file_log_line log_line;
uint8_t is_file_writing_allowed = 0;
uint8_t is_file_ready_to_upload = 0;
uint16_t file_size_counter = 0;
uint8_t critical_file_number = 0;
uint8_t file_delete_counter = 0;

struct file files;
struct file new_file;

char log_server_ip[20+1] = "2.188.210.4";
char log_server_port[20+1] = "21";
char log_server_user[20+1] = "IroTeam1";
char log_server_pass[20+1] = "IroTeam@1324";

void file_init()
{
	file_clear_buffer();
	file_clear_names();

	return;
}

void file_fill_log_buff(char* msg)
{
//	if(!is_file_writing_allowed && msg){
//
//		char* line = NULL;
//
//		line = malloc(strlen((char*)qctl_clk_str) + 2 + strlen(msg) + 3); // time: log_line\r\n => strlen(time) + 2 + strlen(ret) + 2 + 1 <NUL>
//		strcpy(line, (char*)qctl_clk_str);
//		strcat(line, ": ");
//		strcat(line, (char*)msg);
//		strcat(line, "\r\n");
//
//		for(int i = 0; i < strlen((char*)line); i++){
//			HAL_IWDG_Refresh(&hiwdg);
//			log_line.buff[log_line.buff_index] = ((char*)line)[i];
//			log_line.buff_index++;
//
//			if(log_line.buff_index > FILE_MAX_SIZE - 1 && strlen(log_line.buff) >= FILE_MAX_SIZE - 1){
//				uart_debug_print("log_line.buff is full!\r\n");
//				log_line.buff_index = 0;
////				if(is_file_ready_to_upload == 0){
//					qctl_file_write_state = FILE_STATE_WRITE_STATE_OPEN;
//					is_file_writing_allowed = 1;
////				}
//			}
//		}
//
////		uart_debug_print(log_line.buff);
////		uart_debug_print("\r\n");
//
//		free(line);
//	}

	return;
}

void file_delete(char* name)
{
//	qctl_can_pub = 0;
//	qctl_resp_is_pending = 1;
//	qctl_file_state = FILE_STATE_DELETE;
//
//	char bf[64];
//
//	if((files.file_num_index < FILE_MAX_NUM) && files.name[files.file_num_index][0] != NULL){
//		sprintf(bf, "AT+QFDEL=");
//		strcat(bf, name);
//		qctl_send_cmd(bf, 1, 300, 1);
//	}

	return;
}

void file_delete_all()
{
//	qctl_can_pub = 0;
//	qctl_resp_is_pending = 1;
	qctl_file_state = FILE_STATE_DELETE_ALL;

	qctl_send_cmd("AT+QFDEL=\"*\"", 1, 300, 1);

	return;
}

void file_clear_buffer()
{
	log_line.buff_index = 0;
	memset(log_line.buff, '\0', sizeof(log_line.buff));

	return;
}

void file_clear_names()
{
	for (uint8_t i = 0; i < FILE_MAX_NUM; ++i) {
		HAL_IWDG_Refresh(&hiwdg);
		files.handler[i] = 0;
		for(uint8_t k = 0; k < FILE_MAX_NAME_SIZE; k++){
			files.name[i][k] = '\0';
		}
	}
	files.file_num_index = 0;

	return;
}

void file_get_file_list()
{
	qctl_send_cmd("AT+QFLST=\"*\"", 1, 300, 1);

	return;
}

void file_open()
{
	qctl_mqtt_clk_set(); // update time for new file name to write every time!
	df_delay_ms(1, 250);

	char bf[100];
	char new_clk_str[21];
	strncpy(new_clk_str, (char*)qctl_clk_str, 21);
	new_clk_str[2] = '_';
	new_clk_str[5] = '_';
	new_clk_str[8] = '_';
	new_clk_str[11] = '_';
	new_clk_str[14] = '_';

	char new_file_name[31];
	sprintf(new_file_name, "\"log_");
	strcat(new_file_name, new_clk_str);
	strcat(new_file_name, ".txt\"");

	sprintf(bf, "AT+QFOPEN=");
	strcat(bf, new_file_name);
	strcat(bf, ",0");

//	qctl_can_pub = 0;
//	qctl_resp_is_pending = 1;

	qctl_send_cmd(bf, 1, 300, 1);
	OS_TIMER_Restart(&osTimer_quectel_state_sentinel);

	strncpy(new_file.name[new_file.file_num_index], new_file_name, FILE_MAX_NAME_SIZE);

	return;
}

void file_get_file_handlers()
{
//	qctl_can_pub = 0;
//	qctl_resp_is_pending = 1;

	qctl_file_state = FILE_STATE_GET_HANDLERS;

	qctl_send_cmd("AT+QFOPEN?", 1, 300, 1);

	return;
}

void file_get_size()
{
	qctl_send_cmd("AT+QFLST=\"*\"", 1, 300, 1);

	return;
}

void file_write()
{
	OS_TIMER_Restart(&osTimer_quectel_state_sentinel);

//	qctl_can_pub = 0;
//	qctl_resp_is_pending = 1;

	// AT+QFWRITE=1027<CR>
	if(new_file.handler[new_file.file_num_index] && strlen(log_line.buff) >= FILE_MAX_SIZE - 1){
		char bf[100];
		sprintf(bf, "AT+QFWRITE=%ld,%d,1", new_file.handler[new_file.file_num_index], FILE_MAX_SIZE - 1);			// file handler, total file size, timeout = 1 second
		qctl_send_cmd(bf, 1, 300, 1);
	}
	else{
		OS_TIMER_Stop(&osTimer_quectel_state_sentinel);
		uart_debug_print("file_write() >>> no file handler set!!\r\n");
	}

	return;
}

void file_close(uint32_t file_handler)
{
	char bf[100];
	sprintf(bf, "AT+QFCLOSE=%ld", file_handler);
	qctl_send_cmd(bf, 1, 300, 1);

	return;
}

void file_write_log()
{
	switch(qctl_file_write_state){
		case FILE_STATE_WRITE_STATE_OPEN:{
			file_open();
			break;
		}
		case FILE_STATE_WRITE_STATE_WRITE:{
			file_write();
			break;
		}
		case FILE_STATE_WRITE_STATE_CLOSE:{
			file_close(files.handler[files.file_num_index]);
			break;
		}
		default:{
			// IDLE
			break;
		}
	}

	return;
}

void file_upload_to_ftp()
{
	switch(qctl_ftp_state){
		case FTP_STATE_CONFIG:{
			file_ftp_config();
			break;
		}
		case FTP_STATE_CONNECTION:{
			uart_debug_print("file_upload_to_ftp >> FTP_STATE_CONNECTION >> wrong path to come!\r\n");
//			file_ftp_open();
			break;
		}
		case FTP_STATE_CHECK_DIR:{
			file_ftp_check_dir();
			break;
		}
		case FTP_STATE_MKDIR:{
			file_ftp_make_dir();
			break;
		}
		case FTP_STATE_UPLOAD:{
			file_upload();
			break;
		}
		default:{
			// IDLE => 0
			break;
		}
	}

	return;
}

void file_ftp_config(){
	switch(qctl_ftp_config_state){
		case FTP_STATE_CONFIG_STATE_SET_USER:{
			qctl_ftp_set_user();
			break;
		}
		case FTP_STATE_CONFIG_STATE_SET_PASS:{
			qctl_ftp_set_pass();
			break;
		}
//		case FTP_STATE_CONFIG_STATE_OPEN_FTP:{
//			file_ftp_open();
//			break;
//		}
		case FTP_STATE_CONFIG_STATE_SET_PATH:{
			qctl_ftp_set_path();
			break;
		}
		case FTP_STATE_CONFIG_STATE_SET_LINK:{
			qctl_ftp_set_link();
			break;
		}
		default:{
			// IDLE
			break;
		}
	}
	return;
}

void file_ftp_open()
{
//	if(qctl_ftp_open_state == 0){
	qctl_ftp_open();
//	}

	return;
}

void file_ftp_make_dir()
{
//	if(qctl_ftp_open_state == 2){
	qctl_ftp_make_dir();
//	}

	return;
}

void file_ftp_check_dir()
{
//	if(qctl_ftp_open_state == 2 && qctl_ftp_is_dir_exist == 0){
	qctl_ftp_check_dir();
//	}

	return;
}

void file_upload()
{
//	if(qctl_ftp_open_state == 2 && qctl_ftp_upload_state == 0){
	qctl_ftp_put_file();
//	}

	return;
}

