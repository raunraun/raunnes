#include "MemoryMap.h"

#include <algorithm>
#include <cassert>

namespace raunnes {
MemoryMap::MemoryMap(uint8_t* prg, uint16_t prgSize) {
    m_Bytes.resize(0x10000);

    assert(prgSize >= 0x4000);
    assert(prgSize % 0x4000 == 0);

    for(uint32_t i = 0x6000; i < 0x7fff; i += prgSize) {
        memcpy(&m_Bytes[i], prg, prgSize);	
    }

    for (uint32_t i = 0x8000; i < 0xbfff; i += prgSize) {
        memcpy(&m_Bytes[i], prg, prgSize);
    }

    for (uint32_t i = 0xc000; i < 0xffff; i += prgSize) {
        memcpy(&m_Bytes[i], prg, prgSize);
    }
}

MemoryMap::~MemoryMap() {
}

uint16_t MemoryMap::Read16(uint16_t address) const {
    if (((size_t)address)+1 < m_Bytes.size()) {
        uint16_t low = m_Bytes[address];
        uint16_t high = m_Bytes[((size_t)address) + 1];
        uint16_t val = (high << 8) | low	;
        return val;
    }
    return 0;
}
    
uint8_t MemoryMap::Read(uint16_t address) const {
    if (address < m_Bytes.size()) {
        return m_Bytes[address];
    }
    return 0;
}
    
void MemoryMap::Write(uint16_t address, uint8_t value) {
    if (address < m_Bytes.size()) {
        m_Bytes[address] = value;
    }
}

}