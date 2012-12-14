#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cassert>

#include "mcu_core.hpp"

void printState( tMCUState& rState )
{

}

void testFunc()
{

}

void test2()
{
    return testFunc();
}

int main( int argc, char *argv[] )
{
    uint8_t mcuMemory[65536];

    for( unsigned int i = 0; i < 65536; ++i )
        mcuMemory[i] = i & 0xFF;

    tMCUState mcu( mcuMemory );

    for( unsigned int i = 0; i < 256; ++i )
    {
        mcu.regPC = i;

        std::cout << std::hex << std::setw(2)  << std::setfill('0') << i;

        std::cout << " : " << mcu.pcDecode() << std::endl;

        if( (i & 7) == 7 )
            std::cin.get();
    }

    return 0;
}