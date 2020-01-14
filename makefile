##################################
# makefile
# Philip Taggart
#
# makefile for myshell and myls
# executables.  Includes clean
# function.
##################################

CC=gcc 
CFLAGS=-g -Wall -std=c11 -pedantic

all: myshell myls
myshell: myshell.o builtIn.o shellCmdFunction.o
	$(CC) $(CFLAGS) -o myshell myshell.o builtIn.o shellCmdFunction.o

%.o : %.c 
	$(CC) -c $(CFLAGS) $< -o $@ 

myls: myls.c
	$(CC) $(CFLAGS) -o myls myls.c


clean:
	rm -f myshell
	rm -f myls
	rm -f *.o
	rm -f *~
	rm -f core