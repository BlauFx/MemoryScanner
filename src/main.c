#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>

int search(int pid, long int start, long int end, long int value) {

    printf("Searching for %ld in pid: %d\n", value, pid);

    //printf("%ld; %ld\n", start, end);

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
    long int AddressPtr = start;
    int found = 0;

    while (AddressPtr <= end)
    {
        data = ptrace(PTRACE_PEEKDATA, pid, AddressPtr, NULL);
        //printf("Current address: %ld\t", AddressPtr);

        printf("AA %ld\n", data);

        if (data == value) {
            found++;
            printf("ayooo\n");
            break; //TODO save all addresses
        }

        AddressPtr += sizeof(long);
    }

    ptrace(PTRACE_POKEDATA, pid, AddressPtr, 234234234);

    printf("BB %ld\n", data);
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    kill(pid, SIGCONT);

    if (found == 0) {
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    char pid[8];
    scanf("%7s", pid);
    int pidInt = strtol(pid, NULL, 10);

    char path[20];
    snprintf(path, sizeof(path), "/proc/%s/maps", pid);

    //printf("%s\n", path);

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

    int value = 1337; //TODO: read from stdin

    int ret = search(pidInt, startNum, endNum, value);

    if (ret == -1) {
        printf("ERROR: Could not find value %d\n", value);
    }
    else {
        printf("Value %d has been found!\n", value);
    }

    return 0;
}
