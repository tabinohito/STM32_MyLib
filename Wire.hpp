/*
 * Wire.hpp
 *
 *  Created on: Aug 23, 2020
 *      Author: tako
 */

#ifndef SRC_WIRE_HPP_
#define SRC_WIRE_HPP_

#include "i2c.h"
#include "stdbool.h"
#include "inttypes.h"

class Wire {
public:
	Wire() = delete;
	Wire(I2C_HandleTypeDef handler);
	inline HAL_StatusTypeDef writeSingleByte(uint8_t addr, uint8_t data);
	inline HAL_StatusTypeDef writeSingleByte(uint8_t addr, uint8_t data,uint16_t tim);
	inline HAL_StatusTypeDef writeByte(uint8_t addr, uint8_t* data);
	inline HAL_StatusTypeDef writeByte(uint8_t addr, uint8_t* data,uint16_t tim);
	inline HAL_StatusTypeDef readSingleByte(uint8_t addr, uint8_t* data);
	inline HAL_StatusTypeDef readByte(uint8_t addr, uint8_t* data);
	inline HAL_StatusTypeDef readRegister(uint8_t addr, uint8_t reg, uint8_t* pResult);
	inline HAL_StatusTypeDef isDeviceReady(uint8_t addr);
	inline HAL_StatusTypeDef writeRegisterByte(uint8_t addr, uint8_t reg, uint8_t data);
	inline HAL_StatusTypeDef readData(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size);

protected:
	I2C_HandleTypeDef myi2c;
};

Wire::Wire(I2C_HandleTypeDef handler):myi2c(handler)
{
	__HAL_I2C_RESET_HANDLE_STATE(&handler);
}


HAL_StatusTypeDef Wire::writeSingleByte(uint8_t addr, uint8_t data)
{
	return HAL_I2C_Master_Transmit(&myi2c, (uint16_t)addr << 1, &data, 1, (uint8_t) 100);
}

HAL_StatusTypeDef Wire::writeSingleByte(uint8_t addr, uint8_t data,uint16_t tim)
{
	return HAL_I2C_Master_Transmit(&myi2c, (uint16_t)addr << 1, &data, 1, tim);
}

HAL_StatusTypeDef Wire::writeByte(uint8_t addr, uint8_t* data)
{
	return HAL_I2C_Master_Transmit(&myi2c, (uint16_t)addr << 1,data, sizeof(data), 100);
}

HAL_StatusTypeDef Wire::writeByte(uint8_t addr, uint8_t* data,uint16_t tim)
{
	return HAL_I2C_Master_Transmit(&myi2c, (uint16_t)addr << 1,data, sizeof(data), tim);
}

HAL_StatusTypeDef Wire::readSingleByte(uint8_t addr, uint8_t* data)
{
	HAL_StatusTypeDef status = HAL_I2C_Master_Receive(&myi2c,((uint16_t)addr << 1)|0x01,data,1, (uint8_t) 100);
	if(status == HAL_OK) HAL_Delay(2);
	return status;
}

HAL_StatusTypeDef Wire::readByte(uint8_t addr, uint8_t* data)
{
	HAL_StatusTypeDef status = HAL_I2C_Master_Receive(&myi2c,((uint16_t)addr << 1)|0x01,data,sizeof(data), (uint8_t) 100);
	if(status == HAL_OK) HAL_Delay(2);
	return status;
}

HAL_StatusTypeDef  Wire::writeRegisterByte(uint8_t addr, uint8_t reg, uint8_t data)
{
	return HAL_I2C_Mem_Write(&myi2c, (uint16_t)addr << 1, (uint16_t) reg, I2C_MEMADD_SIZE_8BIT, &data, 1, (uint8_t) 100);
}

HAL_StatusTypeDef Wire::readRegister(uint8_t addr, uint8_t reg, uint8_t* pResult)
{
	return readData(addr, reg, pResult, 1);
}


HAL_StatusTypeDef Wire::readData(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size)
{
	return HAL_I2C_Mem_Read(&myi2c, (uint16_t)addr << 1, (uint16_t) reg, I2C_MEMADD_SIZE_8BIT, buffer, size, (uint8_t) 500);
}

HAL_StatusTypeDef Wire::isDeviceReady(uint8_t addr)
{
	HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&myi2c, (uint16_t)addr << 1, 5, (uint32_t) 1000);
	if(status == HAL_OK) HAL_Delay(250);
	return status;
}



#endif /* SRC_WIRE_HPP_ */
