#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <fcntl.h>
#include "helper.h"

int read_byte(int fd, unsigned long address) {
    int value;
    pread(fd, &value, sizeof(char), address);
    return value;
}

int read_int(int fd, unsigned long address) {
    int value;
    pread(fd, &value, sizeof(int), address);
    return value;
}

long read_long(int fd, unsigned long address) {
    long value;
    pread(fd, &value, sizeof(long), address);
    return value;
}

void search(int fd, long int start, long int end, long int value, Node *list)
{
    long data;
    long int address = start;

    Node *currentNode = list;

    while (currentNode->next != NULL) {
        currentNode = currentNode->next;
    }

    printf("searching start: 0x%lx, end: 0x%lx\n", address, end);

    while (address <= end)
    {
        // long int data = read_byte(fd, address);
        // if (data == value) {
        //     printf("Found byte \taddr: 0x%lx with value: %ld\n", address, data);
        //
        //     currentNode->address = address;
        //     currentNode->value = data;
        //
        //     currentNode->type = NODE_TYPE_BYTE;
        //
        //     currentNode->next = malloc(sizeof(Node));
        //     currentNode = currentNode->next;
        // }

        data = read_int(fd, address);

        if (data == value) {
            // printf("Found integer \taddr: 0x%lx with value: %ld\n", address, data);
            currentNode->type = NODE_TYPE_INT;

            currentNode->address = address;
            currentNode->value = data;

            currentNode->next = malloc(sizeof(Node));
            currentNode = currentNode->next;
        }

        data = read_long(fd, address);

        if (data == value) {
            // printf("Found long \taddr: 0x%lx with value: %ld\n", address, data);
            currentNode->type = NODE_TYPE_LONG;

            currentNode->address = address;
            currentNode->value = data;

            currentNode->next = malloc(sizeof(Node));
            currentNode = currentNode->next;
        }

        address += sizeof(int);
    }
}
