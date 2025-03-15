#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include "helper.h"
#include <fcntl.h>

void changeValue(int pid, long int addr, long int value, int size) {

    char mem_path[64];
    snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pid);
    int fd = open(mem_path, O_RDWR);
    if (fd == -1) {
        printf("Failed to open /proc/%d/mem", pid);
        return;
    }

    printf("Changing addr %ld with value %ld and size: %d\n", addr, value, size);

    lseek(fd, addr, SEEK_SET);
    write(fd, &value, size);

    close(fd);
}

void printAllNodes(Node *list)
{
    Node *current = list;

    int i = 1;
    while (current != NULL) {
        if (current->value != -1 && current->address != 0) {
            char* type = "";

            switch (current->type) {
                case NODE_TYPE_BYTE:
                    type = "byte";
                    break;

                case NODE_TYPE_INT:
                    type = "int";
                    break;

                case NODE_TYPE_LONG:
                    type = "long";
                    break;
                default:
                    type= "unkown";
            }

            printf("#%d address (%s): \t0x%lx with value: \t%ld\n", i, type, current->address, current->value);
        }

        current = current->next;
        i++;
    }
}

void filterChangedValues(int pidInt, long int value, Node *list, int active)
{
    char mem_path[64];
    snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pidInt);
    int fd = open(mem_path, O_RDONLY);
    if (fd == -1) {
        printf("Failed to open /proc/%d/mem", pidInt);
    }

    Node *current = list;
    unsigned char buffer[sizeof(long)];

    while (current != NULL)
    {
        if (current->address != 0 && current->value != -1)
        {
            long int ret = 0;

            if (current->type == NODE_TYPE_BYTE)
                ret = -1; //read_byte(fd, current->address);
            else if (current->type == NODE_TYPE_INT) {
                pread(fd, &ret, sizeof(int), current->address);
            }
            else if (current->type == NODE_TYPE_LONG)
                ret = -1; //read_long(fd, current->address);

            if  (ret != value) {
                current->value = active == 0 ? ret : -1;
            }
            else {
                current->value = ret;
            }
        }

        current = current->next;
    }

    close(fd);

    printf("Filtering done\n");
}

int main(int argc, char* argv[])
{
    printf("Type the pid of the program\n");

    char pid[8];
    scanf("%7s", pid);
    int pidInt = strtol(pid, NULL, 10);

    printf("Type the value to search for\n");

    char val[32];
    scanf("%31s", val);
    while (getchar() != '\n');
    int value = strtol(val, NULL, 10);

    char path[20];
    snprintf(path, sizeof(path), "/proc/%s/maps", pid);

    printf("%s\n", path);

    FILE *filePtr = fopen(path, "r");

    if (!filePtr)
    {
        printf("FilePtr is null!\n");
        return 0;
    }

    char buffer[512];

    BufLine *rwPages = malloc(sizeof(BufLine));
    BufLine *ptr = rwPages;

    while (!feof(filePtr)) {
        if (fgets(buffer, sizeof(buffer), filePtr)) {

            char addr[32], perms[8];
            int fields = sscanf(buffer, "%31s %7s", addr, perms);

            // Check if the entry has 5 fields (meaning no file mapping) and is "rw-p"
            if (fields == 2 && strcmp(perms, "rw-p") == 0) {
                // printf("%s\n", addr);

                char *start = strtok(addr, "-");
                char *end = strtok(NULL, "-");

                long int startInt = strtol(start, NULL, 16);
                long int endInt = strtol(end, NULL, 16);

                strcpy(ptr->buffer, addr);
                ptr->start = startInt;
                ptr->end = endInt;

                ptr->next = malloc(sizeof(BufLine));
                ptr = ptr->next;
            }

            if (strstr(buffer, "[heap]")) {
                printf("FOUND [HEAP]\n");
                fclose(filePtr);
                break;
            }
        }
    }

    Node *list = malloc(sizeof(Node));
    ptr = rwPages;

    printf("OKAY: going to start searching\n");

    char mem_path[64];
    snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pidInt);
    int fd = open(mem_path, O_RDONLY);
    if (fd == -1) {
        printf("Failed to open /proc/%d/mem", pidInt);
    }

    while (ptr->next != NULL) {
        long int startNum = ptr->start;
        long int endNum = ptr->end;

        printf("Starting: \t0x%lx\n", startNum);
        printf("Ending: \t0x%lx\n", endNum);

        search(fd, startNum, endNum, value, list);
        ptr = ptr->next;
    }

    close(fd);

    while (rwPages->next != NULL)
    {
        ptr = rwPages;
        rwPages = rwPages->next;
        free(ptr);
    }

    ptr = NULL;

    printf("--------------------------------------\n");
    filterChangedValues(pidInt, value, list, 0);
    printAllNodes(list);

    char inputBuf[32];
    while (1) {
        printf("1 to refresh, 2 refresh with new value, 3: to change a value\n");

        scanf("%31s", inputBuf);
        while (getchar() != '\n');

        if (inputBuf[0] == '1')
        {
            printf("Refreshing...\n");
            filterChangedValues(pidInt, value, list, 0);

            sleep(1);
            printAllNodes(list);
        }
        else if (inputBuf[0] == '2')
        {
            printf("Type the new value in\n");

            char val[32];
            scanf("%31s", val);
            while (getchar() != '\n');
            int newValue = strtol(val, NULL, 10);

            printf("Refreshing...\n");
            filterChangedValues(pidInt, newValue, list, 1);
            value = newValue;

            sleep(1);
            printAllNodes(list);
        }
        else if (inputBuf[0] == '3')
        {
            printf("Type the # of the address you want to change.\n");

            scanf("%31s", inputBuf);
            while (getchar() != '\n');

            int index = strtol(inputBuf, NULL, 10);
            index--;

            Node *ptrList = list;

            while (index) {
                ptrList = ptrList->next;
                index--;
            }

            printf("Type the value you want to insert\n");
            scanf("%31s", inputBuf);
            while (getchar() != '\n');

            int valueVar = strtol(inputBuf, NULL, 10);

            int size = ptrList->type == NODE_TYPE_BYTE ? 1 : (ptrList->type == NODE_TYPE_INT ? 4 : 8);
            changeValue(pidInt, ptrList->address, valueVar, size);
        }
    }

    return 0;
}
