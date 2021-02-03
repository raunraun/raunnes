#pragma once

#include <cstdint>

#include "MemoryMap.h"

namespace raunnes {

class CPUCore6502 {
public:
	class CPUCore6502State {
	public:
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

		void SetFlags(uint8_t Flag) {
			S = Flag;
		}
	};

	struct DynamicExecutionInfo {
		uint16_t PC;
		uint16_t Address;
	};

	typedef void (CPUCore6502::*ExecutionDelegate)(CPUCore6502::DynamicExecutionInfo&);

	enum AddressingMode {
		AddressingModeAbsolute = 1,
		AddressingModeAbsoluteX,
		AddressingModeAbsoluteY,
		AddressingModeAccumulator,
		AddressingModeImmediate,
		AddressingModeImplied,
		AddressingModeIndexedIndirect,
		AddressingModeIndirect,
		AddressingModeIndirectIndexed,
		AddressingModeRelative,
		AddressingModeZeroPage,
		AddressingModeZeroPageX,
		AddressingModeZeroPageY,
	};

	struct InstructionDetails {
		uint32_t AddresingMode;
		uint32_t InstructionSize;
		uint32_t CycleCount;
		uint32_t PageCrossCycleCost;
		char* Name;
		CPUCore6502::ExecutionDelegate Delegate;
	};

	typedef void(*ExecutionCallBack)(const InstructionDetails&, const DynamicExecutionInfo&, const CPUCore6502State& );

public:
	CPUCore6502(MemoryMap& mem);
	~CPUCore6502();

	void InstallPreExecutionCallBack(ExecutionCallBack cb);
	void InstallPostExecutionCallBack(ExecutionCallBack cb);

	void Reset();
	void Execute();

	void JMP(DynamicExecutionInfo& info);

public:
	CPUCore6502(const CPUCore6502&) = delete;
	CPUCore6502& operator=(const CPUCore6502&) = delete;

private:
	CPUCore6502State m_State;
	uint64_t m_Cycles;
	MemoryMap& m_Memory;
	ExecutionCallBack m_PreexecutionCallBack;
	ExecutionCallBack m_PostexecutionCallBack;

};
}