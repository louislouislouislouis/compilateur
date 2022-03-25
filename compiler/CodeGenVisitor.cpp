#include "CodeGenVisitor.h"
#include <string>

#ifdef __APPLE__
#define MAIN "_main"
#else
#define MAIN "main"
#endif
/*
Prints the basic layout into the .s file and starts the evaluation of the program by calling visitChildren(ctx).

int main(){
	...
}
.globl  main
main:
	push %rbp
	movq %rsp, %rbp
	...
	popq %rbp
	ret
 */
antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{
	// Generate the prologue
	std::cout << ".globl	" << MAIN << "\n"
			  << MAIN << ":\n	pushq %rbp\n	movq %rsp, %rbp\n\n";

	// Generate the body of the main function
	visitChildren(ctx);

	// Generate the epilogue
	std::cout << "\n	popq %rbp\n	ret\n";

	// Check for unused variables
	symbolTable.checkUse();

	return nullptr;
}

/*
evaluates an expression like
int var_1 = -9090;
int var_1 = var_2;
int var_1 = -9090, var_2 = var_3;
int var_1, var_2, var_3 = -90909;

remark when no variable is assigned to the varaible, the memoryspace can hold ANY value
*/
antlrcpp::Any CodeGenVisitor::visitSdecl(ifccParser::SdeclContext *ctx)
{
	// Get the name of the variable
	std::string dname = ctx->ID()->getText();

	// Get the type of the variable
	std::string dtype = ctx->parent->children[0]->getText();

	// Add the variable to the symbol table
	symbolTable.add(dname,
					4,
					{ctx->ID()->getSymbol()->getLine(), ctx->ID()->getSymbol()->getCharPositionInLine()});

	// If the variable is not assigned to anything, we can just return
	if (ctx->rval() != nullptr)
	{
		// Parse the right hand side of the expression
		auto root = visitChildren(ctx).as<ArithmeticNode<int> *>();

		// Generate the code for the right hand side
		root->writeASM(&symbolTable, dname, std::cout);

		// Free the memory allocated for the right hand side
		delete root;

		// Clear temp variables
		symbolTable.clearTemp();
	}

	return nullptr;
}

/*
evaluates an expression like
var_1 = -9902;
var_1 = var_2;
*/
antlrcpp::Any CodeGenVisitor::visitSassign(ifccParser::SassignContext *ctx)
{
	// Get the name of the variable
	std::string dname = ctx->ID()->getText();

	// Parse the right hand side of the expression
	auto root = visitChildren(ctx).as<ArithmeticNode<int> *>();

	// Generate the code for the right hand side
	root->writeASM(&symbolTable, dname, std::cout);

	// Free the memory allocated for the right hand side
	delete root;

	// Clear temp variables
	symbolTable.clearTemp();

	return nullptr;
}

/*
this funtion is called for evaluating a return statement
the return value is an rval (see grammar)
*/
antlrcpp::Any CodeGenVisitor::visitRet(ifccParser::RetContext *ctx)
{
	// Parse the return value
	auto root = visitChildren(ctx).as<ArithmeticNode<int> *>();

	// Generate the code for the return value
	root->writeASM(&symbolTable, "%eax", std::cout);

	// Free the memory allocated for the right hand side
	delete root;

	// Clear temp variables
	symbolTable.clearTemp();

	return nullptr;
}

/*
Called when encountering a int literal
*/
antlrcpp::Any CodeGenVisitor::visitConst(ifccParser::ConstContext *ctx)
{
	std::string constante = ctx->CONST()->getText();
	ArithmeticNode<int> *node = new ConstNode<int>(std::stoi(constante));

	// Return the ConstNode
	return node;
}

antlrcpp::Any CodeGenVisitor::visitBitwise(ifccParser::BitwiseContext *ctx)
{
	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode<int> *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode<int> *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

/*
Called when encountering a variable
*/
antlrcpp::Any CodeGenVisitor::visitId(ifccParser::IdContext *ctx)
{
	std::string name = ctx->ID()->getText();
	symbolTable.used(name);
	ArithmeticNode<int> *node = new VarNode<int>(name);

	// Return the VarNode
	return node;
}

/*
Called when encountering a + or -
*/
antlrcpp::Any CodeGenVisitor::visitAddminus(ifccParser::AddminusContext *ctx)
{
	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode<int> *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode<int> *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

/*
Called when encountering a *, / or %
*/
antlrcpp::Any CodeGenVisitor::visitMuldiv(ifccParser::MuldivContext *ctx)
{

	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode<int> *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode<int> *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

/*
Called when encountering parentheses
*/
antlrcpp::Any CodeGenVisitor::visitPar(ifccParser::ParContext *ctx)
{
	// Return the node of the expression in the parenthesis
	return visit(ctx->arithmetic()).as<ArithmeticNode<int> *>();
}

/*
Called when encountering a unary operator (-, +, !)
*/
antlrcpp::Any CodeGenVisitor::visitUnary(ifccParser::UnaryContext *ctx)
{
	// Parse the expression
	auto *operand = visit(ctx->arithmetic()).as<ArithmeticNode<int> *>();

	// Create the node
	ArithmeticNode<int> *node = new UnaryNode<int>(ctx->op->getText(), operand);

	// If there are no variables in the expression, we can evaluate it
	if (node->type() == Type::CONST)
	{
		int value = node->eval();
		delete node;

		// Return the value as a ConstNode
		return (ArithmeticNode<int> *)new ConstNode<int>(value);
	}

	// Return the UnaryNode
	return node;
}

/*
Called when encountering a equality operator (==, !=)
*/
antlrcpp::Any CodeGenVisitor::visitCompeq(ifccParser::CompeqContext *ctx)
{
	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode<int> *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode<int> *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

/*
Called when encountering a relational operator (<, <=, >, >=)
*/
antlrcpp::Any CodeGenVisitor::visitComprel(ifccParser::ComprelContext *ctx)
{
	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode<int> *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode<int> *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

/*
Called when encountering a logical operator (&&, ||)
*/
antlrcpp::Any CodeGenVisitor::visitOplog(ifccParser::OplogContext *ctx)
{
	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode<int> *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode<int> *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

ArithmeticNode<int> *CodeGenVisitor::binaryOp(ArithmeticNode<int> *left, ArithmeticNode<int> *right, std::string op)
{
	ArithmeticNode<int> *node = new BinaryNode<int>(op, left, right);

	// If there are no variables in the expression, we can evaluate it
	if (node->type() == Type::CONST)
	{
		int value = node->eval();
		delete node;

		// Return the value as a ConstNode
		return (ArithmeticNode<int> *)new ConstNode<int>(value);
	}

	// Return the BinaryNode
	return node;
}