
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
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostIP;
    char buf[BUFSIZE];

    hostIP = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(0);
    }

    /* build the server's Internet address
    I borrowed this segment of code from https://www.geeksforgeeks.org/socket-programming-cc/ and adjusted it a little */
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    memmove((char *)hostIP, (char *)&serveraddr.sin_addr.s_addr, sizeof(hostIP));
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server 
    I borrowed this segment of code from https://www.geeksforgeeks.org/socket-programming-cc/ and adjusted it a little */
    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) {
        perror("ERROR connecting");
        exit(0);}

    /* get message line from the user */
    printf("Enter string: ");
    bzero(buf, BUFSIZE);
    fgets(buf, BUFSIZE, stdin);

    /* send the message line to the server
    I borrowed the read and write lines from https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpclient.c */
    n = write(sockfd, buf, strlen(buf));
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(0);}
    /* print the server's replies */
    while (strlen(buf) != 1){
        memset(buf, 0, BUFSIZE);
        n = read(sockfd, buf, BUFSIZE);
        if (n < 0){
          perror("ERROR reading from socket");
          exit(0);}

        printf("From server: %s \n", buf);
        if (strcmp(buf,"Sorry, cannot compute!") == 0)
            return 0;
    }
    close(sockfd);
    
    return 0;
}