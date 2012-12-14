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

// These addressing modes access memory
enum eAddressingMode_Mem
{
    am_Immediate,
    am_ZeroPage,
    am_ZeroPage_X,
    am_ZeroPage_Y,
    am_Relative,
    am_Absolute,
    am_Absolute_X,
    am_Absolute_Y,
    am_Indirect,
    am_Indirect_X,
    am_Indirect_Y,
};

enum eAddressingMode_Register
{
    am_Accum,
};

// These addressing modes do not 
enum eAddressingMode_Null
{
    am_Implied,
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

    void stackPushByte( uint8_t value ) // Pushes a byte on to the stack
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

    uint8_t stackPopByte() // Pops a byte from the stack
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

    void stackPushWord( uint16_t value )
    {
        stackPushByte( static_cast<uint8_t>(value) );
        stackPushByte( static_cast<uint8_t>(value >> 8) );
    }

    uint16_t stackPopWord()
    {
        uint16_t result = stackPopByte();
        result |= (stackPopByte() << 8);
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

    void pcBranchOffset( uint8_t offset )
    {
        int8_t signedOffset = static_cast<int8_t>(offset);
        regPC += signedOffset;
    }

    class tMemoryAccessor
    {
        tMCUState& m_rState;
    public:
        uint16_t m_memAddr;

        tMemoryAccessor( tMCUState& rState, uint16_t memAddr ) : m_rState( rState ), m_memAddr( memAddr ) {}
        uint8_t operator=( uint8_t writeValue )
        { m_rState.memWriteByte( m_memAddr, writeValue ); }
        operator uint8_t() const
        { return m_rState.memReadByte( m_memAddr ); }
    };

    class tRegisterAccessor
    {
        uint8_t& m_rRegister;
    public:
        tRegisterAccessor( uint8_t& rRegister ) : m_rRegister( rRegister ) {}
        uint8_t operator=( uint8_t writeValue )
        { m_rRegister = writeValue; }
        operator uint8_t() const
        { return m_rRegister; }
    };

    class tNullAccessor
    {
        // Empty class - does nothing - will cause an error if an attempt is made to read/write to it
    };

    inline tMemoryAccessor makeAccessor( eAddressingMode_Mem mode )
    {
        switch( mode )
        {
        case am_Immediate:      return tMemoryAccessor( *this, ++regPC );
        case am_ZeroPage:       return tMemoryAccessor( *this, pcReadByte() );
        case am_ZeroPage_X:     return tMemoryAccessor( *this, (pcReadByte() + regX) & UINT8_MAX );
        case am_ZeroPage_Y:     return tMemoryAccessor( *this, (pcReadByte() + regY) & UINT8_MAX );
        case am_Relative:       return tMemoryAccessor( *this, ++regPC );
        case am_Absolute:       return tMemoryAccessor( *this, pcReadWord() );
        case am_Absolute_X:     return tMemoryAccessor( *this, pcReadWord() + regX );
        case am_Absolute_Y:     return tMemoryAccessor( *this, pcReadWord() + regY );
        case am_Indirect:       return tMemoryAccessor( *this, pcReadWord() );
        case am_Indirect_X:     return tMemoryAccessor( *this, memReadWord( (pcReadByte() + regX) & UINT8_MAX ) );
        case am_Indirect_Y:     return tMemoryAccessor( *this, memReadWord( pcReadByte() ) + regY );
        default:                assert( false );
        }

        return tMemoryAccessor( *this, 0 );
    }


    inline tRegisterAccessor makeAccessor( eAddressingMode_Register mode )
    {
        switch( mode )
        {
        case am_Accum:          return tRegisterAccessor( regA );
        default:                assert( false );
        }

        // Unknown accessor type!  Just return A
        return tRegisterAccessor( regA );
    }


    inline tNullAccessor makeAccessor( eAddressingMode_Null )
    { return tNullAccessor(); }

    /*
    am_Immediate,
    am_ZeroPage,
    am_ZeroPage_X,
    am_ZeroPage_Y,
    am_Relative,
    am_Absolute,
    am_Absolute_X,
    am_Absolute_Y,
    am_Indirect,
    am_Indirect_X,
    am_Indirect_Y,
*/
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
    inline void testNegativeZero( const uint8_t& rValue ) { testNegative( rValue ); testZero( rValue ); }

    // Returns whether or not two bytes have the same sign
    inline static bool sameSign( uint8_t value1, uint8_t value2 )
    { return ((value1 ^ value2) & 0x80) == 0; }

private:
    tMCUState(); // Disallowed - always need a pointer to memory
};

#endif
