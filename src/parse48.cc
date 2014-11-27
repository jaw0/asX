

// $Id: parse48.cc,v 1.5 1993/08/02 23:59:25 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

// mcs-48 specific parsing


#include <as.h>
#include <symtab.h>
#include <token.h>
#include <string.h>
#include <object.h>
#include <expr.h>

extern Mnemonic get_token(void);
void expect_comma(void){

	if(currtok.mnem != COMMA)
		error("I see no \",\" here");
	get_token();
}

void expect_at(void){

	if(currtok.mnem != ATSIGN)
		error("I see no \"@\" here");
	get_token();
}

void expect_a(void){

	if(currtok.mnem != AAA)
		error("I see no \"A\" here");
	get_token();
}

void expect_at_a(void){
	
	expect_at();
	expect_a();
}

void expect_a_at_a(void){

	expect_a();
	expect_comma();
	expect_at_a();
}

int whreg(void){
	int r;
	
	switch(currtok.mnem){
	  case R0: r = 0; break;
	  case R1: r = 1; break;
	  case R2: r = 2; break;
	  case R3: r = 3; break;
	  case R4: r = 4; break;
	  case R5: r = 5; break;
	  case R6: r = 6; break;
	  case R7: r = 7; break;
	  case ATSIGN:
		get_token();
		switch(currtok.mnem){
		  case R0: r = 8; break;
		  case R1: r = 9; break;
		  default:
			error("invalid register for indexing");
			break;
		}
		break;
	  case POUND:
		r = -1;
		break;
	  default:
		error("invalid register specification");
		break;
	}
	get_token();
	return r;
}
int garg(Byte opc=0);
int garg(Byte opc){
	int r=0;
	Expr ex;

	r = whreg();
	if(r==-1){
		obj.emit((Byte)(opc+3));
		ex.nofwd = 0;
		ex.pc = obj.pc();
		ex.how = METH_ASIS_BYTE;
		ex.parse();
		return -1;
	}
	if(opc){
		opc += (r>7)?(r-8):(r+8);
		obj.emit( opc );
	}
	return r;
}
			
int agarg(Byte opc=0);
int agarg(Byte opc){
	expect_a();
	expect_comma();
	return garg(opc);
}

Expr expr;			// see parse96 for explanation

void parse48(void){
	// Expr expr;
	Byte foo;
	int n;
	
	switch(currtok.mnem){

	  case JMP:
		expr.how = METH_MUNG_JMP;
		goto jcall;
	  case CALL:
		expr.how = METH_MUNG_CALL;
	  jcall:
		get_token();
		expr.pc = obj.pc();
		expr.nofwd = 0;
		expr.parse("illegal target for JMP/CALL");
		break;

	  case RETR: foo = 0x93; goto doone;
	  case NOP:  foo = 0x00; goto doone;
	  case RET:  foo = 0x83;
	  doone:
		get_token();
		obj.emit( (Byte) foo );
		break;

	  case JC:
		foo = 0xF6;
	  condjmp:
		get_token();
		obj.emit( (Byte) foo );
		expr.pc = obj.pc();
		expr.how = METH_ASIS_BYTE;
		expr.nofwd = 0;
		expr.parse("illegal target for conditional jump");
		break;
	  case JNC: foo = 0xE6; goto condjmp;
	  case JB0: foo = 0x12; goto condjmp;
	  case JB1: foo = 0x32; goto condjmp;
	  case JB2: foo = 0x52; goto condjmp;
	  case JB3: foo = 0x72; goto condjmp;
	  case JB4: foo = 0x92; goto condjmp;
	  case JB5: foo = 0xB2; goto condjmp;
	  case JB6: foo = 0xD2; goto condjmp;
	  case JB7: foo = 0xF2; goto condjmp;
	  case JF0: foo = 0xb6; goto condjmp;
	  case JF1: foo = 0x76; goto condjmp;
	  case JNI: foo = 0x86; goto condjmp;
	  case JNT0:foo = 0x26; goto condjmp;
	  case JNT1:foo = 0x46; goto condjmp;
	  case JNZ: foo = 0x96; goto condjmp;
	  case JTF: foo = 0x16; goto condjmp;
	  case JT0: foo = 0x36; goto condjmp;
	  case JT1: foo = 0x56; goto condjmp;
	  case JZ:  foo = 0xc6; goto condjmp;
	  case JMPP:
		get_token();
		expect_at_a();
		obj.emit( (Byte)0xb3);
		break;
	  case DAA:
		foo = 0x57;
	  naonly:
		obj.emit( (Byte) foo);
		get_token();
		if(currtok.mnem != AAA )
			error("illegal arguement, (must be A)");
		get_token();
		break;
	  case SWAP: foo = 0x47; goto naonly;
	  case RLA:  foo = 0xe7; goto naonly;
	  case RLCA: foo = 0xf7; goto naonly;
	  case RRA:  foo = 0x77; goto naonly;
	  case RRCA: foo = 0x67; goto naonly;
	  case ENABLE:
		get_token();
		switch(currtok.mnem){
		  case III:
			foo = 0x05;
			break;
		  case TCNTI:
			foo = 0x25;
			break;
		  default:
			error("what's this you're wanting to enable?");
			break;
		}
		obj.emit((Byte)foo);
		get_token();
		break;
	  case DIS:
		get_token();
		switch(currtok.mnem){
		  case III:
			foo = 0x15;
			break;
		  case TCNTI:
			foo = 0x35;
			break;
		  default:
			error("what's this you're wanting to disable?");
			break;
		}
		obj.emit((Byte)foo);
		get_token();
		break;
	  case ENT0:
		get_token();
		if(currtok.mnem != CLK)
			error("only CLK can be enabled on T0");
		obj.emit((Byte)0x75);
		get_token();
		break;
	  case STRT:
		get_token();
		switch (currtok.mnem){
		  case TTT:
			foo = 0x55;
			break;
		  case CNT:
			foo = 0x45;
			break;
		  default:
			error("can only START T or CNT");
		}
		obj.emit((Byte)foo);
		get_token();
		break;
	  case STOP:
		get_token();
		if(currtok.mnem != TCNT)
			error("what is this you're trying to STOP (try TCNT)");
		obj.emit((Byte)0x65);
		get_token();
		break;
	  case SEL:
		get_token();
		switch(currtok.mnem){
		  case RB0:
			foo = 0xc5;
			break;
		  case RB1:
			foo = 0xd5;
			break;
		  case MB0:
			foo = 0xe5;
			break;
		  case MB1:
			foo = 0xf5;
			break;
		  default:
			error("what is it you're trying to select");
		}
		obj.emit((Byte)foo);
		get_token();
		break;
	  case MOVP:
		foo = 0xa3;
	  movpx:
		get_token();
		expect_a_at_a();
		obj.emit((Byte)foo);
		break;
	  case MOVP3:
		foo = 0xe3;
		goto movpx;
	  case XCH:
		get_token();
		n = agarg(0x20);
		if (n==-1) error("cannot exchange a constant!");
		break;
	  case XCHD:
		get_token();
		n = agarg(0x30);
		if (n<8) error("invalid target for XCHD");
		break;
	  case ADD:
		get_token();
		agarg(0x60);
		break;
	  case ADDC:
		get_token();
		agarg(0x70);
		break;
	  case INC:
		get_token();
		switch (currtok.mnem){
		  case AAA:
			get_token();
			obj.emit((Byte) 0x17);
			break;
		  default:
			n = garg(0x10);
			if(n==-1) error("cannot increment a constant!");
			break;
		}
		break;
	  case DEC:
		get_token();
		switch (currtok.mnem){
		  case AAA:
			get_token();
			obj.emit((Byte) 0x07);
			break;
		  default:
			n = garg(0xc0);
			if(n==-1) error("cannot decrement a constant!");
			break;
		}
		break;

	  case XRL:
		get_token();
		agarg(0xd0);
		break;
	  case DJNZ:
		get_token();
		n = garg(0xe0);
		if (n==-1)error("cannot decrement a constant!");
		if (n>7)  error("non-standard usage, decrement indexed");
		break;
	  case CLR:
		get_token();
		switch(currtok.mnem){
		  case AAA: foo = 0x27; break;
		  case F0:  foo = 0x85; break;
		  case F1:  foo = 0xa5; break;
		  case CCC: foo = 0x97; break;
		  default:
			error("cannot CLR this thing which you speak of");
			break;
		}
		get_token();
		obj.emit(foo);
		break;
	  case CPL:
		get_token();
		switch(currtok.mnem){
		  case AAA: foo = 0x37; break;
		  case F0:  foo = 0x95; break;
		  case F1:  foo = 0xb5; break;
		  case CCC: foo = 0xa7; break;
		  default:
			error("cannot CPL this thing which you speak of");
			break;
		}
		get_token();
		obj.emit(foo);
		break;
	  case MOVX:
		get_token();
		switch(currtok.mnem){
		  case AAA:
			n = agarg(0x80);
			if(n<8) error("invalid target for MOVX");
			break;
		  case ATSIGN:
			get_token();
			switch(currtok.mnem){
			  case R0: foo = 0x90; break;
			  case R1: foo = 0x91; break;
			  default:
				error("invalid register for indexing");
				break;
			}
			get_token();
			obj.emit(foo);
			expect_comma();
			expect_a();
			break;
		  default:
			error("what's this you'd like to MOVX?");
			break;
		}
	  case MOVD:
		get_token();
		switch(currtok.mnem){
		  case AAA:
			get_token();
			expect_comma();
			switch(currtok.mnem){
			  case P4: foo = 0x0c; break;
			  case P5: foo = 0x0d; break;
			  case P6: foo = 0x0e; break;
			  case P7: foo = 0x0f; break;
			  default:
				error("cannot MOVD here");
				break;
			}
			get_token();
			obj.emit(foo);
			foo = 0;	// flag
			break;
		  case P4: foo = 0x3c; break;
		  case P5: foo = 0x3d; break;
		  case P6: foo = 0x3e; break;
		  case P7: foo = 0x3f; break;
		  default:
			error("cannot MOVD here");
			break;
		}
		if(!foo){	// Px
			get_token();
			expect_comma();
			expect_a();
			obj.emit(foo);
		}
		break;
	  case INA:
		get_token();
		expect_a();
		expect_comma();
		switch(currtok.mnem){
		  case BUS: foo = 0x08; break;	// non standard usage (use INS A, BUS)
		  case P1:  foo = 0x09; break;
		  case P2:  foo = 0x0a; break;
		  case P3:  foo = 0x0b; break; // non standard usage (there is no P3)
		  case P4:  foo = 0x0c; break; // non standard usage (use MOVD A, Pn)
		  case P5:  foo = 0x0d; break; // non standard usage
		  case P6:  foo = 0x0e; break; // non standard usage
		  case P7:  foo = 0x0f; break; // non standard usage
		  default:
			error("what is this you'd like to input?");
			break;
		}
		get_token();
		obj.emit(foo);
		break;
	  case INS:
		get_token();
		expect_a();
		expect_comma();
		switch(currtok.mnem){
		  case BUS:
			foo = 0x08;
			break;
		  default:
			error("I see no bus here");
			break;
		}
		get_token();
		obj.emit(foo);
		break;
	  case OUTL:
		get_token();
		switch(currtok.mnem){
		  case BUS: foo = 0x02; break;
		  case P1:  foo = 0x39; break;
		  case P2:  foo = 0x3a; break;
		  case P3:  foo = 0x3b; break; // non standard usage (there is no P3) 
		  case P4:  foo = 0x3c; break; // non standard usage (use MOVD Pn, A)
		  case P5:  foo = 0x3d; break; //"
		  case P6:  foo = 0x3e; break; //"
		  case P7:  foo = 0x3f; break; //"
		  default:
			error("what is it you're trying to OUTL");
			break;
		}
		get_token();
		expect_comma();
		expect_a();
		obj.emit(foo);
		break;
	  case ORLD:
		get_token();
		switch(currtok.mnem){
		  case P4: foo = 0x8c; break;
		  case P5: foo = 0x8d; break;
		  case P6: foo = 0x8e; break;
		  case P7: foo = 0x8f; break;
		  default:
			error("I see no port here with which to ORLD");
			break;
		}
		obj.emit(foo);
		get_token();
		expect_comma();
		expect_a();
		break;
	  case ANLD:
		get_token();
		switch(currtok.mnem){
		  case P4: foo = 0x9c; break;
		  case P5: foo = 0x9d; break;
		  case P6: foo = 0x9e; break;
		  case P7: foo = 0x9f; break;
		  default:
			error("I see no port here with which to ANLD");
			break;
		}
		obj.emit(foo);
		get_token();
		expect_comma();
		expect_a();
		break;
	  case ORL:
		foo = 0x40;
	  oranl:
		get_token();
		switch(currtok.mnem){
		  case AAA:
			agarg(foo);
			foo = 0;
			break;
		  case BUS: foo += 0x48; break;
		  case P1:  foo += 0x49; break;
		  case P2:  foo += 0x4a; break;
		  default:
			error("illegal perversion");
			break;
		}
		if(foo){
			get_token();
			expect_comma();
			if(currtok.mnem != POUND)
				error("invalid syntax for perversion");
			get_token();
			obj.emit(foo);
			expr.nofwd = 0;
			expr.how = METH_ASIS_BYTE;
			expr.pc = obj.pc();
			expr.parse("bad perversion");
		}
		break;
	  case ANL:
		foo = 0x50;
		goto oranl;

		// [ sound of trumpets in the background ]
		// [ a drum roll takes form ]
	  case MOV:
		// the biggee....
		get_token();
		switch(currtok.mnem){
		  case PSW:
			get_token();
			expect_comma();
			expect_a();
			obj.emit((Byte)0xd7);
			break;
		  case TTT:
			get_token();
			expect_comma();
			expect_a();
			obj.emit((Byte)0x62);
			break;
		  case AAA:
			get_token();
			expect_comma();
			switch(currtok.mnem){
			  case PSW:
				get_token();
				obj.emit((Byte)0xc7);
				break;
			  case TTT:
				get_token();
				obj.emit((Byte)0x42);
				break;
			  case POUND:
				get_token();
				obj.emit((Byte)0x23);
				expr.how = METH_ASIS_BYTE;
				expr.pc = obj.pc();
				expr.nofwd = 0;
				expr.parse();
				break;
			  case ATSIGN:
				get_token();
				switch(currtok.mnem){
				  case R0: n = 8; break;
				  case R1: n = 9; break;
				  default:
					error("invalid register for indexing");
					break;
				}
				goto areg;
			  case R0: n = 0; goto areg;
			  case R1: n = 1; goto areg;
			  case R2: n = 2; goto areg;
			  case R3: n = 3; goto areg;
			  case R4: n = 4; goto areg;
			  case R5: n = 5; goto areg;
			  case R6: n = 6; goto areg;
			  case R7: n = 7;
			  areg:
				get_token();
				obj.emit((Byte) (0xf0 + (n^8)));
				break;
			  default:
				error("unable to MOV desired object");
				break;
			}
			break;
		  case ATSIGN:
			get_token();
			switch(currtok.mnem){
			  case R0: n = 8; break;
			  case R1: n = 9; break;
			  default:
				error("invalid register for indexing");
				break;
			}
			goto mreg;
		  case R0: n = 0; goto mreg;
		  case R1: n = 1; goto mreg;
		  case R2: n = 2; goto mreg;
		  case R3: n = 3; goto mreg;
		  case R4: n = 4; goto mreg;
		  case R5: n = 5; goto mreg;
		  case R6: n = 6; goto mreg;
		  case R7: n = 7;
		  mreg:
			get_token();
			expect_comma();
			switch(currtok.mnem){
			  case AAA:
				get_token();
				obj.emit((Byte) (0xa0 + (n^0x8)));
				break;
			  case POUND:
				get_token();
				obj.emit((Byte) (0xb0 + (n^0x8)));
				expr.pc = obj.pc();
				expr.nofwd = 0;
				expr.how = METH_ASIS_BYTE;
				expr.parse("bad move");
				break;
			  default:
				error("unable to MOV desired object");
				break;
			}
			break;
		  default:
			error("unable to MOV desired object");
			break;
		}
		break;

	  default:
		get_token();
		error("Syntax Error");
		break;

		// All Done!
		
	}// eo sw
}


