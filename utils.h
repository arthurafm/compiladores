#ifndef _UTILS_H_
#define _UTILS_H_

#define ERR_UNDECLARED  10
#define ERR_DECLARED    11
#define ERR_VARIABLE    20
#define ERR_FUNCTION    21

#define SIZE_TABLE 40

/* --- Estrutura da AST --- */

typedef struct lex_value {
    int num_line;
    char *token_type;
    char *token_value;
    char *type;
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
} ht_item;

typedef struct linkedlist {
    ht_item *item;
    struct linkedlist* next;
} linked_list;

typedef struct hashtable {
    ht_item **items;
    linked_list **overflow_buckets;
    int size;
    int count;
} hash_table;

typedef struct pilha_t{
	hash_table** escopos;
	int num_escopos;
	int escopos_ignorar;
} pilha;

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
unsigned long hash_function (unsigned char *str, int size);

/* Cria um item na hash table */
ht_item* create_item (char *key, int num_line, char *nature, char *type);

/* Cria uma hash table */
hash_table* create_table (int size);

/* Destrói um item de hash table */
void free_item (ht_item *item);

/* Destrói uma hash table */
void free_table (hash_table *table);

/* Insere em uma hash table */
void ht_insert (hash_table *table, char *key, int num_line, char *nature, char *type);

/* Busca em uma hash table */
ht_item* ht_search(hash_table *table, char *key);

/* Cria uma lista encadeada */
linked_list *create_list (); 

/* Insere na lista encadeada */
linked_list* list_insert (linked_list* list, ht_item* item);

/* Remove da lista encadeada */
ht_item* list_remove (linked_list* list);

/* Destrói uma lista encadeada */
void free_list (linked_list* list);

/* Cria lista encadeada dentro de hash table */
linked_list **create_overflow_buckets (hash_table *table);

/* Destrói lista encadeada dentro de hash table */
void free_overflow_buckets (hash_table* table);

/* Lida com colisões */
void handle_collision (hash_table *table, unsigned long index, ht_item *item);

/* Deleta de uma hash table */
void ht_delete (hash_table *table, char *key);

/* Printa uma hash table */
void print_table (hash_table *table);

pilha *criarPilha();
void addEscopo(pilha* pilha_atual);
void excluirEscopo(pilha* pilha_atual);
ht_item* encontrarItemPilha(pilha* pilha_atual, char *key);
void addItemEscopoOfsset(pilha* pilha_atual, int offset, char *key, int num_line, char *nature, char *type);
void addItemEscopo(pilha* pilha_atual, char *key, int num_line, char *nature, char *type);
void printaPilha(pilha *pilha_atual);

#endif //_UTILS_H_
