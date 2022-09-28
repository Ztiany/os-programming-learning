/**********************************************************************\
*                Copyright (C) Michael Kerrisk, 2010.                  *
*                                                                      *
* This program is free software. You may use, modify, and redistribute *
* it under the terms of the GNU Affero General Public License as       *
* published by the Free Software Foundation, either version 3 or (at   *
* your option) any later version. This program is distributed without  *
* any warranty. See the file COPYING for details.                      *
\**********************************************************************/

/* t_mmap.c

   Demonstrate the use of mmap() to create a shared file mapping.
*/
#include <sys/mman.h>
#include <fcntl.h>
#include <lib/tlpi_hdr.h>

#define MEM_SIZE 10

/** 提示：确保所指定的文件的 size >= MEM_SIZE 指定的值。 */
int main(int argc, char *argv[]) {
    //内存地址和文件描述符
    char *addr = NULL;
    int fd;

    //step1: 检查参数
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file [new-value]\n", argv[0]);


    //step2: 打开文件
    fd = open(argv[1], O_RDWR);
    if (fd == -1)
        errExit("open");

    //step3: 执行映射
    addr = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (addr == MAP_FAILED || addr == NULL)
        errExit("mmap");

    if (close(fd) == -1)                /* No longer need 'fd' */
        errExit("close");

    printf("Current string=%.*s\n", MEM_SIZE, addr);
    /* Secure practice: output at most MEM_SIZE bytes */

    //step4: 往共享内存中写入数据
    if (argc > 2) {                     /* Update contents of region */
        if (strlen(argv[2]) >= MEM_SIZE)
            cmdLineErr("'new-value' too large\n");

        memset(addr, 0, MEM_SIZE);      /* Zero out region */
        strncpy(addr, argv[2], MEM_SIZE - 1);

        if (msync(addr, MEM_SIZE, MS_SYNC) == -1)
            errExit("msync");

        printf("Copied \"%s\" to shared memory\n", argv[2]);
    }

    exit(EXIT_SUCCESS);
}