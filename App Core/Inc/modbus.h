/*
 * modbus.h
 *
 *  Created on: Jul 22, 2023
 *      Author: IroTeam
 */

#ifndef INC_MODBUS_H_
#define INC_MODBUS_H_

// ###################################################### Includes #########################################################
#include <e2.h>
#include "defines.h"
// #########################################################################################################################

#define Num_Max_Sen                     32
#define Len_Max_Sen_Send                25
#define Len_Max_Sen_Recv                21

// ###################################################### Variables ########################################################
// #########################################################################################################################

// ###################################################### Prototypes #######################################################

extern void modbus_proc(uint8_t number);
extern void modbus_send_ascii(uint8_t send_modbus[Len_Max_Sen_Send + 1]);

extern void modbus_send_rtu(uint8_t send_modbus[Len_Max_Sen_Send + 1]);
extern void modbus_find_row(uint8_t type);
extern void modbus_set_sen(char* msg);

// #########################################################################################################################

#endif /* INC_MODBUS_H_ */
