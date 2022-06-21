#include <stdio.h>
#include <stdlib.h>

void inplace_swap(int *x, int *y) {
    *y = *x ^ *y;
    *x = *x ^ *y;
    *y = *x ^ *y;
}

void reverse_array(int arr[], int cnt) {
    int first, last;
    for (first = 0, last = cnt - 1; first < last; first++, last--) {
        inplace_swap(&arr[first], &arr[last]);
    }
}

void print_array(int arr[], int length) {
    for (int i = 0; i < length; ++i) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    printf("===单个数交换===：\n");
    int x = 100;
    int y = 99;
    inplace_swap(&x, &y);
    printf("x = %d, y = %d\n", x, y);

    printf("===数组反转===：\n");
    int arr1[] = {1, 2, 3, 4, 5, 6};
    int arr2[] = {1, 2, 3, 4, 5};
    print_array(arr1, 6);
    print_array(arr2, 5);
    reverse_array(arr1, 6);
    reverse_array(arr2, 5);
    print_array(arr1, 6);
    print_array(arr2, 5);
    return EXIT_SUCCESS;
}