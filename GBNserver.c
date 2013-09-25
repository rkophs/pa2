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

    //Set up recv buffer and ACK response buffer
    char buffer[MAXBUFFSIZE + 32];
    char ack[32];
    strncpy(ack, "OK\0", 3);
    
    //Instantiate window with N buffers of 1024 bytes:
    struct window* rWin = windowInit(RWS, MAXBUFFSIZE);
    char payload[MAXBUFFSIZE];
    //Iterate through Synchronous connection:
    while (1) {
        
        //Wait for client to send data:
        bzero(buffer, sizeof (buffer));
        if(recvfrom(sd, buffer, sizeof (buffer), 0, (struct sockaddr *) &client, &clientLen) < 0){
            //This timeout control is not set until the entire file has been written!
            printf("Timeout occurred. Client terminated. Now server is terminating\n");
            break;
        }

        //If data buffer starts with hdr (which it always should):
        if (!strcmp(strtok(buffer, " "), "hdr\0")) {
            int filesize = atoi(strtok(NULL, " "));     //Parse file size
            int seq = atoi(strtok(NULL, " "));          //Parse sequence #
            
            //Copy packet buffer into payload to pass to window:
            int i;
            for(i = 0; i < MAXBUFFSIZE; i++){
                payload[i] = buffer[i+32];
            }
            insertSubBuffer(rWin, seq, payload, sizeof(payload));
            printWindow(rWin);
            //If possible, write all buffers <= cumAck into file:
            while(rWin->cumSeq >= rWin->min){
                char *left = recvShiftWindow(rWin); //Only truly shifts if appr.
                //Write left to file here!!!
                free(left);
                left = NULL; //Must do so to avoid double freeing of mem.
            }
            
            if (rWin->cumSeq >= filesize) { //This logic is incorrect
                printf("Seq# %i is beyond file scope\n", seq);
                break;
            } else if ((seq + 1) * MAXBUFFSIZE >= filesize) {
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
            strncpy(ack, "ACK", 3);
            insertNum(ack, rWin->cumSeq, 15);
            insertNum(ack, rWin->rws, 28);

            if (sendto_(sd, ack, sizeof (ack), 0, (struct sockaddr *) &client, clientLen) < 1) {
                printf("Error sending ACK\n");
            }
        }
    }

    free(rWin);
}

