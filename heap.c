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
int main(int argc,char **argv)
{
	int size=100;
	if (argc>1)
		sscanf(argv[1],"%d",&size);
	srand(time(NULL));
	int *arrh,*arri;
	arrh=malloc(size*sizeof(int));
	arri=malloc(size*sizeof(int));
	/**/
	for (int i=0;i<size;i++) {
		int n=rand()%1000;
		arrh[i]=n;
		arri[i]=n;
	}
	puts("Random distribution:");
	clock_t h=clock();
	heapsort(arrh,size);
	h=clock()-h;
	printf("Heapsort took %fms\n",1000.0*h/CLOCKS_PER_SEC);
	clock_t i=clock();
	insertion_sort(arri,size);
	i=clock()-i;
	printf("Insertion sort took %fms\n",1000.0*i/CLOCKS_PER_SEC);
	putchar('\n');
	/* Lazy copy paste */
	for (int i=0;i<size;i++) {
		int n=size-i;
		arrh[i]=n;
		arri[i]=n;
	}
	puts("Reverse order:");
	h=clock();
	heapsort(arrh,size);
	h=clock()-h;
	printf("Heapsort took %fms\n",1000.0*h/CLOCKS_PER_SEC);
	i=clock();
	insertion_sort(arri,size);
	i=clock()-i;
	printf("Insertion sort took %fms\n",1000.0*i/CLOCKS_PER_SEC);
	putchar('\n');
	/* Lazy copy paste */
	for (int i=0;i<size;i++) {
		int n=i+1;
		arrh[i]=n;
		arri[i]=n;
	}
	for (int i=0;i<size/100;i++) {
		int n1=size/100+rand()%(size-size/50);
		int n2=n1-size/100+rand()%(size/50);
		swap(&arrh[n1],&arrh[n2]);
		swap(&arri[n1],&arri[n2]);
	}
	puts("Mostly sorted:");
	h=clock();
	heapsort(arrh,size);
	h=clock()-h;
	printf("Heapsort took %fms\n",1000.0*h/CLOCKS_PER_SEC);
	i=clock();
	insertion_sort(arri,size);
	i=clock()-i;
	printf("Insertion sort took %fms\n",1000.0*i/CLOCKS_PER_SEC);
	putchar('\n');
	/**/
	free(arrh);
	free(arri);
	return 0;
}
