//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#define WIDTH 200
#define HEIGHT 50
#define AREA (WIDTH*HEIGHT)
#ifndef VISUALIZE
#define VISUALIZE 0
#endif
enum dir {
	SOUTHWEST=1,SOUTH,SOUTHEAST,
	WEST,NONE,EAST,
	NORTHWEST,NORTH,NORTHEAST
}; // Represents numpad
int dir_offset(enum dir d,int w)
{
	if (d<1||d>9)
		return 0;
	d=9-d;
	return (1-d%3)+(d/3-1)*w;
}
int dist(int p1,int p2,int w)
{ // Ranked in order of speed
	int dx=p2%w-p1%w;
	int dy=p2/w-p1/w;
	//return dx*dx+dy*dy; // Euclidean
	if (dx<0)
		dx=-dx;
	if (dy<0)
		dy=-dy;
	return dx+dy; // Taxicab
	//return dx>dy?dx:dy; // Chessboard
}
void randomize_map(char *m,int w,int h)
{
	int a=w*h;
	for (int i=0;i<a;i++)
		m[i]='#';
	for (int x=1;x<w-1;x++)
	for (int y=1;y<h-1;y++)
		if (rand()%3)
			m[x+y*w]=' ';
}
void print_map(char *m,int w,int h)
{
	int a=w*h;
	for (int i=0;i<a;i++) {
		switch (m[i]) {
		case '#':
			printf("\033[1;30m");
			break;
		case '.':
			printf("\033[1;33m");
			break;
		case 'O':
			printf("\033[1;34m");
			break;
		case 'X':
			printf("\033[1;31m");
			break;
		case '?':
			printf("\033[1;32m");
			break;
		default:
			printf("\033[m");
		}
		putchar(m[i]);
		if (i%w==w-1)
			putchar('\n');
	}
	printf("\033[m");
}
void swap(int *a,int *b)
{
	int c=*a;
	*a=*b;
	*b=c;
}
void clkslp(int ms)
{
	clock_t t=clock();
	while (1000.0*(clock()-t)/CLOCKS_PER_SEC<ms);
}
void print_distmap(int *m,const int w,const int h)
{
	for (int i=0;i<w*h;i++) {
		switch (m[i]) {
		case -3:
			printf("\033[1;37m?");
			break;
		case -2:
			printf("\033[1;30m#");
			break;
		case -1:
			putchar(' ');
			break;
		default:
			printf("\033[1;%dm%c",31+(m[i]/10)%6,'0'+m[i]%10);
		}
		if (i%w==w-1)
			putchar('\n');
	}
	printf("\033[m");
}
int *old_path(int *m,const int w,const int h,int start,int goal)
{
	m[start]=-1;
	m[goal]=0;
	bool possible=true;
	int x_b[2]={goal%w,goal%w};
	int y_b[2]={goal/w,goal/w};
	for (int d=1;possible&&m[start]<0;d++) {
		/*
		if (VISUALIZE) {
			print_distmap(m,w,h);
			clkslp(100);
		}
		*/
		possible=false;
		for (int x=x_b[0];x<=x_b[1];x++)
		for (int y=y_b[0];y<=y_b[1];y++) {
			int i=x+y*w;
			if (m[i]!=d-1)
				continue;
			for (int dx=-1;dx<=1;dx++)
			for (int dy=-1;dy<=1;dy++) {
				int n=i+dx+dy*w;
				int xd=n%w-i%w;
				if ((!dx&&!dy)||xd<-1||xd>1||n<0||n>=w*h)
					continue;
				if (m[n]==-1) {
					m[n]=d;
					possible=true;
				}
			}
		}
		if (x_b[0]>0)
			x_b[0]--;
		if (x_b[1]<w-1)
			x_b[1]++;
		if (y_b[0]>0)
			y_b[0]--;
		if (y_b[1]<h-1)
			y_b[1]++;
	}
	return m;
}
int path(int *map,int w,int h,int start,int goal)
{ // map: n: distance, -1: unvisited, -2: walls, -3: visited
	if (start==goal)
		return 0;
	if (map[start]>=0)
		return map[start];
	int nbs[8],dists[8],n=0;
	map[start]=-3;
	for (int i=1;i<=9;i++) {
		int nb=start+dir_offset(i,w);
		int dx=nb%w-start%w;
		if (nb==goal)
			return 1;
		if (dx<-1||dx>1||nb<0||nb>=w*h)
			continue;
		if (i==5||map[nb]<-1)
			continue;
		nbs[n]=nb;
		dists[n]=dist(nb,goal,w);
		n++;
	}
	if (n<1)
		return -1; // ???
	for (int i=0;i<n-1;i++)
		for (int j=i+1;j&&dists[j]<dists[j-1];j--) {
			swap(&dists[j],&dists[j-1]);
			swap(&nbs[j],&nbs[j-1]);
		}
	int max=-1;
	if (VISUALIZE) {print_distmap(map,w,h); clkslp(50);}
	for (int i=0;i<n;i++) {
		if (max>0&&dists[i]>=max)
			continue;
		int d=path(map,w,h,nbs[i],goal);
		map[nbs[i]]=d;
		if (max<0||(d>0&&d<max))
			max=d;
	}
	if (VISUALIZE) {print_distmap(map,w,h); clkslp(50);}
	map[start]=1+max;
	return 1+max;
}

#define NO_MAIN
#include "heap.c"
long heuristic(int w, int h, int start, int goal)
{
	int start_x = start % w;
	int start_y = start / w;
	int goal_x = goal % w;
	int goal_y = goal / w;
	int dx = goal_x - start_x;
	int dy = goal_y - start_y;
#define ABS(x) (x < 0 ? -(x) : x)
	return ABS(dx) + ABS(dy);
}
long heap[AREA];
int newpath(int *map, int w, int h, int start, int goal)
{
	long size = 0;
	heap_push(heap, size++, (heuristic(w, h, start, goal) << 32) | start);
	map[start] = -3;
	while (size > 0) {
		int next = heap_pop(heap, size--) & 0xffffffff;
		int x = next % w;
		int y = next / w;
		for (int dx = -1; dx <= 1; dx++)
		for (int dy = -1; dy <= 1; dy++) {
			int neighbor = next + dx + (dy * w);
			if (dx == 0 && dy == 0)
				continue;
			if (dx < 0 && x <= 0)
				continue;
			if (dy < 0 && y <= 0)
				continue;
			if (dx > 0 && x >= w-1)
				continue;
			if (dy > 0 && y >= h-1)
				continue;
			if (neighbor == goal)
				return 0;
			if (map[neighbor] != -1)
				continue;
			heap_push(heap, size++, (heuristic(w, h, neighbor, goal) << 32) | neighbor);
			map[neighbor] = -3;
		}
		map[next] = 0;
		if (VISUALIZE) {print_distmap(map,w,h); clkslp(50);}
	}
	return -1;
}

int main(int argc,char **argv)
{
	static char map[AREA];
	//static char disp[AREA];
	unsigned int seed=time(NULL);
	if (argc>1)
		sscanf(argv[1],"%u",&seed);
	srand(seed);

	randomize_map(map,WIDTH,HEIGHT);
	int start=rand()%AREA,goal=rand()%AREA;
	//map[start]='O';
	//map[goal]='X';
	/*for (int i=0;i<AREA;i++)
		disp[i]=map[i];
	disp[start]='O';
	disp[goal]='X';*/
	printf("Start coordinates: %d,%d\n",start%WIDTH,start/WIDTH);
	//print_map(disp,WIDTH,HEIGHT);
	
	// Test old pathfinding algorithm
	{
	int *imap=malloc(AREA*sizeof(int));
	for (int i=0;i<AREA;i++)
		imap[i]=map[i]==' '?-1:-2;
	clock_t t=clock();
	old_path(imap,WIDTH,HEIGHT,start,goal);
	t=clock()-t;
	printf("Pathfinding (BFS) took %fms\n",1000.0*t/CLOCKS_PER_SEC);
	map[start]='O';
	map[goal]='X';
	print_distmap(imap,WIDTH,HEIGHT);
	}

	// Test new pathfinding algorithm
	{
	int *imap=malloc(AREA*sizeof(int));
	for (int i=0;i<AREA;i++)
		imap[i]=map[i]==' '?-1:-2;
	clock_t t=clock();
	printf("Path length: %d\n",path(imap,WIDTH,HEIGHT,start,goal));
	t=clock()-t;
	printf("Pathfinding (DFS?) took %fms\n",1000.0*t/CLOCKS_PER_SEC);
	map[start]='O';
	map[goal]='X';
	print_distmap(imap,WIDTH,HEIGHT);
	}

	// Test new pathfinding algorithm
	{
	int *imap=malloc(AREA*sizeof(int));
	for (int i=0;i<AREA;i++)
		imap[i]=map[i]==' '?-1:-2;
	clock_t t=clock();
	printf("Path length: %d\n",newpath(imap,WIDTH,HEIGHT,start,goal));
	t=clock()-t;
	printf("Pathfinding (A*?) took %fms\n",1000.0*t/CLOCKS_PER_SEC);
	map[start]='O';
	map[goal]='X';
	print_distmap(imap,WIDTH,HEIGHT);
	}

	printf("%u\n",seed);
	return 0;
}
