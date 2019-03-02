#include "myshell.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#include "inputhandle.h"
extern char ** environ;

myshell myShell;
int main() {
  myShell.update_pathlist();
  while (1) {
    inputhandle(myShell);
  }
  return EXIT_SUCCESS;
}
