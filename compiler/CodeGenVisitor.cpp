#include "CodeGenVisitor.h"

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
    std::cout << ".globl    " << MAIN << "\n " << MAIN << ": \n 	pushq %rbp\n 	movq %rsp, %rbp\n\n";
    visitChildren(ctx); // not safe for use in visitors that modify the tree structure
    std::cout << "\n 	popq %rbp\n 	ret\n";
    symbolTable.checkUse(); //warings for all unused variables
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
    std::string varname = ctx->ID()->getText(); //get the name of the variable as string
    symbolTable.add(varname,
                    4, //in the futer we should consider the type of the variable, 4 is not always the right size
                    {ctx->ID()->getSymbol()->getLine(), ctx->ID()->getSymbol()->getCharPositionInLine()}); //save the position in the program

    if (ctx->rval() != nullptr) //we have an expression which assigns int var = CONST|ID;
    {
        if (ctx->rval()->CONST() != nullptr)
        {
            //save the constant on the stack
            std::cout << " 	movl	$" << stoi(ctx->rval()->CONST()->getText()) << ", -" << symbolTable.getOffset(varname) << "(%rbp)\n";
        }
        else
        {   
            //int varname = var_1;
            //assign(var_1, varname);
            assign(ctx->rval()->ID()->getText(), varname);
        }
    }

    return 0;
}
/*
evaluates an expression like 
var_1 = -9902; 
var_1 = var_2;
*/
antlrcpp::Any CodeGenVisitor::visitSassign(ifccParser::SassignContext *ctx)
{
    std::string id = ctx->ID()->getText();

    if (ctx->rval()->CONST() != nullptr) //case var = -9902;
    {
        //movl $"-9902", -"offset_of_variable(%rbp)"
        std::cout << " 	movl	$" << stoi(ctx->rval()->CONST()->getText()) << ", -" << symbolTable.getOffset(id) << "(%rbp)\n";
    }
    else// case var_1 = var_2;
    {
        //assign(var_1, valueof(var_2))
        //!!!possible bug because value not id of first argument is passed!!!   
        assign(ctx->rval()->ID()->getText(), id); 
    }

    return 0;
}
/*
this funtion is called for evaluating a return statement
the return statement can either be a CONSTANT [0-9]+
or a variable (ID) [a-zA-Z_][a-zA-Z0-9_]*
*/
antlrcpp::Any CodeGenVisitor::visitRet(ifccParser::RetContext *ctx)
{
    if (ctx->rval()->CONST() != nullptr) // it's a constant
    {
        //load the value of the constant expression into the %eax register which is used to return values
        std::cout << " 	movl	$" << ctx->rval()->CONST()->getText() << ", %eax\n";
    }
    else    // return variable
    {   
        //get the position of the variable on the stack, and load the bytes from
        //this register into the %eax register
        std::cout << " 	movl	-" << symbolTable.getOffset(ctx->rval()->ID()->getText()) << "(%rbp), %eax\n";
    }

    return 0;
}

/*
evaluates an expression like variable_id_2 = variable_id_1;
*/
void CodeGenVisitor::assign(std::string id_1, std::string id_2)
{
    //movl -offset(%rbp),%eax move value on this position in the stack into %eax
    std::cout << " 	movl	-" << symbolTable.getOffset(id_1) << "(%rbp), %eax\n";
    //move the new value in %eax into an other register
    std::cout << " 	movl	%eax, -" << symbolTable.getOffset(id_2) << "(%rbp)\n";
}
