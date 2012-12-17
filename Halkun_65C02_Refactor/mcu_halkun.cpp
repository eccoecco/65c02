/*
// JoshX '77 65c02 Core for the Arduino 2560 Mega
// By Joshua "Halkun" Walker for Paul "Homr" Klimek
// Based on the Javascript 6502 core by Stian Soreng
// and the Arduino port by Rickard Andersson
//
// This code is GPL
// TODO: EEPROM?
*/

#include "mcu_trace.hpp"
#include <cstdint>

#define UMASK  0xFF
#define C_FLAG	0x01
#define Z_FLAG	0x02
#define I_FLAG	0x04
#define D_FLAG	0x08
#define B_FLAG 	0x10
#define X_FLAG	0x20
#define V_FLAG	0x40
#define N_FLAG	0x80


typedef uint8_t byte;

/*
// Inital 6502 values
// These are type "int" becuse gcc will promote byte
// to full ints for bitwise operations. It's either
// cast on every bit flip, or just bitmask 0xFF 
*/

int regA = 0;
int regX = 0;
int regY = 0;
int regP = 0;
int regPC = 0;
int regSP = 0xFF;
//byte * const memory = (byte *) 0x8000; 		//0x8000=0xFFFF 32k window.
byte current_bank;

void banksel( byte )
{
    // Do nothing
}

#ifdef DO_MCU_TRACE

tMemoryTraceQueue g_traceQueue;
const uint8_t *g_pReadSequence = 0;
uint8_t g_lastWriteResult = 0;

void setTraceState( const tTraceState& rTrace )
{
    regA = rTrace.regA;
    regX = rTrace.regX;
    regY = rTrace.regY;
    regP = rTrace.regP;
    regPC = rTrace.regPC;
    regSP = rTrace.regSP;
}

tTraceState getTraceState()
{
    tTraceState trace;

    trace.regA = regA;
    trace.regX = regX;
    trace.regY = regY;
    trace.regP = regP;
    trace.regPC = regPC;
    trace.regSP = regSP;

    return trace;
}

void resetLastWrite()
{
    g_lastWriteResult = 0;
}

uint8_t getLastWrite()
{
    return g_lastWriteResult;
}

void setReadSequence( const uint8_t *pDebugRead )
{
    g_pReadSequence = pDebugRead;
}

tMemoryTraceQueue& getMemoryTraceQueue()
{
    return g_traceQueue;
}

void memStoreByte(int addr, byte value )
{
    g_traceQueue.push( tMemoryTrace( addr, value, false ) );
    g_lastWriteResult = value;
}

// memReadByte() - Peek a byte, don't touch any registers
byte memReadByte(int addr ) 
{ 
    byte readValue = *g_pReadSequence;
    g_traceQueue.push( tMemoryTrace( addr, readValue, true ) );

    g_pReadSequence++;

    return readValue;
}

#else

uint8_t     g_pMemory[65536];

// memStoreByte() - Poke a byte, don't touch any registers
void memStoreByte(int addr,byte value )
{
	// Write registers
/*	if(addr==0x0302) {Serial.write(value& 0xff);}  
	else
	if(addr==0x0300) {LCD.write(value& 0xff);}  
	else
	if(addr==0x0308) {banksel(value& 0xff);}  
	else*/
	{
		byte save_bank=current_bank;
		if (addr <= 0x7FFF)
		{
			banksel(0);
			g_pMemory[addr]=value;
			banksel(save_bank);
		}
		if (addr >= 0x8000)
		{
			g_pMemory[addr-0x8000]=value;
		}
	}
}

// memReadByte() - Peek a byte, don't touch any registers
byte memReadByte(int addr ) 
{  

/*	if(addr==0x0303)  {return (serial_read());}	
	else
	if(addr==0x0301)  {return (keypad_read());}
	else*/
	{
		byte value;
		byte save_bank = current_bank;
		if (addr <= 0x7FFF)
		{
			banksel(0);
			value=g_pMemory[addr] ;
			banksel(save_bank);
		}
		if (addr >= 0x8000)
		{
			value=g_pMemory[addr-0x8000];
		}
		return value;
	}
}

#endif


// CPUreset() - Resets CPU to startup state
void CPUreset() {
	banksel(1);                            //bank 1 is always loaded on reset, it has the reset vector
	regA = regX = regY = 0;
	regSP = 0xFF;
	regP = 0;
	regPC =  memReadByte(0xFFFC) + (memReadByte(0xFFFD) << 8);
}

// stackPush() - Push byte to stack
void stackPush(int value ) {
	byte save_bank=current_bank;
	banksel(0);
	if( regSP > 0 ) {
        memStoreByte( (regSP&0xff)+0x100, value & 0xff );
		regSP--;
//		memory[(regSP&0xff)+0x100] = value & 0xff;
	}
	banksel(save_bank);
}

// // stackPop() - Pop byte from stack
int stackPop() {
	int value;
	byte save_bank=current_bank;
	banksel(0);
	if( regSP < 0xff ) {
//		value = memory[regSP+0x100];
		regSP++;
        value = memReadByte( regSP + 0x100 );
		banksel(save_bank);
		return(value);
	} 
	else
	{
		banksel(save_bank);
		return(0);
	}
}

// popByte() - Pops a byte
int popByte()
{
	int value = memReadByte(regPC);
	regPC++;
	return(value & 0xff);
}

// popWord() - Pops a word using popByte() twice
int popWord() {
	return popByte() + (popByte() << 8);
}

// memStoreByte() - Poke a byte, don't touch any registers

//set zero and negative processor flags based on result
void setNVflags(int val)
{
    val &= 0xFF;

	if( val ) 
	{regP &= ~Z_FLAG;}
	else 
	{regP |= Z_FLAG;}
	
	if( val & N_FLAG )
	{regP |= N_FLAG;}
	else
	{regP &= ~N_FLAG;}
}

void setNVflagsForRegA()
{
	setNVflags(regA);
}
void setNVflagsForRegX()
{
	setNVflags(regX);
}
void setNVflagsForRegY()
{
	setNVflags(regY);
}
void ORA(void)
{
	setNVflagsForRegA();
}
void AND(void)
{
	setNVflagsForRegA();
}
void EOR(void)
{
	setNVflagsForRegA();
}
void ASL(int val)
{
	setNVflags(val);
}
void LSR(int val)
{
	setNVflags(val);
}
void ROL(int val)
{
	setNVflags(val);
}
void ROR(int val)
{
	setNVflags(val);
}
void LDA(void)
{
	setNVflagsForRegA();
}
void LDX(void)
{
	setNVflagsForRegX();
}
void LDY(void)
{
	setNVflagsForRegY();
}


void BIT(int value)
{
	if (value & N_FLAG) 
	{regP |= N_FLAG;}
	else 
	{regP &= ~N_FLAG;}
	
	if (value & V_FLAG) 
	{regP |= V_FLAG;}
	else 
	{regP &= ~V_FLAG;}
	
	if (regA & value)
	{regP &= ~Z_FLAG;}
	else
	{regP |= Z_FLAG;}
}



// CLC() - CLear Carry
void CLC(void)
{
	regP &= ~C_FLAG;
}

// SEC() - SEt Carry
void SEC(void)
{
	regP |= C_FLAG;
}

// CLV - CLear oVerflow
void CLV(void)
{
	regP &= ~V_FLAG;
}

// setOverflow() - Sets overflow bit
void setOverflow(void)
{
	regP |= V_FLAG;
}

void dec(int addr)
{
	int value;
	value = memReadByte( addr );
	--value;
	memStoreByte( addr, value&UMASK );
	setNVflags(value);
}

void INC(int addr)
{
	int value;
	value = memReadByte( addr );
	++value;
	memStoreByte( addr, value&UMASK );
	setNVflags(value);
}




// jumpBranch() - Branch relative
void jumpBranch(int offset ) {
	if( offset > 0x7f )
	regPC = (regPC - (0x100 - offset));
	else
	regPC = (regPC + offset );
}


int overflowSet() {
	return regP & V_FLAG;
}

int decimalMode() {
	return regP & D_FLAG;
}

int carrySet() {
	return regP & C_FLAG;
}

int negativeSet() {
	return regP & N_FLAG;
}

int zeroSet() {
	return regP & Z_FLAG;
}




// doCompare() - Do a comparison
void doCompare(int reg,int val ) 
{
	if (reg >= val)
	{SEC();}
	else
	{CLC();}
	val = (reg - val);
	setNVflags(val);
}


// testSBC()
void testSBC(int value )
{
	int tmp,w;
	if ((regA ^ value) & N_FLAG)
	{setOverflow();}
	else
	{CLV();}

	if (decimalMode()) {
		tmp = 0xf + (regA & 0xf) - (value & 0xf) + carrySet();
		if (tmp < 0x10) {
			w = 0;
			tmp -= 6;
		} else {
			w = 0x10;
			tmp -= 0x10;
		}
		w += 0xf0 + (regA & 0xf0) - (value & 0xf0);
		if (w < 0x100) {
			CLC();
			if (overflowSet() && w < 0x80) { CLV(); }
			w -= 0x60;
		} else {
			SEC();
			if (overflowSet() && w >= 0x180) { CLV(); }
		}
		w += tmp;
	} else {
		w = UMASK + regA - value + carrySet();
		if (w < 1+UMASK) {
			CLC();
			if (overflowSet() && w < N_FLAG) { CLV(); }
		} else {
			SEC();
			if (overflowSet() && w >= (3*N_FLAG)) { CLV(); }
		}
	}
	regA = w & UMASK;
	setNVflagsForRegA();		


}


// testADC()
void testADC(int value) {
	int tmp;
	if ((regA ^ value) & N_FLAG) {
		CLV();
	} else {
		setOverflow();
	}

	if (decimalMode()) {
		tmp = (regA & 0xf) + (value & 0xf) + carrySet();
		if (tmp >= 10) {
			tmp = 0x10 | ((tmp + 6) & 0xf);
		}
		tmp += (regA & 0xf0) + (value & 0xf0);
		if (tmp >= 160) {
			SEC();
			if (overflowSet() && tmp >= 0x180) { CLV(); }
			tmp += 0x60;
		} else {
			CLC();
			if (overflowSet() && tmp < 0x80) { CLV(); }
		}
	} else {
		tmp = regA + value + carrySet();
		if (tmp >= 2*N_FLAG) {
			SEC();
			if (overflowSet() && tmp >= 3*N_FLAG) { CLV(); }
		} else {
			CLC();
			if (overflowSet() && tmp < N_FLAG) { CLV(); }
		}
	}
	regA = tmp & UMASK;
	setNVflagsForRegA();
}

int memGetWord(int addr)
{	
	int val;
	val= memReadByte( addr ) + (memReadByte( addr+1) << 8);
	return val;
}

void setCarryFlagFromBit7(int value)
{
	regP = (regP & ~C_FLAG) | ((value>>7)&1);
}

void setCarryFlagFromBit0(int value)
{
	regP = (regP & ~C_FLAG) | (value&1);
}

// execute() - Execute an instruction from memory
void execute() 
{
	int opcode,value,zp,offset,currAddr,sf,ov,addr,currP,currA;
	opcode = popByte();
	switch( opcode ) {
	case 0x00:                            // BRK implied
		if (regP & 0x04) {break;}
		else
		{
			regP |= B_FLAG;  //set break bit
//			stackPush( regPC );
            stackPush( (regPC >> 8) & UMASK);
            stackPush( (regPC & UMASK) );
			stackPush( regP );
			regP &= ~B_FLAG; //set brk back
			regPC = memReadByte( 0xFFFE ) + (memReadByte( 0xFFFF) << 8);
		}	
		break;
	case 0x01:                            // ORA INDX
		zp = (popByte() + regX) & UMASK;
		addr = memGetWord(zp);
		value = memReadByte( addr );
		regA |= value;
		ORA();
		break;		
	case 0x04:                            // TSB ZP
		zp = popByte();					  // Step 1: BIT operand (Z flag only)
		value = memReadByte( zp );
		if( value & regA ) regP &= 0xfd; else regP |= 0x02;
		currP = regP;					 // Step 2: PHP
		currA = regA;					 // Step 3: PHA		
		regA |= value;						 // Step 4: ORA operand (Z flag only)
		if( regA ) regP &= 0xfd; else regP |= 0x02;
		memStoreByte( zp, regA );		//Step 6. STA operand
		regA = currA;					//Step 7. PLA
		regP = currP;					//step 8. PLP
		break;
	case 0x05:                            // ORA ZP
		zp = popByte();
		regA |= memReadByte( zp );
		ORA();
		break;
	case 0x06:                            // ASL ZP
		zp = popByte();
		value = memReadByte( zp );
		setCarryFlagFromBit7(value);
		value = value << 1;
		memStoreByte( zp, value );
		ASL( value );
		break;
	case 0x08:                            // PHP
		stackPush( regP | 0x30);
		break;
	case 0x09:                            // ORA IMM
		regA |= popByte();
		ORA();
		break;
	case 0x0a:                            // ASL IMPL
		setCarryFlagFromBit7(regA);
		regA = (regA<<1) & UMASK;
		ASL(regA);
		break;	
	case 0x0c:                            // TSB ABS
        addr = popWord();
		value = memReadByte( addr );  //Setp 1: BIT oprand (Z flag only)
		if( value & regA ) regP &= 0xfd; else regP |= 0x02;
		currP = regP;					 // Step 2: PHP
		currA = regA;					 // Step 3: PHA		
        regA |= value;
		memStoreByte( addr, regA );		//Step 6. STA operand
		regA = currA;					//Step 7. PLA
		regP = currP;					//step 8. PLP
		break;
	case 0x0d:                            // ORA ABS
		regA |= memReadByte( popWord() );
		ORA();
		break;
	case 0x0e:                            // ASL ABS
		addr = popWord();
		value = memReadByte( addr );
		setCarryFlagFromBit7(value);
		value = value << 1;
		memStoreByte( addr, value );
		ASL(value);
		break;
	case 0x10:                            // BPL
		offset = popByte();
		if (!negativeSet()) { jumpBranch(offset); }
		break;	
	case 0x11:                            // ORA INDY
		zp = popByte();
		value = memGetWord(zp)+ regY;
		regA |= memReadByte(value);
		ORA();
		break;
	case 0x12:                            // ORA (ZP)
		zp = popByte();
		value = memGetWord(zp);
		regA |= memReadByte(value);
		ORA();
		break;	
	case 0x14:                            // TRB ZP
		zp = popByte();					  // Step 1: BIT operand (only Z flag)
		value = memReadByte( zp );
		if( value & regA ) regP &= 0xfd; else regP |= 0x02;
		currP = regP;					 // Step 2: PHP
		currA = regA;					 // Step 3: PHA		
		regA ^= 0xFF;					 // Step 4. EOR #$FF
		regA &= value;				    //Step 5. AND operand
		memStoreByte( zp, regA );		//Step 6. STA operand
		regA = currA;					//Step 7. PLA
		regP = currP;					//step 8. PLP
		break;
	case 0x15:                            // ORA ZPX
		addr = (popByte() + regX) & UMASK;
		regA |= memReadByte(addr);
		ORA();
		break;
	case 0x16:                            // ASL ZPX
		addr = (popByte() + regX) & UMASK;
		value = memReadByte(addr);
		setCarryFlagFromBit7(value);
		value = value << 1;
		memStoreByte( addr, value );
		ASL(value);
		break;
	case 0x18:                            // CLC
		CLC();
		break;
	case 0x19:                            // ORA ABSY
		addr = popWord() + regY;
		regA |= memReadByte( addr );
		ORA();
		break;	
	case 0x1a:                          // INC A
		regA = (regA + 1) & UMASK;
		setNVflagsForRegA();
		break;
	case 0x1c:                            // TRB ABS
		value = memReadByte( popWord() ); // Step 1: BIT operand (Only Z flag)
		if( value & regA ) regP &= 0xfd; else regP |= 0x02;
		currP = regP;					 // Step 2: PHP
		currA = regA;					 // Step 3: PHA		
		regA ^= 0xFF;					 // Step 4. EOR #$FF (Only Z flag)
		if( regA ) regP &= 0xfd; else regP |= 0x02;
		regA &= value;						//Step 5. AND operand (Only Z flag)
		if( regA ) regP &= 0xfd; else regP |= 0x02;
		memStoreByte( value, regA );		//Step 6. STA operand
		regA = currA;					//Step 7. PLA
		regP = currP;					//step 8. PLP
		break;
	case 0x1d:                            // ORA ABSX
		addr = popWord() + regX;
		regA |= memReadByte( addr );
		ORA();
		break;
	case 0x1e:                            // ASL ABSX
		addr = popWord() + regX;
		value = memReadByte( addr );
		setCarryFlagFromBit7(value);
		value = value << 1;
		memStoreByte( addr, value );
		ASL(value);
		break;
	case 0x20:                            // JSR ABS
		addr = popWord();
		currAddr = regPC-1;
		stackPush( ((currAddr >> 8) & UMASK) );
		stackPush( (currAddr & UMASK) );
		regPC = addr;
		break;
	case 0x21:                            // AND INDX
		zp = (popByte() + regX)&UMASK;
		addr = memGetWord(zp);
		value = memReadByte( addr );
		regA &= value;
		AND();
		break;
	case 0x24:                            // BIT ZP
		zp = popByte();
		value = memReadByte( zp );
		BIT(value);
		break;
	case 0x25:                            // AND ZP
		zp = popByte();
		regA &= memReadByte( zp );
		AND();
		break;
	case 0x26:                            // ROL ZP
		sf = carrySet();
		addr = popByte();
		value = memReadByte( addr ); 
		setCarryFlagFromBit7(value);
		value = value << 1;
		value += sf;
		memStoreByte( addr, value );
		ROL(value);
		break;
	case 0x28:                            // PLP
		regP = stackPop() | 0x30;		// There is no B bit!
		break;
	case 0x29:                            // AND IMM
		regA &= popByte();
		AND();
		break;
	case 0x2a:                            // ROL A
		sf = carrySet();
		setCarryFlagFromBit7(regA);
        regA = (regA << 1) & UMASK;
		regA += sf;
		ROL(regA);
		break;
	case 0x2c:                            // BIT ABS
		value = memReadByte( popWord() );
		BIT(value);
		break;
	case 0x2d:                            // AND ABS
		value = memReadByte( popWord() );
		regA &= value;
		AND();
		break;
	case 0x2e:                            // ROL ABS
		sf = carrySet();
		addr = popWord();
		value = memReadByte( addr );
		setCarryFlagFromBit7(value);
		value = value << 1;
		value += sf;
		memStoreByte( addr, value );
		ROL(value);
		break;
	case 0x30:                            // BMI
		offset = popByte();
		if (negativeSet()) { jumpBranch(offset); }
		break;
	case 0x31:                            // AND INDY
		zp = popByte();
		value = memGetWord(zp) + regY;
		regA &= memReadByte(value);
		AND();
		break;
	case 0x32:                            // AND (ZP)
		zp = popByte();
		value = memGetWord(zp);
		regA &= memReadByte(value);
		AND();
		break;
	case 0x34:                            // BIT ZP,X
		addr = (popByte() + regX) & UMASK;
		value = memReadByte( addr );
		BIT(value);
		break;		
	case 0x35:                            // AND INDX
		addr = (popByte() + regX) & UMASK;
		regA &= memReadByte(addr);
		AND();
		break;
	case 0x36:                            // ROL ZPX
		sf = carrySet();
		addr = (popByte() + regX) & UMASK;
		value = memReadByte( addr );
		setCarryFlagFromBit7(value);
		value = value << 1;
		value += sf;
		memStoreByte( addr, value );
		ROL(value);
		break;
	case 0x38:                            // SEC
		SEC();
		break;
	case 0x39:                            // AND ABSY
		addr = popWord() + regY;
		value = memReadByte( addr );
		regA &= value;
		AND();
		break;
	case 0x3a:                            // DEC A
		regA = (regA-1) & UMASK;
		setNVflagsForRegA();
		break;
	case 0x3c:                            // BIT ABS,X
		addr = popWord() + regX;
		value = memReadByte(addr);
		BIT(value);
		break;
	case 0x3d:                            // AND ABSX
		addr = popWord() + regX;
		value = memReadByte( addr );
		regA &= value;
		AND();
		break;
	case 0x3e:                            // ROL ABSX
		sf = carrySet();
		addr = popWord() + regX;
		value = memReadByte( addr );
		setCarryFlagFromBit7(value);
		value = value << 1;
		value += sf;
		memStoreByte( addr, value );
		ROL(value);
		break;
	case 0x40:                            // RTI
		regP = stackPop() | X_FLAG;		// There is no B bit!
        value = stackPop();
        value |= stackPop() << 8;
		regPC = value;
		break;
	case 0x41:                            // EOR INDX
		zp = (popByte() + regX)&UMASK;
		value = memGetWord(zp);
		regA ^= memReadByte(value);
		EOR();
		break;
	case 0x45:                            // EOR ZPX
		addr = (popByte() + regX) & UMASK;
		value = memReadByte( addr );
		regA ^= value;
		EOR();
		break;
	case 0x46:                            // LSR ZP
		addr = popByte() & UMASK;
		value = memReadByte( addr );
		setCarryFlagFromBit0(value);
		value = value >> 1;
		memStoreByte( addr, value );
		LSR(value);
		break;
	case 0x48:                            // PHA
		stackPush( regA );
		break;
	case 0x49:                            // EOR IMM
		regA ^= popByte();
		EOR();
		break;
	case 0x4a:                            // LSR
		setCarryFlagFromBit0(regA);
		regA = regA >> 1;
		LSR(regA);
		break;
	case 0x4c:                            // JMP abs
		regPC = popWord();
		break;
	case 0x4d:                            // EOR abs
		addr = popWord();
		value = memReadByte( addr );
		regA ^= value;
		EOR();
		break;
	case 0x4e:                           // LSR abs
		addr = popWord();
		value = memReadByte( addr );
		setCarryFlagFromBit0(value);
		value = value >> 1;
		memStoreByte( addr, value );
		LSR(value);
		break;
	case 0x50:                           // BVC (on overflow clear)
		offset = popByte();
		if (!overflowSet()) { jumpBranch(offset); }
		break;
	case 0x51:                           // EOR INDY
		zp = popByte();
		value = memGetWord(zp) + regY;
		regA ^= memReadByte(value);
		EOR();
		break;
	case 0x52:                           // EOR (ZP)
		zp = popByte();
		value = memGetWord(zp);
		regA ^= memReadByte(value);
		EOR();
		break;
	case 0x55:                           // EOR ZPX
		addr = (popByte() + regX) & UMASK;
		regA ^= memReadByte( addr );
		EOR();
		break;
	case 0x56:                           // LSR ZPX
		addr = (popByte() + regX) & UMASK;
		value = memReadByte( addr );
		setCarryFlagFromBit0(value);
		value = value >> 1;
		memStoreByte( addr, value );
		LSR(value);
		break;
	case 0x58:                           // CLI
		regP &= ~I_FLAG;
		break;
	case 0x59:                           // EOR ABSY
		addr = popWord() + regY;
		value = memReadByte( addr );
		regA ^= value;
		EOR();
		break;
	case 0x5a:                            // PHY
		stackPush( regY );
		break;
	case 0x5d:                           // EOR ABSX
		addr = popWord() + regX;
		value = memReadByte( addr );
		regA ^= value;
		EOR();
		break;
	case 0x5e:                           // LSR ABSX
		addr = popWord() + regX;
		value = memReadByte( addr );
		setCarryFlagFromBit0(value);
		value = value >> 1;
		memStoreByte( addr, value );
		LSR(value);
		break;
	case 0x60:                           // RTS
        value = stackPop();
        value |= (stackPop() << 8);
		regPC = value + 1;
		break;
	case 0x61:                           // ADC INDX
		zp = (popByte() + regX)&UMASK;
		addr = memGetWord(zp);
		value = memReadByte( addr );
		testADC( value );
		break;
	case 0x64:                           // STZ ZP
		memStoreByte( popByte(), 0 );
		break;
	case 0x65:                           // ADC ZP
		addr = popByte();
		value = memReadByte( addr );
		testADC( value );
		break;
	case 0x66:                           // ROR ZP
		sf = carrySet();
		addr = popByte();
		value = memReadByte( addr );
		setCarryFlagFromBit0(value);
		value = value >> 1;
		if( sf ) {value |= N_FLAG;}
		memStoreByte( addr, value );
		ROR(value);
		break;
	case 0x68:                           // PLA
		regA = stackPop();
		setNVflagsForRegA();
		break;
	case 0x69:                           // ADC IMM
		value = popByte();
		testADC( value );
		break;
	case 0x6a:                           // ROR A
		sf = regP&1;
		setCarryFlagFromBit0(regA);
		regA = regA >> 1;
		if( sf ) regA |= N_FLAG;
		ROR(regA);
		break;
	case 0x6c: 							// JMP INDIR
		regPC = memGetWord(popWord());
		break;
	case 0x6d:                           // ADC ABS
		addr = popWord();
		value = memReadByte( addr );
		testADC( value );
		break;
	case 0x6e:                           // ROR ABS
		sf = carrySet();
		addr = popWord();
		value = memReadByte( addr );
		setCarryFlagFromBit0(value);
		value = value >> 1;
		if( sf ) value |= N_FLAG;
		memStoreByte( addr, value );
		ROR(value);
		break;
	case 0x70:                           // BVS (branch on overflow set)
		offset = popByte();
		if (overflowSet()) { jumpBranch(offset); }
		break;
	case 0x71:                           // ADC INY
		zp = popByte();
		addr = memGetWord(zp);
		value = memReadByte( addr + regY );
		testADC( value );
		break;
	case 0x72:                           // ADC (ZP)
		zp = popByte();
		addr = memGetWord(zp);
		value = memReadByte(addr);
		testADC( value );
		break;
	case 0x74:                           // STZ ZPX
		memStoreByte( popByte() + regX, 0 );
		break;		
	case 0x75:                           // ADC ZPX
		addr = (popByte() + regX) & UMASK;
		value = memReadByte( addr );
		testADC( value );
		break;
	case 0x76:                           // ROR ZPX
		sf = carrySet();
		addr = (popByte() + regX) & UMASK;
		value = memReadByte( addr );
		setCarryFlagFromBit0(value);
		value = value >> 1;
		if( sf ) value |= N_FLAG;
		memStoreByte( addr, value );
		ROR(value);
		break;
	case 0x78:                           // SEI
		regP |= I_FLAG;
		break;
	case 0x79:                           // ADC ABSY
		addr = popWord();
		value = memReadByte( addr + regY );
		testADC( value );
		break;
	case 0x7a:                           // PLY
		regY = stackPop();
		setNVflagsForRegY();
		break;

	case 0x7c:                      // JMP INDIR,X
		value = (popWord() + regX) & 0xFFFF;  /* fixes value > 0xFFFF */
		/* below fixes result when value = 0xFFFF */
		addr = memReadByte(value) + (memReadByte((value+1) & 0xFFFF)<<8); 
		regPC = addr;
		break;


	case 0x7d:                           // ADC ABSX
		addr = popWord();
		value = memReadByte( addr + regX );
		testADC( value );
		break;
	case 0x7e:                           // ROR ABSX
		sf = carrySet();
		addr = popWord() + regX;
		value = memReadByte( addr );
		setCarryFlagFromBit0(value);
		value = value >> 1;
		if( sf ) value |= N_FLAG;
		memStoreByte( addr, value );
		ROR(value);
		break;
	case 0x80:                          // BRA
		offset = popByte();
		jumpBranch( offset );
		break;			
	case 0x81:                           // STA INDX
		zp = (popByte()+regX)&UMASK;
		addr = memGetWord(zp);
		memStoreByte( addr, regA );
		break;
	case 0x84:                           // STY ZP
		memStoreByte( popByte(), regY );
		break;
	case 0x85:                           // STA ZP
		memStoreByte( popByte(), regA );
		break;
	case 0x86:                           // STX ZP
		memStoreByte( popByte(), regX );
		break;
	case 0x88:                           // DEY (1 byte)
		regY = (regY-1) & UMASK;
		setNVflagsForRegY();
		break;
	case 0x89:                          // BIT IMM
		value = popByte();
		BIT(value);
		break;	
	case 0x8a:                           // TXA (1 byte);
		regA = regX & UMASK;
		setNVflagsForRegA();
		break;
	case 0x8c:                           // STY abs
		memStoreByte( popWord(), regY );
		break;
	case 0x8d:                           // STA ABS (3 bytes)
		memStoreByte( popWord(), regA );
		break;
	case 0x8e:                           // STX abs
		memStoreByte( popWord(), regX );
		break;
	case 0x90:                           // BCC (branch on carry clear)
		offset = popByte();
		if (!carrySet()) { jumpBranch(offset); }
		break;
	case 0x91:                           // STA INDY
		zp = popByte();
		addr = addr = memGetWord(zp) + regY;
		memStoreByte( addr, regA );
		break;
	case 0x92:                           // STA (ZP)
		zp = popByte();
		addr = memReadByte(zp) + (memReadByte(zp+1)<<8);
		memStoreByte( addr, regA );
		break;
	case 0x94:                           // STY ZPX
		memStoreByte( popByte() + regX & UMASK, regY );
		break;
	case 0x95:                           // STA ZPX
		memStoreByte( popByte() + regX & UMASK, regA );
		break;
	case 0x96:                           // STX ZPY
		memStoreByte( popByte() + regY & UMASK, regX );
		break;
	case 0x98:                           // TYA
		regA = regY & UMASK;
		setNVflagsForRegA();
		break;
	case 0x99:                           // STA ABSY
		memStoreByte( popWord() + regY, regA );
		break;
	case 0x9a:                           // TXS
		regSP = regX & UMASK;
		break;
	case 0x9c:                           // STZ ABS (3 bytes)
		memStoreByte( popWord(), 0 );
		break;
	case 0x9d:                           // STA ABSX
		addr = popWord();
		memStoreByte( addr + regX, regA );
		break;
	case 0x9e:                           // STZ ABSX
		addr = popWord();
		memStoreByte( addr + regX, 0 );
		break;
	case 0xa0:                           // LDY IMM
		regY = popByte();
		LDY();
		break;
	case 0xa1:                           // LDA INDX
		zp = (popByte()+regX)&UMASK;
		addr = memGetWord(zp);
		regA = memReadByte( addr );
		LDA();
		break;
	case 0xa2:                           // LDX IMM
		regX = popByte();
		LDX();
		break;
	case 0xa4:                           // LDY ZP
		regY = memReadByte( popByte() );
		LDY();
		break;
	case 0xa5:                           // LDA ZP
		regA = memReadByte( popByte() );
		LDA();
		break;
	case 0xa6:                          // LDX ZP
		regX = memReadByte( popByte() );
		LDX();
		break;
	case 0xa8:                          // TAY
		regY = regA & UMASK;
		setNVflagsForRegY();
		break;
	case 0xa9:                          // LDA IMM
		regA = popByte();
		LDA();
		break;
	case 0xaa:                          // TAX
		regX = regA & UMASK;
		setNVflagsForRegX();
		break;
	case 0xac:                          // LDY ABS
		regY = memReadByte( popWord() );
		LDY();
		break;
	case 0xad:                          // LDA ABS
		regA = memReadByte( popWord() );
		LDA();
		break;
	case 0xae:                          // LDX ABS
		regX = memReadByte( popWord() );
		LDX();
        break;
	case 0xb0:                          // BCS
		offset = popByte();
		if (carrySet()) { jumpBranch(offset); }
		break;
	case 0xb1:                          // LDA INDY
		zp = popByte();
		addr = memReadByte(zp) + (memReadByte(zp+1)<<8) + regY;
		regA = memReadByte( addr );
		if( regA ) regP &= 0xfd; else regP |= 0x02;
		if( regA & 0x80 ) regP |= 0x80; else regP &= 0x7f;
		break; 
	case 0xb2:                          // LDA (ZP)
		zp = popByte();
		addr = memGetWord(zp) + regY;
		regA = memReadByte( addr );
		LDA();
		break; 
	case 0xb4:                          // LDY ZPX
		regY = memReadByte ((popByte() + regX )& UMASK );
		LDY();
		break;
	case 0xb5:                          // LDA ZPX
		regA = memReadByte((popByte() + regX) & UMASK );
		LDA();
		break;
	case 0xb6:                          // LDX ZPY
		regX = memReadByte(( popByte() + regY )& UMASK );
		LDX();
		break;
	case 0xb8:                          // CLV
		CLV();
		break;
	case 0xb9:                          // LDA ABSY
		addr = popWord() + regY;
		regA = memReadByte( addr );
		LDA();
		break;
	case 0xba:                          // TSX
		regX = regSP & UMASK;
		LDX();
		break;
	case 0xbc:                          // LDY ABSX
		addr = popWord() + regX;
		regY = memReadByte( addr );
		LDY();
		break;
	case 0xbd:                          // LDA ABSX
		addr = popWord() + regX;
		regA = memReadByte( addr );
		LDA();
		break;
	case 0xbe:                          // LDX ABSY
		addr = popWord() + regY;
		regX = memReadByte( addr );
		LDX();
		break;
	case 0xc0:                          // CPY IMM
		value = popByte();
		doCompare(regY, value);
		break;
	case 0xc1:                          // CMP INDY
		zp = (popByte() + regX) & UMASK;
		addr = memGetWord(zp);
		value = memReadByte( addr );
		doCompare( regA, value );
		break;
	case 0xc4:                          // CPY ZP
		value = memReadByte( popByte() );
		doCompare( regY, value );
		break;
	case 0xc5:                          // CMP ZP
		value = memReadByte( popByte() );
		doCompare( regA, value );
		break;
	case 0xc6:                          // DEC ZP
		zp = popByte();
		dec(zp);
		break;
	case 0xc8:                          // INY
		regY = (regY + 1) & UMASK;
		setNVflagsForRegY();
		break;
	case 0xc9:                          // CMP IMM
		value = popByte();
		doCompare( regA, value );
		break;
	case 0xca:                          // DEX
		regX = (regX-1) & 0xff;
		setNVflagsForRegX();
		break;
	case 0xcc:                          // CPY ABS
		value = memReadByte( popWord() );
		doCompare( regY, value );
		break;
	case 0xcd:                          // CMP ABS
		value = memReadByte( popWord() );
		doCompare( regA, value );
		break;
	case 0xce:                          // DEC ABS
		addr = popWord();
		dec(addr);
		break;
	case 0xd0:                          // BNE
		offset = popByte();
		if (!zeroSet()) { jumpBranch(offset); }
		break;
	case 0xd1:                          // CMP INDY
		zp = popByte();
		addr = memGetWord(zp) + regY;
		value = memReadByte( addr );
		doCompare( regA, value );
		break;
	case 0xd2:                          // CMP (ZP)
		zp = popByte();
		addr = memGetWord(zp);
		value = memReadByte( addr );
		doCompare( regA, value );
		break;		
	case 0xd5:                          // CMP ZPX
		value = memReadByte(( popByte() + regX ) & UMASK);
		doCompare( regA, value );
		break;
	case 0xd6:                          // DEC ZPX
		addr = (popByte() + regX)  & UMASK;
		dec(addr);
		break;
	case 0xd8:                          // CLD (CLear Decimal)
		regP &= ~D_FLAG;
		break;
	case 0xd9:                          // CMP ABSY
		addr = popWord() + regY;
		value = memReadByte( addr );
		doCompare( regA, value );
		break;
	case 0xda:                            // PHX
		stackPush( regX );
		break;
	case 0xdd:                          // CMP ABSX
		addr = popWord() + regX;
		value = memReadByte( addr );
		doCompare( regA, value );
		break;
	case 0xde:                          // DEC ABSX
		addr = popWord() + regX;
		dec(addr);
		break;
	case 0xe0:                          // CPX IMM
		value = popByte();
		doCompare( regX, value );
		break;
	case 0xe1:                          // SBC INDX
		zp = (popByte()+regX)&UMASK;
		addr = memGetWord(zp);
		value = memReadByte( addr );
		testSBC( value );
		break;
	case 0xe4:                          // CPX ZP
		value = memReadByte( popByte() );
		doCompare( regX, value );
		break;
	case 0xe5:                          // SBC ZP
		addr = popByte();
		value = memReadByte( addr );
		testSBC( value );
		break;
	case 0xe6:                          // INC ZP
		zp = popByte();
		INC( zp );
		break;
	case 0xe8:                          // INX
		regX = (regX + 1) & UMASK;
		setNVflagsForRegX();
		break;
	case 0xe9:                         // SBC IMM
		value = popByte();
		testSBC( value );
		break;
	case 0xea:                         // NOP
		break;
	case 0xec:                         // CPX ABS
		value = memReadByte( popWord() );
		doCompare( regX, value );
		break;
	case 0xed:                         // SBC ABS
		addr = popWord();
		value = memReadByte( addr );
		testSBC( value );
		break;
	case 0xee:                         // INC ABS
		addr = popWord();
		INC(addr);
		break;
	case 0xf0:                         // BEQ
		offset = popByte();
		if (zeroSet()) { jumpBranch(offset); }
		break;
	case 0xf1:                         // SBC INDY
		zp = popByte();
		addr = memGetWord(zp);
		value = memReadByte( addr + regY );
		testSBC( value );
		break;
	case 0xf2:                         // SBC (ZP)
		zp = popByte();
		addr = memGetWord(zp);
		value = memReadByte(addr);
		testSBC( value );
		break;
	case 0xf5:                         // SBC ZPX
		addr = (popByte() + regX)&UMASK;
		value = memReadByte( addr );
		testSBC( value );
		break;
	case 0xf6:                         // INC ZPX
		addr = (popByte() + regX) & UMASK;
		INC(addr);
		break;
	case 0xf8:                         // SED
		regP |= D_FLAG;
		break;
	case 0xf9:                          // SBC ABSY
		addr = popWord();
		value = memReadByte( addr + regY );
		testSBC( value );
		break;
	case 0xfa:                           // PLX
		regX = stackPop();
		setNVflagsForRegX();
		break;
	case 0xfd:                         // SBC ABSX
		addr = popWord();
		value = memReadByte( addr + regX );
		testSBC( value );
		break;
	case 0xfe: // INC ABSX
		addr = popWord() + regX;
		INC(addr);
		break;		
	default:      
		break;
	}
}
