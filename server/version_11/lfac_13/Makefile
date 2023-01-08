all:
	lex tema.l
	yacc -d tema2.y
	gcc lex.yy.c y.tab.c -o ex
clean:
	rm -f ex lex.yy.c y.tab.c
