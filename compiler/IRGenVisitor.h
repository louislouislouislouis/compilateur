#pragma once


#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include "IR.h"

using namespace std;

class  IRGenVisitor : public ifccBaseVisitor {
	private:
		vector<CFG*> instructionsList;
		SymbolTable symbolTable;
	public:
		IRGenVisitor(std::ostream &out = std::cout, std::ostream &err = std::cerr) : symbolTable(out, err){};
		virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override ;
		virtual antlrcpp::Any visitSdecl(ifccParser::SdeclContext *ctx) override;

	

};

