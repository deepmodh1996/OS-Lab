#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>

// ./get-one-file files/foo0.txt localhost 5000 display

#define BILLION 1E9;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int downloaded = 0;

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        printf("\nReceived SIGINT; dowmloaded %d bytes so far.\n", downloaded);
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("\ncan't catch SIGINT\n");
    }

    int sockfd, portno, n;//, users, totTime, sleepTime;
    // char* type;
    // double success=0.0, response=0.0;

    // struct timespec requestStart, requestEnd;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 5) {
       fprintf(stderr,"usage %s fileName hostname port disp \n", argv[0]);
       exit(0);
    }

    /* get the total user threads */
    // users = atoi(argv[3]);

    /* get the experiment time time */
    // totTime = atoi(argv[4]);

    /* get the sleeping time */
    // sleepTime = atoi(argv[5]);

    /* create socket, get sockfd handle */

    portno = atoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* fill in server address in sockaddr_in datastructure */

    server = gethostbyname(argv[2]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    /* setting up the client thread */

    /* connect to server */

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    /* ask user for input */

    // printf("Please enter the message: ");
    bzero(buffer,256);
    // fgets(buffer,255,stdin);

    /* send user message to server */

    n = write(sockfd,argv[1],strlen(argv[1]));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);

    // clock_gettime(CLOCK_REALTIME, &requestStart);

    /* read reply from server */
    for ( ; ; ) {
        char buff[4000];
        n = read(sockfd,buff,2048);
        if (n < 0) error("ERROR reading from socket");
        if (n == 0){

            // clock_gettime(CLOCK_REALTIME, &requestEnd);
            // success++;

            // printf("sleeping ...\n"); 
            // sleep(sleepTime);
            // printf("....  woke up\n");
            
            // double resnano = (requestEnd.tv_nsec - requestStart.tv_nsec)/BILLION;
            // response += (requestEnd.tv_sec - requestStart.tv_sec + resnano);
            break; 
        }
        downloaded += strlen(buff);
        if (strcmp(argv[4],"display") == 0) {
            for (int i = 1; i < strlen(buff); i++) {
                printf("%c", buff[i]);
            }
        }
        //printf("buff 0 is %c\n",buff[0]);
    }
     if (strcmp(argv[4],"display") == 0) {printf("\n");}
    close(sockfd);
    // printf("Done!\n");
    // printf("throughput = %f\n", success/totTime);
    // printf("average response time = %f\n", response);///success);
    return 0;
}