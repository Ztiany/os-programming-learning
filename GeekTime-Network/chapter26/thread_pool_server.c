/*
 ============================================================================
 
 Author      : Ztiany
 Description : 基于线程尺的 server

 ============================================================================
 */
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "lib/read.h"
#include "../lib/common.h"
#include "../lib/tcp_server.h"

#define  THREAD_NUMBER      4
#define  BLOCK_QUEUE_SIZE   100

extern void loop_echo(int);

/** 线程池 */
typedef struct {
    /** thread ID */
    long thread_count;
    /** connections handle 【没有用到】*/
    pthread_t thread_id;
} Thread;

/**  客户端 fd 队列 */
typedef struct {
    /** 队列长度 */
    int length;
    /** 一个 client fd 数组 */
    int *fd;
    /** 队尾 */
    int rear;
    /** 队首 */
    int front;

    /*线程安全相关*/
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} block_queue;

/** 初始化队列 */
void block_queue_init(block_queue *queue, int queue_length) {
    queue->length = queue_length;
    queue->fd = calloc(queue_length, sizeof(int));
    queue->front = queue->rear = 0;
    pthread_mutex_init(&(queue->mutex), NULL);
    pthread_cond_init(&(queue->cond), NULL);
}

/** 入队 */
void block_queue_push(block_queue *queue, int fd) {
    pthread_mutex_lock(&queue->mutex);

    //【TODO】没有考虑队列满的情况
    queue->fd[queue->rear] = fd;
    if (++queue->rear == queue->length) {
        queue->rear = 0;
    }
    yolanda_msgx("push client(%d)", fd);

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

/** 出队 */
int block_queue_pop(block_queue *queue) {
    pthread_mutex_lock(&queue->mutex);

    //【TODO】没有考虑队列满的情况
    while (queue->front == queue->rear) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    int fd = queue->fd[queue->front];
    if (++queue->front == queue->length) {
        queue->front = 0;
    }
    yolanda_msgx("pop client(%d)", fd);

    pthread_mutex_unlock(&queue->mutex);

    return fd;
}

void *server_task(void *args) {
    //分离线程
    pthread_t p_tid = pthread_self();
    pthread_detach(p_tid);

    block_queue *queue = (block_queue *) args;

    while (true) {
        int fd = block_queue_pop(queue);

        printf("thread(%lu) start to serve client(%d).\n", p_tid, fd);
        //循环地服务该客户端，直到该客户端退出。
        loop_echo(fd);
        printf("client(%d) exited, and thread(%lu) is free now.\n", fd, p_tid);
    }

    return NULL;
}

/** 线程池 */
Thread *thread_array;

/*客户端使用 telnet 即可。*/
int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    //创建一个 tcp server
    int server_fd = tcp_server_listen(SERV_PORT);

    //初始化一个队列
    block_queue queue;
    block_queue_init(&queue, BLOCK_QUEUE_SIZE);

    //初始化一个线程池
    thread_array = calloc(THREAD_NUMBER, sizeof(Thread));
    for (int i = 0; i < THREAD_NUMBER; ++i) {
        pthread_create(&(thread_array[i].thread_id), NULL, server_task, &queue);
    }

    while (true) {
        struct sockaddr_in child_addr;
        bzero(&child_addr, sizeof(child_addr));
        socklen_t child_addr_len = sizeof(child_addr);
        int client_fd = accept(server_fd, TO_SOCK_ADDR(child_addr), &child_addr_len);
        if (client_fd < 0) {
            error(1, errno, "accept error");
        }

        yolanda_msgx("a new client(%d) has been accepted.", client_fd);
        block_queue_push(&queue, client_fd);
    }

    return EXIT_SUCCESS;
}
