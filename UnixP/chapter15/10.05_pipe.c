#include    <lib/apue.h>

//=================================================
//【程序 10-5：经由管道从父进程向子进程传送数据】
//=================================================

void testPipeType(int fd) {
    //测试管道的文件类型
    struct stat st;
    fstat(fd, &st);
    printf("fd is FIFO: %d\n", S_ISFIFO(st.st_mode));
}

int main(int argc, const char *argv[]) {
    int n;
    int fd[2];
    pid_t pid;
    char line[MAXLINE];

    if (pipe(fd) < 0) {//创建一个管道，对应两个 fd，一个用于读，一个用于写。
        err_sys("pipe error");
    }

    testPipeType(fd[0]);

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid > 0) {//parent
        close(fd[0]);
        write(fd[1], "hello world\n", 12);
    } else {//child
        close(fd[1]);
        n = read(fd[0], line, MAXLINE);
        write(STDOUT_FILENO, line, n);
    }

    exit(EXIT_SUCCESS);
}