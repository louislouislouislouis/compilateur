#include "IRGenVisitor.h"
antlrcpp::Any IRGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
    CFG* firstInstruction = new CFG();
    instructionsList.push_back(firstInstruction);
    visitChildren(ctx);
	return 0;
}
