%{
	/* GRUPO G */
	/* Arthur Alves Ferreira Melo - 00333985 */
	/* Emanuel Pacheco Thiel -  00170313 */

	#include "parser.tab.h"

%}

%option yylineno
%option noyywrap

%%

[ ]+ { }
[\t]+ { }
[\r]+ { }
\n { }
\/\/(.)* { /* In-line comment - DO NOTHING */ }
\/\*(.|\n)*\*\/ { /* Multi-line comment - DO NOTHING */ }
int return TK_PR_INT;
float return TK_PR_FLOAT;
bool return TK_PR_BOOL;
if return TK_PR_IF;
else return TK_PR_ELSE;
while return TK_PR_WHILE;
return return TK_PR_RETURN;
\- return *yytext;
\! return *yytext;
\* return *yytext;
\/ return *yytext;
\% return *yytext;
\+ return *yytext;
\< return *yytext;
\> return *yytext;
\{ return *yytext;
\} return *yytext;
\( return *yytext;
\) return *yytext;
\= return *yytext;
\, return *yytext;
\; return *yytext;
(\<\=) return TK_OC_LE;
(\>\=) return TK_OC_GE;
(\=\=) return TK_OC_EQ;
(\!\=) return TK_OC_NE;
(\&) return TK_OC_AND;
(\|) return TK_OC_OR;
false {
	yylval.valor_lexico.num_line = get_line_number();
	yylval.valor_lexico.token_type = strdup("literal");
	yylval.valor_lexico.token_value = strdup(yytext);
	return TK_LIT_FALSE;
	}
true {
	yylval.valor_lexico.num_line = get_line_number();
	yylval.valor_lexico.token_type = strdup("literal");
	yylval.valor_lexico.token_value = strdup(yytext);
	return TK_LIT_TRUE;
	}
([a-z]|_)([a-z]|_|[0-9])* {
	yylval.valor_lexico.num_line = get_line_number();
	yylval.valor_lexico.token_type = strdup("identificador");
	yylval.valor_lexico.token_value = strdup(yytext);
	return TK_IDENTIFICADOR;
	}
([0-9])* {
	yylval.valor_lexico.num_line = get_line_number();
	yylval.valor_lexico.token_type = strdup("literal");
	yylval.valor_lexico.token_value = strdup(yytext);
	return TK_LIT_INT;
	}
([0-9])*\.([0-9])+ {
	yylval.valor_lexico.num_line = get_line_number();
	yylval.valor_lexico.token_type = strdup("literal");
	yylval.valor_lexico.token_value = strdup(yytext);
	return TK_LIT_FLOAT;
	}
. return TK_ERRO;

%%


