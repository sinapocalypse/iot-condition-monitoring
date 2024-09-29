/*
 * microchipMonitoring.h
 *
 *  Created on: Jul 22, 2023
 *      Author: IroTeam
 */

#ifndef INC_MICROCHIPMONITORING_H_
#define INC_MICROCHIPMONITORING_H_

// ###################################################### Includes #########################################################
#include "e2.h"
#include "defines.h"
#include "adc.h"
// #########################################################################################################################

#define RESET_CAUSE_UNKNOWN								0
#define RESET_CAUSE_LOW_POWER_RESET						1
#define RESET_CAUSE_WINDOW_WATCHDOG_RESET				2
#define RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET			3
#define RESET_CAUSE_SOFTWARE_RESET						4
#define RESET_CAUSE_POWER_ON_POWER_DOWN_RESET			5
#define RESET_CAUSE_EXTERNAL_RESET_PIN_RESET			6

// ###################################################### Variables ########################################################
extern uint16_t adc_read;
extern float rawC;
extern float tempC;

extern float cm_temperature;
extern uint32_t cm_wtd_cnt;
extern uint32_t cm_swt_cnt;

extern uint8_t reset_status;
extern uint8_t reset_cause;

extern char reset_cause_msg[128];

// #########################################################################################################################

// ###################################################### Prototypes #######################################################
extern uint8_t cm_get_last_wtd();
extern void cm_read_reset_code();

extern void cm_create_io_buffer();
extern void cm_create_all_buffer();

extern void cm_get_chip_temp();
// #########################################################################################################################

#endif /* INC_MICROCHIPMONITORING_H_ */
