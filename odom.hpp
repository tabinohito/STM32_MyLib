/*
 * odom.hpp
 *
 *  Created on: 2020/03/17
 *      Author: tako
 */

#ifndef ODOM_HPP_
#define ODOM_HPP_

#include <cassert>
#include "cmath"
#include <tuple>

template <typename _tp>
class odom{
public:
	odom() = delete;

	odom(float _radius,int32_t _pulse,uint8_t _wheel,float _yaw):radius(_radius),pulse(_pulse),wheel(_wheel),offset(_yaw){

	}

	void update(const int32_t dt,const int32_t pulseX,const int32_t pulseY){
		assert(wheel == 2);
		float dt_t = (float)dt;

		poseX += ((2.0f * M_PI * radius / 1000) * (float)(pulseX)) / (float)(pulse);
		poseY += ((2.0f * M_PI * radius / 1000) * (float)(pulseY)) / (float)(pulse);;

		speedX = poseX / (dt_t / 1000);
		speedY = poseY / (dt_t / 1000);
	}

	//なんかおかしい
	void update(const int32_t dt,const int32_t wheelA,const int32_t wheelB,const int32_t wheelC){
		assert(wheel == 3);
		float dt_t = (float)dt;

		float x1 = ((2.0f * M_PI * radius / 1000) * (float)(wheelA)) / (float)(pulse); //mile meter
		float x2 = ((2.0f * M_PI * radius / 1000) * (float)(wheelB)) / (float)(pulse); //mile meter
		float x3 = ((2.0f * M_PI * radius / 1000) * (float)(wheelC)) / (float)(pulse); //mile meter

		float x = to_x(x1,x2,x3);
		float y = to_y(x1,x2,x3);

		poseX += x;
		poseY += y;

		speedX = poseX / (dt_t / 1000);
		speedY = poseY / (dt_t / 1000);
	}

	inline void updateImu(const float imu){omega = imu - offset;}

	inline std::tuple<_tp,_tp,float> pose(){
		return std::forward_as_tuple(poseX,poseY,omega);
	}

	inline std::tuple<_tp,_tp> speed(){
		return std::forward_as_tuple(speedX,speedY);
	}

	~odom(){}

private:
	static constexpr float sqrt3 = 1.73205;
	auto to_y (auto y1,auto y2,auto y3){
		return (1/3) * ((y2 * sqrt3 * -1) + (y3 * sqrt3));
	};
	auto to_x (auto x1,auto x2,auto x3){
		return (1/3) * (2 * x1 - x2 - x3);
	};

	_tp poseX = 0; //Y mm
	_tp poseY = 0; //Y mm
	float omega = 0;

	_tp speedX = 0; //X m/s
	_tp speedY = 0; //Y m/s

	const float radius;//計測輪の半径
	const int32_t pulse;//1回転のパルス数
	const uint8_t wheel;//計測輪の数

	const float offset;
};

#endif /* ODOM_HPP_ */
