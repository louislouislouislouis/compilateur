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
	// std::cout << ".globl	" << MAIN << "\n";
	// current_cfg()->gen_asm(std::cout);
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
		auto node = visitChildren(ctx).as<ArithmeticNode *>();
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
	auto root = visitChildren(ctx).as<ArithmeticNode *>();

	auto op = ctx->OPASSIGN->getText();
	ArithmeticNode *node = root;
	if (op != "=")
	{
		node = new BinaryNode(op.substr(0, op.length() - 1), new VarNode(dname, current_cfg()->getST()->getSize(dname)), root);
	}

	// Generate the code for the right hand side
	node->generate(current_cfg(), dname);

	return nullptr;
}

antlrcpp::Any IRGenVisitor::visitAssignChain(ifccParser::AssignChainContext *ctx)
{
	// Get the name of the variable
	std::string dname = ctx->ID()->getText();

	// Parse the right hand side of the expression
	auto root = visitChildren(ctx).as<ArithmeticNode *>();

	auto op = ctx->OPASSIGN->getText();
	ArithmeticNode *node = root;
	if (op != "=")
	{
		node = new BinaryNode(op.substr(0, op.length() - 1), new VarNode(dname, current_cfg()->getST()->getSize(dname)), root);
	}

	// Generate the code for the right hand side
	node->generate(current_cfg(), dname);
	delete node;
	return dynamic_cast<ArithmeticNode *>(new VarNode(dname, current_cfg()->getST()->getSize(dname)));
}

antlrcpp::Any IRGenVisitor::visitRet(ifccParser::RetContext *ctx)
{
	// Parse the return value
	auto node = visitChildren(ctx).as<ArithmeticNode *>();
	if (!current_cfg()->getST()->exists("rvar"))
		current_cfg()->getST()->addTemp("rvar", "int", {ctx->getStart()->getLine(), ctx->getStart()->getCharPositionInLine()});
	node->generate(current_cfg(), "rvar");
	current_cfg()->current_bb->add_IRInstr(IRInstr::Operation::ret, "rvar");

	return nullptr;
}

antlrcpp::Any IRGenVisitor::visitConst(ifccParser::ConstContext *ctx)
{
	std::string constante = ctx->CONST()->getText();
	ArithmeticNode *node;
	if (constante[0] == '\'')
	{
		node = new ConstNode(constante[1]);
	}
	else
	{
		node = new ConstNode(std::stoi(constante));
	}

	// Return the ConstNode
	return node;
}

antlrcpp::Any IRGenVisitor::visitBitwise(ifccParser::BitwiseContext *ctx)
{
	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode *>();

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
	ArithmeticNode *node = new VarNode(name, current_cfg()->getST()->getSize(name));

	// Return the VarNode
	return node;
}

/*
Called when encountering a + or -
*/
antlrcpp::Any IRGenVisitor::visitAddminus(ifccParser::AddminusContext *ctx)
{
	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

/*
Called when encountering a *, / or %
*/
antlrcpp::Any IRGenVisitor::visitMuldiv(ifccParser::MuldivContext *ctx)
{

	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

/*
Called when encountering parentheses
*/
antlrcpp::Any IRGenVisitor::visitPar(ifccParser::ParContext *ctx)
{
	// Return the node of the expression in the parenthesis
	return visit(ctx->arithmetic()).as<ArithmeticNode *>();
}

/*
Called when encountering a unary operator (-, +, !)
*/
antlrcpp::Any IRGenVisitor::visitUnary(ifccParser::UnaryContext *ctx)
{
	// Parse the expression
	auto *operand = visit(ctx->arithmetic()).as<ArithmeticNode *>();

	// Create the node
	ArithmeticNode *node = new UnaryNode(ctx->op->getText(), operand);

	// If there are no variables in the expression, we can evaluate it
	if (node->type() == Type::CONST)
	{
		int value = node->eval();
		delete node;

		// Return the value as a ConstNode ,current_cfg()->getST()->getSize("char")
		return (ArithmeticNode *)new ConstNode(value);
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
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

/*
Called when encountering a relational operator (<, <=, >, >=)
*/
antlrcpp::Any IRGenVisitor::visitComprel(ifccParser::ComprelContext *ctx)
{
	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

/*
Called when encountering a shift operator (<<, >>)
*/
antlrcpp::Any IRGenVisitor::visitShift(ifccParser::ShiftContext *ctx)
{
	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

/*
Called when encountering a logical operator (&&, ||)
*/
antlrcpp::Any IRGenVisitor::visitOplog(ifccParser::OplogContext *ctx)
{
	// Parse the left hand side of the expression
	auto left = visit(ctx->arithmetic()[0]).as<ArithmeticNode *>();

	// Parse the right hand side of the expression
	auto right = visit(ctx->arithmetic()[1]).as<ArithmeticNode *>();

	// Return the BinaryNode
	return binaryOp(left, right, ctx->op->getText());
}

ArithmeticNode *IRGenVisitor::binaryOp(ArithmeticNode *left, ArithmeticNode *right, std::string op)
{
	ArithmeticNode *node = new BinaryNode(op, left, right);

	// If there are no variables in the expression, we can evaluate it
	if (node->type() == Type::CONST)
	{
		int value = node->eval();
		delete node;

		// Return the value as a ConstNode
		return (ArithmeticNode *)new ConstNode(value);
	}

	// Return the BinaryNode
	return node;
}

antlrcpp::Any IRGenVisitor::visitLoopW(ifccParser::LoopWContext *ctx)
{
	in_loop = true;
	std::string cmp_name = "*" + std::to_string(this->current_cfg()->getBbs()->size());
	this->current_cfg()->getST()->addTemp(cmp_name, "int");
	auto conditionBlock = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name(), cmp_name);

	this->current_cfg()->add_bb(conditionBlock, true);
	this->current_cfg()->current_bb = conditionBlock;

	auto node = visit(ctx->inlineArithmetic()).as<ArithmeticNode *>();
	node->generate(this->current_cfg(), cmp_name);

	auto body = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name());
	this->current_cfg()->add_bb(body, true);

	auto next = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name());
	this->current_cfg()->add_bb(next, false);

	this->current_cfg()->current_bb = body;
	loop_cond = conditionBlock;
	loop_next = next;
	body->exit_true = conditionBlock;
	for (auto expr : ctx->expr())
	{
		visit(expr);
	}

	this->current_cfg()->current_bb = next;
	in_loop = false;
	return nullptr;
}

antlrcpp::Any IRGenVisitor::visitConditionnal(ifccParser::ConditionnalContext *ctx)
{

	auto initial = this->current_cfg()->current_bb->exit_true;
	auto node = visit(ctx->inlineArithmetic()).as<ArithmeticNode *>();

	std::string cmp_name = "*" + std::to_string(this->current_cfg()->getBbs()->size());
	this->current_cfg()->getST()->addTemp(cmp_name, "int");

	auto conditionBlock = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name(), cmp_name);
	auto thenB = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name());
	BasicBlock *elseB;
	auto next = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name());

	conditionBlock->exit_false = next;

	this->current_cfg()->add_bb(conditionBlock, true);
	this->current_cfg()->current_bb = conditionBlock;
	node->generate(this->current_cfg(), cmp_name);

	this->current_cfg()->add_bb(thenB, true);

	if (ctx->elseexpr() != nullptr)
	{
		elseB = new BasicBlock(this->current_cfg(), this->current_cfg()->new_BB_name());
		elseB->exit_true = next;
		this->current_cfg()->add_bb(elseB, false);
		this->current_cfg()->current_bb = elseB;
		visit(ctx->elseexpr());
	}

	this->current_cfg()->current_bb = thenB;
	visit(ctx->ifexpr());

	this->current_cfg()->add_bb(next, true);

	next->exit_true = initial;
	this->current_cfg()->current_bb = next;

	return nullptr;
}
bool IRGenVisitor::genCode(InstructionSet instructionSet)
{
	switch (instructionSet)
	{
	case x86:
		std::cout << ".globl	" << MAIN << "\n";
		functions[0]->gen_asm(std::cout);
		break;

	default:
		break;
	}
	return true;
}

antlrcpp::Any IRGenVisitor::visitLoopCtrl(ifccParser::LoopCtrlContext *ctx)
{
	if (!in_loop)
	{

		std::cerr << "Error: loop control outside loop" << std::endl;
		exit(1);
	}

	auto keyWord = ctx->keyW->getText();
	if (keyWord == "break")
	{
		this->current_cfg()->current_bb->add_IRInstr(IRInstr::jmp, loop_next->label);
	}
	else
	{
		this->current_cfg()->current_bb->add_IRInstr(IRInstr::jmp, loop_cond->label);
	}
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