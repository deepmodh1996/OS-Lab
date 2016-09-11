#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>

#define BILLION 1E9;

void error(char *msg)
{
    perror(msg);
    // exit(0);
}

/* create thread argument struct for thr_func() */
typedef struct thread_data {
    struct hostent *server;
    int tid,port_no,tot_time,sleep_time; //tid = Thread ID
    char *file_type;
    double success, response;
} thread_data;

thread_data* cnst(char *argv[]){
    /* fill in server address in sockaddr_in datastructure */
    thread_data *q = malloc(sizeof(thread_data));
    
    q->server = gethostbyname(argv[1]);
    if (q->server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    /* get the servers port number */
    q->port_no = atoi(argv[2]);

    /* get the experiment time time */
    q->tot_time = atoi(argv[4]);

    /* get the sleeping time */
    q->sleep_time = atoi(argv[5]);

    /* set the file type for the users */
    q->file_type = argv[6];
    
    q->success = 0.0;
    q->response = 0.0;

    return q;
}
 
/* thread function */
void *thr_func(void *arg) {
    thread_data *data = (thread_data *)arg;

    /* threadStart, threadEnd keeps track of experiment timeing */
    struct timespec threadStart, threadEnd;
    clock_gettime(CLOCK_REALTIME, &threadStart);

    while( (threadEnd.tv_sec - threadStart.tv_sec) < data->tot_time) {

        /* create socket, get sockfd handle */
        struct timespec requestStart, requestEnd;

        struct sockaddr_in serv_addr;

        int sockfd, n;
        sockfd = socket(AF_INET, SOCK_STREAM , 0);
        if (sockfd < 0) {
            error("ERROR opening socket");
        }

        /* fill in server address in sockaddr_in datastructure */
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)data->server->h_addr, 
             (char *)&serv_addr.sin_addr.s_addr,
             data->server->h_length);
        serv_addr.sin_port = htons(data->port_no);

        /* connect to server */
        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
            error("ERROR connecting");
        }

        /* send request to server */
    /*****************************************************************************/
        int file_num = 1; // file number in case of fixed

        if (strcmp(data->file_type,"random") == 0) { 
            file_num = rand() % 10000;
        }

        char num_str[10]; // storing file_num in char type
        sprintf(num_str, "%d", file_num);

        /* creating request to send */
        char buffer[256];
        strcpy (buffer,"files/foo");
        // strcpy (buffer,"files/foo");
        strcat (buffer,num_str);
        strcat (buffer,".txt");

        /* sending request to server */
        n = write(sockfd,buffer,strlen(buffer));

        if (n < 0) {
            clock_gettime(CLOCK_REALTIME, &threadEnd);
            close(sockfd);
            printf("Error connecting\n");
            if ((threadEnd.tv_sec - threadStart.tv_sec) < data->tot_time) {continue;}
            error("ERROR writing to socket");
        }

        clock_gettime(CLOCK_REALTIME, &requestStart); // to find time of response
        
        /* read reply from server */
        // printf("Start read %d\n",data->tid);
        for ( ; ; ) {
            char buff[3000];
            n = read(sockfd,buff,2048);

            if (n < 0) {printf("Error reading\n"); break;error("ERROR reading from socket");}
            // here you can save the file
            // Note : remove first char from buff (which is '1' and part of protocol; explained in readMe.txt)
            // printf("%.1s",buff);
            
            /* reading last packet from socket */
            if (n == 0 || buff[0] == '0'){

                clock_gettime(CLOCK_REALTIME, &requestEnd);

                // printf("sleeping ... %d\n", data->tid); 
                sleep(data->sleep_time);
                // printf("....  woke up %d\n",data->tid);
                
                /* updating statistics */
                data->success++;
                double resnano = (requestEnd.tv_nsec - requestStart.tv_nsec)/BILLION;
                data->response += (requestEnd.tv_sec - requestStart.tv_sec + resnano);

                break; 
            }
        }
        // printf("End read %d\n",data->tid);

        /* closing socket */
        close(sockfd);
        clock_gettime(CLOCK_REALTIME, &threadEnd);
    }
    pthread_exit(NULL);
}




/*****************************************************************************************
                start of main function
*****************************************************************************************/


int main(int argc, char *argv[])
{
    int users,tot_time;
    double success=0.0; // success is total number of successful file recieved
    double response=0.0; // response is total response time

    if (argc < 7) {
       fprintf(stderr,"usage %s hostname port users expTime sleepTime fileType\n", argv[0]);
       exit(0);
    }

    users = atoi(argv[3]);
    tot_time = atoi(argv[4]);

    /* creating thread data */
    int i, rc; // i, rc are temporary variables
   
    pthread_t thr[users]; // creating thread object
    thread_data thr_data[users]; // creating thread data

    for(i=0; i<users; i++){ thr_data[i] = *(cnst(argv)); } // initializing thread data
    
    /* create threads */
    for (i = 0; i < users; i++) {
        thr_data[i].tid = i;
        if ((rc = pthread_create(&thr[i], NULL, thr_func, &thr_data[i]))) {
            fprintf(stderr, "error: pthread_create, error: %d\n", rc);
            return EXIT_FAILURE;
        }
    }

    /* block until all threads complete */
    for (i = 0; i < users; i++) { pthread_join(thr[i], NULL); }

    /* calculating statistics */
    for (i = 0; i < users; i++) {
        response += thr_data[i].response;
        success += thr_data[i].success;
    }

    printf("Done!\n");
    printf("throughput = %.3f req/s\n", success/tot_time);
    printf("average response time = %.3f sec\n", response/success);
    // printf("%.3f %.3f\n", success/tot_time, response/success);
    return 0;
}
