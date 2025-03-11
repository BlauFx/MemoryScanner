#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//This is an example program to modifiy the values from outside

int main(int argc, char* argv[])
{
    int *x = malloc(sizeof(int));
    *x = 123456789;

    int *y = malloc(sizeof(int));
    *y = 123456789;

    int *z = malloc(sizeof(int));
    *z = 123456789;

    while (1) {
        printf("PID: %d; value x is %d, value of y is: %d, value of z is: %d\n", getpid(), *x, *y, *z);
        sleep(10);
        *x = ++(*x);
        sleep(10);
        *z = ++(*z);
    }

    return 0;
}
