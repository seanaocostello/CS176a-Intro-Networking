
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
  int parentfd; 
  int childfd; 
  int portno; 
  int clientlen; 
  struct sockaddr_in serveraddr; 
  struct sockaddr_in clientaddr; 
  char buf[BUFSIZE]; 
  int n; 

  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0) {
    perror("ERROR opening socket");
    exit(0);
  }

  /*
   * build the server's Internet address
   I borrowed this segment of code from https://www.geeksforgeeks.org/socket-programming-cc/ by amitds and adjusted it a little
   */
  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   I borrowed this segment of code from https://www.geeksforgeeks.org/socket-programming-cc/ by amitds
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0){ 
    perror("ERROR on binding");
    exit(0);}

  /* 
   * listen: make this socket ready to accept connection requests 
   I borrowed this segment of code from https://www.geeksforgeeks.org/socket-programming-cc/ by amitds and adjusted it a little
   */
  if (listen(parentfd, 3) < 0){ 
    perror("ERROR on listen");
    exit(0);}

  /* 
   * main loop: wait for a connection request, echo input line, 
   * then close connection.
     I 
   */

  clientlen = sizeof(clientaddr);
  
  while (1) {
    /* 
     * accept: wait for a connection request 
     I borrowed the read and write lines from https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpserver.c 
     */
    memset(buf, 0, BUFSIZE);
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0) {
      perror("ERROR on accept");
      exit(0);
    }
    char word = {'h','a','n','g','m','a','n'}
    char incorrectChars [6];
    int incorrect = 0;
    printf("Hello");
    /* 
     * read: read input string from the client
     */
    
    n = read(childfd, buf, BUFSIZE);
    printf("server received %d bytes: %s", n, buf);
    if (n < 0) {
      perror("ERROR reading from socket");
      exit(0);}
    if (buf[0] != 1){
      memset(buf, 0, BUFSIZE);
      buf[0] = 31;
      strcat(buf,"Error! Please guess one letter.");
      n = write(childfd, buf, 32);
    }
    else{

    }

      /*
    while (strlen(buf) != 1){
      //Return message formatting
      for(int i = 0; i < strlen(buf) - 1; i++){
        if (buf[i] >= '9' || buf[i] <= '0'){
          n = write(childfd, "Sorry, cannot compute!", 22);
          return 0;
        }
      }
      int count = 0;
      for (int i = 0; i < strlen(buf); i++){
        if (buf[i]-'0' >= 0)
          count += (buf[i] - '0');
      }*/
      sprintf(buf,"%d", count);
      /* 
       * write: echo the input string back to the client 
       */ 
      n = write(childfd, buf, strlen(buf));
      if (n < 0) {
        perror("ERROR writing to socket");
        exit(0);
      }
      sleep(1); //To prevent the server from writing to the socket twice before the client ever reads.
    }
  }
  close(childfd);
}