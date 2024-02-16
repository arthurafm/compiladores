#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

extern int yylineno;
extern void *arvore;
extern pilha *stack;
extern asm_prog *prog;

int get_line_number() {
    return yylineno;
}

void yyerror(const char *s) {
	  printf("In the line %d, the following error occurred: %s.\n", get_line_number(), s);
}

/* --- Árvore Sintática Abstrata --- */

void set_root(tree_t *t) {
  arvore = t;
}

tree_t *tree_new(lex_val info) {
  tree_t *n = NULL;
  n = calloc(1, sizeof(tree_t));

  if (n != NULL){
    n->info.num_line = info.num_line;
    n->info.token_type = strdup(info.token_type);
    n->info.token_value = strdup(info.token_value);
    n->info.type = strdup(info.type);
    n->reg = NULL;
    n->isLast = 0;
    n->label = NULL;
    n->jumpTo = NULL;

    n->prog = newILOCprog();

    n->number_of_children = 0;
    n->children = NULL;
  }

  return n;
}

void tree_free(tree_t *tree) {
  if (tree != NULL) {
    int i;
    for (i = 0; i < tree->number_of_children; i++){
      tree_free(tree->children[i]);
    }
    free(tree->children);
    free(tree->reg);
    free(tree->label);
    free(tree->jumpTo);
    free(tree->info.token_type);
	free(tree->info.token_value);
    free(tree->info.type);

    freeILOCprog(tree->prog);

    free(tree);
  } else {
    printf("Erro: %s recebeu parâmetro tree = %p.\n", __FUNCTION__, tree);
  }
}

void tree_add_child(tree_t *tree, tree_t *child) {
  if (tree != NULL) {
    tree->number_of_children++;
    tree->children = realloc(tree->children, tree->number_of_children * sizeof(tree_t*));
    tree->children[tree->number_of_children-1] = child;
  }
}

static void imprimirLabels(tree_t *tree) {
	if (tree != NULL) {
        if (strcmp(tree->info.token_value, strdup("")) != 0) {
            printf("%p [label=\"%s\"]\n", (void *) tree, tree->info.token_value);
        }
		for (int i = 0; i < tree->number_of_children; i++){
            imprimirLabels(tree->children[i]);
        }
	}
}

static void imprimirArestas(tree_t *tree) {
	for (int i = 0; i < tree->number_of_children; i++) {
		if (tree->children[i] != NULL) {
            if (strcmp(tree->children[i]->info.token_value, strdup("")) != 0) {
                printf("%p, %p\n", (void *) tree, (void *) tree->children[i]);
            }
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

/* --- Tabela de Símbolos --- */

/* Algoritmo djb2, disponível em http://www.cse.yorku.ca/~oz/hash.html */
unsigned long hash_function (unsigned char *str, int size) {
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c;

    return hash % size;
}

ht_item* create_item (char *key, int num_line, char *nature, char *type) {

    ht_item *item = (ht_item *) malloc(sizeof(ht_item));
    item->key = (char *) malloc(strlen(key) + 1);
    item->nature = (char *) malloc(strlen(nature) + 1);
    item->type = (char *) malloc(strlen(type) + 1);

    strcpy(item->key, key);
    item->num_line = num_line;
    strcpy(item->nature, nature);
    strcpy(item->type, type);

    return item;
}

hash_table* create_table (int size) {
    
    hash_table *table = (hash_table *) malloc(sizeof(hash_table));
    table->items = (ht_item **) calloc(size, sizeof(ht_item*));
    table->size = size;
    table->count = 0;

    for (int i = 0; i < size; i++) {
        table->items[i] = NULL;
    }

    table->overflow_buckets = create_overflow_buckets(table);

    return table;
}

void free_item (ht_item *item) {

    free(item->key);
    free(item->nature);
    free(item->type);

    free(item);
}

void free_table (hash_table *table) {

    for (int i = 0; i < table->size; i++) {
        ht_item *item = table->items[i];
        if (item != NULL) {
            free(item);
        }
    }

    free_overflow_buckets(table);
    free(table->items);
    free(table);
}

void ht_insert (hash_table *table, char *key, int num_line, char *nature, char *type) {
    
    ht_item *item = create_item(key, num_line, nature, type);
    
    int index = hash_function(key, table->size);

    ht_item *cur_item = table->items[index];

    if (cur_item == NULL) {
        if (table->count == table->size) {
            printf("Insert error: hash table is full.\n");
            free(item);
            return;
        }

        table->items[index] = item;
        item->offset = table->count;
        table->count++;
    }
    else {
        if (strcmp(cur_item->key, key) == 0) {
            /* Erro -> Identificador já foi declarado */
            printf("The identifier \'%s\', in the line %d, was already declared in line %d.", key, num_line, cur_item->num_line);
            exit(ERR_DECLARED);
        }
        else {
            handle_collision(table, index, item);
            return;
        }
    }
    
}

ht_item* ht_search (hash_table *table, char *key) {
    int index = hash_function(key, table->size);
    ht_item *item = table->items[index];
    linked_list *head = table->overflow_buckets[index];

    if (item != NULL) {
        if (strcmp(item->key, key) == 0) {
            return item;
        }
        if (head == NULL) {
            return NULL;
        }

        item = head->item;
        head = head->next;
    }

    return NULL;
}

linked_list *create_list () {
    linked_list* list = (linked_list *) malloc(sizeof(linked_list));

    return list;
}

linked_list* list_insert (linked_list* list, ht_item* item) {
    if (!list)
    {
        linked_list* head = create_list();
        head->item = item;
        head->next = NULL;
        list = head;
        return list;
    }
    else if (list->next == NULL)
    {
        linked_list* node = create_list();
        node->item = item;
        node->next = NULL;
        list->next = node;
        return list;
    }

    linked_list* temp = list;

    while (temp->next->next)
    {
        temp = temp->next;
    }

    linked_list* node = create_list();
    node->item = item;
    node->next = NULL;
    temp->next = node;
    return list;
}

ht_item* list_remove (linked_list* list) {
    if (!list)
        return NULL;

    if (!list->next)
        return NULL;

    linked_list* node = list->next;
    linked_list* temp = list;
    temp->next = NULL;
    list = node;
    ht_item* it = NULL;
    memcpy(temp->item, it, sizeof(ht_item));
    free(temp->item->key);
    free(temp->item->nature);
    free(temp->item->type);
    free(temp->item);
    free(temp);
    return it;
}

void free_list (linked_list* list) {
    linked_list* temp = list;

    while (list)
    {
        temp = list;
        list = list->next;
        free(temp->item->key);
        free(temp->item->nature);
        free(temp->item->type);
        free(temp->item);
        free(temp);
    }
}

linked_list **create_overflow_buckets (hash_table *table) {
    linked_list **buckets = (linked_list**) calloc(table->size, sizeof(linked_list*));

    for (int i = 0; i < table->size; i++)
        buckets[i] = NULL;

    return buckets;
}

void free_overflow_buckets (hash_table* table) {
    linked_list **buckets = table->overflow_buckets;

    for (int i = 0; i < table->size; i++)
        free_list(buckets[i]);

    free(buckets);
}

void handle_collision (hash_table *table, unsigned long index, ht_item *item) {
    linked_list *head = table->overflow_buckets[index];

    if (head == NULL)
    {
        head = create_list();
        head->item = item;
        table->overflow_buckets[index] = head;
        return;
    }
    else {
        table->overflow_buckets[index] = list_insert(head, item);
        return;
    }
}

void ht_delete(hash_table *table, char *key) {
    int index = hash_function(key, table->size);
    ht_item *item = table->items[index];
    linked_list *head = table->overflow_buckets[index];

    if (item == NULL)
    {
        return;
    }
    else {
        if (head == NULL && strcmp(item->key, key) == 0)
        {
            table->items[index] = NULL;
            free_item(item);
            table->count--;
            return;
        }
        else if (head != NULL)
        {
            if (strcmp(item->key, key) == 0)
            {
                free_item(item);
                linked_list *node = head;
                head = head->next;
                node->next = NULL;
                table->items[index] = create_item(node->item->key, node->item->num_line, node->item->nature, node->item->type);
                free_list(node);
                table->overflow_buckets[index] = head;
                return;
            }

            linked_list *curr = head;
            linked_list *prev = NULL;

            while (curr)
            {
                if (strcmp(curr->item->key, key) == 0)
                {
                    if (prev == NULL)
                    {
                        free_list(head);
                        table->overflow_buckets[index] = NULL;
                        return;
                    }
                    else
                    {
                        prev->next = curr->next;
                        curr->next = NULL;
                        free_list(curr);
                        table->overflow_buckets[index] = head;
                        return;
                    }
                }

                curr = curr->next;
                prev = curr;
            }
        }
    }
}

pilha *criarPilha(){
    pilha *nova_pilha = (pilha *) malloc(sizeof(pilha));
    nova_pilha->num_escopos = 0;
    nova_pilha->escopos_ignorar = 0;
    nova_pilha->escopos = NULL;
    addEscopo(nova_pilha);
    return nova_pilha;
}

void addEscopo(pilha* pilha_atual){
	if(pilha_atual->escopos_ignorar == 0){
		pilha_atual->num_escopos++;
		pilha_atual->escopos = realloc(pilha_atual->escopos, pilha_atual->num_escopos * sizeof(hash_table*));
		hash_table *temp = create_table(SIZE_TABLE);
		pilha_atual->escopos[pilha_atual->num_escopos - 1] = temp;
	}
	else{
		pilha_atual->escopos_ignorar--;
	}
}

void excluirEscopo(pilha* pilha_atual){
    	if(pilha_atual->num_escopos > 1){
		hash_table *hash_table_excluir = pilha_atual->escopos[pilha_atual->num_escopos - 1];
		free_table(hash_table_excluir);
		pilha_atual->num_escopos--;
		pilha_atual->escopos = realloc(pilha_atual->escopos, pilha_atual->num_escopos * sizeof(hash_table*));
	}
}

ht_item* encontrarItemPilha(pilha* pilha_atual, char *key){
	int contador = pilha_atual->num_escopos;
	hash_table *hash_table_atual;
	while(contador >= 1){
		hash_table_atual = pilha_atual->escopos[contador - 1];
		if(ht_search(hash_table_atual, key) != NULL){
			return ht_search(hash_table_atual, key);
		}
		contador--;
	}
	return NULL;
}

void addItemEscopoOfsset(pilha* pilha_atual, int offset, char *key, int num_line, char *nature, char *type){
	if(pilha_atual->num_escopos - 1 - offset >= 0){
		hash_table *hash_table_atual;
		hash_table_atual = pilha_atual->escopos[pilha_atual->num_escopos - 1 - offset];
		ht_item *item_atual = encontrarItemPilha(pilha_atual, key);
		if (item_atual != NULL) {
		    /* Erro -> Identificador já foi declarado */
		    printf("The identifier \'%s\', in the line %d, was already declared in line %d.", key, num_line, item_atual->num_line);
		    exit(ERR_DECLARED);
		}
		ht_insert(hash_table_atual, key, num_line, nature, type);
	}
}

void addItemEscopo(pilha* pilha_atual, char *key, int num_line, char *nature, char *type){
	addItemEscopoOfsset(pilha_atual, 0, key, num_line, nature, type);
}

void print_table (hash_table *table) {
     printf("\nHash Table\n-------------------\n");
    for (int i = 0; i < table->size; i++)
    {
        if (table->items[i] != NULL)
        {
            printf("Chave: %s, Num_line: %d, Natureza: %s, Tipo: %s\n", table->items[i]->key, table->items[i]->num_line, table->items[i]->nature, table->items[i]->type);
        }
    }
    printf("-------------------\n\n");
}

void printaPilha(pilha *pilha_atual) {
    int count_closure = 1;
    printf("\nINICIO DE PILHA\n");
    for (int i = 0; i < pilha_atual->num_escopos; i++) {
        printf("ESCOPO (%d):\n", count_closure);
        print_table(pilha_atual->escopos[i]);
        count_closure++;
    }
    printf("\nFIM DE PILHA\n");
}

char *inferencia_tipos(char *tipo1, char *tipo2){
	char *retorno;
	if(strcmp(tipo1, tipo2) == 0){
		retorno = strdup(tipo1);
	}
	else if(strcmp("float", tipo1) == 0 || strcmp("float", tipo2) == 0){
		retorno = strdup("float");
	}
	else if(strcmp("int", tipo1) == 0 || strcmp("int", tipo2) == 0){
		retorno = strdup("int");
	}
	else{
		retorno = strdup("bool");
	}
	
	return retorno;
}

/* --- Geração de código intermediário --- */

iloc_op* newILOCop (char *label,
                    char *operation,
                    char *input_1,
                    char *input_2,
                    char *output_1,
                    char *output_2,
                    short control_flux) {
    iloc_op *n = malloc(sizeof(iloc_op));

    n->label = label;
    n->operation = operation;
    n->input_1 = input_1;
    n->input_2 = input_2;
    n->output_1 = output_1;
    n->output_2 = output_2;
    n->control_flux = control_flux;

    return n;
}

void freeILOCop (iloc_op* op) {
    free(op->label);
    free(op->operation);
    free(op->input_1);
    free(op->input_2);
    free(op->output_1);
    free(op->output_2);
    free(op);
}

iloc_prog* newILOCprog () {
    
    iloc_prog* n = malloc(sizeof(iloc_prog));

    n->operation = newILOCop(
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        -1
    );

    n->next_op = NULL;

    return n;
}

void freeILOCprog (iloc_prog *prog) {
    iloc_prog* temp = prog;

    while (prog) {
        temp = prog;
        prog = prog->next_op;
        freeILOCop(temp->operation);
        free(temp);
    }
}

short inMain = 0; // Se na main, inMain = 1, caso contrário, inMain = 0
void printILOC (tree_t *tr) {
    if (tr != NULL) {

        tree_t *t;
        if (inMain == 0) {
            t = findMainStart(tr);
            pruneFunction(t);
        }
        else {
            t = tr;
        }

        if (t != NULL) {
            for (int i = 0; i < t->number_of_children; i++) {
                if (t->children[i] != NULL) {
                    if (strcmp(t->children[i]->info.token_type, strdup("comando simples")) != 0) {
                        printILOC (t->children[i]);
                    }
                }
            }
            if (t->prog != NULL) {
                iloc_prog *cursor = t->prog;
                while (cursor != NULL) {
                    if (cursor->operation != NULL) {
                        if (cursor->operation->label != NULL) {
                            printf("%s: ", cursor->operation->label);
                        }
                        if (cursor->operation->operation != NULL) {
                            printf("%s", cursor->operation->operation);
                        }
                        if (cursor->operation->input_1 != NULL) {
                            printf(" %s", cursor->operation->input_1);
                        }
                        if (cursor->operation->input_2 != NULL) {
                            printf(", %s", cursor->operation->input_2);
                        }
                        if (cursor->operation->output_1 != NULL) {
                            if (cursor->operation->control_flux == 0) { // Se não for uma operação de controle de fluxo
                                printf(" => %s", cursor->operation->output_1);
                            }
                            else if (cursor->operation->control_flux == 1) { // Se for uma operação de controle de fluxo
                                printf(" -> %s", cursor->operation->output_1);
                            }
                        }
                        if (cursor->operation->output_2 != NULL) {
                            printf(", %s", cursor->operation->output_2);
                        }
                        if (cursor->operation->operation != NULL) {
                            printf("\n");
                        }
                    }
                    cursor = cursor->next_op;
                }
            }
            for (int i = 0; i < t->number_of_children; i++) {
                if (t->children[i] != NULL) {
                    if (strcmp(t->children[i]->info.token_type, strdup("comando simples")) == 0) {
                        printILOC (t->children[i]);
                    }
                }
            }
        }
    }
}

char* createLabel (int *counter) {
    char *counterString = malloc(sizeof(char) * 8);
    sprintf(counterString, "L%d", *counter);
    *counter += 1;
    return strdup(counterString);
}

char* createRegister (int *counter) {
    char *counterString = malloc(sizeof(char) * 8);
    sprintf(counterString, "r%d", *counter);
    *counter += 1;
    return strdup(counterString);
}

iloc_prog* addOpToProg (iloc_prog *prog, iloc_op *op) {
    if (prog->operation->operation == NULL) {
        iloc_prog *n_prog = malloc(sizeof(iloc_prog));
        n_prog->operation = op;
        n_prog->next_op = NULL;
        prog = n_prog;
    }
    else {
        iloc_prog *cursor = prog;

        while (cursor->next_op != NULL) {
            cursor = cursor->next_op;
        }

        iloc_prog *n_prog = malloc(sizeof(iloc_prog));
        n_prog->operation = op;
        n_prog->next_op = NULL;
        cursor->next_op = n_prog;
    }
    return prog;
}

iloc_prog* addOpToProgBeginning (iloc_prog *prog, iloc_op *op) {
    if (prog->operation->operation == NULL) {
        iloc_prog *n_prog = malloc(sizeof(iloc_prog));
        n_prog->operation = op;
        n_prog->next_op = NULL;
        prog = n_prog;
        return prog;
    }
    else {
        iloc_prog *n_prog = malloc(sizeof(iloc_prog));
        n_prog->operation = op;
        n_prog->next_op = prog;
        return n_prog;
    }
}

char* checkContext (pilha* pilha_atual, char *key) {
    // Caso haja mais de um escopo, é local
	int contador = pilha_atual->num_escopos;
	hash_table *hash_table_atual;
	while(contador >= 1){
		hash_table_atual = pilha_atual->escopos[contador - 1];
		if (ht_search(hash_table_atual, key) != NULL){
            if (contador == pilha_atual->num_escopos) {
                return strdup("rfp");
            }
            else {
                return strdup("rbss");
            }
		}
		contador--;
	}
    return NULL;
}

tree_t* findFirstOp (tree_t *t) {
    if (t != NULL) {
        if (t->prog->operation->operation != NULL) {
            if (t->number_of_children == 0) {
                return t;
            }
            for (int i = 0; i < t->number_of_children; i++) {
                if (findFirstOp(t->children[i]) != NULL) {
                    return findFirstOp(t->children[i]);
                }
            }
        }
    }
    return NULL;
}

tree_t* findLastProg (tree_t *t) {
    if (t != NULL) {
        tree_t *cursor = t;
        while ((cursor->isLast != 1) && (cursor != NULL) && (cursor->number_of_children != 0)) {
            cursor = cursor->children[cursor->number_of_children - 1];
        }
        if ((strcmp(t->info.token_value, strdup("if")) == 0) || (strcmp(t->info.token_value, strdup("while")) == 0)) {
            cursor = cursor->children[cursor->number_of_children - 1];
        }
        return cursor;
    }
    return NULL;
}

tree_t* findMainStart (tree_t *t) {
    if (t != NULL) {
        if (strcmp(t->info.token_value, strdup("main")) == 0) {
            ht_item *id_hash_table = encontrarItemPilha(stack, t->info.token_value);
            if (strcmp(id_hash_table->nature, strdup("function")) == 0) {
                inMain = 1;
                return t;
            }
            else {
                return NULL;
            }
        }
        else {
            for (int i = 0; i < t->number_of_children; i++) {
                if (findMainStart(t->children[i]) != NULL) {
                    return findMainStart(t->children[i]);
                }
            }
        }
    }
    return NULL;
}

void pruneFunction (tree_t *t) {
    if (t != NULL) {
        ht_item *id_hash_table = encontrarItemPilha(stack, t->info.token_value);
        if (strcmp(id_hash_table->nature, strdup("function")) == 0) {
            if (t->number_of_children > 1) {
                tree_free(t->children[t->number_of_children - 1]);
                t->children[t->number_of_children - 1] = NULL;
            }
        }
    }
}

/* --- Geração de código assembly --- */

void generateAsm (tree_t *tr) {

    printf("\t.data\n");
    
    /* Gerar segmento de dados */

    printAsmDataSegment(stack);

    /* Gerar segmento de código */
    tree_t *t;

    if (tr != NULL) {
        /* Isolando a main */
        if (inMain == 0) {
            t = findMainStart(tr);
            pruneFunction(t);
        }
        else {
            t = tr;
        }

        if (t != NULL) {
            /* Declaração da main */
            printf("\t.text\n");
            printf("\t.globl main\n");
            printf("main:\n");
            printf(".LFB0:\n");

            prog = asm_prog_insert(prog, strdup("\tpush %rbp\n"));
            prog = asm_prog_insert(prog, strdup("\tmovq %rsp, %rbp\n"));

            createAsmProg(t);

            prog = asm_prog_insert(prog, strdup("\tpopq %rbp\n"));
            prog = asm_prog_insert(prog, strdup("\tret\n"));

            printAsmProg(prog);
        }
        printf("\n");
    }
}

void printAsmDataSegment (pilha *stack) {
    /* Retirada da hash table de escopo global */
    hash_table *ht = stack->escopos[0];
    /* Varredura em todos os itens */
    for (int i = 0; i < ht->size; i++) {
        if (ht->items[i] != NULL) {
            if (strcmp(ht->items[i]->nature, strdup("function")) != 0) {
                printf("%s:\n", ht->items[i]->key);
                printf("\t.zero 4\n");
            }
        }
    }
}

/* Variável para saber se está dentro de uma expressão complexa */
short isInComplexExp = 0;

/* Variável para saber se está dentro de uma expressão lógica */
short isInLogicalExp = 0;

/* Variável para saber se variáveis relacionais devem ser negadas */
/* 0 = não negue, 1 = negue */
short negateOperations = 0;

/* Variável para setar a direção do jump */
/* 0 = gera o bif, 1 = pega do nodo */
short setJumpTo = 0;

/* Variável para saber se contexto mais próximo é um if, while, ou nenhum */
/* 0 = nenhum, 1 = if, 2 = while */
short closestContext = 0;

short isLastBinaryOp (tree_t *t) {
    if (t != NULL) {
        /* É operador */
        if (strcmp(t->info.token_type, strdup("operador")) == 0) {
            if (t->number_of_children == 2) {
                if ((strcmp(t->children[0]->info.token_type, strdup("operador")) != 0) && (strcmp(t->children[1]->info.token_type, strdup("operador")) != 0)) {
                    return 0;
                }
                else if ((strcmp(t->children[0]->info.token_type, strdup("operador")) != 0) || (strcmp(t->children[1]->info.token_type, strdup("operador")) != 0)) {
                    return 1;
                }
                else {
                    return 2;
                }
            }
            else if (t->number_of_children == 1) {
                if (strcmp(t->children[0]->info.token_type, strdup("operador")) != 0) {
                    return 0;
                }
                else {
                    return 2;
                }
            }
            else {
                return 2;
            }
        }
    }
    return 3;
}

char* whichRegister (int depth) {
    switch(depth) {
        case 1:
            return strdup("ebx");
            break;
        case 2:
            return strdup("ecx");
            break;
        case 3:
            return strdup("edx");
            break;
        case 4:
            return strdup("r8d");
            break;
        case 5:
            return strdup("r9d");
            break;
        case 6:
            return strdup("r10d");
            break;
        default:
            break;
    }
}

short isArithmeticOp (tree_t *t) {
    if (strcmp(t->info.token_type, strdup("operador")) == 0) {
        if (strcmp(t->info.token_value, strdup("+")) == 0) {
            return 0;
        }
        else if (strcmp(t->info.token_value, strdup("-")) == 0) {
            return 0;
        }
        else if (strcmp(t->info.token_value, strdup("*")) == 0) {
            return 0;
        }
        else if (strcmp(t->info.token_value, strdup("/")) == 0) {
            return 0;
        }
        else {
            return 1;
        }
    }
    else {
        return 2;
    }
}

char* findLabelinBlock (tree_t *t) {
    if (t != NULL) {
        tree_t *firstOp = findFirstOp(t);
        if (firstOp != NULL) {
            if (firstOp->prog->operation->label != NULL) {
                return strdup(firstOp->prog->operation->label);
            }
        }
    }
    return NULL;
}

asm_prog* create_asm_prog() {
    asm_prog* prog = (asm_prog *) malloc(sizeof(asm_prog));

    return prog;
}

asm_prog* asm_prog_insert(asm_prog *prog, char* instruction) {
    if (!prog)
    {
        asm_prog* head = create_asm_prog();
        head->instruction = instruction;
        head->next = NULL;
        head->isLeader = 0;
        prog = head;
        return prog;
    }
    else if (prog->next == NULL)
    {
        asm_prog* node = create_asm_prog();
        node->instruction = instruction;
        node->next = NULL;
        node->isLeader = 0;
        prog->next = node;
        return prog;
    }

    asm_prog* temp = prog;

    while (temp->next)
    {
        temp = temp->next;
    }

    asm_prog* node = create_asm_prog();
    node->instruction = instruction;
    node->next = NULL;
    node->isLeader = 0;
    temp->next = node;
    return prog;
}

char* asm_prog_remove (asm_prog *prog) {
    if (!prog)
        return NULL;

    if (!prog->next)
        return NULL;

    asm_prog* node = prog->next;
    asm_prog* temp = prog;
    temp->next = NULL;
    prog = node;
    char* it = strdup(temp->instruction);
    free(temp->instruction);
    free(temp);
    return it;
}

void free_asm_prog (asm_prog* prog) {
    asm_prog* temp = prog;

    while (prog)
    {
        temp = prog;
        prog = prog->next;
        free(temp->instruction);
        free(temp);
    }
}

void printAsmProg (asm_prog *prog) {
    asm_prog *cursor = prog;
    while (cursor != NULL) {
        printf("%s", cursor->instruction);
        cursor = cursor->next;
    }
}

void createAsmProg (tree_t *tr) {
    if (tr != NULL) {
        if (isLastBinaryOp(tr) == 2) { // Caso exista parenteses na expressão -- limitado a 5 níveis de profundidade
            if (isArithmeticOp(tr) == 0) {
                createAsmProg(tr->children[0]);
                isInComplexExp++;
                char* reg = whichRegister(isInComplexExp);
                char *buffer = malloc(sizeof(char) * 50);
                sprintf(buffer, "\tmovl %%eax, %%%s\n", reg);
                prog = asm_prog_insert(prog, strdup(buffer));
                free(buffer);
                createAsmProg(tr->children[1]);
                char *buffer3 = malloc(sizeof(char) * 50);
                strcpy(buffer3, "");
                getArithmeticOp(tr);
                strcat(buffer3, getArithmeticOp(tr));
                char *buffer2 = malloc(sizeof(char) * 50);
                sprintf(buffer2, " %%%s, %%eax\n", reg);
                strcat(buffer3, buffer2);
                prog = asm_prog_insert(prog, strdup(buffer3));
                free(buffer3);
                free(buffer2);
                isInComplexExp--;
            }
            else if (isArithmeticOp(tr) == 1) {
                /* Se contexto mais próximo é um if */
                if (closestContext == 1) { 
                    /* Quando é &, as operações só se concatenam */
                    if (strcmp(tr->info.token_value, strdup("&")) == 0) {
                        tr->children[0]->jumpTo = strdup(tr->jumpTo);
                        tr->children[1]->jumpTo = strdup(tr->jumpTo);
                        createAsmProg(tr->children[0]);
                        createAsmProg(tr->children[1]);
                    }
                    /* Quando é |, as operações são invertidas (condizentes com o sinal), apenas a última se mantém */
                    else if (strcmp(tr->info.token_value, strdup("|")) == 0) {
                        /* Todos os jumps, exceto o último, também deve ser alterados para o bloco de bif */
                        if (negateOperations == 0) {
                            negateOperations = 1;

                            char *labeltoJumpTo;
                            if (setJumpTo == 0) {
                                char *buffer1 = strdup(tr->jumpTo);
                                char *buffer2 = malloc(sizeof(char) * strlen(buffer1));
                                strncpy(buffer2, buffer1 + 1, strlen(buffer1));
                                buffer2[strlen(buffer1) - 1] = '\0';
                                int labelValue = atoi(buffer2) - 1;
                                
                                labeltoJumpTo = malloc(sizeof(char) * 8);
                                sprintf(labeltoJumpTo, "L%d", labelValue);
                                setJumpTo = 1;
                            }
                            else {
                                labeltoJumpTo = strdup(tr->jumpTo);
                            }
                            tr->children[0]->jumpTo = strdup(labeltoJumpTo);
                            tr->children[1]->jumpTo = strdup(labeltoJumpTo);
                            createAsmProg(tr->children[0]);
                            /* Se o filho da direita, último a ser processado, tiver algum operando final */
                            if (isLastBinaryOp(tr->children[1]) != 2) {
                                char *buffer1 = strdup(tr->jumpTo);
                                char *buffer2 = malloc(sizeof(char) * strlen(buffer1));
                                strncpy(buffer2, buffer1 + 1, strlen(buffer1));
                                buffer2[strlen(buffer1) - 1] = '\0';
                                char *jumpToOp1 = strdup(tr->children[0]->jumpTo);
                                char *jumpToNode = strdup(tr->jumpTo);
                                int labelValue = atoi(buffer2);
                                if (strcmp(jumpToOp1, jumpToNode) == 0) {
                                    labelValue += 1;
                                }
                                char *buffer = malloc(sizeof(char) * 8);
                                sprintf(buffer, "L%d", labelValue);
                                tr->children[1]->jumpTo = strdup(buffer);
                                negateOperations = 0;
                                createAsmProg(tr->children[1]);
                                setJumpTo = 0;
                            }
                            else {
                                negateOperations = 0;
                                createAsmProg(tr->children[1]);
                            }
                            
                        }
                        else {
                            char *labeltoJumpTo = strdup(tr->jumpTo);
                            for (int i = 0; i < tr->number_of_children; i++) {
                                if (tr->children[i] != NULL) {
                                    tr->children[i]->jumpTo = strdup(labeltoJumpTo);
                                    createAsmProg(tr->children[i]);
                                }
                            }
                        }
                    }
                }
                /* Se contexto mais próximo é um while */
                else if (closestContext == 2) {
                    /* Quando é |, as operações são invertidas e se concatenam */
                    if (strcmp(tr->info.token_value, strdup("|")) == 0) {
                        /* Operações só são concatenadas */
                        tr->children[0]->jumpTo = strdup(tr->jumpTo);
                        tr->children[1]->jumpTo = strdup(tr->jumpTo);
                        createAsmProg(tr->children[0]);
                        createAsmProg(tr->children[1]);
                    }
                    /* Quando é &, as operações são invertidas (condizentes com o sinal), apenas a última se mantém */
                    else if (strcmp(tr->info.token_value, strdup("&")) == 0) {
                        /* Todos os jumps, exceto o último, também deve ser alterados para o bloco de post */
                        if (negateOperations == 1) {
                            negateOperations = 0;

                            char *labeltoJumpTo;
                            if (setJumpTo == 0) {
                                char *buffer1 = strdup(tr->jumpTo);
                                char *buffer2 = malloc(sizeof(char) * strlen(buffer1));
                                strncpy(buffer2, buffer1 + 1, strlen(buffer1));
                                buffer2[strlen(buffer1) - 1] = '\0';
                                int labelValue = atoi(buffer2) + 1;
                                
                                labeltoJumpTo = malloc(sizeof(char) * 8);
                                sprintf(labeltoJumpTo, "L%d", labelValue);
                                setJumpTo = 1;
                            }
                            else {
                                labeltoJumpTo = strdup(tr->jumpTo);
                            }
                            tr->children[0]->jumpTo = strdup(labeltoJumpTo);
                            tr->children[1]->jumpTo = strdup(labeltoJumpTo);
                            createAsmProg(tr->children[0]);
                            /* Se o filho da direita, último a ser processado, tiver algum operando final */
                            if (isLastBinaryOp(tr->children[1]) != 2) {
                                char *buffer1 = strdup(tr->jumpTo);
                                char *buffer2 = malloc(sizeof(char) * strlen(buffer1));
                                strncpy(buffer2, buffer1 + 1, strlen(buffer1));
                                buffer2[strlen(buffer1) - 1] = '\0';
                                char *jumpToOp1 = strdup(tr->children[0]->jumpTo);
                                char *jumpToNode = strdup(tr->jumpTo);
                                int labelValue = atoi(buffer2);
                                if (strcmp(jumpToOp1, jumpToNode) == 0) {
                                    labelValue -= 1;
                                }
                                char *buffer = malloc(sizeof(char) * 8);
                                sprintf(buffer, "L%d", labelValue);
                                tr->children[1]->jumpTo = strdup(buffer);
                                negateOperations = 1;
                                createAsmProg(tr->children[1]);
                                setJumpTo = 0;
                            }
                            else {
                                negateOperations = 1;
                                createAsmProg(tr->children[1]);
                            }
                            
                        }
                        else {
                            char *labeltoJumpTo = strdup(tr->jumpTo);
                            for (int i = 0; i < tr->number_of_children; i++) {
                                if (tr->children[i] != NULL) {
                                    tr->children[i]->jumpTo = strdup(labeltoJumpTo);
                                    createAsmProg(tr->children[i]);
                                }
                            }
                        }
                    }
                }
            }
        }
        /* Controle de fluxo */
        /* Caso seja um if/if-else */
        else if (strcmp(tr->info.token_value, strdup("if")) == 0) {
            short buffer = closestContext;
            isInLogicalExp = 1;

            char* label_bif = findLabelinBlock(tr->children[1]);
            char* label_belse = findLabelinBlock(tr->children[2]);
            char* label_post = strdup(tr->label);

            closestContext = 1; // Seta que contexto mais próximo é um if
            /* Caso seja um if simples */
            if (tr->number_of_children <= 3) {
                tr->children[0]->jumpTo = strdup(label_post);
                createAsmProg(tr->children[0]);
                isInLogicalExp = 0;
                createAsmProg(tr->children[1]);
                createAsmProg(tr->children[2]);
            }
            /* Caso seja um if-else */
            else {
                tr->children[0]->jumpTo = strdup(label_belse);
                createAsmProg(tr->children[0]);
                isInLogicalExp = 0;
                createAsmProg(tr->children[1]);
                char *buffer = malloc(sizeof(char) * 50);
                sprintf(buffer, "\tjmp .%s\n", label_post);
                prog = asm_prog_insert(prog, strdup(buffer));
                free(buffer);
                createAsmProg(tr->children[2]);
                createAsmProg(tr->children[3]);
            }
            closestContext = buffer;
        }
        /* Caso seja um while */
        else if (strcmp(tr->info.token_value, strdup("while")) == 0) {
           /* Jumps tem que ser condizentes */
            short buffer = closestContext;
            isInLogicalExp = 1;

            char* label_test = findLabelinBlock(tr->children[0]);
            char* label_cb = findLabelinBlock(tr->children[1]);
            char* label_post = strdup(tr->label);

            closestContext = 2; // Seta que contexto mais próximo é um while
            char *buffer2 = malloc(sizeof(char) * 50);
            sprintf(buffer2, "\tjmp .%s\n", label_test);
            prog = asm_prog_insert(prog, strdup(buffer2));
            free(buffer2);
            createAsmProg(tr->children[1]);
            negateOperations = 1;
            tr->children[0]->jumpTo = strdup(label_cb);
            createAsmProg(tr->children[0]);
            negateOperations = 0;
            createAsmProg(tr->children[2]);
            closestContext = buffer;
        }
        else {
            /* Organiza ordem de operações */
            for (int i = 0; i < tr->number_of_children; i++) {
                if (tr->children[i] != NULL) {
                    if (strcmp(tr->children[i]->info.token_type, strdup("comando simples")) != 0) {
                        createAsmProg (tr->children[i]);
                    }
                }
            }
            if (tr->prog != NULL) {
                iloc_prog *cursor = tr->prog;
                while (cursor != NULL) {
                    if (cursor->operation != NULL) {
                        if (cursor->operation->label != NULL) {
                            char *buffer = malloc(sizeof(char) * 50);
                            sprintf(buffer, ".%s:\n", cursor->operation->label);
                            prog = asm_prog_insert(prog, strdup(buffer));
                            free(buffer);
                        }
                        if (cursor->operation->operation != NULL) {
                            /* Atribuição */
                            if (strcmp(cursor->operation->operation, strdup("storeAI")) == 0) {
                                /* Caso seja uma atribuição direta */
                                if (strcmp(tr->children[1]->info.token_type, strdup("operador")) != 0) {
                                    char *buffer = malloc(sizeof(char) * 50);
                                    sprintf(buffer, "\tmovl ");
                                    strcat(buffer, getVarInClosure(tr->children[1]));
                                    strcat(buffer, strdup(", %eax\n"));
                                    prog = asm_prog_insert(prog, strdup(buffer));
                                    free(buffer);
                                }
                                char *buffer = malloc(sizeof(char) * 50);
                                sprintf(buffer, "\tmovl %%eax, ");
                                strcat(buffer, getVarInClosure(tr));
                                strcat(buffer, strdup("\n"));
                                prog = asm_prog_insert(prog, strdup(buffer));
                                free(buffer);
                            }

                            /* Operações aritméticas */
                            if (isArithmeticOp(tr) == 0) {
                                if (isLastBinaryOp(tr) == 0) { // Caso ambos operandos sejam finais
                                    /* Lê o primeiro operando */
                                    char *buffer = malloc(sizeof(char) * 50);
                                    sprintf(buffer, "\tmovl ");
                                    strcat(buffer, getVarInClosure(tr->children[0]));
                                    strcat(buffer, strdup(", %eax\n"));
                                    prog = asm_prog_insert(prog, strdup(buffer));
                                    free(buffer);
                                    /* Opera sobre o segundo */
                                    if (strcmp(tr->info.token_value, strdup("/")) != 0) {
                                        /* Se for uma negação unária */
                                        if ((strcmp(tr->info.token_value, strdup("-")) == 0) && (tr->number_of_children == 1)) {
                                            char *buffer = malloc(sizeof(char) * 50);
                                            strcpy(buffer, strdup(""));
                                            strcat(buffer, getArithmeticOp(tr));
                                            strcat(buffer, strdup(" %eax\n"));
                                            prog = asm_prog_insert(prog, strdup(buffer));
                                            free(buffer);
                                        }
                                        else {
                                            char *buffer = malloc(sizeof(char) * 50);
                                            strcpy(buffer, strdup(""));
                                            strcat(buffer, strdup(getArithmeticOp(tr)));
                                            strcat(buffer, strdup(" "));
                                            strcat(buffer, getVarInClosure(tr->children[1]));
                                            strcat(buffer, strdup(", %eax\n"));
                                            prog = asm_prog_insert(prog, strdup(buffer));
                                            free(buffer);
                                        }
                                    }
                                    else {
                                        /* Se for divisão, checa se operador é literal */
                                        /* idivl não funciona sob literais */
                                        if (strcmp(tr->children[1]->info.token_type, strdup("literal")) == 0) {
                                            char *reg = whichRegister(isInComplexExp + 1);
                                            char *buffer = malloc(sizeof(char) * 50);
                                            strcpy(buffer, strdup("\tmovl "));
                                            strcat(buffer, getVarInClosure(tr->children[1]));
                                            char *buffer2 = malloc(sizeof(char) * 50);
                                            sprintf(buffer2, ", %%%s\n", reg);
                                            strcat(buffer, buffer2);
                                            prog = asm_prog_insert(prog, strdup(buffer));
                                            free(buffer);
                                            free(buffer2);
                                            char *buffer3 = malloc(sizeof(char) * 50);
                                            strcpy(buffer3, getArithmeticOp(tr));
                                            char *buffer4 = malloc(sizeof(char) * 50);
                                            sprintf(buffer4, " %%%s\n", reg);
                                            strcat(buffer3, buffer4);
                                            prog = asm_prog_insert(prog, strdup(buffer3));
                                            free(buffer3);
                                            free(buffer4);
                                        }
                                        else {
                                            char *buffer = malloc(sizeof(char) * 50);
                                            strcpy(buffer, getArithmeticOp(tr));
                                            strcat(buffer, strdup(" "));
                                            strcat(buffer, getVarInClosure(tr->children[1]));
                                            strcat(buffer, strdup("\n"));
                                            prog = asm_prog_insert(prog, strdup(buffer));
                                            free(buffer);
                                        }
                                    }
                                }
                                else if (isLastBinaryOp(tr) == 1) { // Caso um operando seja final
                                    /* Opera sobre %eax */
                                    if (strcmp(tr->children[0]->info.token_type, strdup("operador")) != 0) { // Se for o primeiro filho que é o operando final
                                        if (strcmp(tr->info.token_value, strdup("/")) != 0) {
                                            char *buffer = malloc(sizeof(char) * 50);
                                            strcpy(buffer, getArithmeticOp(tr));
                                            strcat(buffer, strdup(" "));
                                            strcat(buffer, getVarInClosure(tr->children[0]));
                                            strcat(buffer, strdup(", %eax\n"));
                                            prog = asm_prog_insert(prog, strdup(buffer));
                                            free(buffer);
                                        }
                                        else {
                                            /* Se for divisão, checa se operador é literal */
                                            /* idivl não funciona sob literais */
                                            if (strcmp(tr->children[0]->info.token_type, strdup("literal")) == 0) {
                                                char *reg = whichRegister(isInComplexExp + 1);
                                                char *buffer = malloc(sizeof(char) * 50);
                                                strcpy(buffer, strdup("\tmovl "));
                                                strcat(buffer, getVarInClosure(tr->children[0]));
                                                char *buffer2 = malloc(sizeof(char) * 50);
                                                sprintf(buffer2, ", %%%s\n", reg);
                                                strcat(buffer, buffer2);
                                                prog = asm_prog_insert(prog, strdup(buffer));
                                                free(buffer);
                                                free(buffer2);
                                                char *buffer3 = malloc(sizeof(char) * 50);
                                                strcpy(buffer3, getArithmeticOp(tr));
                                                char *buffer4 = malloc(sizeof(char) * 50);
                                                sprintf(buffer4, " %%%s\n", reg);
                                                strcat(buffer3, buffer4);
                                                prog = asm_prog_insert(prog, strdup(buffer3));
                                                free(buffer3);
                                                free(buffer4);
                                            }
                                            else {
                                                char *buffer = malloc(sizeof(char) * 50);
                                                strcpy(buffer, getArithmeticOp(tr));
                                                strcat(buffer, strdup(" "));
                                                strcat(buffer, strdup(getVarInClosure(tr->children[0])));
                                                strcat(buffer, strdup("\n"));
                                                prog = asm_prog_insert(prog, strdup(buffer));
                                                free(buffer);
                                            }
                                        }
                                    }
                                    else {
                                        if (strcmp(tr->info.token_value, strdup("/")) != 0) {
                                            char *buffer = malloc(sizeof(char) * 50);
                                            strcpy(buffer, getArithmeticOp(tr));
                                            strcat(buffer, strdup(" "));
                                            strcat(buffer, getVarInClosure(tr->children[1]));
                                            strcat(buffer, strdup(", %eax\n"));
                                            prog = asm_prog_insert(prog, strdup(buffer));
                                            free(buffer);
                                        }
                                        else {
                                            /* Se for divisão, checa se operador é literal */
                                            /* idivl não funciona sob literais */
                                            if (strcmp(tr->children[1]->info.token_type, strdup("literal")) == 0) {
                                                char *reg = whichRegister(isInComplexExp + 1);
                                                char *buffer = malloc(sizeof(char) * 50);
                                                strcpy(buffer, strdup("\tmovl "));
                                                strcat(buffer, getVarInClosure(tr->children[1]));
                                                char *buffer2 = malloc(sizeof(char) * 50);
                                                sprintf(buffer2, ", %%%s\n", reg);
                                                strcat(buffer, buffer2);
                                                prog = asm_prog_insert(prog, strdup(buffer));
                                                free(buffer);
                                                free(buffer2);
                                                char *buffer3 = malloc(sizeof(char) * 50);
                                                strcpy(buffer3, getArithmeticOp(tr));
                                                char *buffer4 = malloc(sizeof(char) * 50);
                                                sprintf(buffer4, " %%%s\n", reg);
                                                strcat(buffer3, buffer4);
                                                prog = asm_prog_insert(prog, strdup(buffer3));
                                                free(buffer3);
                                                free(buffer4);
                                            }
                                            else {
                                                char *buffer = malloc(sizeof(char) * 50);
                                                strcpy(buffer, getArithmeticOp(tr));
                                                strcat(buffer, strdup(" "));
                                                strcat(buffer, getVarInClosure(tr->children[1]));
                                                strcat(buffer, strdup("\n"));
                                                prog = asm_prog_insert(prog, strdup(buffer));
                                                free(buffer);
                                            }
                                        }
                                    }
                                }
                            }
                            /* Operações relacionais */
                            else if (isArithmeticOp(tr) == 1) {
                                if (isInLogicalExp == 1) {
                                    if (isLastBinaryOp(tr) == 0) { // Caso ambos operandos sejam finais
                                        /* Lê o primeiro operando */
                                        char *buffer = malloc(sizeof(char) * 50);
                                        strcpy(buffer, strdup("\tmovl "));
                                        strcat(buffer, getVarInClosure(tr->children[0]));
                                        strcat(buffer, strdup(", %edx\n"));
                                        prog = asm_prog_insert(prog, strdup(buffer));
                                        free(buffer);
                                        /* Lê o segundo operando */
                                        char *buffer2 = malloc(sizeof(char) * 50);
                                        strcpy(buffer2, strdup("\tmovl "));
                                        strcat(buffer2, getVarInClosure(tr->children[1]));
                                        strcat(buffer2, strdup(", %eax\n"));
                                        prog = asm_prog_insert(prog, strdup(buffer2));
                                        free(buffer2);
                                    }
                                    else if (isLastBinaryOp(tr) == 1) {
                                        /* Assume que o operando não final está em eax */
                                        char *buffer = malloc(sizeof(char) * 50);
                                        strcpy(buffer, strdup("\tmovl "));
                                        if (strcmp(tr->children[0]->info.token_type, strdup("operador")) != 0) { // Se for o primeiro filho que é o operando final
                                            strcat(buffer, getVarInClosure(tr->children[0]));
                                        }
                                        else {
                                            strcat(buffer, getVarInClosure(tr->children[1]));
                                        }
                                        strcat(buffer, strdup(", %edx\n"));
                                        prog = asm_prog_insert(prog, strdup(buffer));
                                        free(buffer);
                                    }
                                    /* Compara os registradores */
                                    char *buffer3 = malloc(sizeof(char) * 50);
                                    strcpy(buffer3, strdup("\tcmpl %eax, %edx\n"));
                                    prog = asm_prog_insert(prog, strdup(buffer3));
                                    free(buffer3);
                                    /* Faz o pulo */
                                    char *buffer = malloc(sizeof(char) * 50);
                                    strcpy(buffer, getRelationalop(tr, negateOperations));
                                    char *buffer2 = malloc(sizeof(char) * 50);
                                    sprintf(buffer2, " .%s\n", tr->jumpTo);
                                    strcat(buffer, buffer2);
                                    prog = asm_prog_insert(prog, strdup(buffer));
                                    free(buffer);
                                    free(buffer2);
                                }
                            }
                            
                            /* Retorno */
                            // Já funciona para expressões, uma vez que %eax é o registrador de retorno e também o padrão das operações
                            if (strcmp(cursor->operation->operation, strdup("ret")) == 0) {
                                /* Caso seja uma atribuição direta */
                                if (strcmp(tr->children[0]->info.token_type, strdup("operador")) != 0) {
                                    char *buffer = malloc(sizeof(char) * 50);
                                    strcpy(buffer, strdup("\tmovl "));
                                    strcat(buffer, getVarInClosure(tr->children[0]));
                                    strcat(buffer, strdup(", %eax\n"));
                                    prog = asm_prog_insert(prog, strdup(buffer));
                                    free(buffer);
                                    tr = NULL;
                                    return;
                                }
                            }
                        }

                    }
                    /* Caso seja negação unária, não itera novamente sobre o programa em assembly - diferente de ILOC */
                    if ((strcmp(tr->info.token_value, strdup("-")) == 0) && (tr->number_of_children == 1)) {
                        cursor = NULL;
                    }
                    else {
                        cursor = cursor->next_op;
                    }
                }
            }
            /* Organiza ordem de operações */
            for (int i = 0; i < tr->number_of_children; i++) {
                if (tr->children[i] != NULL) {
                    if (strcmp(tr->children[i]->info.token_type, strdup("comando simples")) == 0) {
                        createAsmProg (tr->children[i]);
                    }
                }
            }
        }
    }
}

void generateAsm_ (tree_t *tr) {
    if (tr != NULL) {
        prog = asm_prog_insert(prog, strdup("\tpush %rbp\n"));
        prog = asm_prog_insert(prog, strdup("\tmovq %rsp, %rbp\n"));

        createAsmProg(tr);

        prog = asm_prog_insert(prog, strdup("\tpopq %rbp\n"));
        prog = asm_prog_insert(prog, strdup("\tret\n"));
    }
}

char* getVarInClosure (tree_t *tr) {
    if (tr != NULL) {
        /* O nodo referencia a variável/literal em si */
        if (strcmp(tr->info.token_type, strdup("identificador")) == 0) {
            iloc_prog *cursor = tr->prog;
            while (strcmp(cursor->operation->operation, strdup("loadAI")) != 0) {
                cursor = cursor->next_op;
            }
            /* Variável é local */
            if (strcmp(cursor->operation->input_1, strdup("rfp")) == 0) {
                char *buffer = malloc(sizeof(char) * 20);
                sprintf(buffer, "-%d(%%rbp)", 4*(atoi(cursor->operation->input_2) + 1));
                return strdup(buffer);
            }
            else {
                /* Variável é global */
                char *buffer = malloc(sizeof(char) * 20);
                sprintf(buffer, "%s(%%rip)", tr->info.token_value);
                return strdup(buffer);
            }
        }
        else if (strcmp(tr->prog->operation->operation, strdup("storeAI")) == 0) {
            /* Variável é local */
            if (strcmp(tr->prog->operation->output_1, strdup("rfp")) == 0) {
                char *buffer = malloc(sizeof(char) * 20);
                sprintf(buffer, "-%d(%%rbp)", 4*(atoi(tr->prog->operation->output_2) + 1));
                return strdup(buffer);
            }
            else {
                /* Variável é global */
                char *buffer = malloc(sizeof(char) * 20);
                sprintf(buffer, "%s(%%rip)", tr->children[0]->info.token_value);
                return strdup(buffer);
            }
        }
        else if (strcmp(tr->info.token_type, strdup("literal")) == 0) {
            char *buffer = malloc(sizeof(char) * 20);
            sprintf(buffer, "$%s", tr->info.token_value);
            return strdup(buffer);
        }
    }
}

char* getArithmeticOp (tree_t *t) {
    if (strcmp(t->info.token_value, strdup("+")) == 0) {
        return strdup("\taddl");
    }
    else if (strcmp(t->info.token_value, strdup("-")) == 0) {
        if (t->number_of_children == 2) {
            return strdup("\tsubl");
        }
        else if (t->number_of_children == 1) {
            return strdup("\tnegl");
        }
    }
    else if (strcmp(t->info.token_value, strdup("*")) == 0) {
        return strdup("\timull");
    }
    else if (strcmp(t->info.token_value, strdup("/")) == 0) {
        return strdup("\tcltd\n\tidivl");
    }
}

char *getRelationalop (tree_t *t, short order) {
    /* Jumps são contrários ao sinal */
    if (strcmp(t->info.token_value, strdup("==")) == 0) {
        if (order == 0) {
            return strdup("\tjne");
        }
        else if (order == 1) {
            return strdup("\tje");
        }
    }
    else if (strcmp(t->info.token_value, strdup("!=")) == 0) {
        if (order == 0) {
            return strdup("\tje");
        }
        else if (order == 1) {
            return strdup("\tjne");
        }
    }
    else if (strcmp(t->info.token_value, strdup("<")) == 0) {
        if (order == 0) {
            return strdup("\tjge");
        }
        else if (order == 1) {
            return strdup("\tjl");
        }
    }
    else if (strcmp(t->info.token_value, strdup(">")) == 0) {
        if (order == 0) {
            return strdup("\tjle");
        }
        else if (order == 1) {
            return strdup("\tjg");
        }
    }
    else if (strcmp(t->info.token_value, strdup("<=")) == 0) {
        if (order == 0) {
            return strdup("\tjg");
        }
        else if (order == 1) {
            return strdup("\tjle");
        }
    }
    else if (strcmp(t->info.token_value, strdup(">=")) == 0) {
        if (order == 0) {
            return strdup("\tjl");
        }
        else if (order == 1) {
            return strdup("\tjge");
        }
    }
}

int setLeaderInstructions () {
    asm_prog *cursor = prog;
    while (cursor != NULL) {
        /* Caso seja a primeira instrução do programa, é uma instrução líder */
        if (cursor == prog) {
            cursor->isLeader = 1;
        }
        /* Caso seja a instrução destino de uma operação de desvio, é uma instrução líder */
        if (cursor->instruction[0] == '.') {
            while (cursor->instruction[0] == '.') {
                cursor = cursor->next;
            }
            cursor->isLeader = 1;
        }

        /* Caso seja uma instrução de desvio */
        /* A próxima instrução vai ser líder */
        if (cursor->instruction[1] == 'j') {
            cursor = cursor->next;
            /* Pula eventuais labels */
            while (cursor->instruction[0] == '.') {
                cursor = cursor->next;
            }
            cursor->isLeader = 1;
        }
        cursor = cursor->next;
    }
    int count = 0;
    cursor = prog;
    while (cursor != NULL) {
        if (cursor->isLeader == 1) {
            count += 1;
        }
        cursor = cursor->next;
    }
    return count;
}

void createCFGraph () {
    /* Seta quais são as instruções líderes */
    int numBasicBlocks = setLeaderInstructions();

    /* Cria os nodos do grafo */
    basic_block *BBs_array[numBasicBlocks];
    asm_prog *cursor = prog;
    for (int i = 0; i < numBasicBlocks; i++) {
        asm_prog *bb_code = NULL;
        while (nextIsntNULLorLeader(cursor) != 1) {
            bb_code = asm_prog_insert(bb_code, strdup(cursor->instruction));
            cursor = cursor->next;
        }
        if (cursor != NULL) {
            bb_code = asm_prog_insert(bb_code, strdup(cursor->instruction));
        }
        BBs_array[i] = malloc(sizeof(basic_block));
        BBs_array[i]->prog = bb_code;
        char *buffer = malloc(sizeof(char) * 5);
        sprintf(buffer, "%d", i);
        BBs_array[i]->name = strdup(buffer);
        if (cursor->next != NULL) {
            cursor = cursor->next;
        }
    }

    /* Decide arestas */
    int edgesMatrix[numBasicBlocks][numBasicBlocks];
    /* Inicia a matriz de arestas zerada */
    for (int i = 0; i < numBasicBlocks; i++) {
        for (int j = 0; j < numBasicBlocks; j++) {
            edgesMatrix[i][j] = 0;
        }
    }

    for (int i = 0; i < numBasicBlocks; i++) {
        asm_prog *cursor = BBs_array[i]->prog;
        int foundUnconditionalJump = 0;
        while (cursor != NULL) {
            if (cursor->instruction[1] == 'j') {
                if (cursor->instruction[2] == 'm') {
                    if (cursor->instruction[3] == 'p') {
                        /* Encontrou um jmp incondicional */
                        foundUnconditionalJump = 1;
                        char *label = strdup(&cursor->instruction[5]);
                        int blockTo = findLabelInBBs(BBs_array, numBasicBlocks, label);
                        edgesMatrix[i][blockTo] = 1;
                    }
                }
                else if (cursor->instruction[2] == 'n'
                        || cursor->instruction[2] == 'e'
                        || cursor->instruction[2] == 'g'
                        || cursor->instruction[2] == 'l') {
                            /* Encontrou um jmp condicional */
                            int j = 0;
                            while (cursor->instruction[j] != '.') {
                                j++;
                            }
                            char *label = strdup(&cursor->instruction[j]);
                            int blockTo = findLabelInBBs(BBs_array, numBasicBlocks, label);
                            edgesMatrix[i][blockTo] = 1;
                }
            }
            cursor = cursor->next;
        }
        /* Não encontrou jump incondicional */
        if (foundUnconditionalJump == 0) {
            if (i != (numBasicBlocks - 1)) {
                edgesMatrix[i][i+1] = 1;
            }
        }
        /* Encontrou jump incondicional */
        else {
            foundUnconditionalJump = 0;
        }
    }
    /* Cria formato DOT */
    generateDOTGraph (numBasicBlocks, edgesMatrix);
}

short nextIsntNULLorLeader (asm_prog *prog) {
    if (prog->next == NULL) {
        return 1;
    }
    else {
        if (prog->next->isLeader == 1) {
            return 1;
        }
        else {
            return 0;
        }
    }
}

int findLabelInBBs (basic_block **BBs_array, int numElems, char *label) {
    for (int i = 0; i < numElems; i++) {
        asm_prog *cursor = BBs_array[i]->prog;
        while(cursor != NULL) {
            char *buffer = strdup(cursor->instruction);
            buffer[strlen(buffer) - 2] = '\n';
            buffer[strlen(buffer) - 1] = '\0';
            if (strcmp(buffer, strdup(label)) == 0) {
                return (i + 1); // Label se refere sempre ao próximo bloco
            }
            cursor = cursor->next;
        }
    }
    return -1; // Erro;
}

void generateDOTGraph (int numNodes, int graph_edges[numNodes][numNodes]) {
    printf("digraph G {\n");
    for (int i = 0; i < numNodes; i++) {
        for (int j = 0; j < numNodes; j++) {
            if (graph_edges[i][j] == 1) {
                printf("%d -> %d;\n", i, j);
            }
        }
        printf("%d\n", i);
    }
    printf("}\n");
}