

// $Id: object.h,v 1.6 1993/08/02 23:59:32 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#ifndef _OBJECT_H
#define _OBJECT_H

#include <stdio.h>
#include <symtab.h>

class Object {
  public:
	enum OutType {None, Intel, Moto, Binary };

  private:
	Boolean incore;			// is it in file or memory
	char tempname[256];		// the temp file
	char mark[ADDRESS_SPACE/8];	// where have we been
	FILE *fp;			// theres that tempfile again
	Byte *core;			// memories, oh memories ....
	long x_pc;			// program counter, NOT ibm
	long max;
	OutType outtype;
	char outname[256];

	void touch(long x)	{		// been there, done that
		mark[ x/8 ] |= (char)(1 << (x % 8));
		max = max<x?x:max;
	}
	Boolean feel(long x)    {return mark[ x/8 ] & (1 << (x % 8));}

  public:
	long dot;
	Object();
	~Object();
	void pc(long n);			// set pc
	long pc(void)		{return x_pc;}	// get pc
	void inc(long n=1);			// incr. pc
	void emit(Byte c);			// emit code
	void emit(Word w);
	void emit(long l);
	void emit(void *p, int n);
	Byte rbyte(long pc);			// read back code
	Word rword(long pc);
	long rlong(long pc);
	void combine(long value, long pc, Method how);	// update the value
	void install(long value, long pc, Method how);	// initial put value
	void setout(OutType t=None, char *name=0);
	void codegen(void);
};

extern Object obj;

#endif !_OBJECT_H
