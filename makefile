run: Sim01
	./Sim01 config_1.cnf
Sim01: Sim01.o
	gcc -Wall -o Sim01 Sim01.o
Sim01.o: Sim01.c
	gcc -lpthread -c Sim01.c
