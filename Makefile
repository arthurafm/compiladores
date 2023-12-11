all: etapa4

bison:
	bison -d parser.y

lex.yy.c: scanner.l bison
	flex scanner.l

etapa3: lex.yy.c main.c parser.tab.c
	@(gcc -o etapa4 main.c lex.yy.c parser.tab.c utils.c)