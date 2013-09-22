#include <stdio.h>
#include <stdlib.h>

int fileSize(char *fileName){
    FILE *file;
    if(!(file = fopen(fileName, "rb"))){
        printf("Error opening file: %s\n", fileName);
        return -1;
    }
    
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    fclose(file);
    return size;
}

void *bufferize(char *fileName) {
    FILE *file;
    if(!(file = fopen(fileName, "rb"))){
        printf("Error opening file: %s\n", fileName);
        return NULL;
    }
    
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    
    void * buffer = malloc(size);
    if(!fread(buffer, 1, size, file)){
        printf("Error reading file (%s) into buffer.\n", fileName);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    return buffer;
}

int writeBuffer(char *fileName, char *buffer, int size){
    FILE *file;
    if(!(file = fopen(fileName, "wb"))){
        printf("Error creating file: %s\n", fileName);
        return -1;
    }
    
    if(!fwrite(buffer, 1, sizeof(buffer), file)){
        printf("Error write to file: %s\n", fileName);
        fclose(file);
        return -1;
    }
    
    fclose(file);
    return 0;
}

int bufferInsert(char *buffer, char *insert, int pos){
    int buffSize = sizeof(buffer);
    if(pos >= buffSize || pos < 0){
        printf("Position outside (%i) of buffer size (%i).\n", pos, buffSize);
        return -1;
    }
    int insertSize = sizeof(insert);
    if(pos + insertSize > buffSize){
        printf("Position (%i) + insert length (%i) exceeds buffer size (%i)",
                pos, insertSize, buffSize);
        return -1;
    }
    
    int it1 = pos;
    int it2 = 0;
    while((it1 < pos + buffSize) && (it2 < insertSize)){
        buffer[it1] = insert[it2];
        it1++;
        it2++;
    }
    
    return 0;
}
