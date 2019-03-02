CFLAGS=-Wall -Werror -ggdb3 -pedantic 
myShell:myShell.cpp myshell.h inputhandle.h
	g++ $(CFLAGS) -o myShell  myShell.cpp
 