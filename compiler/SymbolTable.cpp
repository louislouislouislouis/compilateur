#include "SymbolTable.h"

SymbolTable::SymbolTable(std::ostream *_out, std::ostream *_err) : out(_out), err(_err)
{
    offsetMap = new std::map<std::string, size_t>();
    posMap = new std::map<std::string, pos>();
    usedMap = new std::map<std::string, bool>();

    offset = 0;
}

SymbolTable::~SymbolTable()
{
    delete offsetMap;
    delete posMap;
    delete usedMap;
}

void SymbolTable::add(std::string id, size_t size, pos pos)
{
    offset += size;
    auto p = offsetMap->emplace(id, offset);
    if (!p.second)
    {
        *err << "Error: " << id << " already declared" << std::endl;
        *err << "\t at line " << pos.line << ":" << pos.column << std::endl;
        *err << "First declared at" << posMap->at(id).line << ":" << posMap->at(id).line << std::endl;
        exit(1);
    }

    posMap->emplace(id, pos);
    usedMap->emplace(id, false);
}

void SymbolTable::used(std::string id)
{
    usedMap->at(id) = true;
}

int SymbolTable::getOffset(std::string id)
{
    checkVar(id);
    return offsetMap->at(id);
}

void SymbolTable::checkVar(std::string id)
{
    if (offsetMap->count(id) == 0)
    {
        *err << "Error: variable " << id << " is not declared" << std::endl;
        exit(1);
    }
}

void SymbolTable::checkUse()
{
    for (auto v : *usedMap)
    {
        if (!v.second)
        {
            *err << "Warning: variable " << v.first << " is never used" << std::endl;
            *err << "\t Declared at line " << posMap->at(v.first).line << ":" << posMap->at(v.first).column << std::endl;
        }
    }
}

pos SymbolTable::getPos(std::string id)
{
    checkVar(id);
    return posMap->at(id);
}
