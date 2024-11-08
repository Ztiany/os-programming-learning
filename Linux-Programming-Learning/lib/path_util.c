#include "path_util.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int getPwd(char *pwd, const int length) {
  return (getcwd(pwd, length) == NULL) ? 0 : 1;
}

void printPwd() {
  char pwd[251] = {0};
  if (getPwd(pwd, 250)) {
    printf("current directory: %s", pwd);
  } else {
    printf("the directory couldn't be determined or SIZE was too small");
  }
}