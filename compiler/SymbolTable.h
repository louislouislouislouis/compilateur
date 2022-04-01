#pragma once
#include <map>
#include <iostream>
#include <vector>

typedef struct
{
    size_t line;
    size_t column;
} pos;

class SymbolTable
{
public:
    SymbolTable(std::ostream &_out, std::ostream &_err);
    ~SymbolTable();
    ;
    /*
     * Add a variable to the symbol table
     * @param id: the name of the variable
     * @param size: the size of the variable in bytes
     * @param pos: the position of the variable in the source code
     * @return the offset of the variable
     *
     */
    size_t add(std::string id, std::string type, pos pos);

    /*
     * Add a temporary variable to the symbol table
     * @param id: the name of the variable
     * @param size: the size of the variable in bytes
     * @return the offset of the variable
     */
    size_t addTemp(std::string id, std::string type, pos pos = {0, 0});

    // Clear the temporary variables
    // void clearTemp();

    /*
     * Get the offset of the variable
     * @param id: the name of the variable
     * @param init: true to diable initialization check (used when assigning a value to a variable)
     */
    size_t getOffset(std::string id, bool init = false);

    // Get the position of the variable in the source code
    pos getPos(std::string id);

    // Mark the variable as used
    void used(std::string id);

    // Check for unused variables
    void checkUse();

private:
    // Offset of the declared variables
    std::map<std::string, size_t> *offsetMap;

    // Position of the declared variables in the source code
    std::map<std::string, pos> *posMap;

    // Used to determine if a variable is initialized and if it is used
    // 0: not initialized
    // 1: initialized
    // 2: used
    std::map<std::string, int> *usedMap;

    // Offset of the last declared variable
    size_t offset;

    // Offset of the last declared variable before a temp variable
    // size_t offsetBeforeTemp;

    // List of temp variables' names
    std::vector<std::string> *tempIds;

    // Streams
    std::ostream &out;
    std::ostream &err;

    // Check if the variable is declared
    void checkVar(std::string id);

    // Check if the variable is initialized
    void checkInit(std::string id);
};
