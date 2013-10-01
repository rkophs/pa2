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
#include "dependencies/fileManip_.h"
#include "dependencies/socket_.h"
#include "dependencies/window_.h"

#define MAXBUFFSIZE 1024
#define SWS 9

int main(int argc, char *argv[]) {

    /* check command line args. */
    if (argc < 7) {
        printf("usage : %s <server_ip> <server_port> <error rate> <random seed> <send_file> <send_log> \n", argv[0]);
        return 1;
    }

    init_net_lib(atof(argv[3]), atoi(argv[4]));
    printf("error rate : %f\n", atof(argv[3]));

    /* socket creation */
    int sd;
    if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("%s: cannot create socket \n", argv[0]);
        return 1;
    }

    /* get server IP address (input must be IP address, not DNS name) */
    struct sockaddr_in server = buildAddr(argv[1], argv[2]);
    unsigned int serverLen = sizeof (server);
    printf("%s: sending file '%s' to '%s:%s' \n", argv[0], argv[5], argv[1], argv[2]);

    /* set timeout for recvfrom function*/
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 50000; //50 ms
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv)) < 0) {
        printf("Error setting timeout\n");
        return 1;
    }

    /* get size of file to send to server*/
    int fileSize;
    if ((fileSize = getFileSize(argv[5])) < 0)
        return 1;
    int lastSize = fileSize % MAXBUFFSIZE;
    int lastSeq = fileSize / MAXBUFFSIZE;
    char *fileBuffer = bufferize(argv[5]);

    char header[32];
    char buffer[MAXBUFFSIZE + sizeof (header)];
    char payload[MAXBUFFSIZE];
    char ack[32];
    printf("File size %i sending to %s:%s\n", fileSize, argv[1], argv[2]);

    int seqMax = fileSize / MAXBUFFSIZE + 1;

    //Initialize window and populate with data;
    struct window *sWin = windowInit(SWS, MAXBUFFSIZE);
    int i;
    for (i = 0; i < SWS && i < seqMax; i++) {
        printf("IT: %i %i\n", i, lastSeq);
        if (i == lastSeq) {
            insertSubBuffer(sWin, i, fileBuffer + i*MAXBUFFSIZE, lastSize);
        } else if (i < lastSeq) {
            insertSubBuffer(sWin, i, fileBuffer + i*MAXBUFFSIZE, MAXBUFFSIZE);
        } else {
            printf("Else");
            insertSubBuffer(sWin, i, NULL, 0);
        }

    }

    printf("TEST %i", lastSize);
    int cumAck = -1;
    while (cumAck < seqMax) {

        //Slide window:
        while (sWin->min <= cumAck && (sWin->min + SWS) <= seqMax) {
            if (sWin->min + SWS == seqMax) {
                sendShiftWindow(sWin, sWin->min + SWS, NULL, 0);
            } else {
                int pos = (sWin->min + SWS) * MAXBUFFSIZE;
                int size = (sWin->min + SWS == lastSeq) ? lastSize : MAXBUFFSIZE;
                printf("SW: size: %i, Seq: %i", size, sWin->min + SWS);
                sendShiftWindow(sWin, sWin->min + SWS, fileBuffer + pos, size);
            }
        }

        //Send each packet in window and only those packets within the fileSize:
        for (i = 0; (i < SWS) && ((sWin->min + i) <= seqMax); i++) {
            buildHeader(header, sizeof (header), "hdr", 3, fileSize, 15, sWin->min + i, 28);
            int payloadSize = sWin->table[i].buffSize;
            printf("header [%i]: %s || size: %i\n", sWin->min + i, header, payloadSize);
            bzero(buffer, sizeof (buffer));
            strncpy(buffer, header, sizeof (header));
            sendPullSubBuffer(sWin, sWin->min + i, payload);
            memcpy(buffer + sizeof (header), payload, payloadSize);
            if (sendto_(sd, buffer, payloadSize + sizeof (header), 0, (struct sockaddr *) &server, serverLen) < 0) {
                printf("Unable to send file size to server\n");
            }
        }

        //Now wait for each response:
        for (i = 0; i < SWS && ((sWin->min + i) <= seqMax); i++) {
            if (sWin->min + i <= seqMax) {
                bzero(ack, sizeof (ack));
                if (recvfrom(sd, ack, sizeof (ack), 0, (struct sockaddr *) &server, &serverLen) < 0) {
                    printf("timeout\n");
                }
                if (!strncmp(ack, "ACK", 3)) {
                    printf("ACK: %s\n", ack);
                    char *token = strtok(ack, " ");
                    int tmpCumAck = atoi(strtok(NULL, " "));
                    if (tmpCumAck > cumAck) {
                        cumAck = tmpCumAck;
                    }
                }
            }
        }
    }

    printf("Outside ]n");
    free(fileBuffer);
    freeWindow(sWin);
    return 0;
}
