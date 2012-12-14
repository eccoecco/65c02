#include "mcu_core.hpp"
#include "mcu_instr.hpp"

DECLARE_INSTRUCTION( 0x01, ORA, am_Indirect_X );
DECLARE_INSTRUCTION( 0x05, ORA, am_ZeroPage );
DECLARE_INSTRUCTION( 0x09, ORA, am_Immediate );
DECLARE_INSTRUCTION( 0x0D, ORA, am_Absolute );
DECLARE_INSTRUCTION( 0x11, ORA, am_Indirect_Y );
DECLARE_INSTRUCTION( 0x15, ORA, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0x19, ORA, am_Absolute_Y );
DECLARE_INSTRUCTION( 0x1D, ORA, am_Absolute_X );

DECLARE_INSTRUCTION( 0x21, AND, am_Indirect_X );
DECLARE_INSTRUCTION( 0x25, AND, am_ZeroPage );
DECLARE_INSTRUCTION( 0x29, AND, am_Immediate );
DECLARE_INSTRUCTION( 0x2D, AND, am_Absolute );
DECLARE_INSTRUCTION( 0x31, AND, am_Indirect_Y );
DECLARE_INSTRUCTION( 0x35, AND, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0x39, AND, am_Absolute_Y );
DECLARE_INSTRUCTION( 0x3D, AND, am_Absolute_X );

DECLARE_INSTRUCTION( 0x41, EOR, am_Indirect_X );
DECLARE_INSTRUCTION( 0x45, EOR, am_ZeroPage );
DECLARE_INSTRUCTION( 0x49, EOR, am_Immediate );
DECLARE_INSTRUCTION( 0x4D, EOR, am_Absolute );
DECLARE_INSTRUCTION( 0x51, EOR, am_Indirect_Y );
DECLARE_INSTRUCTION( 0x55, EOR, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0x59, EOR, am_Absolute_Y );
DECLARE_INSTRUCTION( 0x5D, EOR, am_Absolute_X );

DECLARE_INSTRUCTION( 0x61, ADC, am_Indirect_X );
DECLARE_INSTRUCTION( 0x65, ADC, am_ZeroPage );
DECLARE_INSTRUCTION( 0x69, ADC, am_Immediate );
DECLARE_INSTRUCTION( 0x6D, ADC, am_Absolute );
DECLARE_INSTRUCTION( 0x71, ADC, am_Indirect_Y );
DECLARE_INSTRUCTION( 0x75, ADC, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0x79, ADC, am_Absolute_Y );
DECLARE_INSTRUCTION( 0x7D, ADC, am_Absolute_X );

DECLARE_INSTRUCTION( 0x81, STA, am_Indirect_X );
DECLARE_INSTRUCTION( 0x85, STA, am_ZeroPage );
DECLARE_INSTRUCTION( 0x8D, STA, am_Absolute );
DECLARE_INSTRUCTION( 0x91, STA, am_Indirect_Y );
DECLARE_INSTRUCTION( 0x95, STA, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0x99, STA, am_Absolute_Y );
DECLARE_INSTRUCTION( 0x9D, STA, am_Absolute_X );

DECLARE_INSTRUCTION( 0xA1, LDA, am_Indirect_X );
DECLARE_INSTRUCTION( 0xA5, LDA, am_ZeroPage );
DECLARE_INSTRUCTION( 0xA9, LDA, am_Immediate );
DECLARE_INSTRUCTION( 0xAD, LDA, am_Absolute );
DECLARE_INSTRUCTION( 0xB1, LDA, am_Indirect_Y );
DECLARE_INSTRUCTION( 0xB5, LDA, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0xB9, LDA, am_Absolute_Y );
DECLARE_INSTRUCTION( 0xBD, LDA, am_Absolute_X );

DECLARE_INSTRUCTION( 0xC1, CMP, am_Indirect_X );
DECLARE_INSTRUCTION( 0xC5, CMP, am_ZeroPage );
DECLARE_INSTRUCTION( 0xC9, CMP, am_Immediate );
DECLARE_INSTRUCTION( 0xCD, CMP, am_Absolute );
DECLARE_INSTRUCTION( 0xD1, CMP, am_Indirect_Y );
DECLARE_INSTRUCTION( 0xD5, CMP, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0xD9, CMP, am_Absolute_Y );
DECLARE_INSTRUCTION( 0xDD, CMP, am_Absolute_X );

DECLARE_INSTRUCTION( 0xE1, SBC, am_Indirect_X );
DECLARE_INSTRUCTION( 0xE5, SBC, am_ZeroPage );
DECLARE_INSTRUCTION( 0xE9, SBC, am_Immediate );
DECLARE_INSTRUCTION( 0xED, SBC, am_Absolute );
DECLARE_INSTRUCTION( 0xF1, SBC, am_Indirect_Y );
DECLARE_INSTRUCTION( 0xF5, SBC, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0xF9, SBC, am_Absolute_Y );
DECLARE_INSTRUCTION( 0xFD, SBC, am_Absolute_X );

DEFINE_INSTRUCTION( ORA )
{
    rState.regA |= memData;
    rState.testNegativeZero( rState.regA );
}

DEFINE_INSTRUCTION( AND )
{
    rState.regA &= memData;
    rState.testNegativeZero( rState.regA );
}

DEFINE_INSTRUCTION( EOR )
{
    rState.regA ^= memData;
    rState.testNegativeZero( rState.regA );
}

DEFINE_INSTRUCTION( ADC )
{
    uint8_t addressedByte = memData;

    // Overflow can only occur when adding two bytes that have identical signs
    bool canOverflow = rState.sameSign( rState.regA, addressedByte );

    if( rState.isFlagSet( flag_D ) )
    {
        // Decimal mode - based on http://www.6502.org/tutorials/decimal_mode.html#A
        // to also give same results when invalid data is passed in
        uint16_t tmp = (rState.regA & 0xF) + (addressedByte & 0xF);
        if( rState.isFlagSet( flag_C ) )
            ++tmp;

        if( tmp >= 10 ) // maps 11 -> 19 to 0x11 -> 0x19 (17 -> 25)
            tmp = ((tmp + 6) & 0xF) + 0x10;

        tmp += (rState.regA & 0xF0) + (addressedByte & 0xF0);

        if( tmp >= 0xA0 ) // 0xA0 represents 100, so add 0x60 to convert 0xA0 to 0x100
            tmp += 0x60;

        rState.regA = tmp & UINT8_MAX;
        rState.modifyFlag( tmp >= 0x100, flag_C );
    }
    else
    {
        uint16_t tmp = rState.regA;
        tmp += addressedByte;
        if( rState.isFlagSet( flag_C ) )
            ++tmp;

        rState.regA = tmp & UINT8_MAX;
        rState.modifyFlag( tmp > UINT8_MAX, flag_C );
    }

    // Overflow has occurred if (not sure if this is true in decimal mode as well):
    // 1.) We were adding two byte with identical signs
    // 2.) The result now has a different sign than the original
    rState.modifyFlag( canOverflow && !rState.sameSign( rState.regA, addressedByte ), flag_V );
    rState.testNegativeZero( rState.regA );
}

DEFINE_INSTRUCTION( STA )
{
    memData = rState.regA;
}

DEFINE_INSTRUCTION( LDA )
{
    rState.regA = memData;
    rState.testNegativeZero( rState.regA );
}

DEFINE_INSTRUCTION( CMP )
{
    uint8_t addressedByte = memData;
    uint8_t tmp = rState.regA - addressedByte;
    rState.modifyFlag( rState.regA >= addressedByte, flag_C );
    rState.testNegativeZero( tmp );
}

DEFINE_INSTRUCTION( SBC )
{
    uint8_t addressedByte = memData;
    // Overflow can only occur when subtracting two bytes that have different signs
    bool canOverflow = !rState.sameSign( rState.regA, addressedByte );

    if( rState.isFlagSet( flag_D ) )
    {
        // Decimal mode - based on http://www.6502.org/tutorials/decimal_mode.html#A
        // to also give same results when invalid data is passed in
        //   This is based on sequence 4, for 65c02 emulation
        int16_t tmp = (rState.regA & 0xF) - (addressedByte & 0xF);
        int16_t result = rState.regA - addressedByte;

        if( !rState.isFlagSet( flag_C ) )
        {
            --tmp;
            --result;
        }

        // Carry flag has same behaviour as in binary mode
        rState.modifyFlag( static_cast<uint16_t>(result) <= UINT8_MAX, flag_C );

        if( result < 0 )
            result -= 0x60;

        if( tmp < 0 )
            result -= 0x06;

        rState.regA = result & UINT8_MAX;
        rState.modifyFlag( tmp >= 0x100, flag_C );
    }
    else
    {
        uint16_t tmp = rState.regA - addressedByte;
        if( !rState.isFlagSet( flag_C ) )
            --tmp;

        rState.regA = tmp & UINT8_MAX;
        rState.modifyFlag( tmp <= UINT8_MAX, flag_C );
    }

    rState.modifyFlag( canOverflow && rState.sameSign( rState.regA, addressedByte ), flag_V );
    // Overflow has occurred if:
    // 1.) We were adding two byte with different signs
    // 2.) The result now has a different sign than the original
    //      (this is implied to have happened if it has the same sign as the addressed byte)
    rState.testNegativeZero( rState.regA );
}

DECLARE_INSTRUCTION( 0x06, ASL, am_ZeroPage );
DECLARE_INSTRUCTION( 0x0A, ASL, am_Accum );
DECLARE_INSTRUCTION( 0x0E, ASL, am_Absolute );
DECLARE_INSTRUCTION( 0x16, ASL, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0x1E, ASL, am_Absolute_X );

DECLARE_INSTRUCTION( 0x26, ROL, am_ZeroPage );
DECLARE_INSTRUCTION( 0x2A, ROL, am_Accum );
DECLARE_INSTRUCTION( 0x2E, ROL, am_Absolute );
DECLARE_INSTRUCTION( 0x36, ROL, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0x3E, ROL, am_Absolute_X );

DECLARE_INSTRUCTION( 0x46, LSR, am_ZeroPage );
DECLARE_INSTRUCTION( 0x4A, LSR, am_Accum );
DECLARE_INSTRUCTION( 0x4E, LSR, am_Absolute );
DECLARE_INSTRUCTION( 0x56, LSR, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0x5E, LSR, am_Absolute_X );

DECLARE_INSTRUCTION( 0x66, ROR, am_ZeroPage );
DECLARE_INSTRUCTION( 0x6A, ROR, am_Accum );
DECLARE_INSTRUCTION( 0x6E, ROR, am_Absolute );
DECLARE_INSTRUCTION( 0x76, ROR, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0x7E, ROR, am_Absolute_X );

DECLARE_INSTRUCTION( 0x86, STX, am_ZeroPage );
DECLARE_INSTRUCTION( 0x8E, STX, am_Absolute );
DECLARE_INSTRUCTION( 0x96, STX, am_ZeroPage_Y );

DECLARE_INSTRUCTION( 0xA2, LDX, am_Immediate );
DECLARE_INSTRUCTION( 0xA6, LDX, am_ZeroPage );
DECLARE_INSTRUCTION( 0xAE, LDX, am_Absolute );
DECLARE_INSTRUCTION( 0xB6, LDX, am_ZeroPage_Y );
DECLARE_INSTRUCTION( 0xBE, LDX, am_Absolute_Y );

DECLARE_INSTRUCTION( 0xC6, DEC, am_ZeroPage );
DECLARE_INSTRUCTION( 0xCE, DEC, am_Absolute );
DECLARE_INSTRUCTION( 0xD6, DEC, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0xDE, DEC, am_Absolute_X );

DECLARE_INSTRUCTION( 0xE6, INC, am_ZeroPage );
DECLARE_INSTRUCTION( 0xEE, INC, am_Absolute );
DECLARE_INSTRUCTION( 0xF6, INC, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0xFE, INC, am_Absolute_X );

DEFINE_INSTRUCTION( ASL )
{
    uint8_t addressedByte = memData;
    rState.modifyFlag( (addressedByte & 0x80) != 0, flag_C );
    addressedByte <<= 1;
    memData = addressedByte;
    rState.testNegativeZero( addressedByte );
}

DEFINE_INSTRUCTION( ROL )
{
    bool carryFlag = rState.isFlagSet( flag_C );

    uint8_t addressedByte = memData;

    rState.modifyFlag( (addressedByte & 0x80) != 0, flag_C );
    addressedByte <<= 1;

    if( carryFlag ) addressedByte |= 0x01;

    memData = addressedByte;

    rState.testNegativeZero( addressedByte );
}

DEFINE_INSTRUCTION( LSR )
{
    uint8_t addressedByte = memData;

    rState.modifyFlag( (addressedByte & 0x01) != 0, flag_C );
    addressedByte >>= 1;
    addressedByte &= 0x7F;

    memData = addressedByte;
    rState.testNegativeZero( addressedByte ); // Wouldn't this always clear the N flag?
}

DEFINE_INSTRUCTION( ROR )
{
    uint16_t tmp = memData;
    if( rState.isFlagSet( flag_C ) )
        tmp |= 0x0100;

    rState.modifyFlag( (tmp & 0x01) != 0, flag_C );
    tmp >>= 1;

    memData = static_cast<uint8_t>(tmp);
    rState.testNegativeZero( static_cast<uint8_t>(tmp) );
}

DEFINE_INSTRUCTION( STX )
{
    memData = rState.regX;
}

DEFINE_INSTRUCTION( LDX )
{
    rState.regX = memData;
    rState.testNegativeZero( rState.regX );
}

DEFINE_INSTRUCTION( DEC )
{
    uint8_t tmp = memData;
    --tmp;
    memData = tmp;

    rState.testNegativeZero( tmp );
}

DEFINE_INSTRUCTION( INC )
{
    uint8_t tmp = memData;
    ++tmp;
    memData = tmp;

    rState.testNegativeZero( tmp );
}

DECLARE_INSTRUCTION( 0x24, BIT, am_ZeroPage );
DECLARE_INSTRUCTION( 0x2C, BIT, am_Absolute );

// The various jumps are special exceptions where we manually get the next word
DECLARE_INSTRUCTION( 0x4C, JMP, am_Absolute );
DECLARE_INSTRUCTION( 0x6C, JMP, am_Indirect );

DECLARE_INSTRUCTION( 0x84, STY, am_ZeroPage );
DECLARE_INSTRUCTION( 0x8C, STY, am_Absolute );
DECLARE_INSTRUCTION( 0x94, STY, am_ZeroPage_X );

DECLARE_INSTRUCTION( 0xA0, LDY, am_Immediate );
DECLARE_INSTRUCTION( 0xA4, LDY, am_ZeroPage );
DECLARE_INSTRUCTION( 0xAC, LDY, am_Absolute );
DECLARE_INSTRUCTION( 0xB4, LDY, am_ZeroPage_X );
DECLARE_INSTRUCTION( 0xBC, LDY, am_Absolute_X );

DECLARE_INSTRUCTION( 0xC0, CPY, am_Immediate );
DECLARE_INSTRUCTION( 0xC4, CPY, am_ZeroPage );
DECLARE_INSTRUCTION( 0xCC, CPY, am_Absolute );

DECLARE_INSTRUCTION( 0xE0, CPX, am_Immediate );
DECLARE_INSTRUCTION( 0xE4, CPX, am_ZeroPage );
DECLARE_INSTRUCTION( 0xEC, CPX, am_Absolute );

DEFINE_INSTRUCTION( BIT )
{
    uint8_t addressedByte = memData;
    rState.modifyFlag( (rState.regA & addressedByte) == 0, flag_Z );
    rState.modifyFlag( (addressedByte & 0x80 ) != 0, flag_N );
    rState.modifyFlag( (addressedByte & 0x40 ) != 0, flag_V );
}

DEFINE_INSTRUCTION( JMP )
{
    // Assume a memory memData
    if( opCode == 0x4C )
        rState.regPC = memData.m_memAddr;
    else if( opCode == 0x6C )
        rState.regPC = rState.memReadWord( memData.m_memAddr );
    else if( opCode == 0x7C )
        rState.regPC = rState.memReadWord( memData.m_memAddr );
    else
    {
        assert( false );
    }
}

DEFINE_INSTRUCTION( STY )
{
    memData = rState.regY;
}

DEFINE_INSTRUCTION( LDY )
{
    rState.regY = memData;
    rState.testNegativeZero( memData );
}

DEFINE_INSTRUCTION( CPY )
{
    uint8_t addressedByte = memData;
    rState.modifyFlag( rState.regY >= addressedByte, flag_C );
    rState.modifyFlag( rState.regY == addressedByte, flag_Z );
    rState.modifyFlag( rState.regY < addressedByte, flag_N );
}

DEFINE_INSTRUCTION( CPX )
{
    uint8_t addressedByte = memData;

    rState.modifyFlag( rState.regX >= addressedByte, flag_C );
    rState.modifyFlag( rState.regX == addressedByte, flag_Z );
    rState.modifyFlag( rState.regX < addressedByte, flag_N );
}

DECLARE_INSTRUCTION( 0x10, BPL, am_Immediate );
DECLARE_INSTRUCTION( 0x30, BMI, am_Immediate );
DECLARE_INSTRUCTION( 0x50, BVC, am_Immediate );
DECLARE_INSTRUCTION( 0x70, BVS, am_Immediate );
DECLARE_INSTRUCTION( 0x90, BCC, am_Immediate );
DECLARE_INSTRUCTION( 0xB0, BCS, am_Immediate );
DECLARE_INSTRUCTION( 0xD0, BNE, am_Immediate );
DECLARE_INSTRUCTION( 0xF0, BEQ, am_Immediate );

DEFINE_INSTRUCTION( BPL )
{
    if( !rState.isFlagSet( flag_N ) )
        rState.pcBranchOffset( memData );
}

DEFINE_INSTRUCTION( BMI )
{
    if( rState.isFlagSet( flag_N ) )
        rState.pcBranchOffset( memData );
}

DEFINE_INSTRUCTION( BVC )
{
    if( !rState.isFlagSet( flag_V ) )
        rState.pcBranchOffset( memData );
}

DEFINE_INSTRUCTION( BVS )
{
    if( rState.isFlagSet( flag_V ) )
        rState.pcBranchOffset( memData );
}

DEFINE_INSTRUCTION( BCC )
{
    if( !rState.isFlagSet( flag_C ) )
        rState.pcBranchOffset( memData );
}

DEFINE_INSTRUCTION( BCS )
{
    if( rState.isFlagSet( flag_C ) )
        rState.pcBranchOffset( memData );
}

DEFINE_INSTRUCTION( BNE )
{
    if( !rState.isFlagSet( flag_Z ) )
        rState.pcBranchOffset( memData );
}

DEFINE_INSTRUCTION( BEQ )
{
    if( rState.isFlagSet( flag_Z ) )
        rState.pcBranchOffset( memData );
}

DECLARE_INSTRUCTION( 0x00, BRK, am_Implied );
DECLARE_INSTRUCTION( 0x20, JSR, am_Absolute );
DECLARE_INSTRUCTION( 0x40, RTI, am_Implied );
DECLARE_INSTRUCTION( 0x60, RTS, am_Implied );

DEFINE_INSTRUCTION( BRK )
{
    rState.regP |= flag_B;
}

DEFINE_INSTRUCTION( JSR )
{
    rState.stackPushWord( rState.regPC - 1 );
    rState.regPC = memData.m_memAddr;
}

DEFINE_INSTRUCTION( RTI )
{
    // TODO - correct flags? - reference code has break bit set
    rState.regP = rState.stackPopByte();
    rState.regPC = rState.stackPopWord();
}

DEFINE_INSTRUCTION( RTS )
{
    rState.regPC = rState.stackPopWord() + 1;
}

DECLARE_INSTRUCTION( 0x08, PHP, am_Implied );
DECLARE_INSTRUCTION( 0x28, PLP, am_Implied );
DECLARE_INSTRUCTION( 0x48, PHA, am_Implied );
DECLARE_INSTRUCTION( 0x68, PLA, am_Implied );
DECLARE_INSTRUCTION( 0x88, DEY, am_Implied );
DECLARE_INSTRUCTION( 0xA8, TAY, am_Implied );
DECLARE_INSTRUCTION( 0xC8, INY, am_Implied );
DECLARE_INSTRUCTION( 0xE8, INX, am_Implied );
DECLARE_INSTRUCTION( 0x18, CLC, am_Implied );
DECLARE_INSTRUCTION( 0x38, SEC, am_Implied );
DECLARE_INSTRUCTION( 0x58, CLI, am_Implied );
DECLARE_INSTRUCTION( 0x78, SEI, am_Implied );
DECLARE_INSTRUCTION( 0x98, TYA, am_Implied );
DECLARE_INSTRUCTION( 0xB8, CLV, am_Implied );
DECLARE_INSTRUCTION( 0xD8, CLD, am_Implied );
DECLARE_INSTRUCTION( 0xF8, SED, am_Implied );
DECLARE_INSTRUCTION( 0x8A, TXA, am_Implied );
DECLARE_INSTRUCTION( 0x9A, TXS, am_Implied );
DECLARE_INSTRUCTION( 0xAA, TAX, am_Implied );
DECLARE_INSTRUCTION( 0xBA, TSX, am_Implied );
DECLARE_INSTRUCTION( 0xCA, DEX, am_Implied );
DECLARE_INSTRUCTION( 0xEA, NOP, am_Implied );

DEFINE_INSTRUCTION( PHP )
{
    rState.stackPushByte( rState.regP );
}

DEFINE_INSTRUCTION( PLP )
{
    rState.regP = rState.stackPopByte();
}

DEFINE_INSTRUCTION( PHA )
{
    rState.stackPushByte( rState.regA );
}

DEFINE_INSTRUCTION( PLA )
{
    rState.regA = rState.stackPopByte();
    rState.testNegativeZero( rState.regA );
}

DEFINE_INSTRUCTION( DEY )
{
    --rState.regY;
    rState.testNegativeZero( rState.regY );
}

DEFINE_INSTRUCTION( TAY )
{
    rState.regY = rState.regA;
    rState.testNegativeZero( rState.regY );
}

DEFINE_INSTRUCTION( INY )
{
    ++rState.regY;
    rState.testNegativeZero( rState.regY );
}

DEFINE_INSTRUCTION( INX )
{
    ++rState.regX;
    rState.testNegativeZero( rState.regX );
}

DEFINE_INSTRUCTION( CLC )
{
    rState.modifyFlag( false, flag_C );
}

DEFINE_INSTRUCTION( SEC )
{
    rState.modifyFlag( true, flag_C );
}

DEFINE_INSTRUCTION( CLI )
{
    rState.modifyFlag( false, flag_I );
}

DEFINE_INSTRUCTION( SEI )
{
    rState.modifyFlag( true, flag_I );
}

DEFINE_INSTRUCTION( TYA )
{
    rState.regA = rState.regY;
    rState.testNegativeZero( rState.regA );
}

DEFINE_INSTRUCTION( CLV )
{
    rState.modifyFlag( false, flag_V );
}

DEFINE_INSTRUCTION( CLD )
{
    rState.modifyFlag( false, flag_D );
}

DEFINE_INSTRUCTION( SED )
{
    rState.modifyFlag( true, flag_D );
}

DEFINE_INSTRUCTION( TXA )
{
    rState.regA = rState.regX;
    rState.testNegativeZero( rState.regA );
}

DEFINE_INSTRUCTION( TXS )
{
    rState.regSP = rState.regX;
    rState.testNegativeZero( rState.regSP );
}

DEFINE_INSTRUCTION( TAX )
{
    rState.regX = rState.regA;
    rState.testNegativeZero( rState.regX );
}

DEFINE_INSTRUCTION( TSX )
{
    rState.regX = rState.regSP;
    rState.testNegativeZero( rState.regX );
}

DEFINE_INSTRUCTION( DEX )
{
    --rState.regX;
    rState.testNegativeZero( rState.regX );
}

DEFINE_INSTRUCTION( NOP )
{
    // No op
}

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
