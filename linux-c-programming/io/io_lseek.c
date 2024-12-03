/*
 * =======================================
 *  演示 Linux 文件操作。
 * =======================================
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void crate_test_file() {
  int fd = open("test.txt", O_RDWR | O_CREAT);
  char buff[30];
  strcpy(buff, "Hello");
  write(fd, buff, strlen(buff));
  close(fd);
}

/*
 * 每个打开的文件都记录着当前读写位置，打开文件时读写位置是 0，表示文件开头，通常读写多少个字节就会将
 * 读写位置往后移多少个字节。但是有一个例外，如果以 O_APPEND 方式打开，每次写操作都会在文件末尾追加
 * 数据，然后将读写位置移到新的文件末尾。lseek 和标准 I/O 库的 fseek 函数类似，可以移动当前读写位
 * 置（或者叫偏移量）。
 */
void test_lseek() {
  int fd = open("test.txt", O_RDWR);
  printf("fd = %d \n", fd);
  long offset = lseek(fd, 0L, SEEK_END);
  printf("offset = %ld \n", offset);
}

int main(void) {
  crate_test_file();
  test_lseek();
  return 0;
}
