#include <fstream>
#include <iostream>

#include "6502Core.h"
#include "MemoryMap.h"

int main(int argc, char** argv) {
    
    std::fstream romFile("../tests/nestest/nestest.nes", std::ios::in | std::ios::binary);

    if (romFile.good()) {
        std::cout << "WOO\n";
    }
    


    return 0;
}
