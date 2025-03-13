#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <fcntl.h>
#include "helper.h"

void search(int fd, long int addr, long int addrEnd, long int value, Node *list)
{
    int data;

    Node *currentNode = list;

    while (currentNode->next != NULL) {
        currentNode = currentNode->next;
    }

    printf("searching start: 0x%lx, end: 0x%lx\n", addr, addrEnd);

    while (addr <= addrEnd)
    {
        data = -1;
        pread(fd, &data, sizeof(int), addr);

        if (__builtin_expect(!!(data == value), 0)) {
            currentNode->type = NODE_TYPE_INT;

            currentNode->address = addr;
            currentNode->value = data;

            currentNode->next = malloc(sizeof(Node));
            currentNode = currentNode->next;
        }

        addr += sizeof(int);
    }
}
