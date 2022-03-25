#include "CodeGenVisitor.h"
#include <string>
#ifdef __APPLE__
#define MAIN "_main"
#else
#define MAIN "main"
#endif
/*
visite de l'axiome donc:
On cale les prerequis du programme minimal et au milieu on visite les expressions entre
prints the basic layout into the .s file and starts the evaluation of the program by calling visitChildren(ctx).

int main(){
    ...
    return 0;
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
	std::cout << ".globl	" << MAIN << "\n"
			  << MAIN << ":\n	pushq %rbp\n	movq %rsp, %rbp\n\n";
	visitChildren(ctx); // not safe for use in visitors that modify the tree structure
	std::cout << "\n	popq %rbp\n	ret\n";
	symbolTable.checkUse(); // warings for all unused variables
	return 0;
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
	std::string varname = ctx->ID()->getText(); // get the name of the variable as string
	symbolTable.add(varname,
					4,																					   // in the futer we should consider the type of the variable, 4 is not always the right size
					{ctx->ID()->getSymbol()->getLine(), ctx->ID()->getSymbol()->getCharPositionInLine()}); // save the position in the program

	if (ctx->rval() != nullptr) // we have an expression which assigns int var = CONST|ID;
	{
		// if (ctx->rval()->CONST() != nullptr)
		// {
		//     std::cerr << "decl:" << std::endl;
		//     // std::cout << "COUCOU\n";
		//     std::cout << std::endl;
		//     std::cout << " 	movl	$" << stoi(ctx->rval()->CONST()->getText()) << ", -" << symbolTable.getOffset(id) << "(%rbp)\n";
		// }
		// else
		if (ctx->rval()->ID() != nullptr)
		{
			// std::cerr << "decl:" << std::endl;
			std::cout << std::endl;
			// int varname = var_1;
			// assign(var_1, varname);
			assign(ctx->rval()->ID()->getText(), varname);
		}
		else if (ctx->rval()->arithmetic() != nullptr)
		{
			visitChildren(ctx);
			int offsetRes = symbolTable.getOffset(ctx->rval()->arithmetic()->getText());
			int offsetAssign = symbolTable.getOffset(varname);
			// std::cerr << "decl:" << std::endl;
			std::cout << std::endl;
			// std::cout<<"Je passe par la\n";
			std::cout << "	movl -" << offsetRes << "(%rbp), %eax\n";
			std::cout << "	movl %eax, -" << offsetAssign << "(%rbp)\n";
		}
		else
		{
			visitChildren(ctx);
		}
	}

	return 0;
}
// visiteur de l'assignation
/*
evaluates an expression like
var_1 = -9902;
var_1 = var_2;
*/
antlrcpp::Any CodeGenVisitor::visitSassign(ifccParser::SassignContext *ctx)
{
	// std::cerr << "assign:" << std::endl;
	std::string id = ctx->ID()->getText();
	// if (ctx->rval()->CONST() != nullptr)
	// {
	//     std::cout << std::endl;
	// movl $"-9902", -"offset_of_variable(%rbp)"
	//     std::cout << " 	movl	$" << stoi(ctx->rval()->CONST()->getText()) << ", -" << symbolTable.getOffset(id) << "(%rbp)\n";
	// }
	// else
	if (ctx->rval()->ID() != nullptr)
	{

		// assign(var_1, valueof(var_2))
		//!!!possible bug because value not id of first argument is passed!!!
		assign(ctx->rval()->ID()->getText(), id);
	}
	else if (ctx->rval()->arithmetic() != nullptr)
	{
		visitChildren(ctx);
		int offsetRes = symbolTable.getOffset(ctx->rval()->arithmetic()->getText());
		int offsetAssign = symbolTable.getOffset(id);
		std::cout << std::endl;
		// std::cout<<"Je passe par la\n";
		std::cout << "	movl -" << offsetRes << "(%rbp), %eax\n";
		std::cout << "	movl %eax, -" << offsetAssign << "(%rbp)\n";
	}
	else
	{
		visitChildren(ctx);
	}
	symbolTable.used(id);
	return 0;
}
/*
this funtion is called for evaluating a return statement
the return statement can either be a CONSTANT [0-9]+
or a variable (ID) [a-zA-Z_][a-zA-Z0-9_]*
*/
// visiteur du return
antlrcpp::Any CodeGenVisitor::visitRet(ifccParser::RetContext *ctx)
{
	// std::cerr << "return:" << std::endl;
	// if (ctx->rval()->CONST() != nullptr)
	// {
	// load the value of the constant expression into the %eax register which is used to return values
	//     std::cout << std::endl;
	//     std::cout << " 	movl	$" << ctx->rval()->CONST()->getText() << ", %eax\n";
	// }
	// else
	if (ctx->rval()->ID() != nullptr)
	{

		std::cout << "	movl	-" << symbolTable.getOffset(ctx->rval()->ID()->getText()) << "(%rbp), %eax\n";
	}
	else if (ctx->rval()->arithmetic() != nullptr)
	{
		// get the position of the variable on the stack, and load the bytes from
		// this register into the %eax register
		visitChildren(ctx);
		int offsetRes = symbolTable.getOffset(ctx->rval()->arithmetic()->getText());
		std::cout << std::endl;
		// std::cout<<"Je passe par la\n";
		std::cout << "	movl -" << offsetRes << "(%rbp), %eax\n";
	}
	else
	{
		visitChildren(ctx);
	}

	return 0;
}

antlrcpp::Any CodeGenVisitor::visitInlineArithmetic(ifccParser::InlineArithmeticContext *ctx)
{
	visitChildren(ctx);
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitConst(ifccParser::ConstContext *ctx)
{
	std::string constante = ctx->CONST()->getText();
	if (!symbolTable.isContained(constante))
		symbolTable.add(constante, 4, {ctx->CONST()->getSymbol()->getLine(), ctx->CONST()->getSymbol()->getCharPositionInLine()}, true);
	// std::cerr << "Const:" << std::endl;
	std::cout << std::endl;
	std::cout << "	movl	$" << constante << ", -" << symbolTable.getOffset(constante) << "(%rbp)\n";
	return 0;
}

antlrcpp::Any CodeGenVisitor::visitBitwise(ifccParser::BitwiseContext *ctx)
{
        visitChildren(ctx);
        std::string element = ctx->getText();
        std::string op = ctx->op->getText();
        if(!symbolTable.isContained(element)){
            symbolTable.add(element,4, {0, 0}, true);
        }
        int offsetGlobal = symbolTable.getOffset(element);
        int offsetVar_1 = symbolTable.getOffset(ctx->arithmetic(0)->getText());
        // std::cout << "var_1 "<<ctx->arithmetic(0)->getText()<<"\n";
        // std::cout << "var_2 "<<ctx->arithmetic(1)->getText()<<"\n";
        int offsetVar_2 = symbolTable.getOffset(ctx->arithmetic(1)->getText());
        std::cerr << "Bitwise:" << std::endl;
        std::cout << std::endl;
        if(op == "&"){
            std::cout << " 	movl	-" <<offsetVar_1<< "(%rbp), %eax\n";
            std::cout << " 	andl	-" <<offsetVar_2<< "(%rbp), %eax\n";
            std::cout << " 	movl	%eax, -"<<offsetGlobal<< "(%rbp)\n";
        }
        else if(op == "|"){
            std::cout << " 	movl	-" <<offsetVar_1<< "(%rbp), %eax\n";
            std::cout << " 	orl	-" <<offsetVar_2<< "(%rbp), %eax\n";
            std::cout << " 	movl	%eax, -"<<offsetGlobal<< "(%rbp)\n";
        }
        else if(op == "^"){
            std::cout << " 	movl	-" <<offsetVar_1<< "(%rbp), %eax\n";
            std::cout << " 	xorl	-" <<offsetVar_2<< "(%rbp), %eax\n";
            std::cout << " 	movl	%eax, -"<<offsetGlobal<< "(%rbp)\n";
        }
        return 0;
}

antlrcpp::Any CodeGenVisitor::visitAddminus(ifccParser::AddminusContext *ctx)
{

	visitChildren(ctx);
	std::string element = ctx->getText();
	int offsetGauche = symbolTable.getOffset(ctx->arithmetic(0)->getText());
	int offsetDroite = symbolTable.getOffset(ctx->arithmetic(1)->getText());
	if (!symbolTable.isContained(element))
		symbolTable.add(element, 4, {0, 0}, true);
	int offsetGlobal = symbolTable.getOffset(element);
	std::string op = ctx->op->getText();
	std::string keyWord;
	if (op == "+")
		keyWord = "addl";
	else
		keyWord = "subl";
	// std::cerr << "AddMinus:" << std::endl;
	std::cout << std::endl;
	std::cout << "	movl -" << offsetGauche << "(%rbp), %eax\n";
	std::cout << "	" << keyWord << " -" << offsetDroite << "(%rbp), %eax\n";
	std::cout << "	movl %eax, -" << offsetGlobal << "(%rbp)\n";

	return 0;
}
antlrcpp::Any CodeGenVisitor::visitMuldiv(ifccParser::MuldivContext *ctx)
{

	visitChildren(ctx);
	std::string element = ctx->getText();
	int offsetGauche = symbolTable.getOffset(ctx->arithmetic(0)->getText());
	int offsetDroite = symbolTable.getOffset(ctx->arithmetic(1)->getText());
	if (!symbolTable.isContained(element))
	{

		symbolTable.add(element, 4, {ctx->getStart()->getLine(), ctx->getStart()->getCharPositionInLine()}, true);
	}
	int offsetGlobal = symbolTable.getOffset(element);
	std::string op = ctx->op->getText();
	std::string keyWord;
	if (op == "*")
		keyWord = "imull";
	else if (op == "/")
		keyWord = "cltd";
	else
		keyWord = "cltd";

	// std::cerr << "MulDiv:" << std::endl;
	std::cout << std::endl;
	std::cout << "	movl -" << offsetGauche << "(%rbp), %eax\n";
	if (op == "*")
	{
		std::cout << "	" << keyWord << " -" << offsetDroite << "(%rbp), %eax\n";
		std::cout << "	movl %eax, -" << offsetGlobal << "(%rbp)\n";
	}
	else
	{
		std::cout << "	" << keyWord << "\n";
		std::cout << "	idivl	"
				  << " -" << offsetDroite << "(%rbp)\n";
		if (op == "/")
		{
			std::cout << "	movl %eax, -" << offsetGlobal << "(%rbp)\n";
		}
		else
		{
			std::cout << "	movl %edx, -" << offsetGlobal << "(%rbp)\n";
		}
	}

	return 0;
}
antlrcpp::Any CodeGenVisitor::visitPar(ifccParser::ParContext *ctx)
{
	visitChildren(ctx);
	std::string element = ctx->getText();
	std::cerr << element << std::endl;
	if (!symbolTable.isContained(element))
		symbolTable.add(element, 4, {0, 0}, true);
	assign(ctx->arithmetic()->getText(), element);
	return 0;
}
// parser de l'assembly de l'assignation
antlrcpp::Any CodeGenVisitor::visitUnary(ifccParser::UnaryContext *ctx)
{
	visitChildren(ctx);
	std::string expression = ctx->getText();
	if (!symbolTable.isContained(expression))
		symbolTable.add(expression, 4, {0, 0}, true);
	int offset = symbolTable.getOffset(expression);
	assign(ctx->arithmetic()->getText(), expression);
	if (ctx->op->getText() == "-")
	{
		std::cout << "	negl -" << offset << "(%rbp)\n";
	}
	else if (ctx->op->getText() == "!")
	{

		std::cout << "	cmpl $0, %eax\n";
		std::cout << "	sete %al\n";
		std::cout << "	movzbl %al, %eax\n";
		std::cout << "	movl %eax, -" << offset << "(%rbp)\n";
	}

	return 0;
}

antlrcpp::Any CodeGenVisitor::visitCompeq(ifccParser::CompeqContext *ctx)
{
	// std::cerr << "Compeq:" << ctx->getText() << std::endl;
	visitChildren(ctx);
	std::string element = ctx->getText();
	if (!symbolTable.isContained(element))
		symbolTable.add(element, 4, {ctx->getStart()->getLine(), ctx->getStart()->getCharPositionInLine()}, true);
	getComp(element, ctx->arithmetic(0), ctx->arithmetic(1), ctx->op->getText());

	return 0;
}
antlrcpp::Any CodeGenVisitor::visitComprel(ifccParser::ComprelContext *ctx)
{
	// std::cerr << "Comprel:" << ctx->getText() << std::endl;
	visitChildren(ctx);
	std::string element = ctx->getText();
	if (!symbolTable.isContained(element))
		symbolTable.add(element, 4, {ctx->getStart()->getLine(), ctx->getStart()->getCharPositionInLine()}, true);
	getComp(element, ctx->arithmetic(0), ctx->arithmetic(1), ctx->op->getText());

	return 0;
}
antlrcpp::Any CodeGenVisitor::visitOplog(ifccParser::OplogContext *ctx)
{
	// std::cerr << "Oplog:" << ctx->getText() << std::endl;
	visitChildren(ctx);
	std::string element = ctx->getText();
	if (!symbolTable.isContained(element))
		symbolTable.add(element, 4, {ctx->getStart()->getLine(), ctx->getStart()->getCharPositionInLine()}, true);

	int offsetGauche = symbolTable.getOffset(ctx->arithmetic(0)->getText());
	int offsetDroite = symbolTable.getOffset(ctx->arithmetic(1)->getText());
	int offsetGlobal = symbolTable.getOffset(element);

	std::string op = ctx->op->getText() == "&&" ? "1" : "2";

	std::cout << std::endl;
	std::cout << "	movl -" << offsetGauche << "(%rbp), %eax\n";
	std::cout << "	cmpl $0, %eax\n";
	std::cout << "	sete %dl\n";

	std::cout << "	movl -" << offsetDroite << "(%rbp), %eax\n";
	std::cout << "	cmpl $0, %eax\n";
	std::cout << "	sete %dh\n";

	std::cout << "	add %dh, %dl\n";
	std::cout << "	movzb %dl, %eax\n";
	std::cout << "	cmpl $" << op << ", %eax\n";
	std::cout << "	setl %al\n";

	std::cout << "	movzb %al, %eax\n";
	std::cout << "	movl %eax, -" << offsetGlobal << "(%rbp)\n";

	return 0;
}

std::map<std::string, std::string> ops = {
	{"==", "e"},
	{"!=", "ne"},
	{"<", "l"},
	{"<=", "le"},
	{">", "g"},
	{">=", "ge"},
};
void CodeGenVisitor::getComp(std::string element, ifccParser::ArithmeticContext *ctxl, ifccParser::ArithmeticContext *ctxr, std::string op)
{

	int offsetGauche = symbolTable.getOffset(ctxl->getText());
	int offsetDroite = symbolTable.getOffset(ctxr->getText());
	int offsetGlobal = symbolTable.getOffset(element);

	std::string keyWord = "set" + ops[op];
	std::cout << std::endl;
	std::cout << "	movl -" << offsetGauche << "(%rbp), %eax\n";
	std::cout << "	cmpl -" << offsetDroite << "(%rbp), %eax\n";
	std::cout << "	" << keyWord << "	%al\n";
	std::cout << "	movzbl %al, %eax\n";
	std::cout << "	movl %eax, -" << offsetGlobal << "(%rbp)\n";
}

void CodeGenVisitor::assign(std::string ids, std::string idd)
{

	// movl -offset(%rbp),%eax move value on this position in the stack into %eax
	std::cout << "	movl	-" << symbolTable.getOffset(ids) << "(%rbp), %eax\n";
	// move the new value in %eax into an other register
	std::cout << "	movl	%eax, -" << symbolTable.getOffset(idd) << "(%rbp)\n";
}
