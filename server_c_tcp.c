#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define BUFSIZE 128


int main(int argc, char **argv) {
  fd_set readfds;
  int parentfd, newfd;
  int childfd[3];
  int portno;
  int clientlen;
  struct sockaddr_in serveraddr;
  struct sockaddr_in clientaddr;
  char buf[BUFSIZE];
  int n;
  int maxFD,sd,activity;
  char strings [15][9];
  for (int i = 0; i < 15; i++){memset(strings[i],0,9);}
  memset(strings,0,15);
  FILE * f = fopen("hangman_words.txt","r");
  for (int i = 0; i < 15; i++){
    fscanf(f,"%s",strings[i]);
  }
  char incorrectChars[3][6];
  //char guessed[strlen(word)+1];                                                                                                                                                                                               
  char word[3][9];
  char guessed[3][9];
  char newWord[9];
  char newGuessed[9];
  char guess = 0;
  int incorrect[3];
  char newIncorrectChars[6];
  memset(newIncorrectChars,0,6);


  portno = atoi(argv[1]);

  /*                                                                                                                                                                                                                            
   * socket: create the parent socket                                                                                                                                                                                           
   */
  for (int i = 0; i < 3; i++){
    childfd[i] = 0;
  }
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
    FD_ZERO(&readfds);
    FD_SET(parentfd, &readfds);
    maxFD = parentfd;
    for (int i = 0 ; i < 3 ; i++){
        sd = childfd[i];
        //if valid socket descriptor then add to read list                                                                                                                                                                      
        if(sd > 0)
            FD_SET(sd , &readfds);
        if(sd > maxFD)
            maxFD = sd;
    }
    activity = select( maxFD + 1 , &readfds , NULL , NULL , NULL);
    if ((activity < 0)){
        printf("select error");
        exit(0);
    }
    if (FD_ISSET(parentfd,&readfds)){
      memset(buf, 0, BUFSIZE);
      newfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
      if (newfd < 0) {
        perror("server-overloaded");
        exit(0);
      }
      int full = 0;
      for (int i = 0; i < 3; i++){
        if (childfd[i] == 0){
          full = 1;
        }
      }
      if (!full){
        buf[0] = 17;
        memcpy(buf+1,"server-overloaded",17);
        n = write(newfd, buf, 18);
        close(newfd);
        memset(newWord,0,9);
        memset(newGuessed,0,9);
        exit(0);
      }
      else{
        n = write(newfd,"apples",6);
        strcpy(newWord,strings[rand()%15]);
        memset(newGuessed,0,9);
        memset(newGuessed,'_',strlen(newWord));
        guess = 0;
        int changed = 0;
        n = read(newfd,buf,BUFSIZE);
        if (buf[1] == 0 && strlen(buf) == 1){
          for (int i = 0; i < 3; i++)   {
            if( childfd[i] == 0 ){
                changed = 1;
                childfd[i] = newfd;
                strcpy(word[i],newWord);
                strcpy(incorrectChars[i],newIncorrectChars);
                strcpy(guessed[i],newGuessed);
                incorrect[i] = 0;
                break;
            }
          }
          buf[0] = 0;
          buf[1] = strlen(newWord);
          buf[2] = 0;
          memcpy(buf+3,newGuessed,strlen(newWord));
          //memcpy(buf+2+strlen(newWord),newIncorrectChars,6);                                                                                                                                                                  
          n = write(newfd, buf, buf[2]+buf[3]+3);
          if (n < 0) {
            perror("ERROR writing to socket");
            exit(0);
          }
        }
      }
    }
    for (int i = 0; i < 3; i++){
      sd = childfd[i];
      if (FD_ISSET(sd , &readfds)){
        srand(time(0));
        memset(buf, 0, BUFSIZE);
        n = read(childfd[i], buf, BUFSIZE);
        printf("server received %d bytes: %s", n, buf);
        for (int i = 0; i < n; i++){ printf("%i ", buf[i]);}
        if (n < 0) {
          perror("ERROR reading from socket");
          exit(0);}
        guess = buf[1];
        memset(buf, 0, BUFSIZE);
        buf[1] = strlen(word[i]);
        int changed = 0;
        for (int j = 0; j < buf[1]; j++){
          if (word[i][j] == guess){
            guessed[i][j] = guess;
            changed = 1;
          }
        }
        if (changed == 0){
          incorrectChars[i][incorrect[i]] = guess;
          incorrect[i]++;
        }
        buf[2] = incorrect[i];
        if (strcmp(word[i],guessed[i])==0){
          memset(buf, 0, BUFSIZE);
          buf[0] = strlen("The word was ") + strlen(word[i])*2;
          memcpy(buf+1,"The word was ",13);
          printf("%s", word[i]);
          for (int j = 0; j < strlen(word[i]); j++){
            buf[14+2*j] = word[i][j];
            buf[15+2*j] = ' ';
          }
          buf[buf[0]+1] = 9;
          memcpy(buf+buf[0]+2,"You Win!",9);
          n = write(childfd[i], buf, buf[0]+1+buf[buf[0]+1]);
          if (n < 0) {
            perror("ERROR writing to socket");
            exit(0);
          }
          close(childfd[i]);
          childfd[i] = 0;
        }
        else{
          if(incorrect[i] >= 6){
            memset(buf, 0, BUFSIZE);
            buf[0] = strlen("The word was ") + strlen(word[i])*2;
            memcpy(buf+1,"The word was ",13);
            for (int j = 0; j < strlen(word[i]); j++){
              buf[14+2*j] = word[i][j];
              buf[15+2*j] = ' ';
            }
            buf[buf[0]+1] = 10;
            memcpy(buf + buf[0]+2,"You Lose!",10);
            n = write(childfd[i], buf, buf[0]+1+buf[buf[0]+1]);
            if (n < 0) {
              perror("ERROR writing to socket");
              exit(0);
            }
            close(childfd[i]);
            childfd[i] = 0;
          }
          else{
            memcpy(buf+3,guessed[i],strlen(word[i]));
            strcat(buf+3+strlen(word[i]), incorrectChars[i]);
            n = write(childfd[i], buf, buf[1]+buf[2]+3);
            if (n < 0) {
              perror("ERROR writing to socket");
              break;
            }
          }
        }
      }
    }
  }
}

