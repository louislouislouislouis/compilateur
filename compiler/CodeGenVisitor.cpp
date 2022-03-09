#include "CodeGenVisitor.h"

#ifdef __APPLE__
#define MAIN "_main"
#else
#define MAIN "main"
#endif

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{
    // int retval = stoi(ctx->CONST()->getText());

    std::cout << ".globl    " << MAIN << "\n " << MAIN << ": \n 	pushq %rbp\n 	movq %rsp, %rbp\n\n";
    visitChildren(ctx);
    std::cout << "\n 	popq %rbp\n 	ret\n";
    symbolTable.checkUse();
    return 0;
}

antlrcpp::Any CodeGenVisitor::visitSdecl(ifccParser::SdeclContext *ctx)
{
    std::string id = ctx->ID()->getText();
    symbolTable.add(id,
                    4,
                    {ctx->ID()->getSymbol()->getLine(), ctx->ID()->getSymbol()->getCharPositionInLine()});

    if (ctx->rval() != nullptr)
    {
        if (ctx->rval()->CONST() != nullptr)
        {
            std::cout << " 	movl	$" << stoi(ctx->rval()->CONST()->getText()) << ", -" << symbolTable.getOffset(id) << "(%rbp)\n";
        }
        else
        {
            assign(ctx->rval()->ID()->getText(), id);
        }
    }

    return 0;
}

antlrcpp::Any CodeGenVisitor::visitSassign(ifccParser::SassignContext *ctx)
{
    std::string id = ctx->ID()->getText();

    if (ctx->rval()->CONST() != nullptr)
    {
        std::cout << " 	movl	$" << stoi(ctx->rval()->CONST()->getText()) << ", -" << symbolTable.getOffset(id) << "(%rbp)\n";
    }
    else
    {
        assign(ctx->rval()->ID()->getText(), id);
    }

    return 0;
}

antlrcpp::Any CodeGenVisitor::visitRet(ifccParser::RetContext *ctx)
{
    if (ctx->rval()->CONST() != nullptr)
    {
        std::cout << " 	movl	$" << ctx->rval()->CONST()->getText() << ", %eax\n";
    }
    else
    {
        std::cout << " 	movl	-" << symbolTable.getOffset(ctx->rval()->ID()->getText()) << "(%rbp), %eax\n";
    }

    return 0;
}

void CodeGenVisitor::assign(std::string ids, std::string idd)
{
    std::cout << " 	movl	-" << symbolTable.getOffset(ids) << "(%rbp), %r8d\n";
    std::cout << " 	movl	%r8d, -" << symbolTable.getOffset(idd) << "(%rbp)\n";
}
