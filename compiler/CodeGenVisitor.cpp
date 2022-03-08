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
    checkVarUse();
    return 0;
}

void CodeGenVisitor::checkVarUse()
{
    for (auto v : varUse)
    {
        if (v.second == 0)
        {
            std::cerr << "Warning: variable " << v.first << " is never used" << std::endl;
            std::cerr << "\t Declared at line " << symbolTable[v.first].second->getLine()
                      << ":" << symbolTable[v.first].second->getCharPositionInLine() << std::endl;
        }
    }
}
antlrcpp::Any CodeGenVisitor::visitSdecl(ifccParser::SdeclContext *ctx)
{

    auto p = symbolTable.emplace(ctx->ID()->getText(), std::make_pair(symbolTable.size(), ctx->ID()->getSymbol()));
    varUse.insert(std::make_pair(ctx->ID()->getText(), 0));
    if (!p.second)
    {
        std::cerr << "Error: " << ctx->ID()->getText() << " already declared" << std::endl;
        std::cerr << "\t at line " << ctx->ID()->getSymbol()->getLine() << ":" << ctx->ID()->getSymbol()->getCharPositionInLine() << std::endl;
        std::cerr << "First declared at" << p.first->second.second->getLine() << ":" << p.first->second.second->getCharPositionInLine() << std::endl;
        exit(1);
    }

    if (ctx->rval() != nullptr)
    {
        if (ctx->rval()->CONST() != nullptr)
        {
            std::cout << " 	movl	$" << stoi(ctx->rval()->CONST()->getText()) << ", -" << varLoc(ctx->ID()) << "(%rbp)\n";
        }
        else
        {
            checkVar(ctx->rval()->ID());
            assign(ctx->rval()->ID(), ctx->ID());
        }
    }

    return 0;
}

antlrcpp::Any CodeGenVisitor::visitSassign(ifccParser::SassignContext *ctx)
{
    checkVar(ctx->ID());

    if (ctx->rval()->CONST() != nullptr)
    {
        std::cout << " 	movl	$" << stoi(ctx->rval()->CONST()->getText()) << ", -" << varLoc(ctx->ID()) << "(%rbp)\n";
    }
    else
    {
        checkVar(ctx->rval()->ID());
        assign(ctx->rval()->ID(), ctx->ID());
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
        checkVar(ctx->rval()->ID());
        std::cout << " 	movl	-" << varLoc(ctx->rval()->ID()) << "(%rbp), %eax\n";
    }

    return 0;
}

void CodeGenVisitor::assign(antlr4::tree::TerminalNode *ids, antlr4::tree::TerminalNode *idd)
{
    std::cout << " 	movl	-" << varLoc(ids) << "(%rbp), %r8d\n";
    std::cout << " 	movl	%r8d, -" << varLoc(idd) << "(%rbp)\n";
}

void CodeGenVisitor::checkVar(antlr4::tree::TerminalNode *id)
{
    if (symbolTable.find(id->getText()) == symbolTable.end())
    {
        std::cerr << "Error: " << id->getText() << " not declared" << std::endl;
        std::cerr << "\t at line " << id->getSymbol()->getLine() << ":" << id->getSymbol()->getCharPositionInLine() << std::endl;
        exit(1);
    }

    varUse.at(id->getText())++;
}
