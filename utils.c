#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#define ARQUIVO_SAIDA "saida.dot"

extern int yylineno;
extern void *arvore;

int get_line_number() {
	return yylineno;
}

void set_root(asd_tree_t *t) {
  arvore = t;
}

void yyerror(const char *s) {
	printf("In the line %d, the following error occurred: %s", get_line_number(), s);
}

asd_tree_t *asd_new(lex_val label)
{
  asd_tree_t *ret = NULL;
  ret = calloc(1, sizeof(asd_tree_t));
  if (ret != NULL){
    ret->label.num_line = label.num_line;
	ret->label.token_type = strdup(label.token_type);
	ret->label.token_value = strdup(label.token_value);
    ret->number_of_children = 0;
    ret->children = NULL;
  }
  return ret;
}

void asd_free(asd_tree_t *tree)
{
  if (tree != NULL){
    int i;
    for (i = 0; i < tree->number_of_children; i++){
      asd_free(tree->children[i]);
    }
    free(tree->children);
    free(tree->label.token_type);
	free(tree->label.token_value);
    free(tree);
  }else{
    printf("Erro: %s recebeu parâmetro tree = %p.\n", __FUNCTION__, tree);
  }
}

void asd_add_child(asd_tree_t *tree, asd_tree_t *child)
{
  if (tree != NULL && child != NULL){
    tree->number_of_children++;
    tree->children = realloc(tree->children, tree->number_of_children * sizeof(asd_tree_t*));
    tree->children[tree->number_of_children-1] = child;
  }else{
    printf("Erro: %s recebeu parâmetro tree = %p / %p.\n", __FUNCTION__, tree, child);
  }
}

static void _asd_print (FILE *foutput, asd_tree_t *tree, int profundidade)
{
  int i;
  if (tree != NULL){
    fprintf(foutput, "%d%*s: Nó '%s' tem %d filhos:\n", profundidade, profundidade*2, "", tree->label.token_value, tree->number_of_children);
    for (i = 0; i < tree->number_of_children; i++){
      _asd_print(foutput, tree->children[i], profundidade+1);
    }
  }else{
    printf("Erro: %s recebeu parâmetro tree = %p.\n", __FUNCTION__, tree);
  }
}

void exporta (void *arvore) {
	FILE *foutput = stderr;
	if (arvore != NULL){
		_asd_print(foutput, arvore, 0);
	} else {
		printf("Erro: %s recebeu parâmetro tree = %p.\n", __FUNCTION__, arvore);
	}
}