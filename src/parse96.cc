
// $Id: parse96.cc,v 1.5 1993/06/22 03:39:13 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

// mcs-96 specific parsing


#include <as.h>
#include <symtab.h>
#include <token.h>
#include <string.h>
#include <object.h>
#include <expr.h>

extern Mnemonic get_token(void);
extern Boolean  no_opto;

static Expr g_ex, ex;
                  /*
		    to overcome "label in block with destructors"
		    problem. (remove the object with a destructor out
		    of the block (in garg(), and parse96())

		    as long as noone tries recursively eval'ing
		    expr's we ought be fine
		  */


void expect_comma(void){

	if(currtok.mnem != COMMA)
		error("I see no \",\" here");
	get_token();
}
			      
int garg(Byte opc, int max){
	// generic operand parsing for 1,2, or 2|3 operands
	// BUG NOTE: is used more than it should, allowing
	// such perversions as "pop #constant" to go thru
	// fix is RSN...
	
	long a, b, c, d;
	long cpc = obj.pc();
	int aa, size;
	Boolean shrtok=0, seen3=0;
	// Expr ex;
	
	switch(max){
	  case 2:	// reg, aop
	  case 3:	// reg, [reg,] aop
		g_ex.nofwd = 1;
		d = g_ex.parse("this ought be a register");
		if( d<0 || d>255)
			error("invalid register");
		expect_comma();
	  aop:
	  case 1:	// aop only
		switch(currtok.mnem){
		  case POUND:	// immediate #foo
			get_token();
			aa = 1;
			size = (opc & 0x10)?1:2;
			obj.emit((Byte)(opc + aa));
			g_ex.nofwd = 0;
			g_ex.pc = cpc + 1;
			g_ex.how = (size==1)?METH_ASIS_BYTE:METH_ASIS_WORD;
			c = g_ex.parse("bad immediate");
			break;
		  case LBRACK:	// indirect [R] or [R]+
			get_token();
			aa = 2;
			obj.emit((Byte)(opc + aa));
			g_ex.nofwd = 1;
			c = g_ex.parse("invalid indirection");
			if (c<0 || c>255 || c&1)
				// must be even register (to hold an address)
				error("invalid register for indirect adressing");
			if(currtok.mnem != RBRACK) error("I see no \"]\" here");
			get_token();
			if(currtok.mnem == PLUS){
				// auto incr. [R]+
				get_token();
				c++;
			}
			obj.emit((Byte) c);
			break;
		  default:
			// is either R or A[R]
			g_ex.nofwd = 0;
			g_ex.pc = cpc + 2; // assume this is A first
					   // A[R] ->
					   // <opc> <R> <A> <...>
			g_ex.how = METH_ASIS_WORD;
			a = g_ex.parse("bad address mode");
			shrtok = !g_ex.didfwd();
			// now what was it we just got?
			if(currtok.mnem == LBRACK){
				// indexed mode A[R]
				
				get_token();
				obj.pc(cpc);
				aa = 3;
				obj.emit((Byte)(opc + aa));
				g_ex.nofwd = 1;
				c = g_ex.parse("bad indexing");	// get R
				if(c<0 || c>255 || c&1)
					// must be even register
					error("invalid register for index mode addressing");
				
				// see if it can be persuaded to go short mode
				// the c++ was backwards...
				if(shrtok && a<127 && a>-128){
					// massage it
					obj.emit((Byte) c); 	// spit out R
					obj.inc(1);		// A is short mode, chop it
				}else{
					c++;
					obj.emit((Byte) c);     // spit out R
					obj.inc(2);		// A is already in place
				}
				if(currtok.mnem != RBRACK)
					error("I see no \"]\" here");
				get_token();
			}else{
				// it better be a register...
				aa = 00;
				if( g_ex.didfwd() || a<0 || a>255){
					error("bad addressing mode, invalid register specification");
				}else{
					if(currtok.mnem == COMMA && !seen3){
						expect_comma();
						// deal with 3rd arg
						if(max != 3)
							error("too many args. for instruction");
						seen3 = 1;
						b = a;		// the R
						opc |= 0x20;	// set 3 oper. in opc
						goto aop;
					}
					obj.pc(cpc);		// re-write
					obj.emit(opc);
					obj.emit((Byte) a);
				}
			}
			break;
		}	// eo sw(mnem)
		break;
	  default:
		bug("parse96:garg");
		break;
	}	// eo sw(max)
	if(seen3) obj.emit((Byte) b);
	if(max>1) obj.emit((Byte) d);
	return aa;	// so we can detect suchlikes as pop #foo
}


void parse96(void){
	Byte foo, bar, baz;
	long qux;
	long glark, tpc;

	// Expr ex;
	
	switch(currtok.mnem){

	  case ADD:  foo = 0x44; bar = 3; goto commn;
	  commn:
		get_token();
		garg(foo, bar);
		break;
	  commnni:
		get_token();
		glark = garg(foo, bar);
		if(glark==1)	// got a #foo
			error("immediate operand makes no sense here");
		break;
		
	  case ADDB: foo = 0x54; bar = 3; goto commn;
	  case SUB:  foo = 0x68; bar = 3; goto commn;
	  case SUBB: foo = 0x78; bar = 3; goto commn;
	  case ADDC: foo = 0xa4; bar = 2; goto commn;
	  case ADDCB:foo = 0xb4; bar = 2; goto commn;
	  case SUBC: foo = 0xa8; bar = 2; goto commn;
	  case SUBCB:foo = 0xb8; bar = 2; goto commn;
	  case CMP:  foo = 0x88; bar = 2; goto commn;
	  case CMPB: foo = 0x98; bar = 2; goto commn;
	  case AND:  foo = 0x40; bar = 3; goto commn;
	  case ANDB: foo = 0x50; bar = 3; goto commn;
	  case OR:   foo = 0x80; bar = 2; goto commn;
	  case ORB:  foo = 0x90; bar = 2; goto commn;
	  case XOR:  foo = 0x84; bar = 2; goto commn;
	  case XORB: foo = 0x94; bar = 2; goto commn;
	  case LD:   foo = 0xa0; bar = 2; goto commn;
	  case LDB:  foo = 0xb0; bar = 2; goto commn;
	  case ST:   foo = 0xc0; bar = 2; goto commnni;
	  case STB:  foo = 0xc4; bar = 2; goto commnni;
	  case XCH:  foo = 0x04; bar = 2; goto commnni;
	  case XCHB: foo = 0x14; bar = 2; goto commnni;
	  case LDBSE:foo = 0xbc; bar = 2; goto commn;
	  case LDBZE:foo = 0xac; bar = 2; goto commn;
	  case MUL:  obj.emit((Byte) 0xfe);     // fall thru
	  case MULU: foo = 0x4c; bar = 3; goto commn;
	  case MULB: obj.emit((Byte) 0xfe);    	// fall thru
	  case MULUB:foo = 0x5c; bar = 3; goto commn;
	  case DIV:  obj.emit((Byte) 0xfe); 	// fall thru
	  case DIVU: foo = 0x8c; bar = 3; goto commn;
	  case DIVB: obj.emit((Byte) 0xfe);    	// fall thru
	  case DIVUB:foo = 0x9c; bar = 3; goto commn;

	  case PUSH: foo = 0xc8; bar = 1; goto commn;
	  case POP:  foo = 0xcc; bar = 1; goto commnni;

	  case PUSHF: foo = 0xf2; goto that;
	  that:
		get_token();
		obj.emit((Byte) foo);
		break;
	  case POPF: foo = 0xf3; goto that;
	  case PUSHA:foo = 0xf4; goto that;
	  case POPA: foo = 0xf5; goto that;
	  case TRAP: foo = 0xf7; goto that;
	  case RET:  foo = 0xf0; goto that;
	  case CLRC: foo = 0xf8; goto that;
	  case SETC: foo = 0xf9; goto that;
	  case DI:   foo = 0xfa; goto that;
	  case EI:   foo = 0xfb; goto that;
	  case CLRVT:foo = 0xfc; goto that;
	  case NOP:  foo = 0xfd; goto that;
	  case RST:  foo = 0xff; goto that;
	  case IPLPD:foo = 0xf6; goto that;
	  case SKIP:
		get_token();
		obj.emit((Word)0x0000); // 0x00**
		break;

	  case JNST: foo = 0xd0; goto condjmp;
	  case JST:  foo = 0xd8; goto condjmp;
	  case JNH:  foo = 0xd1; goto condjmp;
	  case JH:   foo = 0xd9; goto condjmp;
	  case JGT:  foo = 0xd2; goto condjmp;
	  case JLE:  foo = 0xda; goto condjmp;
	  case JNC:  foo = 0xd3; goto condjmp;
	  case JC:   foo = 0xdb; goto condjmp;
	  case JNVT: foo = 0xd4; goto condjmp;
	  case JVT:  foo = 0xdc; goto condjmp;
	  case JNV:  foo = 0xd5; goto condjmp;
	  case JV:   foo = 0xdd; goto condjmp;
	  case JGE:  foo = 0xd6; goto condjmp;
	  case JLT:  foo = 0xde; goto condjmp;
	  case JNE:  foo = 0xd7; goto condjmp;
	  case JE:   foo = 0xdf; goto condjmp;
	  condjmp:
		get_token();
	  condjmpnt:
		tpc = obj.pc();
		if( !no_opto && currtok.mnem != NEAR){
			// be tricky, make sure we always generate correct code
			// assume forward jump,
			// do as: j!cond X; ljmp foo; X:
			
			ex.nofwd = 0;
			ex.pc = obj.pc() + ((foo<0x40)?4:3);
			ex.how = METH_ASIS_WORD | METH_RELATIVE;
			glark = ex.parse("bad jump target");
			qux = glark - tpc  - ((foo<0x40)?3:2);
			obj.pc(tpc);
			if( ex.didfwd() || qux<-128 ||qux>127 ){
				// if fwd or far away
				obj.emit((Byte) (foo ^ 8));
				if(foo<0x40) obj.emit((Byte)bar);
				obj.emit((Byte) 0x3);			// j!cond past ljmp
				obj.emit((Byte) 0xe7);			// ljmp
				obj.inc(2);				// address already there
			}else{
				// make it short
				obj.emit((Byte)foo);
				if(foo<0x40) obj.emit((Byte)bar);
				obj.emit((Byte)qux);
			}
		}else{
			// do it the normal way
			if(currtok.mnem == NEAR) get_token();
			obj.emit((Byte)foo);
			if(foo<0x40) obj.emit((Byte)bar);
			ex.nofwd = 0;
			ex.pc = obj.pc();
			ex.how = METH_ASIS_BYTE | METH_RELATIVE;	// relative addressing
			qux = ex.parse("bad jump target");
			if (!ex.didfwd()){
				// check range now
				qux -= obj.pc();
				if ( qux > 127 || qux < -128 )
					error("jump target too far");
			}

		}
		break;

	  case JBS: foo = 0x38; goto ojbc;
	  case JBC: foo = 0x30;
	  ojbc:
		get_token();
		ex.nofwd = 1;
		qux = ex.parse("register required");
		if (qux <0 || qux>255)
			error("invalid register");
		bar = (Byte)qux;
		expect_comma();
		qux = ex.parse("bitno. required");
		if(qux > 7 || qux <0)
			error("bitno. out of range");
		baz = (Byte)qux;
		expect_comma();
		foo |= baz;
		goto condjmpnt;

	  case DJNZ:  foo = 0xe0; goto odjnz;
	  case DJNZW: foo = 0xe1;
	  odjnz:
		get_token();
		obj.emit(foo);
		ex.nofwd = 1;
		qux = ex.parse("register required");
		if (qux <0 || qux>255)
			error("invalid register");
		foo = (Byte)qux;
		obj.emit(foo);
		expect_comma();
		ex.nofwd = 0;
		ex.pc = obj.pc();
		ex.how = METH_ASIS_BYTE | METH_RELATIVE;	// relative addressing
		qux = ex.parse("bad jump target");
		if (!ex.didfwd()){
				// check range now (approx)
			qux -= obj.pc();
			if ( qux > 127 || qux < -128 )
				error("jump target too far");
		}
		break;
		
	  case SHL:   foo = 0x09; goto shift;
	  case SHLB:  foo = 0x19; goto shift;
	  case SHLL:  foo = 0x0d; goto shift;
	  case SHR:   foo = 0x08; goto shift;
	  case SHRA:  foo = 0x0a; goto shift;
	  case SHRAB: foo = 0x1a; goto shift;
	  case SHRAL: foo = 0x0e; goto shift;
	  case SHRB:  foo = 0x18; goto shift;
	  case SHRL:  foo = 0x0c; goto shift;
	  shift:
		get_token();
		obj.emit(foo);
		ex.nofwd = 1;
	        qux = ex.parse("invalid shift");
		if (qux <0 || qux>255)
			error("invalid register");
		bar = (Byte)qux;
		expect_comma();
		if(currtok.mnem == POUND){
			get_token();
			qux = ex.parse("constant required");
			if(qux<0 || qux > 15)
				error("shift constant out of range");
		}else{
			qux = ex.parse("register required");
			if (qux<15 || qux>255 )
				error("invalid register");
		}
		baz = (Byte)qux;
		obj.emit(baz);
		obj.emit(bar);
		break;

		// one register only
	  case NEG:  foo = 0x03; goto ronly;
	  case NEGB: foo = 0x13; goto ronly;
	  case NOT:  foo = 0x02; goto ronly;
	  case NOTB: foo = 0x12; goto ronly;
	  case CLR:  foo = 0x01; goto ronly;
	  case CLRB: foo = 0x11; goto ronly;
	  case DEC:  foo = 0x05; goto ronly;
	  case DECB: foo = 0x15; goto ronly;
	  case EXT:  foo = 0x06; goto ronly;
	  case EXTB: foo = 0x16; goto ronly;
	  case INC:  foo = 0x07; goto ronly;
	  case INCB: foo = 0x17; goto ronly;
	  ronly:	// 1 reg
		get_token();
		obj.emit(foo);
		ex.nofwd = 1;
		qux = ex.parse("register required");
		if (qux <0 || qux>255)
			error("invalid register");
		foo  = (Byte)qux;
		obj.emit(foo);
		break;

		// two register only
	  case CMPL:  foo = 0xc5; goto twreg;
	  case BMOV:  foo = 0xc1; goto twreg;
	  case BMOVI: foo = 0xcd; goto twreg;
	  case NORML: foo = 0x0f; goto twreg;
	  twreg:	// 2 reg
		get_token();
		obj.emit((Byte)foo);
		ex.nofwd = 1;
		qux = ex.parse("register required");
		bar = (Byte)qux;
		if (qux <0 || qux>255)
			error("invalid register");
		expect_comma();
		qux = ex.parse("register required");
		baz = (Byte)qux;
		if (qux <0 || qux>255)
			error("invalid register");
		obj.emit(baz);
		obj.emit(bar);
		break;
		
	  case BR:
		get_token();
		obj.emit((Byte)0xe3);
		if(currtok.mnem != LBRACK) error("I see no \"[\" here");
		get_token();
		ex.nofwd = 1;
		qux = ex.parse("register required");
		foo = (Byte)qux;
		if (qux <0 || qux>255)
			error("invalid register");
		obj.emit(foo);
		if(currtok.mnem != RBRACK) error("I see no \"]\" here");
		get_token();
		break;

	  case SJMP:  ex.how = METH_MUNG_SJMP  | METH_RELATIVE; goto cjmp;
	  case SCALL: ex.how = METH_MUNG_SCALL | METH_RELATIVE; goto cjmp;
	  cjmp:
		get_token();
		ex.nofwd = 0;
		ex.pc = obj.pc();
		qux = ex.parse("invalid destination");
		qux -= obj.pc();
		if ( !ex.didfwd() && ( qux < -1024 || qux >1023) )
			error("target address out of range");
		break;

	  case LJMP:  foo = 0xe7; goto djmp;
	  case LCALL: foo = 0xef; goto djmp;
	  djmp:
		get_token();
		obj.emit(foo);
		ex.nofwd = 0;
		ex.pc = obj.pc();
		ex.how = METH_ASIS_WORD | METH_RELATIVE;
		ex.parse("invalid destination");
		break;


	  case JMP:  foo = 0xe7; goto opjmp;
	  case CALL: foo = 0xef; goto opjmp;
	  opjmp:
		tpc = obj.pc();
		// figure best form (short or long?)
		get_token();
		obj.emit(foo);
		ex.nofwd = 0;
		ex.pc = obj.pc();
		ex.how = METH_ASIS_WORD | METH_RELATIVE;
		qux = ex.parse("invalid destination");
		qux -= obj.pc();
		if ( !ex.didfwd() && qux > -1024 && qux <1023 ){
			// munge to short form
			qux += obj.pc();
			obj.pc(tpc);
			obj.install(qux, tpc,
				    (Method)(((foo==0xe7)
				    ?METH_MUNG_SJMP
				    :METH_MUNG_SCALL
				     )|METH_RELATIVE)
				    );
		}
		break;
		
	  case TIJMP:
		// a blemish
		// insufficienly documented in data book
		// ergo, obj.emitted code likely incorrect ...
		warn("TIJMP code may be incorrect");
		get_token();
		obj.emit((Byte)0xe2);
		ex.nofwd = 1;
		foo = (Byte)ex.parse("register required");
		bar = (Byte)ex.parse("byte required");
		baz = (Byte)ex.parse("register required");
		obj.emit(baz);
		obj.emit(bar);
		obj.emit(foo);
		break;


	  default:
		error("unrecognized syntax");
		get_token();
		break;
	} // eo sw
}


































	
