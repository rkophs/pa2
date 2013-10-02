/* GBNclient.c */
/* This is a sample UDP client/sender using "sendto_.h" to simulate dropped packets.  */
/* This code will not work unless modified. */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>   /* memset() */
#include <sys/time.h> /* select() */
#include <signal.h>
#include <unistd.h>
#include "dependencies/sendto_.h"
#include "dependencies/socket_.h"
#include "dependencies/window_.h"
#include "dependencies/logger_.h"

#define MAXBUFFSIZE 1024
#define SWS 9

int main(int argc, char *argv[]) {

    /* -----------------MAIN Initial Setup Routine. ------------------------------------------------------------------*/
    /* Open logger*/
    FILE *log;
    if(!(log = fopen("clientLog.txt", "wb"))){
        printf("Error instantiating log file\n");
        return;
    }
    
    /* check command line args. */
    if (argc < 7) {
        printf("usage : %s <server_ip> <server_port> <error rate> <random seed> <send_file> <send_log> \n", argv[0]);
        fclose(log);
        return 1;
    }

    init_net_lib(atof(argv[3]), atoi(argv[4]));
    LOGGER(log, "error rate : %f\n", atof(argv[3]));

    /* socket creation */
    int sd;
    if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        LOGGER(log, "%s: cannot create socket \n", argv[0]);
        fclose(log);
        return 1;
    }

    /* get server IP address (input must be IP address, not DNS name) */
    struct sockaddr_in server = buildAddr(argv[1], argv[2]);
    unsigned int serverLen = sizeof (server);
    LOGGER(log, "%s: sending file '%s' to '%s:%s' \n", argv[0], argv[5], argv[1], argv[2]);

    /* set timeout for recvfrom function*/
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 50000; //50 ms
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv)) < 0) {
        LOGGER(log, "Error setting timeout\n");
        fclose(log);
        return 1;
    }

    /* Open input file*/
    FILE *file;
    if (!(file = fopen(argv[5], "rb"))) {
        LOGGER(log, "Error opening file: %s\n", argv[5]);
        fclose(log);
        return 1;
    }

    /* Get file size*/
    fseek(file, 0L, SEEK_END);
    int fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    /* Copy file into buffer:*/
    char * fileBuffer;
    if ((fileBuffer = malloc(fileSize)) == NULL) {
        LOGGER(log, "Not enough memory to bufferize file.\n");
    }
    if (!fread(fileBuffer, 1, fileSize, file)) {
        LOGGER(log, "Error reading file (%s) into buffer.\n", argv[5]);
        free(fileBuffer);
        fclose(file);
        fclose(log);
        return 1;
    }
    fclose(file);

    /* Find the last seq.# for the file and its buffer size*/
    int lastSize = fileSize % MAXBUFFSIZE;
    int lastSeq = fileSize / MAXBUFFSIZE;

    char header[32];
    char payload[MAXBUFFSIZE];
    char buffer[sizeof (payload) + sizeof (header)];
    char ack[32];

    //Initialize window and populate with data;
    struct window *sWin = windowInit(SWS, MAXBUFFSIZE);
    int i;
    for (i = 0; i < SWS && i <= lastSeq; i++) {
        if (i <= lastSeq) {
            int size = (i == lastSeq) ? lastSize : MAXBUFFSIZE;
            insertSubBuffer(sWin, i, fileBuffer + i*MAXBUFFSIZE, size);
        } else {
            insertSubBuffer(sWin, i, NULL, 0);
        }
    }

    /* -----------------MAIN Go Back N Protocol Looping. -------------------------------------------------------*/
    int LAR = -1;
    int LFS0 = -1;
    int LFS1 = -1;
    while (LAR < (lastSeq + 1)) {

        //Slide window until beyond cumSeq:
        while (sWin->min <= LAR && (sWin->min + SWS) <= (lastSeq + 1)) {
            if (sWin->min + SWS == (lastSeq + 1)) {
                sendShiftWindow(sWin, sWin->min + SWS, NULL, 0);
            } else {
                int pos = (sWin->min + SWS) * MAXBUFFSIZE;
                int size = (sWin->min + SWS == lastSeq) ? lastSize : MAXBUFFSIZE;
                sendShiftWindow(sWin, sWin->min + SWS, fileBuffer + pos, size);
            }
        }
 
        //Send each packet in window and only those packets within the fileSize + 1:
        for (i = 0; (i < SWS) && ((sWin->min + i) <= (lastSeq + 1)); i++) {
            buildHeader(header, sizeof (header), "hdr", 3, fileSize, 15, sWin->min + i, 28);
            int payloadSize = sWin->table[i].buffSize;
            bzero(buffer, sizeof (buffer));
            strncpy(buffer, header, sizeof (header));
            sendPullSubBuffer(sWin, sWin->min + i, payload);
            memcpy(buffer + sizeof (header), payload, payloadSize);
            if (sendto_(sd, buffer, payloadSize + sizeof (header), 0, (struct sockaddr *) &server, serverLen) < 0) {
                LOGGER(log, "Unable to send file size to server\n");
            }
            SENDR_SEND_LOGGER(log, sWin->min + i, LFS0, LFS1, LAR);
        }
        LFS0 = sWin->min;
        LFS1 = sWin->min + i - 1;

        //Now wait for each response:
        for (i = 0; i < SWS && ((sWin->min + i) <= (lastSeq + 1)); i++) {
            bzero(ack, sizeof (ack));
            if (recvfrom(sd, ack, sizeof (ack), 0, (struct sockaddr *) &server, &serverLen) < 0) {
                LOGGER(log, "Timeout occurred.\n");
            }
            if (!strncmp(ack, "ACK", 3)) {
                strtok(ack, " ");
                int tmpLAR = atoi(strtok(NULL, " "));
                SENDR_RECV_LOGGER(log, tmpLAR, LFS0, LFS1, LAR);
                if (tmpLAR > LAR) {
                    LAR = tmpLAR;
                }
            }
        }
    }

    free(fileBuffer);
    freeWindow(sWin);
    fclose(log);
    return 0;
}
