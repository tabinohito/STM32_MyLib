/*
 * packet.hpp
 *
 *  Created on: 2020/09/24
 *      Author: tako
 */

#ifndef SRC_PACKET_HPP_
#define SRC_PACKET_HPP_

#include <array>
#include <vector>

template <typename tp>  // テンプレート共用体だって作れる
union packet {
	packet(tp _val) :val(_val){ }
	packet() : tp(0){ }
	~packet() {  }
	tp val;
	std::array<uint8_t,sizeof(tp)> buf = {};
};

template <typename T>
class com{
public:
	com() = delete;

	com(T temp):MyPacket(temp) { };

	T get() const { return MyPacket.val;};

	void rowGet(std::array<uint8_t , sizeof(T)> &buf) const { buf = MyPacket.buf; };
	void rowSet(std::array<uint8_t , sizeof(T)> &buf) { MyPacket.buf = buf; };

	void encode(std::vector<uint8_t> &buffer){
		buffer.push_back(BEGIN());

		for(size_t i = 0;i < MyPacket.buf.size();i++){
			if(MyPacket.buf.at(i) == END()){
				buffer.push_back(ESC());
				buffer.push_back(ESC_END());
			}
			else if(MyPacket.buf.at(i) == BEGIN()){
				buffer.push_back(ESC());
				buffer.push_back(ESC_BEGIN());
			}
			else if(MyPacket.buf.at(i) == ESC()){
				buffer.push_back(ESC());
				buffer.push_back(ESC_ESC());
			}
			else {
				buffer.push_back(MyPacket.buf.at(i));
			}
		}

		buffer.push_back(END());
	};

	void decode(std::vector<uint8_t> &buffer){
		std::vector<uint8_t> buffer_temp = {};
		if(buffer.front() == 0xC1) buffer.erase(buffer.begin()); //もし、Start Bitがいたら無視
		size_t i = 0;
		while(i < buffer.size()){
			if(buffer.at(i) == 0xC0){

			}
			else if(buffer.at(i) == 0xDB){
				i++;
				uint8_t next = buffer.at(i);

				if(next == 0xDC)  buffer_temp.push_back(0xC0);
				else if(next == 0xDD)  buffer_temp.push_back(0xDB);
				else if(next == 0xDE)  buffer_temp.push_back(0xC1);
			}
			else {
				buffer_temp.push_back(buffer.at(i));
			}
			i++;
		}

		std::copy(buffer_temp.begin(),buffer_temp.end(),MyPacket.buf.begin());
	};

	static constexpr uint8_t END() { return 0xC0;};
	static constexpr uint8_t BEGIN() { return 0xC1; };

private:
	packet<T> MyPacket;

	static constexpr uint8_t ESC() {return 0xDB;};
	static constexpr uint8_t ESC_END() {return 0xDC;};
	static constexpr uint8_t ESC_ESC() {return 0xDD;};
	static constexpr uint8_t ESC_BEGIN() {return 0xDE;};
};

#endif /* SRC_PACKET_HPP_ */
