

// $Id: expr.h,v 1.4 1993/05/28 04:40:51 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

// data needed to parse expressions

#ifndef _EXPR_H
#define _EXPR_H

class Expr {
  private:
	Boolean gotothop, gotfwd, mrpf;
	char *errmsg;

	// parse intermediate stages
	long expr(void);
	long orexpr(void);
	long andexpr(void);
	long xorexpr(void);
	long shiftexpr(void);
	long addexpr(void);
	long multexpr(void);
	long unary(void);
	long primary(void);
	
  public:
	Boolean nofwd;
	long pc;
	Method how;
	long value;

	Expr(){}
	~Expr(){}
	Boolean didfwd(void){return gotfwd;}	// was there a fwd ref in previous pars job
	long parse(char *msg=0);		// this is the entry point from the outside world
};


#endif !_EXPR_H

