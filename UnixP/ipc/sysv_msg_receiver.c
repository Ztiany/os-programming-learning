#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>

//=================================================
//演示 System V 消息队列：接收端
//=================================================

#define MSG_LENGTH 1024

struct message {
    long type;
    char content[MSG_LENGTH];
};

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        printf("pass a message queue id through the arguments.\n");
        exit(EXIT_FAILURE);
    }

    int length;
    int messageQueueId;
    struct message message;
    memset(&message, 0, sizeof(message));

    messageQueueId = atoi(argv[1]);

    while (1) {
        length = msgrcv(messageQueueId, &message, MSG_LENGTH, 0, 0);
        printf("receive length: %d\n", length);
        if (length > 0) {
            printf("receive result: %s\n", message.content);
        } else {
            break;
        }
    }

    return EXIT_SUCCESS;
}