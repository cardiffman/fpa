/*
 * fpsuper.h
 *
 *  Created on: May 10, 2018
 *      Author: Mike
 */

#ifndef FPSUPER_H_
#define FPSUPER_H_

#include <list>
#include <memory>
#include <string>
#include "fpnode.h"
struct Token;
struct Node;

struct CoreScDefn {
	std::string name;
	std::list<std::string> args;
	std::shared_ptr<Node> expr;
};

std::list<CoreScDefn> Definitions(Token& token);

//CoreScDefn
//Definition(const std::string& name, const std::shared_ptr<Node>& expr);

#endif /* FPSUPER_H_ */
