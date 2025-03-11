#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//This is an example program to modifiy the values from outside

int main(int argc, char* argv[])
{
    int *x = malloc(sizeof(int));
    *x = 1337;

    int *y = malloc(sizeof(int));
    *y = 123456789;

    while (1) {
        printf("PID: %d; value x is %d, value of y is: %d\n", getpid(), *x, *y);
        sleep(10);
    }

    return 0;
}
