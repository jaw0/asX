

// $Id: parse.cc,v 1.7 1993/08/02 23:59:23 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#include <as.h>
#include <symtab.h>
#include <token.h>
#include <string.h>
#include <object.h>
#include <expr.h>

static Mnemonic parseline(void);
long expr(void);
Mnemonic get_token(void);

static long enumber = 0;

static void eatline(void){

	while(currtok.mnem != EOLINE && currtok.mnem != EOFILE)
		get_token();
	// leave EOLINE as next, so lineno gets ++ed
}

void parsefile(void){
	// ought do preproc. here
	
	get_token();
	while( parseline() != EOFILE );

}

static Mnemonic parseline(void){
	// the real work goes on here ...
	char buf[256];
	long n;
	Expr expr;

//  resw:	label in block with destructors not allowed [sigh]
	obj.dot = obj.pc();
	
	switch(currtok.mnem){

	  case EOFILE:		// all gone...
		break;
	  case EOLINE:
		lineno++;	// increment line number
		get_token();
		break;
	  case POUND:
		// handle preprocessor output (line numbers and suchlike)
		// they are expected to be of the format:
		// # line "filename" [foo]
		get_token();
		if ( currtok.mnem != CONST ){
			error("invalid preprocessor line");
			eatline();
			break;
		}
		lineno = (int)currtok.value - 1;	// refers to next line
		get_token();
		if (currtok.mnem != STRING){
			error("invalid preprocessor line");
			eatline();
			break;
		}
		currfile = new char [strlen(currtok.name)];
		strcpy(currfile, currtok.name);
		eatline();
		break;
	  case LABEL:
		strcpy(buf, currtok.name);
		get_token();
		switch(currtok.mnem){
		  case COLON:
			// a label
			syms.add(buf, obj.pc(), SYM_OK);
			get_token();
			break;
		  case EQU:
			// an equate
			get_token();
			expr.nofwd = 1;
			n = expr.parse("invalid rhs for equate");
			syms.add(buf, n, SYM_OK);
			eatline();	// nothing else allowed on line
			break;
		  default:
			error("what is this thing \"", buf, "\" that you speak of?");
			eatline();
			break;
		}
		break;
	  case FWDREF:
		currtok.symb->update(obj.pc(), SYM_OK);
		get_token();
		if (currtok.mnem != COLON){
			error("I see no colon here");
			eatline();
			break;
		}
		get_token();
		break;
	  case ORG:
		get_token();
		expr.nofwd = 1;
		n = expr.parse("invalid org directive, constant required");
		obj.pc(n);
		eatline();
		break;
	  case PSSKIP:			// to the loo my darlin'
		get_token();
		expr.nofwd = 1;
		n = expr.parse("invalid skip directive, constant required");
		obj.inc( n );
		eatline();
		break;
	  case EVEN:
		n = obj.pc();
		n %= 2;
		if (n) obj.inc( n );
		eatline();
		break;
	  case ALIGN:
		get_token();
		expr.nofwd = 1;
		n = expr.parse("invalid align directive, constant required");
		if((n!=1)
		   &&(n!=2)
		   &&(n!=4)
		   &&(n!=8)
		   &&(n!=16)
		   &&(n!=32)
		   &&(n!=43)	// to match the typo in the describble file...
		   &&(n!=64)
		   &&(n!=128)
		   &&(n!=256)
		   ){
			error("illegal alignment specifier, must be one of: 1,2,4,8,16,32,64,128,256");
		}
		n = (n - (obj.pc()%n))%n;
		if (n) obj.inc( n );
		eatline();
		break;
	  case ABORT:
		get_token();
		warn("user abort, please play again");
		exit(-1);
	  case BYTE:
		get_token();
		do {
			if (currtok.mnem == COMMA) get_token();
			expr.pc = obj.pc();
			expr.how = METH_ASIS_BYTE;
			expr.nofwd = 0;
			expr.parse("bad .byte" /* see orthodontist */);
		} while( currtok.mnem == COMMA );
		break;
	  case WORD:
		get_token();
		do {
			if (currtok.mnem == COMMA) get_token();
			expr.pc = obj.pc();
			expr.how = METH_ASIS_WORD;
			expr.nofwd = 0;
			expr.parse("bad .word" /* wash mouth with soap */);
		} while( currtok.mnem == COMMA );
		break;		
	  case LONG:
		get_token();
		do {
			if (currtok.mnem == COMMA) get_token();
			expr.pc = obj.pc();
			expr.how = METH_ASIS_LONG;
			expr.nofwd = 0;
			expr.parse("bad .long");
		} while( currtok.mnem == COMMA );
		break;		
	  case ASCII:
		get_token();
		if (currtok.mnem != STRING){
			error("I see no string here");
			eatline();
			break;
		}
		obj.emit(currtok.name, strlen(currtok.name));
		eatline();
		break;
	  case ASCIZ:
		get_token();
		if (currtok.mnem != STRING){
			error("I see no string here");
			eatline();
			break;
		}
		obj.emit(currtok.name, strlen(currtok.name));
		obj.emit( (Byte)00 );
		eatline();
		break;
	  case ENUM:
		get_token();
		do {
			if (currtok.mnem == COMMA) get_token();
			switch(currtok.mnem){
			  case FWDREF:
			  case LABEL:
				strcpy(buf, currtok.name);
				break;
			  default:
				error("illegal identifier for .enum");
				break;
			}
			get_token();
			if(currtok.mnem==EQU){
				get_token();
				expr.nofwd = 1;
				enumber = expr.parse("enum must eval to a constant");
			}
			syms.add(buf, enumber, SYM_OK);
			enumber++;
		} while( currtok.mnem == COMMA );
		eatline();
		break;
	  case PROC:
		// enter new scope for local labels
		syms.nproc();
		get_token();
		break;
	  case TITLE:
		warn("title not yet implemented");
		eatline();
		break;
	  case DOHEX:
		get_token();
		if(currtok.mnem == STRING)
			obj.setout(Object::Intel, currtok.name);
		else
			obj.setout(Object::Intel);
		eatline();
		break;
	  case DOS19:
		get_token();
		if(currtok.mnem == STRING)
			obj.setout(Object::Moto, currtok.name);
		else
			obj.setout(Object::Moto);
		eatline();
		break;
	  case DOBIN:
		get_token();
		if(currtok.mnem == STRING)
			obj.setout(Object::Binary, currtok.name);
		else
			obj.setout(Object::Binary);
		eatline();
		break;
	  case DOSYM:
		get_token();
		syms.setdodump(1);
		if(currtok.mnem == STRING)
			syms.setdumpname(currtok.name);
		eatline();
		break;
	  case DOLOCAL:
		syms.setdumplocal(1);
		eatline();
		break;
		

	  default:

#ifdef MCS48
		extern void parse48(void);
		parse48();	// machine dependant parsing
#else
#	ifdef MCS96
		extern void parse96(void);
		parse96();
#	endif
#endif
		break;

	} //  eo sw

	
	return currtok.mnem;
}


















