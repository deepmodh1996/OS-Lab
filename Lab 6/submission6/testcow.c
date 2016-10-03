#include "types.h"
#include "user.h"

int main(void)
{
	printf(1, "Parent %d\n", getNumFreePages());
	char str[1000];
	memset(str,'$',990);

	int ret = fork();
	if(ret==0){
		str[990-3] = '^';
		printf(1, "Child %d\n", getNumFreePages());
		// printf(1, "Child 1 second %d\n", getNumFreePages());
		exit();
	} else {
		ret = fork();
		if(ret==0){
			memset(str,'&',300);
			printf(1, "Child 2 %d\n", getNumFreePages());
			exit();
		} else {
	 		if( wait() < 0 ){printf(1, "Error in killing child \n");}
	 		if( wait() < 0 ){printf(1, "Error in killing child \n");}
		}
	}
	exit();
}