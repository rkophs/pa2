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
#include "sendto_.h"
#include "fileManip_.h"
#include "socket_.h"

#define MAXBUFFSIZE 1024

int main(int argc, char *argv[]) {

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
    struct sockaddr_in server = buildLocalAddr(NULL, argv[1]);
    struct sockaddr_in client;
    unsigned int clientLen = sizeof (client);
    if (bind(sd, (struct sockaddr *) &server, sizeof (server)) < 0) {
        printf("%s: cannot to bind port number %s \n", argv[0], argv[1]);
        exit(1);
    }

    char initials[100];
    char ack[16];
    strncpy(ack, "OK\0", 3);
    int rcvHead = 1;
    while (rcvHead) {
        bzero(initials, sizeof (initials));
        recvfrom(sd, initials, sizeof (initials), 0, (struct sockaddr *) &client, &clientLen);
        char *token1 = strtok(initials, " ");
        if (!strcmp(token1, "fs\0")) {
            char *token2 = strtok(NULL, " "); //filesize
            char *token3 = strtok(NULL, " "); //SWS
            int fileSize = atoi(token2);
            int SWS = atoi(token3);
            printf("Received file size %i and SWS %i from client #%i times. Sending ACK...\n", fileSize, SWS, rcvHead);
            rcvHead++;
            /* Respond using send to_ in order to simulate dropped packets */
            if (sendto_(sd, ack, sizeof (ack), 0, (struct sockaddr *) &client, clientLen) < 1) {
                printf("Error sending ACK\n");
                return 0;
            }
        } 
        
        if(!strcmp(token1, "head\0")){
            int seq = atoi(strtok(NULL, " ")); //seq#
            int filesize = atoi(strtok(NULL, " "));
            
            printf("Received %i bytes of %i total bytes in Seq# %i. Sending ACK <%i><%i>...\n", MAXBUFFSIZE, filesize, seq, seq, 6);
            
            rcvHead = 0;
        }
    }

    //writeBuffer(argv[4], initials, 1114448);
}

