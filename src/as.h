

// $Id: as.h,v 1.9 1993/06/24 01:45:22 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#ifndef _AS_H
#define _AS_H

#include "/dev/null"	// for bug reports and user complaints
#include <version.h>
#include <patchlevel.h>
#include <iostream.h>	// for debugging output

#ifdef MCS48
#	define ADDRESS_SPACE 4096
#	define ADDRESS_START 0
#else
#	define ADDRESS_SPACE 65536
#	define ADDRESS_START 0x2000
#endif

#define AUTHOR  	"jaw@magoo.roc.ny.us (Jeff Weisberg)"
#define VERSION 	"asX Ver. " _VERSION "." _PATCHLEVEL "; (c) 1993 Jeff Weisberg"

#define TBLSIZE 	253    
#define MAXERR		10

typedef unsigned char 	Byte;
typedef short 		Word;
typedef int 		Boolean;

// some global vars
extern char 	*version;
extern int 	lineno;			// line num. in input file
extern char 	*currfile;		// name of current input file
extern char 	*basefile;		// name of input file from command line
extern Boolean 	verbose;		// print out some extra diagnosticals


extern char pushchar(char c), popchar(void);
extern void pushstr(char *t);
extern void error(const char *a=0, const char *b=0, const char *c=0, const char *d=0, const char *e=0);
extern void warn(const char *a=0, const char *b=0, const char *c=0, const char *d=0, const char *e=0);
extern void parsefile(void);
extern void bug(char *where=0), cleanup(void);




#if  ( defined(MCS48) && defined(MCS96) ) || ( !defined(MCS48) && !defined(MCS96) )
#	include "TARGET must be specified in the Makefile as either 48 or 96"
#endif

#endif !_AS_H

