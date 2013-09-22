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
#include "sendto_.h"
#include "fileManip_.h"
#include "socket_.h"

#define MAXBUFFSIZE 1024

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
    struct sockaddr_in server = buildLocalAddr(argv[1], argv[2]);
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
    char fileSizeStr[16];
    bzero(fileSizeStr, sizeof(fileSizeStr));
    sprintf(fileSizeStr, "%d", fileSize);
    char SWS[] = "9";
    
    char initials[100];
    bzero(initials, sizeof (initials));
    strncpy(initials, "fs ", 3);
    strcat(initials, fileSizeStr);
    strcat(initials, " ");
    strcat(initials, SWS);
    printf("inits: %s", initials);
    
    char ack[16];
    printf("File size %i sending to %s:%s, awaiting ACK...\n", fileSize, argv[1], argv[2]);

    int sendingHeader = 1;
    while (sendingHeader) {
        //send put <filename> <size> message to server:
        if (sendto(sd, initials, sizeof (initials), 0, (struct sockaddr *) &server, serverLen) < 0) {
            printf("Unable to send file size to server\n");
            return 1;
        }

        bzero(ack, sizeof (ack));
        if (recvfrom(sd, ack, sizeof(ack), 0, (struct sockaddr *) &server, &serverLen) < 0) {
            printf("timeout #%i\n", sendingHeader);
            sendingHeader++;
        }
        if (!strcmp(ack, "OK\0")) {
            printf("...ACK received.\n", ack);
            sendingHeader = 0;
        }
    }
    

    void *buffer;
    if ((buffer = bufferize(argv[5])) == NULL) {
        return 1;
    }

    //writeBuffer("test.jpg", buffer, fileSize);

    /* Call sendto_ in order to simulate dropped packets */
    //sendto(sd, buffer, fileSize, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));

    return 0;
}
