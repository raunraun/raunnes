#pragma once

#include <functional>
#include <vector>
#include <cstdint>

namespace raunnes {

class MemoryMap {
public:
    MemoryMap(uint8_t* prg, uint16_t prgSize, uint8_t* chr, uint16_t chrSize);
    ~MemoryMap();

    uint8_t Read(uint16_t address) const;
    void Write(uint16_t address, uint8_t value);

    uint8_t ReadChr(uint16_t address) const;
    
public:
    MemoryMap(const MemoryMap&) = delete;
    MemoryMap& operator=(const MemoryMap&) = delete;

private:
    std::vector<uint8_t> m_Bytes;
    std::vector<uint8_t> m_ChrBytes;
};


}