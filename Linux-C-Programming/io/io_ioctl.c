/*
 * =======================================
 *  演示 Linux 文件操作。
 * =======================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

/*
 ioctl 用于向设备发控制和配置命令，有些命令也需要读写一些数据，但这些数据是不能用 read/write 读
 写的，称为 Out-of-band 数据。也就是说，read/write 读写的数据是 in-band 数据，是 I/O 操作的
 主体，而 ioctl 命令传送的是控制信息，其中的数据是辅助的数据。例如，在串口线上收发数据通过
 read/write 操作，而串口的波特率、校验位、停止位通过 ioctl 设置，A/D 转换的结果通过 read 读取，
 而 A/D 转换的精度和工作频率通过 ioctl 设置。

 以下程序使用 TIOCGWINSZ 命令获得终端设备的窗口大小，在图形界面的终端里多次改变终端窗口的大小并运
 行该程序，观察结果。
*/
int main(void) {
  struct winsize size;

  if (isatty(STDOUT_FILENO) == 0)
    exit(1);

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) < 0) {
    perror("ioctl TIOCGWINSZ error");
    exit(1);
  }
  printf("%d rows, %d columns\n", size.ws_row, size.ws_col);

  return 0;
}