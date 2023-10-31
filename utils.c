#include <stdio.h>

extern int yylineno;

int get_line_number() {
	return yylineno;
}

void yyerror(const char *s) {
	printf("In the line %d, the following error occurred: %s", get_line_number(), s);
}