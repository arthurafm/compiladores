%{ 
    /* GRUPO G */
	/* Arthur Alves Ferreira Melo - 00333985 */
	/* Emanuel Pacheco Thiel -  00170313 */
	
	#include <string.h>

	int yylex(void);
	void yyerror(char const *mensagem);
%}

%code requires { #include "utils.h" }

%define parse.error verbose /* Syntax error report */

%union {
	lex_val valor_lexico;
	asd_tree_t *tree;
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
		asd_add_child($$, $2);
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
var: type id_list';';

/* Functions */
function: function_header function_body {
	$$ = $1;
	asd_add_child($$, $2);
};

function_header: param_list_parenthesis TK_OC_GE type'!' id {
	$$ = $5;
};

function_body: '{' simple_command_list '}' {
	$$ = $2;
};
function_body: '{' '}' {
	$$ = NULL;
};

/* Simple Commands */
simple_command_list: simple_command';' {
	$$ = $1;
};
simple_command_list: simple_command';' simple_command_list {
	if ($1 != NULL) {
		$$ = $1;
		asd_add_child($$, $3);
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
		asd_add_child($$, $3);
	}
	else {
		$$ = $3;
	}
}


simple_command: type id_list {
	$$ = NULL;
} /* Variable declaration */
simple_command: id '=' precedence_A {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("=");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
}; /* Attribution */
simple_command: id'('argument_list')' {
	lex_val id_lex = $1->label;
	int str_len = strlen(id_lex.token_value) + 7;
	char *tkn_value = malloc(sizeof(char) * str_len);
	strcpy(tkn_value, "call ");
	strcat(tkn_value, id_lex.token_value);
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup(tkn_value);
	$$ = asd_new(lexem);
	free(tkn_value);
	asd_add_child($$, $3);
}; /* Function call */
simple_command: id'('')' {
	lex_val id_lex = $1->label;
	int str_len = strlen(id_lex.token_value) + 7;
	char *tkn_value = malloc(sizeof(char) * str_len);
	strcpy(tkn_value, "call ");
	strcat(tkn_value, id_lex.token_value);
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup(tkn_value);
	$$ = asd_new(lexem);
	free(tkn_value);
}; /* Function call */
simple_command: TK_PR_RETURN precedence_A {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("return");
	$$ = asd_new(lexem);
	asd_add_child($$, $2);
}; /* Return command */
simple_command: TK_PR_IF '(' precedence_A ')' function_body {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("if");
	$$ = asd_new(lexem);
	asd_add_child($$, $3);
	asd_add_child($$, $5);
}; /* Flow Control */
simple_command: TK_PR_IF '(' precedence_A ')' function_body TK_PR_ELSE function_body {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("if");
	$$ = asd_new(lexem);
	asd_add_child($$, $3);
	asd_add_child($$, $5);
	asd_add_child($$, $7);
}; /* Flow Control */
simple_command: TK_PR_WHILE '(' precedence_A ')' function_body {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("while");
	$$ = asd_new(lexem);
	asd_add_child($$, $3);
	asd_add_child($$, $5);
}; /* Flow Control */

/* Expressions */
expr: id {
	$$ = $1;
};
expr: lit {
	$$ = $1;
};
expr: id'('argument_list')' {
	lex_val id_lex = $1->label;
	int str_len = strlen(id_lex.token_value) + 7;
	char *tkn_value = malloc(sizeof(char) * str_len);
	strcpy(tkn_value, "call ");
	strcat(tkn_value, id_lex.token_value);
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup(tkn_value);
	$$ = asd_new(lexem);
	free(tkn_value);
	asd_add_child($$, $3);
};
expr: id'('')' {
	lex_val id_lex = $1->label;
	int str_len = strlen(id_lex.token_value) + 7;
	char *tkn_value = malloc(sizeof(char) * str_len);
	strcpy(tkn_value, "call ");
	strcat(tkn_value, id_lex.token_value);
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup(tkn_value);
	$$ = asd_new(lexem);
	free(tkn_value);
};

precedence_A: precedence_B;
precedence_A: precedence_A TK_OC_OR precedence_B {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("|");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};

precedence_B: precedence_C;
precedence_B: precedence_B TK_OC_AND precedence_C {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("&");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};

precedence_C: precedence_D;
precedence_C: precedence_C TK_OC_EQ precedence_D {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("==");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};
precedence_C: precedence_C TK_OC_NE precedence_D {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("!=");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};

precedence_D: precedence_E;
precedence_D: precedence_D '<' precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("<*>");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};
precedence_D: precedence_D '>' precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup(">");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};
precedence_D: precedence_D TK_OC_LE precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("<=");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};
precedence_D: precedence_D TK_OC_GE precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup(">=");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};

precedence_E: precedence_F;
precedence_E: precedence_E '+' precedence_F {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("+");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};
precedence_E: precedence_E '-' precedence_F {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("-");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};

precedence_F: precedence_G;
precedence_F: precedence_F '*' precedence_G {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("*");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};
precedence_F: precedence_F '/' precedence_G {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("/");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};
precedence_F: precedence_F '%' precedence_G {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("%");
	$$ = asd_new(lexem);
	asd_add_child($$, $1);
	asd_add_child($$, $3);
};

precedence_G: expr;
precedence_G: '('precedence_A')' {
	$$ = $2;
};
precedence_G: '-'precedence_G {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("-");
	$$ = asd_new(lexem);
	asd_add_child($$, $2);
};
precedence_G: '!'precedence_G {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("!");
	$$ = asd_new(lexem);
	asd_add_child($$, $2);
};


/* -- Aux expressions -- */


/* Types */
type: TK_PR_INT;	/* int */
type: TK_PR_FLOAT;	/* float */
type: TK_PR_BOOL;	/* bool */

/* Identifiers */
id: TK_IDENTIFICADOR {
	$$ = asd_new($1);
};

id_list: id;
id_list: id',' id_list;

/* Literals */
lit: TK_LIT_INT {
	$$ = asd_new($1);
};
lit: TK_LIT_FLOAT {
	$$ = asd_new($1);	
};
lit: TK_LIT_TRUE {
	$$ = asd_new($1);
};
lit: TK_LIT_FALSE {
	$$ = asd_new($1);
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
	asd_add_child($$, $3);
};

%%
