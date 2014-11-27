
// $Id: input.cc,v 1.7 1993/08/02 23:59:18 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#include <as.h>
#include <string.h>
#include <fstream.h>
#include <stack.h>

static Stack instk;
ifstream input;

char pushchar(char c){

	instk.push(c);
	return c;
}

char popchar(void){
	int c;
	char cc;
	
	c = instk.pop();
	if ( c > 0 ) return c;

	// snarf from file

	if( !input.good() ) return 0;
	input.get(cc);
	return cc;
				// not quite the way I want to do it
				// but 'tis enough, 'twill serve
}


