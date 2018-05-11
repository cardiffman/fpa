/*
 * fpnode.cpp
 *
 *  Created on: May 10, 2018
 *      Author: Mike
 */
#include "fpnode.h"
#include "fptoken.h"
#include "fputil.h"

#include <iostream>
#include <list>
#include <algorithm>

using namespace std;


ostream& operator<<(ostream& os, const Node& expr) {
	return os << expr.to_string();
}

string ts(const shared_ptr<Node>& n) { return n->to_string(); }
std::string NodeApply::to_string() const {
	return '(' + join(mapf(terms.begin(), terms.end(), [](auto const& n) { return n->to_string(); }),',') + ')';
	//return '(' + join(mapf(terms.begin(), terms.end(), ts),',') + ')';
}

shared_ptr<Node>
Identifier(const string& id)
{
	auto node = make_shared<NodeId>();
	node->id = id;
	return node;
}
shared_ptr<Node>
Number(int value)
{
	auto node = make_shared<NodeNum>();
	node->value = value;
	return node;
}
shared_ptr<Node>
Abstract(const string& id, const shared_ptr<Node>& body_expr)
{
	auto node = make_shared<NodeAbstract>();
	node->id = id;
	node->expr = body_expr;
	return node;
}
shared_ptr<Node>
Application(const shared_ptr<Node>& left_side, const shared_ptr<Node>& right_side)
{
	auto node = make_shared<NodeApply>();
	node->terms.push_back(left_side);
	node->terms.push_back(right_side);
	return node;
}
shared_ptr<Node>
parseExpr6(Token& token)
{
	list<shared_ptr<Node>> terms;
	while (true)
	{
		if (token.type == TT_SEMI||token.type == TT_RPAREN || token.type == TT_IN || token.type == TT_EOF)
			break;
		if (token.type == TT_LAMBDA)
		{
			token.next();
			if (token.type != TT_ID && token.type != TT_OP)
				throw "ID or OPerator token expected after lambda";
			string id = token.text;
			token.next();
			if (token.type != TT_DOT)
				throw "lambda requires dot between parameter and body";
			token.next();
			auto body = parseExpression(token);
			auto abs = Abstract(id, body);
			terms.push_back(abs);
		}
		else if (token.type == TT_LPAREN)
		{
			token.next();
			auto inside = parseExpression(token);
			if (token.type != TT_RPAREN)
				throw "Mismatched left paren";
			token.next();
			terms.push_back(inside);
		}
#if 0
		else if (token.type == TT_OP)
		{
			terms.push_back(Identifier(token.text));
			token.next();
		}
#else
		else if (token.type == TT_OP)
		{
			break;
		}
#endif
		else if (token.type == TT_ID)
		{
			terms.push_back(Identifier(token.text));
			token.next();
		}
		else if (token.type == TT_NUM)
		{
			terms.push_back(Number(token.value));
			token.next();
		}
		else
		{
			for (auto t: terms)
				cout << '['<< t->to_string() << "] ";
			cout << endl;
			throw "Illegal token " + token.to_string();
		}
	}
	//if (token.type == TT_SEMI)
	//	token.next();
	if (terms.size()==1)
		return terms.front();
#if 0
	cout << "Generating application nodes for " << terms.size() << " terms" << endl;
	for (auto t: terms)
		cout << '['<< t->to_string() << "] ";
	cout << endl;
#endif
	auto call = terms.front(); terms.pop_front();
	while (terms.size()>0)
	{
		auto arg = terms.front(); terms.pop_front();
		auto application = Application(call, arg);
		call = application;
	}
	return call;
}
shared_ptr<Node>
parseExpr5(Token& token)
{
	auto expr = parseExpr6(token);
	if (token.type == TT_OP && token.text == "*") {
		auto op = token.text;
		token.next();
		auto expr2 = parseExpr5(token);
		expr = Application(Application(Identifier(op), expr), expr2);
	} else if (token.type == TT_OP && token.text == "/") {
		auto op = token.text;
		token.next();
		auto expr2 = parseExpr6(token);
		expr = Application(Application(Identifier(op), expr), expr2);
	}
	return expr;
}
shared_ptr<Node>
parseExpr4(Token& token)
{
	auto expr = parseExpr5(token);
	if (token.type == TT_OP && token.text == "+") {
		auto op = token.text;
		token.next();
		auto expr2 = parseExpr4(token);
		expr = Application(Application(Identifier(op), expr), expr2);
	} else if (token.type == TT_OP && token.text == "-") {
		auto op = token.text;
		token.next();
		auto expr2 = parseExpr5(token);
		expr = Application(Application(Identifier(op), expr), expr2);
	}
	return expr;
}
shared_ptr<Node>
parseExpr3(Token& token)
{
	auto expr = parseExpr4(token);
	if (token.type == TT_OP && (token.text == ">" || token.text=="<" || token.text=="==")) {
		auto op = token.text;
		token.next();
		auto expr2 = parseExpr4(token);
		expr = Application(Application(Identifier(op), expr), expr2);
	}
	return expr;
}
shared_ptr<Node>
parseExpr2(Token& token)
{
	auto expr = parseExpr3(token);
	if (token.type == TT_OP && token.text == "&") {
		auto op = token.text;
		token.next();
		auto expr2 = parseExpr2(token);
		expr = Application(Application(Identifier(op), expr), expr2);
	}
	return expr;
}
shared_ptr<Node>
parseExpr1(Token& token)
{
	auto expr = parseExpr2(token);
	if (token.type == TT_OP && token.text == "|") {
		auto op = token.text;
		token.next();
		auto expr2 = parseExpr1(token);
		expr = Application(Application(Identifier(op), expr), expr2);
	}
	return expr;
}
shared_ptr<Node>
parseExpression(Token& token)
{
	return parseExpr1(token);
}


