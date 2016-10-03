#include "types.h"
#include "stat.h"
#include "user.h"

#define PI 3.14159265619619

int
main(void)
{
 printf(1, "Testing schedular for CPU-Bound processes \n");
 int ret = fork();
 if(ret==0){ // child 1 with default priority i.e. 0(of initproc)

 	printf(1, "Child 1 has priority %d \n", getprio());
	int tim = uptime();
 	// a cpu bound process
 	int count2 = 20;
 	float temp2 = 0;
	float temp = 51;
 	while(count2--){
	 	int count = 0;
	 	while(count<1000){
			int loop1 = 100;
			while(loop1--){temp /= PI;}
			int loop2 = 10000;
			while(loop2--){temp *= PI;}
			count++;
			temp2 += temp;
		}
		temp2 -= temp;
		temp++;
		// printf(1,"%d - 1\n", 9-count2);
	}
	float temp3 = temp2;
 	
 	printf(1, "Child 1 runs for %d (%d) \n", uptime()-tim, temp3);

 	exit();
 }
 else{
 	ret = fork();
 	if(ret==0){ // child 2 with priority 2
		
		if(setprio(2) < 0){
	 		printf(1, "Error in setting priority of child 1 \n");
		}

 		printf(1, "Child 2 has priority %d \n", getprio());
 		int tim = uptime();
	 	// 
	 	int count2 = 20;
	 	float temp2 = 0;
 		float temp = 51;
	 	while(count2--){
		 	int count = 0;
		 	while(count<1000){
				int loop1 = 100;
				while(loop1--){temp /= PI;}
				int loop2 = 10000;
				while(loop2--){temp *= PI;}
				count++;
				temp2 += temp;
			}
			temp2 -= temp;
			temp++;
			// printf(1,"%d - 2\n", 9-count2);
		}
		float temp3 = temp2;
	 	
	 	printf(1, "Child 2 runs for %d (%d) \n", uptime()-tim, temp3);

	 	exit();

 	}else{
 	ret = fork();
 	if(ret==0){ // child 3 with priority 1

	 	if(setprio(1) < 0){
	 		printf(1, "Error in setting priority of child 3 \n");
	 	}

 		printf(1, "Child 3 has priority %d \n", getprio());
 		int tim = uptime();
	 	// 
	 	int count2 = 20;
	 	float temp2 = 0;
 		float temp = 51;
	 	while(count2--){
		 	int count = 0;
		 	while(count<1000){
				int loop1 = 100;
				while(loop1--){temp /= PI;}
				int loop2 = 10000;
				while(loop2--){temp *= PI;}
				count++;
				temp2 += temp;
			}
			temp2 -= temp;
			temp++;
			// printf(1,"%d - 3\n", 9-count2);
		}
		float temp3 = temp2;
	 	
	 	printf(1, "Child 3 runs for %d (%d) \n", uptime()-tim, temp3);

	 	exit();

 	}else{
 		if( wait() < 0 ){printf(1, "Error in killing child \n");}
 		if( wait() < 0 ){printf(1, "Error in killing child \n");}
 		if( wait() < 0 ){printf(1, "Error in killing child \n");}
 		// waiting for all the CPU bound processes to finish

 		printf(1, "Testing schedular for IO-Bound processes \n");

 		char buf[20] = "abcdefghijklmnopq";

 		ret = fork();
 		if(ret == 0){
 			if(setprio(10) < 0){
		 		printf(1, "Error in setting priority of child 1 \n");
		 	}

 			printf(1, "Child 1 has priority %d \n", getprio());
		 	int tim = uptime();
 			int fd = open("foo1.txt", 1|0x200);
 			int numWrite = 2;
 			while( numWrite--){
 				int numi = 200;
 				while(numi--){write(fd,buf,10);	}
 			}
 			close(fd);
 			printf(1, "Child 1 runs for %d \n", uptime()-tim);
	 		exit();
 		}
 		else{
 			ret = fork();
 			if(ret == 0){
 				printf(1, "Child 2 has priority %d \n", getprio());
			 	int tim = uptime();
	 			int fd = open("foo2.txt", 2|0x200);
	 			int numWrite = 2;
	 			while( numWrite--){
	 				int numi = 200;
	 				while(numi--){write(fd,buf,10);	}
	 			}
	 			printf(1, "Child 2 runs for %d \n", uptime()-tim);
	 			close(fd);
	 			exit();
 			}
 			else{

 				if( wait() < 0 ){printf(1, "Error in killing child \n");}
 				if( wait() < 0 ){printf(1, "Error in killing child \n");}
 				// wait for the IO bound processes
 			}
 		}

 	}	
 	}
 }
 exit();
}