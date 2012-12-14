#include "mcu_core.hpp"
#include "mcu_instr.hpp"

#include "mcu_6502.hpp"
#include "mcu_65c02.hpp"

#include <sstream>
#include <iomanip>

// These macros are used to help construct a 256 element switch statement going from 0 to 255.
// Typed out, it would look something like:
// switch( opCode )
// {
// case 0: return mcuInstructionExecute< 0 >( *this );
// case 1: return mcuInstructionExecute< 1 >( *this );
// case 2: return mcuInstructionExecute< 2 >( *this );
// ...
// case 255: return mcuInstructionExecute< 255 >( *this );
// }
// However, I'm much too lazy to generate all that, so I use these macros to get the preprocessor
// to generate the statements for me.

#define EXEC_OPCODE( opCode, templateFuncName )  case (opCode): return templateFuncName< (opCode) >( *this );
#define EXEC_OPCODE_8( opCode, templateFuncName )  \
    EXEC_OPCODE( (opCode), templateFuncName );     EXEC_OPCODE( (opCode) + 1, templateFuncName ); \
    EXEC_OPCODE( (opCode) + 2, templateFuncName ); EXEC_OPCODE( (opCode) + 3, templateFuncName ); \
    EXEC_OPCODE( (opCode) + 4, templateFuncName ); EXEC_OPCODE( (opCode) + 5, templateFuncName ); \
    EXEC_OPCODE( (opCode) + 6, templateFuncName ); EXEC_OPCODE( (opCode) + 7, templateFuncName );
#define EXEC_OPCODE_64( opCode, templateFuncName )  \
    EXEC_OPCODE_8( (opCode), templateFuncName );         EXEC_OPCODE_8( (opCode) + 1 * 8, templateFuncName ); \
    EXEC_OPCODE_8( (opCode) + 2 * 8, templateFuncName ); EXEC_OPCODE_8( (opCode) + 3 * 8, templateFuncName ); \
    EXEC_OPCODE_8( (opCode) + 4 * 8, templateFuncName ); EXEC_OPCODE_8( (opCode) + 5 * 8, templateFuncName ); \
    EXEC_OPCODE_8( (opCode) + 6 * 8, templateFuncName ); EXEC_OPCODE_8( (opCode) + 7 * 8, templateFuncName );

void tMCUState::pcExecute()
{
    uint8_t opCode = pcReadByte();

    switch( opCode )
    {
    EXEC_OPCODE_64( 0, mcuInstructionExecute );
    EXEC_OPCODE_64( 1 * 64, mcuInstructionExecute );
    EXEC_OPCODE_64( 2 * 64, mcuInstructionExecute );
    EXEC_OPCODE_64( 3 * 64, mcuInstructionExecute );
    }
}

// Decodes the current instruction into a human readable string
std::string tMCUState::decodeFullOpcode( uint16_t memPos )
{
    std::string addressingMode = decodeAddressing( memPos );

    std::string opCodeName = decodeOpcode( memPos );

    if( !addressingMode.empty() )
    {
        opCodeName += " ";
        opCodeName += addressingMode;
    }

    return opCodeName;
}

// Adds one byte for the opcode
uint8_t tMCUState::decodeFullOpcodeLength( uint16_t memPos )
{
    return 1 + decodeAddressingLength( memPos );
}

// Returns number of bytes used in the addressing of the opcode@memPos
uint8_t tMCUState::decodeAddressingLength( uint16_t memPos )
{
    uint8_t opCode = memReadByte( memPos );

    switch( opCode )
    {
    EXEC_OPCODE_64( 0, mcuInstructionDecodeLength );
    EXEC_OPCODE_64( 1 * 64, mcuInstructionDecodeLength );
    EXEC_OPCODE_64( 2 * 64, mcuInstructionDecodeLength );
    EXEC_OPCODE_64( 3 * 64, mcuInstructionDecodeLength );
    }

    return 0;
}

// Grabs the opcode's name
std::string tMCUState::decodeOpcode( uint16_t memPos )
{
    uint8_t opCode = memReadByte( memPos );

    switch( opCode )
    {
    EXEC_OPCODE_64( 0, mcuInstructionName );
    EXEC_OPCODE_64( 1 * 64, mcuInstructionName );
    EXEC_OPCODE_64( 2 * 64, mcuInstructionName );
    EXEC_OPCODE_64( 3 * 64, mcuInstructionName );
    }

    return "??";
}

std::string tMCUState::decodeAddressing( uint16_t memPos )
{
    uint8_t opCode = memReadByte( memPos );
    m_decodePos = memPos + 1;

    switch( opCode )
    {
    EXEC_OPCODE_64( 0, mcuInstructionDecodeAddressing );
    EXEC_OPCODE_64( 1 * 64, mcuInstructionDecodeAddressing );
    EXEC_OPCODE_64( 2 * 64, mcuInstructionDecodeAddressing );
    EXEC_OPCODE_64( 3 * 64, mcuInstructionDecodeAddressing );
    }

    return "";
}

std::string tMCUState::decodeAddressing( eAddressingMode_Mem mode )
{
    std::stringstream addressedElement;

    uint16_t memLoc = m_decodePos;

    addressedElement << std::hex << std::setfill('0');

    switch( mode )
    {
    case am_Immediate:      addressedElement << "#$" << std::setw(2) << unsigned(memReadByte( memLoc )); break;
    case am_ZeroPage:       addressedElement << "$" << std::setw(2) << unsigned(memReadByte( memLoc )); break;
    case am_ZeroPage_X:     addressedElement << "$" << std::setw(2) << unsigned(memReadByte( memLoc )) << ", X"; break;
    case am_ZeroPage_Y:     addressedElement << "$" << std::setw(2) << unsigned(memReadByte( memLoc )) << ", Y"; break;
    case am_Relative:       addressedElement << "*" << std::dec << int(int8_t(memReadByte( memLoc ))); break;
    case am_Absolute:       addressedElement << "$" << std::setw(4) << unsigned(memReadWord( memLoc )); break;
    case am_Absolute_X:     addressedElement << "$" << std::setw(4) << unsigned(memReadWord( memLoc )) << ", X"; break;
    case am_Absolute_Y:     addressedElement << "$" << std::setw(4) << unsigned(memReadWord( memLoc )) << ", Y"; break;
    case am_Indirect:       addressedElement << "($" << std::setw(4) << unsigned(memReadWord( memLoc )) << ")"; break;
    case am_Indirect_X:     addressedElement << "($" << std::setw(2) << unsigned(memReadByte( memLoc )) << ", X)"; break;
    case am_Indirect_Y:     addressedElement << "($" << std::setw(2) << unsigned(memReadByte( memLoc )) << "), Y"; break;
    case am_Indirect_ZP:    addressedElement << "($" << std::setw(2) << unsigned(memReadByte( memLoc )) << ")"; break;
    case am_AbsIdxIndirect: addressedElement << "($" << std::setw(4) << unsigned(memReadWord( memLoc )) << ", X)"; break;
        break;
    }

    return addressedElement.str();
}

std::string tMCUState::decodeAddressing( eAddressingMode_Register mode )
{
    std::string addressedElement;
    switch( mode )
    {
    case am_Accum: addressedElement = "A"; break;
    }

    return addressedElement;
}

std::string tMCUState::decodeAddressing( eAddressingMode_Null mode )
{
    return "";
}