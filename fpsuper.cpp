/*
 * fpsuper.cpp
 *
 *  Created on: May 10, 2018
 *      Author: Mike
 */

#include "fpsuper.h"
#include "fptoken.h"
#include "fpnode.h"

using namespace std;

CoreScDefn
Definition(const string& name, const list<string>& args, const shared_ptr<Node>& expr)
{
	CoreScDefn def;
	def.args = args;
	def.expr = expr;
	def.name = name;
	return def;
}

list<CoreScDefn>
Definitions(Token& token)
{
	list<CoreScDefn> definitions;

	while (token.type != TT_EOF)
	{
		if (token.type != TT_ID && token.type != TT_OP)
			throw "identifier required not " + token.to_string();
		auto id = token.text;
		token.next();
		list<string> args;
		while (token.type == TT_ID) {
			args.push_back(token.text);
			token.next();
		}
		if (token.type != TT_EQUALS)
			throw "Equals required";
		token.next();
		auto body = parseExpression(token);
		if (token.type == TT_SEMI)
			token.next();
		definitions.push_back(Definition(id, args, body));
	}
	return definitions;
}

#if 0
pair<list<shared_ptr<Node>>,shared_ptr<Node>>
Let(Token& token)
{
	list<shared_ptr<Node>> definitions;

	while (true)
	{
		shared_ptr<Node> definition;
		if (token.type != TT_ID && token.type != TT_OP)
			throw "identifier required";
		auto id = token.text;
		token.next();
		if (token.type != TT_EQUALS)
			throw "Equals required";
		token.next();
		auto body = parseExpression(token);
		if (token.type == TT_SEMI)
			token.next();
		definition = Definition(id, body);
		definitions.push_back(definition);
	}
	return definitions;
}
#endif

