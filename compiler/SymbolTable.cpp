#include "SymbolTable.h"

SymbolTable::SymbolTable(std::ostream *_out, std::ostream *_err) : out(_out), err(_err)
{
    /*
    stores for a variable given by the string the position of the variable on the stack
    by using the offest parameter.
    eg: int var --> offsetMap->emplace(var, 4)
    eg:char foo --> offsetMap->emplace(foo, 1)
    */
    offsetMap = new std::map<std::string, size_t>();
    /*
    stores for each variable given by a string the linenumber in the file.c
    pos is defined in the .h file with a line and a colum attribute.
    */
    posMap = new std::map<std::string, pos>();
    usedMap = new std::map<std::string, bool>();
    /*
    points to the next free block of memory of the stack
    */
    offset = 0;
}

/*
constructor creates new structs
*/
SymbolTable::~SymbolTable()
{
    delete offsetMap;
    delete posMap;
    delete usedMap;
}
/*
stores the name, size of the new variable and saves it's offset on the stack
*/
void SymbolTable::add(std::string id, size_t size, pos pos)
{
    offset += size;  // strange because now the offset of a variable points to the end of the variable...?
    auto p = offsetMap->emplace(id, offset);
    if (!p.second)
    {
        *err << "Error: " << id << " already declared" << std::endl;
        *err << "\t at line " << pos.line << ":" << pos.column << std::endl;
        *err << "First declared at " << posMap->at(id).line << ":" << posMap->at(id).line << std::endl;
        exit(1);
    }

    posMap->emplace(id, pos);
    usedMap->emplace(id, false);
}

/*
sets the variable to used which means that a value has been assiged to it
*/
void SymbolTable::used(std::string id)
{
    usedMap->at(id) = true;
}

/*
retruns the offset on the stack of the given variable
stops the program if tha variable is not declared
*/
int SymbolTable::getOffset(std::string id)
{
    checkVar(id);
    return offsetMap->at(id);
}

/*
checks if the variable that should be accessed is declared, 
stops the program if tha variable is not declared
*/
void SymbolTable::checkVar(std::string id)
{
    if (offsetMap->count(id) == 0)
    {
        *err << "Error: variable " << id << " is not declared" << std::endl;
        exit(1);
    }
}

/*
checks if any variable has never been used 
returs a warning if not
*/
void SymbolTable::checkUse()
{
    for (auto v : *usedMap)
    {
        if (!v.second) //second argument in <string,bool> is false, so not used
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
