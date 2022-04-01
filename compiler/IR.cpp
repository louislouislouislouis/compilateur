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
        o << " 	movl -" << bb->cfg->localSymbolTable.getOffset(params[1]) << "(%rbp), %eax\n";
        o << " 	" << "addl" << " -" << bb->cfg->localSymbolTable.getOffset(params[2]) << "(%rbp), %eax\n";
        o << " 	movl %eax, -" << bb->cfg->localSymbolTable.getOffset(params[0]) << "(%rbp)\n";
        break;
    case sub:
        // C: var1 = var2 - var3
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << " 	movl -" << bb->cfg->localSymbolTable.getOffset(params[1]) << "(%rbp), %eax\n";
        o << " 	" << "subl" << " -" << bb->cfg->localSymbolTable.getOffset(params[2]) << "(%rbp), %eax\n";
        o << " 	movl %eax, -" << bb->cfg->localSymbolTable.getOffset(params[0]) << "(%rbp)\n";
        break;
    case mul:
        // C: var1 = var2 * var3
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << " 	movl -" << bb->cfg->localSymbolTable.getOffset(params[1]) << "(%rbp), %eax\n";
        o << " 	" << "imull" << " -" << bb->cfg->localSymbolTable.getOffset(params[2]) << "(%rbp), %eax\n";
        o << " 	movl %eax, -" << bb->cfg->localSymbolTable.getOffset(params[0]) << "(%rbp)\n";
        break;
    case rmem:
        // C: dest = *addr
        // params[0] = var1 (dest)
        // params[1] = var2 (*addr)
        break;
    case wmem:
        /* code */
        break;
    case call:
        /* code */
        break;
    case cmp_eq:
        // var2 == var3 ?
        // C: var1 = (var2==var3);
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << " 	movl -" << bb->cfg->localSymbolTable.getOffset(params[1]) << "(%rbp), %eax\n";
        o << " 	" << "cmpl" << " -" << bb->cfg->localSymbolTable.getOffset(params[2]) << "(%rbp), %eax\n";
        o << " 	movl %eax, -" << bb->cfg->localSymbolTable.getOffset(params[0]) << "(%rbp)\n";
        o << " jne " << bb->exit_false->label << "\n";
        o << " jmp " << bb->exit_true->label << "\n";
        break;
        break;
    case cmp_lt: 
        // var2 < var3 ?
        // C: var1 = (var2==var3);
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << " 	movl -" << bb->cfg->localSymbolTable.getOffset(params[1]) << "(%rbp), %eax\n";
        o << " 	" << "cmpl" << " -" << bb->cfg->localSymbolTable.getOffset(params[2]) << "(%rbp), %eax\n";
        o << " 	movl %eax, -" << bb->cfg->localSymbolTable.getOffset(params[0]) << "(%rbp)\n";
        o << " jge " << bb->exit_false->label << "\n";
        o << " jmp " << bb->exit_true->label << "\n";
        break;
    case cmp_le:
        // var2 <= var3 ?
        // C: var1 = (var2==var3);
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << " 	movl -" << bb->cfg->localSymbolTable.getOffset(params[1]) << "(%rbp), %eax\n";
        o << " 	" << "cmpl" << " -" << bb->cfg->localSymbolTable.getOffset(params[2]) << "(%rbp), %eax\n";
        o << " 	movl %eax, -" << bb->cfg->localSymbolTable.getOffset(params[0]) << "(%rbp)\n";
        o << " jle " << bb->exit_true->label << "\n";
        o << " jmp " << bb->exit_false->label << "\n";
        break;                                    
    default:
        break;
    }

}