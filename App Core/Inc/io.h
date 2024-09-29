/*
 * io.h
 *
 *  Created on: Jul 15, 2023
 *      Author: IroTeam
 */

#ifndef INC_IO_H_
#define INC_IO_H_

// ###################################################### Includes #########################################################
#include "e2.h"
#include "defines.h"
// #########################################################################################################################
// IO_EXPANDER
#define	IOX_ID_INVALID						16

//IO_Expanders_Chip_Address
#define IO1_ADDR	0x21 << 1 //P0 and P1 input
#define IO2_ADDR	0x22 << 1 //P0 input P1 output
#define IO3_ADDR	0x23 << 1 //P0 and P1 output

// ###################################################### Variables #######################################################
extern uint8_t iox_id;
extern uint8_t iox_id_array[16];
// ###################################################### Variables #######################################################

// ###################################################### Prototypes #######################################################
extern void io_init();
extern void io_read_inputs();
extern void io_read_outputs();

extern void io_read_io_expanders();

extern void io_write_outputs();
extern void io_write_io_expanders();

extern void io_proc();

extern void io_set_iox_id(char* msg);
extern void io_find_io_expander();

extern void io_set_enable(char* msg);
extern void io_set_out(char* msg);
extern uint32_t io_hex_to_bin(char* hexString);
// #########################################################################################################################

#endif /* INC_IO_H_ */
