#include "CodeGenVisitor.h"
antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{
    int retval = stoi(ctx->CONST()->getText());
    
#ifdef __APPLE__
    std::cout<<".globl    _main\n"
        " _main: \n"
        "     movl    $"<<retval<<", %eax\n"
        "     ret\n";
#elif
    std::cout<<".globl    main\n"
        " main: \n"
        "     movl    $"<<retval<<", %eax\n"
        "     ret\n";
    #endif

    return 0;
}







