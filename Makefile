all: parser.y scanner.l utils.c
	bison -d parser.y
	flex -o scanner.lex.c scanner.l
	gcc -fsanitize=address -g -Werror -o etapa3 utils.c scanner.lex.c parser.tab.c -lm 

utils: utils.h utils.c main.c
	gcc -fsanitize=address -g -Werror -o etapa3 utils.c main.c

clean:
	rm -rf parser scanner etapa3
