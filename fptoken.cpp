/*
 * fptoken.cpp
 *
 *  Created on: May 10, 2018
 *      Author: Mike
 */

#include "fptoken.h"
#include <iostream>
using namespace std;

bool Token::next()
{
	text.clear();
	int ch = file.get();
	if (ch == -1)
	{
		type = TT_EOF;
		return false;
	}
	while (true)
	{
		while (ch==' ' || ch=='\t' || ch=='\r' || ch=='\n')
			ch = file.get();
		if (ch == -1)
			break;
		if (ch != '#')
			break;
		while (ch!='\n' && file.good())
		{
			ch = file.get();
		}
	}
	if (isalpha(ch) || ch=='_')
	{
		text.push_back(ch);
		ch = file.get();
		while (file.good() && (ch == '_' || isalpha(ch) || isdigit(ch)))
		{
			text.push_back(ch);
			ch = file.get();
		}
		file.unget();
		if (text == "in")
			type = TT_IN;
		else if (text == "let")
			type = TT_LET;
		else
			type = TT_ID;
		return true;
	}
	if (isdigit(ch))
	{
		value = ch-'0';
		ch = file.get();
		while (file.good() && (isdigit(ch)))
		{
			value = value*10+ch-'0';
			ch = file.get();
		}
		file.unget();
		type = TT_NUM;
		return true;
	}
	if (ch=='(' || ch==')' || ch==';')
	{
		type = (TokenType)ch;
		return true;
	}
	if (ch == EOF)
	{
		type = TT_EOF;
		return false;
	}
	while (ispunct(ch) && ch!='#' && ch != ')' && ch !=')' && ch!=';')
	{
		text.push_back(ch);
		ch = file.get();
	}
	file.unget();
	if (text=="\\")
		type = TT_LAMBDA;
	else if (text==".")
		type = TT_DOT;
	else if (text=="=")
		type = TT_EQUALS;
	else
		type = TT_OP;
	return true;
}

string Token::to_string() const {
	switch (type)
	{
	case TT_ID:
	case TT_OP:
		return text;
	case TT_NUM:
		return std::to_string(value);
	case TT_LPAREN:
	case TT_RPAREN:
	case TT_SEMI:
	case TT_LAMBDA:
	case TT_DOT:
	case TT_EQUALS:
		return string(1, (char)type);
	case TT_IN:
		return "in";
	case TT_LET:
		return "let";
	case TT_EOF:
		return "EOF";
	}
	return "";
}

