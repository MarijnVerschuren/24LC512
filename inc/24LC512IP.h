//
// Created by marijn on 1/12/23.
//

/* //
 * // Pinout
 * //
 *
 * // Diagram
 * 			  ______
 *		A0	-| *	|-	Vcc
 *		A1	-|		|-	WP
 *		A2	-|		|-	SCL
 *		Vss	-|______|-	SDA
 *
 * // Pins
 *	Ax:		I2C address pins
 *	Vss:	gnd
 *	Vcc:	2v5 - 5v5
 *	WP:		write protect (enabled when HIGH)
 *	SCL:	serial clock
 *	SDA:	serial data
 *
 * */

#ifndef MCU_24LC512IP_H
#define MCU_24LC512IP_H
#include "stdint.h"
#include "stddef.h"

#define ROM_CAPACITY 0xffff
#define ROM_PAGE_SIZE 32
#define ROM_I2C_BASE_ADDRESS 0x50


typedef uint8_t (*I2C_start_t)(uint8_t address);
typedef uint8_t (*I2C_request_t)(uint8_t address, uint32_t size);
typedef uint8_t (*I2C_write_t)(uint8_t byte);
typedef uint8_t (*I2C_write_buffer_t)(uint8_t* buffer, uint32_t size);
typedef uint8_t (*I2C_read_t)(void);
typedef uint8_t (*I2C_read_buffer_t)(uint8_t* buffer, uint32_t size);
typedef uint8_t (*I2C_end_t)(void);

typedef enum {
	success =					0x00u,
	addr_nack =					0x01u,
	data_nack =					0x02u,
	buffer_error =				0x04u,
	exceeded_rom_capacity =		0x08u,	// trying to write outside the rom capacity
	unexpected_size =			0x10u,	// received the wrong amount of bytes
	timeout =					0x20u,
	i2c_arbitration_error =		0x40u,
	i2c_bus_error =				0x80u	// any other error on the i2c bus
} _24LC512IP_StatusTypeDef;

typedef struct {
	uint8_t i2c_addr;
	uint32_t timeout;
} _24LC512IP_TypeDef;

// TODO: pass function pointers for initialization
// TODO: add this repo as a submodule for EEPROM_Programmer (merge that change to EEPROM_Programmer main)
void init_24LC512IP_lib(I2C_start_t I2C_start, I2C_request_t I2C_request, I2C_write_t I2C_write, I2C_write_buffer_t I2C_write_buffer, I2C_read_t I2C_read, I2C_read_buffer_t I2C_read_buffer, I2C_end_t I2C_end);
_24LC512IP_TypeDef* new_24LC512IP(uint8_t i2c_addr, uint32_t timeout);

_24LC512IP_StatusTypeDef rom_write(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t byte);
_24LC512IP_StatusTypeDef rom_read(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* byte);
_24LC512IP_StatusTypeDef i2c_stat(_24LC512IP_TypeDef* handle);
_24LC512IP_StatusTypeDef rom_write_buffer(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size, bool check = false);
_24LC512IP_StatusTypeDef rom_read_buffer(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size);
_24LC512IP_StatusTypeDef rom_write_buffer_check(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size);



#endif //MCU_24LC512IP_H