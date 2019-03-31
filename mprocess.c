#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
// First do with processes
// Next do with threads
// Write REPORT ? performance comparision with normal merge sort, proc1merge sort, thread1 merge sort ? how

key_t key=IPC_PRIVATE;
void mergesort(int* arr,int st1,int en2);
void merge(int* arr,int st1,int en2);
int main()
{
	int n;
	scanf("%d",&n);

	//create
	int shmid=shmget(key,n*sizeof(int),IPC_CREAT| 0666);
	if(shmid<0)
	{
		perror("shmget");
		_exit(1);
	}
	
	//attach
	int * arr=shmat(shmid,0,0);
	if(arr==(void*)-1)
	{
		perror("shmat");
		_exit(1);
	}

	for(int i=0;i<n;i++)
		scanf("%d",&arr[i]);

	mergesort(arr,0,n-1);

	for(int i=0;i<n;i++)
		printf("%d ",arr[i]);
	printf("\n");
	
	//detach
	int detacherror=shmdt(arr);
	if(detacherror<0)
	{
		perror("shmdt");
		_exit(1);
	}
	
	//delete
	int deleteerror=shmctl(shmid,IPC_RMID,NULL);
	if(deleteerror<0)
	{
		perror("shmctl");
		_exit(1);
	}

	return 0;
}
void mergesort(int* arr,int st1,int en2)
{
	int en1=(st1+en2)/2;
	int st2=en1+1;

	if(en2<=st1)
		return ;
	//if size < 5 do selection sort
	if(en2-st1<4)
	{
		//selection sort;
	
		for(int i=st1;i<=en2;i++)
		{
			for(int j=i+1;j<=en2;j++)
			{
				if(arr[j]<arr[i])
				{
					int temp=arr[i];
					arr[i]=arr[j];
					arr[j]=temp;
				}
			}

		}

		return ;
	}

	// type 1 // create 2 processes for left and right doing
	int left=fork();
	if(left==0)
	{
		mergesort(arr,st1,en1);
		exit(0);
	}
	else{
		mergesort(arr,st2,en2);
		int status;
		waitpid(left,&status,0);
		merge(arr,st1,en2);
	}
	
	return;
}
void merge(int* arr,int st1,int en2)
{
	int en1=(st1+en2)/2;
	int st2=en1+1;
	int i=st1;
	int j=st2;
	int arr2[en2-st1+1];
	for(int k=0;k<=en2-st1;)
	{
		if(i>en1)
		{
			arr2[k++]=arr[j++];
		}
		else if(j>en2)
		{
			arr2[k++]=arr[i++];
		}
		else
		{
			if(arr[j]<=arr[i])
				arr2[k++]=arr[j++];
			else
				arr2[k++]=arr[i++];
		}
	}
	for(int k=st1;k<=en2;k++)
	{
		arr[k]=arr2[k-st1];
	}
	
	return ;
}
