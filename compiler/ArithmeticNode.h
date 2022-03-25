#pragma once

#include <map>
#include "antlr4-runtime.h"
#include "SymbolTable.h"
#include <string>
#include <iostream>
#include <sstream>

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
	virtual void writeASM(SymbolTable *s, std::string dest, std::ostream &o) const {};
};

template <typename T>
class ConstNode : public ArithmeticNode<T>
{

private:
	T value;

public:
	T eval() const override { return value; }
	void writeASM(SymbolTable *s, std::string dest, std::ostream &o) const override
	{
		std::string sDest = dest[0] == '%' ? dest : this->oftos(s->getOffset(dest, true));

		o << "	movl	$" << value << ", " << sDest << std::endl;
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
	void writeASM(SymbolTable *s, std::string dest, std::ostream &o) const override
	{

		std::string sSrc = this->oftos(s->getOffset(name));
		if (dest.at(0) == '%')
		{
			o << "	movl	" << sSrc << ", " << dest << std::endl;
		}
		else
		{
			o << "	movl	" << sSrc << ", %eax" << std::endl;
			o << "	movl	%eax, " << this->oftos(s->getOffset(dest, true)) << std::endl;
		}
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
	void writeASM(SymbolTable *s, std::string dest, std::ostream &o) const override
	{
		std::string sDest = dest[0] == '%' ? dest : this->oftos(s->getOffset(dest, true));

		if (op == "+")
		{
			ArithmeticNode<T>::left->writeASM(s, sDest, o);
		}
		else if (op == "-")
		{
			ArithmeticNode<T>::left->writeASM(s, sDest, o);
			o << "	negl	" << sDest << std::endl;
		}
		else if (op == "!")
		{

			ArithmeticNode<T>::left->writeASM(s, "%eax", o);
			o << "	cmpl	$0, %eax" << std::endl;
			o << "	sete	%al\n";
			o << "	movzbl	%al, %eax\n";
			if (sDest != "%eax")
			{
				o << "	movl	%eax, " << sDest << std::endl;
			}
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
	void writeASM(SymbolTable *s, std::string dest, std::ostream &o) const override
	{

		std::string sDest = dest[0] == '%' ? dest : this->oftos(s->getOffset(dest, true));
		std::map<std::string, std::string> ops = {
			{"+", "addl"},
			{"-", "subl"},
			{"*", "imull"},
			{"/", "idivl"},
			{"%", "idivl"},
			{"==", "e"},
			{"!=", "ne"},
			{"<", "l"},
			{"<=", "le"},
			{">", "g"},
			{">=", "ge"},
			{"&&", "andl"},
			{"||", "orl"},
			{"|", "orl"},
			{"&", "andl"},
			{"^", "xorl"},
		};
		std::string keyWord = ops[op];
		std::string rName;
		size_t rOffset;
		std::string asmName;
		std::cerr << "BinaryNode::writeASM-> " << op << " " << (int)this->right->type() << std::endl;
		if (this->right->type() == Type::VAR)
		{
			rName = dynamic_cast<VarNode<int> *>(this->right)->getName();
			rOffset = s->getOffset(rName);
			asmName = this->oftos(rOffset);
		}
		else if (this->right->type() == Type::CONST)
		{

			asmName = "$" + std::to_string(this->right->eval());
			std::cerr << this->right->eval() << std::endl;
		}
		else
		{
			rName = this->ptos(this->right);
			std::cerr << "rName: " << rName << std::endl;

			rOffset = s->addTemp(
				rName,
				4);
			this->right->writeASM(s, rName, o);
			asmName = this->oftos(rOffset);
		}
		std::cerr << "asmName: " << asmName << std::endl;

		this->left->writeASM(s, "%eax", o);

		if (op == "+" || op == "-" || op == "*" || op == "|" || op == "^" || op == "&")
		{
			o << "	" << keyWord << "	" << asmName << ", %eax" << std::endl;
			if (dest != "%eax")
				o << "	movl	%eax, " << sDest << std::endl;
		}
		else if (op == "%" || op == "/")
		{
			if (asmName[0] == '$')
			{
				o << "	movl	" << asmName << ", %ecx" << std::endl;
				asmName = "%ecx";
			}

			o << "	cltd" << std::endl;
			o << "	" << keyWord << "	" << asmName << std::endl;
			std::string src = op == "%" ? "%edx" : "%eax";
			if (src != sDest)
			{
				o << "	movl	" << src << ", " << sDest << std::endl;
			}
		}
		else if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
		{
			keyWord = "set" + keyWord;
			o << "	cmpl	" << asmName << ", %eax" << std::endl;
			o << "	" << keyWord << "	%al" << std::endl;
			o << "	movzbl	%al, %eax" << std::endl;
			if (dest != "%eax")
				o << "	movl	%eax, " << sDest << std::endl;
		}
		else if (op == "&&" || op == "||")
		{
			// TODO: Find a way to do this more efficiently
			// left in %eax
			// asmName right
			int v = op == "&&" ? 1 : 2;
			std::cout << "	cmpl $0, %eax\n";
			std::cout << "	sete %dl\n";

			std::cout << "	movl	" << asmName << ", %eax\n";
			std::cout << "	cmpl $0, %eax\n";
			std::cout << "	sete %dh\n";

			std::cout << "	add %dh, %dl\n";
			std::cout << "	movzb %dl, %eax\n";
			std::cout << "	cmpl $" << v << ", %eax\n";
			std::cout << "	setl %al\n";

			std::cout << "	movzb %al, %eax\n";
			if (dest != "%eax")
				o << "	movl	%eax, " << sDest << std::endl;
		}
	}

	BinaryNode(std::string o, ArithmeticNode<T> *l, ArithmeticNode<T> *r) : ArithmeticNode<int>(l, r), op(o){};
};
