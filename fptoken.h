/*
 * fptoken.h
 *
 *  Created on: May 10, 2018
 *      Author: Mike
 */

#ifndef FPTOKEN_H_
#define FPTOKEN_H_
#include <string>
#include <iostream>

enum TokenType {
	TT_SEMI=';', TT_LPAREN='(', TT_RPAREN=')', TT_LAMBDA='\\', TT_DOT='.', TT_EQUALS='=',
	TT_LET, TT_IN,
	TT_ID, TT_NUM, TT_OP, TT_EOF
};
struct Token {
	Token(std::istream& input) : file(input) { value=0; type=TT_EOF;}
	TokenType type;
	std::string text;
	int value;
	std::istream& file;
	bool next();
	std::string to_string() const;
};


#endif /* FPTOKEN_H_ */
