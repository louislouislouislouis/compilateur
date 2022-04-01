#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <initializer_list>
#include <sstream>
// Declarations from the parser -- replace with your own
#include "SymbolTable.h"
//#include "type.h"
//#include "symbole.h"
using namespace std;
class BasicBlock;
class CFG;
class DefFonction;

//! The class for one 3-address instruction
class IRInstr
{

public:
	/** The instructions themselves -- feel free to subclass instead */
	typedef enum
	{
		ldconst,
		copy,
		add,
		sub,
		mul,
		neg,
		not_,
		ret,
		div,
		mod,
		lt,
		gt,
		eq,
		neq,
		leq,
		geq,
		and_,
		or_,
		band,
		bor,
		bxor,
		// rmem,
		// wmem,
		// call,
	} Operation;

	/**  constructor */
	// IRInstr(BasicBlock* bb_, Operation op, Type t, vector<string> params);  --OLD ONE
	IRInstr(BasicBlock *bb_, Operation op, vector<string> params) : bb(bb_), op(op), params(params){};

	/** Actual code generation */
	void gen_asm(ostream &o); /**< x86 assembly code generation for this IR instruction */
	vector<std::string> *get_params() { return &params; }
	Operation getOp() { return op; }

private:
	BasicBlock *bb; /**< The BB this instruction belongs to, which provides a pointer to the CFG this instruction belong to */
	Operation op;
	// Type t; --OLD
	vector<string> params; /**< For 3-op instrs: d, x, y; for ldconst: d, c;  For call: label, d, params;  for wmem and rmem: choose yourself */
						   // if you subclass IRInstr, each IRInstr subclass has its parameters and the previous (very important) comment becomes useless: it would be a better design.
};

/**  The class for a basic block */

/* A few important comments.
	 IRInstr has no jump instructions.
	 cmp_* instructions behaves as an arithmetic two-operand instruction (add or mult),
	  returning a boolean value (as an int)

	 Assembly jumps are generated as follows:
	 BasicBlock::gen_asm() first calls IRInstr::gen_asm() on all its instructions, and then
			if  exit_true  is a  nullptr,
			the epilogue is generated
		else if exit_false is a nullptr,
		  an unconditional jmp to the exit_true branch is generated
				else (we have two successors, hence a branch)
		  an instruction comparing the value of test_var_name to true is generated,
					followed by a conditional branch to the exit_false branch,
					followed by an unconditional branch to the exit_true branch
	 The attribute test_var_name itself is defined when converting
  the if, while, etc of the AST  to IR.

Possible optimization:
	 a cmp_* comparison instructions, if it is the last instruction of its block,
	   generates an actual assembly comparison
	   followed by a conditional jump to the exit_false branch
*/

class BasicBlock
{
public:
	BasicBlock(CFG *cfg, string entry_label, std::string test_var = "");
	void gen_asm(ostream &o);
	/**< x86 assembly code generation for this basic block (very simple) */

	// void add_IRInstr(IRInstr::Operation op, Type t, vector<string> params); --OLD
	void add_IRInstr(IRInstr::Operation op, vector<string> params);
	void add_IRInstr(IRInstr::Operation op, string param);

	// No encapsulation whatsoever here. Feel free to do better.
	BasicBlock *exit_true;	  /**< default pointer to the next basic block, true branch. If nullptr, return from procedure */
	BasicBlock *exit_false;	  /**< pointer to the next basic block, false branch. If null_ptr, the basic block ends with an unconditional jump */
	string label;			  /**< label of the BB, also will be the label in the generated code GOTO*/
	CFG *cfg;				  /** < the CFG where this block belongs */
	vector<IRInstr *> instrs; /** < the instructions themselves. */
	string test_var_name;	  /** < when generating IR code for an if(expr) or while(expr) etc,
														store here the name of the variable that holds the value of expr */
protected:
};

/** The class for the control flow graph, also includes the symbol table */

/* A few important comments:
	 The entry block is the one with the same label as the AST function name.
	   (it could be the first of bbs, or it could be defined by an attribute value)
	 The exit block is the one with both exit pointers equal to nullptr.
	 (again it could be identified in a more explicit way)

 */
class CFG
{
public:
	// CFG(DefFonction* ast); --OLD
	CFG(std::string name) : localSymbolTable(std::cout, std::cerr), bbs(), name(name)
	{
		this->nextBBnumber = 0;
		this->current_bb = nullptr;
		this->add_bb(new BasicBlock(this, "." + this->name + "_p"));
		this->add_bb(new BasicBlock(this, "." + this->name + "_e"));
		this->add_bb(new BasicBlock(this, new_BB_name()));
		this->bbs[0]->exit_true = this->bbs[2];
		this->bbs[2]->exit_true = this->bbs[1];
		this->current_bb = this->bbs[2];
	};

	// DefFonction *ast; /**< The AST this CFG comes from */

	void add_bb(BasicBlock *bb, bool cond = true);

	// x86 code generation: could be encapsulated in a processor class in a retargetable compiler
	void gen_asm(ostream &o)
	{

		this->gen_prolog(o);
		for (int i = 0; i < this->bbs.size(); i++)
		{
			auto bb = this->bbs[i];
			if (i >= 2)
			{
				bb->gen_asm(o);
			}
		}
		this->gen_epilog(o);
	}
	string IR_reg_to_asm(string reg); /**< helper method: inputs a IR reg or input variable, returns e.g. "-24(%rbp)" for the proper value of 24 */
	SymbolTable *getST() { return &this->localSymbolTable; }
	std::string getName() { return this->name; }
	std::vector<BasicBlock *> *getBbs() { return &this->bbs; }
	// symbol table methods
	// void add_to_symbol_table(string name, Type t);
	// string create_new_tempvar(Type t);
	size_t get_var_off(string name)
	{

		return this->localSymbolTable.getOffset(name, true);
	}
	// Type get_var_type(string name);

	// basic block management
	string new_BB_name() { return "." + name + "_" + std::to_string(nextBBnumber++); }
	BasicBlock *current_bb;

	void gen_prolog(ostream &o)
	{
		o << this->name << ":\n	pushq %rbp\n	movq %rsp, %rbp\n";
	}

	void gen_epilog(ostream &o)
	{
		o << "." + this->name + "_e:\n"
		  << "\tpopq %rbp\n	ret\n";
	}

protected:
	// map <string, Type> SymbolType; /**< part of the symbol table  */
	// map <string, int> SymbolIndex; /**< part of the symbol table  */
	// int nextFreeSymbolIndex; /**< to allocate new symbols in the symbol table */
	int nextBBnumber; /**< just for naming */
	std::string name;
	vector<BasicBlock *> bbs; /**< all the basic blocks of this CFG*/
	SymbolTable localSymbolTable;
};
