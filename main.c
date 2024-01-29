#include <stdio.h>
#include "utils.h"
extern int yyparse(void);
extern int yylex_destroy(void);
void *arvore = NULL;
pilha *stack = NULL;
iloc_prog *prog = NULL;
int labelCounter = 1;
int registerCounter = 1; 
void exporta (void *arvore);
int main (int argc, char **argv)
{
  stack = criarPilha();
  int ret = yyparse(); 
  exporta (arvore);
  printILOC(prog);
  yylex_destroy();
  return ret;
}
