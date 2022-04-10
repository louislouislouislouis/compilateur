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
    this->instrs = std::vector<IRInstr *>();
}
void BasicBlock::gen_asm(ostream &o)
{

    o << label << ":" << std::endl;
    for (auto instr : instrs)
        instr->gen_asm(o);
    if (instrs.size() != 0 && instrs.back()->getOp() == IRInstr::Operation::ret)
        return;

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
        o << "\tcmpb\t$1,-" << this->cfg->get_var_off(test_var_name) << "(%rbp) " << std::endl;
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
    std::map<uint, char> typeSize = {
        {4, 'l'},
        {2, 'w'},
        {1, 'b'},
        {8, 'q'}};
    char suffix = 0;
    std::string left = "", right = "", dest = "";
    if (op != IRInstr::jmp)
    {
        suffix = typeSize[this->bb->cfg->getST()->getSize(this->params[0])];
        dest = "-" + this->bb->cfg->get_var_off(this->params[0]) + "(%rbp)";
    }
    uint leftSize = 8, rightSize = 8;
    if (params.size() == 3)
    {
        if (params[1][0] == '$')
        {
            left = params[1];
            right = "-" + this->bb->cfg->get_var_off(params[2]) + "(%rbp)";

            rightSize = this->bb->cfg->getST()->getSize(params[2]);
        }
        else if (params[2][0] == '$')
        {
            left = "-" + this->bb->cfg->get_var_off(params[1]) + "(%rbp)";
            right = params[2];

            leftSize = this->bb->cfg->getST()->getSize(params[1]);
        }
        else
        {
            left = "-" + this->bb->cfg->get_var_off(params[1]) + "(%rbp)";
            right = "-" + this->bb->cfg->get_var_off(params[2]) + "(%rbp)";
            leftSize = this->bb->cfg->getST()->getSize(params[1]);
            rightSize = this->bb->cfg->getST()->getSize(params[2]);
        }
    }

    auto comp = [&](std::string op)
    {
        o << "\tmov" << suffix << " \t" << left << ", %eax" << endl;
        o << "\tcmp" << suffix << " \t" << right << ", %eax" << endl;
        o << "\tset" << op << " \t%al" << endl;
        o << "\tmovzb" << suffix << " \t%al, %eax" << endl;
        o << "\tmov" << suffix << " \t%eax, " << dest << endl;
    };

    auto bit = [&](std::string op)
    {
        o << "\tmov" << suffix << " \t" << left << ", %eax" << endl;
        o << "\t" << op << "l \t" << right << ", %eax" << endl;
        o << "\tmov" << suffix << " \t%eax, " << dest << endl;
    };

    switch (op)
    {
    case ldconst:
        // C: var = const
        // params[0] = var
        // params[1] = const
        o << "\tmov" << suffix << " \t$" << params[1] << ", " << dest << endl;
        break;
    case jmp:
        // C: goto label
        // params[0] = label
        o << "\tjmp " << params[0] << std::endl;
        break;
    case copy:
        // C: var1 = var2
        // params[0] = var1
        // params[1] = var2
        o << "\tmov" << suffix << " \t-" << this->bb->cfg->get_var_off(params[1]) << "(%rbp), %eax" << endl;
        o << "\tmov" << suffix << " \t%eax,  " << dest << endl;
        break;
    case add:

        o << "\tmov" << suffix << " \t" << left << ", %eax" << endl;
        o << "\tadd" << suffix << " \t" << right << ", %eax" << endl;

        o << "\tmov" << suffix << " \t%eax, " << dest << endl;
        break;
    case sub:
        // C: var1 = var2 - var3
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << "\tmov" << suffix << " \t" << left << ", %eax" << endl;
        o << "\tsub" << suffix << " \t" << right << ", %eax" << endl;

        o << "\tmov" << suffix << " \t%eax, " << dest << endl;
        break;
    case mul:
        // C: var1 = var2 * var3
        // params[0] = var1
        // params[1] = var2
        // params[2] = var3
        o << "\tmov" << suffix << " \t" << left << ", %eax" << endl;
        o << "\timul" << suffix << " \t" << right << ", %eax" << endl;

        o << "\tmov" << suffix << " \t%eax, " << dest << endl;
        break;
    case neg:

        o << "\tneg" << suffix << "\t " << dest << std::endl;
        break;
    case not_:
        o << "	cmp" << suffix << "	$0, " << dest << std::endl;
        o << "	sete	%al\n";
        o << "	movzb" << suffix << "	%al, %eax\n";

        o << "	mov" << suffix << "	%eax, " << dest << std::endl;
        break;
    case ret:
        o << "\tmov" << suffix << " \t " << dest << ", %eax" << endl;
        o << "\tjmp " << (*(this->bb->cfg->getBbs()))[1]->label << std::endl;
        break;
    case div:
        if (leftSize == 1)
            o << "\tmovsbl\t" << left << ", %eax" << std::endl;
        else
            o << "\tmov" << suffix << "\t" << left << ", %eax" << std::endl;

        if (rightSize == 1)
            o << "\tmovsbl\t" << right << ", %ecx" << std::endl;
        else
            o << "\tmov" << suffix << "\t" << right << ", %ecx" << std::endl;
        o << "\tcltd" << std::endl;
        o << "\tidivl\t %ecx" << std::endl;

        o << "\tmov" << suffix << "\t %eax, " << dest << std::endl;

        break;
    case mod:
        if (leftSize == 1)
            o << "\tmovsbl\t" << left << ", %eax" << std::endl;
        else
            o << "\tmov" << suffix << "\t" << left << ", %eax" << std::endl;

        if (rightSize == 1)
            o << "\tmovsbl\t" << right << ", %ecx" << std::endl;
        else
            o << "\tmov" << suffix << "\t" << right << ", %ecx" << std::endl;
        o << "\tcltd" << std::endl;
        o << "\tidivl\t %ecx" << std::endl;

        o << "\tmov" << suffix << "\t %edx, " << dest << std::endl;

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
        o << "\tmovl \t" << left << ", %eax" << endl;
        o << "\tmov \t" << right << ", %cl" << endl;
        o << "\t shl \t"
          << "%cl"
          << ", %eax" << endl;
        o << "\tmovl \t%eax, " << dest << endl;
        break;
    case shiftR:
        o << "\tmovl \t" << left << ", %eax" << endl;
        o << "\tmov \t" << right << ", %cl" << endl;
        o << "\t shr \t"
          << "%cl"
          << ", %eax" << endl;
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
        o << "\tnot" << suffix << "\t " << dest << std::endl;
        break;
    default:
        break;
    }
}