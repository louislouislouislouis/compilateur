#include "IRGenVisitor.h"
#ifdef __APPLE__
#define MAIN "_main"
#else
#define MAIN "main"
#endif

antlrcpp::Any IRGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{

	functions.push_back(new CFG(MAIN));

	// Generate the body of the main function
	visitChildren(ctx);
	std::cout << ".globl	" << MAIN << "\n";
	current_cfg()->gen_asm(std::cout);
	// Check for unused variables
	current_cfg()->getST()->checkUse();

	return nullptr;
}

antlrcpp::Any IRGenVisitor::visitSdecl(ifccParser::SdeclContext *ctx)
{
	// Get the type of the variable
	std::string type = ctx->parent->children[0]->getText();

	// Get the name of the variable
	string name = ctx->ID()->getText();

	// Add the variable to the symbol table
	current_cfg()->getST()->add(name, type, {ctx->getStart()->getLine(), ctx->getStart()->getCharPositionInLine()});

	// Generate the code for the variable
	if (ctx->rval() != nullptr)
	{
		// Parse the right hand side of the expression
		auto node = visitChildren(ctx).as<ArithmeticNode<int> *>();
		node->generate(current_cfg(), name);
		// Clear temp variables
		// current_cfg()->getST()->clearTemp();
	}

	return nullptr;
}

antlrcpp::Any IRGenVisitor::visitSassign(ifccParser::SassignContext *ctx)
{
	// Get the name of the variable
	std::string dname = ctx->ID()->getText();

	// Parse the right hand side of the expression
	auto root = visitChildren(ctx).as<ArithmeticNode<int> *>();

	// Generate the code for the right hand side
	root->generate(current_cfg(), dname);

	return nullptr;
}

antlrcpp::Any IRGenVisitor::visitRet(ifccParser::RetContext *ctx)
{
	// Parse the return value
	auto node = visitChildren(ctx).as<ArithmeticNode<int> *>();
	current_cfg()->getST()->addTemp("rvar", "int", {ctx->getStart()->getLine(), ctx->getStart()->getCharPositionInLine()});
	node->generate(current_cfg(), "rvar");
	current_cfg()->current_bb->add_IRInstr(IRInstr::Operation::ret, "rvar");

	return nullptr;
}

antlrcpp::Any IRGenVisitor::visitConst(ifccParser::ConstContext *ctx)
{
	std::string constante = ctx->CONST()->getText();
	ArithmeticNode<int> *node = new ConstNode<int>(std::stoi(constante));

	// Return the ConstNode
	return node;
}

antlrcpp::Any IRGenVisitor::visitBitwise(ifccParser::BitwiseContext *ctx)
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
antlrcpp::Any IRGenVisitor::visitId(ifccParser::IdContext *ctx)
{
	std::string name = ctx->ID()->getText();
	current_cfg()->getST()->used(name);
	ArithmeticNode<int> *node = new VarNode<int>(name);

	// Return the VarNode
	return node;
}

/*
Called when encountering a + or -
*/
antlrcpp::Any IRGenVisitor::visitAddminus(ifccParser::AddminusContext *ctx)
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
antlrcpp::Any IRGenVisitor::visitMuldiv(ifccParser::MuldivContext *ctx)
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
antlrcpp::Any IRGenVisitor::visitPar(ifccParser::ParContext *ctx)
{
	// Return the node of the expression in the parenthesis
	return visit(ctx->arithmetic()).as<ArithmeticNode<int> *>();
}

/*
Called when encountering a unary operator (-, +, !)
*/
antlrcpp::Any IRGenVisitor::visitUnary(ifccParser::UnaryContext *ctx)
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
antlrcpp::Any IRGenVisitor::visitCompeq(ifccParser::CompeqContext *ctx)
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
antlrcpp::Any IRGenVisitor::visitComprel(ifccParser::ComprelContext *ctx)
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
antlrcpp::Any IRGenVisitor::visitOplog(ifccParser::OplogContext *ctx)
{
	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode<int> *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode<int> *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

ArithmeticNode<int> *IRGenVisitor::binaryOp(ArithmeticNode<int> *left, ArithmeticNode<int> *right, std::string op)
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

antlrcpp::Any IRGenVisitor::visitLoopW(ifccParser::LoopWContext *ctx)
{
	std::string cmp_name = "*" + std::to_string(this->current_cfg()->getBbs()->size());
	this->current_cfg()->getST()->addTemp(cmp_name, "int");
	auto conditionBlock = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name(), cmp_name);

	this->current_cfg()->add_bb(conditionBlock, true);
	this->current_cfg()->current_bb = conditionBlock;

	auto node = visit(ctx->inlineArithmetic()).as<ArithmeticNode<int> *>();
	node->generate(this->current_cfg(), cmp_name);

	auto body = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name());
	this->current_cfg()->add_bb(body, true);

	auto next = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name());
	this->current_cfg()->add_bb(next, false);

	this->current_cfg()->current_bb = body;
	for (auto expr : ctx->expr())
	{
		visit(expr);
	}
	body->exit_true = conditionBlock;
	this->current_cfg()->current_bb = next;

	return nullptr;
}

antlrcpp::Any IRGenVisitor::visitConditionnal(ifccParser::ConditionnalContext *ctx)
{
	std::string cmp_name = "*" + std::to_string(this->current_cfg()->getBbs()->size());
	this->current_cfg()->getST()->addTemp(cmp_name, "int");
	auto conditionBlock = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name(), cmp_name);

	this->current_cfg()->add_bb(conditionBlock, true);
	this->current_cfg()->current_bb = conditionBlock;

	auto node = visit(ctx->inlineArithmetic()).as<ArithmeticNode<int> *>();
	node->generate(this->current_cfg(), cmp_name);

	auto body = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name());
	this->current_cfg()->add_bb(body, true);

	auto next = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name());
	this->current_cfg()->add_bb(next, false);

	this->current_cfg()->current_bb = body;
	for (auto expr : ctx->expr())
	{
		visit(expr);
	}
	body->exit_true = next;
	this->current_cfg()->current_bb = next;

	return nullptr;
}

/*
visit(conditionnal)
	add true
	add false

	cfg.currentbb = true
	visit(true)

	cfg.currentbb = false
	visit(false)

	add endif
	link true et false to endif

	cfg.currentbb = endif


*/