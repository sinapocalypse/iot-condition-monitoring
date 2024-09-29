/*
 * proc.h
 *
 *  Created on: Jul 22, 2023
 *      Author: IroTeam
 */

#ifndef INC_PROC_H_
#define INC_PROC_H_

// ###################################################### Headers ########################################################
#include "e2.h"
#include "defines.h"
#include "quectel.h"
#include "io.h"
#include "sht3x.h"
#include "microchipMonitoring.h"
#include "modbus.h"
#include "file.h"

#include "iwdg.h"

// #########################################################################################################################

#define FOTA_MAX_FILE_LINE                  30 // FOR STM .hex file
#define SHT3_ERR_MAX_COUNTER                100
#define MODBUS_ERR_MAX_COUNTER              20

// ###################################################### Variables ########################################################
extern uint16_t line_num;
extern uint16_t sht3_err_counter;
extern uint16_t modbus_err_counter;

extern unsigned int data_length;
extern uint8_t fota_data_bytes[16 * FOTA_MAX_FILE_LINE];
extern uint16_t fota_data_index;
extern int data_index;
// #########################################################################################################################

// ###################################################### Prototypes #######################################################
extern void proc_init();
	extern void proc_file_init();
	extern void proc_read_directory_existance();
    extern void proc_wdt();
    extern void proc_io_exp_id();
    extern void proc_read_output_num();
    extern void proc_init_modbus();
    	extern void proc_init_default_sensor();
	extern void proc_init_log_file();

extern void proc_datetime();
extern void proc_log_status();

extern void proc_task_io();
extern void proc_thermal_sensor_1();
extern void proc_thermal_sensor_2();

extern void proc_task_operator();
    extern void proc_task_operator_QueueSet();
        extern void proc_task_operator_QueueSet_separate(char* msg);
            extern void proc_task_operator_QueueSet_for_gateway();
    extern void proc_task_operator_get_ino_or_all();
        extern void proc_task_operator_get_all_for_gateway();
        extern void proc_task_operator_get_ino_for_gateway();
        extern void proc_task_operator_get_fota_stat_for_gateway();

extern void proc_task_io_sensor();

extern void proc_task_quectel();

extern void proc_reset_wdt(uint8_t reset);
extern void proc_reset_chip(uint8_t reset_code);

//extern void proc_rtc();
extern void proc_update_log_status(char* msg);

extern void proc_start_fota(uint8_t fota_flag, char* fota_data);

extern void proc_separate_fota_resp(char* data);
extern void proc_separate_gsm_resp(char* data, uint16_t lenwith0);

extern void proc_task_sep();

extern uint8_t global_flag;
extern uint16_t global_counter;
// #########################################################################################################################

#endif /* INC_PROC_H_ */
