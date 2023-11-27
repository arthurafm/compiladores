#ifndef _UTILS_H_
#define _UTILS_H_

typedef struct lex_value {
    int num_line;
    char *token_type;
    char *token_value;
} lex_val;

typedef struct asd_tree {
  lex_val label;
  int number_of_children;
  struct asd_tree **children;
} asd_tree_t;

int get_line_number();

void yyerror(const char *s);

/*
 * Função asd_new, cria um nó sem filhos com o label informado.
 */
asd_tree_t *asd_new(lex_val label);

/*
 * Função asd_tree, libera recursivamente o nó e seus filhos.
 */
void asd_free(asd_tree_t *tree);

/*
 * Função asd_add_child, adiciona child como filho de tree.
 */
void asd_add_child(asd_tree_t *tree, asd_tree_t *child);

#endif //_UTILS_H_