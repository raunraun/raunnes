#include "MemoryMap.h"

#include <algorithm>
#include <cassert>

namespace raunnes {

MemoryMapEntry::MemoryMapEntry(MemoryMap const& map, uint32_t start, uint32_t end) :
    m_Map(map),
    m_Start(start),
    m_End(end) {
}

bool MemoryMapEntry::contains(uint32_t address) const {
    return address >= m_Start && address <= m_End;
}




MemoryMapEntryAlwaysFault::MemoryMapEntryAlwaysFault(MemoryMap const& map) :
   MemoryMapEntry(map, 0, 0) {
}

uint8_t MemoryMapEntryAlwaysFault::operator[](uint32_t address) {
    assert(0);
    return 0;
}



MemoryMapEntryRAM::MemoryMapEntryRAM(MemoryMap const& map, uint32_t start, uint32_t end) :
    MemoryMapEntry(map, start, end) {

    uint32_t size = end - start;
    m_Bytes.resize(size);
}

uint8_t MemoryMapEntryRAM::operator[](uint32_t address) {
    uint8_t a = m_Start - address;
    return m_Bytes[a];
}




MemoryMapEntryROM::MemoryMapEntryROM(MemoryMap const& map, uint32_t start, uint32_t end) :
    MemoryMapEntry(map, start, end) {

    uint32_t size = end - start;
    m_Bytes.resize(size);
}

uint8_t MemoryMapEntryROM::operator[](uint32_t address) {
    uint8_t a = m_Start - address;
    return m_Bytes[a];
}




MemoryMap::MemoryMap() :
    m_AlwaysFaultEntry(*this) {

}

MemoryMap::~MemoryMap() {

}

void MemoryMap::map(uint32_t start, uint32_t end, MMIOCallback cb) {
   // m_Entries.push_back(new MemoryMapEntry(*this, start, end));
}

void MemoryMap::ram(uint32_t start, uint32_t end) {
    m_Entries.push_back(new MemoryMapEntryRAM(*this, start, end));
}

void MemoryMap::rom(uint32_t start, uint32_t end) {
    m_Entries.push_back(new MemoryMapEntryROM(*this, start, end));
}

MemoryMapEntry& MemoryMap::operator[](uint32_t address) {
    auto map = std::find_if(m_Entries.begin(),
        m_Entries.end(),
        [&address](const MemoryMapEntry* mme) { return true; });
    
    return (map != m_Entries.end()) ? **map : m_AlwaysFaultEntry;
}

}