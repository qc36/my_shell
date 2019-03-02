#include <stdio.h>

int main() {
  int input[2] = {0};
  for (int i = 0; i <= 5; i++) {
    input[i] = i;
    printf("%c", input[i]);
  }
  return 0;
}
