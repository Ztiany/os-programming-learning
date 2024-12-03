#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>

/*
 * =======================================
 *  演示 System V 消息队列：发送端
 * =======================================
 */

#define MSG_LENGTH 1024

struct message {
  long type;
  char content[MSG_LENGTH];
};

int main(int argc, const char *argv[]) {
  int messageQueueId;
  key_t key;
  int length;
  int result;

  // ftok: Generates key for System V style IPC.
  if ((key = ftok("/", 1024)) < 0) {
    printf("ftok error \n");
  }
  printf("Message Queue Key = %d\n", key);

  // msgget: Get messages queue.
  if ((messageQueueId = msgget(key, IPC_CREAT | 0666)) == -1) {
    printf("msgget error \n");
    exit(EXIT_FAILURE);
  }
  printf("Message Queue Id = %d\n", messageQueueId);

  struct message message;
  memset(&message, 0, sizeof(message));

  message.type = getpid();
  while (fgets(message.content, MSG_LENGTH, stdin) != NULL) {
    length = strlen(message.content);
    if (length == 1) {
      break;
    }

    if (message.content[length - 1] == '\n') {
      message.content[length - 1] = '\0';
    }
    printf("sending: %s\n", message.content);
    result = msgsnd(messageQueueId, &message, length, 0);
    printf("sending result: %d\n", result);
  }

  result = msgctl(messageQueueId, IPC_RMID, NULL);
  printf("destroy msg result: %d\n", result);
  return EXIT_SUCCESS;
}