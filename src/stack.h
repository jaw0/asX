

// $Id: stack.h,v 1.3 1993/05/28 04:40:51 jaw Exp $

// Copyright (c) 1993 Jeff Weisberg
// see the file "License"

#ifndef _STACK_H
#define _STACK_H

class Stack {
	// an implentation of a stack
  private:
	int   ch;
	Stack *next;
  public:
	Stack(){ch=-1;next=0;}
	~Stack(){}
	void push(char c);
	int  pop(void);
};


#endif !_STACK_H
