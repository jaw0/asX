

// $Id: codegen.cc,v 1.9 1993/08/02 23:59:14 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#include <as.h>
#include <object.h>
#include <symtab.h>
#include <fstream.h>
#include <string.h>
#include <iomanip.h>

#define HEX(n)	setw(n)<<setfill('0')<<hex

void resolve(void){
	// resolve forward refs.
	
	RelocItem *ri;

	for(ri=relocd.head(); ri; ri=ri->next()){
		if( ri->typof() != SYM_OK){
			cerr << "unresolved reference to symbol \"";
			cerr << ri->namof();
			cerr << "\" (first occurance on line ";
			cerr << ri->where();
			cerr << " of file \"" << ri->file() << "\"";
			cerr << ")" << endl;
		}
			
		obj.combine( ri->valof(), ri->pc(), ri->how() );
	}
}



void Object::install(long value, long npc, Method how){
	// insert value into object file
	
	long opc = pc();	// save pc
	long foo;

	pc(npc);
	if( how & METH_MINUS ) {
		value = -value;
		how &= ~ METH_MINUS;	// clear flag;
	}
	if( how & METH_RELATIVE ){	//  adjust for relative mode, subtract pc
		how &= ~ METH_RELATIVE;
		value -= npc
			+ (how==METH_ASIS_BYTE ? 1
			: how==METH_ASIS_WORD ? 2
			: how==METH_ASIS_LONG ? 4
			: 2);
	}
	
	switch (how){
	  case METH_ASIS_BYTE:
		emit( (Byte)(value&0xFF) );
		break;
	  case METH_ASIS_WORD:
		emit( (Word)(value&0xFFFF) );
		break;
	  case METH_ASIS_LONG:
		emit( (long) value );
		break;
	  case METH_MUNG_JMP:
		foo = ((value << 5) & 0xE000) | (value & 0xFF) | 0x0400;
		emit((Byte) (foo>>8));
		emit((Byte) (foo&0xff));
		break;
	  case METH_MUNG_CALL:
		foo = ((value << 5) & 0xE000) | (value & 0xFF) | 0x1400;
		emit((Byte) (foo>>8));
		emit((Byte) (foo&0xff));
		break;
	  case METH_MUNG_SJMP:
		foo = (value&0x7ff) | 0x2000;
		emit((Byte) (foo>>8));
		emit((Byte) (foo&0xff));
		break;
	  case METH_MUNG_SCALL:
		foo = (value&0x7ff) | 0x2800;
		emit((Byte) (foo>>8));
		emit((Byte) (foo&0xff));
		break;
	  case METH_RELATIVE:
	  case METH_MINUS:
	  default:
		bug("codegen:install");
		break;
	}
	// pc(opc); do not restore pc!
	// it show be advanced approp'ly

}

void Object::combine(long value, long npc, Method how){
	// update value in object file

	long opc = pc();	// save pc
	long foo;
	
	pc(npc);
	if( how & METH_MINUS ) {
		value = -value;
		how &= ~ METH_MINUS;	// clear flag;
	}
	how &= ~ METH_RELATIVE;		// clear flag, already been adjusted
	
	switch (how){
	  case METH_ASIS_BYTE:
		value += rbyte(npc);
		emit( (Byte)(value&0xFF) );
		break;
	  case METH_ASIS_WORD:
		value += rword(npc);
		emit( (Word)(value&0xFFFF) );
		break;
	  case METH_ASIS_LONG:
		value += rlong(npc);
		emit( (long) value );
		break;
	  case METH_MUNG_JMP:
		foo = (rbyte(npc)<<8) | rbyte(npc+1);
		foo = (foo&0xff) | ((foo>>5)&0x300);
		value += foo;
		foo = ((value << 5) & 0xE000) | (value & 0xFF) | 0x0400;
		emit((Byte) (foo>>8));
		emit((Byte) (foo&0xff));
		break;
	  case METH_MUNG_CALL:
		foo = (rbyte(npc)<<8) | rbyte(npc+1);
		foo = (foo&0xff) | ((foo>>5)&0x300);
		value += foo;
		foo = ((value << 5) & 0xE000) | (value & 0xFF) | 0x1400;
		emit((Byte) (foo>>8));
		emit((Byte) (foo&0xff));
		break;
	  case METH_MUNG_SJMP:
		foo = (rbyte(npc)<<8) | rbyte(npc+1);
		foo &= 0x7ff;
		value += foo;
		foo = (value&0x7ff) | 0x2000;
		emit((Byte) (foo>>8));
		emit((Byte) (foo&0xff));
		break;
	  case METH_MUNG_SCALL:
		foo = (rbyte(npc)<<8) | rbyte(npc+1);
		foo &= 0x7ff;
		value += foo;
		foo = (value&0x7ff) | 0x2800;
		emit((Byte) (foo>>8));
		emit((Byte) (foo&0xff));
		break;
	  case METH_RELATIVE:
	  case METH_MINUS:
	  default:
		bug("codegen:combine");
		break;
	}
	pc(opc);
}



void Object::codegen(void){
	// generate output file
	
	ofstream out;
	Byte d, len, chk;
	Word addrs;
	long i=0,n;
	
	if(outname && *outname){
		out.open(outname);
		if (out.fail()){
			warn("Could not open output file");
			return;
		}
	}else
		out.open("as.out");
	
	switch(outtype){

	  case Intel:
		while(i<ADDRESS_SPACE){
			len = 1;
			chk = 0;
			addrs = (Word)i;
			if( !feel(i++)) continue;
			while( feel(i++) && len<16)	// count run length
				len++;
			if(len==16) i--;
		
			// emit run
			out << ":" << HEX(2) << (unsigned int)len;
			out << "00";	// type = data
			out << HEX(4) << (unsigned short)addrs;
			for(n=0;n<len;n++){
				d = rbyte(addrs + n);
				chk += d;
				out << HEX(2) << (unsigned int)d;
			}
		
			chk = -chk;
			out << HEX(2) << (unsigned int)chk << endl;
		}
		out << ":00000001FF" << endl;
		out.close();
		break;
	  case Binary:
	
		if(incore)
			out.write(core+ADDRESS_START, (int)max+1);
		else
			for(i=ADDRESS_START;i<=max;i++)
				out.put((char)rbyte(i));
		out.close();
		break;

	  case Moto:
		warn("S format Not yet supported");
		break;
	  case None:
	  default:
		break;
	}// eo sw
}






