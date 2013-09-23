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

    char buffer[MAXBUFFSIZE + 32];
    char ack[32];
    strncpy(ack, "OK\0", 3);
    int rcvHead = 1;
    while (1) {
        bzero(buffer, sizeof (buffer));
        if(recvfrom(sd, buffer, sizeof (buffer), 0, (struct sockaddr *) &client, &clientLen) < 0){
            //This timeout control is not set until the entire file has been written!
            printf("Timeout occurred. Client terminated. Now server is terminating\n");
            break;
        }

        if (!strcmp(strtok(buffer, " "), "hdr\0")) {
            int filesize = atoi(strtok(NULL, " "));
            int seq = atoi(strtok(NULL, " "));
            if (seq * MAXBUFFSIZE >= filesize /*&& RWS == 0*/) {
                printf("Seq# %i is beyond file scope\n", seq);
                break;
            } else if ((seq + 1) * MAXBUFFSIZE >= filesize /*&& RWS == 0*/) {
                //last part of file is received
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 500000; //500 ms
                if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv)) < 0) {
                    printf("Error setting timeout\n");
                }
            }
            printf("Received %i bytes of %i total bytes in Seq# %i. Sending ACK <%i><%i>...\n", MAXBUFFSIZE, filesize, seq, seq, 6);
            memset(ack, ' ', sizeof (ack));
            ack[31] = 0;
            strcpy(ack, "ACK");
            insertNum(ack, seq, 15);
            insertNum(ack, 6, 28);

            if (sendto_(sd, ack, sizeof (ack), 0, (struct sockaddr *) &client, clientLen) < 1) {
                printf("Error sending ACK\n");
            }
        }
    }

    //writeBuffer(argv[4], initials, 1114448);
}

