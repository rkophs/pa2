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

#define MAXBUFFSIZE 1024

int main(int argc, char *argv[]) {

    /* check command line args. */
    if (argc < 7) {
        printf("usage : %s <server_ip> <server_port> <error rate> <random seed> <send_file> <send_log> \n", argv[0]);
        exit(1);
    }

    /* Note: you must initialize the network library first before calling sendto_().  The arguments are the <errorrate> and <random seed> */
    init_net_lib(atof(argv[3]), atoi(argv[4]));
    printf("error rate : %f\n", atof(argv[3]));

    /* socket creation */
    int sd;
    if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("%s: cannot create socket \n", argv[0]);
        exit(1);
    }

    /* get server IP address (input must be IP address, not DNS name) */
    struct sockaddr_in clientAddr;
    bzero(&clientAddr, sizeof (clientAddr)); //zero the struct
    clientAddr.sin_family = AF_INET; //address family
    clientAddr.sin_port = htons(atoi(argv[2])); //sets port to network byte order
    clientAddr.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address
    printf("%s: sending file '%s' to '%s:%s' \n", argv[0], argv[5], argv[1], argv[2]);

    int fileSize;
    if ((fileSize = getFileSize(argv[5])) < 0) {
        return 1;
    }
    char initials[100];
    char fileSizeStr[16];
    sprintf(fileSizeStr, "%d", fileSize);
    strcat(initials, fileSizeStr);

    //send put <filename> <size> message to server:
    if (sendto(sd, initials, sizeof (initials), 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) < 0) {
        printf("Unable to send file size to server\n");
        return 1;
    }
    printf("File size %i sent to %s:%s, awaiting ACK...\n", fileSize, argv[1], argv[2]);

    /* Receive message from client */
    struct sockaddr_in remote;
    unsigned int remote_length = sizeof (remote);
    
    char ack[100];
    bzero(ack, sizeof(ack));
    
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 50000; //50 ms
    if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
        printf("Error setting timeout\n");
    }
    if(recvfrom(sd, ack, 100, 0, (struct sockaddr *) &remote, &remote_length)<0){
        printf("timeout reached\n");
        return 0;
    }
    if(!strcmp(ack, "OK\0")){
        printf("...ACK received.\n", ack);
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
