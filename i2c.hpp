/*
 * i2c.hpp
 *
 *  Created on: Dec 25, 2020
 *      Author: tako
 */

#ifndef SRC_I2C_HPP_
#define SRC_I2C_HPP_

#include "i2c.h"
#include "stdbool.h"
#include "inttypes.h"
#include "vector"

class i2c {
public:
	i2c() = delete;

	i2c(I2C_HandleTypeDef* i2c_ch):_i2c(i2c_ch){

	};

	inline bool i2c_writeSingleByte(uint8_t addr, uint8_t data){
		if (HAL_I2C_Master_Transmit(_i2c, (uint16_t)addr << 1, &data, 1, (uint8_t) 100) != HAL_OK) return false;
		return true;
	};
	inline bool i2c_writeSingleByte(uint8_t addr, uint8_t data,uint16_t tim){
		if (HAL_I2C_Master_Transmit(_i2c, (uint16_t)addr << 1, &data, 1, tim) != HAL_OK) return false;
		return true;
	};
	inline bool i2c_writeByte(uint8_t addr, uint8_t* data){
		if (HAL_I2C_Master_Transmit(_i2c, (uint16_t)addr << 1,data, sizeof(data), 100) != HAL_OK)
			return false;
		return true;
	};
	inline bool i2c_writeByte(uint8_t addr, uint8_t* data,uint16_t tim){
		if (HAL_I2C_Master_Transmit(_i2c, (uint16_t)addr << 1,data, sizeof(data), tim) != HAL_OK)
			return false;
		return true;
	};
	inline bool i2c_readSingleByte(uint8_t addr, uint8_t* data){
		if (HAL_I2C_Master_Receive(_i2c,((uint16_t)addr << 1)|0x01,data,1, (uint8_t) 100) != HAL_OK){
			return false;
		}
		HAL_Delay(2);
		return true;
	};
	inline bool i2c_writeRegisterByte(uint8_t addr, uint8_t reg, uint8_t data){
		if (HAL_I2C_Mem_Write(_i2c, (uint16_t)addr << 1, (uint16_t) reg, I2C_MEMADD_SIZE_8BIT, &data, 1, (uint8_t) 100) != HAL_OK)
			return false;
		return true;
	};
	inline bool i2c_readRegister(uint8_t addr, uint8_t reg, uint8_t* pResult){
		return i2c_readData(addr, reg, pResult, 1);
	};
	inline bool i2c_readData(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size){
		if (HAL_I2C_Mem_Read(_i2c, (uint16_t)addr << 1, (uint16_t) reg, I2C_MEMADD_SIZE_8BIT, buffer, size, (uint8_t) 500) != HAL_OK){
			return false;
		}
		return true;
	};
	inline bool i2c_isDeviceReady(uint8_t addr){
		if (HAL_I2C_IsDeviceReady(_i2c, (uint16_t)addr << 1, 5, (uint32_t) 1000) != HAL_OK)
			return false;
		HAL_Delay(250);
		return true;
	};
	inline void i2c_isDeviceReady_get(std::vector<uint8_t> &addr)
	{
		addr.clear();
		for (uint8_t i=1; i<128; i++){
			if (i2c_isDeviceReady(i)){ // HAL_ERROR or HAL_BUSY or HAL_TIMEOUT
				addr.push_back(i);// Received an ACK at that address
			}
		}
	};

private:
	I2C_HandleTypeDef* const _i2c;
};

#endif /* SRC_I2C_HPP_ */
