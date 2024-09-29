/*
 * IE2.h
 *
 *  Created on: Jun 6, 2023
 *      Author: IroTeam
 */

#ifndef INC_E2_H_
#define INC_E2_H_


// ###################################################### Includes #########################################################
#include "defines.h"
#include "i2c.h"
#include "iwdg.h"
#include "stm32f1xx_hal_flash_ex.h"
#include "microchipMonitoring.h"
#include "quectel.h"
#include "io.h"
// #########################################################################################################################


// ###################################################### Defines ########################################################
#define EE2_1_CHIP_ADDR       				0x56 << 1
#define EE2_2_CHIP_ADDR       				0x50 << 1
//#define ETX_APP_START_ADDRESS   			0x08008800

// ---------------------------- EXTERNAL EEPROM METADATA ----------------------------
#define EE2_METADATA_START_ADDRESS			0
#define EE2_Loc_MyId                        (EE2_METADATA_START_ADDRESS + 0)

// IO EXPANDERS INFO
#define EE2_Loc_IOX_NUM                     (EE2_METADATA_START_ADDRESS + 1)
#define EE2_Loc_START_IOX_ID                (EE2_METADATA_START_ADDRESS + 2)  // MAX : 5 IDs >> ADDRESS: IE2_METADATA_START_ADDRESS + 7

#define EE2_Loc_OUTPUTS_NUM					(EE2_METADATA_START_ADDRESS + 8)

#define EE2_Loc_DATE_YEAR					(EE2_METADATA_START_ADDRESS + 9)
#define EE2_Loc_DATE_MONTH					(EE2_METADATA_START_ADDRESS + 10)
#define EE2_Loc_DATE_DAY					(EE2_METADATA_START_ADDRESS + 11)
#define EE2_Loc_DATE_HOUR					(EE2_METADATA_START_ADDRESS + 12)
#define EE2_Loc_DATE_MINUTE					(EE2_METADATA_START_ADDRESS + 13)
#define EE2_Loc_DATE_SECOND					(EE2_METADATA_START_ADDRESS + 14)

#define EE2_Loc_LOG_SERVICE					(EE2_METADATA_START_ADDRESS + 15)
#define EE2_Loc_LOG_DIRECTORY_FLAG			(EE2_METADATA_START_ADDRESS + 16)

// Default Sensor Status
#define EE2_Loc_Default_Sensor_Status       (EE2_METADATA_START_ADDRESS + 19)

// FOTA METADATA
#define EE2_Loc_FOTA_STATUS                 (EE2_METADATA_START_ADDRESS + 20)
#define EE2_Loc_FOTA_CRC_BYTE0              (EE2_METADATA_START_ADDRESS + 21)
#define EE2_Loc_FOTA_CRC_BYTE1              (EE2_METADATA_START_ADDRESS + 22)
#define EE2_Loc_FOTA_CRC_BYTE2              (EE2_METADATA_START_ADDRESS + 23)
#define EE2_Loc_FOTA_CRC_BYTE3              (EE2_METADATA_START_ADDRESS + 24)
#define EE2_Loc_FOTA_App_SIZE_BYTE0         (EE2_METADATA_START_ADDRESS + 25)
#define EE2_Loc_FOTA_App_SIZE_BYTE1         (EE2_METADATA_START_ADDRESS + 26)
#define EE2_Loc_FOTA_App_SIZE_BYTE2         (EE2_METADATA_START_ADDRESS + 27)
#define EE2_Loc_FOTA_App_SIZE_BYTE3         (EE2_METADATA_START_ADDRESS + 28)
#define EE2_Loc_FOTA_ROW_START              (EE2_METADATA_START_ADDRESS + 29)

// WDT INFO
#define EE2_Loc_Wdt_Num_Byte3            	(EE2_METADATA_START_ADDRESS + 30)
#define EE2_Loc_Wdt_Num_Byte2            	(EE2_METADATA_START_ADDRESS + 31)
#define EE2_Loc_Wdt_Num_Byte1            	(EE2_METADATA_START_ADDRESS + 32)
#define EE2_Loc_Wdt_Num_Byte0            	(EE2_METADATA_START_ADDRESS + 33)

// SWT INFO
#define EE2_Loc_Swt_Num_Byte3            	(EE2_METADATA_START_ADDRESS + 34)
#define EE2_Loc_Swt_Num_Byte2            	(EE2_METADATA_START_ADDRESS + 35)
#define EE2_Loc_Swt_Num_Byte1            	(EE2_METADATA_START_ADDRESS + 36)
#define EE2_Loc_Swt_Num_Byte0            	(EE2_METADATA_START_ADDRESS + 37)

// RESET_STATUS INFO
#define EE2_Loc_Reset_Status             	(EE2_METADATA_START_ADDRESS + 38)
#define EE2_Loc_Reset_Code_Address         	(EE2_METADATA_START_ADDRESS + 39)

// MQTT INFO
#define EE2_Loc_EMQ_USERNAME				(EE2_METADATA_START_ADDRESS + 40)
#define EE2_Loc_EMQ_PASS					(EE2_METADATA_START_ADDRESS + 60)

// FTP INFO
#define EE2_Loc_FTP_USERNAME				(EE2_METADATA_START_ADDRESS + 80)
#define EE2_Loc_FTP_PASS					(EE2_METADATA_START_ADDRESS + 100)

// SENSOR INFO
#define EE2_Loc_Sen_Num						(EE2_METADATA_START_ADDRESS + 120)
#define EE2_Loc_Sen_Str						(EE2_METADATA_START_ADDRESS + 121)

// Internal FLASH Addresses
#define	IE2_FOTA_START_APP_ADDRESS			0x08040400
#define	IE2_FOTA_START_BOOT_ADDRESS			0x08000000

// Internal FLASH Sensor Commands Addresses
#define	IE2_SENSOR_CMD_START_ADDRESS		0x08008000
#define	IE2_METADATA_START_ADDRESS			0x08007800

#define METADATA_STRING_LENGTH				200

// ---------------------------- EXTERNAL EEPROM METADATAS ----------------------------
// ---------------------------- RESET REASON ----------------------------
#define Reset_Code_Server          		  	11
#define Reset_Code_FOTA_Fail				1
#define Reset_Code_FOTA_Done              	2
#define Reset_Code_Invalid_SiliconId      	3
#define Reset_Code_Mqtt_Open_Fail        	4
#define Reset_Code_Mqtt_Open_Resp_Fail   	5
#define Reset_Code_Mqtt_Conn_Resp_Fail   	6
#define Reset_Code_Mqtt_Sub_Resp_Fail    	7
#define Reset_Code_Mqtt_Pub_Resp_Fail    	8
#define Reset_Code_Quectel_Lost          	9
#define Reset_Code_I2C_Lost          	 	10
// ---------------------------- RESET REASON ----------------------------
// #########################################################################################################################
// ###################################################### Variables ########################################################
// #########################################################################################################################
// ###################################################### Prototypes #######################################################
// ###################################################### Internal FLASH ###################################################
extern uint16_t internal_flash_read_byte(uint32_t Flash_Address);
extern HAL_StatusTypeDef internal_flash_write_word(uint32_t Flash_Address, uint32_t data);
extern uint8_t internal_flash_erase();

extern uint32_t internal_flash_get_page(uint32_t Address);
extern uint32_t internal_flash_write_sensor(uint32_t *Data, uint32_t numberofwords);
extern void internal_flash_read_sensor();

extern uint32_t internal_flash_write_metadata();
extern void internal_flash_read_metadata();
// #########################################################################################################################
// #########################################################################################################################

#endif /* INC_E2_H_ */
