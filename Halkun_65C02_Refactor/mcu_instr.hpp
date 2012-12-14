/*

  mcu_instr.hpp - Helper functions for MCU instructions

*/

#ifndef MCU_INSTR_HPP
#define MCU_INSTR_HPP

#include <cstdint>
#include <cassert>
#include <string>

#include "mcu_core.hpp"

// Generic template for all instructions acts as a NOP in release mode, but asserts in debug
template <uint8_t opCodeNumber> inline void mcuInstructionExecute( tMCUState& rState ) { assert( false ); }
// Generic template for uninstantiated instructions returns an empty string as its name
template <uint8_t opCodeNumber> inline std::string mcuInstructionName() { return ""; }

// Use this to declare the instruction to exist by declaring its opcode, mnemonic, and addressing mode
// DECLARE_INSTRUCTION( 0, BRK, am_ZeroPage );
//   This will declare:
// 1.) mcuInstruction_BRK() to exist (so that other functions can call the instruction by its name), and
// 2.) mcuInstructionExecute<0> to exist, and for it to inline mcuInstruction_BRK() with zero page addressing mode, and
// 3.) mcuInstructionName<0> to return "BRK".
#define DECLARE_INSTRUCTION( instrOpCode, instrName, addressingMode ) \
    template<typename tAccessor> inline void mcuInstruction_ ## instrName( tMCUState&, tAccessor, uint8_t ); \
    template<> inline void mcuInstructionExecute< instrOpCode >( tMCUState& rState ) { mcuInstruction_ ## instrName( rState, rState.makeAccessor( addressingMode ), instrOpCode ); } \
    template<> inline std::string mcuInstructionName< instrOpCode >() { return #instrName; }

// Use this to actually instantiate the instruction
// DEFINE_INSTRUCTION( BRK )
// {
//     // Manipulate rState.xx
// }
//   This will actually instantiate mcuInstruction_BRK(), so that you do not need to type
// the opcode again.  Any typos will cause a link error.
//   The arguments that can be used are:
// 1.) rState - the main mcu state
// 2.) memData - an accessor to the data in question, if any
// 3.) opCode - the numeric opcode
#define DEFINE_INSTRUCTION( instrName ) \
    template<typename tAccessor> inline void mcuInstruction_ ## instrName( tMCUState& rState, tAccessor memData, uint8_t opCode )

#endif
