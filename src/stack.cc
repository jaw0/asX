

// $Id: stack.cc,v 1.4 1993/05/28 04:40:51 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#include <as.h>
#include <stack.h>

void Stack::push(char c){
	Stack *cur = new Stack;
	cur->ch = c;
	cur->next = next;
	next = cur;
}

int Stack::pop(void){
	int c;
	Stack *s;
	
	if (next){
		c = next->ch;
		s = next;
		next = next->next;
		delete s;
	}else{
		c = -1;
	}
	return c;
}

