
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFSIZE 128

int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    char *hostIP; 
    char buf[BUFSIZE];

    /* get command line arguments */
    hostIP = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* build the server's Internet address 
    I borrowed this segment of code from https://www.geeksforgeeks.org/socket-programming-cc/ by amitds and adjusted it a little */
    memset(&serveraddr, 0, sizeof(serveraddr)); 
    serveraddr.sin_family = AF_INET;
    memmove((char *)hostIP, 
	  (char *)&serveraddr.sin_addr.s_addr, sizeof(hostIP));
    serveraddr.sin_port = htons(portno);

    /* get a message from the user */
    memset(buf, 0, BUFSIZE);
    printf("Enter string: ");
    fgets(buf, BUFSIZE, stdin);

    /* send the message to the server 
    I borrowed the sendto and recvfrom lines from https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpclient.c */
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
     /* Loop printing the server's replies */
     while (strlen(buf) != 1){
        
        memset(buf, 0, strlen(buf));
        n = recvfrom(sockfd, buf, BUFSIZE, 0, &serveraddr, &serverlen);
        printf("From server: %s \n", buf);
        if (strcmp(buf,"Sorry, cannot compute!") == 0)
            return 0;
    }
    close(sockfd);
    return 0;
}