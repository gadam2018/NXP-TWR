// LINUX COMPILE: gcc app.c -o app -lrt  -lpthread    or?   gcc -Wall -w app.c -std=gnu99 -lrt  
// LINUX RUN: sudo ./app
#define _GNU_SOURCE // for affinity routines -> sched_setaffinity()
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
// #include <sys/time.h> ???
#if defined(_POSIX_MEMLOCK)
   #include <sys/mman.h>
#else
   #warning mlockall is not available
#endif 
#include <sched.h> // for affinity sched_getaffinity() and sched_setaffinity() routines, set_scheduler
#include <pthread.h>
#include <string.h>
#include <errno.h>

#define ARRAY_SIZE 100 // Options: 100 1000 10000 100000 1000000
#define MATRICE_SIZE 512 // Options: 512 1024 2048 4096 8192
#define THREADS_PAIRS 0 // Options: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15

int array_selsort[ARRAY_SIZE];
int array_quicksort[ARRAY_SIZE];
int array_matrixadd[MATRICE_SIZE];
int array_matrixmul[MATRICE_SIZE];

unsigned long long int sum;
unsigned long long int mul;

void *thread_selsort_function(void *);
void *thread_quicksort_function(void *);
void *thread_matrixadd_function(void *);
void *thread_matrixmul_function(void *);

// Setting process scheduling algorithm to RR with priority 1
set_process_scheduler()
{
	struct sched_param param;
	const struct sched_param priority = {99}; //1
	param.sched_priority = 99; //max_prio; //1
		
	if (sched_setscheduler(getpid(), SCHED_RR, &priority) == -1) //SCHED_RR
	    fprintf(stderr,"%s Error setting scheduler ... (root privileges needed)\n",strerror(errno));
    
	 printf("Scheduler priority is %d\n",param.sched_priority );
	 printf("RR scheduler max priority is:%d\n",sched_get_priority_max(SCHED_RR));
}

// Getting process scheduling algorithm
get_process_scheduler()
{
switch(sched_getscheduler(getpid()))
{
	case SCHED_OTHER: printf("PROCESS scheduler is SCHED_OTHER scheduler\n"); break; // ROUND ROBIN POLICY
	case SCHED_FIFO: printf("PROCESS scheduler is SCHED_FIFO scheduler\n"); break; 
	case SCHED_RR: printf("PROCESS scheduler is SCHED_RR scheduler\n"); break; 
	default:  printf("Not known scheduler\n"); break;
} 
} 

// Getting process affinity
void get_process_affinity()
{
char str[80];
int pid=getpid();
int count=0, k;
cpu_set_t myset;
CPU_ZERO(&myset);
sched_getaffinity(0,sizeof(myset),&myset); // 0 means current process
strcpy(str," ");
for(k=0; k<CPU_SETSIZE;++k)
{
	if(CPU_ISSET(k,&myset))
	{
		++count;
		char cpunum[3];
		sprintf(cpunum,"%d",k);
		strcat(str,cpunum);
	}
}
printf("\nThe process with pid %d has affinity %d CPUs ... %s\n",pid, count, str);
}

//SET PROCESS AFFINITY
void set_process_affinity() //ONLY TO FIRST CORE
{
	cpu_set_t myset;
	CPU_ZERO(&myset);
	CPU_SET(0,&myset);
	sched_setaffinity(0,sizeof(myset),&myset);
}

// Getting thread affinity
void get_thread_affinity()
{
int pid, k, count=0;
char str[80];
cpu_set_t myset;
CPU_ZERO(&myset);
pthread_getaffinity_np(pthread_self(),sizeof(myset),&myset);
pid=pthread_self();
strcpy(str," ");
for(k=0; k<CPU_SETSIZE;++k)
{
	if(CPU_ISSET(k,&myset))
	{
		++count;
		char cpunum[3];
		sprintf(cpunum,"%d",k);
		strcat(str,cpunum);
	}
}
printf("\nThe thread %d has affinity %d CPUs ... %s\n",pid, count, str);
}

// Setting thread affinity
void set_thread_affinity(int core)
{
cpu_set_t myset;
CPU_ZERO(&myset);
CPU_SET(core,&myset);
pthread_setaffinity_np(pthread_self(),sizeof(myset),&myset); 
}

// Selection sort
void selectionsort(int array[], int size){
int temp ;
int i ;
int j ;
int index_min ;
int min ;
for (i = 0; i < size; i++){
	min = array[i];
	index_min = i ;
	for (j = i; j < ARRAY_SIZE; j++){
		if (array[j] < min){
			min = array[j];
			index_min = j ;
		}
	}
	temp = array[i];
	array[i] = array[index_min];
	array[index_min] = temp;
}
return;
}

//Quicksort
void quicksort(int array[], int first, int last){
int hub,left,right;
int temp;
if (last > first){
	hub = array[first];
	left = first + 1;
	right = last+1;
	while (left < right)
		if (array[left] < hub)
		left++;
		else {
				right--;
				temp = array[left];
				array[left] = array[right];
				array[right] = temp;
			}
	left--;
	temp = array[first];
	array[first] = array[left];
	array[left] = temp;
	quicksort(array, first , left);
	quicksort(array, right , last);
	}
return;
}

//Matrix addition
void matrixadd(int array[], int dim){
	int i;
	sum=0;
	for (i = 0; i < dim; i++)
		sum=sum+array[i];		
}

//Matrix multiplication
void matrixmul(int array[], int dim){
	int i;
	mul=1;
	for (i = 0; i < dim; i++)
		mul=mul*array[i];		
}


int main()
{

pthread_t thread_id[32];
 
int i;
int tmp;
char debug_char = 'a';
int sortarray[ARRAY_SIZE];
int matrixarray[MATRICE_SIZE];

#if defined(_POSIX_MEMLOCK)
 if(mlockall(MCL_CURRENT | MCL_FUTURE)<0)// requires <sys/mman.h> keep pages in memory
   {perror("Could not lock process in memory"); exit(2);}
#else
   #warning mlockall is not available
#endif

// Setting process scheduling algorithm to RR with priority 1
set_process_scheduler();
get_process_scheduler();
//get_process_affinity();
set_process_affinity();
get_process_affinity();

for (i = 0 ; i < ARRAY_SIZE; i++)
	sortarray[i] = random() % 1000; // creates a random number between 0 and 999
for (i = 0 ; i < MATRICE_SIZE; i++)
	matrixarray[i] = random() % 1000; // creates a random number between 0 and 999

for (i = 0; i < ARRAY_SIZE; i++) array_selsort[i] = sortarray[i];
for (i = 0; i < ARRAY_SIZE; i++) array_quicksort[i] = sortarray[i];
for (i = 0; i < MATRICE_SIZE; i++) array_matrixadd[i] = matrixarray[i];
for (i = 0; i < MATRICE_SIZE; i++) array_matrixmul[i] = matrixarray[i];

printf("\n UnSorted data:\n"); for (i = 0; i < ARRAY_SIZE; i++) printf("%d ",sortarray[i]);
printf("\n Raw matrix data:\n"); for (i = 0; i < MATRICE_SIZE; i++) printf("%d ",matrixarray[i]);

for (i=0; i<=THREADS_PAIRS; i++) // use 0 for 1 pair of 2 threads and 15 for 16 pairs, i.e. 2 up to 32 threads
{
pthread_create( &thread_id[i], NULL, thread_selsort_function, (void *) sortarray); // selection sort thread
pthread_create( &thread_id[++i], NULL, thread_quicksort_function, (void *) matrixarray); // quick sort thread
pthread_join(thread_id[i], NULL); pthread_join(thread_id[++i], NULL);
}

for (i=0; i<=THREADS_PAIRS; i++) // use 0 for 1 pair of 2 threads and 15 for 16 pairs, i.e. 2 up to 32 threads
{
pthread_create( &thread_id[i], NULL, thread_matrixadd_function, (void *) matrixarray); // matrix addition thread 
pthread_create( &thread_id[++i], NULL, thread_matrixmul_function, (void *) matrixarray); // matrix multiplication thread
pthread_join(thread_id[i], NULL); pthread_join(thread_id[++i], NULL);
}

printf("\n");
return 0;
}

// Selection sort thread function
void *thread_selsort_function(void *array2){
int i;
struct timeval tv;
struct timespec tms, ts_diff, ts_begin, ts_end;

#if defined(_POSIX_MEMLOCK)
 if(mlockall(MCL_CURRENT | MCL_FUTURE)<0)  // requires <sys/mman.h>
   {perror("Could not lock thread in memory"); exit(2);}
#else
   #warning mlockall is not available
#endif

//Setting THREAD scheduling algorithm to RR and priority 99
struct sched_param the_priority;
the_priority.sched_priority = 99; //1
pthread_setschedparam(pthread_self(), SCHED_RR, &the_priority);

//Setting THREAD AFFINITY
//get_thread_affinity();
set_thread_affinity(0);
get_thread_affinity();

clock_gettime(CLOCK_MONOTONIC, &tms); ts_begin=tms;
selectionsort(array_selsort, ARRAY_SIZE);
clock_gettime(CLOCK_MONOTONIC, &tms); ts_end=tms;

ts_diff.tv_nsec=ts_end.tv_nsec -ts_begin.tv_nsec; 
printf("\n Selection Sort: Time elapsed in secs = %.06f in msecs = %.06f and in usecs = %.03f", ts_diff.tv_nsec/1000000000.0, ts_diff.tv_nsec/1000000.0, ts_diff.tv_nsec/1000.0);

printf("\nSelection sort Sorted:\n"); for (i = 0; i < ARRAY_SIZE; i++) printf("%d ",array_selsort[i]); printf("\n");

}

// Quick sort thread function
void *thread_quicksort_function(void *array2){
int i;
struct timeval tv;
struct timespec tms, ts_diff, ts_begin, ts_end;

#if defined(_POSIX_MEMLOCK)
 if(mlockall(MCL_CURRENT | MCL_FUTURE)<0)  // requires <sys/mman.h>
   {perror("Could not lock thread in memory"); exit(2);}
#else
   #warning mlockall is not available
#endif

//Setting THREAD scheduling algorithm to RR and priority 99
struct sched_param the_priority;
the_priority.sched_priority = 99; //1
pthread_setschedparam(pthread_self(), SCHED_RR, &the_priority);

//Setting THREAD AFFINITY
//get_thread_affinity();
set_thread_affinity(0);
get_thread_affinity();

clock_gettime(CLOCK_MONOTONIC, &tms); ts_begin=tms;
selectionsort(array_quicksort, ARRAY_SIZE);
clock_gettime(CLOCK_MONOTONIC, &tms); ts_end=tms;

ts_diff.tv_nsec=ts_end.tv_nsec -ts_begin.tv_nsec; 
printf("\n Quicksort: Time elapsed in secs = %.06f in msecs = %.06f and in usecs = %.03f", ts_diff.tv_nsec/1000000000.0, ts_diff.tv_nsec/1000000.0, ts_diff.tv_nsec/1000.0);

printf("\nQuick sort Sorted:\n"); for (i = 0; i < ARRAY_SIZE; i++) printf("%d ",array_quicksort[i]); printf("\n");

}

// Matrix addition thread function
void *thread_matrixadd_function(void *array2){
int i;
struct timeval tv;
struct timespec tms, ts_diff, ts_begin, ts_end;

#if defined(_POSIX_MEMLOCK)
 if(mlockall(MCL_CURRENT | MCL_FUTURE)<0)  // requires <sys/mman.h>
   {perror("Could not lock thread in memory"); exit(2);}
#else
   #warning mlockall is not available
#endif

//Setting THREAD scheduling algorithm to RR and priority 99
struct sched_param the_priority;
the_priority.sched_priority = 99; //1
pthread_setschedparam(pthread_self(), SCHED_RR, &the_priority);

//Setting THREAD AFFINITY
//get_thread_affinity();
set_thread_affinity(0);
get_thread_affinity();

clock_gettime(CLOCK_MONOTONIC, &tms); ts_begin=tms; //CLOCK_REALTIME
matrixadd(array_matrixadd, MATRICE_SIZE);
clock_gettime(CLOCK_MONOTONIC, &tms); ts_end=tms;

ts_diff.tv_nsec=ts_end.tv_nsec -ts_begin.tv_nsec; 
printf("\n Matrix add: Time elapsed in secs = %.06f in msecs = %.06f and in usecs = %.03f", ts_diff.tv_nsec/1000000000.0, ts_diff.tv_nsec/1000000.0, ts_diff.tv_nsec/1000.0);

printf("\n Matrix addition result is %llu\n", sum);

}

// Matrix multiplication thread function
void *thread_matrixmul_function(void *array2){
	int i;
struct timeval tv;
struct timespec tms, ts_diff, ts_begin, ts_end;

#if defined(_POSIX_MEMLOCK)
 if(mlockall(MCL_CURRENT | MCL_FUTURE)<0)  // requires <sys/mman.h>
   {perror("Could not lock thread in memory"); exit(2);}
#else
   #warning mlockall is not available
#endif

//Setting THREAD scheduling algorithm to RR and priority 99
struct sched_param the_priority;
the_priority.sched_priority = 99; //1
pthread_setschedparam(pthread_self(), SCHED_RR, &the_priority);

//Setting THREAD AFFINITY
//get_thread_affinity();
set_thread_affinity(0);
get_thread_affinity();

clock_gettime(CLOCK_MONOTONIC, &tms); ts_begin=tms; //CLOCK_REALTIME
matrixmul(array_matrixmul, MATRICE_SIZE);
clock_gettime(CLOCK_MONOTONIC, &tms); ts_end=tms;

ts_diff.tv_nsec=ts_end.tv_nsec -ts_begin.tv_nsec; 
printf("\n Matrix multiplication: Time elapsed in secs = %.06f in msecs = %.06f and in usecs = %.03f", ts_diff.tv_nsec/1000000000.0, ts_diff.tv_nsec/1000000.0, ts_diff.tv_nsec/1000.0);

printf("\n Matrix multiplication result is %llu\n", mul);
}
