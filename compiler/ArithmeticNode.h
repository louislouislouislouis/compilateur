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
class ArithmeticNode
{
protected:
	// Left and right children
	ArithmeticNode *left;
	ArithmeticNode *right;
	uint size;
	long cast(long val) const
	{
		// std::cout<<"size="<<size<<" val="<<val<<"\n";
		if (size == 1)
		{
			return (char)val;
		}
		else if (size == 4)
		{
			return (int)val;
		}
		else
		{
			return val;
		}
	}

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
	ArithmeticNode(ArithmeticNode *l, ArithmeticNode *r, uint sizeP = 8) : left(l), right(r), size(sizeP){};
	ArithmeticNode() : left(nullptr), right(nullptr), size(8){};
	ArithmeticNode(uint size) : left(nullptr), right(nullptr), size(size){};
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
	virtual long eval() const { return 0; };

	/*
	 * Writes the code for the node to the output stream
	 * @param s The symbol table
	 * @param dest the destination variable
	 * @param o The output stream
	 */
	virtual void generate(CFG *cfg, std::string dest){};
};

class ConstNode : public ArithmeticNode
{

private:
	long value;

public:
	long eval() const override { return cast(value); }
	void generate(CFG *cfg, std::string dest) override
	{

		// std::string sDest = dest[0] == '%' ? dest : this->oftos(cfg->getST()->getOffset(dest, true));

		// o << "	movl	$" << value << ", " << sDest << std::endl;
		this->size = cfg->getST()->getSize(dest);
		long value = cast(this->value);
		cfg->current_bb->add_IRInstr(IRInstr::Operation::ldconst, std::vector<std::string>{dest, std::to_string(value)});
	}
	ConstNode(long v) : value(v){};
};

class VarNode : public ArithmeticNode
{
private:
	std::string name;

public:
	Type type() const override { return Type::VAR; }
	void generate(CFG *cfg, std::string dest) override
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
	VarNode(std::string n, uint size) : name(n), ArithmeticNode(size){};
};

class UnaryNode : public ArithmeticNode
{
private:
	std::string op;

public:
	long eval() const override
	{
		if (op == "+")
			return cast(ArithmeticNode::left->eval());
		else if (op == "-")
			return cast(-ArithmeticNode::left->eval());
		else if (op == "!")
			return cast(!ArithmeticNode::left->eval());
		else if (op == "~")
			return cast(~ArithmeticNode::left->eval());
		return 0;
	}
	void generate(CFG *cfg, std::string dest) override
	{
		// std::string sDest = dest[0] == '%' ? dest : this->oftos(cfg->getST()->getOffset(dest, true));

		if (op == "+")
		{
			ArithmeticNode::left->generate(cfg, dest);
		}
		else if (op == "-")
		{
			ArithmeticNode::left->generate(cfg, dest);
			cfg->current_bb->add_IRInstr(IRInstr::Operation::neg, dest);
			// o << "	negl	" << sDest << std::endl;
		}
		else if (op == "!")
		{

			ArithmeticNode::left->generate(cfg, dest);
			cfg->current_bb->add_IRInstr(IRInstr::Operation::not_, dest);
			// o << "	cmpl	$0, %eax" << std::endl;
			// o << "	sete	%al\n";
			// o << "	movzbl	%al, %eax\n";
			// if (sDest != "%eax")
			// {
			// 	o << "	movl	%eax, " << sDest << std::endl;
			// }
		}
		else if (op == "~")
		{
			ArithmeticNode::left->generate(cfg, dest);
			cfg->current_bb->add_IRInstr(IRInstr::Operation::bnot, dest);
			// o << "	notl	" << sDest << std::endl;
		}
		else
		{
			std::cout << "Unary operator not supported" << std::endl;
		}
	}
	UnaryNode(std::string op, ArithmeticNode *o) : ArithmeticNode(o, nullptr), op(op){};
};

class BinaryNode : public ArithmeticNode
{
private:
	std::string op;

public:
	long eval() const override
	{
		long l = ArithmeticNode::left->eval(), r = ArithmeticNode::right->eval();

		if (op == "+")
			return cast(l + r);
		else if (op == "-")
			return cast(l - r);
		else if (op == "*")
			return cast(l * r);
		else if (op == "/")
			return cast(l / r);
		else if (op == "%")
			return cast(l % r);
		else if (op == "<")
			return cast(l < r);
		else if (op == ">")
			return cast(l > r);
		else if (op == "==")
			return cast(l == r);
		else if (op == "!=")
			return cast(l != r);
		else if (op == "<=")
			return cast(l <= r);
		else if (op == ">=")
			return cast(l >= r);
		else if (op == "&&")
			return cast(l && r);
		else if (op == "||")
			return cast(l || r);
		else if (op == "&")
			return cast(l & r);
		else if (op == "|")
			return cast(l | r);
		else if (op == "^")
			return cast(l ^ r);

		else
			return 0;
	}
	void generate(CFG *cfg, std::string dest) override
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
			{"<<", IRInstr::Operation::shiftL},
			{">>", IRInstr::Operation::shiftR},
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
			rName = dynamic_cast<VarNode *>(this->right)->getName();
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
			lName = dynamic_cast<VarNode *>(this->left)->getName();
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

	BinaryNode(std::string o, ArithmeticNode *l, ArithmeticNode *r) : ArithmeticNode(l, r), op(o){};
};
