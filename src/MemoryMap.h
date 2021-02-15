#pragma once

#include <functional>
#include <vector>
#include <cstdint>

namespace raunnes {

class MemoryMap {
public:
    MemoryMap(uint8_t* prg, uint16_t prgSize);
    ~MemoryMap();

    uint16_t Read16(uint16_t address) const;
    uint8_t Read(uint16_t address) const;
    void Write(uint16_t address, uint8_t value);


public:
    MemoryMap(const MemoryMap&) = delete;
    MemoryMap& operator=(const MemoryMap&) = delete;

private:
    std::vector<uint8_t> m_Bytes;
};


}