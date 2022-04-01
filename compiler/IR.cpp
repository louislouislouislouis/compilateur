#include "IR.h"

void CFG::add_bb(BasicBlock *bb, bool cond)
{
    this->bbs.push_back(bb);
    if (this->current_bb)
    {
        if (cond)
        {
            this->current_bb->exit_true = bb;
        }
        else
        {
            this->current_bb->exit_false = bb;
        }
    }
}

BasicBlock::BasicBlock(CFG *cfg, string entry_label, std::string test_var)
{
    this->cfg = cfg;
    this->label = entry_label;
    this->test_var_name = test_var;
}
void BasicBlock::gen_asm(ostream &o)
{

    o << label << ":" << std::endl;
    for (auto instr : instrs)
        instr->gen_asm(o);
    if (exit_true == nullptr)
    {
        o << "\tjmp " << (*(this->cfg->getBbs()))[1]->label << std::endl;
    }
    else if (exit_false == nullptr)
    {
        o << "\tjmp " << exit_true->label << std::endl;
    }
    else
    {
        o << "\tcmp " << test_var_name << ", 1" << std::endl;
        o << "\tjne " << exit_false->label << std::endl;
        o << "\tjmp " << exit_true->label << std::endl;
    }
}

void BasicBlock::add_IRInstr(IRInstr::Operation op, std::string param)
{
    this->add_IRInstr(op, vector<std::string>{param});
}

void BasicBlock::add_IRInstr(IRInstr::Operation op, std::vector<std::string> params)
{
    this->instrs.push_back(new IRInstr(this, op, params));
}

void IRInstr::gen_asm(ostream &o)
{
    switch (op)
    {
    case ldconst:
        // C: var = const
        // params[0] = var
        // params[1] = const
        o << "\tmovl \t$" << params[1] << ", -" << this->bb->cfg->get_var_off(params[0]) << "(%rbp)" << endl;
        break;
    case copy:
        // C: var1 = var2
        // params[0] = var1
        // params[1] = var2
        o << "\tmovl \t-" << this->bb->cfg->get_var_off(params[1]) << "(%rbp), %eax" << endl;
        o << "\tmovl \t%eax, -" << this->bb->cfg->get_var_off(params[0]) << "(%rbp)" << endl;
        break;
    case add:
        // C: var1 = var2 + var3
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3       
        o << "\tmovl \t-" << this->bb->cfg->get_var_off(params[1]) << "(%rbp), %eax" << endl;
        o << "\taddl \t-" << this->bb->cfg->get_var_off(params[2]) << "(%rbp), %eax" << endl;
        o << "\tmovl \t%eax, -" << this->bb->cfg->get_var_off(params[0]) << "(%rbp)" << endl;
        break;
    case sub:
        // C: var1 = var2 - var3
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << " 	movl -" << this->bb->cfg->get_var_off(params[1]) << "(%rbp), %eax\n";
        o << " 	" << "subl" << " -" << this->bb->cfg->get_var_off(params[2]) << "(%rbp), %eax\n";
        o << " 	movl %eax, -" << this->bb->cfg->get_var_off(params[0]) << "(%rbp)\n";
        break;
    case mul:
        // C: var1 = var2 * var3
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << " 	movl -" << this->bb->cfg->get_var_off(params[1]) << "(%rbp), %eax\n";
        o << " 	" << "imull" << " -" << this->bb->cfg->get_var_off(params[2]) << "(%rbp), %eax\n";
        o << " 	movl %eax, -" << this->bb->cfg->get_var_off(params[0]) << "(%rbp)\n";
        break;
    case eq:
        // var2 == var3 ?
        // C: var1 = (var2==var3);
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << " 	movl -" << this->bb->cfg->get_var_off(params[1]) << "(%rbp), %eax\n";
        o << " 	" << "cmpl" << " -" << this->bb->cfg->get_var_off(params[2]) << "(%rbp), %eax\n";
        o << " 	movl %eax, -" << this->bb->cfg->get_var_off(params[0]) << "(%rbp)\n";
        o << " jne " << bb->exit_false->label << "\n";
        o << " jmp " << bb->exit_true->label << "\n";
        break;
        break;
    case lt: 
        // var2 < var3 ?
        // C: var1 = (var2==var3);
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << " 	movl -" << this->bb->cfg->get_var_off(params[1]) << "(%rbp), %eax\n";
        o << " 	" << "cmpl" << " -" << this->bb->cfg->get_var_off(params[2]) << "(%rbp), %eax\n";
        o << " 	movl %eax, -" << this->bb->cfg->get_var_off(params[0]) << "(%rbp)\n";
        o << " jge " << bb->exit_false->label << "\n";
        o << " jmp " << bb->exit_true->label << "\n";
        break;
    case leq:
        // var2 <= var3 ?
        // C: var1 = (var2==var3);
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << " 	movl -" << this->bb->cfg->get_var_off(params[1]) << "(%rbp), %eax\n";
        o << " 	" << "cmpl" << " -" << this->bb->cfg->get_var_off(params[2]) << "(%rbp), %eax\n";
        o << " 	movl %eax, -" << this->bb->cfg->get_var_off(params[0]) << "(%rbp)\n";
        o << " jle " << bb->exit_true->label << "\n";
        o << " jmp " << bb->exit_false->label << "\n";
        break;    
    case ret:
        o << "\tmovl \t-" << this->bb->cfg->get_var_off(params[0]) << "(%rbp), %eax" << endl;
    default:
        break;
    }
}