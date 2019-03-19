#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
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
{
	int dx=p2%w-p1%w;
	int dy=p2/w-p1/w;
	//return dx*dx+dy*dy; // Euclidean
	if (dx<0)
		dx=-dx;
	if (dy<0)
		dy=-dy;
	//return dx>dy?dx:dy; // Chessboard
	return dx+dy; // Taxicab
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
int path_length(char *map,int w,int h,int start,int goal,int maxlen)
{
	if (maxlen<1)
		return -1;
	map[start]='x';
	int nbs[8],dirs[8],dists[8],n=0;
	// Collect unevaluated neighbors
	for (int i=1;i<=9;i++) {
		if (i==5)
			continue;
		int j=start+dir_offset(i,w);
		int dx=start%w-j%w;
		if (j==goal)
			return 1;
		if (dx<-1||dx>1||j<0||j>=w*h)
			continue;
		if (map[j]!=' ')
			continue;
		nbs[n]=j;
		dists[n]=dist(j,goal,w);
		dirs[n]=i;
		n++;
		map[j]='?';
	}
	if (VISUALIZE) {
		print_map(map,w,h);
		clkslp(100);
	}
	// Sort neighbors by best possible distance to goal
	if (n<1)
		return -1;
	for (int i=0;i<n-1;i++)
	for (int j=i+1;j&&dists[j]<dists[j-1];j--) {
		swap(&dists[j],&dists[j-1]);
		swap(&dirs[j],&dirs[j-1]);
		swap(&nbs[j],&nbs[j-1]);
	}
	// Try each neighbor for a new best path, if possible
	int max=maxlen;
	for (int i=0;i<n;i++) {
		if (dists[i]>=max) {
			//printf("%d,%d: No good path through %d,%d (%d>=%d)\n",start%w,start/w,nbs[i]%w,nbs[i]/w,dists[i],max);
			continue;
		}
		int d=path_length(map,w,h,nbs[i],goal,max-1);
		if (d>0&&d<max) {
			//printf("%d,%d: New best path through %d,%d (%d<%d)\n",start%w,start/w,nbs[i]%w,nbs[i]/w,d,max);
			max=d+1;
		}
	}
	return max==maxlen?-1:max;
}
#include "oldpath.c"
int main(int argc,char **argv)
{
	static const int WIDTH=200,HEIGHT=50;
	static const int AREA=WIDTH*HEIGHT;
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

	// Time old pathfining algorithm
	/*
	clock_t t=clock();
	int *dm=plan_path(map,WIDTH,HEIGHT,start,goal);
	t=clock()-t;
	printf("Pathfinding (BFS) took %fms\n",1000.0*t/CLOCKS_PER_SEC);
	print_distmap(dm,WIDTH,HEIGHT);
	free(dm);
	*/

	// Test new pathfinding algorithm
	clock_t t=clock();
	printf("Path length: %d\n",path_length(map,WIDTH,HEIGHT,start,goal,AREA*AREA));
	t=clock()-t;
	printf("Pathfinding (A*?) took %fms\n",1000.0*t/CLOCKS_PER_SEC);
	map[start]='O';
	map[goal]='X';
	print_map(map,WIDTH,HEIGHT);

	printf("%u\n",seed);
	return 0;
}
