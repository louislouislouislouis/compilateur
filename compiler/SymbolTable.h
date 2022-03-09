#include "map"
#include "iostream"

typedef struct
{
    size_t line;
    size_t column;
} pos;

class SymbolTable
{
private:
    std::map<std::string, size_t> *offsetMap;
    std::map<std::string, pos> *posMap;
    std::map<std::string, bool> *usedMap;

    size_t offset;

    std::ostream *out;
    std::ostream *err;

    void checkVar(std::string id);

public:
    SymbolTable(std::ostream *_out, std::ostream *_err);
    ~SymbolTable();

    void add(std::string id, size_t offset, pos pos);
    int getOffset(std::string id);
    pos getPos(std::string id);
    void used(std::string id);
    void checkUse();
};
