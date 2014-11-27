

// $Id: object.cc,v 1.6 1993/08/02 23:59:22 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#include <as.h>
#include <object.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>

Object obj;


	
Object::Object(){
	// constructor;
	int i;
	
	core = new Byte[ADDRESS_SPACE];
	incore = 1;
	if(!core){
		incore = 0;
		// not enuff mem, use file
		if(! (fp = fopen(tmpnam(tempname), "rw")) ){
			error("could not open temporary file");
			exit(-1);
		}
	}
	x_pc = max = 0;
	for(i=0;i<ADDRESS_SPACE/8;i++) mark[i] = 0;
	outtype = Intel;
	*outname =0;
}

Object::~Object(){
	// destructor

	if(!incore){
		// need to remove file
		unlink(tempname);
	}else
		delete[] core;
}

void Object::pc(long n){
	// set pc

	x_pc = n;
	if (x_pc<0){
		error("program counter went negative, aborting");
		exit(-1);
	}
	if(x_pc>ADDRESS_SPACE){
		warn("program counter has exceeded the address space");
	}
	if(!incore){
		fseek(fp, x_pc, 0);
	}
	
}

void Object::inc(long n){
	// incr pc

	x_pc += n;
	if(!incore){
		fseek(fp, x_pc, 0);
	}
}

void Object::emit(Byte c){

	touch(x_pc);
	if(incore)
		core[x_pc] = c;
	else
		fwrite((char*)&c, 1, 1, fp);
	x_pc++;
}

void Object::emit(Word w){

	//assumes target is a little indian

	touch(x_pc); touch(x_pc+1);
	
	if(incore){
		core[x_pc] = (Byte)(w&0xFF);
		core[x_pc+1]=(Byte)(w>>8);
	}else{
		char c=w&0xFF, cc=w>>8;
		fwrite( &c, 1, 1, fp);
		fwrite(&cc, 1, 1, fp);
	}
	x_pc +=2;
}

void Object::emit(long l){

	// assumes target is little little indian

	touch(x_pc); touch(x_pc+1); touch(x_pc+2); touch(x_pc+3);

	if(incore){
		core[x_pc] = (Byte)(l&0xFF);	// onr little,
		core[x_pc+1]=(Byte)((l>>8)&0xFF);	// two little,
		core[x_pc+2]=(Byte)((l>>16)&0xFF);// three little indians,
		core[x_pc+3]=(Byte)((l>>24)&0xFF);// four little,
	}else{
		char c=(char)(l&0xff), cc=(char)((l>>8)&0xFF),
			ccc=(char)((l>>16)&0xFF), cccc=(char)((l>>24)&0xFF);
		fwrite(   &c, 1, 1, fp);	// five little,
		fwrite(  &cc, 1, 1, fp);	// six little indains,
		fwrite( &ccc, 1, 1, fp);	// seven little
		fwrite(&cccc, 1, 1, fp);	// eight little
		"nine little indians"; 		// ten little indian boys!
	}
	x_pc += 4;
}

void Object::emit(void *p, int n){
	int i;
	
	for(i=0;i<n;i++) touch(x_pc + i);
	
	if(incore){
		memcpy(core+x_pc, p, n);
	}else{
		fwrite((const char *)p, n, 1, fp);
	}
	x_pc += n;
}

Byte Object::rbyte(long npc){
	Byte c;
	
	if(incore) return core[npc];

	fseek(fp, npc, 0);
	fread((char*)&c, 1, 1, fp);
	fseek(fp, x_pc, 0);	// restore
	return c;
}

Word Object::rword(long npc){
	Byte c, cc;

	if (incore) return (core[npc] | (core[npc+1]<<8));

	fseek(fp, npc, 0);
	fread((char*) &c, 1, 1, fp);
	fread((char*)&cc, 1, 1, fp);
	fseek(fp, x_pc, 0);
	return (c | (cc << 8));
}

long Object::rlong(long npc){
	Byte c,cc,ccc,cccc;

	if(incore) return core[npc]
		| (core[npc+1]<<8)
		| (core[npc+2]<<16)
		| (core[npc+3]<<24);

	fseek(fp, npc, 0);
	fread((char*)   &c, 1, 1, fp);
	fread((char*)  &cc, 1, 1, fp);
	fread((char*) &ccc, 1, 1, fp);
	fread((char*)&cccc, 1, 1, fp);
	fseek(fp, x_pc, 0);
	return c | (cc<<8) | (ccc<<16) | (cccc<<24);
}


		
void Object::setout(OutType t, char *n){

	if(t!=None)
		outtype=t;
	if(n && *n)
		strcpy(outname, n);
}
