#include "MemoryMap.h"

#include <algorithm>
#include <cassert>
#include <cstring>

namespace raunnes {
MemoryMap::MemoryMap(uint8_t* prg, uint16_t prgSize, uint8_t* chr, uint16_t chrSize) {
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

    // Copy CHR Rom to 0x0 - 0x1fff
    m_PPUBytes.resize(0x4000);
    memcpy(&m_PPUBytes[0], chr, chrSize);
}

MemoryMap::~MemoryMap() {
}
    
uint8_t MemoryMap::Read(uint16_t address) const {
    if (address < m_Bytes.size()) {

        if((address >= 0x2000) && (address <= 0x2007)) {
            // PPU registers
            return 0;
        } else {
            return m_Bytes[address];
        }
    }
    return 0;
}
    
void MemoryMap::Write(uint16_t address, uint8_t value) {
    if (address < m_Bytes.size()) {
        m_Bytes[address] = value;
    }
}

uint8_t MemoryMap::ReadPPU(uint16_t address) const {
    // Address range 	Size 	Description
    // $0000-$0FFF      $1000 	Pattern table 0
    // $1000-$1FFF      $1000 	Pattern table 1
    // $2000-$23FF      $0400 	Nametable 0
    // $2400-$27FF      $0400 	Nametable 1
    // $2800-$2BFF      $0400 	Nametable 2
    // $2C00-$2FFF      $0400 	Nametable 3
    // $3000-$3EFF      $0F00 	Mirrors of $2000-$2EFF
    // $3F00-$3F1F      $0020 	Palette RAM indexes
    // $3F20-$3FFF      $00E0 	Mirrors of $3F00-$3F1F

    int size = m_PPUBytes.size();
    if (address < m_PPUBytes.size()) {

        if((address >= 0x3000) && (address <= 0x3eff)) {
            return m_PPUBytes[address - 0x1000];

        } else if((address >= 0x3f20) && (address <= 0x3fff)) {
            uint16_t newAddr = 0x3f00 + (address % 0x1f);
            return m_PPUBytes[newAddr];
        
        } else {
            return m_PPUBytes[address];
        }
        
    }
    return 0;
}

}