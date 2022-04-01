#include "SymbolTable.h"

SymbolTable::SymbolTable(std::ostream &_out, std::ostream &_err) : out(_out),
                                                                   err(_err)
{
    // Initialize data structures
    offsetMap = new std::map<std::string, size_t>();
    posMap = new std::map<std::string, pos>();
    usedMap = new std::map<std::string, int>();
    tempIds = new std::vector<std::string>();

    // Initialize variables
    offset = 0;
}

SymbolTable::~SymbolTable()
{
    // Delete data structures
    delete offsetMap;
    delete posMap;
    delete usedMap;
    delete tempIds;
}

size_t SymbolTable::add(std::string id, std::string type, pos pos)
{
    // Calculate the offset and add it to the map
    std::map<std::string, uint> typeSize = {
        {"int", 4},
        {"char", 1},
        {"bool", 1}};
    auto size = typeSize[type];
    offset += size;
    auto p = offsetMap->emplace(id, offset);

    // Check if the variable was already declared
    if (!p.second)
    {
        err << "Error: " << id << " already declared" << std::endl;
        err << "\t at line " << pos.line << ":" << pos.column << std::endl;
        err << "First declared at " << posMap->at(id).line << ":" << posMap->at(id).line << std::endl;
        exit(1);
    }

    // Add relevant data to the maps
    posMap->emplace(id, pos);
    usedMap->emplace(id, 0);

    // Return the offset
    return offset;
}

size_t SymbolTable::addTemp(std::string id, std::string type, pos pos)
{
    auto r = this->add(id, type, pos);
    this->tempIds->push_back(id);
    this->usedMap->at(id) = 2;
    return r;
}

size_t SymbolTable::getOffset(std::string id, bool init)
{
    // Check if the variable is declared
    checkVar(id);

    // If the variable is not initialized and init is false, throw an error
    if (!init)
        checkInit(id);

    // Return the offset
    return offsetMap->at(id);
}

pos SymbolTable::getPos(std::string id)
{
    // Check if the variable is declared
    checkVar(id);

    // Return the position
    return posMap->at(id);
}

void SymbolTable::used(std::string id)
{
    // Set the used 'flag'

    usedMap->at(id) = 2;
}

void SymbolTable::checkUse()
{
    // Check if there are unused variables
    for (auto v : *usedMap)
    {
        // If the variable is not used, emit a warning
        if (v.second != 2)
        {
            err << "Warning: variable " << v.first << " is never used" << std::endl;
            err << "\t Declared at line " << posMap->at(v.first).line << ":" << posMap->at(v.first).column << std::endl;
        }
    }
}

void SymbolTable::checkVar(std::string id)
{
    // Check if the variable is declared
    if (offsetMap->count(id) == 0)
    {
        // Throw an error
        err << "Error: variable " << id << " is not declared" << std::endl;

        // Print the content of the map
        // for (auto it : *offsetMap)
        //     err << it.first << ": " << it.second << std::endl;

        exit(1);
    }
}

void SymbolTable::checkInit(std::string id)
{
    // Check if the variable is initialized
    if (usedMap->at(id) == 0)
    {
        err << "Error: " << id << " not initialized" << std::endl;
        exit(1);
    }
}