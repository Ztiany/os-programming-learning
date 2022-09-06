#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

//=================================================
//【程序 10-2：捕捉 SIGUSR1 和 SIGUSR2 的简单程序】
//=================================================

#include    <lib/apue.h>

static void sig_usr(int);    /* one handler for both signals */

int main(void) {
    printf("my pid = %d", getpid());
    fflush(stdout);

    if (signal(SIGUSR1, sig_usr) == SIG_ERR)
        err_sys("can't catch SIGUSR1");

    if (signal(SIGUSR2, sig_usr) == SIG_ERR)
        err_sys("can't catch SIGUSR2");

    for (;;)
        pause();
}

/* argument is signal number */
static void sig_usr(int sig_no) {
    if (sig_no == SIGUSR1)
        printf("received SIGUSR1\n");
    else if (sig_no == SIGUSR2)
        printf("received SIGUSR2\n");
    else
        err_dump("received signal %d\n", sig_no);
}

#pragma clang diagnostic pop