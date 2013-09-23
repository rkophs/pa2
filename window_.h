#include <stdlib.h>

typedef struct subBuffer {
    int seq;
    char *buffer;
} subBuffer;

typedef struct window {
    int size;
    int freeCount;
    int * seqStr;
    subBuffer *table;
} window;

//Initialize the window and window size, making each table element NULL

window *windowInit(int size) {
    if (size < 1) {
        return NULL;
    }
    window* tmp;
    printf("tmp init\n");
    if ((tmp = malloc(sizeof (window))) == NULL) {
        return NULL; //Not enough mem
    }
    printf("table init\n");
    if ((tmp->table = malloc(sizeof (subBuffer) * size)) == NULL) {
        return NULL; //Not enough mem
    }
    printf("seqStr init\n");
    if ((tmp->seqStr == (int *)malloc(sizeof(int)*size))) {
        return NULL; //Not enough mem
    }

    printf("sizeof %li\n", sizeof(tmp->seqStr[2]));
    printf("sizeof %li\n", sizeof(&(tmp->seqStr)));
    printf("sizeof %li\n", sizeof(*(tmp->seqStr)));
    printf("fill init\n");
    int i;
    for (i = 0; i < size; i++) {
        printf("seq");
        tmp->table[i].seq = -1;
        printf("str");
        tmp->seqStr[i] = -1;
    }
    printf("size init\n");
    tmp->size = size;
    tmp->freeCount = size;

    return tmp;
}

//Get the assigned buffer for the specified sequence

char* getSubBuffer(window* buffer, int seq) {
    int i;
    int size = buffer->size;
    for (i = 0; i < size; i++) {
        if (buffer->table[i].seq == -1 && buffer->table[i].seq == seq) {
            return buffer->table[i].buffer;
        }
    }
    return NULL;
}

void removeSubBuffer(window* buffer, int seq) {
    int i;
    int size = buffer->size;
    for (i = 0; i < size; i++) {
        if (buffer->table[i].seq == seq) {
            free(buffer->table[i].buffer);
            buffer->table[i].buffer = NULL;
            buffer->table[i].seq = -1;
            buffer->freeCount++;
            int j;
            for (j = 0; i < size; j++) {
                if (buffer->seqStr[j] == seq) {
                    buffer->seqStr[j] = -1;
                }
            }
        }
    }
}

int seqExists(window* buffer, int seq) {
    int i;
    int size = buffer->size;
    for (i = 0; i < size; i++) {
        if (buffer->table[i].seq == seq) {
            return 1;
        }
    }
    return 0;
}

int insertSubBuffer(window* buffer, int seq, char* payload, int payLoadSize) {
    if (seqExists(buffer, seq)) {
        return 0;
    }
    int i;
    int size = buffer->size;
    for (i = 0; i < size; i++) {
        subBuffer *tmp = &(buffer->table[i]);
        if (tmp->seq == -1) {
            if ((buffer->table[i].buffer = malloc(payLoadSize)) == NULL) {
                return 0; //not enough mem;
            }
            tmp->seq = seq;
            memcpy(tmp->buffer, payload, payLoadSize);
            buffer->freeCount--;
        }
    }
}

void sortSeq(window *buffer) { //Selection sort( inefficient but easy to code)
    int i, j;
    int min = -1;
    int size = buffer->size;
    for (i = 0; i < size; i++) {
        for (j = i; j < size; j++) {
            if (buffer->seqStr[i] < min) {
                int tmp = buffer->seqStr[i];
                buffer->seqStr[i] = buffer->seqStr[j];
                buffer->seqStr[j] = tmp;
            }
        }
    }
}

int getCumSeq(window *buffer) {
    sortSeq(buffer);
    int min = -1;
    int i;
    int size = buffer->size;
    for (i = 0; i < size; i++) {
        if (buffer->seqStr[i] != -1 && min == -1) {
            min = buffer->seqStr[i];
        } else if (buffer->seqStr[i] != -1 && buffer->seqStr[i] < min) {
            min = buffer->seqStr[i];
        }
    }

    //Assuming a sorted seqStr:
    for (i = 0; i < size; i++) {
        if (buffer->seqStr[i] != -1 && buffer->seqStr[i] == min + 1) {
            min = buffer->seqStr[i];
        }
    }
    
    return min;
}

void freeWindow(window* buffer) {
    int i;
    int size = buffer->size;
    for (i = 0; i < size; i++) {
        free(buffer->table[i].buffer);
    }
    free(buffer->table);
    free(buffer->seqStr);
    free(buffer);
}