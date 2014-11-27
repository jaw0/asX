

// $Id: symtab.cc,v 1.9 1993/08/02 23:59:29 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"


#include <as.h>
#include <symtab.h>
#include <string.h>
#include <fstream.h>
#include <iomanip.h>

Symtab syms;
Reloc  relocd;

int Symtab::hash(char *p){
	int i=0;

	// adapted from Stroustrup's C++ book
	while (*p) i = i<<1 ^ *p++;
	i = i<0 ? -i : i;
	i %= TBLSIZE;

	return i;
}

long Symtab::valof(char *name){
	Symbol *s;
	
	s = find(name);

	return s ? s->valof() : 0;
}

int Symtab::exist(char *name){
	Symbol *s;

	s = find(name);

	return s ? 1 : 0;
}
	
Symbol *Symbol::find(char *what, long scp){
	// find a match
	
	if(!this) return NULL;
	if ( (status!=SYM_EMPTY)	// legit entry?
	     && !strcmp(what, name)	// name match?
	     && (!scope || scp==scope)	// proper scope if local?
	     )
		return this;

	if (!next)
		return NULL;

	return next->find(what);
}

void Symtab::add(char *name, long offset, Sym_Stat type){
	int h = hash(name);
	Symbol *s, *nx = symbol[h];

	if ( name[strlen(name)-1]!='$' && exist(name) ){
		// it is not local but already exists
		error("redefinition of ", name);
		return;
	}

	if ( nx && (s=nx->old()) )
		;
	else
		s = new Symbol;
	s->add(nx, name, offset, type, scope);
	symbol[h] = cache = s;
}
	
Sym_Stat Symtab::typof(char *name){
	Symbol *s;

	s = find(name);

	return s ? s->typof() : SYM_EMPTY;
}

void Symbol::add(Symbol *nx, char *n, long v, Sym_Stat t, long scp){

	name = new char[ strlen(n) + 1 ];
	strcpy(name, n);
	status = t;
	value = v;
	if(name[strlen(name)-1]=='$')	// local label
		scope = scp;
	else
		scope = 0;
	firstseen = lineno;	// save spot we 1st saw it
	firstfile = currfile;
	next = nx;
	if (nx) nx->prev = this;
}


Symbol *Symbol::old(void){
	// find an empty entry, so we can reuse it
	
	if ( status==SYM_EMPTY )
		return this;

	if (!next)
		return NULL;

	return next->old();
}

Symbol *Symtab::find(char *name){
	// find item in the symbol tabol
	
	Symbol *s;

	if ( cache					  // cache exist?
	     && (cache->status != SYM_EMPTY)		  // has a legit entry?
	     && !strcmp(name, cache->name)		  // name match?
	     && ( !cache->scope || cache->scope == scope) // proper scope if local?
	     ){
		return cache;
	}else{
		s = symbol[ hash(name) ];
		s = s? s->find(name, scope):0;
		if (s) cache = s;
		
		return s;
	}
}
	
void Reloc::add(unsigned long a, Method h, Symbol *s){
	RelocItem *ri;

	ri = new RelocItem;
	ri->add(a,h,s, x_tail?x_tail->x_next:NULL);

	if (x_tail)
		x_tail->x_next = ri;
	else
		x_tail = x_head = ri;
}

void Symbol::update(long v, Sym_Stat s){

	if (status == SYM_OK){
		// can't really ever happen
		error("redefinition of ", name);
	}
	status = s;
	value = v;
}

Symtab::Symtab() {
	// hard hat area
	
	int i;
	for(i=0;i<TBLSIZE;i++)symbol[i]=NULL;
	cache = NULL;
	scope = 1;
	*dumpfile = 0;
	dodump = dumplocal = 0;
	
}

void Symtab::setdumpname(char *n){

	if(n && *n)
		strcpy(dumpfile, n);
}


void Symtab::dump(void){
	// dump the symbol table

	ofstream out;
	long i;

	if( !dodump) return;
	
	if(dumpfile && *dumpfile){
		out.open(dumpfile);
		if (out.fail()){
			warn("Could not open symbol file");
			return;
		}
	}else
		out.open("a.sym");
	
	out.width(32);
	out.setf(ios::left, ios::adjustfield);
	out << "Symbol name";
	out.width(4);
	out << "L";
	out.width(6);
	out << "line";
	out.width(6);
	out << "value";
	out << endl << endl;
	
	for(i=0;i<TBLSIZE; i++){
		if ( symbol[i] ) symbol[i]->dump(out, dumplocal);
	}
}

void Symbol::dump(ofstream &out, Boolean doloc){
	Boolean ami;

	if (next) next->dump(out, doloc);

	ami = name[strlen(name)-1]=='$';
	if(!ami || doloc){
		// dump me
		out.width(32);
		out.setf(ios::left, ios::adjustfield);
		out << name;
		out.width(4);
		if(ami) out << dec << scope;
		else out << ".";
		out.width(6);
		out << dec << firstseen;
		//out.width(6);
		out << "0x" << hex <<value;
		out << endl;
	}

}

