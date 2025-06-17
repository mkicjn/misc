//$(which tcc) $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static inline void swap(int *a,int *b)
{
	int c=*a;
	*a=*b;
	*b=c;
}
#define PARENT(x)  (((x) - 1) >> 1)
#define LEFT(x)  (((x) << 1) + 1)
#define RIGHT(x) (((x) + 1) << 1)

void siftup(int *h, int n, int i)
{
	(void)n; // Unused arg, retained for consistency
	while (i > 0)
		if (h[i] > h[PARENT(i)])
			swap(&h[i], &h[PARENT(i)]);
}

void siftdown(int *h, int n, int i)
{
	while (i < n) {
		int l = LEFT(i), r = RIGHT(i), s;
		if (l >= n)
			break;
		s = r < n && h[r] >= h[l] ? r : l;
		if (h[i] < h[s])
			swap(&h[i], &h[s]);
		else
			break;
		i = s;
	}
}

void heapsort(int *h, int n)
{
	// Heapify
	for (int i = PARENT(n); i >= 0; i--)
		siftdown(h, n, i);
	// Sort
	for (int i = n - 1; i >= 0; i--) {
		swap(&h[0], &h[i]);
		siftdown(h, i, 0);
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
