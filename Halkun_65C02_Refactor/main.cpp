#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cassert>

#include "mcu_core.hpp"

struct tHexFormat
{
    uint16_t m_Num;
    uint8_t m_Digits;

    tHexFormat( uint16_t number ) : m_Num( number ), m_Digits( 4 ) {}
    tHexFormat( uint8_t number ) : m_Num( number ), m_Digits( 2 ) {}
};

std::ostream& operator<<( std::ostream& os, const tHexFormat& rFormat )
{
    os << std::hex << std::setw( rFormat.m_Digits ) << std::setfill('0') << rFormat.m_Num;
    return os;
}

void printDisassembly( tMCUState& rState, uint16_t memPos, unsigned totalInstructions )
{
    for( unsigned currentInstruction = 0; currentInstruction < totalInstructions; ++currentInstruction )
    {
        std::cout << "  " << tHexFormat( memPos ) << " : ";

        // Print out the binary data relevant to the instruction
        unsigned instructionBytes = rState.decodeFullOpcodeLength( memPos );
        for( unsigned byteIndex = 0; byteIndex < instructionBytes; ++byteIndex )
            std::cout << tHexFormat( rState.memReadByte( memPos + byteIndex ) ) << " ";

        // Pad up to 5 data bytes worth to align the display
        for( unsigned padIndex = instructionBytes ; padIndex < 5; ++padIndex )
            std::cout << "   ";

        std::cout << rState.decodeFullOpcode( memPos ) << std::endl;

        memPos += instructionBytes;
    }
}

void printState( tMCUState& rState )
{
    std::cout
        << "A=" << tHexFormat( rState.regA ) << "  X=" << tHexFormat( rState.regX ) << "  Y=" << tHexFormat( rState.regY ) << "  S=" << tHexFormat( rState.regSP ) << "  PC=" << tHexFormat( rState.regPC ) << std::endl
        << "P=" << tHexFormat( rState.regP ) << "  ";

    const char *pFlagNames[] = { "C", "Z", "I", "D", "B", "X", "V", "N" };

    for( unsigned bitPos = 0; bitPos < 8; ++bitPos )
        std::cout << (rState.isFlagSet( eFlags(1<< bitPos) ) ? " " : "N" ) << pFlagNames[bitPos] << " ";

    std::cout << std::endl;
/*    flag_C = 0x01, // Carry
    flag_Z = 0x02, // Zero
    flag_I = 0x04, // IRQ
    flag_D = 0x08, // Decimal
    flag_B = 0x10, // Break
    flag_X = 0x20, // Undefined (?) - always set
    flag_V = 0x40, // Sign bit overflow
    flag_N = 0x80, // Negative*/

}

int main( int argc, char *argv[] )
{
    uint8_t mcuMemory[65536];

    for( unsigned int i = 0; i < 65536; ++i )
        mcuMemory[i] = i & 0xFF;

    tMCUState mcu( mcuMemory );

    printState( mcu );

    printDisassembly( mcu, 0, 30 );

    std::cin.get();
    return 0;
    for( unsigned int i = 0; i < 256; ++i )
    {
        mcu.regPC = i;

        std::cout << std::hex << std::setw(2)  << std::setfill('0') << i;

        std::cout << " : " << mcu.pcDecode() << std::endl;

        if( (i & 7) == 7 )
            break;
    }

    mcu.regPC = 0;
    for( unsigned i = 0; i < 8; ++i )
    {
        uint8_t opCodeLength = mcu.decodeFullOpcodeLength( mcu.regPC );
        std::cout << " : " << mcu.pcDecode() << " [" << unsigned(opCodeLength) << "]" << std::endl;
        mcu.regPC += opCodeLength;
    }

    std::cin.get();

    return 0;
}