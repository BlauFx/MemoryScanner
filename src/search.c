#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <fcntl.h>
#include "helper.h"

// Reading different types
int read_int(int fd, unsigned long address) {
    lseek(fd, address, SEEK_SET);
    int value;
    read(fd, &value, sizeof(int));
    return value;
}

long read_long(int fd, unsigned long address) {
    lseek(fd, address, SEEK_SET);
    long value;
    read(fd, &value, sizeof(long));
    return value;
}

int search(int pid, long int start, long int end, long int value, Node *list)
{
    long data;
    long int address = start;

    Node *currentNode = list;

    while (currentNode->next != NULL) {
        currentNode = currentNode->next;
    }

    char mem_path[64];
    snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pid);
    int fd = open(mem_path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open /proc/<pid>/mem");
        return -1;
    }

    printf("searching long, start: %ld, end: %ld\n", address, end);

    while (address <= end) {

        int readInt = read_int(fd, address);
        long readLong = read_long(fd, address);

        if (readInt == value) {
            printf("Found integer, addr: %ld\n", address);

            currentNode->address = address;
            currentNode->value = data;
            currentNode->next = malloc(sizeof(Node));
            currentNode = currentNode->next;
        }
        else if (readLong == value)
        {
            printf("Found long, addr: %ld\n", address);

            currentNode->address = address;
            currentNode->value = data;
            currentNode->next = malloc(sizeof(Node));
            currentNode = currentNode->next;
        }

        address += sizeof(long);
    }

/*
    while (addressPtr <= end)
    {
        data = ptrace(PTRACE_PEEKDATA, pid, (void*)addressPtr, NULL);

        if (data == value) {

            // printf("XX %ld\n", data);

            currentNode->address = addressPtr;
            currentNode->value = data;
            currentNode->next = malloc(sizeof(Node));
            currentNode = currentNode->next;
        }

        unsigned char *bytes = (unsigned char*)&data; // Cast to byte array
        for (int i = 0; i < sizeof(long); i++) {
            if  (bytes[i] == value)
            {
                printf("x YY %d %ld %ld\n", i, addressPtr, value);
                currentNode->address = addressPtr;
                currentNode->value = bytes[i];

                currentNode->isByte = 1;
                currentNode->bytePos = i;

                currentNode->next = malloc(sizeof(Node));
                currentNode = currentNode->next;
            }
        }

        addressPtr += sizeof(long);
    }*/

    if (currentNode->next == NULL) {
        return -1;
    }

    return 0;
}
