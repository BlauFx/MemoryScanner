#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <fcntl.h>
#include "helper.h"

int read_int(int fd, unsigned long address) {
    int value;
    pread(fd, &value, sizeof(int), address);
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
        data = read_int(fd, address);

        if (data == value) {
            // printf("Found integer \taddr: 0x%lx with value: %ld\n", address, data);
            currentNode->type = NODE_TYPE_INT;

            currentNode->address = address;
            currentNode->value = data;

            currentNode->next = malloc(sizeof(Node));
            currentNode = currentNode->next;
        }

        address += sizeof(int);
    }
}
