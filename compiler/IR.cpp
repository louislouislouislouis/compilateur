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
    this->exit_true = nullptr;
    this->exit_false = nullptr;
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
        o << "\tcmpl\t$1,-" << this->cfg->get_var_off(test_var_name) << "(%rbp) " << std::endl;
        o << "\tjne\t" << exit_false->label << std::endl;
        o << "\tjmp\t" << exit_true->label << std::endl;
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
    std::string left = "", right = "", dest = "-" + this->bb->cfg->get_var_off(this->params[0]) + "(%rbp)";
    if (params.size() == 3)
    {
        if (params[1][0] == '$')
        {
            left = params[1];
            right = "-" + this->bb->cfg->get_var_off(params[2]) + "(%rbp)";
        }
        else if (params[2][0] == '$')
        {
            left = "-" + this->bb->cfg->get_var_off(params[1]) + "(%rbp)";
            right = params[2];
        }
        else
        {
            left = "-" + this->bb->cfg->get_var_off(params[1]) + "(%rbp)";
            right = "-" + this->bb->cfg->get_var_off(params[2]) + "(%rbp)";
        }
    }

    auto comp = [&](std::string op)
    {
        o << "\tmovl \t" << left << ", %eax" << endl;
        o << "\tcmpl \t" << right << ", %eax" << endl;
        o << "\tset" << op << " \t%al" << endl;
        o << "\tmovzbl \t%al, %eax" << endl;
        o << "\tmovl \t%eax, " << dest << endl;
    };

    auto bit = [&](std::string op)
    {
        o << "\tmovl \t" << left << ", %eax" << endl;
        o << "\t" << op << "l \t" << right << ", %eax" << endl;
        o << "\tmovl \t%eax, " << dest << endl;
    };

    switch (op)
    {
    case ldconst:
        // C: var = const
        // params[0] = var
        // params[1] = const
        o << "\tmovl \t$" << params[1] << ", " << dest << endl;
        break;
    case copy:
        // C: var1 = var2
        // params[0] = var1
        // params[1] = var2
        o << "\tmovl \t-" << this->bb->cfg->get_var_off(params[1]) << "(%rbp), %eax" << endl;
        o << "\tmovl \t%eax,  " << dest << endl;
        break;
    case add:

        o << "\tmovl \t" << left << ", %eax" << endl;
        o << "\taddl \t" << right << ", %eax" << endl;

        o << "\tmovl \t%eax, " << dest << endl;
        break;
    case sub:
        // C: var1 = var2 - var3
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << "\tmovl \t" << left << ", %eax" << endl;
        o << "\tsubl \t" << right << ", %eax" << endl;

        o << "\tmovl \t%eax, " << dest << endl;
        break;
    case mul:
        // C: var1 = var2 * var3
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << "\tmovl \t" << left << ", %eax" << endl;
        o << "\timull \t" << right << ", %eax" << endl;

        o << "\tmovl \t%eax, " << dest << endl;
        break;
    case neg:

        o << "\tnegl\t " << dest << std::endl;
        break;
    case not_:
        o << "	cmpl	$0, " << dest << std::endl;
        o << "	sete	%al\n";
        o << "	movzbl	%al, %eax\n";

        o << "	movl	%eax, " << dest << std::endl;
        break;  
    case ret:
        o << "\tmovl \t " << dest << ", %eax" << endl;
        break;
    case div:

        o << "\tmovl \t" << left << ", %eax" << std::endl;
        o << "\tcltd" << std::endl;
        if (right[0] == '$')
        {
            o << "\tmovl \t" << right << ", %ecx" << std::endl;
            right = "%ecx";
        }

        o << "\tidivl\t" << right << std::endl;

        o << "\tmovl\t %eax, " << dest << std::endl;

        break;
    case mod:
        o << "\tmovl \t" << left << ", %eax" << std::endl;
        o << "\tcltd" << std::endl;
        if (right[0] == '$')
        {
            o << "\tmovl \t" << right << ", %ecx" << std::endl;
            right = "%ecx";
        }

        o << "\tidivl\t" << right << std::endl;

        o << "\tmovl\t %edx, " << dest << std::endl;

        break;
    case eq:
        // var2 == var3 ?
        // C: var1 = (var2==var3);
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        comp("e");
        break;
    case neq:
        comp("ne");
        break;
    case lt:
        comp("l");
        break;
    case leq:
        // var2 <= var3 ?
        // C: var1 = (var2==var3);
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        comp("le");
        break;
    case gt:
        comp("g");
        break;
    case geq:
        comp("ge");
        break;
    case shiftL:
        o << "\tmovl \t" << left << ", %eax" <<endl;
        o<< "\tmov \t" << right << ", %cl" <<endl;
        o << "\t shl \t" << "%cl" << ", %eax" << endl;
        o << "\tmovl \t%eax, " << dest << endl;
        break;
    case shiftR:
        o << "\tmovl \t" << left << ", %eax" <<endl;
        o<< "\tmov \t" << right << ", %cl" <<endl;
        o << "\t shr \t" << "%cl" << ", %eax" << endl;
        o << "\tmovl \t%eax, " << dest << endl;
        break;    
    case and_:
        // TODO: implement
        break;
    case or_:
        // TODO: implement
        break;
    case band:
        bit("and");
        break;
    case bor:
        bit("or");
        break;
    case bxor:
        bit("xor");
        break;
    case bnot:
        o << "\tnotl\t " << dest << std::endl;
        break;
    default:
        break;
    }
}