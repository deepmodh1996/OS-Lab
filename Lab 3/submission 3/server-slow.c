#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen; //clilen = size of client address
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;

    int n; // temporary variable for return of read, write, ... from socket

    if (argc < 2) {
     fprintf(stderr,"ERROR, no port provided\n");
     exit(1);
    }

    /* create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    error("ERROR opening socket");

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
    error("setsockopt(SO_REUSEADDR) failed"); 

    /* fill in port number to listen on. IP address can be anything (INADDR_ANY) */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* bind socket to this port number on this machine */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    /* listen for incoming connection requests and creating as well as killing child process */
    while(1==1) {

        /* killing child precess */
        while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
        
        /* start listening */
        listen(sockfd,5);
        clilen = sizeof(cli_addr);

        /* accept a new request, create a newsockfd */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) { error("ERROR on accept"); }

        /* read message from client */
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        printf("Here is the message: %s\n",buffer);

        /* creating child process */
        pid_t child_pid = fork();

        if(child_pid == 0){ // child process

            // NOTE : child communicates to client using same socket on which client binded to server that is newsockfd
            /* opening file to send */
            long long int ffdd = open(buffer,0, O_RDONLY);
            // printf("requested :  %s opened as : %lld, yo buddy: %d\n",buffer,ffdd,getpid());
            off_t fsize = lseek(ffdd,0,SEEK_END);
            lseek(ffdd,0,SEEK_SET);

            /* sending file */
            /* Protocol : first byte of packet is '1' if packet is not last packet; else '0' */
            char buff[3000];
            for (long int i = 0; i < fsize; i+=2047)
            {
                /* sending file maximum of 2047 + 1 bytes in one packet */
                if (i+2047 >= fsize) { // sending last packet
                    buff[0] = '0';
                    read(ffdd, &buff[1], fsize-i);
                    buff[fsize-i+1]='\0';
                } else { // sending remaining packets
                    buff[0] = '1';
                    read(ffdd, &buff[1], 2047);
                }
                // printf("%s\n", buff ); // To check what is being read from file at server

                /* writing in socket */
                n = write(newsockfd,buff,2048);
                if (n < 0) {
                    error("ERROR writing to socket");
                }
                sleep(1);
            }

            /* closing socket */
            int close_return = close(newsockfd);
            //printf("%d closed\n",close_return);
            
            /* closing opend file */
            // printf("%d\n", fcntl(ffdd, F_GETFL));
            int cl = close(ffdd);
            // printf("closing file : %d %lld status : %d\n",cl,ffdd,fcntl(ffdd, F_GETFL) );
            if (cl != 0) {error("ERROR closing file");}

            exit(0);
        }
         else{
             // parent process
            int close_return = close(newsockfd);
             // printf("%d this is parent\n",child_pid);
         }
        
    }

    /* closing listening port */
    close(sockfd);
    return 0;
}