/* GBNserver.c */
/* This is a sample UDP server/receiver program */
/* This code will not work unless modified. */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <stdlib.h>
#include <time.h>
#include "dependencies/sendto_.h"
#include "dependencies/fileManip_.h"
#include "dependencies/socket_.h"
#include "dependencies/window_.h"

#define MAXBUFFSIZE 1024
#define RWS 6

int main(int argc, char *argv[]) {

    int nBytes = 0;

    /* check command line args. */
    if (argc < 6) {
        printf("usage : %s <server_port> <error rate> <random seed> <send_file> <send_log> \n", argv[0]);
        exit(1);
    }

    /* Note: you must initialize the network library first before calling sendto_().  The arguments are the <errorrate> and <random seed> */
    init_net_lib(atof(argv[2]), atoi(argv[3]));
    printf("error rate : %f\n", atof(argv[2]));

    /* socket creation */
    int sd;
    if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("%s: cannot open socket \n", argv[0]);
        return 1;
    }

    /* set up address spaces and bind to socket*/
    struct sockaddr_in server = buildAddr(NULL, argv[1]);
    struct sockaddr_in client;
    unsigned int clientLen = sizeof (client);
    if (bind(sd, (struct sockaddr *) &server, sizeof (server)) < 0) {
        printf("%s: cannot to bind port number %s \n", argv[0], argv[1]);
        return 1;
    }

    //Set up recv buffer and ACK response buffer
    char buffer[MAXBUFFSIZE + 32];
    char ack[32];

    //Instantiate window with N buffers of 1024 bytes:
    struct window* rWin = windowInit(RWS, MAXBUFFSIZE);
    char payload[MAXBUFFSIZE];

    //Open file:
    FILE *file;
    if(!(file = fopen(argv[4], "wb"))){
        printf("Error creating file: %s\n", argv[4]);
        return 0;
    }
    
    //Iterate through Synchronous connection:
    while (1) {

        //Wait for client to send data:
        bzero(buffer, sizeof (buffer));
        if ((nBytes = recvfrom(sd, buffer, sizeof (buffer), 0, (struct sockaddr *) &client, &clientLen)) < 0) {
            //This timeout control is not set until the entire file has been written!
            //In case of a timeout, the file is completely received and this server may exit.
            printf("Timeout occurred. Client terminated. Now server is terminating\n");
            break;
        }

        //If data buffer starts with 'hdr' (which it always should):
        if (!strcmp(strtok(buffer, " "), "hdr\0")) {
            int filesize = atoi(strtok(NULL, " ")); //Parse file size
            int seq = atoi(strtok(NULL, " ")); //Parse sequence #
            printf("Received seq#: %i of size: %i\n", seq, nBytes);

            //Copy packet buffer into payload to pass to window:
            int i;
            for (i = 0; i < MAXBUFFSIZE; i++) {
                payload[i] = buffer[i + 32];
            }
            insertSubBuffer(rWin, seq, payload, nBytes - 32);
            printWindow(rWin);

            //If possible, write all buffers lesser than cumAck into file:
            while (rWin->cumSeq >= rWin->min) {
                int size = rWin->table[0].buffSize;
                if (recvShiftWindow(rWin, payload) >= 0 && size > 0) {
                    printf("Writing with Size: %i\n", size);
                    if (!fwrite(payload, 1, size, file)) {
                        printf("Error write to file\n");
                        fclose(file);
                        exit(1);
                    }
                } //Only shifts if appr.

            }

            /*Set up timeout with client after all seq.# arrived for file
             * (At which point the client stops sending ACKs.)
             */
            if ((rWin->cumSeq) * MAXBUFFSIZE >= filesize) {
                printf("Setting up timeout with cumAck: %i and fileSize: %i\n", rWin->cumSeq, filesize);
                //last part of file is received
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 500000; //500 ms
                if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv)) < 0) {
                    printf("Error setting timeout\n");
                }
            }

            buildHeader(ack, sizeof (ack), "ACK", 3, rWin->cumSeq, 15, rWin->rws, 28);

            if (sendto_(sd, ack, sizeof (ack), 0, (struct sockaddr *) &client, clientLen) < 1) {
                printf("Error sending ACK\n");
            }
        }
    }

    fclose(file);
    freeWindow(rWin);
    return 0;
}
