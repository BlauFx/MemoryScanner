typedef enum {
    NODE_TYPE_BYTE,
    NODE_TYPE_INT,
    NODE_TYPE_LONG
} NodeType;

typedef struct Node {
    long int address;
    long int value;
    NodeType type;
    int pos;
    struct Node *next;
} Node;

typedef struct BufLine {
    char buffer[32];
    long int start;
    long int end;
    struct BufLine *next;
} BufLine;

void search(int pid, long int start, long int end, long int value, Node *list);

int read_byte(int fd, unsigned long address);

int read_int(int fd, unsigned long address);

long read_long(int fd, unsigned long address);
