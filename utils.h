#ifndef _UTILS_H_
#define _UTILS_H_

#define ERR_UNDECLARED  10
#define ERR_DECLARED    11
#define ERR_VARIABLE    20
#define ERR_FUNCTION    21

#define SIZE_TABLE 40

/* --- Estrutura da Tabela de Símbolos --- */

typedef struct hash_table_item {
    char *key;
    int num_line;
    char *nature;
    char *type;
    int offset;
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

/* --- Estrutura da ILOC --- */
typedef struct iloc_operation {
    char *label; // Rótulo para desvio de fluxo, caso exista
    char *operation;
    char *input_1;
    char *input_2;
    char *output_1;
    char *output_2;
    short control_flux; // Variável booleana saber se é uma operação de fluxo de controle, 0 = não, 1 = sim
} iloc_op;

typedef struct iloc_program {
    iloc_op *operation;
    struct iloc_program *next_op;
} iloc_prog;

/* --- Estrutura da AST --- */

typedef struct lex_value {
    int num_line;
    char *token_type;
    char *token_value;
    char *type;
} lex_val;

typedef struct tree {
    lex_val info;
    char *reg;
    char *label;
    short isLast; // Flag para saber se é o último simple_command dentro de um command_block; 0 = Não, 1 = Sim
    iloc_prog *prog;
    int number_of_children;
    struct tree **children;
} tree_t;


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
char *inferencia_tipos(char *tipo1, char *tipo2);

/* Cria uma operação ILOC */
iloc_op* newILOCop (char *label,
                    char *operation,
                    char *input_1,
                    char *input_2,
                    char *output_1,
                    char *output_2,
                    short control_flux);

/* Deleta operação ILOC */
void freeILOCop (iloc_op *op);

/* Cria um programa ILOC */
iloc_prog* newILOCprog ();

/* Deleta programa ILOC */
void freeILOCprog (iloc_prog *prog);

/* Printa programa ILOC */
void printILOC (tree_t *t);

/* Fornece nomes de rótulos a serem utilizados na geração de código */
char* createLabel (int *counter);

/* Fornece nomes de registradores temporários a serem utilizados na geração de código */
char* createRegister (int *counter);

/* Concatena programas ILOC */
iloc_prog* addOpToProg (iloc_prog *prog, iloc_op *op);

/* Checa contexto do identificador, se é global ou local; 0 = global, 1 = local, 2 = não encontrou */
char* checkContext (pilha* pilha_atual, char *key);

/* Acha a primeira operação ILOC a ser realizada em um comando simples */
iloc_op* findFirstOp (tree_t *t);

/* Acha a última operação ILOC a ser realizada em um bloco de comando */
tree_t* findLastProg (tree_t *t);

/* Procura a main dentro da AST */
tree_t* findMainStart (tree_t *t);

/* Poda a AST mantendo somente a função no nodo raiz */
void pruneFunction (tree_t *t);

#endif //_UTILS_H_
