#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

extern int yylineno;
extern void *arvore;
extern pilha *stack;

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
            printf("\tpush %%rbp\n");
            printf("\tmovq %%rsp, %%rbp\n");
            
            printAsmCodeSegment(t);

            printf("\tpopq %%rbp\n");
            printf("\tret\n");
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
void printAsmCodeSegment (tree_t *tr) {
    if (tr != NULL) {

        if (isLastBinaryOp(tr) == 2) { // Caso exista parenteses na expressão -- limitado a 5 níveis de profundidade
           printAsmCodeSegment(tr->children[0]);
           isInComplexExp++;
           char* reg = whichRegister(isInComplexExp);
           printf("\tmovl %%eax, %%%s\n", reg);
           printAsmCodeSegment(tr->children[1]);
           printArithmeticOp(tr);
           printf(" %%%s, %%eax\n", reg);
           isInComplexExp--;
        }
        else {
            /* Organiza ordem de operações */
            for (int i = 0; i < tr->number_of_children; i++) {
                if (tr->children[i] != NULL) {
                    if (strcmp(tr->children[i]->info.token_type, strdup("comando simples")) != 0) {
                        printAsmCodeSegment (tr->children[i]);
                    }
                }
            }
            if (tr->prog != NULL) {
                iloc_prog *cursor = tr->prog;
                while (cursor != NULL) {
                    if (cursor->operation != NULL) {
                        if (cursor->operation->label != NULL) {
                            printf(".%s:\n", cursor->operation->label);
                        }
                        if (cursor->operation->operation != NULL) {
                            /* Atribuição */
                            if (strcmp(cursor->operation->operation, strdup("storeAI")) == 0) {
                                /* Caso seja uma atribuição direta */
                                if (strcmp(tr->children[1]->info.token_type, strdup("operador")) != 0) {
                                    printf("\tmovl ");
                                    printVarInClosure(tr->children[1]);
                                    printf(", %%eax\n");
                                }
                                printf("\tmovl %%eax, ");
                                printVarInClosure(tr);
                                printf("\n");
                            }

                            /* Operações aritméticas */
                            if (isArithmeticOp(tr) == 0) {
                                if (isLastBinaryOp(tr) == 0) { // Caso ambos operandos sejam finais
                                    /* Lê o primeiro operando */
                                    printf("\tmovl ");
                                    printVarInClosure(tr->children[0]);
                                    printf(", %%eax\n");
                                    /* Opera sobre o segundo */
                                    printArithmeticOp(tr);
                                    printf(" ");
                                    printVarInClosure(tr->children[1]);
                                    printf(", %%eax\n");
                                }
                                else if (isLastBinaryOp(tr) == 1) { // Caso um operando seja final
                                    /* Opera sobre %eax */
                                    printArithmeticOp(tr);
                                    printf(" ");
                                    if (strcmp(tr->children[0]->info.token_type, strdup("operador")) != 0) { // Se for o primeiro filho que é o operando final
                                        printVarInClosure(tr->children[0]);
                                    }
                                    else {
                                        printVarInClosure(tr->children[1]);
                                    }
                                    printf(", %%eax\n");
                                }
                            }
                            
                            /* Retorno */
                            // Já funciona para expressões, uma vez que %eax é o registrador de retorno e também o padrão das operações
                            if (strcmp(cursor->operation->operation, strdup("ret")) == 0) {
                                if (strcmp(tr->children[0]->info.token_type, strdup("operador")) != 0) {
                                    printf("\tmovl ");
                                    printVarInClosure(tr->children[0]);
                                    printf(", %%eax\n");
                                }
                            }

                            /* Controle de fluxo */

                            /* Jumps incondicionais em ILOC são transcritos diretamente */
                            // if () {

                            // }

                        }

                    }
                    cursor = cursor->next_op;
                }
            }
            /* Organiza ordem de operações */
            for (int i = 0; i < tr->number_of_children; i++) {
                if (tr->children[i] != NULL) {
                    if (strcmp(tr->children[i]->info.token_type, strdup("comando simples")) == 0) {
                        printAsmCodeSegment (tr->children[i]);
                    }
                }
            }
        }
    }
}

short isLastBinaryOp (tree_t *t) {
    if (t != NULL) {
        /* É operador */
        if (strcmp(t->info.token_type, strdup("operador")) == 0) {
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
    }
    return 3;
}

void printVarInClosure (tree_t *tr) {
    /* O nodo referencia a variável/literal em si */
    if (strcmp(tr->info.token_type, strdup("identificador")) == 0) {
        /* Variável é local */
        if (strcmp(tr->prog->operation->input_1, strdup("rfp")) == 0) {
            printf("-%d(%%rbp)", 4*(atoi(tr->prog->operation->input_2) + 1));
        }
        else {
            /* Variável é global */
            printf("%s(%%rip)", tr->info.token_value);
        }
    }
    else if (strcmp(tr->prog->operation->operation, strdup("storeAI")) == 0) {
        /* Variável é local */
        if (strcmp(tr->prog->operation->output_1, strdup("rfp")) == 0) {
            printf("-%d(%%rbp)", 4*(atoi(tr->prog->operation->output_2) + 1));
        }
        else {
            /* Variável é global */
            printf("%s(%%rip)", tr->children[0]->info.token_value);
        }
    }
    else if (strcmp(tr->info.token_type, strdup("literal")) == 0) {
        printf("$%s", tr->info.token_value);
    }
}

void printArithmeticOp (tree_t *t) {
    if (strcmp(t->info.token_value, strdup("+")) == 0) {
        printf("\taddl");
    }
    else if (strcmp(t->info.token_value, strdup("-")) == 0) {
        printf("\tsubl");
    }
    else if (strcmp(t->info.token_value, strdup("*")) == 0) {
        printf("\timull");
    }
    else if (strcmp(t->info.token_value, strdup("/")) == 0) {
        printf("\tcltd\n");
        printf("\tidivl");
    }
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