#include "IRGenVisitor.h"
antlrcpp::Any IRGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
    
	instructionsList.push_back(new CFG());
	
	// Generate the body of the main function
    visitChildren(ctx);

    // Check for unused variables
	globalSymbolTable.checkUse();
    
	return 0;
}
