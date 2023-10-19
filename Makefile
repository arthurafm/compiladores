CC=gcc
CFLAGS=-I.
DEPS = tokens.h
OBJ = lex.yy.o get_line_number.o main.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

etapa1: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
