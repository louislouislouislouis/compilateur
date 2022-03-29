#include "IR.h"

void CFG::add_bb(BasicBlock *bb) {
    this->bbs.push_back(bb);
    if (this->current_bb) {
        this->current_bb->exit_true = bb;
    }
}