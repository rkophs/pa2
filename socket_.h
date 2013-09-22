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

struct sockaddr_in buildLocalAddr(char * ip, char * port){
    struct sockaddr_in addr;
    bzero(&addr, sizeof (addr)); //zero the struct
    addr.sin_family = AF_INET; //address family
    addr.sin_port = htons(atoi(port)); //sets port to network byte order
    addr.sin_addr.s_addr = ip == NULL ?  INADDR_ANY : inet_addr(ip); //sets remote IP address
    return addr;
}