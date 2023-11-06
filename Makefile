all: etapa2

bison:
	@(bison -d parser.y)

lex.yy.c: scanner.l bison
	@(flex scanner.l)	

etapa2: lex.yy.c main.c parser.tab.c
	@(gcc -o etapa2 main.c lex.yy.c parser.tab.c utils.c)