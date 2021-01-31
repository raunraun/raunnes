#pragma once

#include <cstdint>

namespace raunnes {
class CPUCore6502State {
	uint8_t A;
	uint8_t X;
	uint8_t Y;

	union {
		uint8_t S;
		struct {
			uint8_t C : 1;
			uint8_t Z : 1;
			uint8_t I : 1;
			uint8_t D : 1;
			uint8_t B : 1;
			uint8_t U : 1;
			uint8_t V : 1;
			uint8_t N : 1;
		};
	};

	uint16_t PC;
	uint16_t SP;

};

class CPUCore6502 {
public:
	CPUCore6502();
	~CPUCore6502();

public:
	CPUCore6502(const CPUCore6502&) = delete;
	CPUCore6502& operator=(const CPUCore6502&) = delete;

public:


private:
	CPUCore6502State m_State;

};
}