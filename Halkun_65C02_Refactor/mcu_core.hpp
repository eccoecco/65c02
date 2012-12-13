#ifndef MCU_CORE_HPP
#define MCU_CORE_HPP

#include <cstdint>
#include <cassert>

enum eFlags
{
    flag_C = 0x01, // Carry
    flag_Z = 0x02, // Zero
    flag_I = 0x04, // IRQ
    flag_D = 0x08, // Decimal
    flag_B = 0x10, // Break
    flag_X = 0x20, // Undefined (?) - always set
    flag_V = 0x40, // Sign bit overflow
    flag_N = 0x80, // Negative
};

// These addressing modes return a reference to a byte of data
enum eAddressingMode
{
    am_Implied,
    am_Accum,
    am_Immediate,
    am_ZeroPage,
    am_ZeroPage_X,
    am_ZeroPage_Y,
    am_Relative,
    am_Absolute,
    am_Absolute_X,
    am_Absolute_Y,
    am_Indirect_X,
    am_Indirect_Y,
};

struct tMCUState
{
    uint8_t regA; // Accumulator
    uint8_t regX; // Index register X
    uint8_t regY; // Index register Y
    uint8_t regP; // Processor (status flag) register
    uint16_t regPC; // Program counter
    uint8_t regSP; // Stack pointer

    uint8_t *m_pMemory; // Pointer to memory

    // Constants
    static const uint16_t cResetVector = 0xFFFC; // Address where the reset vector should be
    static const uint16_t cStackOffset = 0x0100; // Address in memory where the stack is offset

    // Constructor - pass in 64k of memory
    tMCUState( uint8_t *pMemory ) : m_pMemory( pMemory )
    { cpuReset(); }

    // Useful functions
    void cpuReset()
    {
        regA = regX = regY = 0;
        regP = 0;
        regSP = UINT8_MAX;
        regPC = memReadWord( cResetVector );
    }

    // @TODO: Allow access to special memory locations
    uint8_t memReadByte( uint16_t address ) const
    { return m_pMemory[address]; }

    uint16_t memReadWord( uint16_t address ) const
    {
        // Returns a 16-bit word (little endian) from address
        uint16_t finalWord = memReadByte( address );
        finalWord |= memReadByte( address + 1 ) << 8;
        return finalWord;
    }

    void memWriteByte( uint16_t address, uint8_t data )
    { m_pMemory[address] = data; }

    void stackPush( uint8_t value ) // Pushes a byte on to the stack
    {
        assert( regSP > 0 );

        if( regSP > 0 )
        {
            memWriteByte( cStackOffset + regSP, value );
            --regSP;
        }
        else
        {
            // Stack overflow!  Should this be logged or something?
        }
    }

    uint8_t stackPop() // Pops a byte from the stack
    {
        assert( regSP < UINT8_MAX );

        if( regSP < UINT8_MAX )
        {
            ++regSP;
            return memReadByte( cStackOffset + regSP );
        }

        // Stack underflow!
        return 0;
    }

    uint8_t pcReadByte() // Reads a byte from where PC is, and increments PC
    {
        return memReadByte( regPC++ );
    }

    uint16_t pcReadWord() // Reads a word from where PC is, and adds 2 to PC
    {
        uint16_t retVal = memReadWord( regPC );
        regPC += 2;
        return retVal;
    }

    // Returns a reference to the data that's addressed
    inline uint8_t& pcDecodeAddress( eAddressingMode mode )
    {
        switch ( mode )
        {
        default: assert( false ); // Unknown addressing mode!  Fall through to accumulator
        case am_Implied:        return regA; // Just return something valid
        case am_Accum:          return regA;
        case am_Immediate:      return m_pMemory[ ++regPC ];
        case am_ZeroPage:       return m_pMemory[ pcReadByte() ];
        case am_ZeroPage_X:     return m_pMemory[ (pcReadByte() + regX) & UINT8_MAX ];
        case am_ZeroPage_Y:     return m_pMemory[ (pcReadByte() + regY) & UINT8_MAX ];
        case am_Relative:       return m_pMemory[ ++regPC ];
        case am_Absolute:       return m_pMemory[ pcReadWord() ];
        case am_Absolute_X:     return m_pMemory[ pcReadWord() + regX ];
        case am_Absolute_Y:     return m_pMemory[ pcReadWord() + regY ];
        case am_Indirect_X:     return m_pMemory[ memReadWord( (pcReadByte() + regX) & UINT8_MAX ) ];
        case am_Indirect_Y:     return m_pMemory[ memReadWord( pcReadByte() ) + regY ];
        }
    }

    inline void modifyFlag( bool setFlag, eFlags flags )
    {
        uint8_t flagMask = static_cast<uint8_t>(flags);

        if( setFlag )
            regP |= flagMask;
        else
            regP &= ~flagMask;
    }

    inline bool isFlagSet( eFlags flag )
    {
        return (regP & flag) != 0;
    }

    inline void testNegative( const uint8_t& rValue ) { modifyFlag( (rValue & 0x80) != 0, flag_N ); }
    inline void testZero( const uint8_t& rValue ) { modifyFlag( (rValue == 0), flag_Z ); }

    // Returns whether or not two bytes have the same sign
    inline static bool sameSign( uint8_t value1, uint8_t value2 )
    { return ((value1 ^ value2) & 0x80) == 0; }

private:
    tMCUState(); // Disallowed - always need a pointer to memory
};

#endif
