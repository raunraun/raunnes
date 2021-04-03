#include "PPU.h"

#include <cassert>

namespace raunnes {

PPU::PPU(MemoryMap& memory) :
    m_Map(memory),
    m_Control(0x0),
    m_Mask(0x0),
    m_Status(0x0),
    m_OAMAddr(0x0),
    m_OAMData(0x0),
    m_Scroll(0x0),
    m_Addr(0x0),
    m_AddrHighEnable(true),
    m_Data(0x0),
    m_OMADMA(0x0) {
}

PPU::~PPU() {
}

uint8_t PPU::I() const {
    return (m_Control << 5) >> 7;
}

void PPU::WriteRegister(uint16_t address, uint8_t value) {
    switch (address) {
    case 0x2000:
        m_Control = value;
        break;
    case 0x2006:
        if (m_AddrHighEnable) {
            m_Addr = 0;
            m_Addr = value;
            m_Addr <<= 8;
        }
        else {
            m_Addr |= value;
            m_AddrTemp = m_Addr;
        }
        m_AddrHighEnable = !m_AddrHighEnable;
        break;
    }
}

uint8_t PPU::ReadRegister(uint16_t address) {
    switch (address) {
    case 0x2000:
        return m_Control;
        break;
    case 0x2007:
        if (I() == 0) {
            m_Addr += 1;
        }
        else {
            m_Addr += 32;
        }
        return m_Data;
        break;
    default:
        assert(0 && "Unimplemented PPU Register Read");
    }
    return 0;
}

}