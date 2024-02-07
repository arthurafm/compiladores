all: etapa6

bison:
	bison -d parser.y

lex.yy.c: scanner.l bison
	flex scanner.l

etapa6: lex.yy.c main.c parser.tab.c
	@(gcc -o etapa6 -g main.c lex.yy.c parser.tab.c utils.c)

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h etapa6