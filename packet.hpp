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

	void decode(std::vector<uint8_t> &buffer) {
		if(buffer.front() == BEGIN()) buffer.erase(buffer.begin()); //もし、Start Bitがいたら無視

		for(size_t i = 0;i < buffer.size();i++){
			if(buffer.at(i) == END()){

			}
			else if(buffer.at(i) == ESC()){
				uint8_t next = buffer.at(i + 1);

				if(next == ESC_END()) MyPacket.buf.at(i) = END();
				else if(next == ESC_ESC()) MyPacket.buf.at(i) = ESC();
				else if(next == ESC_BEGIN()) MyPacket.buf.at(i) = BEGIN();

				++i;
			}
			else {
				MyPacket.buf.at(i) = buffer.at(i);
			}
		}
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
