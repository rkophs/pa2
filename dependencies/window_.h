/* Author: Ryan Kophs
 * Date: 24 September 2013
 * Notes: This is a storage mechanism for a window of payload buffers in a 
 *      Go Back N protocol. It is used for either a receiver or sender window
 *      and some specific functions (as indicated) will only be used by the
 *      EITHER sender OR the receiver. If we were using C++, I could have 
 *      abstracted and sub-classed all this, but unfortunately that is not the
 *      case. Comments will indicate where appropriate.
 */
#include <stdlib.h>

//Element buffer to populate internally in the window:

struct subBuffer {
    int seq; //Seq # to corresponding buffer
    char *buffer; //Buffer array (1024 bytes)
    int buffSize;
};

//Window object:

struct window {
    int rws;                    //# of empty buffers (Only used by Recv Window)
    int size;                   //'N' elements in the window
    int cumSeq;                 //Cumulative ACK Seq (Only used by Recv Window)
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
    if ((tmp = (struct window *) malloc(sizeof (struct window))) == NULL) {
        return NULL; //Not enough mem
    }

    //Build underlying array of 'N' buffer structs in window
    if ((tmp->table = (struct subBuffer *) malloc(sizeof (struct subBuffer) * windowSize)) == NULL) {
        free(tmp);
        return NULL; //Not enough mem
    }

    //Set each buffer seq# to -1
    //Allocate memory for each unerlying buffer and set to 0
    int i;
    for (i = 0; i < windowSize; i++) {
        tmp->table[i].seq = -1;
        tmp->table[i].buffSize = 0;
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
    tmp->rws = windowSize;
    tmp->size = windowSize;
    tmp->cumSeq = -1;
    tmp->min = -1;
    tmp->bufferSize = payloadSize;

    return tmp;
}

/*Pull specific buffer out of window
 * This should never be called externally. Again, in C++ this would have been
 *  marked as protected;
 * Params:
 *      buffer: Window to pull buffer from
 *      seq: Seq# of buffer to pull
 *      reset: set specificed buffer to 0 if true
 */
int pullSubBuffer(struct window* buffer, int seq, int reset, char *tmp) {
    int min = buffer->min; //Beginning seq# of window
    int max = buffer->size + min; //Ending seq# of window

    //If specified seq is out of range, or corresponding buffer is empty:
    if (seq < min || seq >= max || buffer->table[seq - min].seq == -1) {
        return -1;
    }

    //Copy buffer to temp, and empty out (zero out):
    int i;
    int bufferSize = buffer->bufferSize;
    for (i = 0; i < bufferSize; i++) {
        tmp[i] = buffer->table[seq - min].buffer[i];
        if(reset){
            buffer->table[seq - min].buffer[i] = 0;
        }
    }
    if(reset){
        buffer->table[seq - min].seq = -1;
        buffer->table[seq - min].buffSize = 0;
    }
    return 0;
}

/*Pull specific buffer out of window
 * This should only be used by sender.
 * Params:
 *      buffer: Window to pull buffer from
 *      seq: Seq# of buffer to pull
 */
int sendPullSubBuffer(struct window* buffer, int seq, char *tmp){
    return pullSubBuffer(buffer, seq, 0, tmp);
}

/* Pull beginning element out if present and shift all later elements left by 1
 * This is only used by the receiver
 * Params:
 *      buffer: Window to pull buffer from
 */
int recvShiftWindow(struct window* buffer, char *tmp) {

    int buffSize = buffer->table[0].buffSize;
    if (pullSubBuffer(buffer, buffer->min, 1, tmp) < 0) {
        return -1;
    } else { //If 1st element was present and popped, now 1st element is empty:
        int i;
        int size = buffer->size;
        //Move proceeding elements left by 1 and set last to initial (zeroize):
        struct subBuffer last = buffer->table[0];
        for (i = 0; i < size - 1; i++) {
            buffer->table[i] = buffer->table[i + 1];
        }
        buffer->table[size - 1] = last;
        //Min moves up 1 because min element is now the next after the shift:
        buffer->min++;
        
        return buffSize;
    }
}

/* Pull beginning element out if present and shift all later elements left by 1
 * This is only used by the sender
 * Params:
 *      buffer: Window to pull buffer from
 */
void sendShiftWindow(struct window *buffer, int seq, char *newBuff, int newBuffSize) {
    int i, j;
    int size = buffer->size;
    int len = buffer->bufferSize;
    
    //Shift everything 1 to the left:
    for (i = 1; i < size; i++) {
        for (j = 0; j < len; j++) {
            buffer->table[i - 1].buffer[j] = buffer->table[i].buffer[j];
        }
        buffer->table[i - 1].seq = buffer->table[i].seq;
        buffer->table[i - 1].buffSize = buffer->table[i].buffSize;
    }
    
    //Set last buffer to 0:
    for (j = 0; j < len; j++) {
        buffer->table[size - 1].buffer[j] = 0;
    }
    buffer->table[size - 1].buffSize = 0;
    buffer->table[size - 1].seq = -1;
    
    //Add new buffer in final position:
    if (newBuff != NULL) {
        for (j = 0; j < newBuffSize; j++) {
            buffer->table[size-1].buffer[j] = newBuff[j];
        }
        buffer->table[size - 1].seq = seq;
        buffer->table[size - 1].buffSize = newBuffSize;
    }
    buffer->cumSeq++;
    buffer->min++;
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
    if (buffer->min == -1 && seq < size) {
        buffer->min = 0;
    }
    int min = buffer->min;

    //Move payload into appropriate spot if within window:
    if (seq >= min && seq < (min + size) && buffer->table[seq - min].seq != seq) {
        buffer->table[seq - min].seq = seq;
        buffer->table[seq - min].buffSize = payLoadSize;
        int i;
        for (i = 0; i < payLoadSize; i++) {
            buffer->table[seq - min].buffer[i] = payload[i];
        }

        //Update cum Ack as appropriate:
        for (i = 0; i < size; i++) {
            if (buffer->table[i].seq == (buffer->cumSeq + 1)) {
                buffer->cumSeq++;
            }
        }
    }
}

/* For debugging purposes, print current window:
 *  Params:
 *      buffer: window buffer to print
 */
void printWindow(struct window *buffer) {
    int i;
    int size = buffer->size;
    printf("[");
    for (i = 0; i < size; i++) {
        printf("(%i, %i) ", buffer->table[i].seq, buffer->table[i].buffSize);
    }
    printf("] min: %i, cumSeq: %i, RWS: %i\n", buffer->min, buffer->cumSeq, buffer->rws);
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