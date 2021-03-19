#include "6502Core.h"
#include <cassert>
#include <cstdint>
#include <functional>

namespace raunnes {

static  CPUCore6502::InstructionDetails g_InstructionDetails[256] = {
    #include "InstructionDetails.csv"
};

 CPUCore6502::CPUCore6502(MemoryMap& mem) : 
     // m_State(),
     m_Cycles(0),
     m_Memory(mem),
     m_PreexecutionCallBack(nullptr),
     m_PostexecutionCallBack(nullptr) {
    
     Reset();
}

 CPUCore6502::~CPUCore6502() {
 }

 void CPUCore6502::InstallPreExecutionCallBack(ExecutionCallBack cb) {
     m_PreexecutionCallBack = cb;
 }

 void CPUCore6502::InstallPostExecutionCallBack(ExecutionCallBack cb) {
     m_PostexecutionCallBack = cb;
 }

void CPUCore6502::Reset() {
    m_State.SP = 0xFD;
    m_State.PC = Read16(0xfffc);
    m_State.PC = 0xc000;

    A() = 0;
    X() = 0;
    Y() = 0;

    m_State.SetFlags(0x24);

    m_Cycles = 7;
}

void CPUCore6502::Execute() {
    uint8_t opcode = Read(m_State.PC);
    InstructionDetails details = g_InstructionDetails[opcode];
    DynamicExecutionInfo info(details, opcode, m_State.PC);

    for (uint32_t i = 1; i < details.InstructionSize; i++) {
        info.m_InstructionBytes[i] = Read(m_State.PC + i);
    }

    info.m_Address = Address(info);
    info.m_PageCrossed = PageCrossed(info);

    if (m_PreexecutionCallBack != nullptr) {
        std::invoke(m_PreexecutionCallBack, 
            details, 
            info, 
            m_State,
            m_Memory,
            m_Cycles);
    }

    m_Cycles += details.CycleCount;
    if (info.m_PageCrossed) {
        m_Cycles += details.PageCrossCycleCost;
    }
    m_State.PC += details.InstructionSize;

    std::invoke(details.Delegate, this, info);
}

void CPUCore6502::Push(uint8_t val) {
    Write(0x100 | m_State.SP, val);
    m_State.SP -= 1;
}

void CPUCore6502::Push16(uint16_t val) {
    uint8_t high = (uint8_t)(val >> 8);
    uint8_t low = (uint8_t)(val & 0xff);

    Push(high);
    Push(low);
}

uint8_t CPUCore6502::Pop() {
    m_State.SP += 1;
    uint8_t val = Read(0x100 | m_State.SP);

    return val;
}

uint16_t CPUCore6502::Pop16() {
    uint16_t low = Pop();
    uint16_t high = Pop();

    uint16_t val = high << 8 | low;

    return val;
}

void CPUCore6502::SetC(uint8_t val) {
    m_State.C = val;
}

void CPUCore6502::SetN(uint8_t val) {
    m_State.N = val >> 7;
}

void CPUCore6502::SetZ(uint8_t val) {
    m_State.Z = val == 0;
}

void CPUCore6502::SetZ(bool val) {
    m_State.Z = val;
}

void CPUCore6502::SetV(bool val) {
    m_State.V = val;
}

void CPUCore6502::SetZN(uint8_t val) {
    SetZ(val);
    SetN(val);
}

uint8_t& CPUCore6502::A() {
    return m_State.A;
}

uint8_t& CPUCore6502::P() {
    return m_State.P;
}

uint8_t& CPUCore6502::X() {
    return m_State.X;
}

uint8_t& CPUCore6502::Y() {
    return m_State.Y;
}

uint16_t& CPUCore6502::PC() {
    return m_State.PC;
}

uint8_t& CPUCore6502::SP() {
    return m_State.SP;
}

uint16_t CPUCore6502::Address(const DynamicExecutionInfo& info) {
    uint16_t val = 0;

    if (info.Details().AddresingMode == AddressingModeAbsolute) {
        val = info.AddressAbsolute();
    }
    else if (info.Details().AddresingMode == AddressingModeAbsolute) {
        val = info.AddressAbsolute();
    }
    else if (info.Details().AddresingMode == AddressingModeAbsoluteX) {
        uint16_t addr1 = info.AddressAbsolute();
        uint16_t addr2 = addr1 + X();

        val = addr2;
    }
    else if (info.Details().AddresingMode == AddressingModeAbsoluteY) {
        uint16_t addr1 = info.AddressAbsolute();
        uint16_t addr2 = addr1 + Y();

        val = addr2;
    }
    else if (info.Details().AddresingMode == AddressingModeIndexedIndirect) {
        uint16_t addr1 = ((uint16_t)X() + (uint16_t)info.Immediate()) & 0xFF;

        uint16_t addr2_low = Read(addr1);
        uint16_t addr2_high = Read((addr1 + 1) & 0xFF);
        uint16_t addr2 = (addr2_high << 8) | addr2_low;

        val = addr2;
    }
    else if (info.Details().AddresingMode == AddressingModeIndirect) {
        uint16_t addr1 = info.AddressIndirect();
        uint16_t addr2 = Read16Bug(addr1);

        val = addr2;
    }
    else if (info.Details().AddresingMode == AddressingModeRelative) {
        int8_t offset = info.Immediate();

        val = (int16_t)PC() + offset;
    }
    else if (info.Details().AddresingMode == AddressingModeIndirectIndexed) {
        uint16_t addr1 = info.Immediate();
        
        uint16_t addr2_low = Read(addr1);
        uint16_t addr2_high = Read((addr1 + 1) & 0xFF);
        uint16_t addr2 = ((addr2_high << 8) | addr2_low) + Y();

        val = addr2;
    }
    else if (info.Details().AddresingMode == AddressingModeZeroPage) {
        val = info.AddressZeropage();
    }
    else if (info.Details().AddresingMode == AddressingModeZeroPageX) {
        uint8_t addr = info.Immediate();
        uint8_t addr2 = addr + X();
        
        val = addr2;
    }
    else if (info.Details().AddresingMode == AddressingModeZeroPageY) {
        uint8_t addr = info.Immediate();
        uint8_t addr2 = addr + Y();

        val = addr2;
    }

    return val;
}

bool CPUCore6502::PageCrossed(const DynamicExecutionInfo& info) {
    bool val = false;

    if (info.Details().AddresingMode == AddressingModeIndirectIndexed) {
        uint16_t addr = Address(info);
        val = !SamePage(addr, addr - Y());
    } 
    else if (info.Details().AddresingMode == AddressingModeAbsoluteY) {
        uint16_t addr = Address(info);
        val = !SamePage(addr, addr - Y());
    }
    else if (info.Details().AddresingMode == AddressingModeAbsoluteX) {
        uint16_t addr = Address(info);
        val = !SamePage(addr, addr - X());
    }

    return val;
}

bool CPUCore6502::SamePage(uint16_t a, uint16_t b) {
    uint16_t currentPage = a / 256;
    uint16_t nextPage = b / 256;

    return currentPage == nextPage;
}

uint8_t CPUCore6502::Read(uint16_t address) {
    return m_Memory.Read(address);
}
uint8_t CPUCore6502::Read16(uint16_t address) {
    uint16_t low = m_Memory.Read(address);
    uint16_t high = m_Memory.Read(address+1);

    return (high << 8) | low;
}
uint16_t CPUCore6502::Read16Bug(uint16_t address) {
    // https://everything2.com/title/6502+indirect+JMP+bug
    uint16_t addr_low = address;
    uint16_t addr_high = (addr_low & 0xFF00) | ((addr_low + 1) & 0x00FF);

    uint16_t low = m_Memory.Read(addr_low);
    uint16_t high = m_Memory.Read(addr_high);

    return (high << 8) | low;
}

void CPUCore6502::Write(uint16_t address, uint8_t value) {
    m_Memory.Write(address, value);
}
void CPUCore6502::Write16(uint16_t address, uint16_t value) {
    m_Memory.Write(address, (uint8_t)value);
    m_Memory.Write(address, (uint8_t)(value >> 8));
}

uint8_t CPUCore6502::Value(const DynamicExecutionInfo& info) {
    uint8_t val = 0;

    if (info.Details().AddresingMode == AddressingModeAbsolute) {
        val = Read(Address(info));
    }
    else if (info.Details().AddresingMode == AddressingModeAbsoluteX) {
        val = Read(Address(info));
    }
    else if (info.Details().AddresingMode == AddressingModeAbsoluteY) {
        val = Read(Address(info));
    }
    else if (info.Details().AddresingMode == AddressingModeImmediate) {
        val = info.Immediate();
    }
    else if (info.Details().AddresingMode == AddressingModeAccumulator) {
        val = A();
    }
    else if (info.Details().AddresingMode == AddressingModeIndexedIndirect) {
        val = Read(Address(info));
    }
    else if (info.Details().AddresingMode == AddressingModeIndirectIndexed) {
        val = Read(Address(info));
    }
    else if (info.Details().AddresingMode == AddressingModeZeroPage) {
        val = Read(Address(info));
    }
    else if (info.Details().AddresingMode == AddressingModeZeroPageX) {
        val = Read(Address(info));
    }
    else if (info.Details().AddresingMode == AddressingModeZeroPageY) {
        val = Read(Address(info));
    }
    else {
        val = Read(Address(info));
        assert(0);
    }

    return val;
}

void CPUCore6502::ValueUpdate(const DynamicExecutionInfo& info, uint8_t value) {
    if (info.Details().AddresingMode == AddressingModeImmediate) {
        assert(0);
    }
    else if (info.Details().AddresingMode == AddressingModeAccumulator) {
        A() = value;
    }
    else {
        Write(info.Address(), value);
    }
}

void CPUCore6502::AddBranchCycles(uint16_t oldPC, uint16_t newPC, uint32_t pageCrossCost) {
    uint16_t currentPage = oldPC / 256;
    uint16_t nextPage = newPC / 256;

    m_Cycles += 1;

    if (currentPage != nextPage) {
        m_Cycles += pageCrossCost;
    }
}

void CPUCore6502::IRQ() {
    Push16(PC());
    Push(m_State.P | 0x10);
    PC() = Read16(0xFFFE);
    m_State.I = 1;
    m_Cycles += 7;
}

void CPUCore6502::NMI() {
    Push16(PC());
    Push(m_State.P | 0x10);
    PC() = Read16(0xFFFA);
    m_State.I = 1;
    m_Cycles += 7;
}

void CPUCore6502::Unimplemented(const DynamicExecutionInfo& info) {
    assert(0);
}

void CPUCore6502::ADC(const DynamicExecutionInfo& info) {
    uint8_t a = A();
    uint8_t b = Value(info);
    uint8_t c = m_State.C;

    A() = a + b + c;

    SetC(A() < a);
    SetN(A());
    SetZ(A());

    // http://www.righto.com/2013/01/a-small-part-of-6502-chip-explained.html
    // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
    // V = not (((A7 NOR B7) and C6) NOR ((A7 NAND B7) NOR C6))
    bool v = (b ^ A()) & (a ^ A()) & 0x80;
    SetV(v);
}

void CPUCore6502::AND(const DynamicExecutionInfo& info) {
    A() &= Value(info);

    SetZN(A());
}


void CPUCore6502::ASL(const DynamicExecutionInfo& info) {
    int8_t val = Value(info);
    int8_t newval = val << 1;
    ValueUpdate(info, newval);

    SetZN(newval);
    SetC(((uint8_t)val) >> 7);
}

void CPUCore6502::BCC(const DynamicExecutionInfo& info) {
    if (m_State.C == 0) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.Details().PageCrossCycleCost);
    
        m_State.PC = newPC;
    }
}

void CPUCore6502::BCS(const DynamicExecutionInfo& info) {
    if (m_State.C) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.Details().PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BEQ(const DynamicExecutionInfo& info) {
    if (m_State.Z == 1) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.Details().PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BIT(const DynamicExecutionInfo& info) {
    uint8_t m = Read(info.Address());

    m_State.Z = (A() & m) == 0;
    m_State.V = ((m & 0x40) >> 6) == 1;
    m_State.N = (m >> 7) == 1;
}

void CPUCore6502::BMI(const DynamicExecutionInfo& info) {
    if (m_State.N == 1) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.Details().PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BNE(const DynamicExecutionInfo& info) {
    if (m_State.Z == 0) {
        uint16_t newPC = Address(info);

        AddBranchCycles(m_State.PC, newPC, info.Details().PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BRK(const DynamicExecutionInfo& info) {
    Push16(PC());
    Push(m_State.P | 0x10);
    PC() = Read16(0xFFFE);
    m_State.I = 1;
}

void CPUCore6502::BPL(const DynamicExecutionInfo& info) {
    if (m_State.N == 0) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.Details().PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BVC(const DynamicExecutionInfo& info) {
    if (m_State.V == 0) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.Details().PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::BVS(const DynamicExecutionInfo& info) {
    if (m_State.V == 1) {
        uint16_t newPC = m_State.PC + info.Immediate();

        AddBranchCycles(m_State.PC, newPC, info.Details().PageCrossCycleCost);

        m_State.PC = newPC;
    }
}

void CPUCore6502::CLC(const DynamicExecutionInfo& info) {
    m_State.C = 0;
}

void CPUCore6502::CLD(const DynamicExecutionInfo& info) {
    m_State.D = 0;
}

void CPUCore6502::CLV(const DynamicExecutionInfo& info) {
    m_State.V = 0;
}

void CPUCore6502::CPX(const DynamicExecutionInfo& info) {
    uint8_t value = Value(info);

    SetZ(X() == value);
    SetC(X() >= value);
    SetN(X() - value);
}

void CPUCore6502::CPY(const DynamicExecutionInfo& info) {
    uint8_t value = Value(info);

    SetZ(Y() == value);
    SetC(Y() >= value);
    SetN(Y() - value);
}

void CPUCore6502::CMP(const DynamicExecutionInfo& info) {
    uint8_t value = Value(info);

    SetZ(A() == value);
    SetC(A() >= value);
    SetN(A() - value);
}

void CPUCore6502::DEC(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info) - 1;

    ValueUpdate(info, val);

    SetZN(val);
}

void CPUCore6502::DEX(const DynamicExecutionInfo& info) {
    X() -= 1;

    SetZN(X());
}

void CPUCore6502::DEY(const DynamicExecutionInfo& info) {
    Y() -= 1;

    SetZN(Y());
}

void CPUCore6502::DCP(const DynamicExecutionInfo& info) {
    int8_t value = Value(info) - 1;
    ValueUpdate(info, value);

    uint8_t uvalue = value;
    
    SetZ(A() == uvalue);
    SetC(A() >= uvalue);
    SetN(A() - uvalue);
}

void CPUCore6502::EOR(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info);

    A() ^= val;

    SetZN(A());
}

void CPUCore6502::INC(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info) + 1;

    ValueUpdate(info, val);

    SetZN(val);
}

void CPUCore6502::INX(const DynamicExecutionInfo& info) {
    X() += 1;

    SetZN(X());
}

void CPUCore6502::INY(const DynamicExecutionInfo& info) {
    Y() += 1;

    SetZN(Y());
}

void CPUCore6502::ISB(const DynamicExecutionInfo& info) {
    uint8_t value = Value(info) + 1;
    ValueUpdate(info, value);

    uint8_t a = A();
    uint8_t b = ~value;
    uint8_t c = m_State.C;

    A() = a + b + c;

    SetC(A() <= a);
    SetN(A());
    SetZ(A());

    // http://www.righto.com/2013/01/a-small-part-of-6502-chip-explained.html
    // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
    // V = not (((A7 NOR B7) and C6) NOR ((A7 NAND B7) NOR C6))
    bool v = (b ^ A()) & (a ^ A()) & 0x80;
    SetV(v);
}

void CPUCore6502::JMP(const DynamicExecutionInfo& info) {
    m_State.PC = info.Address();
}

void CPUCore6502::JSR(const DynamicExecutionInfo& info) {
    uint16_t address = m_State.PC - 1;
    Push16(address);
    m_State.PC = info.Address();
}

void CPUCore6502::LAX(const DynamicExecutionInfo& info) {
    A() = Value(info);
    X() = A();

    SetZN(A());
}

void CPUCore6502::LDA(const DynamicExecutionInfo& info) {
    A() = Value(info);

    SetZN(A());
}

void CPUCore6502::LDX(const DynamicExecutionInfo& info) {
    X() = Value(info);

    SetZN(X());
}

void CPUCore6502::LDY(const DynamicExecutionInfo& info) {
    Y() = Value(info);

    SetZN(Y());
}

void CPUCore6502::LSR(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info);
    uint8_t newval = val >> 1;
    ValueUpdate(info, newval);

    SetZN(newval);
    SetC(val & 0x1);
}

void CPUCore6502::NOP(const DynamicExecutionInfo& info) {
    return;
}

void CPUCore6502::ORA(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info);

    A() |= val;

    SetZN(A());
}

void CPUCore6502::PHA(const DynamicExecutionInfo& info) {
    Push(A());
}

void CPUCore6502::PHP(const DynamicExecutionInfo& info) {
    // B Flag is always pushed as set
    Push(m_State.P | 0x10);
}

void CPUCore6502::PLA(const DynamicExecutionInfo& info) {
    A() = Pop();

    SetZN(A());
}

void CPUCore6502::PLP(const DynamicExecutionInfo& info) {
    // Ignores bits 4, 5
    // http://wiki.nesdev.com/w/index.php/Status_flags#I:_Interrupt_Disable
    // http://visual6502.org/wiki/index.php?title=6502_BRK_and_B_bit
    m_State.P = Pop() & 0xEF | 0x20;
}

void CPUCore6502::RLA(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info);
    uint8_t newval = (val << 1) | m_State.C;

    ValueUpdate(info, newval);

    A() &= newval;

    SetZN(A());
    SetC(val >> 7);
}

void CPUCore6502::ROL(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info);
    uint8_t newval = (val << 1) | m_State.C;

    ValueUpdate(info, newval);

    SetZN(newval);
    SetC(val >> 7);
}

void CPUCore6502::ROR(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info);
    uint8_t newval = (val >> 1) | (m_State. C << 7);

    ValueUpdate(info, newval);

    SetZN(newval);
    SetC(val & 1);
}

void CPUCore6502::RRA(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info);
    uint8_t newval = (val >> 1) | (m_State.C << 7);

    ValueUpdate(info, newval);

    uint8_t a = A();
    uint8_t b = newval;
    uint8_t c = val & 1;

    A() = a + b + c;

    SetC(A() < a);
    SetN(A());
    SetZ(A());

    // http://www.righto.com/2013/01/a-small-part-of-6502-chip-explained.html
    // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
    // V = not (((A7 NOR B7) and C6) NOR ((A7 NAND B7) NOR C6))
    bool v = (b ^ A()) & (a ^ A()) & 0x80;
    SetV(v);
}

void CPUCore6502::RTS(const DynamicExecutionInfo& info) {
    uint16_t newPC = Pop16() + 1;

    m_State.PC = newPC;
}

void CPUCore6502::RTI(const DynamicExecutionInfo& info) {
    P() = Pop() & 0xEF | 0x20;
    PC() = Pop16();
}

void CPUCore6502::SAX(const DynamicExecutionInfo& info) {
    uint8_t val = A() & X();

    ValueUpdate(info, val);

}

void CPUCore6502::SBC(const DynamicExecutionInfo& info) {
    uint8_t a = A();
    uint8_t b = ~Value(info);
    uint8_t c = m_State.C;

    A() = a + b + c;

    SetC(A() < a);
    SetN(A());
    SetZ(A());

    // http://www.righto.com/2013/01/a-small-part-of-6502-chip-explained.html
    // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
    // V = not (((A7 NOR B7) and C6) NOR ((A7 NAND B7) NOR C6))
    bool v = (b ^ A()) & (a ^ A()) & 0x80;
    SetV(v);
}

void CPUCore6502::SEC(const DynamicExecutionInfo& info) {
    m_State.C = 1;
}

void CPUCore6502::SEI(const DynamicExecutionInfo& info) {
    m_State.I = 1;
}

void CPUCore6502::SED(const DynamicExecutionInfo& info) {
    m_State.D = 1;
}

void CPUCore6502::SLO(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info);
    uint8_t newval = val << 1;
    ValueUpdate(info, newval);

    A() |= newval;

    SetZN(A());
    SetC(val >> 7);
}

void CPUCore6502::SRE(const DynamicExecutionInfo& info) {
    uint8_t val = Value(info);
    uint8_t newval = val >> 1;
    ValueUpdate(info, newval);

    A() ^= newval;

    SetZN(A());
    SetC(val & 0x1);
}

void CPUCore6502::STA(const DynamicExecutionInfo& info) {
    Write(Address(info), A());
}

void CPUCore6502::STX(const DynamicExecutionInfo& info) {
    Write(Address(info), X());
}

void CPUCore6502::STY(const DynamicExecutionInfo& info) {
    Write(Address(info), Y());
}

void CPUCore6502::TAX(const DynamicExecutionInfo& info) {
    X() = A();

    SetZN(X());
}

void CPUCore6502::TAY(const DynamicExecutionInfo& info) {
    Y() = A();

    SetZN(Y());
}

void CPUCore6502::TSX(const DynamicExecutionInfo& info) {
    X() = SP();

    SetZN(X());
}

void CPUCore6502::TXA(const DynamicExecutionInfo& info) {
    A() = X();

    SetZN(A());
}

void CPUCore6502::TXS(const DynamicExecutionInfo& info) {
    SP() = X();
}

void CPUCore6502::TYA(const DynamicExecutionInfo& info) {
    A() = Y();

    SetZN(A());
}

}