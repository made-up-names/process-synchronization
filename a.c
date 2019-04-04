#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
int main()
{
	printf("10\n");
	for(int i=0;i<10;i++)
		printf("%d ",rand()%1000);
	return 0;
}
