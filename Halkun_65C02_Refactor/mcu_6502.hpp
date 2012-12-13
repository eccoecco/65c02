/*

  mcu_6502.hpp - 6502 specific instructions

*/

#ifndef MCU_6502_HPP
#define MCU_6502_HPP

#include "mcu_instr.hpp"

DECLARE_INSTRUCTION( 0x00, BRK );
DECLARE_INSTRUCTION( 0x01, ORA );
DECLARE_INSTRUCTION( 0, BRK );
DECLARE_INSTRUCTION( 0, BRK );
DECLARE_INSTRUCTION( 0, BRK );
DECLARE_INSTRUCTION( 0, BRK );
DECLARE_INSTRUCTION( 0, BRK );
DECLARE_INSTRUCTION( 0, BRK );
DECLARE_INSTRUCTION( 0, BRK );
DECLARE_INSTRUCTION( 0, BRK );


// Generic template for all instructions acts as a NOP in release mode, but asserts in debug
template <uint8_t opCodeNumber> inline void mcuInstructionExecute( tMCUState& rState ) { assert( false ); }
// Generic template for uninstantiated instructions returns ?? as its name
template <uint8_t opCodeNumber> inline std::string mcuInstructionName() { return "??"; }

// Use this to declare the instruction to exist, e.g.
// DECLARE_INSTRUCTION( 0, BRK );
//   This will declare:
// 1.) mcuInstruction_BRK() to exist (so that other functions can call the instructions by its name), and
// 2.) mcuInstructionExecute<0> to exist, and for it to inline mcuInstruction_BRK(), and
// 3.) mcuInstructionName<0> to return "BRK".
#define DECLARE_INSTRUCTION( opCode, instrName ) \
    inline void mcuInstruction_ ## instrName( tMCUState&, uint8_t ); \
    template<> inline void mcuInstructionExecute< opCode >( tMCUState& rState ) { mcuInstruction_ ## instrName( rState, opCode ); } \
    template<> inline std::string mcuInstructionName< opCode >() { return #instrName; }

// Use this to actually instantiate the instruction
// DEFINE_INSTRUCTION( BRK )
// {
//     // Manipulate rState.xx
// }
//   This will actually instantiate mcuInstruction_BRK(), so that you do not need to type
// the opcode again.  Any typos will cause a link error.
#define DEFINE_INSTRUCTION( instrName ) \
    inline void mcuInstruction_ ## instrName( tMCUState& rState )

#endif
