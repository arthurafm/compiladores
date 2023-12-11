#ifndef _UTILS_H_
#define _UTILS_H_

#define ERR_UNDECLARED  10
#define ERR_DECLARED    11
#define ERR_VARIABLE    20
#define ERR_FUNCTION    21

/* --- Estrutura da AST --- */

typedef struct lex_value {
    int num_line;
    char *token_type;
    char *token_value;
    char *token_type;
} lex_val;

typedef struct tree {
    lex_val info;
    int number_of_children;
    struct tree **children;
} tree_t;

/* --- Estrutura da Tabela de Símbolos --- */

typedef struct hash_table_item {
    char *key;
    int num_line;
    char *nature;
    char *type;
    char *token_value;
} ht_item;

typedef struct hashtable {
    ht_item **items;
    int size;
    int count;
} hash_table;

typedef struct linkedlist {
    ht_item *item;
    struct linkedlist* next;
} linked_list;

/* Retorna o número da linha atual do código-fonte */
int get_line_number();

/* Printa uma mensagem de erro */
void yyerror(const char *s);

/* --- Árvore Sintática Abstrata --- */

/* Seta raiz da árvore */
void set_root(tree_t *t);

/* Cria uma nova árvore */
tree_t *tree_new(lex_val info);

/* Destrói uma árvore e seus filhos */
void tree_free(tree_t *tree);

/* Adiciona child como filho de tree */
void tree_add_child(tree_t *tree, tree_t *child);

/* --- Tabela de Símbolos --- */

/* Função de hash */
unsigned long hash_function (unsigned char *str);

/* Cria um item na hash table */
ht_item* create_item (char *key, int num_line, char *nature, char *type, char *token_value);

/* Cria uma hash table */
hash_table* create_table (int size);

/* Destrói um item de hash table */
void free_item (ht_item* item)

/* Destrói uma hash table */
void free_table (hash_table *table);

/* Insere em uma hash table */
void ht_insert (hash_table *table, char *key, int num_line, char *nature, char *type, char *token_value);

/* Busca em uma hash table */
char* ht_search(hash_table *table, char *key);

#endif //_UTILS_H_