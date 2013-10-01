#include <stdio.h>
#include <stdlib.h>

int getFileSize(char *fileName) {
    FILE *file;
    if (!(file = fopen(fileName, "rb"))) {
        printf("Error opening file: %s\n", fileName);
        return -1;
    }

    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    fclose(file);
    return size;
}

char *bufferize(char *fileName) {
    FILE *file;
    if (!(file = fopen(fileName, "rb"))) {
        printf("Error opening file: %s\n", fileName);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char * buffer = malloc(size);
    if (!fread(buffer, 1, size, file)) {
        printf("Error reading file (%s) into buffer.\n", fileName);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return buffer;
}

int bufferInsert(char *buffer, int buffSize, char *insert, int insertSize, int pos) {
    if (pos >= buffSize || pos < 0) {
        printf("Position outside (%i) of buffer size (%i).\n", pos, buffSize);
        return -1;
    }
    if (pos + insertSize > buffSize) {
        printf("Position (%i) + insert length (%i) exceeds buffer size (%i)",
                pos, insertSize, buffSize);
        return -1;
    }

    int it1 = pos;
    int it2 = 0;
    while ((it1 < pos + buffSize) && (it2 < insertSize)) {
        buffer[it1] = insert[it2];
        it1++;
        it2++;
    }

    return 0;
}

char *bufferExtract(char* buffer, int buffSize, int pos, int extractSize) {

    if (pos + extractSize >= buffSize || extractSize < 0 || pos < 0) {
        printf("Pos (%i) + extract size (%i) outside of buffer (%i).\n",
                pos, extractSize, buffSize);
        return NULL;
    }

    int it1 = pos;
    int it2 = 0;
    char *extract = malloc(extractSize);
    while ((it1 < buffSize) && (it2 < extractSize)) {
        extract[it2] = buffer[it1];
    }

    return extract;
}
