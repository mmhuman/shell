CC=gcc
FLAGS=-Wall -ansi -pedantic -g

.PHONY: all
all: shell

shell: main.c _error.o environment.o read.o processes.o structs.o functions.o
	$(CC) $(FLAGS) main.c _error.o environment.o read.o processes.o structs.o functions.o -o shell

_error.o: _error.h _error.c 
	$(CC) -c $(FLAGS) _error.c

environment.o: environment.c environment.h
	$(CC) -c $(FLAGS) environment.c

read.o: read.c read.h
	$(CC) -c $(FLAGS) read.c

structs.o: structs.c structs.h
	$(CC) -c $(FLAGS) structs.c

processes.o: processes.c processes.h
	$(CC) -c $(FLAGS) processes.c

functions.o: functions.c functions.h
	$(CC) -c $(FLAGS) functions.c



.PHONY: clean
clean:
	rm -f *.o
