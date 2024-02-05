%{ 
    /* GRUPO G */
	/* Arthur Alves Ferreira Melo - 00333985 */
	/* Emanuel Pacheco Thiel -  00170313 */

	#include <string.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include "utils.h"

	int yylex(void);
	void yyerror(char const *mensagem);

	extern pilha *stack;
	extern int labelCounter;
	extern int registerCounter;

	/* Problemas atuais: */
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
%type<tree> command_block
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
%type<tree> param
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
		token = strtok(NULL, s);
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

function_header: open_premature_closure param_list_parenthesis TK_OC_GE type'!' id {
	lex_val id_info = $6->info;
	char *id_type = strdup($4.type);
	$6->info.type = strdup(id_type);
	addItemEscopoOfsset(stack, 1, id_info.token_value, id_info.num_line, strdup("function"), strdup(id_type));
	$$ = $6;
	// if(strcmp(id_info.token_value, "main") == 0){
	// 	printf("era a main\n");
	// }
	// else{
	// 	printf("nao era a main\n");
	// }
	free(id_type);
};

function_body: open_closure '{' simple_command_list '}' close_closure {
	$$ = $3;
};
function_body: open_closure '{' '}' close_closure {
	$$ = NULL;
};

command_block: '{' simple_command_list '}' {
	$$ = $2;
};
command_block: '{' '}' {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("");
	lexem.type = strdup("");
	$$ = tree_new(lexem);

	iloc_op *op_nop = newILOCop (
		NULL,
		strdup("nop"),
		NULL,
		NULL,
		NULL,
		NULL,
		-1
	);

	$$->prog = addOpToProg($$->prog, op_nop);
};

open_closure: {
	// printaPilha(stack);
	addEscopo(stack);
};

close_closure: {
	// printaPilha(stack);
	excluirEscopo(stack);
};

open_premature_closure:	{
	// printaPilha(stack);
	addEscopo(stack);
	stack->escopos_ignorar++;
}

/* Simple Commands */
simple_command_list: simple_command';' {
	$$ = $1;
	$$->isLast = 1;
	if ((strcmp($$->info.token_value, strdup("if")) == 0) || (strcmp($$->info.token_value, strdup("while")) == 0)) {
		tree_t *last = $$->children[$$->number_of_children - 1];

		lex_val lexem;
		lexem.num_line = get_line_number();
		lexem.token_type = strdup("comando simples");
		lexem.token_value = strdup("");
		lexem.type = strdup("");
		last = tree_new(lexem);

		iloc_op *op_nop = newILOCop (
			strdup($$->label),
			strdup("nop"),
			NULL,
			NULL,
			NULL,
			NULL,
			-1
		);

		last->prog = addOpToProg(last->prog, op_nop);

		$$->children[$$->number_of_children - 1] = last;
	}
	tree_t *test = $$;
	int a;
	a = 1;
};
simple_command_list: simple_command';' simple_command_list {
	if ($1 != NULL) {
		$$ = $1;
		if ((strcmp($$->info.token_value, strdup("if")) == 0) || (strcmp($$->info.token_value, strdup("while")) == 0)) {
			$$->children[$$->number_of_children - 1] = $3;
			tree_t *firstOp = findFirstOp($3);
			if (firstOp->prog->operation->label != NULL) {
				iloc_op *op_nop = newILOCop (
					strdup($$->label),
					strdup("nop"),
					NULL,
					NULL,
					NULL,
					NULL,
					-1
				);
				firstOp->prog = addOpToProgBeginning(firstOp->prog, op_nop);
			}
			else {
				firstOp->prog->operation->label = strdup($$->label);
				int a;
				a = 1;
			}
		}
		else {
			tree_add_child($$, $3);
		}
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
		token = strtok(NULL, s);
	}

	free(list);
	free(token);
	free(s);
	$$ = NULL;
} /* Variable declaration */
simple_command: id '=' precedence_A {
	$$ = NULL;
	lex_val id_info = $1->info;
	lex_val expr_info = $3->info;
	ht_item *id_hash_table = encontrarItemPilha(stack, strdup(id_info.token_value));
	if (id_hash_table == NULL) {
		/* Erro -> Variável ainda não declarada */
		printf("The variable \'%s\', in the line %d, wasn\'t declared.\n", id_info.token_value, id_info.num_line);
		exit(ERR_UNDECLARED);
	}
	else {
		if (strcmp(id_hash_table->nature, strdup("function")) == 0 ) {
			printf("The identifier \'%s\', in the line %d, is a function, not a variable.", id_info.token_value, id_info.num_line);
			exit(ERR_FUNCTION);
		}
		lex_val lexem;
		lexem.num_line = get_line_number();
		lexem.token_type = strdup("comando simples");
		lexem.token_value = strdup("=");
		lexem.type = strdup(id_hash_table->type);
		
		$$ = tree_new(lexem);

		if ($3->prog != NULL) {
			char *temp_out2 = malloc(sizeof(char) * 8);
			sprintf(temp_out2, "%d", id_hash_table->offset);
			iloc_op *op_store = newILOCop(
				NULL,
				strdup("storeAI"),
				strdup($3->reg),
				NULL,
				checkContext(stack, strdup(id_info.token_value)),
				strdup(temp_out2),
				0
			);

			$$->prog = addOpToProg($$->prog, op_store);
			tree_add_child($$, $1);
			tree_add_child($$, $3);
		}
	}
}; /* Attribution */
simple_command: id'('argument_list')' {
	lex_val id_lex = $1->info;
	ht_item *id_hash_table = encontrarItemPilha(stack, id_lex.token_value);
	if (id_hash_table == NULL) {
		/* Erro -> Função não foi declarada */
		printf("The function \'%s\', in the line %d, wasn\'t declared.", id_lex.token_value, id_lex.num_line);
		exit(ERR_UNDECLARED);
	}
	else {
		if (strcmp(strdup("function"), id_hash_table->nature) == 0) {
			int str_len = strlen(id_lex.token_value) + 7;
			char *tkn_value = malloc(sizeof(char) * str_len);
			strcpy(tkn_value, "call ");
			strcat(tkn_value, id_lex.token_value);
			lex_val lexem;
			lexem.num_line = get_line_number();
			lexem.token_type = strdup("comando simples");
			lexem.token_value = strdup(tkn_value);
			lexem.type = strdup(id_hash_table->type);
			$$ = tree_new(lexem);
			free(tkn_value);
			tree_add_child($$, $3);
		}
		else {
			/* Erro -> Identificador não é função */
			printf("The identifier \'%s\', in the line %d, isn\'t a function.", id_lex.token_value, id_lex.num_line);
			exit(ERR_VARIABLE);
		}
	}
}; /* Function call */
simple_command: id'('')' {
	lex_val id_lex = $1->info;
	ht_item *id_hash_table = encontrarItemPilha(stack, id_lex.token_value);
	if (id_hash_table == NULL) {
		/* Erro -> Função ainda não foi declarada */
		printf("The function \'%s\', in the line %d, wasn\'t declared.", id_lex.token_value, id_lex.num_line);
		exit(ERR_UNDECLARED);
	}
	else {
		if (strcmp(strdup("function"), id_hash_table->nature) == 0) {
			int str_len = strlen(id_lex.token_value) + 7;
			char *tkn_value = malloc(sizeof(char) * str_len);
			strcpy(tkn_value, "call ");
			strcat(tkn_value, id_lex.token_value);
			lex_val lexem;
			lexem.num_line = get_line_number();
			lexem.token_type = strdup("comando simples");
			lexem.token_value = strdup(tkn_value);
			lexem.type = strdup(id_hash_table->type);
			$$ = tree_new(lexem);
			free(tkn_value);
		}
		else {
			/* Erro -> Identificador não é função */
			printf("The identifier \'%s\', in the line %d, isn\'t a function.", id_lex.token_value, id_lex.num_line);
			exit(ERR_VARIABLE);
		}
	}
}; /* Function call */
simple_command: TK_PR_RETURN precedence_A {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("return");
	lexem.type = strdup($2->info.type);
	$$ = tree_new(lexem);
	tree_add_child($$, $2);
}; /* Return command */
simple_command: TK_PR_IF '(' precedence_A ')' command_block {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("if");
	lexem.type = strdup($3->info.type);
	$$ = tree_new(lexem);

	char *label_if = createLabel(&labelCounter), *label_post = createLabel(&labelCounter);

	$$->label = label_post;

	tree_t *firstOp_cb = findFirstOp($5);

	/*
		Tá aqui o erro atual
		O programa é atualizado aqui, mas por algum motivo, essa mudança não persiste
		Dentro dessa expresão gramatical, o erro está acontecendo
	 */
	if (firstOp_cb->prog->operation->label != NULL) {
		iloc_op *op_nop = newILOCop (
			strdup(label_if),
			strdup("nop"),
			NULL,
			NULL,
			NULL,
			NULL,
			-1
		);
		firstOp_cb->prog = addOpToProgBeginning(firstOp_cb->prog, op_nop);
	}
	else {
		firstOp_cb->prog->operation->label = strdup(label_if);
	}

	iloc_op *op_jump = newILOCop (
		NULL,
		strdup("jumpI"),
		NULL,
		NULL,
		strdup(label_post),
		NULL,
		1
	);

	tree_t *lastSC_cb = findLastProg($5);
	lastSC_cb->prog = addOpToProg(lastSC_cb->prog, op_jump);

	iloc_op *op_cbr = newILOCop (
		NULL,
		strdup("cbr"),
		strdup($3->reg),
		NULL,
		strdup(label_if),
		strdup(label_post),
		1
	);
	$$->prog = addOpToProg($$->prog, op_cbr);

	tree_t *test_5 = $5;

	tree_add_child($$, $3);
	tree_add_child($$, $5);
	tree_add_child($$, NULL);
}; /* Flow Control */
simple_command: TK_PR_IF '(' precedence_A ')' command_block TK_PR_ELSE command_block {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("if");
	lexem.type = strdup($3->info.type);
	$$ = tree_new(lexem);

	/* Seta labels nos command_block */
	char *label_if = createLabel(&labelCounter), *label_else = createLabel(&labelCounter), *label_post = createLabel(&labelCounter);

	/* Após command_block's, deve ter um jump para após do command_block do else */
	$$->label = label_post;

	/* Aplica labels nos blocos de comando relacionados ao if-else */
	tree_t *firstOp_cb1 = findFirstOp($5);
	tree_t *firstOp_cb2 = findFirstOp($7);

	if (firstOp_cb1->prog->operation->label != NULL) {
		iloc_op *op_nop = newILOCop (
			strdup(label_if),
			strdup("nop"),
			NULL,
			NULL,
			NULL,
			NULL,
			-1
		);
		firstOp_cb1->prog = addOpToProgBeginning(firstOp_cb1->prog, op_nop);
	}
	else {
		firstOp_cb1->prog->operation->label = strdup(label_if);
	}

	if (firstOp_cb2->prog->operation->label != NULL) {
		iloc_op *op_nop = newILOCop (
			strdup(label_else),
			strdup("nop"),
			NULL,
			NULL,
			NULL,
			NULL,
			-1
		);
		firstOp_cb2->prog = addOpToProgBeginning(firstOp_cb2->prog, op_nop);
	}
	else {
		firstOp_cb2->prog->operation->label = strdup(label_else);
	}

	iloc_op *op_jump = newILOCop (
		NULL,
		strdup("jumpI"),
		NULL,
		NULL,
		strdup(label_post),
		NULL,
		1
	);

	// label_else não está sendo usado?

	tree_t *lastSC_cb1 = findLastProg($5);
	tree_t *lastSC_cb2 = findLastProg($7);
	lastSC_cb1->prog = addOpToProg(lastSC_cb1->prog, op_jump);
	lastSC_cb2->prog= addOpToProg(lastSC_cb2->prog, op_jump);

	iloc_op *op_cbr = newILOCop (
		NULL,
		strdup("cbr"),
		strdup($3->reg),
		NULL,
		strdup(label_if),
		strdup(label_else),
		1
	);
	$$->prog = addOpToProg($$->prog, op_cbr);

	tree_add_child($$, $3);
	tree_add_child($$, $5);
	tree_add_child($$, $7);
	tree_add_child($$, NULL);

}; /* Flow Control */
simple_command: TK_PR_WHILE '(' precedence_A ')' command_block {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("comando simples");
	lexem.token_value = strdup("while");
	lexem.type = strdup($3->info.type);
	$$ = tree_new(lexem);

	char *label_while = createLabel(&labelCounter), *label_cb = createLabel(&labelCounter), *label_post = createLabel(&labelCounter);

	$$->label = label_post;

	tree_t *firstOp_condition = findFirstOp($3);
	tree_t *firstOp_cb = findFirstOp($5);

	if (firstOp_condition->prog->operation->label != NULL) {
		iloc_op *op_nop = newILOCop (
			strdup(label_while),
			strdup("nop"),
			NULL,
			NULL,
			NULL,
			NULL,
			-1
		);
		firstOp_condition->prog = addOpToProgBeginning(firstOp_condition->prog, op_nop);
	}
	else {
		firstOp_condition->prog->operation->label = strdup(label_while);
	}

	if (firstOp_cb->prog->operation->label != NULL) {
		iloc_op *op_nop = newILOCop (
			strdup(label_cb),
			strdup("nop"),
			NULL,
			NULL,
			NULL,
			NULL,
			-1
		);
		firstOp_cb->prog = addOpToProgBeginning(firstOp_cb->prog, op_nop);
	}
	else {
		firstOp_cb->prog->operation->label = strdup(label_cb);
	}

	iloc_op *op_jump = newILOCop (
		NULL,
		strdup("jumpI"),
		NULL,
		NULL,
		strdup(label_while),
		NULL,
		1
	);

	tree_t *lastSC_cb = findLastProg($5);
	lastSC_cb->prog = addOpToProg(lastSC_cb->prog, op_jump);

	iloc_op *op_cbr = newILOCop (
		NULL,
		strdup("cbr"),
		strdup($3->reg),
		NULL,
		strdup(label_cb),
		strdup(label_post),
		1
	);
	$$->prog = addOpToProg($$->prog, op_cbr);

	tree_add_child($$, $3);
	tree_add_child($$, $5);
	tree_add_child($$, NULL);
}; /* Flow Control */

/* Expressions */
expr: id {
	lex_val id_lex = $1->info;
	ht_item *id_hash_table = encontrarItemPilha(stack, id_lex.token_value);
	if (id_hash_table == NULL) {
		/* Erro -> Variável ainda não foi declarada */
		printf("The variable \'%s\', in the line %d, wasn\'t declared.", id_lex.token_value, id_lex.num_line);
		exit(ERR_UNDECLARED);
	}
	else {
		if (strcmp(id_hash_table->nature, strdup("function")) == 0) {
			/* Erro -> Identificador é função */
			printf("The identifier \'%s\', in the line %d, is a function.", id_lex.token_value, id_lex.num_line);
			exit(ERR_FUNCTION);
		}
	}
	$$ = $1;
	/* Load de variável */
	char *temp_in2 = malloc(sizeof(char) * 8);
	ht_item *id2 = encontrarItemPilha(stack, strdup($1->info.token_value));
	sprintf(temp_in2, "%d", id2->offset);
	iloc_op *op_load = newILOCop (
		NULL,
		strdup("loadAI"),
		checkContext(stack, strdup($1->info.token_value)),
		strdup(temp_in2),
		createRegister(&registerCounter),
		NULL,
		0
	);

	$$->prog = addOpToProg($$->prog, op_load);
	$$->reg = op_load->output_1;
};
expr: lit {
	//printf("%s tem tipo: %s", $1->info.token_value, $1->info.type);
	$$ = $1;
	/* Load Imediato */
	iloc_op *op_load = newILOCop (
		NULL,
		strdup("loadI"),
		strdup($1->info.token_value),
		NULL,
		createRegister(&registerCounter),
		NULL,
		0
	);

	$$->prog = addOpToProg($$->prog, op_load);
	$$->reg = strdup(op_load->output_1);
};
expr: id'('argument_list')' {
	lex_val id_lex = $1->info;
	ht_item *id_hash_table = encontrarItemPilha(stack, id_lex.token_value);
	if (id_hash_table == NULL) {
		/* Erro -> Função não foi declarada */
		printf("The function \'%s\', in the line %d, wasn\'t declared.", id_lex.token_value, id_lex.num_line);
		exit(ERR_UNDECLARED);
	}
	else {
		if (strcmp(strdup("function"), id_hash_table->nature) == 0) {
			int str_len = strlen(id_lex.token_value) + 7;
			char *tkn_value = malloc(sizeof(char) * str_len);
			strcpy(tkn_value, "call ");
			strcat(tkn_value, id_lex.token_value);
			lex_val lexem;
			lexem.num_line = get_line_number();
			lexem.token_type = strdup("comando simples");
			lexem.token_value = strdup(tkn_value);
			lexem.type = strdup(id_lex.type);
			$$ = tree_new(lexem);
			free(tkn_value);
			tree_add_child($$, $3);
			$$->prog = NULL;
		}
		else {
			/* Erro -> Identificador não é função */
			printf("The identifier \'%s\', in the line %d, isn\'t a function.", id_lex.token_value, id_lex.num_line);
			exit(ERR_VARIABLE);
		}
	}
};
expr: id'('')' {
	lex_val id_lex = $1->info;
	ht_item *id_hash_table = encontrarItemPilha(stack, id_lex.token_value);
	if (id_hash_table == NULL) {
		/* Erro -> Função não foi declarada */
		printf("The function \'%s\', in the line %d, wasn\'t declared.", id_lex.token_value, id_lex.num_line);
		exit(ERR_UNDECLARED);
	}
	else {
		if (strcmp(strdup("function"), id_hash_table->nature) == 0) {
			int str_len = strlen(id_lex.token_value) + 7;
			char *tkn_value = malloc(sizeof(char) * str_len);
			strcpy(tkn_value, "call ");
			strcat(tkn_value, id_lex.token_value);
			lex_val lexem;
			lexem.num_line = get_line_number();
			lexem.token_type = strdup("comando simples");
			lexem.token_value = strdup(tkn_value);
			lexem.type = strdup(id_lex.type);
			$$ = tree_new(lexem);
			free(tkn_value);
			$$->prog = NULL;
		}
		else {
			/* Erro -> Identificador não é função */
			printf("The identifier \'%s\', in the line %d, isn\'t a function.", id_lex.token_value, id_lex.num_line);
			exit(ERR_VARIABLE);
		}
	}
	
};

precedence_A: precedence_B{
	$$ = $1;
};
precedence_A: precedence_A TK_OC_OR precedence_B {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("|");
	lexem.type = strdup("bool");
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores

		iloc_op *op_or = newILOCop (
			NULL,
			strdup("or"),
			strdup($1->reg),
			strdup($3->reg),
			createRegister(&registerCounter),
			NULL,
			0
		);

		$$->prog = addOpToProg($$->prog, op_or);
	}

	tree_add_child($$, $1);
	tree_add_child($$, $3);
};

precedence_B: precedence_C{
	$$ = $1;
};
precedence_B: precedence_B TK_OC_AND precedence_C {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("&");
	lexem.type = strdup("bool");
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores
		$$->reg = createRegister(&registerCounter);

		iloc_op *op_and = newILOCop (
			NULL,
			strdup("and"),
			strdup($1->reg),
			strdup($3->reg),
			strdup($$->reg),
			NULL,
			0
		);

		$$->prog = addOpToProg($$->prog, op_and);
	}

	tree_add_child($$, $1);
	tree_add_child($$, $3);
};

precedence_C: precedence_D{
	$$ = $1;
};
precedence_C: precedence_C TK_OC_EQ precedence_D {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("==");
	//lexem.type = strdup(inferencia_tipos($1->info.type, $3->info.type));
	lexem.type = strdup("bool");
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores
		$$->reg = createRegister(&registerCounter);

		iloc_op *op_eq = newILOCop (
			NULL,
			strdup("cmp_EQ"),
			strdup($1->reg),
			strdup($3->reg),
			strdup($$->reg),
			NULL,
			1
		);

		$$->prog = addOpToProg($$->prog, op_eq);
	}

	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_C: precedence_C TK_OC_NE precedence_D {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("!=");
	//lexem.type = strdup(inferencia_tipos($1->info.type, $3->info.type));
	lexem.type = strdup("bool");
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores
		$$->reg = createRegister(&registerCounter);

		iloc_op *op_ne = newILOCop(
			NULL,
			strdup("cmp_NE"),
			strdup($1->reg),
			strdup($3->reg),
			strdup($$->reg),
			NULL,
			1
		);

		$$->prog = addOpToProg($$->prog, op_ne);
	}

	tree_add_child($$, $1);
	tree_add_child($$, $3);
};

precedence_D: precedence_E{
	$$ = $1;
};
precedence_D: precedence_D '<' precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("<");
	lexem.type = strdup("bool");
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores
		$$->reg = createRegister(&registerCounter);

		iloc_op *op_lt = newILOCop (
			NULL,
			strdup("cmp_LT"),
			strdup($1->reg),
			strdup($3->reg),
			strdup($$->reg),
			NULL,
			1
		);

		$$->prog = addOpToProg($$->prog, op_lt);
	}

	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_D: precedence_D '>' precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup(">");
	lexem.type = strdup("bool");
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores
		$$->reg = createRegister(&registerCounter);

		iloc_op *op_gt = newILOCop(
			NULL,
			strdup("cmp_GT"),
			strdup($1->reg),
			strdup($3->reg),
			strdup($$->reg),
			NULL,
			1
		);

		$$->prog = addOpToProg($$->prog, op_gt);
	}

	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_D: precedence_D TK_OC_LE precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("<=");
	lexem.type = strdup("bool");
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores
		$$->reg = createRegister(&registerCounter);

		iloc_op *op_le = newILOCop(
			NULL,
			strdup("cmp_LE"),
			strdup($1->reg),
			strdup($3->reg),
			strdup($$->reg),
			NULL,
			1
		);

		$$->prog = addOpToProg($$->prog, op_le);
	}

	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_D: precedence_D TK_OC_GE precedence_E {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup(">=");
	lexem.type = strdup("bool");
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores
		$$->reg = createRegister(&registerCounter);

		iloc_op *op_ge = newILOCop(
			NULL,
			strdup("cmp_GE"),
			strdup($1->reg),
			strdup($3->reg),
			strdup($$->reg),
			NULL,
			1
		);

		$$->prog = addOpToProg($$->prog, op_ge);
	}

	tree_add_child($$, $1);
	tree_add_child($$, $3);
};

precedence_E: precedence_F{
	$$ = $1;
};
precedence_E: precedence_E '+' precedence_F {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("+");
	lexem.type = strdup(inferencia_tipos($1->info.type, $3->info.type));
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores
		$$->reg = createRegister(&registerCounter);

		iloc_op *op_add = newILOCop(
			NULL,
			strdup("add"),
			strdup($1->reg),
			strdup($3->reg),
			strdup($$->reg),
			NULL,
			0
		);

		$$->prog = addOpToProg($$->prog, op_add);
	}

	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_E: precedence_E '-' precedence_F {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("-");
	lexem.type = strdup(inferencia_tipos($1->info.type, $3->info.type));
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores
		$$->reg = createRegister(&registerCounter);

		iloc_op *op_sub = newILOCop (
			NULL,
			strdup("sub"),
			strdup($1->reg),
			strdup($3->reg),
			strdup($$->reg),
			NULL,
			0
		);

		$$->prog = addOpToProg($$->prog, op_sub);
	}

	tree_add_child($$, $1);
	tree_add_child($$, $3);
};

precedence_F: precedence_G{
	$$ = $1;
};
precedence_F: precedence_F '*' precedence_G {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("*");
	lexem.type = strdup(inferencia_tipos($1->info.type, $3->info.type));
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores
		$$->reg = createRegister(&registerCounter);

		iloc_op *op_mult = newILOCop (
			NULL,
			strdup("mult"),
			strdup($1->reg),
			strdup($3->reg),
			strdup($$->reg),
			NULL,
			0
		);

		$$->prog = addOpToProg($$->prog, op_mult);
	}

	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_F: precedence_F '/' precedence_G {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("/");
	lexem.type = strdup(inferencia_tipos($1->info.type, $3->info.type));
	$$ = tree_new(lexem);

	if (($1->reg != NULL) && ($3->reg != NULL)) { // Resultados estão em registradores
		$$->reg = createRegister(&registerCounter);

		iloc_op *op_div = newILOCop (
			NULL,
			strdup("div"),
			strdup($1->reg),
			strdup($3->reg),
			strdup($$->reg),
			NULL,
			0
		);

		$$->prog = addOpToProg($$->prog, op_div);
	}
	tree_add_child($$, $1);
	tree_add_child($$, $3);
};
precedence_F: precedence_F '%' precedence_G {
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("%");
	lexem.type = strdup(inferencia_tipos($1->info.type, $3->info.type));
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
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("-");
	lexem.type = strdup($2->info.type);
	$$ = tree_new(lexem);

	if ($2->reg != NULL) {

		char *reg_temp = createRegister(&registerCounter);
		iloc_op *op_load = newILOCop (
			NULL,
			strdup("loadI"),
			strdup("0"),
			NULL,
			strdup(reg_temp),
			NULL,
			0
		);

		$$->prog = addOpToProg($$->prog, op_load);

		$$->reg = createRegister(&registerCounter);

		iloc_op *op_sub = newILOCop (
			NULL,
			strdup("sub"),
			strdup(op_load->output_1),
			strdup($2->reg),
			strdup($$->reg),
			NULL,
			0
		);

		$$->prog = addOpToProg($$->prog, op_sub);
	}

	tree_add_child($$, $2);
};
precedence_G: '!'precedence_G {
	lex_val expr_info = $2->info;
	lex_val lexem;
	lexem.num_line = get_line_number();
	lexem.token_type = strdup("operador");
	lexem.token_value = strdup("!");
	lexem.type = strdup($2->info.type);
	$$ = tree_new(lexem);

	if ($2->reg != NULL) {

		/* Criação de um temporário possuindo true como valor */
		char *reg_temp1 = createRegister(&registerCounter);
		iloc_op *op_load1 = newILOCop(
			NULL,
			strdup("loadI"),
			strdup("1"),
			NULL,
			strdup(reg_temp1),
			NULL,
			0
		);
		$$->prog = addOpToProg($$->prog, op_load1);

		char *reg_temp2 = createRegister(&registerCounter);
		iloc_op *op_cmp = newILOCop (
			NULL,
			strdup("cmp_EQ"),
			strdup(reg_temp1),
			strdup(reg_temp1),
			strdup(reg_temp2),
			NULL,
			1
		);
		$$->prog = addOpToProg($$->prog, op_cmp);

		/* True XOR p -> !p */
		$$->reg = createRegister(&registerCounter);
		iloc_op *op_xor = newILOCop (
			NULL,
			strdup("xor"),
			strdup($2->reg),
			strdup(reg_temp2),
			strdup($$->reg),
			NULL,
			0
		);
		$$->prog = addOpToProg($$->prog, op_xor);
	}

	tree_add_child($$, $2);
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

	$$ = strdup(new_list);

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

param: type id {
	char *id = $2->info.token_value;
	char *type = $1.type;
	addItemEscopo(stack, strdup(id), get_line_number(), strdup("variable"), strdup(type));

	free(id);
	free(type);
	$$ = NULL;
};

/* Arguments */
argument_list: precedence_A {
	$$ = $1;
};
argument_list: precedence_A',' argument_list {
	$$ = $1;
	tree_add_child($$, $3);
};

%%
