#pragma once

#include <map>
#include "antlr4-runtime.h"
#include "SymbolTable.h"
#include <string>
#include <iostream>
#include <sstream>
#include "IR.h"

// Node type definitions
enum class Type
{
	CONST,
	VAR,
	COMP
};

/*
 * This class defines a node in the abstract expression tree.
 * It contains a type, two children, and can be evaluated.
 * It is used to generate optimized code.
 */
template <typename T>
class ArithmeticNode
{
protected:
	// Left and right children
	ArithmeticNode<T> *left;
	ArithmeticNode<T> *right;

	// Helper functions
	static std::string ptos(const void *p)
	{
		std::ostringstream ss;
		ss << p;
		return ss.str();
	}
	static std::string oftos(size_t i)
	{
		std::ostringstream ss;
		ss << "-" << i << "(%rbp)";
		return ss.str();
	}

public:
	ArithmeticNode(ArithmeticNode<T> *l, ArithmeticNode<T> *r) : left(l), right(r){};
	ArithmeticNode() : left(nullptr), right(nullptr){};
	~ArithmeticNode()
	{
		if (left != nullptr)
			delete left;
		if (right != nullptr)
			delete right;
	};

	/*
	 * Returns the type of the node (Check if CONST)
	 */
	virtual Type type() const
	{
		if (left != nullptr && left->type() != Type::CONST)
			return Type::COMP;
		else if (right != nullptr && right->type() != Type::CONST)
			return Type::COMP;
		return Type::CONST;
	};
	/*
	 * Evaluates the node and returns the result
	 * Result is meaningful only if the node is of type CONST
	 */
	virtual T eval() const { return T(); };

	/*
	 * Writes the code for the node to the output stream
	 * @param s The symbol table
	 * @param dest the destination variable
	 * @param o The output stream
	 */
	virtual void generate(CFG *cfg, std::string dest) const {};
};

template <typename T>
class ConstNode : public ArithmeticNode<T>
{

private:
	T value;

public:
	T eval() const override { return value; }
	void generate(CFG *cfg, std::string dest) const override
	{

		// std::string sDest = dest[0] == '%' ? dest : this->oftos(cfg->getST()->getOffset(dest, true));

		// o << "	movl	$" << value << ", " << sDest << std::endl;
		cfg->current_bb->add_IRInstr(IRInstr::Operation::ldconst, std::vector{dest, std::to_string(value)});
	}
	ConstNode(T v) : value(v){};
};

template <typename T>
class VarNode : public ArithmeticNode<T>
{
private:
	std::string name;

public:
	Type type() const override { return Type::VAR; }
	void generate(CFG *cfg, std::string dest) const override
	{

		// std::string sSrc = this->oftos(cfg->getST()->getOffset(name));
		// if (dest[0] == '%')
		// cfg->current_bb->add_IRInstr(IRInstr::Operation::wmem, std::vector<std::string>{dest, name});
		// else
		cfg->current_bb->add_IRInstr(IRInstr::Operation::copy, std::vector<std::string>{dest, name});
		// if (dest.at(0) == '%')
		// {
		// 	o << "	movl	" << sSrc << ", " << dest << std::endl;
		// }
		// else
		// {
		// 	o << "	movl	" << sSrc << ", %eax" << std::endl;
		// 	o << "	movl	%eax, " << this->oftos(s->getOffset(dest, true)) << std::endl;
		// }
	};
	std::string getName() const { return name; };
	VarNode(std::string n) : name(n){};
};

template <typename T>
class UnaryNode : public ArithmeticNode<T>
{
private:
	std::string op;

public:
	T eval() const override
	{
		if (op == "+")
			return ArithmeticNode<T>::left->eval();
		else if (op == "-")
			return -ArithmeticNode<T>::left->eval();
		else if (op == "!")
			return !ArithmeticNode<T>::left->eval();
		else
			return T();
	}
	void generate(CFG *cfg, std::string dest) const override
	{
		// std::string sDest = dest[0] == '%' ? dest : this->oftos(cfg->getST()->getOffset(dest, true));

		if (op == "+")
		{
			ArithmeticNode<T>::left->generate(cfg, dest);
		}
		else if (op == "-")
		{
			ArithmeticNode<T>::left->generate(cfg, dest);
			cfg->current_bb->add_IRInstr(IRInstr::Operation::neg, dest);
			// o << "	negl	" << sDest << std::endl;
		}
		else if (op == "!")
		{

			ArithmeticNode<T>::left->generate(cfg, "%eax");
			cfg->current_bb->add_IRInstr(IRInstr::Operation::not_, dest);
			// o << "	cmpl	$0, %eax" << std::endl;
			// o << "	sete	%al\n";
			// o << "	movzbl	%al, %eax\n";
			// if (sDest != "%eax")
			// {
			// 	o << "	movl	%eax, " << sDest << std::endl;
			// }
		}
	}
	UnaryNode(std::string op, ArithmeticNode<T> *o) : ArithmeticNode<int>(o, nullptr), op(op){};
};

template <typename T>
class BinaryNode : public ArithmeticNode<T>
{
private:
	std::string op;

public:
	T eval() const override
	{
		T l = ArithmeticNode<T>::left->eval(), r = ArithmeticNode<T>::right->eval();

		if (op == "+")
			return l + r;
		else if (op == "-")
			return l - r;
		else if (op == "*")
			return l * r;
		else if (op == "/")
			return l / r;
		else if (op == "%")
			return l % r;
		else if (op == "<")
			return l < r;
		else if (op == ">")
			return l > r;
		else if (op == "==")
			return l == r;
		else if (op == "!=")
			return l != r;
		else if (op == "<=")
			return l <= r;
		else if (op == ">=")
			return l >= r;
		else if (op == "&&")
			return l && r;
		else if (op == "||")
			return l || r;
		else if (op == "&")
			return l & r;
		else if (op == "|")
			return l | r;
		else if (op == "^")
			return l ^ r;
		else
			return T();
	}
	void generate(CFG *cfg, std::string dest) const override
	{

		// std::string sDest = dest[0] == '%' ? dest : this->oftos(s->getOffset(dest, true));
		std::map<std::string, IRInstr::Operation> ops = {
			{"+", IRInstr::Operation::add},
			{"-", IRInstr::Operation::sub},
			{"*", IRInstr::Operation::mul},
			{"/", IRInstr::Operation::div},
			{"%", IRInstr::Operation::mod},
			{"<", IRInstr::Operation::lt},
			{">", IRInstr::Operation::gt},
			{"==", IRInstr::Operation::eq},
			{"!=", IRInstr::Operation::neq},
			{"<=", IRInstr::Operation::leq},
			{">=", IRInstr::Operation::geq},
			{"&&", IRInstr::Operation::and_},
			{"||", IRInstr::Operation::or_},
			{"&", IRInstr::Operation::band},
			{"|", IRInstr::Operation::bor},
			{"^", IRInstr::Operation::bxor},

		};
		auto operation = ops[op];
		std::string rName, lName;
		// std::cerr << "BinaryNode::generate-> " << op << " " << (int)this->right->type() << std::endl;
		if (this->right->type() == Type::VAR)
		{
			rName = dynamic_cast<VarNode<int> *>(this->right)->getName();
		}
		else if (this->right->type() == Type::CONST)
		{
			rName = "$" + std::to_string(this->right->eval());
		}
		else
		{
			rName = this->ptos(this->right);
			cfg->getST()->addTemp(
				rName,
				"int");
			this->right->generate(cfg, rName);
		}

		if (this->left->type() == Type::VAR)
		{
			lName = dynamic_cast<VarNode<int> *>(this->left)->getName();
		}
		else if (this->left->type() == Type::CONST)
		{
			lName = "$" + std::to_string(this->left->eval());
		}
		else
		{
			lName = this->ptos(this->left);
			cfg->getST()->addTemp(
				lName,
				"int");
			this->left->generate(cfg, lName);
		}

		cfg->current_bb->add_IRInstr(operation, {dest, lName, rName});
		// std::cerr << "asmName: " << asmName << std::endl;

		// this->left->generate(s, "%eax", o);

		// if (op == "+" || op == "-" || op == "*" || op == "|" || op == "^" || op == "&")
		// {
		// 	o << "	" << keyWord << "	" << asmName << ", %eax" << std::endl;
		// 	if (dest != "%eax")
		// 		o << "	movl	%eax, " << sDest << std::endl;
		// }
		// else if (op == "%" || op == "/")
		// {
		// 	if (asmName[0] == '$')
		// 	{
		// 		o << "	movl	" << asmName << ", %ecx" << std::endl;
		// 		asmName = "%ecx";
		// 	}

		// 	o << "	cltd" << std::endl;
		// 	o << "	" << keyWord << "	" << asmName << std::endl;
		// 	std::string src = op == "%" ? "%edx" : "%eax";
		// 	if (src != sDest)
		// 	{
		// 		o << "	movl	" << src << ", " << sDest << std::endl;
		// 	}
		// }
		// else if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
		// {
		// 	keyWord = "set" + keyWord;
		// 	o << "	cmpl	" << asmName << ", %eax" << std::endl;
		// 	o << "	" << keyWord << "	%al" << std::endl;
		// 	o << "	movzbl	%al, %eax" << std::endl;
		// 	if (dest != "%eax")
		// 		o << "	movl	%eax, " << sDest << std::endl;
		// }
		// else if (op == "&&" || op == "||")
		// {
		// 	// TODO: Find a way to do this more efficiently
		// 	// left in %eax
		// 	// asmName right
		// 	int v = op == "&&" ? 1 : 2;
		// 	std::cout << "	cmpl $0, %eax\n";
		// 	std::cout << "	sete %dl\n";

		// 	std::cout << "	movl	" << asmName << ", %eax\n";
		// 	std::cout << "	cmpl $0, %eax\n";
		// 	std::cout << "	sete %dh\n";

		// 	std::cout << "	add %dh, %dl\n";
		// 	std::cout << "	movzb %dl, %eax\n";
		// 	std::cout << "	cmpl $" << v << ", %eax\n";
		// 	std::cout << "	setl %al\n";

		// 	std::cout << "	movzb %al, %eax\n";
		// 	if (dest != "%eax")
		// 		o << "	movl	%eax, " << sDest << std::endl;
		// }
	}

	BinaryNode(std::string o, ArithmeticNode<T> *l, ArithmeticNode<T> *r) : ArithmeticNode<int>(l, r), op(o){};
};
