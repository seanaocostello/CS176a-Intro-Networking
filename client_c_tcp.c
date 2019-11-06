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
    I borrowed this segment of code from https://www.geeksforgeeks.org/socket-programming-cc/ by amitds and adjusted it a little */
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    memmove((char *)hostIP, (char *)&serveraddr.sin_addr.s_addr, sizeof(hostIP));
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server                                                                                                                                                                             
    I borrowed this segment of code from https://www.geeksforgeeks.org/socket-programming-cc/ by amitds and adjusted it a little */
    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) {
        perror("ERROR connecting");
        exit(0);}

    //sleep(1);
    //printf("Hello \n");
    memset(buf, 0, BUFSIZE);
    n = read(sockfd, buf, BUFSIZE);
    char connect [buf[0]+1];
    memset(connect,0,buf[0]+1);
    memcpy(connect, buf+1, buf[0]);
    if (strcmp(connect,"server-overloaded") == 0){
        printf("%s \n", connect);
        exit(0);
    }

    /* get message line from the user */
    printf("Ready to start game? (y/n): ");
    bzero(buf, BUFSIZE);
    fgets(buf, BUFSIZE, stdin);
    //printf("%s",buf);                                                                                                                                                                                                         
    //int gameOver = 0;                                                                                                                                                                                                         
    int initiate = 0;

    while (1){
        if (initiate == 1){
            printf("Letter to guess: ");
            bzero(buf, BUFSIZE);
            fgets(buf, BUFSIZE, stdin);
            char word[15];
            strcpy(word,buf);
            bzero(buf, BUFSIZE);
            if (strlen(word) == 2){
                if(word[0] > 64 && word[0] < 91){
                    word[0]+=32;
                }
                else if (!(word[0] > 96 && word[0] < 123)){
                    printf("Error! Please guess one letter. \n");
                    continue;
                }
                buf[0] = 1;
                buf[1] = word[0];
                n = write(sockfd, buf, strlen(word));
            }
            else{
                printf("Error! Please guess one letter. \n");
                continue;
            }
            if (n < 0){
              perror("ERROR writing to socket");
              exit(0);}
            //exit(0);                                                                                                                                                                                                          
        }
        else{
            if (buf[0] == 'y'){
                initiate = 1;
                buf[0] = 1;
                buf[1] = 0;
                n = write(sockfd,buf,2);
                if (n < 0){
                  perror("ERROR writing to socket");
                  exit(0);}
            }
            else{exit(0);}
        }
        memset(buf, 0, BUFSIZE);
        n = read(sockfd, buf, BUFSIZE);
        if (n < 0){
          perror("ERROR reading from socket");
          exit(0);}
        if (buf[0] != 0){
            char message [buf[0]+1];
            memset(message,0,buf[0]+1);
            memcpy(message, buf+1, buf[0]);
            //printf("%s \n", buf);                                                                                                                                                                                             
            printf("%s \n", message);
            char check [14];
            memset(check,0,14);
            memcpy(check,buf+1,13);
            if (strcmp(check, "The word was ") == 0){
                char winLose [11];
                memset(winLose,0,11);
                memcpy(winLose, buf+buf[0]+2,buf[buf[0]+1]);
                printf("%s \n",winLose);
                printf("Game Over! \n");
                break;
            }
            if (strcmp(message,"server-overloaded") == 0){
                exit(0);
            }
        }
        else{
            char word[15];
            memset(word,0,15);
            strncpy(word, buf+3,buf[1]);
            char incorrect [7];
            memset(incorrect,0,7);
            strncpy(incorrect, buf+3+buf[1], buf[2]);
            printf("%s \nIncorrect Guesses: %s \n \n", word, incorrect);
            if (buf[2] >= 6){break;}
        }
    }
    close(sockfd);

    return 0;
}
