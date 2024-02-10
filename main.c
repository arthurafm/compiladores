#include <stdio.h>
#include "utils.h"
extern int yyparse(void);
extern int yylex_destroy(void);
void *arvore = NULL;
pilha *stack = NULL;
int labelCounter = 1;
int registerCounter = 1; 
void exporta (void *arvore);

extern int yy_flex_debug;

int main (int argc, char **argv)
{
	yy_flex_debug = 1;
  stack = criarPilha();
  int ret = yyparse();
  // exporta (arvore);
  // printILOC(arvore);
  generateAsm(arvore);
  yylex_destroy();
  return ret;
}
