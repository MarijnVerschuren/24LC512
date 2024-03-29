//
// Created by marijn on 1/12/23.
//
#include "../inc/24LC512IP.h"


#define MIN(x, y) (((x) < (y)) ? (x) : (y))


I2C_start_t			I2C_start =			NULL;
I2C_request_t		I2C_request =		NULL;
I2C_write_t			I2C_write =			NULL;
I2C_write_buffer_t	I2C_write_buffer =	NULL;
I2C_read_t			I2C_read =			NULL;
I2C_read_buffer_t	I2C_read_buffer =	NULL;
I2C_end_t			I2C_end =			NULL;
delay_t				delay_ms =			NULL;
get_tick_t			get_tick =			NULL;


void init_24LC512IP_lib(
		I2C_start_t			_I2C_start,
		I2C_request_t		_I2C_request,
		I2C_write_t			_I2C_write,
		I2C_write_buffer_t	_I2C_write_buffer,
		I2C_read_t			_I2C_read,
		I2C_read_buffer_t	_I2C_read_buffer,
		I2C_end_t			_I2C_end,
		delay_t				_delay_ms,
		get_tick_t			_get_tick
) {
	I2C_start =			_I2C_start;
	I2C_request =		_I2C_request;
	I2C_write =			_I2C_write;
	I2C_write_buffer =	_I2C_write_buffer;
	I2C_read =			_I2C_read;
	I2C_read_buffer =	_I2C_read_buffer;
	I2C_end =			_I2C_end;
	delay_ms =			_delay_ms;
	get_tick =			_get_tick;
}
_24LC512IP_TypeDef* new_24LC512IP(uint8_t i2c_addr, uint32_t timeout) {
	_24LC512IP_TypeDef* handle = (_24LC512IP_TypeDef*)malloc(sizeof(_24LC512IP_TypeDef));
	handle->i2c_addr = i2c_addr;
	handle->timeout = timeout;
	return handle;
}
_24LC512IP_StatusTypeDef rom_write(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t byte) {
	if (!I2C_start || !I2C_write || !I2C_end) { return lib_error; }  // lib uninitialized
	I2C_start(handle->i2c_addr);
	I2C_write(rom_addr >> 8);
	I2C_write(rom_addr & 0xff);
	I2C_write(byte);
	return (_24LC512IP_StatusTypeDef)I2C_end();
}
_24LC512IP_StatusTypeDef rom_read(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* byte) {
	if (!I2C_start || !I2C_write || !I2C_request || !I2C_read || !I2C_end) { return lib_error; }  // lib uninitialized
	I2C_start(handle->i2c_addr);
	I2C_write(rom_addr >> 8);
	I2C_write(rom_addr & 0xff);
	_24LC512IP_StatusTypeDef stat = (_24LC512IP_StatusTypeDef)I2C_end();
	I2C_request(handle->i2c_addr, 1u);
	*byte = I2C_read();
	return stat;
}
_24LC512IP_StatusTypeDef i2c_stat(_24LC512IP_TypeDef* handle) {
	if (!I2C_start || !I2C_end) { return lib_error; }  // lib uninitialized
	I2C_start(handle->i2c_addr);
	return (_24LC512IP_StatusTypeDef)I2C_end();
}

_24LC512IP_StatusTypeDef rom_write_buffer(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size, uint8_t check) {
	if (!I2C_start || !I2C_write || !I2C_write_buffer || !I2C_request || !I2C_read || !I2C_read_buffer || !I2C_end || !delay_ms || !get_tick) { return lib_error; }  // lib uninitialized
	if (rom_addr + size - 1 > ROM_CAPACITY) { return exceeded_rom_capacity; }
	if (!buffer) { return buffer_error; }
	const uint16_t start_addr = rom_addr;
	const uint16_t start_size = size;
	uint8_t* start_buffer = buffer;
	_24LC512IP_StatusTypeDef stat;
	while (size > 0) {
		uint8_t n = MIN(ROM_PAGE_SIZE - (rom_addr % ROM_PAGE_SIZE), size);
		I2C_start(handle->i2c_addr);
		I2C_write(rom_addr >> 8);
		I2C_write(rom_addr & 0xff);
		I2C_write_buffer(buffer, n);
		stat = (_24LC512IP_StatusTypeDef)I2C_end();
		if ((uint8_t)stat) { return stat; }

		buffer += n;
		rom_addr += n;
		size -= n;

		delay_ms(5);  // typical page write takes 5ms
		uint32_t start = get_tick();
		while (i2c_stat(handle)) {
			delay_ms(1);  // prevent spamming rom
			if (get_tick() - start > handle->timeout) { return timeout; }
		}
	}
	if (check) {
		delay_ms(5);  // wait an other 5 ms just to be sure
		return rom_write_buffer_check(handle, start_addr, start_buffer, start_size);
	}
	return success;
}
_24LC512IP_StatusTypeDef rom_read_buffer(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size) {
	if (!I2C_start || !I2C_write || !I2C_request || !I2C_read || !I2C_read_buffer || !I2C_end) { return lib_error; }  // lib uninitialized
	if (rom_addr + size - 1 > ROM_CAPACITY) { return exceeded_rom_capacity; }
	if (!buffer) {  // set pointer
		I2C_start(handle->i2c_addr);
		I2C_write(rom_addr >> 8);
		I2C_write(rom_addr & 0xff);
		return (_24LC512IP_StatusTypeDef)I2C_end();
	}
	_24LC512IP_StatusTypeDef stat;
	while (size > 0) {
		uint8_t n = MIN(ROM_PAGE_SIZE - (rom_addr % ROM_PAGE_SIZE), size);
		I2C_start(handle->i2c_addr);
		I2C_write(rom_addr >> 8);
		I2C_write(rom_addr & 0xff);
		stat = (_24LC512IP_StatusTypeDef)I2C_end();
		I2C_request(handle->i2c_addr, n);
		if ((uint8_t)stat) { return stat; }
		I2C_read_buffer(buffer, n);

		buffer += n;
		rom_addr += n;
		size -= n;
	}
	return success;
}

_24LC512IP_StatusTypeDef rom_write_buffer_check(_24LC512IP_TypeDef* handle, uint16_t rom_addr, uint8_t* buffer, uint16_t size) {
	if (!delay_ms) { return lib_error; }
	uint8_t* check_buffer = malloc(size);
	if (!check_buffer) { return buffer_error; }
	_24LC512IP_StatusTypeDef stat;
	stat = rom_read_buffer(handle, rom_addr, check_buffer, size);
	if (stat) { return stat; }
	for (uint16_t i = 0; i < size; i++) {
		if (buffer[i] != check_buffer[i]) {
			stat = rom_write(handle, rom_addr + i, buffer[i]);
			if (stat) { return stat; }
			delay_ms(5);  // write takes 5ms typically
		}
	}
	free(check_buffer);
	return success;
}