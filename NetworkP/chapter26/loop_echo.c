#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <error.h>
#include "../lib/common.h"
#include "../lib/chars.h"

void loop_echo(int fd) {
    char output_buf[MAX_LINE + 1];
    size_t output_buf_used = 0;
    int result;

    while (1) {
        char ch;
        result = recv(fd, &ch, 1, 0);

        //断开连接或者出错
        if (result == 0) {
            break;
        } else if (result == -1) {
            error(1, errno, "read error");
            break;
        }

        if (output_buf_used < sizeof(output_buf)) {
            output_buf[output_buf_used++] = rot13_char(ch);
        }

        if (ch == '\n') {
            send(fd, output_buf, output_buf_used, 0);
            output_buf_used = 0;
            continue;
        }
    }

}