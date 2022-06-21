#include <stdio.h>
#include <stdlib.h>

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer pointer, size_t length) {
    for (int i = 0; i < length; ++i) {
        //C 格式化指令 "%.2x" 表明整数必须用至少两个数字的十六进制格式输出。
        //引用 pointer[i) 表示我们想要读取以 pointer 指向的位置为起始的第 i 个位置处的字节。
        printf("%.2x", pointer[i]);
    }
    printf("\n");
}

void show_int(int x) {
    // 这种强制类型转换告诉编译器， 程序应该把这个指针看成指向一个字节序列， 而不是指向一个原始数据类型的对象。 然后， 这个指针会被看成是对象使用的最低字节地址。
    show_bytes((byte_pointer) &x, sizeof(int));
}

void show_float(float x) {
    show_bytes((byte_pointer) &x, sizeof(float));
}

void show_pointer(void *x) {
    show_bytes((byte_pointer) &x, sizeof(void *));
}

int main(int argc, char **argv) {
    show_int(10);

    return EXIT_SUCCESS;
}