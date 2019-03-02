#include <stdio.h>
#include <stdlib.h>

#include <iostream>

int main(int argc, char * argv[]) {
  if (argc < 2) {
    fprintf(stderr, "wrong former!\n");
    exit(EXIT_FAILURE);
  }
  for (int i = 1; i < argc; i++) {
    std::cout << "argv[" << i << "]:" << argv[i] << std::endl;
  }
  return 0;
}
