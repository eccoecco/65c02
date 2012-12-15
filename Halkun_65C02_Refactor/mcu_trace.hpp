#ifndef MCU_TRACE_HPP
#define MCU_TRACE_HPP

#include <cstdint>
#include <queue>

#ifdef DO_MCU_TRACE

struct tMemoryTrace
{
    uint16_t m_address;
    uint8_t m_value;
    bool m_isRead;

    tMemoryTrace( uint16_t addr, uint8_t val, bool read ) : m_address( addr ), m_value( val ), m_isRead( read )
    {}
};

typedef std::queue< tMemoryTrace > tMemoryTraceQueue;

struct tTraceState
{
    int regA;
    int regX;
    int regY;
    int regP;
    int regPC;
    int regSP;
};

#endif

#endif
