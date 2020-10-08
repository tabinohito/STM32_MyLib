/*
 * IOPin.hpp
 *
 *  Created on: Jun 2, 2020
 *      Author: tako
 */

#ifndef IOPIN_HPP_
#define IOPIN_HPP_


class IOPin{
	public:
		IOPin() = delete;

		IOPin(GPIO_TypeDef* port_,uint16_t pin_):port(port_),pin(pin_){

		};

		inline void write(bool state) const{port->BSRR = pin << (state ? 0 : 16);}//HAL_GPIO_WritePin(port, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
		inline bool read() const{return (port->IDR & pin) != 0;}//HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET;
		inline void toggle() const{port->ODR ^= pin;}//HAL_GPIO_TogglePin(port, pin);

		~IOPin(){

		};

	private:
		GPIO_TypeDef* const	port;
		const uint16_t		pin;
};


#endif /* IOPIN_HPP_ */
