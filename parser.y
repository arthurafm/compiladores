%{ 
    /* GRUPO G */
	/* Arthur Alves Ferreira Melo - 00333985 */
	/* Emanuel Pacheco Thiel -  00170313 */
	
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

/* The Language */
program: list;
program: /* Empty symbol */;

list: element list;
list: element;

element: var;
element: function;

/* Variables */
var: type id_list';';

/* Functions */
function: function_header function_body;

function_header: param_list_parenthesis TK_OC_GE type'!' id;

function_body: '{' simple_command_list '}';
function_body: '{' '}';

/* Simple Commands */
simple_command_list: simple_command';';
simple_command_list: simple_command';' simple_command_list;

simple_command: type id_list;													/* Variable declaration */
simple_command: id '=' precedence_A;													/* Attribution */
simple_command: id'('argument_list')';											/* Function call */
simple_command: TK_PR_RETURN precedence_A;												/* Return command */
simple_command: TK_PR_IF '(' precedence_A ')' function_body;							/* Flow Control */
simple_command: TK_PR_IF '(' precedence_A ')' function_body TK_PR_ELSE function_body;	/* Flow Control */
simple_command: TK_PR_WHILE '(' precedence_A ')' function_body;							/* Flow Control */

/* Expressions */
expr: id;
expr: lit;
expr: id'('argument_list')';

precedence_A: precedence_B;
precedence_A: precedence_A TK_OC_OR precedence_B;

precedence_B: precedence_C;
precedence_B: precedence_B TK_OC_AND precedence_C;

precedence_C: precedence_D;
precedence_C: precedence_C TK_OC_EQ precedence_D;
precedence_C: precedence_C TK_OC_NE precedence_D;

precedence_D: precedence_E;
precedence_D: precedence_D '<' precedence_E;
precedence_D: precedence_D '>' precedence_E;
precedence_D: precedence_D TK_OC_LE precedence_E;
precedence_D: precedence_D TK_OC_GE precedence_E;

precedence_E: precedence_F;
precedence_E: precedence_E '+' precedence_F;
precedence_E: precedence_E '-' precedence_F;

precedence_F: precedence_G;
precedence_F: precedence_F '*' precedence_G;
precedence_F: precedence_F '/' precedence_G;
precedence_F: precedence_F '%' precedence_G;

precedence_G: expr;
precedence_G: '('precedence_A')';
precedence_G: '-'precedence_G;
precedence_G: '!'precedence_G;


/* -- Aux expressions -- */


/* Types */
type: TK_PR_INT;	/* int */
type: TK_PR_FLOAT;	/* float */
type: TK_PR_BOOL;	/* bool */

/* Identifiers */
id: TK_IDENTIFICADOR;

id_list: id;
id_list: id',' id_list;

/* Literals */
lit: TK_LIT_INT;
lit: TK_LIT_FLOAT;
lit: TK_LIT_TRUE;
lit: TK_LIT_FALSE;

/* Parameters */
param_list_parenthesis: '(' ')';
param_list_parenthesis: '(' param_list ')';

param_list: param;
param_list: param ',' param_list;

param: type id;

/* Arguments */
argument_list: expr',' argument_list;
argument_list: expr;


%%
