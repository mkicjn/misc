#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "jump.c"
#ifndef VISUALIZE
#define VISUALIZE 0
#endif
enum dir {
	SOUTHWEST=1,SOUTH,SOUTHEAST,
	WEST,NONE,EAST,
	NORTHWEST,NORTH,NORTHEAST
}; // Represents numpad
int dist(int p1,int p2,int w)
{
	int dx=p2%w-p1%w;
	int dy=p2/w-p1/w;
	return dx*dx+dy*dy; // Euclidean
	/*
	if (dx<0)
		dx=-dx;
	if (dy<0)
		dy=-dy;
	*/
	//return dx+dy; // Taxicab
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
void clocksleep(int ms)
{
	clock_t t=clock();
	while (1000.0*(clock()-t)/CLOCKS_PER_SEC<ms);
}
int path_length(char *map,int w,int h,int start,int goal,int maxlen)
{
	int jps[8],dists[8],dirs[8];
	int n=0;
	for (int i=1;i<=9;i++) {
		int j=jump[i](map,w,h,start,goal);
		if (j<0)
			continue;
		int sjd=dist(start,j,w); // Start->Jump dist
		if (j==goal) {
			//printf("(%d,%d): Found goal (i=%d)\n",start%w,start/w,i);
			return sjd;
		}
		//printf("(%d,%d): Found jump point %d,%d (i=%d)\n",start%w,start/w,j%w,j/w,i);
		if (sjd>maxlen) {
			//printf("(%d,%d):\tDiscarded; direct jump is too long\n",start%w,start/w);
			continue;
		}
		int jgd=dist(j,goal,w); // Jump->Goal dist
		if (sjd+jgd>maxlen) {
			//printf("(%d,%d):\tDiscarded; shortest possible path is too long\n",start%w,start/w);
			continue;
		}
		jps[n]=j;
		dists[n]=sjd+jgd; // Minimum possible path length
		dirs[n]=i;
		n++;
		map[j]='?';
		if (VISUALIZE) {
			print_map(map,w,h);
			clocksleep(200);
		}
	}
	if (n<1) {
		//printf("(%d,%d): No viable jump points\n",start%w,start/w);
		return -1;
	}
	// Sort jump points by shortest possible path distance
	for (int i=0;i<n-1;i++)
		for (int j=i+1;j&&dists[j]<dists[j-1];j--) {
			swap(&dists[j],&dists[j-1]);
			swap(&dirs[j],&dirs[j-1]);
			swap(&jps[j],&jps[j-1]);
		}
	// Check each jump point for viable paths, updating maxlen if improvement is made
	for (int i=0;i<n;i++) {
		//printf("(%d,%d): Analyzing path through %d,%d (maxlen=%d)\n",start%w,start/w,jps[i]%w,jps[i]/w,maxlen);
		if (dists[i]>maxlen) // If it's not possible for the jump point to yield a shorter path:
			goto DISCARD_JP; // Discard it
		int jd=dist(start,jps[i],w); // Get jump distance
		int pl=path_length(map,w,h,jps[i],goal,maxlen-jd); // Get actual path length
		if (pl<0||jd+pl>maxlen) { // If no path or path too long:
			// Discard jump point
DISCARD_JP:
			//printf("(%d,%d): Discarding jump point %d,%d (%d)\n",start%w,start/w,jps[i]%w,jps[i]/w,pl);
			n--;
			jps[i]=jps[n];
			dirs[i]=dirs[n];
			dists[i]=dists[n];
			i--;
			continue;
		} else {
			// Update the criterion
			//printf("(%d,%d): New best path through %d,%d (%d)\n",start%w,start/w,jps[i]%w,jps[i]/w,maxlen);
			maxlen=jd+pl;
		}
	}
	// Return minimum path length (i.e. search criteria maxlen)
	return maxlen;
}
int dir_offset(enum dir d,int w)
{
	if (d<1||d>9)
		return 0;
	d=9-d;
	return (1-d%3)+(d/3-1)*w;
}
#include "oldpath.c"
int main(int argc,char **argv)
{
	static const int WIDTH=200,HEIGHT=50;
	static const int AREA=WIDTH*HEIGHT;
	static char map[AREA];
	static char disp[AREA];
	unsigned int seed=time(NULL);
	if (argc>1)
		sscanf(argv[1],"%u",&seed);
	srand(seed);

	randomize_map(map,WIDTH,HEIGHT);
	int start=rand()%AREA,goal=rand()%AREA;
	map[start]='O';
	map[goal]='X';
	for (int i=0;i<AREA;i++)
		disp[i]=map[i];

	// Test new pathfinding algorithm
	int n=0;
	clock_t t=clock();
	disp[start]='O';
	printf("Start coordinates: %d,%d\n",start%WIDTH,start/WIDTH);
	printf("Path length: %d\n",path_length(map,WIDTH,HEIGHT,start,goal,WIDTH*WIDTH+HEIGHT*HEIGHT));
	disp[goal]='X';
	t=clock()-t;
	print_map(disp,WIDTH,HEIGHT);
	printf("Pathfinding (JPS) took %fms\n",1000.0*t/CLOCKS_PER_SEC);
	map[goal]='X';
	print_map(map,WIDTH,HEIGHT);

	t=clock();
	int *dm=plan_path(map,WIDTH,HEIGHT,start,goal);
	t=clock()-t;
	print_distmap(dm,WIDTH,HEIGHT);
	free(dm);
	printf("Pathfinding (BFS) took %fms\n",1000.0*t/CLOCKS_PER_SEC);

	printf("%u\n",seed);
	return 0;
}
