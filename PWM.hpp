/*
 * PWMPin.hpp
 *
 *  Created on: Jun 2, 2020
 *      Author: tako
 */

#ifndef PWMPIN_HPP_
#define PWMPIN_HPP_

#include "tim.h"
#include "cmath"
#include "Serial.hpp"

enum class timMode{
	APB1,APB2
};

class PWM{
	public:
		PWM() = delete;

		PWM(TIM_HandleTypeDef* tim_,uint32_t ch_,uint16_t pTimeReload,int16_t pre_,timMode MyTim,bool interrupt = false):
		tim(tim_)
		,ch(ch_)
		,mTimeReload(pTimeReload)
		,MasterClock(timMode::APB1 == MyTim ? HAL_RCC_GetPCLK1Freq() : HAL_RCC_GetPCLK2Freq())
		{
			setPiriod(pTimeReload);
			setPrescaler(pre_);
			if(interrupt) HAL_TIM_Base_Start_IT(tim);
		};

		inline virtual void trigger(bool status){
			status == true ? HAL_TIM_PWM_Start_IT(tim,ch): HAL_TIM_PWM_Stop(tim,ch);
		};

		inline void Duty(int pulse){
			__HAL_TIM_SET_COMPARE(tim,ch,pulse);
		};

		//range 0.0 ~ 1.0
		inline HAL_StatusTypeDef fDuty(float pfDuty){
			if(!(0 <= pfDuty && pfDuty <= 1)) return HAL_ERROR;
			int pDuty = pfDuty * (float)mTimeReload;
			Duty(pDuty);
			return HAL_OK;
		};

		//range -1.0 ~ 1.0
		inline HAL_StatusTypeDef lapDuty(float pfDuty){
			if(!(0 <= std::fabs(pfDuty) && std::fabs(pfDuty) <= 1)) return HAL_ERROR;
			int pDuty = (pfDuty + 1)/2 * (float)mTimeReload;
			Duty(pDuty);
			return HAL_OK;
		};

		inline int16_t getPre() const{ return pre; }
		inline uint32_t getPiriod() const{ return mTimeReload; }
		inline uint32_t getClock() const{ return MasterClock; }

		inline void setPrescaler(const uint16_t pre_){
			pre = pre_;
			__HAL_TIM_SET_PRESCALER(tim,pre_);
		}

		inline void setPiriod(const uint16_t pTimeReload){
			mTimeReload = pTimeReload;
			__HAL_TIM_SET_AUTORELOAD(tim,mTimeReload);
		}

		~PWM(){

		};

	private:
		TIM_HandleTypeDef* const tim;
		const uint32_t ch;
		uint16_t mTimeReload;
		int16_t pre;
		const uint32_t MasterClock;
};

class servo{
public:
	servo() = delete;
	servo(PWM _MyPWM,uint16_t _max_us = 2100,uint16_t _min_us = 900):esc(_MyPWM),max_us(_max_us),min_us(_min_us){
		esc.setPiriod(10000);
		esc.setPrescaler(((esc.getClock() / esc.getPiriod()) / freaqency));
	}

	inline void setTime(uint16_t us){
		esc.trigger(false);
		if((min_us <= us) && (us <= max_us)){
			latest_us = us;
			float param = static_cast<float>(us) / 10000;
			esc.fDuty(param / 2);
		}
		esc.trigger(true);
	};

	//range 0 ~ 1.0
	inline void fset(float percent){
		esc.trigger(false);
		if((0 <= percent && percent <= 1)){
			setTime(((max_us - min_us) * percent) + min_us);
		}
		esc.trigger(true);
	};

	//range -1.0 ~ 1.0
	inline void lset(float percent){
		fset((percent + 1.0f) / 2.0);
	};

	inline uint16_t getTime() const { return latest_us; };

private:
	PWM esc;
	static constexpr auto freaqency = 50;
	const uint16_t max_us;
	const uint16_t min_us;
	uint16_t latest_us;
};

class speaker{
public:
	speaker() = delete;
	speaker(PWM _MyPWM):spk(_MyPWM){

	}

	inline void toneMode(const bool status){
		spkStatus = status;
		spk.trigger(status);
		if(!status) spk.Duty(0);
	}

	inline void tone(const int16_t freaqency){
		spk.trigger(false);
		int32_t result = (spk.getClock() / spk.getPre()) /freaqency;
		if(result != 0){
			spk.setPiriod(result);
			spk.Duty(result/2);
			spk.trigger(true);
		}
	}

	inline void toneNote(const uint8_t note){
		nowNote = note;
		tone(m2t[note]);
	}

	inline uint8_t onNote() const{ return nowNote; }

	inline bool spkMode() const{ return spkStatus; }

	inline void off(){ spk.trigger(false); }

private:
	PWM spk;
	bool spkStatus = false;
	uint8_t nowNote;
	int16_t m2t[127] = {
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			3,
			11,
			19,
			27,
			35,
			43,
			51,
			59,
			67,
			75,
			83,
			91,
			99,
			107,
			115,
			123,    //NOTE_B2
			131,    //NOTE_C3
			139,    //NOTE_CS3
			147,    //NOTE_D3
			156,    //NOTE_DS3
			165,    //NOTE_E3
			175,    //NOTE_F3
			185,    //NOTE_FS3
			196,    //NOTE_G3
			208,    //NOTE_GS3
			220,    //NOTE_A3
			233,    //NOTE_AS3
			247,    //NOTE_B3
			262,    //NOTE_C4
			277,    //NOTE_CS4
			294,    //NOTE_D4
			311,    //NOTE_DS4
			330,    //NOTE_E4
			349,    //NOTE_F4
			370,    //NOTE_FS4
			392,    //NOTE_G4
			415,    //NOTE_GS4
			440,    //NOTE_A4
			466,    //NOTE_AS4
			494,    //NOTE_B4
			523,    //NOTE_C5
			554,    //NOTE_CS5
			587,    //NOTE_D5
			622,    //NOTE_DS5
			659,    //NOTE_E5
			698,    //NOTE_F5
			740,    //NOTE_FS5
			784,    //NOTE_G5
			831,    //NOTE_GS5
			880,    //NOTE_A5
			932,    //NOTE_AS5
			988,    //NOTE_B5
			1047,   //NOTE_C6
			1109,   //NOTE_CS6
			1175,   //NOTE_D6
			1245,   //NOTE_DS6
			1319,   //NOTE_E6
			1397,   //NOTE_F6
			1480,   //NOTE_FS6
			1568,   //NOTE_G6
			1661,   //NOTE_GS6
			1760,   //NOTE_A6
			1865,   //NOTE_AS6
			1976,   //NOTE_B6
			2093,   //NOTE_C7
			2217,   //NOTE_CS7
			2349,   //NOTE_D7
			2489,   //NOTE_DS7
			2637,   //NOTE_E7
			2794,   //NOTE_F7
			2960,   //NOTE_FS7
			3136,   //NOTE_G7
			3322,   //NOTE_GS7
			3520,   //NOTE_A7
			3729,   //NOTE_AS7
			3951,   //NOTE_B7
			4186,   //NOTE_C8
			4435,   //NOTE_CS8
			4699,   //NOTE_D8
			4978    //NOTE_DS8
	};
};



#endif /* PWMPIN_HPP_ */
