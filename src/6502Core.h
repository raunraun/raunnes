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
            uint8_t P;
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
            P = Flag;
        }
    };

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

    struct InstructionDetails;
    struct DynamicExecutionInfo;

    typedef void (CPUCore6502::* ExecutionDelegate)(const CPUCore6502::DynamicExecutionInfo&);

    struct InstructionDetails {
        uint32_t AddresingMode;
        uint32_t InstructionSize;
        uint32_t CycleCount;
        uint32_t PageCrossCycleCost;
        char* Name;
        CPUCore6502::ExecutionDelegate Delegate;

    };
    struct DynamicExecutionInfo {
        DynamicExecutionInfo(InstructionDetails& d, uint8_t o, uint16_t pc) : 
            details(d), PC(pc) {
            InstructionBytes[0] = o; InstructionBytes[1] = 0; InstructionBytes[2] = 0;
        }
        InstructionDetails& details;
        uint16_t PC;
        uint8_t InstructionBytes[3];

        uint8_t Opcode() const { return InstructionBytes[0]; }
        uint16_t Address() const {
            switch (details.AddresingMode) {
            case(AddressingModeAbsolute):
                return ((uint16_t*)&InstructionBytes[1])[0];
                break;
            case(AddressingModeZeroPage):
                return InstructionBytes[1];
                break;
            default:
                return 0xffff;
            };
        }

        uint16_t AddressZeropage() const { return InstructionBytes[1]; }
        uint8_t Immediate() const { return InstructionBytes[1]; }
    };

    typedef void(*ExecutionCallBack)(const InstructionDetails&, const DynamicExecutionInfo&, const CPUCore6502State&, const MemoryMap&, const uint64_t cycles);

public:
    CPUCore6502(MemoryMap& mem);
    ~CPUCore6502();

    void InstallPreExecutionCallBack(ExecutionCallBack cb);
    void InstallPostExecutionCallBack(ExecutionCallBack cb);

    void Reset();
    void Execute();

    void Push(uint8_t val);
    void Push16(uint16_t val);
    uint8_t Pop();
    uint16_t Pop16();

    void SetC(uint8_t val);
    void SetN(uint8_t val);
    void SetZ(uint8_t val);
    void SetZ(bool val);
    void SetV(bool val);

    uint8_t& A();
    uint8_t& X();
    uint8_t& Y();
    uint16_t& PC();
    uint16_t& SP();

    uint8_t Value(const DynamicExecutionInfo& info);

    void AddBranchCycles(uint16_t oldPC, uint16_t newPC, uint32_t pageCrossCost);

    void Unimplemented(const DynamicExecutionInfo& info);

    void ADC(const DynamicExecutionInfo& info);
    void AND(const DynamicExecutionInfo& info);
    void BCC(const DynamicExecutionInfo& info);
    void BCS(const DynamicExecutionInfo& info);
    void BEQ(const DynamicExecutionInfo& info);
    void BIT(const DynamicExecutionInfo& info);
    void BMI(const DynamicExecutionInfo& info);
    void BNE(const DynamicExecutionInfo& info);
    void BPL(const DynamicExecutionInfo& info);
    void BVC(const DynamicExecutionInfo& info);
    void BVS(const DynamicExecutionInfo& info);
    void CLC(const DynamicExecutionInfo& info);
    void CLD(const DynamicExecutionInfo& info);
    void CLV(const DynamicExecutionInfo& info);
    void CPX(const DynamicExecutionInfo& info);
    void CPY(const DynamicExecutionInfo& info);
    void CMP(const DynamicExecutionInfo& info);
    void EOR(const DynamicExecutionInfo& info);
    void INX(const DynamicExecutionInfo& info);
    void INY(const DynamicExecutionInfo& info);
    void JMP(const DynamicExecutionInfo& info);
    void JSR(const DynamicExecutionInfo& info);
    void LDA(const DynamicExecutionInfo& info);
    void LDX(const DynamicExecutionInfo& info);
    void LDY(const DynamicExecutionInfo& info);
    void NOP(const DynamicExecutionInfo& info);
    void ORA(const DynamicExecutionInfo& info);
    void PHA(const DynamicExecutionInfo& info);
    void PHP(const DynamicExecutionInfo& info);
    void PLA(const DynamicExecutionInfo& info);
    void PLP(const DynamicExecutionInfo& info);
    void RTS(const DynamicExecutionInfo& info);
    void SBC(const DynamicExecutionInfo& info);
    void SEC(const DynamicExecutionInfo& info);
    void SEI(const DynamicExecutionInfo& info);
    void SED(const DynamicExecutionInfo& info);
    void STA(const DynamicExecutionInfo& info);
    void STX(const DynamicExecutionInfo& info);

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