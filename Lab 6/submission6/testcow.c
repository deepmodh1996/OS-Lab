#include "types.h"
#include "user.h"
#include "fcntl.h"

#define PGSIZE 1<<12

int arr[PGSIZE];
int main()
{
	printf(1,"#free-pages [in parent(before forks)]= %d\n",getnumfreepages());
	int pid=fork();
	if(pid==0){
		printf(1,"#free-pages [in child-1 (before)]= %d\n",getnumfreepages());
		//make small write
		pid=1;
		printf(1,"#free-pages [in child-1 (after)]= %d\n",getnumfreepages());
		sleep(5);
		exit();
	}
	pid=fork();
	if(pid==0){
		printf(1,"#free-pages [in child-2 (before)]= %d\n",getnumfreepages());
		//make big writes causing more page faults
		memset(arr,0,sizeof(arr));
		printf(1,"#free-pages [in child-2 (after)]= %d\n",getnumfreepages());
		sleep(5);
		exit();
	}
	wait();
	wait();
	printf(1,"#free-pages [in parent(after all reaps)]= %d\n",getnumfreepages());
	exit();
}

/*
testcow.c forks two children. Note that a printf call will write to user stack and hence will itself
cause pagefault and pages are used in creating page tables also.
First child makes small update in 'pid' and does not cause new page to be allocated.
Second child makes a big update on array of size 4*PGSIZE hence causing 5 new pages to be
allocated.(4-5 pages can be spanned by the array)
Hence pages are allocateed only when process tries to write into them as required by CoW.

Output(on lenovo-g500-i3 2-core 64-bit system):
#free-pages [in parent(before forks)]= 56771
#free-pages [in child-1 (before)]= 56635
#free-pages [in child-1 (after)]= 56635
#free-pages [in child-2 (before)]= 56635
#free-pages [in child-2 (after)]= 56630
#free-pages [in parent(after all reaps)]= 56771
*/