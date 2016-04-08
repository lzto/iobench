/*
 * IO benchmark
 * producer consumer model
 * 2016 Tong Zhang <ztong@vt.edu>
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define PAGE_SIZE 4096
#define NUM_OF_THREAD 8
#define BUF_CNT (NUM_OF_THREAD)
#define BUFFER_SIZE (PAGE_SIZE)

#define LOOP_CNT (1024 << 6)



//global big lock
pthread_mutex_t global_lock;

//buffer for each thread
char buffer[BUF_CNT][BUFFER_SIZE];
//pointer for each buffer
int bp[BUF_CNT];
//lock for each buffer pointer
pthread_mutex_t lock[BUF_CNT];


//running thread
int running = 0;

//producer fill the buffer
void* producer(void* arg)
{
	static struct timeval time_r_start;
	static struct timeval time_r_end;

	int p = (int)(0xff & (long)(arg));
	unsigned long q = LOOP_CNT;

	gettimeofday(&time_r_start,NULL);

	while(q>0)
	{
		//fill the buffer
		pthread_mutex_lock(&lock[p]);
		bp[p] = BUFFER_SIZE;

		buffer[p][0] = q;
		pthread_mutex_unlock(&lock[p]);
		//wait buffer empty
		//printf("W(%d):%lu\n",p,q);
		do
		{
			if(bp[p]==0)
			{
				break;
			}
		}while(1);
		q--;
	}
	gettimeofday(&time_r_end,NULL);
	double sec = ((double)(time_r_end.tv_sec - time_r_start.tv_sec)+
	 			 (time_r_end.tv_usec-time_r_start.tv_usec)/1000000.0);
	pthread_mutex_lock(&global_lock);
	running--;
	printf("Produced %d in %f secs\n", p, sec);
	pthread_mutex_unlock(&global_lock);

}


//consumer fetch data from the buffer and write into file
void* consumer()
{
	int i;
	FILE * fp = fopen("./test.data","w");
	while(running)
	{
		for(i=0;i<NUM_OF_THREAD;i++)
		{
			if(bp[i]!=BUFFER_SIZE)
			{
				continue;
			}
			if(pthread_mutex_trylock(&lock[i])!=0)
			{
				continue;
			}
			fwrite(buffer[i],BUFFER_SIZE,1,fp);
			//printf("-R(%d):%d\n", i, buffer[i][0]);
			bp[i] = 0;
			pthread_mutex_unlock(&lock[i]);
		}
	}
	printf("Consumed\n");
	fclose(fp);
	printf("Closed.\n");
}

int main()
{
	int i;
	static struct timeval time_r_start;
	static struct timeval time_r_end;

	int ts = (BUFFER_SIZE >> 10) * BUF_CNT * LOOP_CNT >> 20;

	printf("data set: %d GB\n", ts );

	pthread_mutex_init(&global_lock, NULL);

	pthread_t tid[NUM_OF_THREAD];

	gettimeofday(&time_r_start,NULL);
	for(i=0;i<NUM_OF_THREAD;i++)
	{
		pthread_mutex_init(&lock[i], NULL);
		pthread_create(&tid[i], NULL, producer, ((void*)(unsigned long)i));
		pthread_mutex_lock(&global_lock);
		running++;
		pthread_mutex_unlock(&global_lock);
	}

	consumer();

	for(i<0;i<NUM_OF_THREAD;i++)
	{
	
		pthread_join(tid[i], NULL);
		pthread_mutex_destroy(&lock[i]);
	}
	gettimeofday(&time_r_end,NULL);
	double sec = ((double)(time_r_end.tv_sec - time_r_start.tv_sec)+
	 			 (time_r_end.tv_usec-time_r_start.tv_usec)/1000000.0);

	pthread_mutex_destroy(&global_lock);
	printf("Fini. %f sec\n", sec);
	printf("%.2f MB/s\n", ((double)ts)*1024/sec);
	return 0;
}

