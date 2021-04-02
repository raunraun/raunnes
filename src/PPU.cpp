#include "PPU.h"

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

void PPU::WriteRegister(uint16_t address, uint8_t value) {
    switch (address) {
    case 0x2006:
        if (m_AddrHighEnable) {
            m_Addr = 0;
            m_Addr = value;
            m_Addr <<= 8;
        }
        else {
            m_Addr |= value;
        }
        m_AddrHighEnable = !m_AddrHighEnable;
        break;
    }
}

}