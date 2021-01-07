/*
 * BNO055.hpp
 *
 *  Created on: Aug 23, 2020
 *      Author: tako
 */

#ifndef SRC_BNO055_HPP_
#define SRC_BNO055_HPP_

#include "i2c.hpp"
typedef struct{
	float x,y,z;
}EULAR;

typedef struct{
	float x,y,z,w;
}QUATERNION;

class BNO055{
public:
	BNO055() = delete;
	BNO055(i2c i2c_module,uint8_t device_address = 0x28):bno_i2c(i2c_module),device_address_(device_address){
		  bno_i2c.i2c_writeRegisterByte(0x28,0x3F,0x20);//bno055_reset()
		  HAL_Delay(700);
		  bno_i2c.i2c_writeRegisterByte(0x28,0x3D,0x08);
		  HAL_Delay(30);
	};

	inline QUATERNION get_quaternion(){
		uint8_t bno_receivedata[8];
		short quat[4];
		bno_i2c.i2c_readData(device_address_,bno_readquat_address,bno_receivedata, 8);

		quat[0] = bno_receivedata[1] << 8 | bno_receivedata[0];
		quat[1] = bno_receivedata[3] << 8 | bno_receivedata[2];
		quat[2] = bno_receivedata[5] << 8 | bno_receivedata[4];
		quat[3] = bno_receivedata[7] << 8 | bno_receivedata[6];

		QUATERNION q = { (float)((float)quat[1]/16384.0),(float)((float)quat[2]/16384.0),(float)((float)quat[3]/16384.0),(float)((float)quat[0]/16384.0) };
		return q;
	};

	inline EULAR get_eular(){
		uint8_t bno_receivedata[6] = {};
		EULAR xyz;

		bno_i2c.i2c_readData(device_address_,0x1A,bno_receivedata,6);

		xyz.z = (int16_t)((bno_receivedata[1] << 8) | bno_receivedata[0]) / 16;
		xyz.x = (int16_t)((bno_receivedata[3] << 8) | bno_receivedata[2]) / 16;
		xyz.y = (int16_t)((bno_receivedata[5] << 8) | bno_receivedata[4]) / 16;

		xyz.x -= offset.x;
		xyz.y -= offset.y;
		xyz.z -= offset.z;

		xyz.z = xyz.z > 180 ? xyz.z - 360:xyz.z;

		HAL_Delay(2);

		return xyz;
	};

	inline EULAR get_accel(){
		uint8_t bno_receivedata[6] = {};
		short e_raw[3] = {};
		const float div = 100.0;

		bno_i2c.i2c_readData(device_address_,bno_readaccel_address,bno_receivedata,6);

		e_raw[0] = bno_receivedata[1] << 8 | bno_receivedata[0];
		e_raw[1] = bno_receivedata[3] << 8 | bno_receivedata[2];
		e_raw[2] = bno_receivedata[5] << 8 | bno_receivedata[4];

		EULAR e = { (float)e_raw[0]/div,(float)e_raw[1]/div,(float)e_raw[2]/div };
		return e;

	};

	inline EULAR get_gyro(){
		uint8_t bno_receivedata[16] = {};
		short e_raw[3] = {};
		const float div = 16.0;

		bno_i2c.i2c_readData(device_address_,bno_readgyro_address,bno_receivedata,6);

		e_raw[0] = bno_receivedata[1] << 8 | bno_receivedata[0];
		e_raw[1] = bno_receivedata[3] << 8 | bno_receivedata[2];
		e_raw[2] = bno_receivedata[5] << 8 | bno_receivedata[4];

		EULAR e = { (float)e_raw[0]/div,(float)e_raw[1]/div,(float)e_raw[2]/div };
		return e;
	};

private:
	const uint8_t device_address_;
	i2c bno_i2c;
	EULAR offset;

	static constexpr uint8_t bno_reset_address = 0x3F;
	static constexpr uint8_t bno_readquat_address = 0x20;
	static constexpr uint8_t bno_readeular_address = 0x1A;
	static constexpr uint8_t bno_readaccel_address = 0x08;
	static constexpr uint8_t bno_readgyro_address = 0x14;
};


#endif /* SRC_BNO055_HPP_ */
