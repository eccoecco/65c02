#include "mcu_core.hpp"
#include "mcu_instr.hpp"

#include "mcu_6502.hpp"
#include "mcu_65c02.hpp"

// These macros are used to help construct a 256 element switch statement going from 0 to 255.
// Typed out, it would look something like:
// switch( opCode )
// {
// case 0: mcuInstructionExecute< 0 >( *this ); break;
// case 1: mcuInstructionExecute< 1 >( *this ); break;
// case 2: mcuInstructionExecute< 2 >( *this ); break;
// ...
// case 255: mcuInstructionExecute< 255 >( *this ); break;
// }
// However, I'm much too lazy to generate all that, so I use these macros to get the preprocessor
// to generate the statements for me.

#define EXEC_OPCODE( opCode )  case (opCode): mcuInstructionExecute< (opCode) >( *this ); break;
#define EXEC_OPCODE_8( opCode )  \
    EXEC_OPCODE( opCode ); EXEC_OPCODE( opCode + 1 ); EXEC_OPCODE( opCode + 2 ); EXEC_OPCODE( opCode + 3 ); \
    EXEC_OPCODE( opCode + 4 ); EXEC_OPCODE( opCode + 5 ); EXEC_OPCODE( opCode + 6 ); EXEC_OPCODE( opCode + 7 );
#define EXEC_OPCODE_64( opCode )  \
    EXEC_OPCODE_8( opCode ); EXEC_OPCODE_8( opCode + 1 * 8 ); EXEC_OPCODE_8( opCode + 2 * 8 ); EXEC_OPCODE_8( opCode + 3 * 8 ); \
    EXEC_OPCODE_8( opCode + 4 * 8 ); EXEC_OPCODE_8( opCode + 5 * 8 ); EXEC_OPCODE_8( opCode + 6 * 8 ); EXEC_OPCODE_8( opCode + 7 * 8 );

void tMCUState::pcExecute()
{
    uint8_t opCode = pcReadByte();

    switch( opCode )
    {
    EXEC_OPCODE_64( 0 );
    EXEC_OPCODE_64( 1 * 64 );
    EXEC_OPCODE_64( 2 * 64 );
    EXEC_OPCODE_64( 3 * 64 );
    }
}