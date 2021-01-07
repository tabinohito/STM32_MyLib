/*
 * Mechatro_Utility.hpp
 *
 *  Created on: Dec 31, 2020
 *      Author: tako
 */

#ifndef SRC_MECHATRO_UTILITY_HPP_
#define SRC_MECHATRO_UTILITY_HPP_

#include "Serial.hpp"
#include "cassert"
#include "vector"
#include "numeric"

class SBDBT : public Serial{
public:
	SBDBT() = delete;
	SBDBT(UART_HandleTypeDef _huart):Serial(_huart,2400){

	};

	inline bool update(){
		if(Serial::available() > 9){
			if(Serial::read() == 0x80){
				std::array<uint8_t,7> sbdbt_data;
				for(size_t i = 0;i < 7;i++){
					sbdbt_data.at(i) = Serial::read();
				}

				uint8_t check_sum = static_cast<uint8_t>(std::accumulate(sbdbt_data.begin(), std::next(sbdbt_data.end(), -1), 0));
				if((check_sum &  0x7F) == (sbdbt_data.at(6) & 0x7F)){
					UP_KEY = ((0x01) & sbdbt_data.at(1)) && (!((0x02) & sbdbt_data.at(1))) ?true:false;
					DOWN_KEY = (!((0x01) & sbdbt_data.at(1))) && ((0x02) & sbdbt_data.at(1))?true:false;
					RIGHT_KEY = ((0x04) & sbdbt_data.at(1)) && (!((0x08) & sbdbt_data.at(1)))?true:false;
					LEFT_KEY = (!((0x04) & sbdbt_data.at(1)))&&((0x08) & sbdbt_data.at(1))?true:false;
					TRIANGLE_KEY = ((0x10) & sbdbt_data.at(1))?true:false;
					CROSS_KEY = ((0x020) & sbdbt_data.at(1))?true:false;
					CIRCLE_KEY = ((0x40) & sbdbt_data.at(1))?true:false;
					SQUARE_KEY = ((0x01) & sbdbt_data.at(0))?true:false;
					L1_KEY = ((0x02) & sbdbt_data.at(0))?true:false;
					L2_KEY = ((0x04) & sbdbt_data.at(0))?true:false;
					R1_KEY = ((0x08) & sbdbt_data.at(0))?true:false;
					R2_KEY = ((0x10) & sbdbt_data.at(0))?true:false;
					START_KEY = ((0x01) & sbdbt_data.at(1)) && ((0x02) & sbdbt_data.at(1))?true:false;
					SELECT_KEY = ((0x04) & sbdbt_data.at(1)) && ((0x08) & sbdbt_data.at(1))?true:false;

					joyL_x = (static_cast<float>(static_cast<int16_t>(sbdbt_data.at(2)) - 0x40) / 0x40);
					joyL_y = (static_cast<float>(static_cast<int16_t>(sbdbt_data.at(3)) - 0x40) / 0x40) * -1;
					joyR_x = (static_cast<float>(static_cast<int16_t>(sbdbt_data.at(4)) - 0x40) / 0x40);
					joyR_y = (static_cast<float>(static_cast<int16_t>(sbdbt_data.at(5)) - 0x40) / 0x40) * -1;

					return true;
				}
			}
		}
		return false;
	};

	inline bool UP() const { return UP_KEY; };
	inline bool DOWN() const { return DOWN_KEY; };
	inline bool RIGHT() const { return RIGHT_KEY; };
	inline bool LEFT() const { return LEFT_KEY; };
	inline bool TRIANGLE() const { return TRIANGLE_KEY; };
	inline bool CROSS() const { return CROSS_KEY; };
	inline bool CIRCLE() const { return CIRCLE_KEY; };
	inline bool SQUARE() const { return SQUARE_KEY; };
	inline bool L1() const { return L1_KEY; };
	inline bool L2() const { return L2_KEY; };
	inline bool R1() const { return R1_KEY; };
	inline bool R2() const { return R2_KEY; };
	inline bool START() const { return START_KEY; };
	inline bool SELECT() const { return SELECT_KEY; };
	inline float R_x() const { return joyR_x; };
	inline float R_y() const { return joyR_y; };
	inline float L_x() const { return joyL_x; };
	inline float L_y() const { return joyL_y; };

private:
	bool UP_KEY = false;
	bool DOWN_KEY = false;
	bool RIGHT_KEY = false;
	bool LEFT_KEY = false;
	bool TRIANGLE_KEY = false;
	bool CROSS_KEY = false;
	bool CIRCLE_KEY = false;
	bool SQUARE_KEY = false;
	bool L1_KEY = false;
	bool L2_KEY = false;
	bool R1_KEY = false;
	bool R2_KEY = false;
	bool START_KEY = false;
	bool SELECT_KEY = false;
	float joyR_x = 0.0f;
	float joyR_y = 0.0f;
	float joyL_x = 0.0f;
	float joyL_y = 0.0f;
};



#endif /* SRC_MECHATRO_UTILITY_HPP_ */
