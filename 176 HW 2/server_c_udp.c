

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 128

int main(int argc, char **argv) {
  int sockfd;
  int portno; 
  int clientlen; 
  struct sockaddr_in serveraddr; 
  struct sockaddr_in clientaddr; 
  char buf[BUFSIZE]; 
  int n; 

  /* 
   * check command line arguments 
   */
  portno = atoi(argv[1]);

  // socket: create the parent socket 

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(0);
  }

  //  build the server's Internet address
  // I borrowed this segment of code from https://www.geeksforgeeks.org/socket-programming-cc/ by amitds and adjusted it a little

  memset(&serveraddr, 0, sizeof(serveraddr)); 
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) {
    perror("ERROR on binding");
    exit(0);
  }

  /* 
   * main loop: wait for a datagram, then send it to the client until it has length 1
   * I borrowed the sendto and recvfrom lines from https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpserver.c
   */
  clientlen = sizeof(clientaddr);
  while (1) {
    // Receive a UDP datagram from a client
    memset(buf, 0, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0){
      perror("ERROR in recvfrom");
      exit(0);
    }
    while(strlen(buf) != 1){
      // Return Message Formatting
      for(int i = 0; i < strlen(buf) - 1; i++){
        if (buf[i] > '9' || buf [i] < '0'){
          printf("BUF[%i]: %i", i, buf[i]);
          n = sendto(sockfd, "Sorry, cannot compute!", 22, 0, 
             (struct sockaddr *) &clientaddr, clientlen);
          return 0;
        }
      }
      int count = 0;
      for (int i = 0; i < strlen(buf); i++){
        if (buf[i]-'0' >= 0)
          count += (buf[i] - '0');
      }
      
      //Echo the formatted input back to the client 
      
      sprintf(buf,"%d", count);
      n = sendto(sockfd, buf, strlen(buf), 0, 
  	       (struct sockaddr *) &clientaddr, clientlen);
      if (n < 0) {
       perror("ERROR in sendto");
       exit(0);
      }
    }
  }
}