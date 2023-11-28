#ifndef _UTILS_H_
#define _UTILS_H_

typedef struct lex_value {
    int num_line;
    char *token_type;
    char *token_value;
} lex_val;

typedef struct tree {
  lex_val label;
  int number_of_children;
  struct tree **children;
} tree_t;

int get_line_number();

void set_root(tree_t *t);

void yyerror(const char *s);

/*
 * Função tree_new, cria um nó sem filhos com o label informado.
 */
tree_t *tree_new(lex_val label);

/*
 * Função tree, libera recursivamente o nó e seus filhos.
 */
void tree_free(tree_t *tree);

/*
 * Função tree_add_child, adiciona child como filho de tree.
 */
void tree_add_child(tree_t *tree, tree_t *child);

#endif //_UTILS_H_