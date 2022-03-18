#include "map"
#include "iostream"
//Structure pour caracteriser la position d'un symbole:
//  -> Pour dire erreur a ligne 6:5 voila voila
typedef struct
{
    size_t line;
    size_t column;
} pos;
//La table des symboles
/*
id=text of the expression we analyse
id   offsetMap posMap usedMap
a -> 0         (5,6)  false
a+b -> 4         (6,12) true
...
*/
class SymbolTable
{
private:
    //offset en memoire pour chaque variable
    std::map<std::string, size_t> *offsetMap;
    std::map<std::string, pos> *posMap;
    std::map<std::string, bool> *usedMap;

    size_t offset;

    std::ostream *out;
    std::ostream *err;
    //pour voir si une variable est declarée
    void checkVar(std::string id);

public:
    //Constructeur:
    // ->init struct data
    // ->offset 0
    // ->Definition flux erreur et sortie
    SymbolTable(std::ostream *_out, std::ostream *_err);
    //Destructeur classique
    ~SymbolTable();
    
    //ajouter un symbole:
    // ->Fonction de la taille, id, et de la position dans le code
    // ->newOffset=offset+size, used a false, pos mis dans map
    void add(std::string id, size_t size, pos pos);
    void add(std::string id, size_t size, pos pos,bool state);
    //getter pour l'offset
    int getOffset(std::string id);
    //getter pour la position
    pos getPos(std::string id);
    //getter pour used
    void used(std::string id);
    //savoir quelles variables sont utilisées et on agit en fontion avec des warnings
    // ->Attention unused!!!
    void checkUse();
    bool isContained(std::string id);
};
