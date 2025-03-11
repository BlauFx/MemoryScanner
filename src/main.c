#include <signal.h>
#include <stdatomic.h>
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

    printf("zero\n");
    int i = 1;
    while (current != NULL) {
        printf("#%d address: %ld with value %ld\n", i, current->address, current->value);
        current = current->next;
        i++;
    }
}

void printNonZeroAddressNodes(Node *list)
{
    printf("non zero\n");
    int i = 1;
    while (list != NULL) {
        if (list->address != 0)
        {
            printf("#%d address: %ld\n", i, list->address);
        }
        list = list->next;
        i++;
    }
}

void removeZeroAddressNodes(Node *head) {

    while (head->address == 0)
    {
        head = head->next;
    }

    //TODO: finish the method
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
    while (current != NULL)
    {
        if (current->address != 0)
        {
            long data = ptrace(PTRACE_PEEKDATA, pid, current->address, NULL);
            printf("data of address: %ld with value: %ld\n", current->address, data);
            if (data == value) {
                //printf("yurr");
            }
            else {
                //Too lazy to delete the node, just set to null xd
                current->address = 0;
                printf("setting to 0\n");
            }
        }

        current = current->next;
    }

    ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

int search(int pid, long int start, long int end, long int value, Node *list)
{
    printf("Searching for %ld in pid: %d\n", value, pid);

    //printf("%ld; %ld\n", start, end);

    if (kill(pid, 0) != 0) {
        kill(pid, SIGCONT);
        sleep(10);
    }

    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == 0)
    {
        printf("PTRACE_ATTACH success\n");
    }
    else
    {
        printf("ERROR: ATTACHING!\n"); //TODO: Check internally for root permissions?
        return -1;
    }

    long data;
    long int addressPtr = start;
    int found = 0;

    Node *currentNode = list;

    while (addressPtr <= end)
    {
        data = ptrace(PTRACE_PEEKDATA, pid, (void*)addressPtr, NULL);
        //printf("Current address: %ld\t", AddressPtr);

        printf("AA %ld\n", data);

        if (data == value) {
            printf("XX %ld\n", data);
            found++;
            //printf("ayooo\n");

            currentNode->address = addressPtr;
            currentNode->value = data;
            currentNode->next = malloc(sizeof(Node));
            currentNode = currentNode->next;
        }

        addressPtr += sizeof(long);
    }

    printf("BB %ld\n", data);
    ptrace(PTRACE_DETACH, pid, NULL, NULL);

    if (found == 0) {
        return -1;
    }

    return 0;
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

    while (!feof(filePtr)) {
        if (fgets(buffer, sizeof(buffer), filePtr)) {
            if (strstr(buffer, "[heap]")) {
                printf("FOUND IT!\n");
                fclose(filePtr);
                break;
            }
        }
    }

    printf("Parsing: %s", buffer);

    char* token = strtok(buffer, " ");
    printf("%s\n", token);

    char *start = strtok(token, "-");
    printf("Starting address: \t%s\n", start);

    char *end = strtok(NULL, "-");
    printf("Ending address: \t%s\n", end);

    long int startNum = strtol(start, NULL, 16);
    long int endNum = strtol(end, NULL, 16);

    Node *list = malloc(sizeof(Node));

    int ret = search(pidInt, startNum, endNum, value, list);

    if (ret == -1) {
        printf("ERROR: Could not find value %d\n", value);
    }
    else {
        printf("Value %d has been found!\n", value);
    }

    printAllNodes(list);

    char input[32];
    while (1) {
        printf("reload? (y/n) \n");
        scanf("%1s", input);
        while (getchar() != '\n');

        if (input[0] == 'y')
        {
            filterChangedValues(pidInt, value, list);
            printAllNodes(list);
        }

        printf("Do you want to change a value? (y/n)\n");
        scanf("%1s", input);

        if (input[0] == 'y')
        {
            printf("Type the index of the address to change the value: \n");
            scanf("%4s", input);
            while (getchar() != '\n');

            int num = strtol(input, NULL, 16);
            num--; //Because user gets index start at #1

            printf("Enter the value: \n");
            scanf("%31s", input);
            while (getchar() != '\n');

            value = strtol(input, NULL, 10);

            Node *cur = list;
            while (num) {
                cur = cur->next;
                num--;
            }

            sleep(1);
            changeValue(pidInt, cur->address, value);
            cur->value = value;
            continue;
        }

        printf("Did the value change? (y/n) \n");
        scanf("%1s", input);
        while (getchar() != '\n');

        if (input[0] == 'y') {
            printf("Type the value to search for\n");

            char val[32];
            scanf("%7s", val);
            while (getchar() != '\n');
            value = strtol(val, NULL, 10);

            filterChangedValues(pidInt, value, list);
            printAllNodes(list);
        }
    }

    return 0;
}
