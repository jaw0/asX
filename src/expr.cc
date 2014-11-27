

// $Id: expr.cc,v 1.6 1993/08/02 23:59:16 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#include <as.h>
#include <stdlib.h>
#include <token.h>
#include <symtab.h>
#include <expr.h>
#include <object.h>

extern Mnemonic get_token();
//extern double log2(double);

long Expr::parse(char *msg){
	
	value = 0;
	gotothop = gotfwd = mrpf = 0;
	errmsg = msg;
	
	value = expr();
	if (gotfwd && gotothop)
		error("invalid use of forward reference.", " ", errmsg);

	if(!nofwd){
		obj.install(value, pc, how);	// constant part of expr
		
	}
	return value;
}

long Expr::expr(void){

	return orexpr();
}

long Expr::orexpr(void){
	long left = andexpr();

	while(currtok.mnem == CHOR){
		get_token();
		gotothop = 1;
		left |= andexpr();
	}
	return left;
}

long Expr::andexpr(void){
	long left = xorexpr();

	while(currtok.mnem == CHAND){
		get_token();
		gotothop = 1;
		left &= xorexpr();
	}
	return left;
}

long Expr::xorexpr(void){
	long left = shiftexpr();

	while(currtok.mnem == CHXOR){
		get_token();
		gotothop = 1;
		left ^= shiftexpr();
	}
	return left;
}

long Expr::shiftexpr(void){
	long left = addexpr();
	while(1){
		switch(currtok.mnem){
		  case LSHIFT:
			get_token();
			left <<= addexpr();
			gotothop = 1;
			break;
		  case RSHIFT:
			get_token();
			left >>= addexpr();
			gotothop = 1;
			break;
		  default:
			return left;
		}
	}
}

long Expr::addexpr(void){
	long left = multexpr();

	while(1){
		switch(currtok.mnem){
		  case PLUS:
			get_token();
			left += multexpr();
			break;
		  case MINUS:
			get_token();
			left -= multexpr();
			if(mrpf){
				// we might have foo - fwdref
				how ^= METH_MINUS;	// toggle bit
			}
			break;
		  default:
			return left;
		}
	}
}

long Expr::multexpr(void){
	long left = unary();

	while(1){
		switch(currtok.mnem){
		  case TIMES:
			get_token();
			left *= unary();
			gotothop = 1;
			break;
		  case DIVIDE:
			get_token();
			left /= unary();
			gotothop = 1;
			break;
		  case MODULO:
			get_token();
			left %= unary();
			gotothop = 1;
			break;
		  default:
			return left;
		}
	}
}

long Expr::unary(void){
	long v;
	
	switch(currtok.mnem){
	  case PLUS:
		get_token();
		return unary();
	  case MINUS:
		get_token();
		v = -unary();
		if(mrpf){
			// we might have a -fwdref
			how ^= METH_MINUS;	// toggle bit
		}
		return v;
	  case CHNOT:
		get_token();
		gotothop = 1;
		return ~unary();
	  default:
		return primary();
	}
}

long Expr::primary(void){
	long v;
	Expr fncexp;
	
	mrpf = 0;
	
	switch(currtok.mnem){
	  case CONST:
		v = currtok.value;
		get_token();
		return v;
	  case DOLLAR:
	  case DOT:		// insert value of prog. counter
		v = obj.dot;	// pc at beginning of line
		get_token();
		return v;
	  case LPAREN:
		get_token();
		v = expr();
		if(currtok.mnem != RPAREN)
			error("I see no closing ) here.", " ", errmsg);
		get_token();
		return v;
	  case LBRACE:
		get_token();
		v = expr();
		if(currtok.mnem != RBRACE)
			error("I see no closing } here.", " ", errmsg);
		get_token();
		return v;	
	  case LABEL:
		syms.add(currtok.name, 0, SYM_FWD);
		currtok.symb = syms.find(currtok.name);
		// fall thru
		// weeeee.....
	  case FWDREF:
		// here is the tough part...
		gotfwd = mrpf = 1;
		if (nofwd){
			error("forward reference to \"", currtok.name, "\" not permitted here.", " ", errmsg);
			get_token(); // gobble, gobble
			return 0;
		}
		relocd.add(pc, how, currtok.symb);
		get_token();	// gotta eat
		return 0;
		
		// feep some creatures ...
		
	  case LOG:
		fncexp.nofwd=1;
		get_token();
		if (currtok.mnem != LPAREN){
			error("log function requires (");
		}else{
			get_token(); // try to keep going
		}
		v = fncexp.parse();
		if (currtok.mnem != LPAREN){
			error("log function requires )");
		}else{
			get_token(); // try to keep going
		}
		return (long)v;  //log2( (double)v );

	  default:
		get_token();
		error("invalid expression");
		return -1;
	}
}










