#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

extern int yylineno;
extern void *arvore;

int get_line_number() {
    return yylineno;
}

void yyerror(const char *s) {
	  printf("In the line %d, the following error occurred: %s", get_line_number(), s);
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
    free(tree->info.token_type);
	  free(tree->info.token_value);
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
		printf("%p [label=\"%s\"]\n", (void *) tree, tree->info.token_value);
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

/* --- Tabela de Símbolos --- */

/* Algoritmo djb2, disponível em http://www.cse.yorku.ca/~oz/hash.html */
unsigned long hash_function (unsigned char *str) {
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c;

    return hash;
}

ht_item* create_item (char *key, int num_line, char *nature, char *type, char *token_value) {

    ht_item *item = (ht_item *) malloc(sizeof(ht_item));
    item->key = (char *) malloc(strlen(key) + 1);
    item->nature = (char *) malloc(strlen(nature) + 1);
    item->type = (char *) malloc(strlen(type) + 1);
    item->token_value = (char *) malloc(strlen(token_value) + 1);

    strcpy(item->key, key);
    item->num_line = num_line;
    strcpy(item->nature, nature);
    strcpy(item->type, type);
    strcpy(item->token_value, token_value);

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

    return table;
}

void free_item (ht_item* item) {

    free(item->key);
    free(item->nature);
    free(item->type);
    free(item->token_value);

    free(item);
}

void free_table (hash_table *table) {

    for (int i = 0; i < table->size; i++) {
        ht_item *item = table->items[i];
        if (item != NULL) {
            free(item);
        }
    }

    free(table->items);
    free(table);
}

void ht_insert (hash_table *table, char *key, int num_line, char *nature, char *type, char *token_value) {
    
    ht_item *item = create_item(key, num_line, nature, type, token_value);
    int index = hash_function(key);

    ht_item *cur_item = table->items[index];

    /* Não houve colisão */
    if (cur_item == NULL) {
        /* A hash table está cheia */
        if (table->count == table->size) {
            printf("Insert error: hash table is full.\n");
            free(item);
            return;
        }

        /* Caso a hash table não esteja cheia */
        table->items[index] = item;
        table->count++;
    }
    /* Houve colisão */
    else {
        /* A mesma key está na tabela */
        /* TODO: Tratamento especial da E4 --> Caso o identificador já esteja na tabela, é necessário que se retorne um erro */
        if (strcmp(cur_item->key, key) == 0) {
            strcpy(table->items[index]->key, key);
            table->items[index]->num_line = num_line;
            strcpy(table->items[index]->nature, nature);
            strcpy(table->items[index]->type, type);
            strcpy(table->items[index]->token_value, token_value);
            return;
        }
        /* Colisão não é na mesma key */
        else {
            handle_collision(table, item);
            return;
        }
    }
}

/* A decidir o que esta função retornará -> Tipo? Token_value? */
char* ht_search(hash_table *table, char *key) {
    int index = hash_function(key);
    ht_item *item = table->items[index];

    if (item != NULL) {
        if (strcmp(item->key, key) == 0) {
            return item->type;
        }
    }

    return NULL;
}
