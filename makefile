HackAssembler.exe : HackAssembler.o parser.o code.o symbol.o
	gcc -Wall -Wextra -Wpedantic -g -o HackAssembler.exe HackAssembler.o parser.o code.o symbol.o

HackAssembler.o : HackAssembler.c parser.h code.h symbol.h
	gcc -Wall -Wextra -Wpedantic -g -c HackAssembler.c

parser.o : parser.c parser.h
	gcc -Wall -Wextra -Wpedantic -g -c parser.c

code.o : code.c code.h
	gcc -Wall -Wextra -Wpedantic -g -c code.c

symbol.o :symbol.c symbol.h
	gcc -Wall -Wextra -Wpedantic -g -c symbol.c
