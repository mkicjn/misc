#include <stdio.h>
#include <stdlib.h>
#include <time.h>
void swap(int *a,int *b)
{
	*a^=*b;
	*b^=*a;
	*a^=*b;
}
void siftup(int *h,int s,int i)
{
	while (i>0)
		if (h[i]>h[(i-1)>>1])
			swap(&h[i],&h[(i-1)>>1]);
}
void siftdown(int *h,int n,int i)
{
	while (i<n) {
		int l=(i<<1)+1,r=(i<<1)+2;
		if (l>=n)
			break;
		int s=r<n&&h[r]>=h[l]?r:l;
		if (h[i]<h[s])
			swap(&h[i],&h[s]);
		else
			break;
		i=s;
	}
}
void heapify(int *h,int n)
{
	for (int i=(n-2)>>1;i>=0;i--)
		siftdown(h,n,i);
}
void heapsort(int *h,int n)
{
	heapify(h,n);
	while (n>0) {
		swap(&h[0],&h[--n]);
		siftdown(h,n,0);
	}
}
void insertion_sort(int *a,int len)
{
	for (int i=0;i<len-1;i++)
		for (int j=i+1;j&&a[j]<a[j-1];j--)
			swap(&a[j],&a[j-1]);
}
void quicksort(int *a,int n)
{
	if (n<2)
		return;
	int p=0;
	for (int i=1;i<n;i++)
		if (a[i]<a[0])
			swap(&a[++p],&a[i]);
	swap(&a[0],&a[p]);
	quicksort(a,p);
	quicksort(&a[p+1],n-p-1);
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
