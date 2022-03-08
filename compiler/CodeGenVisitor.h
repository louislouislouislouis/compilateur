#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include <map>

class CodeGenVisitor : public ifccBaseVisitor
{
public:
	virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
	virtual antlrcpp::Any visitSdecl(ifccParser::SdeclContext *ctx) override;
	virtual antlrcpp::Any visitSassign(ifccParser::SassignContext *ctx) override;
	virtual antlrcpp::Any visitRet(ifccParser::RetContext *ctx) override;

	std::map<std::string, std::pair<int, antlr4::Token *>> symbolTable;
	std::map<std::string, int> varUse;
	int varLoc(antlr4::tree::TerminalNode *id) { return symbolTable[id->getText()].first * 4 + 4; }
	void assign(antlr4::tree::TerminalNode *ids, antlr4::tree::TerminalNode *idd);
	void checkVar(antlr4::tree::TerminalNode *id);
	void checkVarUse();
};
