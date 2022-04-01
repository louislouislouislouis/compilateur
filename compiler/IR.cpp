#include "IR.h"

void CFG::add_bb(BasicBlock *bb) {
    this->bbs.push_back(bb);
    if (this->current_bb) {
        this->current_bb->exit_true = bb;
    }
    this->current_bb = bb;
}

BasicBlock::BasicBlock(CFG *cfg, string entry_label) {
    this->cfg = cfg;
    this->label = entry_label;
}

void IRInstr::gen_asm(ostream &o) {
    switch (this->op)
    {
    case ldconst:
        // C: var = const
        // params[0] = var
        // params[1] = const
        o << " 	movl	$" << params[1] << ", -" << bb->cfg->localSymbolTable.getOffset(params[0]) << "(%rbp)\n";
        break;
    case copy:
        // C: var1 = var2
        // params[0] = var1
        // params[1] = var2
        o << " 	movl	-" << bb->cfg->localSymbolTable.getOffset(params[1]) << "(%rbp), %r8d\n";
        o << " 	movl	%r8d, -" << bb->cfg->localSymbolTable.getOffset(params[0]) << "(%rbp)\n";
        break;
    case add:
        // C: var1 = var2 + var3
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        /* code */
        break;
    case sub:
        /* code */
        break;
    case mul:
        /* code */
        break;
    case rmem:
        /* code */
        break;
    case wmem:
        /* code */
        break;
    case call:
        /* code */
        break;
    case cmp_eq:
        /* code */
        break;
    case cmp_lt:
        /* code */
        break;
    case cmp_le:
        /* code */
        break;                                    
    default:
        break;
    }

}