/*

  mcu_6502.hpp - 65C02 specific instructions

*/

#ifndef MCU_65C02_HPP
#define MCU_65C02_HPP

#include "mcu_instr.hpp"

// 65c02 extensions

// Old instructions with new addressing modes
DECLARE_INSTRUCTION( 0x12, ORA, am_Indirect_ZP );
DECLARE_INSTRUCTION( 0x32, AND, am_Indirect_ZP );
DECLARE_INSTRUCTION( 0x52, EOR, am_Indirect_ZP );
DECLARE_INSTRUCTION( 0x72, ADC, am_Indirect_ZP );
DECLARE_INSTRUCTION( 0x92, STA, am_Indirect_ZP );
DECLARE_INSTRUCTION( 0xB2, LDA, am_Indirect_ZP );
DECLARE_INSTRUCTION( 0xD2, CMP, am_Indirect_ZP );
DECLARE_INSTRUCTION( 0xF2, SBC, am_Indirect_ZP );

DECLARE_INSTRUCTION( 0x34, BIT, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0x3C, BIT, am_Absolute_X );
DECLARE_INSTRUCTION( 0x89, BIT, am_Immediate );

DECLARE_INSTRUCTION( 0x1A, INC, am_Accum );
DECLARE_INSTRUCTION( 0x3A, DEC, am_Accum );

DECLARE_INSTRUCTION( 0x7C, JMP, am_AbsIdxIndirect );

// New instructions
DECLARE_INSTRUCTION( 0x64, STZ, am_ZeroPage );
DECLARE_INSTRUCTION( 0x74, STZ, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0x9C, STZ, am_Absolute );
DECLARE_INSTRUCTION( 0x9E, STZ, am_Absolute_X );

DECLARE_INSTRUCTION( 0x04, TSB, am_ZeroPage );
DECLARE_INSTRUCTION( 0x0C, TSB, am_ZeroPage );
DECLARE_INSTRUCTION( 0x14, TRB, am_Absolute );
DECLARE_INSTRUCTION( 0x1C, TRB, am_Absolute );

DECLARE_INSTRUCTION( 0x80, BRA, am_Immediate );

DECLARE_INSTRUCTION( 0x5A, PHY, am_Implied );
DECLARE_INSTRUCTION( 0x7A, PLY, am_Implied );
DECLARE_INSTRUCTION( 0xDA, PHX, am_Implied );
DECLARE_INSTRUCTION( 0xFA, PLX, am_Implied );

DEFINE_INSTRUCTION( STZ )
{
    memData = 0;
}

DEFINE_INSTRUCTION( TSB )
{
    uint8_t addressedByte = memData;

    rState.modifyFlag( (addressedByte & rState.regA) != 0, flag_Z );

    memData = addressedByte | rState.regA;
}

DEFINE_INSTRUCTION( TRB )
{
    uint8_t addressedByte = memData;

    rState.modifyFlag( (addressedByte & rState.regA) != 0, flag_Z );

    memData = addressedByte & ~rState.regA;
}

DEFINE_INSTRUCTION( BRA )
{
    rState.pcBranchOffset( memData );
}

DEFINE_INSTRUCTION( PHY )
{
    rState.stackPushByte( rState.regY );
}

DEFINE_INSTRUCTION( PLY )
{
    rState.regY = rState.stackPopByte();
    rState.testNegativeZero( rState.regY );
}

DEFINE_INSTRUCTION( PHX )
{
    rState.stackPushByte( rState.regX );
}

DEFINE_INSTRUCTION( PLX )
{
    rState.regX = rState.stackPopByte();
    rState.testNegativeZero( rState.regX );
}

#endif
