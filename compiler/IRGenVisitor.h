#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include "IR.h"
#include "ArithmeticNode.h"

using namespace std;

class IRGenVisitor : public ifccBaseVisitor
{
private:
	SymbolTable globalSymbolTable;

	CFG *current_cfg()
	{
		return functions.back();
	}

public:
	vector<CFG *> functions;
	// enum of instruction sets supported
	typedef enum
	{
		x86,
		arm
	} InstructionSet;
	bool in_loop = false;
	BasicBlock *loop_cond = nullptr;
	BasicBlock *loop_next = nullptr;

	IRGenVisitor(std::ostream &out = std::cout, std::ostream &err = std::cerr) : globalSymbolTable(out, err), functions(){};
	virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
	virtual antlrcpp::Any visitSdecl(ifccParser::SdeclContext *ctx) override;
	virtual antlrcpp::Any visitSassign(ifccParser::SassignContext *ctx) override;
	virtual antlrcpp::Any visitConst(ifccParser::ConstContext *ctx) override;
	virtual antlrcpp::Any visitBitwise(ifccParser::BitwiseContext *ctx) override;
	virtual antlrcpp::Any visitAddminus(ifccParser::AddminusContext *ctx) override;
	virtual antlrcpp::Any visitMuldiv(ifccParser::MuldivContext *ctx) override;
	virtual antlrcpp::Any visitPar(ifccParser::ParContext *ctx) override;
	virtual antlrcpp::Any visitUnary(ifccParser::UnaryContext *ctx) override;
	virtual antlrcpp::Any visitComprel(ifccParser::ComprelContext *ctx) override;
	virtual antlrcpp::Any visitCompeq(ifccParser::CompeqContext *ctx) override;
	virtual antlrcpp::Any visitShift(ifccParser::ShiftContext *ctx) override;
	virtual antlrcpp::Any visitOplog(ifccParser::OplogContext *ctx) override;
	virtual antlrcpp::Any visitId(ifccParser::IdContext *ctx) override;
	virtual antlrcpp::Any visitRet(ifccParser::RetContext *ctx) override;
	virtual antlrcpp::Any visitLoopW(ifccParser::LoopWContext *ctx) override;
	virtual antlrcpp::Any visitConditionnal(ifccParser::ConditionnalContext *ctx) override;
	virtual antlrcpp::Any visitAssignChain(ifccParser::AssignChainContext *ctx) override;
	virtual antlrcpp::Any visitLoopCtrl(ifccParser::LoopCtrlContext *ctx) override;
	bool genCode(InstructionSet instructionSet);

	ArithmeticNode *binaryOp(ArithmeticNode *left, ArithmeticNode *right, std::string op);
};
