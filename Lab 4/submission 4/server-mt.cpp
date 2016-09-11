#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <queue>
#include <pthread.h>
#include <unistd.h>

/* request queue */
std::queue<int> request_queue;

pthread_mutex_t mutex_request_queue;
pthread_cond_t has_request;
pthread_cond_t has_space;

void error(char *msg)
{
    perror(msg);
    //return 1;
}

typedef struct thread_data {
    pthread_t thread_id;
    int thread_number;
    int curr_socket;
    char buffer[256];
} thread_data;

 
/* thread function */
void *thread_func(void *arg) {
    thread_data *data = (thread_data *)arg;
    int n; // temporary variable

    /* worker thread works forever */
    while(1==1){
        pthread_mutex_lock (&mutex_request_queue);

        while(request_queue.empty()){ 
            // printf("waiting for request from client in thread number %d\n",data->thread_number);
            pthread_cond_wait(&has_request, &mutex_request_queue);
        }

        data->curr_socket = request_queue.front();
        request_queue.pop();

        pthread_cond_signal(&has_space);

        pthread_mutex_unlock (&mutex_request_queue);

////////////////        printf("Thread %d has %d socket\n",data->thread_id, data->curr_socket );
        
        /* handle current client */

           /* read message from client */
            bzero(data->buffer,256);
            n = read(data->curr_socket,data->buffer,255);
            if (n < 0) error("ERROR reading from socket");
     
            // printf("%s on %d\n",data->buffer, data->thread_number);

            // NOTE : worker_thread communicates to client using same socket on which client binded to server that is newsockfd
            
            /* opening file to send */
            long long int ffdd = open(data->buffer,0, O_RDONLY);
            // printf("requested :  %s opened as : %lld\n",data->buffer,ffdd);
            off_t fsize = lseek(ffdd,0,SEEK_END);
            lseek(ffdd,0,SEEK_SET);

            /* sending file */
            /* Protocol : first byte of packet is '1' if packet is not last packet; else '0' */
            char buff[3000];
            for (long int i = 0; i < fsize; i+=2047)
            {
                /* sending file maximum of 2047 + 1 bytes in one packet */
                // if (i+2047 >= (fsize)) { // sending last packet
                //     buff[0] = '0';
                //     read(ffdd, &buff[1], fsize-i);
                //     //buff[fsize-i+1]='\0';
                // } else { // sending remaining packets
                //     buff[0] = '1';
                //     read(ffdd, &buff[1], 2047);
                // }
                bzero(buff,3000);
                if (i+2047 >= fsize) { // sending last packet
                    buff[0] = '0';
                    // buff[fsize-i+1]='\0';
                } else { // sending remaining packets
                    buff[0] = '1';
                    // read(ffdd, &buff[1], 2047);
                }
                read(ffdd, &buff[1], 2047);
                // printf("%d thread number sends : %s\n",data->thread_number, buff ); // To check what is being read from file at server

                /* writing in socket */
                n = write(data->curr_socket,buff,2048);
                if (n < 0) {
                    break;
                    error("ERROR writing to socket");
                    // printf("vvvvvvvvvvvvvvvv%dvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n", data->thread_number);
                }
            }

            /* closing socket */
            int close_return = close(data->curr_socket);
            //printf("%d closed\n",close_return);
            
            /* closing opend file */
            // printf("%d\n", fcntl(ffdd, F_GETFL));
            int cl = close(ffdd);
            //printf("closing file : %d %lld status : %d\n",cl,ffdd,fcntl(ffdd, F_GETFL) );
            if (cl != 0) {error("ERROR closing file");}


    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen; //clilen = size of client address
    struct sockaddr_in serv_addr, cli_addr;

    int num_worker = 5;
    int queue_size = 5;
    int i;

    int n; // temporary variable for return of read, write, ... from socket

    if (argc < 4) {
     fprintf(stderr,"ERROR, Incorrect input\n");
     return 1;
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

    num_worker = atoi(argv[2]);
    queue_size = atoi(argv[3]);


    /* initializing lock */
    pthread_mutex_init(&mutex_request_queue, NULL);

    /* creating pool of workers */
    thread_data thr_data[num_worker]; // creating thread data
    
    /* create threads */
    for (i = 0; i < num_worker; i++) {
        thr_data[i].thread_number = i;
        int rc;
        if ((rc = pthread_create(&thr_data[i].thread_id, NULL, thread_func, &thr_data[i]))) {
            fprintf(stderr, "error: pthread_create, error: %d\n", rc);
            return EXIT_FAILURE;
        }
    }

    
    // int rss = 0;
    /* listen for incoming connection requests and creating as well as killing child process */
    while(1==1) {
        // rss++;
        // printf("%d rss\n",rss );
        /* start listening */
        listen(sockfd,5);
        clilen = sizeof(cli_addr);

        /* accept a new request, create a newsockfd */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) { error("ERROR on accept"); }

        /* pushing socket file descriptor in request queue*/
        // lock, send signal to wake up worker   
        pthread_mutex_lock (&mutex_request_queue);
        
        while( (queue_size>0) && (request_queue.size() >= queue_size) ){ 
            // printf("queue is full, waiting for queue have some space\n");
            pthread_cond_wait(&has_space, &mutex_request_queue);
        }

        request_queue.push(newsockfd);
        // printf("queue size is %d\n",request_queue.size());
        pthread_cond_signal(&has_request);

        pthread_mutex_unlock (&mutex_request_queue);     
        

    }

    /* deleting lock on request_queue */
    pthread_mutex_destroy(&mutex_request_queue);

    /* block until all threads complete */
    for (i = 0; i < num_worker; i++) { pthread_join(thr_data[i].thread_id, NULL); }

    /* closing listening port */
    close(sockfd);
    return 0;
}