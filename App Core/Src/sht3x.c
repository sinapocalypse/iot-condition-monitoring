#include "sht3x.h"
#include "defines.h"
#include <assert.h>


sht3x_handle_t handle;
uint16_t sht3x_temperature_1 = 0;
uint16_t sht3x_humidity_1 = 0;

uint16_t sht3x_temperature_2 = 0;
uint16_t sht3x_humidity_2 = 0;

/**
 * Registers addresses.
 */
typedef enum
{
	SHT3X_COMMAND_MEASURE_HIGHREP_STRETCH = 0x2c06,
	SHT3X_COMMAND_CLEAR_STATUS = 0x3041,
	SHT3X_COMMAND_SOFT_RESET = 0x30A2,
	SHT3X_COMMAND_HEATER_ENABLE = 0x306d,
	SHT3X_COMMAND_HEATER_DISABLE = 0x3066,
	SHT3X_COMMAND_READ_STATUS = 0xf32d,
	SHT3X_COMMAND_FETCH_DATA = 0xe000,
	SHT3X_COMMAND_MEASURE_HIGHREP_10HZ = 0x2737,
	SHT3X_COMMAND_MEASURE_LOWREP_10HZ = 0x272a
} sht3x_command_t;

static uint8_t calculate_crc(const uint8_t *data, size_t length)
{
	uint8_t crc = 0xff;
	for (size_t i = 0; i < length; i++) {
		crc ^= data[i];
		for (size_t j = 0; j < 8; j++) {
			if ((crc & 0x80u) != 0) {
				crc = (uint8_t)((uint8_t)(crc << 1u) ^ 0x31u);
			} else {
				crc <<= 1u;
			}
		}
	}
	return crc;
}

static bool sht3x_send_command(sht3x_handle_t *handle, sht3x_command_t command)
{
	uint8_t command_buffer[2] = {(command & 0xff00u) >> 8u, command & 0xffu};

	if (HAL_I2C_Master_Transmit(handle->i2c_handle, handle->device_address << 1u, command_buffer, sizeof(command_buffer),
	                            SHT3X_I2C_TIMEOUT) != HAL_OK) {
		return false;
	}

	return true;
}

static uint16_t uint8_to_uint16(uint8_t msb, uint8_t lsb)
{
	return (uint16_t)((uint16_t)msb << 8u) | lsb;
}

bool sht3x_init(sht3x_handle_t *handle)
{
	assert(handle->i2c_handle->Init.NoStretchMode == I2C_NOSTRETCH_DISABLE);
	// TODO: Assert i2c frequency is not too high

	//TEST
	HAL_StatusTypeDef status = HAL_ERROR;
	status = HAL_I2C_IsDeviceReady(&hi2c1, handle->device_address << 1u, 0x00000001, 500);
	if(status != HAL_OK){
//		uart_debug_print("sht3x >> I2C >> NOT OK!\r\n", 1);
		MX_I2C1_Init();
		return false;
	}
	else{
//		uart_debug_print("sht3x >> I2C >> OK\r\n", 1);
	}

	uint8_t status_reg_and_checksum[3];
	if (HAL_I2C_Mem_Read(handle->i2c_handle, handle->device_address << 1u, SHT3X_COMMAND_READ_STATUS, 2, (uint8_t*)&status_reg_and_checksum,
					  sizeof(status_reg_and_checksum), SHT3X_I2C_TIMEOUT) != HAL_OK) {
		return false;
	}

	uint8_t calculated_crc = calculate_crc(status_reg_and_checksum, 2);

	if (calculated_crc != status_reg_and_checksum[2]) {
		return false;
	}

	return true;
}

bool sht3x_read_temperature_and_humidity(sht3x_handle_t *handle, uint16_t *temperature, uint16_t *humidity)
{
	if(!sht3x_send_command(handle, SHT3X_COMMAND_MEASURE_HIGHREP_STRETCH)){
		uart_debug_print("SHT3 Sensor: Error sending command to sensor!\r\n");
		return false;
	}

	// TODO: Last modified by SINA!!!
	HAL_Delay(20);

	uint8_t buffer[6];
	if (HAL_I2C_Master_Receive(handle->i2c_handle, handle->device_address << 1u, buffer, sizeof(buffer), 1000) != HAL_OK) {
		uart_debug_print("SHT3 Sensor: Error receiving data from sensor!\r\n");
		return false;
	}

	uint8_t temperature_crc = calculate_crc(buffer, 2);
	uint8_t humidity_crc = calculate_crc(buffer + 3, 2);

	if (temperature_crc != buffer[2] || humidity_crc != buffer[5]) {
		uart_debug_print("SHT3 Sensor: Error CRC!\r\n");
		return false;
	}

	uint16_t temperature_raw = uint8_to_uint16(buffer[0], buffer[1]);
	uint16_t humidity_raw = uint8_to_uint16(buffer[3], buffer[4]);

	float temp = 0.0f;
	float hum = 0.0f;

	temp = (-45.0f + 175.0f * (float)temperature_raw / 65535.0f) * 10.0f;
	hum = (100.0f * (float)humidity_raw / 65535.0f) * 10.0f;

	*temperature = (short int)temp;	// needs to multiply by 10 for sever side!
	*humidity = (short int)hum;					// needs to multiply by 10 for sever side!

	// TODO: Last modified by SINA!!!
	if(!sht3x_send_command(handle, SHT3X_COMMAND_SOFT_RESET)){
		uart_debug_print("SHT3 Sensor: Error SOFT_RESET sensor!\r\n");
		return false;
	}

	return true;
}

bool sht3x_set_heater_enable(sht3x_handle_t *handle, bool enable)
{
	if (enable) {
		return sht3x_send_command(handle, SHT3X_COMMAND_HEATER_ENABLE);
	} else {
		return sht3x_send_command(handle, SHT3X_COMMAND_HEATER_DISABLE);
	}
}
