

// $Id: main.cc,v 1.4 1993/06/24 01:48:38 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#include <as.h>
#include <patchlevel.h>
#include <fstream.h>
#include <object.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

extern	void 		resolve(void), init_words(void);

extern 	ifstream 	input;
char 	*version 	= VERSION;
int 	lineno;
char 	*currfile = 0;
char 	*basefile;
Boolean	no_opto		=0;

char 	*prepargs[32] = {
	// what to pass to cpp
	"/usr/lib/cpp",
	"-D__AS_X__",
#ifdef MCS48
	"-DMCS48",
	"-Dmcs48"
#else
	"-DMCS96",
	"-Dmcs96"
#endif
};

int 	dbg 	= 0;
extern	int 	errno;
char 	tempfile[256];
Boolean verbose = 0;

main(int argc, char **argv){
	int C=1, i, ncpp=3;
	
	init_words();
	while( --argc){
		// process command line args

		if( argv[C][0] == '-'){
			for(i=1;i<strlen(argv[C]); i++){
				switch(argv[C][i]){
				  case 'v':
					// verbose
					verbose = 1;
					break;
				  case 'V':
					// version
					cerr << version << endl;
					break;
				  case 'h':
					obj.setout(Object::Intel);
					break;
				  case 's':
					obj.setout(Object::Moto);
					break;
				  case 'b':
					obj.setout(Object::Binary);
					break;
				  case 'O':
					no_opto = 1;
					break;
				  case 'o':
					C++;
					argc--;
					i = strlen(argv[C]);
					obj.setout(Object::None, argv[C]);
					break;
				  case 't':
					C++;
					argc--;
					i = strlen(argv[C]);
					syms.setdumpname(argv[C]);
					syms.setdodump(1);
					break;
				  case 'L':
					syms.setdumplocal(1);
					break;
				  case 'T':
					syms.setdodump(1);
					break;
					
				  case 'I':
				  case 'D':
				  case 'U':
					// pass to cpp
					prepargs[ncpp++] = argv[C];
					i = strlen(argv[C]);
					break;

				}
			}
			// What language is this written in?
			C++;

		}else{
			// it is a file name
			// do our thing
			
			// preproc
			tmpnam(tempfile);		// get a tempfile
			prepargs[ncpp] = argv[C]; 	// input file
			prepargs[ncpp+1] = tempfile;	// out file
			prepargs[ncpp+2] = (char*)0;

			char buf[1024];
			*buf=0;
			for (i=0;i<ncpp+2;i++){
				sprintf(buf,"%s %s", buf, prepargs[i]);
			}
			
			dbg = system(buf);
			
			input.open(tempfile);	// open file
			parsefile();		// assemble it
			input.close();
			unlink(tempfile);	// remove tempfile

			C++;
		}
	}

	resolve();	// resolve fwd refs.
	// spit out resultant file
	obj.codegen();

	syms.dump();
}


void cleanup(void){

	unlink(tempfile);
}

