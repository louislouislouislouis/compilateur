#include "IRGenVisitor.h"
antlrcpp::Any IRGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
    
	instructionsList.push_back(new CFG())
	
	// Generate the body of the main function
    visitChildren(ctx);

    // Check for unused variables
	symbolTable.checkUse();
    
	return 0;
}
antlrcpp::Any IRGenVisitor::visitSdecl(ifccParser::SdeclContext *ctx) 
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
	return 0;
}
