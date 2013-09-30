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

    char header[32];
    char buffer[MAXBUFFSIZE + sizeof (header)];
    memset(header, ' ', sizeof (header));
    header[31] = 0;
    strncpy(header, "hdr ", 4);
    insertNum(header, fileSize, 15);

    char ack[32];
    printf("File size %i sending to %s:%s\n", fileSize, argv[1], argv[2]);

    int seqMax = 33; //20 * fileSize / MAXBUFFSIZE + 1;

    //Initialize window and populate with data;
    struct window *sWin = windowInit(SWS, MAXBUFFSIZE);
    int i;
    for (i = 0; i < SWS; i++) {
        insertSubBuffer(sWin, i, "a", 1);
    }

    int cumAck = -1;
    while (cumAck < seqMax) {
        //Slide window:
        while (sWin->min <= cumAck) {
            sendShiftWindow(sWin, sWin->min + SWS, "a", 1);
        }
         
        //Send each packet in window:
        for (i = 0; i < SWS; i++) {
            if (sWin->min + i <= seqMax) {
                mempnset(header, ' ', 17, 12);
                insertNum(header, sWin->min + i, 28);
                printf("header [%i]: %s\n", sWin->min + i, header);
                bzero(buffer, sizeof (buffer));
                strncpy(buffer, header, sizeof (header));
                if (sendto_(sd, buffer, sizeof (buffer), 0, (struct sockaddr *) &server, serverLen) < 0) {
                    printf("Unable to send file size to server\n");
                }
            }
        }

        //Now wait for each response:
        for (i = 0; i < SWS; i++) {
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

    freeWindow(sWin);
    return 0;
}
