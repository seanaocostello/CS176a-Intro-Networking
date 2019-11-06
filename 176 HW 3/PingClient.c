#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#define BUFSIZE 128

double timeDiff(struct timespec t1, struct timespec t2){
    return (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_nsec - t1.tv_nsec)/1000000.0;
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostName;
    char buf[BUFSIZE];
    struct timeval timeout;//, start, sent, rec;                                                                                                                         
    struct timespec start, sent, rec;
    fd_set readfds, masterfds;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int packetsTransmitted = 0;
    int packetsReceived = 0;
    double minRTT = 1000000000;
    double maxRTT = 0;
    double avgRTT = 0;

    /* get command line arguments */
    hostName = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if ((server = gethostbyname(hostName)) == NULL){
      perror("DNS Fetch failed");
      exit(1);
    }

    /* build the server's Internet address                                                                                                                               
    I borrowed this segment of code from https://www.geeksforgeeks.org/socket-programming-cc/ by amitds and adjusted it a little */
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    memmove((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    memset(buf, 0, BUFSIZE);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);


    /* send the message to the server                                                                                                                                    
    I borrowed the sendto and recvfrom lines from https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpclient.c */
    serverlen = sizeof(serveraddr);
    for (int seqNum = 0; seqNum < 10; seqNum++){
        FD_ZERO(&masterfds);
        FD_SET(sockfd, &masterfds);
        memcpy(&readfds, &masterfds, sizeof(fd_set));
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &sent);
        sprintf(buf, "PING %i %f \n", seqNum, timeDiff(start, sent));
        n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
        packetsTransmitted++;
        memset(buf, 0, strlen(buf));
        setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
        n = recvfrom(sockfd, buf, BUFSIZE, 0, &serveraddr, &serverlen);
        if (n >= 0){
          clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &rec);
          printf("PING received from %s: seq#=%i time=%.3f ms \n", hostName, seqNum, timeDiff(start, rec) - timeDiff(start, sent));
          packetsReceived++;
          if (timeDiff(start, rec) - timeDiff(start, sent) < minRTT){
              minRTT = timeDiff(start, rec) - timeDiff(start, sent);
          }
          if (timeDiff(start, rec) - timeDiff(start, sent) > maxRTT){
              maxRTT = timeDiff(start, rec) - timeDiff(start, sent);
          }
          avgRTT += timeDiff(start, rec) - timeDiff(start, sent);
          sleep(1);
          }
        else{
          printf("Request timeout for icmp_seq %i \n", seqNum);
        } 
    }
    printf("--- ping statistics --- \n%i packets transmitted, %i received, %.f%s packet loss \nrtt min/avg/max = %.3f %.3f %.3f ms \n",
        packetsTransmitted, packetsReceived, ((double) (packetsTransmitted - packetsReceived)/packetsTransmitted)*100, "%", minRTT, avgRTT/packetsReceived, maxRTT);
    close(sockfd);
    return 0;
}