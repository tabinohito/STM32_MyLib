/*
 * SysTick_interrupt.hpp
 *
 *  Created on: Mar 15, 2020
 *      Author: tako
 */

#ifndef SYSTICK_INTERRUPT_HPP_
#define SYSTICK_INTERRUPT_HPP_

#include <functional>
#include <map>

class SysTick_interrupt{
public:
	SysTick_interrupt() = delete;
	SysTick_interrupt(const std::function<void(void)>&& addFunc,TIM_HandleTypeDef* tim):tim_(tim){
		callFunctions_.insert(std::make_pair(this,addFunc));
		HAL_TIM_Base_Start_IT(tim_);
	}

	static void update(){
		for(auto i:callFunctions_){i.second();}
	}

	virtual ~SysTick_interrupt(){
		SysTick_interrupt::callFunctions_.erase(this);
	}
private:
	static inline std::map<SysTick_interrupt* const,const std::function<void(void)>> callFunctions_;
	TIM_HandleTypeDef* const tim_;
};

extern "C"{
	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{
		SysTick_interrupt::update();
	}
}


#endif /* SYSTICK_INTERRUPT_HPP_ */
