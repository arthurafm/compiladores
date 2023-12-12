#include <stdio.h>
#include "utils.h"
extern int yyparse(void);
extern int yylex_destroy(void);
void *arvore = NULL;
pilha *stack = NULL;
void exporta (void *arvore);
int main (int argc, char **argv)
{
  stack = criarPilha();
  int ret = yyparse(); 
  exporta (arvore);
  yylex_destroy();
  return ret;
}
