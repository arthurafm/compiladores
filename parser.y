%{ 
    /* GRUPO G */
	/* Arthur Alves Ferreira Melo - 00333985 */
	/* Emanuel Pacheco Thiel -  */
	
	int yylex(void);
	void yyerror (char const *mensagem);
%}

%define parse.error verbose /* Syntax error report */

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
%token TK_IDENTIFICADOR
%token TK_LIT_INT
%token TK_LIT_FLOAT
%token TK_LIT_FALSE
%token TK_LIT_TRUE
%token TK_ERRO

%%

program: list; 
program: /* Empty symbol */ ;

list: list element;
list: element;

element: function;
element: global_var;

type: TK_PR_INT;	/* int */
type: TK_PR_FLOAT;	/* float */
type: TK_PR_BOOL;	/* bool */

id: TK_IDENTIFICADOR;

id_list: id;
id_list: id_list','id;

global_var: type id_list';';

param: type id;

param_list: param;
param_list: param_list',' param;

function_header: '(' param_list ')' TK_OC_GE type'!' id;

lit: TK_LIT_INT;
lit: TK_LIT_FLOAT;
lit: TK_LIT_TRUE;
lit: TK_LIT_FALSE;

operand: id;
operand: lit;
operand: id'('argument_list')';

expr: operand;

argument: id;
argument: expr;

argument_list: argument;
argument_list: argument_list',' argument;

simple_command: type id_list;			/* Variable declaration */
simple_command: id '=' expr;			/* Attribution */
simple_command: id'('argument_list')';	/* Function call */
simple_command: TK_PR_RETURN expr;		/* Return command */

simple_command_list: simple_command;
simple_command_list: simple_command_list';' simple_command';';

function_body: '{' simple_command_list '}'
function_body: '{''}';

function: function_header function_body;


%%
