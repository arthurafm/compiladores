all: etapa1

lex.yy.c: scanner.l
	@(flex scanner.l)	

etapa1: lex.yy.c main.c tokens.h
	@(gcc -o etapa1 main.c lex.yy.c get_line_number.c)