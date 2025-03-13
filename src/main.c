#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include "helper.h"

void changeValue(int pid, long int address, long int value)
{
    printf("Changing address: %ld with value %ld\n", address, value);

    ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    sleep(2);

    ptrace(PTRACE_POKEDATA, pid, address, value);
    sleep(2);

    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    sleep(2);
    //kill(pid, SIGCONT);
}

void printAllNodes(Node *list)
{
    Node *current = list;

    int i = 1;
    while (current != NULL) {
        if (current->value == -1 || current->address == 0) {
        }
        else
        {
            if (current->isByte == 1)
            {
                unsigned char *bytes = (unsigned char*)&current->value; // Cast to byte array
                for (int j = 0; j < sizeof(long); j++) {
                    if (bytes[j] != 0) {
                        // printf("#%d data of address byte %i: %ld with value: %d\n", i, j, current->address, bytes[j]);
                    }
                }
            }
            else
            {
                printf("#%d address: %ld with value (long): %ld\n", i, current->address, current->value);
            }
        }

        current = current->next;
        i++;
    }
}

void filterChangedValues(int pid, long int value, Node *list)
{
    printf("pid: %d\n", pid);

    if (kill(pid, 0) != 0) {
        kill(pid, SIGCONT);
        sleep(10);
    }

    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == 0)
    {
        printf("PTRACE_ATTACH 2 success\n");
    }
    else
    {
        printf("ERROR: ATTACHING 2!\n"); //TODO: Check internally for root permissions?
    }

    sleep(2);

    Node *current = list;
    Node *prev = NULL;

    while (current != NULL)
    {
        if (current->address != 0)
        {
            long data = ptrace(PTRACE_PEEKDATA, pid, current->address, NULL);

            if (current->isByte > 0)
            {
                int tmp = -1;

                unsigned char *bytes = (unsigned char*)&data; // Cast to byte array
                for (int i = 0; i < sizeof(long); i++) {
                    // printf("data of address bytes: %ld with value: %d\n", current->address, bytes[i]);
                    if  (bytes[i] == value) {
                        tmp = i;
                        current->value = value;
                    }
                }

                if (tmp == -1)
                {
                    current->value = -1;
                }
            }
            else
            {
                printf("data of address: %ld with value: %ld\n", current->address, data);
                current->value = data;

                if (data != value)
                {
                    current->value = -1;
                }
            }
        }
        else
        {

        }

        prev = current;
        current = current->next;
    }

    ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

void attachORdetach(int pid, int attach) {

    if (kill(pid, 0) != 0) {
        kill(pid, SIGCONT);
        sleep(10);
    }

    if (attach) {
        if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == 0)
        {
            printf("PTRACE_ATTACH success\n");
        }
        else
        {
            printf("ERROR: ATTACHING!\n"); //TODO: Check internally for root permissions?
        }
    }
    else {
        if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == 0)
        {
            printf("PTRACE_DETACH success\n");
        }
        else
        {
            printf("ERROR: DETACHING!\n"); //TODO: Check internally for root permissions?
        }
    }

}

int search(int pid, long int start, long int end, long int value, Node *list)
{
    long data;
    long int addressPtr = start;

    Node *currentNode = list;

    while (currentNode->next != NULL) {
        currentNode = currentNode->next;
    }

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
    }

    if (currentNode->next == NULL) {
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc == 1) {
        printf("Enter as an argument (for example) the .data section virtual address\n");
        printf("To ge the VMA: \"objdump -h /your/binaryfile | grep \" .data \" | awk '{print $4}\n");
        exit(0);
    }

    char *vma = argv[1];
    long int VMAInt = strtol(vma, NULL, 16);

    printf("VMA: %ld \n", VMAInt);


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

    int i = 0;

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

                if (startInt >= VMAInt)
                {
                    printf("startINT: %ld\n", startInt);
                    strcpy(ptr->buffer, addr);
                    ptr->start = startInt;
                    ptr->end = endInt;

                    ptr->next = malloc(sizeof(BufLine));
                    ptr = ptr->next;
                    i++;
                }
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

    while (ptr->next != NULL)
    {
        printf("a Starting: \t%ld\n", ptr->start);
        printf("a Ending: \t%ld\n", ptr->end);

        if (ptr->next->next->next->next == NULL) {
            ptr->next->next->next = NULL;
            break;
        }

        ptr = ptr->next;
    }

    // return 0;

    attachORdetach(pidInt, 1);

    printf("OKAY: going to start searching\n");

    while (i && ptr->next != NULL) {
        long int startNum = ptr->start;
        long int endNum = 0x14147f842; //ptr->end;

        printf("Starting: \t%ld\n", startNum);
        printf("Ending: \t%ld\n", endNum);

        int ret = search(pidInt, startNum, endNum, value, list);
        if (ret == -1)
        {
            printf("ERROR: Could not find value %d\n", value);
        }
        else
        {
            printf("Value %d has been found!\n", value);
        }

        break;

        i--;
        ptr = ptr->next;
    }

    attachORdetach(pidInt, 0);
    printAllNodes(list);

    printf("All address in the heap with the value %d have been found.\n", value);

    char input[32];
    while (1) {
        printf("Did the value change? (y/n) \n");
        scanf("%1s", input);
        while (getchar() != '\n');

        if (input[0] == 'y') {
            printf("Type the value to search for\n");

            char val[32];
            scanf("%31s", val);
            while (getchar() != '\n');
            value = strtol(val, NULL, 10);

            filterChangedValues(pidInt, value, list);
            printf("TEST!!!!\n");
            printAllNodes(list);
        }

        printf("Do you want to change a value? (y/n)\n");
        scanf("%1s", input);

        if (input[0] == 'y')
        {
            printf("Type the index of the address to change the value: \n");
            scanf("%4s", input);
            while (getchar() != '\n');

            int num = strtol(input, NULL, 10);
            num--; //Because user gets index start at #1

            printf("Enter the value: \n");
            scanf("%31s", input);
            while (getchar() != '\n');

            value = strtol(input, NULL, 10);

            printf("Value entered: %d\n", value);

            Node *cur = list;

            while (num) {
                printf("Num is: %d with cur addrress: %ld\n", num, cur->address);
                cur = cur->next;
                num--;
            }

            printf("Num is: %d with cur addrress: %ld\n", num, cur->address);

            sleep(1);
            changeValue(pidInt, cur->address, value);
            cur->value = value;
        }
    }

    return 0;
}
