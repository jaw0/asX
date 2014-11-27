

// $Id: lexer.cc,v 1.11 1993/08/02 23:59:19 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#include <as.h>
#include <symtab.h>
#include <token.h>
#include <ctype.h>
#include <string.h>


Symtab rsvwd;
Token currtok;


// Hey Pete! this look familair?
// getnum works on real computers too!

static unsigned char vallof(char c){

	if(c<='9') return c - '0';
	if(c<'A') return 255;
	if(c<='Z') return c - 'A' + 0xA;
	if(c<'a') return 255;
	if(c<='z') return c - 'a' + 0xa;
	return 255;
}

inline static int islegit(int base, char c){
	return vallof(c) < base;
}

static long getnum(int base, char cc=0){
        char ch;
        long val=0;

	if (cc) pushchar(cc);
        while( islegit(base, (ch=popchar())))
                val = (val * (long)base) + (long)vallof(ch);

	pushchar(ch);
        return val;
}



static char escchar(void){
	char ch;
	int n=0;

	ch = popchar();
	switch(ch){
	  case '0': case '1': case '2':
	  case '3': case '4': case '5':
	  case '6': case '7':
	  		// read an octal number
		n = getnum(8, ch);
		return (char)n;
	  case '\\': return '\\';
	  case 'a':  return '\a';
	  case 'b':  return '\b';
	  case 'n':  return '\n';
	  case 'r':  return '\r';
	  case 't':  return '\t';
	  case 'v':  return '\v';
	  case 'f':  return '\f';
	  case 'e':  return '\033';	// esc
	  case '\n':
		lineno++;
		return '\n';
	  default:
		return ch;
	}
}

Mnemonic get_token(void){

	int ch, n;

	ch = popchar();
	if(ch<1) return currtok.mnem = EOFILE;
	if(ch < '\t' || ch > 126){
		// don`t even bother with the file if it looks like this
		error("illegal characters on input");
		return currtok.mnem = EOFILE;
	}
	
  resw:
	switch (ch){

	  case ' ':
	  case '\t':
	  case '\r':
		// eat white space
	        ch = popchar();
		goto resw;

	  case '/':
		ch = popchar();
		if(ch!='/'){
			pushchar(ch);
			currtok.mnem = DIVIDE;
			break;
		}
		// fall thru' for // comment
	  case ';':	// eat comment
		while((ch!='\n')&&(ch>0))
			ch=popchar(); // till eol
		currtok.mnem = EOLINE;
		break;

	  case '\n': currtok.mnem = EOLINE;	break;
	  case '#':  currtok.mnem = POUND;	break;
	  case ',':  currtok.mnem = COMMA;	break;
	  case '@':  currtok.mnem = ATSIGN;	break;
	  case '+':  currtok.mnem = PLUS;	break;
	  case '-':  currtok.mnem = MINUS;	break;
	  case '*':  currtok.mnem = TIMES;	break;
	  // case '/':  currtok.mnem = DIVIDE;	break;
	  case '&':  currtok.mnem = CHAND;	break;
	  case '|':  currtok.mnem = CHOR;	break;
	  case '^':  currtok.mnem = CHXOR;	break;
	  case '=':  currtok.mnem = EQU;	break;
	  case '(':  currtok.mnem = LPAREN;	break;
	  case ')':  currtok.mnem = RPAREN;	break;
	  case '[':  currtok.mnem = LBRACK;	break;
	  case ']':  currtok.mnem = RBRACK;	break;
	  case '{':  currtok.mnem = LBRACE;	break;
	  case '}':  currtok.mnem = RBRACE;	break;
	  case '~':  currtok.mnem = CHNOT;	break;
	  case '<':
		ch = popchar();
		if(ch == '<'){
			currtok.mnem = LSHIFT;
			break;
		}
		error("invalid characters \"<\"");
		pushchar(ch);
		break;
	  case '>':
		ch = popchar();
		if(ch == '>'){
			currtok.mnem = RSHIFT;
			break;
		}
		error("invalid characters \">\"");
		pushchar(ch);
		break;
	  case '"':
		// a string
		n = 0;
		do{
		  notest:
			currtok.name[n] = popchar();
			if( currtok.name[n] == '\\' ){
				currtok.name[n] = escchar();
				n++;
				goto notest;
			}
			n++;
		}while ( currtok.name[n-1]
			 && currtok.name[n-1] != '\n'
			 && currtok.name[n-1] != '"' );
		n--;
		if(currtok.name[n] != '"')
			error("I see no \" terminating this string");
		currtok.name[n] = 0;
		currtok.mnem = STRING;
		break;

	  case '`':		
		currtok.mnem = CONST;
		currtok.value = popchar();
		if( currtok.value == '\\' ) currtok.value = escchar();
		break;
	  case '\'':
		currtok.mnem = CONST;
		currtok.value = popchar();
		if( currtok.value == '\\' ) currtok.value = escchar();
		if (popchar() != '\'')
			error("illegal character constant");
		break;
	  case ':':
		currtok.mnem = COLON;
		"10 PRINT \"My Name\";";
		"20 GOTO 10";

		break;
	  case '$':		// either a lone $, or hex num.
		ch = popchar();
		if ( islegit(16, ch)){
			currtok.value = getnum(16, ch);
			currtok.mnem = CONST;
			break;
		}
		pushchar(ch);
		currtok.mnem = DOLLAR;
		break;
	  case '%':		// a binary num
		currtok.value = getnum(2);
		currtok.mnem = CONST;
		break;

	  case '0':
                ch = popchar();
                switch(ch){
                  case 'x': n = 16;	break;	// 0xnn hex
		  case 'b': n = 2;	break;	// 0bnn binary
		  case 'd': n = 10;	break;	// 0dnn decimal
		  case 'o': n = 8;	break;	// 0onn octal

		  case '1': case '2': case '3': case '4':
		  case '5': case '6': case '7': case '0':	// 0nn octal
			n = -8;
			break;

		  default:	// just a 0
			n = 0;
			pushchar(ch);
			break;
		}
		if(n>0) currtok.value = getnum(n);
		else if(n<0) currtok.value = getnum(-n, ch);
		else  currtok.value = 0;
		currtok.mnem = CONST;
		break;
		
	  case '1': case '2': case '3':
	  case '4': case '5': case '6':
	  case '7': case '8': case '9':
		currtok.value = getnum(10, ch);
		currtok.mnem  = CONST;
		break;

	  default:
		char *p;
		if(! (isalpha(ch)||ch=='.'||ch=='$'||ch=='_' )){
			char quux[2];
			quux[0] = ch; quux[1] = 0;
                        error("bad character on input \"", quux, "\"");
		}
                p = currtok.name;
                *p++ = ch;
                while( (isalnum(ch=popchar())||ch=='.'||ch=='$'||ch=='_'  )) *p++ = ch;
                pushchar(ch);
                *p = 0;
		char buff[256];
		int i = 0;
		
		// check reserved word as lowercase
		while(buff[i]=tolower(currtok.name[i])) i++;
		if ( rsvwd.exist(buff)){
			currtok.mnem = rsvwd.valof(buff);
			if(rsvwd.typof(buff) != SYM_OK)
				warn("frowned upon syntax \"", currtok.name, "\"");
			break;
		}
		Symbol *sy = syms.find(currtok.name);

		if ( !sy){
			currtok.mnem = LABEL;
			break;
		}
		if ( sy->typof() == SYM_OK ){
			currtok.mnem = CONST;
			currtok.value = sy->valof();
			break;
		}
		if (sy->typof() == SYM_FWD ){
			currtok.mnem = FWDREF;
			currtok.symb = sy;
			break;
		}

		// it had better fit one of the above conditions
		bug("lexer:get_token");

		
	}// eo sw

        if (verbose)	// debug output
		cerr << "Line: " << lineno
		     << " \tToken: " << (int)currtok.mnem
		     << ",\t " << currtok.value
		     << ",\t " << currtok.name
		     <<endl ;
	return currtok.mnem;
}


void init_words(void){

	rsvwd.add(".org", ORG, SYM_OK);
	rsvwd.add(".title", TITLE, SYM_OK);
	rsvwd.add(".ascii", ASCII, SYM_OK);
	rsvwd.add(".asciz", ASCIZ, SYM_OK);
	rsvwd.add(".byte", BYTE, SYM_OK);
	rsvwd.add(".word", WORD, SYM_OK);
	rsvwd.add(".long", LONG, SYM_OK);
	rsvwd.add(".skip", PSSKIP, SYM_OK);
	rsvwd.add(".align", ALIGN, SYM_OK);
	rsvwd.add(".even", EVEN, SYM_OK);
	rsvwd.add(".proc", PROC, SYM_OK);
	rsvwd.add(".globl", GLOBL, SYM_OK);
	rsvwd.add(".", DOT, SYM_OK);
	rsvwd.add(".abort", ABORT, SYM_OK);
	rsvwd.add(".enum", ENUM, SYM_OK);
	rsvwd.add(".near", NEAR, SYM_OK);
	rsvwd.add(".hex", DOHEX, SYM_OK);
	rsvwd.add(".bin", DOBIN, SYM_OK);
	rsvwd.add(".s19", DOS19, SYM_OK);
	rsvwd.add(".symtab", DOSYM, SYM_OK);
	rsvwd.add(".symlocal", DOLOCAL, SYM_OK);
	
	rsvwd.add("equ", EQU, SYM_OK);
	// some compatiblity with that /other/ assembler
	// flag usage and warn
	rsvwd.add("db", BYTE, SYM_FWD);
	rsvwd.add("dfb", BYTE, SYM_FWD);
	rsvwd.add("dfs", PSSKIP, SYM_FWD);
	rsvwd.add("org", ORG, SYM_FWD);
	rsvwd.add("dw", WORD, SYM_FWD);
	rsvwd.add("dfw", WORD, SYM_FWD);


#ifdef MCS48
	rsvwd.add("add", ADD, SYM_OK);
	rsvwd.add("addc", ADDC , SYM_OK);
	rsvwd.add("anl", ANL, SYM_OK);
	rsvwd.add("orl", ORL, SYM_OK);
	rsvwd.add("xrl", XRL, SYM_OK);
	rsvwd.add("inc", INC, SYM_OK);
	rsvwd.add("dec", DEC, SYM_OK);
	rsvwd.add("clr", CLR, SYM_OK);
	rsvwd.add("da", DAA, SYM_OK);
	rsvwd.add("swap", SWAP, SYM_OK);
	rsvwd.add("rl", RLA, SYM_OK);
	rsvwd.add("rlc", RLCA, SYM_OK);
	rsvwd.add("rr", RRA, SYM_OK);
	rsvwd.add("rrc", RRCA, SYM_OK);
	rsvwd.add("in", INA, SYM_OK);
	rsvwd.add("ins", INS, SYM_OK);
	rsvwd.add("outl", OUTL, SYM_OK);
	rsvwd.add("bus", BUS, SYM_OK);
	rsvwd.add("movd", MOVD, SYM_OK);
	rsvwd.add("jmp", JMP, SYM_OK);
	rsvwd.add("jmpp", JMPP, SYM_OK);
	rsvwd.add("dnjz", DJNZ, SYM_OK);
	rsvwd.add("jc", JC, SYM_OK);
	rsvwd.add("jnc", JNC, SYM_OK);
	rsvwd.add("jz", JZ, SYM_OK);
	rsvwd.add("jnz", JNZ, SYM_OK);
	rsvwd.add("jt0", JT0, SYM_OK);
	rsvwd.add("jnt0", JNT0, SYM_OK);
	rsvwd.add("jt1", JT1, SYM_OK);
	rsvwd.add("jnt1", JNT1, SYM_OK);
	rsvwd.add("jf0", JF0, SYM_OK);
	rsvwd.add("jf1", JF1, SYM_OK);
	rsvwd.add("jtf", JTF, SYM_OK);
	rsvwd.add("jni", JNI, SYM_OK);
	rsvwd.add("jb0", JB0, SYM_OK);
	rsvwd.add("jb1", JB1, SYM_OK);
	rsvwd.add("jb2", JB2, SYM_OK);
	rsvwd.add("jb3", JB3, SYM_OK);
	rsvwd.add("jb4", JB4, SYM_OK);
	rsvwd.add("jb5", JB5, SYM_OK);
	rsvwd.add("jb6", JB6, SYM_OK);
	rsvwd.add("jb7", JB7, SYM_OK);
	rsvwd.add("call", CALL, SYM_OK);
	rsvwd.add("ret", RET, SYM_OK);
	rsvwd.add("retr", RETR, SYM_OK);
	rsvwd.add("cpl", CPL, SYM_OK);
	rsvwd.add("mov", MOV, SYM_OK);
	rsvwd.add("xch", XCH, SYM_OK);
	rsvwd.add("xchd", XCHD, SYM_OK);
	rsvwd.add("movx", MOVX, SYM_OK);
	rsvwd.add("movp", MOVP, SYM_OK);
	rsvwd.add("movp3", MOVP3, SYM_OK);
	rsvwd.add("start", STRT, SYM_OK);
	rsvwd.add("strt", STRT, SYM_OK);
	rsvwd.add("stop", STOP, SYM_OK);
	rsvwd.add("dis", DIS, SYM_OK);
	rsvwd.add("en", ENABLE, SYM_OK);
	rsvwd.add("nop", NOP, SYM_OK);
	rsvwd.add("a", AAA, SYM_OK);
	rsvwd.add("t", TTT, SYM_OK);
	rsvwd.add("i", III, SYM_OK);
	rsvwd.add("c", CCC, SYM_OK);
	rsvwd.add("tcnt", TCNT, SYM_OK);
	rsvwd.add("tcnti", TCNTI, SYM_OK);
	rsvwd.add("cnt", CNT, SYM_OK);
	rsvwd.add("rb0", RB0, SYM_OK);
	rsvwd.add("rb1", RB1, SYM_OK);
	rsvwd.add("mb0", MB0, SYM_OK);
	rsvwd.add("mb1", MB1, SYM_OK);
	rsvwd.add("clk", CLK, SYM_OK);
	rsvwd.add("ent0", ENT0, SYM_OK);
	rsvwd.add("r0", R0, SYM_OK);
	rsvwd.add("r1", R1, SYM_OK);
	rsvwd.add("r2", R2, SYM_OK);
	rsvwd.add("r3", R3, SYM_OK);
	rsvwd.add("r4", R4, SYM_OK);
	rsvwd.add("r5", R5, SYM_OK);
	rsvwd.add("r6", R6, SYM_OK);
	rsvwd.add("r7", R7, SYM_OK);
	rsvwd.add("f0", F0, SYM_OK);
	rsvwd.add("f1", F1, SYM_OK);
	rsvwd.add("p1", P1, SYM_OK);
	rsvwd.add("p2", P2, SYM_OK);
	// rsvwd.add("p3", P3, SYM_OK); // there is NO P3 !!!!
	rsvwd.add("p4", P4, SYM_OK);
	rsvwd.add("p5", P5, SYM_OK);
	rsvwd.add("p6", P6, SYM_OK);
	rsvwd.add("p7", P7, SYM_OK);
	rsvwd.add("psw", PSW, SYM_OK);
	rsvwd.add("orld", ORLD, SYM_OK);
	rsvwd.add("anld", ANLD, SYM_OK);
	rsvwd.add("sel", SEL, SYM_OK);
#endif
#ifdef MCS96
	rsvwd.add("add", ADD, SYM_OK);
	rsvwd.add("addb", ADDB, SYM_OK);
	rsvwd.add("addc", ADDC, SYM_OK);
	rsvwd.add("addcb", ADDCB, SYM_OK);
	rsvwd.add("sub", SUB, SYM_OK);
	rsvwd.add("subb", SUBB, SYM_OK);
	rsvwd.add("subc", SUBC, SYM_OK);
	rsvwd.add("subcb", SUBCB, SYM_OK);
	rsvwd.add("cmp", CMP, SYM_OK);
	rsvwd.add("cmpb", CMPB, SYM_OK);
	rsvwd.add("mul", MUL, SYM_OK);
	rsvwd.add("mulb", MULB, SYM_OK);
	rsvwd.add("mulu", MULU, SYM_OK);
	rsvwd.add("mulub", MULUB, SYM_OK);
	rsvwd.add("div", DIV, SYM_OK);
	rsvwd.add("divb", DIVB, SYM_OK);
	rsvwd.add("divu", DIVU, SYM_OK);
	rsvwd.add("divub", DIVUB, SYM_OK);
	rsvwd.add("and", AND, SYM_OK);
	rsvwd.add("andb", ANDB, SYM_OK);
	rsvwd.add("or", OR, SYM_OK);
	rsvwd.add("orb", ORB, SYM_OK);
	rsvwd.add("xor", XOR, SYM_OK);
	rsvwd.add("xorb", XORB, SYM_OK);
	rsvwd.add("ld", LD, SYM_OK);
	rsvwd.add("ldb", LDB, SYM_OK);
	rsvwd.add("st", ST, SYM_OK);
	rsvwd.add("stb", STB, SYM_OK);
	rsvwd.add("ldbse", LDBSE, SYM_OK);
	rsvwd.add("ldbze", LDBZE, SYM_OK);
	rsvwd.add("push", PUSH, SYM_OK);
	rsvwd.add("pop", POP, SYM_OK);
	rsvwd.add("pushf", PUSHF, SYM_OK);
	rsvwd.add("popf", POPF, SYM_OK);
	rsvwd.add("sjmp", SJMP, SYM_OK);
	rsvwd.add("ljmp", LJMP, SYM_OK);
	rsvwd.add("jmp", JMP, SYM_OK);
	rsvwd.add("scall", SCALL, SYM_OK);
	rsvwd.add("lcall", LCALL, SYM_OK);
	rsvwd.add("call", CALL, SYM_OK);
	rsvwd.add("ret", RET, SYM_OK);
	rsvwd.add("jc", JC, SYM_OK);
	rsvwd.add("jnc", JNC, SYM_OK);
	rsvwd.add("je", JE, SYM_OK);
	rsvwd.add("jne", JNE, SYM_OK);
	rsvwd.add("jge", JGE, SYM_OK);
	rsvwd.add("jlt", JLT, SYM_OK);
	rsvwd.add("jgt", JGT, SYM_OK);
	rsvwd.add("jle", JLE, SYM_OK);
	rsvwd.add("jh", JH, SYM_OK);
	rsvwd.add("jnh", JNH, SYM_OK);
	rsvwd.add("jv", JV, SYM_OK);
	rsvwd.add("jnv", JNV, SYM_OK);
	rsvwd.add("jnvt", JNVT, SYM_OK);
	rsvwd.add("jst", JST, SYM_OK);
	rsvwd.add("jnst", JNST, SYM_OK);
	rsvwd.add("jbs", JBS, SYM_OK);
	rsvwd.add("jbc", JBC, SYM_OK);
	rsvwd.add("djnz", DJNZ, SYM_OK);
	rsvwd.add("djnzw", DJNZW, SYM_OK);
	rsvwd.add("dec", DEC, SYM_OK);
	rsvwd.add("decb", DECB, SYM_OK);
	rsvwd.add("inc", INC, SYM_OK);
	rsvwd.add("incb", INCB, SYM_OK);
	rsvwd.add("neg", NEG, SYM_OK);
	rsvwd.add("negb", NEGB, SYM_OK);
	rsvwd.add("ext", EXT, SYM_OK);
	rsvwd.add("extb", EXTB, SYM_OK);
	rsvwd.add("not", NOT, SYM_OK);
	rsvwd.add("notb", NOTB, SYM_OK);
	rsvwd.add("clr", CLR, SYM_OK);
	rsvwd.add("clrb", CLRB, SYM_OK);
	rsvwd.add("shl", SHL, SYM_OK);
	rsvwd.add("shlb", SHLB, SYM_OK);
	rsvwd.add("shll", SHLL, SYM_OK);
	rsvwd.add("shr", SHR, SYM_OK);
	rsvwd.add("shrb", SHRB, SYM_OK);
	rsvwd.add("shrl", SHRL, SYM_OK);
	rsvwd.add("shra", SHRA, SYM_OK);
	rsvwd.add("shrab", SHRAB, SYM_OK);
	rsvwd.add("shral", SHRAL, SYM_OK);
	rsvwd.add("setc", SETC, SYM_OK);
	rsvwd.add("clrc", CLRC, SYM_OK);
	rsvwd.add("clrvt", CLRVT, SYM_OK);
	rsvwd.add("rst", RST, SYM_OK);
	rsvwd.add("di", DI, SYM_OK);
	rsvwd.add("ei", EI, SYM_OK);
	rsvwd.add("nop", NOP, SYM_OK);
	rsvwd.add("skip", SKIP, SYM_OK);
	rsvwd.add("norml", NORML, SYM_OK);
	rsvwd.add("trap", TRAP, SYM_OK);
	rsvwd.add("pusha", PUSHA, SYM_OK);
	rsvwd.add("popa", POPA, SYM_OK);
	rsvwd.add("iplpd", IPLPD, SYM_OK);
	rsvwd.add("cmpl", CMPL, SYM_OK);
	rsvwd.add("bmov", BMOV, SYM_OK);
	rsvwd.add("br", BR, SYM_OK);

#endif

}











	
