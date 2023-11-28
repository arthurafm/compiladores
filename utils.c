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

void set_root(tree_t *t) {
  arvore = t;
}

void yyerror(const char *s) {
	printf("In the line %d, the following error occurred: %s", get_line_number(), s);
}

tree_t *tree_new(lex_val label) {
  tree_t *ret = NULL;
  ret = calloc(1, sizeof(tree_t));

  if (ret != NULL){
    ret->label.num_line = label.num_line;
    ret->label.token_type = strdup(label.token_type);
    ret->label.token_value = strdup(label.token_value);
    ret->number_of_children = 0;
    ret->children = NULL;
  }

  return ret;
}

void tree_free(tree_t *tree) {
  if (tree != NULL) {
    int i;
    for (i = 0; i < tree->number_of_children; i++){
      tree_free(tree->children[i]);
    }
    free(tree->children);
    free(tree->label.token_type);
	  free(tree->label.token_value);
    free(tree);
  } else {
    printf("Erro: %s recebeu parâmetro tree = %p.\n", __FUNCTION__, tree);
  }
}

void tree_add_child(tree_t *tree, tree_t *child) {
  if (tree != NULL && child != NULL) {
    tree->number_of_children++;
    tree->children = realloc(tree->children, tree->number_of_children * sizeof(tree_t*));
    tree->children[tree->number_of_children-1] = child;
  }
}

static void imprimirLabels(tree_t *tree) {
	if (tree != NULL) {
		printf("%p [label=\"%s\"]\n", (void *) tree, tree->label.token_value);
		for (int i = 0; i < tree->number_of_children; i++){
      imprimirLabels(tree->children[i]);
    }
	}
}

static void imprimirArestas(tree_t *tree) {
	for (int i = 0; i < tree->number_of_children; i++) {
		if (tree->children[i] != NULL) {
			printf("%p, %p\n", (void *) tree, (void *) tree->children[i]);
		}
  }

  for (int i = 0; i < tree->number_of_children; i++) {
		if (tree->children[i] != NULL) {
			imprimirArestas(tree->children[i]);
		}
  }
}

void exporta (void *arvore) {
	if (arvore != NULL) {
		imprimirLabels(arvore);
		printf("\n\n");
		imprimirArestas(arvore);
	}
}