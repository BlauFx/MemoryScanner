typedef struct Node {
    long int address;
    long int value;
    int isByte;
    int bytePos;
    struct Node *next;
} Node;

typedef struct BufLine {
    char buffer[32];
    long int start;
    long int end;
    struct BufLine *next;
} BufLine;

int search(int pid, long int start, long int end, long int value, Node *list);
