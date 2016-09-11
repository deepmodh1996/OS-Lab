#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <set>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int j_sq;	// to couter SIGINT while getsq is running

void sig_handler(int signo)
{
	if (signo == SIGINT) {
		printf("Recieved SIGINT ... Press enter to get back to shell prompt\n");
		j_sq = -100;
	}
}


/* given code snippet for string parsing */
char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for(i =0; i < strlen(line); i++){

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0){
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0; 
			}
		} else {
			token[tokenIndex++] = readChar;
		}
	}
 
	free(token);
	tokens[tokenNo] = NULL ;
	return tokens;
}



int main(void)
{
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		printf("Can't catch SIGINT\n");
	}

	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
	std::set<pid_t> bg_pid;
	char server_ip[100], port_no[10];
	// strcpy(server_ip, "localhost");strcpy(port_no, "5000");


	/* waiting to listen from user */
	while (1) {

		/* reaping finished child process */
		int child_killed = waitpid((pid_t)(-1), 0, WNOHANG);
        while (child_killed > 0) {
        	printf(" ... background download process pid-%d finished\n", child_killed); 
        	bg_pid.erase(child_killed);
        	child_killed = waitpid((pid_t)(-1), 0, WNOHANG);
        }

		printf("Hello> ");			 	// printing prompt message
		bzero(line, MAX_INPUT_SIZE); 	// erasing previous stored line
		gets(line); 					// taking one line as input
		line[strlen(line)] = '\n';		//terminate with new line
		tokens = tokenize(line);


		/* checking different cases of input command */
		if (tokens[0] != NULL){ 	// check if only enter is pressed

		/* exit */
			if (strcmp(tokens[0],"exit") == 0){
				/* killing all background process before exit*/
				while (!bg_pid.empty()){
					if(kill(*bg_pid.begin(), SIGKILL) == 0) { printf("Backround process pid-%d killed\n", *bg_pid.begin());}
					else {printf("Cannot kill backround process pid-%d\n", *bg_pid.begin());}
					bg_pid.erase(bg_pid.begin());
				}
				//printf("Bye!\n");
				return 0;

		/* cd */
			} else if (strcmp(tokens[0],"cd") == 0) {
				/*cd implimentation*/
				if (tokens[1]!=NULL && tokens[2]==NULL) {
					if (chdir(tokens[1]) != 0) {printf("some error occured executing %s\n", tokens[0]);}
				} else {printf("Incorrect usage\nUse as: cd directoryname\n");}


		/* server */
			} else if (strcmp(tokens[0],"server") == 0) {
				/* modifying server data */
				if (tokens[1]!=NULL && tokens[2]!=NULL && tokens[3]==NULL) { 
					bzero(server_ip,100); bzero(port_no,10);
					strcpy(server_ip, tokens[1]);strcpy(port_no, tokens[2]);
				} else {printf("Incorrect usage\nUse as: server server-IP server-port\n");}


		/* getfl */
			} else if (strcmp(tokens[0],"getfl") == 0) {
				/* checking : getfl must have atleast one filename */
				if (tokens[1]!=NULL) {
					
					/* checking existance of server data */
					if (strcmp(server_ip,"") == 0) {printf("no serverIP provided\n");continue;}
					if (strcmp(port_no,"") == 0) {printf("no port no provided\n");continue;}

				/* getfl > */
					/* checking type of getfl */
					if (tokens[2]!=NULL) {
						if (strcmp(tokens[2],">") != 0 && strcmp(tokens[2],"|") != 0) {printf("Incorrect usage\nUse as: getfl filename\n");}
						else if (strcmp(tokens[2],">") == 0) {
							if (tokens[3]!=NULL && tokens[4]==NULL) {
								pid_t pid = fork();
								if(pid==0){
									// creating file with name of token[3]
									remove( tokens[3] );	// removing output file if already exists
									long long int fd = open(tokens[3],O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
									dup2(fd, STDOUT_FILENO); // redirecting output of child process to fd
									close(fd);
									char *cmd[] = {"./get-one-file-sig", tokens[1], server_ip, port_no, "display", NULL};
									execv("get-one-file-sig",cmd);
									return 0;
								}
								else{waitpid(pid,0,0);} // waiting for child process to terminate
							} else {printf("Incorrect usage\nUse as: getfl filename > outputfile\n");}

				/* getfl | */
						/* pipe handeling*/
						} else if (strcmp(tokens[2],"|") == 0) {
							char some_command[1024]; int j = 3;
							strcpy(some_command, "");
							while(tokens[j]!=NULL){
								strcat(some_command, " ");strcat(some_command, tokens[j]);j++;
							}
							int fd[2];
							pipe(fd); // creating pipe
							pid_t pid, pid_pipe;
							pid = fork();
							if(pid==0){ // creating child
								dup2(fd[1],STDOUT_FILENO);
								close(fd[0]); // closing unrequired connection of pipe
								char *cmd[] = {"./get-one-file-sig", tokens[1], server_ip, port_no, "display", NULL};
								execv("get-one-file-sig",cmd);
								return 0;
							}
							pid_pipe = fork();
							if(pid_pipe==0) {
								dup2(fd[0], STDIN_FILENO); // redirecting input of child 2
								close(fd[1]); // closing unrequired sides
								char *temp_tokens[MAX_NUM_TOKENS];int i_temp = 3;
								/* creating array of arguments after pipe */
								while (tokens[i_temp] != NULL){ 
									temp_tokens[i_temp-3] = tokens[i_temp]; i_temp++;
								}temp_tokens[i_temp-3] = NULL;
								/* executing command after pipe */
								execvp(temp_tokens[0],temp_tokens); 
								return 0;
							}
							waitpid(pid,0,0); // waiting for child process to terminate
							/* closing pipe file descriptors in parent */
							close(fd[0]);close(fd[1]); 
							waitpid(pid_pipe,0,0); // waiting for child process to terminate
						} else {printf("Incorrect usage\nUse as: getfl filename\n");}

				/* simple getfl */	
					} else {
						pid_t pid = fork();
						if(pid==0){
							char *cmd[] = {"./get-one-file-sig", tokens[1], server_ip, port_no, "display", NULL};
							execv("get-one-file-sig",cmd);
							return 0;
						}
						else{waitpid(pid,0,0);} // waiting for child process to terminate
					}
				} else {printf("Incorrect usage\nUse as: getfl filename\n");}


		/* getsq */
			} else if (strcmp(tokens[0],"getsq") == 0) {
				/* 	j_sq is used to handle ctrl + c
				 	j_sq is set to -100 when SIGINT is recieved */
				if (tokens[1]!=NULL) { j_sq = 1;
					while(tokens[j_sq]!= NULL && j_sq > 0) { 
						pid_t pid = fork();
						if(pid==0){
							char *cmd[] = {"./get-one-file-sig", tokens[j_sq], server_ip, port_no, "nodisplay", NULL};
							execv("get-one-file-sig",cmd);
							return 0;
						}
						else{waitpid(pid,0,0);} // waiting for child process to terminate
						j_sq++;
					}
				} else {printf("Incorrect usage, give atleast one file.\nUse as: getsq file1 file2 file3 ...\n");}


		/* getpl */
			} else if (strcmp(tokens[0],"getpl") == 0) {
				if (tokens[1]!=NULL) {
					/* storing opened pid of child in set_pid*/
					std::set<pid_t> set_pid;
					for(int i = 1;tokens[i]!=NULL;i++){
						pid_t pid = fork();
						if(pid==0){
							char *cmd[] = {"./get-one-file-sig", tokens[i], server_ip, port_no, "nodisplay", NULL};
							execv("get-one-file-sig",cmd);
							return 0;
						}
						else{set_pid.insert(pid);}
					}
					/* repeaing all child processes created */
					std::set<pid_t>::iterator it;
					for(it=set_pid.begin();it!=set_pid.end();++it){waitpid((*it),0,0);} // waiting for child process to terminate
					set_pid.clear();
				} else {printf("Incorrect usage\nUse as: getpl file1 file2 file3 ...\n");}


		/* getbg */
			} else if (strcmp(tokens[0],"getbg") == 0) {
				if (tokens[1]!=NULL && tokens[2]==NULL) {
					pid_t pid = fork();
					if(pid==0){
						/* get-one-file does not teminate on ctrl + c */
						char *cmd[] = {"./get-one-file", tokens[1], server_ip, port_no, "nodisplay", NULL};
						execv("get-one-file",cmd);
						return 0;
					}
					else{ bg_pid.insert(pid); }
				} else {printf("Incorrect usage\nUse as: getbg filename\n");}


		/* simple linux commands */	
			} else {
				pid_t pid = fork();
				if(pid==0){
					if(execvp(tokens[0],tokens)==-1) {
						printf("%s\n", strerror(errno));
					}
					return 0;
				}
				else{waitpid(pid,0,0);} // waiting for child process to terminate
			}
		}

		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}