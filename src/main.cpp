#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    
    std::fstream romFile("../tests/nestest/nestest.nes", std::ios::in | std::ios::binary);

    if (romFile.good()) {
        std::cout << "WOO\n";
    }
    


    return 0;
}
