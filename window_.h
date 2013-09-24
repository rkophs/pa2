#include <stdlib.h>

//Element buffer to populate internally in the window:
struct subBuffer {
    int seq;                    //Seq # to corresponding buffer
    char *buffer;               //Buffer array (1024 bytes)
};

//Window object:
struct window {
    int size;                   //'N' elements in the window
    int cumSeq;                 //Sumulative ACK Seq
    int min;                    //Beginning element position of window
    int bufferSize;             //Size of each element buffer (1024 bytes)
    struct subBuffer *table;    //Array of 'N' buffers
};

/* Initialize the window
 *  Params:
 *      windowSize: Number of buffers of saved packets
 *      payloadSize: Size of each packet buffer
 */
struct window *windowInit(int windowSize, int payloadSize) {
    if (windowSize < 1) { //Only initialize with atleast 1 buffer
        return NULL;
    }
    
    //Build window struct:
    struct window *tmp;
    if ((tmp = (struct window *)malloc(sizeof (struct window))) == NULL) {
        return NULL; //Not enough mem
    }
    
    //Build underlying array of 'N' buffer structs in window
    if ((tmp->table = (struct subBuffer *)malloc(sizeof (struct subBuffer) * windowSize)) == NULL) {
        free(tmp);
        return NULL; //Not enough mem
    }
    
    //Set each buffer seq# to -1
    //Allocate memory for each unerlying buffer and set to 0
    int i;
    for (i = 0; i < windowSize; i++) {
        tmp->table[i].seq = -1;
        if ((tmp->table[i].buffer = malloc(payloadSize)) == NULL) {
            free(tmp->table);
            free(tmp);
            return NULL;
        }
        int j;
        for (j = 0; j < payloadSize; j++) {
            tmp->table[i].buffer[j] = 0;
        }
    }

    //Set window size, Cum ACK to -1, beginning window element Seq# to -1 and buffersize
    tmp->size = windowSize;
    tmp->cumSeq = -1;
    tmp->min = -1;
    tmp->bufferSize = payloadSize;
    
    return tmp;
}

/*Pull specific buffer out of window and set buffer to initial
 * Params:
 *      buffer: Window to pull buffer from
 *      seq: Seq# of buffer to pull
 */
char* pullSubBuffer(struct window* buffer, int seq) {
    int min = buffer->min;          //Beginning seq# of window
    int max = buffer->size + min;   //Ending seq# of window
    
    //If specified seq is out of range, or corresponding buffer is empry:
    if(seq < min || seq >= max || buffer->table[seq-min].seq == -1){
        return NULL;
    }
    
    //Malloc a return buffer to copy to.
    char *tmp;
    if((tmp = malloc(buffer->bufferSize)) == NULL){
        return NULL; //not enough memory to return
    }
    
    //Copy buffer to temp, and empty out (zero out):
    int i;
    int bufferSize = buffer->bufferSize;
    for (i = 0; i < bufferSize; i++) {
        tmp[i] = buffer->table[seq-min].buffer[i];
        buffer->table[seq-min].buffer[i] = 0;
    }
    buffer->table[seq-min].seq = -1;
    
    return tmp;
}

/* Pull beginning element out if present and shift all later elements left by 1
 * Params:
 *      buffer: Window to pull buffer from
 */
char *shiftWindow(struct window* buffer){
    
    //Pull out first element:
    char *tmp;
    if((tmp = pullSubBuffer(buffer, buffer->min)) == NULL){
        return NULL;
    } else { //If 1st element was present and popped, now 1st element is empty:
        int i;
        int size = buffer->size;
        //Move proceding elements left by 1 and set last to initial (zeroize):
        struct subBuffer last = buffer->table[0];
        for (i = 0; i < size - 1; i++) {
            buffer->table[i] = buffer->table[i + 1];
        }
        buffer->table[size - 1] = last;
        
        //Min moves up 1 because min element is now the next after the shift:
        buffer->min++;
        
        return tmp;
    }
}

/* Add a new buffer in the appropriate location corresponding to seq #
 *  Params:
 *      buffer: window to pull buffer from
 *      seq: Seq # of corresponding buffer
 *      payload: The buffer to copy
 *      payLoadSize: Size of the buffer to copy
 */
void insertSubBuffer(struct window* buffer, int seq, char* payload, int payLoadSize) {

    int size = buffer->size;
    
    //First valid buffer is received if min is still set to -1
    if(buffer->min == -1 && seq < size){
        buffer->min = 0;
    }
    int min = buffer->min;
    
    //Move payload into appropriate spot if within window:
    if (seq >= min && seq < (min + size)) {
        buffer->table[seq-min].seq = seq;
        int i;
        for (i = 0; i < payLoadSize; i++) {
            buffer->table[seq-min].buffer[i] = payload[i];
        }
        
        //Update cum Ack as appropriate:
        for(i = 0; i < size; i++){
            if(buffer->table[i].seq == (buffer->cumSeq + 1)){
                buffer->cumSeq++;
            }
        }
    }
}

/* For debugging purposes, print current window:
 *  Params:
 *      buffer: window buffer to print
 */
void printWindow(struct window *buffer){
    int i;
    int size = buffer->size;
    printf("[");
    for (i = 0; i < size; i++) {
        printf("(%i, %s) ", buffer->table[i].seq, buffer->table[i].buffer);
    }
    printf("]\n");
}

/* Destruct window memory:
 *  Params:
 *      buffer: window buffer to destruct
 */
void freeWindow(struct window* buffer) {
    int i;
    int size = buffer->size;
    for (i = 0; i < size; i++) {
        free(buffer->table[i].buffer);
    }
    free(buffer->table);
    free(buffer);
}