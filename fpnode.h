/*
 * fpnode.h
 *
 *  Created on: May 10, 2018
 *      Author: Mike
 */
#ifndef INCLUDED_FPNODE_H
#define INCLUDED_FPNODE_H
#include <memory>
#include <string>
#include <vector>

struct Node;
struct NodeAbstract;
struct NodeApply;
struct NodeDefine;
struct NodeLet;
struct NodeId;
struct NodeNum;
struct NodeConstructor;
struct NodeCase;
struct NodeVisitor;

struct Node {
	virtual ~Node() {}
	virtual std::string to_string() const = 0;
	virtual void acceptVisit(NodeVisitor*) = 0;
};
struct NodeVisitor {
	virtual ~NodeVisitor() {}
	virtual void visit(NodeDefine*) = 0;
	virtual void visit(NodeAbstract*) = 0;
	virtual void visit(NodeLet*) = 0;
	virtual void visit(NodeConstructor*) = 0;
	virtual void visit(NodeCase*) = 0;
	virtual void visit(NodeApply* expr) = 0;
	virtual void visit(NodeId* expr)  = 0;
	virtual void visit(NodeNum* expr) = 0;
};
struct NodeAbstract : public Node {
	virtual std::string to_string() const {
		return '\\' + id + '.' + expr->to_string();
	}
	void acceptVisit(NodeVisitor* v) { v->visit(this); };
	std::string id;
	std::shared_ptr<Node> expr;
};
struct NodeApply : public Node {
	virtual std::string to_string() const;
	virtual void acceptVisit(NodeVisitor* v) { v->visit(this); };
	std::vector<std::shared_ptr<Node>> terms;
};
struct NodeDefine : public Node {
	virtual std::string to_string() const {
		return "def " + name + "=" + expr->to_string();
	}
	virtual void acceptVisit(NodeVisitor* v) { v->visit(this); };
	std::string name;
	std::vector<std::string> args;
	std::shared_ptr<Node> expr;
};
struct NodeLet : public Node {
	virtual std::string to_string() const {
		return "";
	}
	virtual void acceptVisit(NodeVisitor* v) { v->visit(this); };
};
struct NodeId : public Node {
	virtual std::string to_string() const {
		return id;
	}
	virtual void acceptVisit(NodeVisitor* v) { v->visit(this); };
	std::string id;
};
struct NodeNum : public Node {
	virtual std::string to_string() const {
		return std::to_string(value);
	}
	virtual void acceptVisit(NodeVisitor* v) { v->visit(this); };
	int value;
};
struct NodeCase : public Node {
	virtual std::string to_string() const {
		return "";
	}
	virtual void acceptVisit(NodeVisitor* v) { v->visit(this); };
};
struct NodeConstructor : public Node {
	virtual std::string to_string() const {
		return "";
	}
	virtual void acceptVisit(NodeVisitor* v) { v->visit(this); };
};

std::ostream& operator<<(std::ostream& os, const Node& expr);
std::shared_ptr<Node> Identifier(const std::string& id);
std::shared_ptr<Node> Number(int value);

struct Token;
std::shared_ptr<Node>
parseExpression(Token& token);

#endif
