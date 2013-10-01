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

struct sockaddr_in buildAddr(char * ip, char * port){
    struct sockaddr_in addr;
    bzero(&addr, sizeof (addr)); //zero the struct
    addr.sin_family = AF_INET; //address family
    addr.sin_port = htons(atoi(port)); //sets port to network byte order
    addr.sin_addr.s_addr = ip == NULL ?  INADDR_ANY : inet_addr(ip); //sets remote IP address
    return addr;
}

void insertNum(char * buffer, int num, int leastSignificantPos){
    if(num == 0){
        buffer[leastSignificantPos] = '0';
        return;
    }
    if(num < 0){
        buffer[leastSignificantPos] = '1';
        buffer[leastSignificantPos - 1] = '-';
        return;
    }
    int tmp = num;
    int i = leastSignificantPos;
    while(tmp){
        buffer[i] = tmp % 10 + 48;
        tmp /= 10;
        i--;
    }
}

void buildHeader(char * buffer, int bufferSize, char * title, int titleSize, 
                 int num1 , int lSigBit1, int num2, int lSigBit2){
    memset(buffer, ' ', bufferSize);
    buffer[bufferSize-1] = 0;
    strncpy(buffer, title, titleSize);
    insertNum(buffer, num1, lSigBit1);
    insertNum(buffer, num2, lSigBit2);
}

void mempnset(char *buffer, char mask, int pos, int len){
    int i;
    for(i = pos; i < pos + len; i++){
        buffer[i] = mask;
    }
}