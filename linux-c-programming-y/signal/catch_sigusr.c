/*
 * =======================================
 *   捕捉 SIGUSR1 和 SIGUSR2 的简单程序
 * =======================================
 */

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static void sig_usr(int); /* one handler for both signals */

int main(void) {
  printf("my pid = %d", getpid());
  fflush(stdout);

  if (signal(SIGUSR1, sig_usr) == SIG_ERR)
    printf("can't catch SIGUSR1\n");

  if (signal(SIGUSR2, sig_usr) == SIG_ERR)
    printf("can't catch SIGUSR2\n");

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
    printf("received signal %d\n", sig_no);
}

#pragma clang diagnostic pop