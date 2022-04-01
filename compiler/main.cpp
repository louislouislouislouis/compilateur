#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "antlr4-runtime.h"
#include "generated/ifccLexer.h"
#include "generated/ifccParser.h"
#include "generated/ifccBaseVisitor.h"

#include "CodeGenVisitor.h"
#include "IRGenVisitor.h"
#include "IR.h"

int main(int argn, const char **argv)
{
  std::stringstream in;
  if (argn == 2)
  {
    std::ifstream lecture(argv[1]);
    in << lecture.rdbuf();
  }
  else
  {
    std::cerr << "usage: ifcc path/to/file.c" << std::endl;
    exit(1);
  }

  antlr4::ANTLRInputStream input(in.str());

  ifccLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  tokens.fill();

  ifccParser parser(&tokens);
  antlr4::tree::ParseTree *tree = parser.axiom();

  if (parser.getNumberOfSyntaxErrors() != 0)
  {
    std::cerr << "error: syntax error during parsing" << std::endl;
    exit(1);
  }

  IRGenVisitor o = IRGenVisitor();
  o.visit(tree);

  // CodeGenVisitor v;
  // v.visit(tree);
  return 0;
}
