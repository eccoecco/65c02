#include <iostream>
#include <cstdint>
#include <cassert>

template <uint8_t opCode> inline void opCodeExecute( uint8_t arg )
{
    std::cout << "Unimplemented opcode: " << opCode << std::endl;
    assert( false );
}

//template <> inline void opCodeExecute<3>( uint8_t arg );

template <> inline void opCodeExecute<5>( uint8_t arg )
{
    std::cerr << "Hello!";
}

int main( int argc, char *argv[] )
{
    switch( argc )
    {
    case 0: opCodeExecute<0>( argc ); break;
    case 1: opCodeExecute<1>( argc ); break;
    case 2: opCodeExecute<2>( argc ); break;
    case 3: opCodeExecute<3>( argc ); break;
    case 4: opCodeExecute<4>( argc ); break;
    case 5: opCodeExecute<5>( argc ); break;
    case 6: opCodeExecute<6>( argc ); break;
    case 7: opCodeExecute<7>( argc ); break;
    default:
        break;
    }

    return 0;
}