#pragma once

#include <functional>
#include <vector>
#include <cstdint>

namespace raunnes {

typedef std::function<void(uint32_t, uint32_t)> MMIOCallback;

class MemoryMapEntry {

    friend class MemoryMap;
public:
    MemoryMapEntry(MemoryMap const& map, uint32_t start, uint32_t end);
    ~MemoryMapEntry() = default;

    virtual uint8_t operator[](uint32_t address) = 0;
    bool contains(uint32_t address) const;

protected:
    MemoryMap const&  m_Map;
    uint32_t          m_Start;
    uint32_t          m_End;
};




class MemoryMapEntryAlwaysFault : public MemoryMapEntry {
public:
    MemoryMapEntryAlwaysFault(MemoryMap const& map);

    uint8_t operator[](uint32_t address) override;
};




class MemoryMapEntryRAM : public MemoryMapEntry {
public:
    MemoryMapEntryRAM(MemoryMap const& map, uint32_t start, uint32_t end);
    uint8_t operator[](uint32_t address) override;

private:
    std::vector<uint8_t> m_Bytes;
};



class MemoryMapEntryROM : public MemoryMapEntry {
public:
    MemoryMapEntryROM(MemoryMap const& map, uint32_t start, uint32_t end);
    uint8_t operator[](uint32_t address) override;

private:
    std::vector<uint8_t> m_Bytes;
};




class MemoryMap {
public:
    MemoryMap();
    ~MemoryMap();

    void map(uint32_t start, uint32_t end, MMIOCallback);
    void ram(uint32_t start, uint32_t end);
    void rom(uint32_t start, uint32_t end);
    MemoryMapEntry& operator[](uint32_t address);

private:
    std::vector<MemoryMapEntry*> m_Entries;
    MemoryMapEntryAlwaysFault m_AlwaysFaultEntry;
};

}