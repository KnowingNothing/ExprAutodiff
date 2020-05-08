CC = gcc

auto: auto.tab.o lex.yy.o util.o genIR.o
	$(CC) auto.tab.o lex.yy.o util.o genIR.o -lfl -lstdc++ -o auto 

auto.tab.c: auto.bison
	bison auto.bison
	mv auto.tab.bison auto.tab.c

lex.yy.c:auto.flex
	flex auto.flex

auto.tab.o:auto.tab.c globals.h util.h
	$(CC) auto.tab.c -c -D PARSER

lex.yy.o:lex.yy.c globals.h
	$(CC) lex.yy.c -c

util.o: util.h util.c globals.h
	$(CC) util.c -c

genIR.o: genIR.cc genIR.h
	g++ genIR.cc -c
	
clean:
	-rm -f *.o lex.yy.c auto.tab.c
