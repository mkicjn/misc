#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int bin_search(int s,int *arr,int l)
{
	// Assumes ascending order
	// Returns negative index if not found
	int min=0,max=l-1;
	while (min!=max-1) {
		int mid=(min+max)/2;
		if (s>arr[mid])
			min=mid;
		else if (s<arr[mid])
			max=mid;
		else
			return mid;
	}
	if (arr[min]==s)
		return min;
	return -min;
}
int main(int argc,char **argv)
{
	srand(time(NULL));
	static const int size=100;
	int arr[size];
	for (int i=0;i<size;i++)
		arr[i]=rand()%size;
	for (int i=0;i<size-1;i++)
		for (int j=i+1;j>0;j--)
			if (arr[j]<arr[j-1]) {
				int c=arr[j-1];
				arr[j-1]=arr[j];
				arr[j]=c;
			}
	for (int i=0;i<size;i++)
		printf("%d:%d ",i,arr[i]);
	putchar('\n');
	int s=rand()%100;
	printf("%d is at %d\n",s,bin_search(s,arr,size));
}
