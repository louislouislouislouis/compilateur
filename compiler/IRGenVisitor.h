#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include "IR.h"
#include "ArithmeticNode.h"

using namespace std;

class IRGenVisitor : public ifccBaseVisitor
{
private:
	vector<CFG *> functions;
	SymbolTable globalSymbolTable;

	CFG *current_cfg()
	{
		return functions.back();
	}

public:
	// enum of instruction sets supported
	typedef enum
	{
		x86,
		arm
	} InstructionSet;

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
	virtual antlrcpp::Any visitOplog(ifccParser::OplogContext *ctx) override;
	virtual antlrcpp::Any visitId(ifccParser::IdContext *ctx) override;
	virtual antlrcpp::Any visitRet(ifccParser::RetContext *ctx) override;
	bool genCode(InstructionSet instructionSet);

	ArithmeticNode<int> *binaryOp(ArithmeticNode<int> *left, ArithmeticNode<int> *right, std::string op);
	~IRGenVisitor()
	{
		for (auto cfg : functions)
		{
			std::cerr << cfg->getName() << std::endl;
			for (auto bb : *(cfg->getBbs()))
			{
				std::cerr << "	" << bb->label << std::endl;
				for (auto inst : bb->instrs)
				{
					std::cerr << "		" << inst->getOp() << "	";
					for (auto v : *(inst->get_params()))
					{
						std::cerr << v << "	";
					}
					std::cerr << std::endl;
				}
			}
		}
	}
};
