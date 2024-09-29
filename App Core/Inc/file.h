/*
 * file.h
 *
 *  Created on: Nov 21, 2023
 *      Author: IroTeam
 */

#ifndef INC_FILE_H_
#define INC_FILE_H_

// ###################################################### Includes #########################################################
#include "defines.h"
// #########################################################################################################################

// ###################################################### Constants #########################################################
#define FILE_MAX_SIZE					2048 + 1	 		// data + 1 <NUL>
#define FILE_MAX_NUM					1
#define FILE_MAX_NAME_SIZE				31	 				//e.g : "log_23_11_29_11_35_07+14.txt"<NUL> => len : 31

#define FILE_MAX_DELETE_FAILURE			5

#define FILE_STATE_IDLE					0
#define FILE_STATE_DELETE_ALL			1
#define FILE_STATE_DELETE				2
#define FILE_STATE_GET_LIST				3
#define FILE_STATE_GET_HANDLERS			4
#define FILE_STATE_WRITE_LOG			5

#define FILE_STATE_WRITE_STATE_IDLE		0
#define	FILE_STATE_WRITE_STATE_OPEN		1
#define FILE_STATE_WRITE_STATE_WRITE	2
#define FILE_STATE_WRITE_STATE_CLOSE	3


// #########################################################################################################################

// ###################################################### Variables #######################################################
extern char log_server_ip[20+1];
extern char log_server_port[20+1];
extern char log_server_user[20+1];
extern char log_server_pass[20+1];

extern uint8_t is_file_writing_allowed;
extern uint8_t is_file_ready_to_upload;
extern uint16_t file_size_counter;
extern uint8_t critical_file_number;
extern uint8_t file_delete_counter;

struct file_log_line {
	uint16_t buff_index;
	char buff[FILE_MAX_SIZE];
};
extern struct file_log_line log_line;

struct file {
	char name[FILE_MAX_NUM][FILE_MAX_NAME_SIZE];
	uint32_t handler[FILE_MAX_NUM];
	uint16_t file_num_index;
};
extern struct file files;
extern struct file new_file;

// ###################################################### Functions #######################################################

// ###################################################### Prototypes #######################################################
extern void file_init();
extern void file_fill_log_buff(char* msg);
extern void file_delete(char* name);
extern void file_delete_all();
extern void file_clear_buffer();
extern void file_clear_names();
extern void file_get_file_list();
extern void file_open();
extern void file_get_file_handlers();
extern void file_get_size();
extern void file_seek();
extern void file_write();
extern void file_read();
extern void file_close(uint32_t file_handler);

extern void file_check_state();
extern void file_write_log();

extern void file_upload_to_ftp();
extern void file_ftp_config();
extern void file_ftp_open();
extern void file_ftp_make_dir();
extern void file_ftp_check_dir();
extern void file_upload();

// #########################################################################################################################

#endif /* INC_FILE_H_ */
