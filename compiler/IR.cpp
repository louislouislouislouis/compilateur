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