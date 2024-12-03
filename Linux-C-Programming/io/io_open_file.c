/*
 * =======================================
 *  演示 Linux 文件操作。
 * =======================================
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

void open_file(const char *filename) {
  int fd = open(filename, O_RDWR | O_CREAT);
  printf("new file fd = %d\n", fd);
  int ret = close(fd);
  printf("close new file ret = %d\n", ret);
}

void delete_file(const char *filename) {
  // unlink: Remove the link NAME.
  int ret = unlink("test.txt");
  printf("delete new file ret = %d\n", ret);
  perror("delete file error");
}

int main(int argc, const char *argv[]) {
  // open_file("test.txt");
  delete_file("test.txt");
}