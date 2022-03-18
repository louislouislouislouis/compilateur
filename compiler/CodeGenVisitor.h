#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include <map>
#include "SymbolTable.h"

class CodeGenVisitor : public ifccBaseVisitor
{
private:
	SymbolTable symbolTable;
	int tempVar;

public:
	CodeGenVisitor(std::ostream *out = &std::cout, std::ostream *err = &std::cerr) : symbolTable(out, err){};
	virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
	virtual antlrcpp::Any visitSdecl(ifccParser::SdeclContext *ctx) override;
	virtual antlrcpp::Any visitSassign(ifccParser::SassignContext *ctx) override;
	virtual antlrcpp::Any visitRet(ifccParser::RetContext *ctx) override;
	virtual antlrcpp::Any visitConst(ifccParser::ConstContext *ctx) override;
	virtual antlrcpp::Any visitAddminus(ifccParser::AddminusContext *ctx) override;
	virtual antlrcpp::Any visitMuldiv(ifccParser::MuldivContext *ctx) override;
	virtual antlrcpp::Any visitPar(ifccParser::ParContext *ctx) override;
	virtual antlrcpp::Any visitInlineArithmetic(ifccParser::InlineArithmeticContext *ctx) override;
	virtual antlrcpp::Any visitMoinsunaire(ifccParser::MoinsunaireContext *ctx) override;

	void assign(std::string ids, std::string idd);
	void checkVar(antlr4::tree::TerminalNode *id);
	void checkVarUse();
};
