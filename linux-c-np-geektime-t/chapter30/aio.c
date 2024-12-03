/*
 ============================================================================
 
 Author      : Ztiany
 Description : aio 演示

 ============================================================================
 */
#include <aio.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "../lib/log.h"
#include "../lib/common.h"

int main(int argc, char **argv) {
    int err;
    int result_size;

    // 创建一个临时文件
    char tmp_name[256];
    snprintf(tmp_name, sizeof(tmp_name), "/tmp/aio_test_%d", getpid());
    unlink(tmp_name);

    int fd = open(tmp_name, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        error(1, errno, "open file failed");
    }
    unlink(tmp_name);

    char buf[BUFFER_SIZE];
    struct aiocb aio_callback;

    //初始化 buf 缓冲，写入的数据应该为 0xfafa 这样的,
    memset(buf, 0xfa, BUFFER_SIZE);
    memset(&aio_callback, 0, sizeof(struct aiocb));
    aio_callback.aio_fildes = fd;
    aio_callback.aio_buf = buf;
    aio_callback.aio_nbytes = BUFFER_SIZE;

    //开始写
    if (aio_write(&aio_callback) == -1) {
        printf(" Error at aio_write(): %s\n", strerror(errno));
        close(fd);
        exit(1);
    }

    //因为是异步的，需要判断什么时候写完
    while (aio_error(&aio_callback) == EINPROGRESS) {
        printf("writing... \n");
    }

    //判断写入的是否正确
    err = aio_error(&aio_callback);
    result_size = aio_return(&aio_callback);
    if (err != 0 || result_size != BUFFER_SIZE) {
        printf(" aio_write failed() : %s\n", strerror(err));
        close(fd);
        exit(1);
    }

    //下面准备开始读数据
    char buffer[BUFFER_SIZE];
    struct aiocb cb;
    cb.aio_nbytes = BUFFER_SIZE;
    cb.aio_fildes = fd;
    cb.aio_offset = 0;
    cb.aio_buf = buffer;

    // 开始读数据
    if (aio_read(&cb) == -1) {
        printf(" air_read failed() : %s\n", strerror(err));
        close(fd);
    }

    //因为是异步的，需要判断什么时候读完
    while (aio_error(&cb) == EINPROGRESS) {
        printf("Reading... \n");
    }

    // 判断读是否成功
    int numBytes = aio_return(&cb);
    if (numBytes != -1) {
        printf("Success.\n");
    } else {
        printf("Error.\n");
    }

    // 清理文件句柄
    close(fd);
    return 0;
}
