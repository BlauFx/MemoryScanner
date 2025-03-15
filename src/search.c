#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <fcntl.h>
#include "helper.h"

#define BUFFER_SIZE sizeof(int) * 1024

void search(int fd, long int addr, long int addrEnd, long int value, Node *list)
{
    Node *currentNode = list;

    while (currentNode->next != NULL) {
        currentNode = currentNode->next;
    }

    printf("searching start: 0x%lx, end: 0x%lx\n", addr, addrEnd);

    int *buffer = malloc(BUFFER_SIZE);

    long int read = 0;
    long int offset = addr;

    while (offset <= addrEnd)
    {
        read = pread(fd, buffer, BUFFER_SIZE, offset);

        //ERROR or EOL reached
        if (read <= 0)
            break;

        int amount = read / sizeof(int);

        for (int i = 0; i < amount; i++)
        {
            if (buffer[i] == value)
            {
                currentNode->type = NODE_TYPE_INT;

                currentNode->address = offset + (i * sizeof(int));
                currentNode->value = buffer[i];

                currentNode->next = malloc(sizeof(Node));
                currentNode = currentNode->next;
            }
        }

        offset += read;
    }
}
