#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static inline int parent(int i)
{
	return (i-1)>>1;
}
static inline int left_child(int i)
{
	return (i<<1)+1;
}
static inline int right_child(int i)
{
	return (i<<1)+2;
}
void swap(int *a,int *b)
{
	*a^=*b;
	*b^=*a;
	*a^=*b;
}
void sift_down(int *a,int start,int end)
{
	int r=start;
	while (left_child(r)<=end) {
		int c=left_child(r),s=r;
		if (a[s]<a[c])
			s=c;
		if (c+1<=end&&a[s]<a[c+1])
			s=c+1;
		if (s==r)
			return;
		else {
			swap(&a[r],&a[s]);
			r=s;
		}
	}
}
void heapify(int *a,int len)
{
	int start=parent(len-1);
	while (start>=0) {
		sift_down(a,start,len-1);
		start--;
	}
}
void heapsort(int *a,int len)
{
	heapify(a,len);
	int end=len-1;
	while (end>0) {
		swap(&a[0],&a[end]);
		end--;
		sift_down(a,0,end);
	}
}
void insertion_sort(int *a,int len)
{
	for (int i=0;i<len-1;i++)
		for (int j=i+1;j&&a[j]<a[j-1];j--)
			swap(&a[j],&a[j-1]);
}
void quicksort(int *arr,int len)
{
	if (len<2)
		return;
	int pivot=0;
	for (int i=1;i<len;i++)
		if (arr[i]<arr[0])
			swap(&arr[++pivot],&arr[i]);
	swap(&arr[0],&arr[pivot]);
	quicksort(arr,pivot);
	quicksort(&arr[pivot+1],len-pivot-1);
}
int main(int argc,char **argv)
{
	int size=100;
	if (argc>1)
		sscanf(argv[1],"%d",&size);
	srand(time(NULL));
	int *arrh,*arrq,*arri;
	arrh=malloc(size*sizeof(int));
	arrq=malloc(size*sizeof(int));
	arri=malloc(size*sizeof(int));
	/**/
	for (int i=0;i<size;i++) {
		int n=rand()%1000;
		arrh[i]=n;
		arrq[i]=n;
		arri[i]=n;
	}
	clock_t h=clock();
	heapsort(arrh,size);
	h=clock()-h;
	printf("Heapsort took %fms\n",1000.0*h/CLOCKS_PER_SEC);
	clock_t q=clock();
	quicksort(arrq,size);
	q=clock()-q;
	printf("Quicksort took %fms\n",1000.0*q/CLOCKS_PER_SEC);
	clock_t i=clock();
	insertion_sort(arri,size);
	i=clock()-i;
	printf("Insertion sort took %fms\n",1000.0*i/CLOCKS_PER_SEC);
	/**/
	free(arrh);
	free(arrq);
	free(arri);
	return 0;
}
