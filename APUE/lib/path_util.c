#include "path_util.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int getPwd(char *pwd, int length) {
    return (getcwd(pwd, length) == NULL) ? 0 : 1;
}

void printPwd() {
    char pwd[251];
    memset(pwd, 0, sizeof(pwd));
    if (getPwd(pwd, 250)) {
        printf("current directory: %s", pwd);
    } else {
        printf("the directory couldn't be determined or SIZE was too small");
    }
}