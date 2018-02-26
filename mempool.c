#include<stdio.h>
#include<stddef.h>
#include<stddef.h>
#include<pthread.h>
#include<stdlib.h>
char memory[2000];


#define MAX_TREADS 10

int allocated = 0;			//flag used for a resumed thread again for asking memory
int free_called = 0;			// its set whenever My_Free() is called to inform waiting threads

static int count =0;			

pthread_once_t foo_once = PTHREAD_ONCE_INIT;
pthread_mutex_t lock; 			//=PTHREAD_MUTEX_INITIALIZER;

void foo_init()
{
    pthread_mutex_init(&lock,NULL);

}


/*Metadata of each block allocated or deallocated*/
struct block{
	size_t size; 
	int free;  /*This flag is used to know whether the block described by /
 the metadata structure is free or not -- > if free, it is set to 1. Otherwise 0.*/
	
	struct block *next; /*Points to the next metadata block*/
};


/*Pointing to the main block of memory which is initially free (no storage allocation yet)*/
struct block *freeList=(void*)memory;

void initialize();
void split(struct block *fitting_slot,size_t size);
void *MyMalloc(size_t noOfBytes);
void merge();
void MyFree(void* ptr);

//mythread_malloc
static void *t_malloc(void *arg) {

	pthread_mutex_lock(&lock);
	int sz = *((int *) arg);
	void *myptr;

	printf("size is %d\n",sz);
	myptr=MyMalloc(sz);

	pthread_mutex_unlock(&lock);

	return myptr;
}


/*Initializing the block of memory*/
void initialize(){
	freeList->size=2000-sizeof(struct block); 
	freeList->free=1;
	freeList->next=NULL;
}


/*Making way for a new block allocation by splitting a free block -- (Assume first fit algorithm)*/
void split(struct block *fitting_slot,size_t size){
	struct block *new=(void*)((void*)fitting_slot+size+sizeof(struct block));
	new->size=(fitting_slot->size)-size-sizeof(struct block);
	new->free=1;
	new->next=fitting_slot->next;
	fitting_slot->size=size;
	fitting_slot->free=0;
	fitting_slot->next=new;
}

/*Function MyMalloc(malloc)*/
void *MyMalloc(size_t noOfBytes){

	struct block *curr,*prev;
	void *result;
	allocated = 0;

do
{
	if(!(freeList->size))
	{ 
	initialize();
	printf("Memory initialized\n");
	}

	curr=freeList;

	while((((curr->size)<noOfBytes)||((curr->free)==0))&&(curr->next!=NULL))
	{
	prev=curr;
	curr=curr->next;
	}

	if((curr->size)==noOfBytes)
	{
	curr->free=0;
	result=(void*)(++curr);
	printf("Block allocated without splitting\n");

	allocated = 1;
	return result;
	
	}-


 else if((curr->size)>(noOfBytes+sizeof(struct block)))
	{

	split(curr,noOfBytes);
	result=(void*)(++curr);
	printf("Block allocated with splitting\n");
	allocated = 1;
	return result;
	
	}

else	{
	result=NULL;
	printf("\nI'm thread %d \nNo sufficient memory to allocate for me\n");
	free_called = 0;

	pthread_mutex_unlock(&lock);		//let other threads use the lock

	//*making a thread wait*************
	while(free_called == 0)
		continue;
	//********************************

	printf("\n\nI'm thread %d allocating for size %d and my waiting is finally over\n\n",pthread_self(),noOfBytes);
	pthread_mutex_lock(&lock);
	allocated = 0;

	}

}while(allocated == 0);

}


/*This is to merge the consecutive free blocks by removing the metadata block in the middle. This will save space.*/
void merge(){
	struct block *curr,*prev;
	curr=freeList;
	
	while((curr->next)!=NULL){

	if((curr->free) && (curr->next->free))
	{
	curr->size+=(curr->next->size)+sizeof(struct block);
	curr->next=curr->next->next;
	}

	prev=curr;
	curr=curr->next;
	}
}


/*Function MyFree(free)*/
void MyFree(void* ptr){

	if(((void*)memory<=ptr)&&(ptr<=(void*)(memory+2000)))
	{
	
	printf("\nIm in free block\n");
	struct block* curr=ptr;
	//--curr;
	curr->free=1;
	merge();

	printf("\nfreed\n");
	free_called = 1;

	}
	else 
	printf("Please provide a valid pointer allocated by MyMalloc\n");

}


//block wise contents
void status()
{
	struct block* curr=freeList;
	printf("\nBlock address\tSize  \t Is_It_Free\n\n");
	printf("********************************************************************");
	while(curr->next!=NULL)
	{

	printf("\n %p\t %d \t %d \n",curr,curr->size,(int)curr->free);

	curr=curr->next;

	}

}


int main(int argc, char const *argv[])
{

int ch,blk;
pthread_t thread_id[MAX_TREADS];
pthread_once(&foo_once, foo_init);
 
pthread_mutex_t console_lock;


void *p[10];				
int size=500,i;
int *arg = malloc(sizeof(*arg));
*arg=size;


do
{
printf("\n***Menu***\n");
printf("\n1 : Add a block");
printf("\n2 : Free a block");
printf("\n3 : Check Block-wise Status ");
printf("\n4 : Exit\n\nEnter a choice : "); 
scanf("%d",&ch);

switch(ch)
{
case 1:    
	printf("\n\n************\n\nEnter size of block %d :",count);
	scanf("%d",arg);

	pthread_create(&thread_id[count], NULL, t_malloc, arg);

  	count++;
	printf("\n\ncount %d",count);

	break;

case 2:
	printf("\n\nEnter the block size to be freed\n");
	scanf("%d",&blk);

	struct block* curr=freeList;

	while(curr->next != NULL)
	{
	if(curr->size == blk)
	{

	MyFree(curr);
	break;

	}	
	curr=curr->next;
	
	}
	
	break;

case 3:	
	status();
	break;

case 4 :
	printf("\nExitting\n");
	return 0;

default :
	printf("Wrong choice");
	break;
	
}
}
while(ch!=4);

for(i=0;i<count;i++)
	pthread_join(thread_id[i],&p[i]);

pthread_mutex_destroy(&lock);

return 0;
}
