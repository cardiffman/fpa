/*
 * fpsuper.h
 *
 *  Created on: Mar 27, 2018
 *      Author: menright
 */

#ifndef FPSUPER_H_
#define FPSUPER_H_
#include <list>

#include "fpnode.h"

struct CoreScDefn {
	std::string name;
	std::list<std::string> args;
	std::shared_ptr<Node> expr;
	std::string to_string() const;
};
struct Token;
std::list<CoreScDefn> Definitions(Token& token);




#endif /* FPSUPER_H_ */
