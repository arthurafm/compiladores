%{ 
    /* GRUPO G */
	/* Arthur Alves Ferreira Melo - 00333985 */
	/* Emanuel Pacheco Thiel -  00170313 */
	
	#include <string.h>
	#include "utils.h"

	int yylex(void);
	void yyerror(char const *mensagem);

	extern pilha *stack;

	/* Problemas atuais: */
	/* Retorno de erro */
	/* Operações tem tipos específicos? */
%}

%code requires { #include "utils.h" }

%define parse.error verbose /* Syntax error report */

%union {
	lex_val valor_lexico;
	tree_t *tree;
	char* list;
}

%token TK_PR_INT
%token TK_PR_FLOAT
%token TK_PR_BOOL
%token TK_PR_IF
%token TK_PR_ELSE
%token TK_PR_WHILE
%token TK_PR_RETURN
%token TK_OC_LE
%token TK_OC_GE
%token TK_OC_EQ
%token TK_OC_NE
%token TK_OC_AND
%token TK_OC_OR
%token<valor_lexico> TK_IDENTIFICADOR
%token<valor_lexico> TK_LIT_INT
%token<valor_lexico> TK_LIT_FLOAT
%token<valor_lexico> TK_LIT_FALSE
%token<valor_lexico> TK_LIT_TRUE
%token TK_ERRO

%type<tree> program
%type<tree> list
%type<tree> element
%type<tree> function
%type<tree> function_header
%type<tree> function_body
%type<tree> simple_command_list
%type<tree> simple_command
%type<tree> expr
%type<tree> precedence_A
%type<tree> precedence_B
%type<tree> precedence_C
%type<tree> precedence_D
%type<tree> precedence_E
%type<tree> precedence_F
%type<tree> precedence_G
%type<tree> lit
%type<tree> id
%type<tree> argument_list
%type<valor_lexico> type
%type<list> id_list

%start program

%%

/* The Language */
program: list {
	$$ = $1;
	set_root($$);
};
program: /* Empty symbol */ {
	$$ = NULL;
	set_root($$);
};

list: element {
	$$ = $1;
};
list: element list {
	if ($1 != NULL) {
		$$ = $1;
		tree_add_child($$, $2);
	}
	else {
		$$ = $2;
	}
};

element: var {
	$$ = NULL;
};
element: function {
	$$ = $1;
};

/* Variables */
var: type id_list';' {
	char *list = $2;
	char *type = $1.type;
	char* s = strdup(",");
	char *token;

	token = strtok(list, s);

	while (token != NULL) {
		addItemEscopo(stack, token, get_line_number(), strdup("variable"), strdup(type));
	}

	free(list);
	free(token);
	free(s);
};

/* Functions */
function: function_header function_body {
	$$ = $1;
	tree_add_child($$, $2);
};

function_header: param_list_parenthesis TK_OC_GE type'!' id {
	lex_val id_info = $5->info;
	char *id_type = strdup($3.type);
	$5->info.type = strdup(id_type);
	addItemEscopo(stack, id_info.token_value, id_info.num_line, strdup("function"), strdup(id_type));
	$$ = $5;
	free(id_type);
};

function_body: open_closure '{' simple_command_list '}' close_closure {
	$$ = $3;
};
function_body: open_closure '{' '}' close_closure {
	$$ = NULL;
};

open_closure: {
	addEscopo(stack);
};

close_closure: {
	excluirEscopo(stack);
};

/* Simple Commands */
simple_command_list: simple_command';' {
	$$ = $1;
};
simple_command_list: simple_command';' simple_command_list {
	if ($1 != NULL) {
		$$ = $1;
		tree_add_child($$, $3);
	}
	else {
		$$ = $3;
	}
};
simple_command_list: function_body';' {
	$$ = $1;
};
simple_command_list: function_body';' simple_command_list {
	if ($1 != NULL) {
		$$ = $1;
		tree_add_child($$, $3);
	}
	else {
		$$ = $3;
	}
}

simple_command: type id_list {
	char *list = $2;
	char *type = $1.type;
	char* s = strdup(",");
	char *token;

	token = strtok(list, s);

	while (token != NULL) {
		addItemEscopo(stack, token, get_line_number(), strdup("variable"), strdup(type));
	}

	free(list);
	free(token);
	free(s);
	$$ = NULL;
} /* Variable declaration */
simple_command: id '=' precedence_A {
	lex_val id_info = $1->info, expr_info = $3->info;
	if (encontrarItemPilha(stack, id_info.token_value) == NULL) {
		/* Erro -> Variável ainda não declarada */
		return ERR_UNDECLARED;
	}
	else if (strcmp(id_info.type, expr_info.type) != 0) {
		/* Erro -> Variável não é do tipo da expressão */
	}
	else {
		lex_val lexem;
		lexem.num_line = get_line_number();
		lexem.token_type = strdup("comando simples");
		lexem.token_value = strdup("=");
		$$ = tree_new(lexem);
		tree_add_child($$, $1);
		tree_add_child($$, $3);
	}
}; /* Attribution */
simple_command: id'('argument_list')' {
	lex_val id_lex = $1->info;
	ht_item *id_hash_table = encontrarItemPilha(stack, id_lex.token_value);
	if (strcmp(strdup("function"), id_hash_table->nature) == 0) {
		int str_len = strlen(id_lex.token_value) + 7;
		char *tkn_value = malloc(sizeof(char) * str_len);
		strcpy(tkn_value, "call ");
		strcat(tkn_value, id_lex.token_value);
		lex_val lexem;
		lexem.num_line = get_line_number();
		lexem.token_type = strdup("comando simples");
		lexem.token_value = strdup(tkn_value);
		$$ = tree_new(lexem);
		free(tkn_value);
		tree_add_child($$, $3);
	}
	else {
		/* Erro -> Identificador não é função */
	}
	
}; /* Function call */
simple_command: id'('')' {
	lex_val id_lex = $1->info;
	ht_item *id_hash_table = encontrarItemPilha(stack, id_lex.token_value);
	if (strcmp(strdup("function"), id_hash_table->nature) == 0) {
		int str_len = strlen(id_lex.token_value) + 7;
		char *tkn_value = malloc(sizeof(char) * str_len);
		strcpy(tkn_value, "call ");
		strcat(tkn_value, id_lex.token_value);
		lex_val lexem;
		lexem.num_line = get_line_number();
		lexem.token_type = strdup("comando simples");
		lexem.token_value = strdup(tkn_value);
		$$ = tree_new(lexem);
		free(tkn_value);
	}
	else {
		/* Erro -> Identificador não é função */
	}
	
}; /* Function call */
simple_command: TK_PR_RETURN precedence_A {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("return");
	$$ = tree_new(lexem);
	tree_add_child($$, $2);
}; /* Return command */
simple_command: TK_PR_IF '(' precedence_A ')' function_body {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("if");
	$$ = tree_new(lexem);
	tree_add_child($$, $3);
	tree_add_child($$, $5);
}; /* Flow Control */
simple_command: TK_PR_IF '(' precedence_A ')' function_body TK_PR_ELSE function_body {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("if");
	$$ = tree_new(lexem);
	tree_add_child($$, $3);
	tree_add_child($$, $5);
	tree_add_child($$, $7);
}; /* Flow Control */
simple_command: TK_PR_WHILE '(' precedence_A ')' function_body {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("while");
	$$ = tree_new(lexem);
	tree_add_child($$, $3);
	tree_add_child($$, $5);
}; /* Flow Control */

/* Expressions */
expr: id {
	$$ = $1;
};
expr: lit {
	$$ = $1;
};
expr: id'('argument_list')' {
	lex_val id_lex = $1->info;
	ht_item *id_hash_table = encontrarItemPilha(stack, id_lex.token_value);
	if (strcmp(strdup("function"), id_hash_table->nature) == 0) {
		int str_len = strlen(id_lex.token_value) + 7;
		char *tkn_value = malloc(sizeof(char) * str_len);
		strcpy(tkn_value, "call ");
		strcat(tkn_value, id_lex.token_value);
		lex_val lexem;
		lexem.num_line = get_line_number();
		lexem.token_type = strdup("comando simples");
		lexem.token_value = strdup(tkn_value);
		$$ = tree_new(lexem);
		free(tkn_value);
		tree_add_child($$, $3);
	}
	else {
		/* Erro -> Identificador não é função */
		return ERR_VARIABLE;
	}
	
};
expr: id'('')' {
	lex_val id_lex = $1->info;
	ht_item *id_hash_table = encontrarItemPilha(stack, id_lex.token_value);
	if (strcmp(strdup("function"), id_hash_table->nature) == 0) {
		int str_len = strlen(id_lex.token_value) + 7;
		char *tkn_value = malloc(sizeof(char) * str_len);
		strcpy(tkn_value, "call ");
		strcat(tkn_value, id_lex.token_value);
		lex_val lexem;
		lexem.num_line = get_line_number();
		lexem.token_type = strdup("comando simples");
		lexem.token_value = strdup(tkn_value);
		$$ = tree_new(lexem);
		free(tkn_value);
	}
	else {
		/* Erro -> Identificador não é função */
		return ERR_VARIABLE;
	}
};

precedence_A: precedence_B;
precedence_A: precedence_A TK_OC_OR precedence_B {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("|");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};

precedence_B: precedence_C;
precedence_B: precedence_B TK_OC_AND precedence_C {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("&");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};

precedence_C: precedence_D;
precedence_C: precedence_C TK_OC_EQ precedence_D {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("==");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_C: precedence_C TK_OC_NE precedence_D {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("!=");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};

precedence_D: precedence_E;
precedence_D: precedence_D '<' precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("<");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_D: precedence_D '>' precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup(">");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_D: precedence_D TK_OC_LE precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("<=");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_D: precedence_D TK_OC_GE precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup(">=");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};

precedence_E: precedence_F;
precedence_E: precedence_E '+' precedence_F {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("+");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_E: precedence_E '-' precedence_F {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("-");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};

precedence_F: precedence_G;
precedence_F: precedence_F '*' precedence_G {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("*");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_F: precedence_F '/' precedence_G {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("/");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_F: precedence_F '%' precedence_G {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("%");
	$$ = tree_new(lexem);
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};

precedence_G: expr;
precedence_G: '('precedence_A')' {
	$$ = $2;
};
precedence_G: '-'precedence_G {
	lex_val expr_info = $2->info;
	if ((strcmp(expr_info.type, "int") == 0) || (strcmp(expr_info.type, "float") == 0)) {
		lex_val lexem;
		lexem.num_line = get_line_number();
		lexem.token_type = strdup("operador");
		lexem.token_value = strdup("-");
		$$ = tree_new(lexem);
		tree_add_child($$, $2);
	}
	else {
		/* Erro -> Expressão não é inteira ou flutuante */
	}
};
precedence_G: '!'precedence_G {
	lex_val expr_info = $2->info;
	if (strcmp(expr_info.type, "bool") == 0) {
		lex_val lexem;
		lexem.num_line = get_line_number();
		lexem.token_type = strdup("operador");
		lexem.token_value = strdup("!");
		$$ = tree_new(lexem);
		tree_add_child($$, $2);
	}
	else {
		/* Erro -> Expressão não é booleana */
	}
	
};


/* -- Aux expressions -- */


/* Types */
type: TK_PR_INT {
	lex_val val;
	val.num_line = get_line_number();
	val.token_type = NULL;
	val.token_value = NULL;
	val.type = strdup("int");
	$$ = val;
};	/* int */
type: TK_PR_FLOAT {
	lex_val val;
	val.num_line = get_line_number();
	val.token_type = NULL;
	val.token_value = NULL;
	val.type = strdup("float");
	$$ = val;
};	/* float */
type: TK_PR_BOOL{
	lex_val val;
	val.num_line = get_line_number();
	val.token_type = NULL;
	val.token_value = NULL;
	val.type = strdup("bool");
	$$ = val;
};	/* bool */

/* Identifiers */
id: TK_IDENTIFICADOR {
	$$ = tree_new($1);
};

id_list: id {
	char *id_name = strdup($1->info.token_value);
	$$ = strdup(id_name);
	free(id_name);
};
id_list: id',' id_list {
	char *id_name = strdup($1->info.token_value);
	int old_id_list_size = strlen($3) + 1;
	char *new_list = malloc(sizeof(char) * (strlen(id_name) + 2 + old_id_list_size));

	strcpy(new_list, id_name);
	strcat(new_list, strdup(","));
	strcat(new_list, $3);

	free(new_list);
	free(id_name);
};

/* Literals */
lit: TK_LIT_INT {
	$$ = tree_new($1);
};
lit: TK_LIT_FLOAT {
	$$ = tree_new($1);	
};
lit: TK_LIT_TRUE {
	$$ = tree_new($1);
};
lit: TK_LIT_FALSE {
	$$ = tree_new($1);
};

/* Parameters */
param_list_parenthesis: '(' ')';
param_list_parenthesis: '(' param_list ')';

param_list: param;
param_list: param ',' param_list;

param: type id;

/* Arguments */
argument_list: precedence_A {
	$$ = $1;
};
argument_list: precedence_A',' argument_list {
	$$ = $1;
	tree_add_child($$, $3);
};

%%
