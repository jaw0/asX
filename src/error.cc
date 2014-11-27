

// $Id: error.cc,v 1.5 1993/06/24 01:15:09 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#include <as.h>
#include <iostream.h>
#include <stdlib.h>

static int nerrs=0;	// number of errors encountered thus far

static void mesg(const char *t, const char *a, const char *b, const char *c, const char *d, const char *e){
	char *x = " ";
	
	cerr << "as: " << t << ": ";
	cerr << "\"" << (const char *)( *currfile?currfile:basefile ) << "\", ";
	cerr << "line " << lineno << ": ";
	cerr << (a?a:x) << (b?b:x) << (c?c:x) << (d?d:x) << (e?e:x) << endl;
}

	
void error(const char *a, const char *b, const char *c, const char *d, const char *e){
	
	mesg("Error", a,b,c,d,e );
	
	if (++nerrs > MAXERR){
		nerrs = 0;
		error("too many errors, giving up.");
		cleanup();
		exit(-1);
	}
}

void warn(const char *a, const char *b, const char *c, const char *d, const char *e){

	mesg("Warning", a,b,c,d,e );
}

void bug(char *where){

	error("internal error (", where?where:"unknown", "), send bug report to ", AUTHOR);
				// yeah, and he'll send you your money back!
}













