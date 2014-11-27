

// $Id: symtab.h,v 1.8 1993/08/02 23:59:35 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#ifndef _SYMTAB_H
#define _SYMTAB_H

enum  Sym_Stat {	// what kind of symbol
	SYM_EMPTY,	// none
	SYM_FWD,	// forward reference
	SYM_OK		// normal case
};

class Symtab;
class Symbol;
class ofstream;

class Symbol {
    friend class Symtab;
    
  private:
    	char     *name;			// name of symbol
        long     value;			// value of symbol	
        Sym_Stat status;
	long	 scope;
	long     firstseen;		// line of first ocuurance
	char	 *firstfile;	       	// and file
        Symbol   *next, *prev;		// linked list maintenance
	
	void     add(Symbol *nx, char *n, long v, Sym_Stat t, long scp);
	void	 kill() 	{status = SYM_EMPTY;}
	Symbol   *old(void);

  public:
        Symbol() 		{next=prev=0; status=SYM_EMPTY; scope=0;}
        ~Symbol() 		{}
	Symbol   *find(char *name, long scp=0);	// follow list, find name
	long     valof() 	{return value;}
	Sym_Stat typof() 	{return status;}
	char	 *namof(void)	{return name;}
	long	 fseen(void)	{return firstseen;}
	char	 *file(void)	{return firstfile;}
	void	 dump(ofstream &out, Boolean dolcl);		
	void     update(long v, Sym_Stat s);
};

class  Symtab {
  private:
	Symbol   *symbol[TBLSIZE];	// hash table
	Symbol   *cache;		// cache to speed up searches
	int      hash(char *name);	// hash function
	long	 scope;
	char	 dumpfile[256];
	Boolean	 dumplocal;
	Boolean	 dodump;
	
  public:
	Symtab();
	~Symtab() 		{}
	
	void     add(char *name, long value, Sym_Stat type);
	long     valof(char *name);
	Sym_Stat typof(char *name);
	Symbol   *find(char *name);
	int      exist(char *name);
	void	 kill(char *name);
	void	 nproc(void) 	{scope++;}	// new scope level
	void	 nproc(int n)	{scope=n;}
	void	 dump(void);  			// dump out the symbol table
	void	 setdumpname(char *n);
	void	 setdumplocal(Boolean b)	{dumplocal=b;}
	void	 setdodump(Boolean b)		{dodump=b;}
};

extern Symtab syms;



enum Method {
	METH_ASIS_BYTE,
	METH_ASIS_WORD,
	METH_ASIS_LONG,
	METH_MUNG_JMP,		// `48
	METH_MUNG_CALL,		// `48
	METH_MUNG_SJMP,		// `96
	METH_MUNG_SCALL,	// `96
	
	METH_RELATIVE = 0x4000,	// relative mode (for bounds test)
	METH_MINUS = 0x8000	// negate the fwd ref.
};


class RelocItem {
	// for reloation data
    friend class Reloc;
  private:
	RelocItem 	*x_next;
	long 		x_pc;
	Method 		x_how;
	Symbol 		*symb;

  public:
	RelocItem()			{x_next=0;symb=0;x_pc=-1;}
	~RelocItem()			{}
	void 		add(long a, Method h, Symbol *s, RelocItem *r){
		x_pc = a; x_how = h; symb = s; x_next = r;
	}
	RelocItem 	*next(void)	{return x_next;}
	long 		pc(void)	{return x_pc;}
	long 		valof(void)	{return symb->valof();}
	Sym_Stat 	typof(void) 	{return symb->typof();}
	char		*namof(void)	{return symb->namof();}
	long		where(void)	{return symb->fseen();}
	char		*file(void)	{return symb->file(); }
	Method 		how(void)	{return x_how;}
};

class Reloc {
  private:
	RelocItem *x_head, *x_tail;

  public:
	Reloc()				{x_head=x_tail=0;}
	~Reloc()			{}
	void 		add(unsigned long a, Method h, Symbol *s);
	RelocItem 	*head(void)	{return x_head;}
};

extern Reloc relocd;


#endif !_SYMTAB_H


