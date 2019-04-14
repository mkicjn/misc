#include <stdio.h>
#include <stdlib.h>
#include <time.h>
void swap(int *a,int *b)
{
	int c=*a;
	*a=*b;
	*b=c;
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
void heapsort(int *h,int n)
{
	for (int i=(n-2)>>1;i>=0;i--)
		siftdown(h,n,i);
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
void benchmark(int *a,int n,void (*f)(int *,int),const char *s)
{
	int *t=malloc(n*sizeof(int));
	for (int i=0;i<n;i++)
		t[i]=a[i];
	clock_t dt=clock();
	f(t,n);
	dt=clock()-dt;
	for (int i=1;i<n;i++)
		if (t[i-1]>t[i]) {
			printf("(failed) ");
			break;
		}
	printf("%s: %fms\n",s,1000.0*dt/CLOCKS_PER_SEC);
	free(t);
}
int main(int argc,char **argv)
{
	srand(time(NULL));
	int n=100;
	if (argc>1)
		sscanf(argv[1],"%d",&n);
	int *a=malloc(n*sizeof(int));
	for (int i=0;i<n;i++)
		a[i]=rand()%n;
	benchmark(a,n,&heapsort,"Heapsort");
	benchmark(a,n,&quicksort,"Quicksort");
	benchmark(a,n,&insertion_sort,"Insertion sort");
	free(a);
	return 0;
}
