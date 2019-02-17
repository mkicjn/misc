#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "jump.c"
enum dir {
	SOUTHWEST=1,SOUTH,SOUTHEAST,
	WEST,NONE,EAST,
	NORTHWEST,NORTH,NORTHEAST
}; // Represents numpad
int dist(int p1,int p2,int w)
{
	int dx=p2%w-p1%w;
	if (dx<0)
		dx=-dx;
	int dy=p2/w-p1/w;
	if (dy<0)
		dy=-dy;
	//return dx*dx+dy*dy; // Euclidean
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
		if (rand()%4)
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
int path_length(char *map,int w,int h,int start,int goal,int maxlen)
{
	if (start==goal)
		return 0;
	if (maxlen<1)
		return -1;
	int dirs[8],jps[8],dists[8],n=0;
	dists[0]=-1;
	// Collect jump points in each direction
	for (int i=1;i<=9;i++) {
		int j=jump[i](map,w,h,start,goal);
		// ^ 5 is ignored; jump[5](...)=-1
		if (j==goal) {
			printf("Found jump from %d,%d =%d=> goal\n",start%w,start/w,i);
			return dist(j,goal,w);
		}
		if (j>=0&&dist(start,j,w)<maxlen) {
			jps[n]=j;
			dists[n]=dist(j,goal,w);
			dirs[n]=i;
			n++;
			map[j]='?';
		}
	}
	if (!n)
		return -1;
	// Sort jump points by distance to goal
	// in order to find a path quickly
	for (int i=0;i<n-1;i++)
		for (int j=i+1;j&&dists[j]<dists[j-1];j--) {
			swap(&dists[j-1],&dists[j]);
			swap(&dirs[j-1],&dirs[j]);
			swap(&jps[j-1],&jps[j]);
		}
	// Check each jump point for viable path
	// Update each distance record with path length
	for (int i=0;i<n;i++) {
		printf("Analyzing jump point from %d,%d =%d=> %d,%d (%d)\n",start%w,start/w,dirs[i],jps[i]%w,jps[i]/w,dists[i]);
		int jd=dist(start,jps[i],w); // Jump distance
		int pl=path_length(map,w,h,jps[i],goal,maxlen-jd); // Path length
		if (pl<0||pl+jd>=maxlen) {
			n--;
			swap(&dists[i],&dists[n]);
			swap(&dirs[i],&dirs[n]);
			swap(&jps[i],&jps[n]);
			i--;
		} else {
			dists[i]=pl+jd;
			// If shorter path to goal is found, signal to ignore longer paths
			printf("Shorter path found through %d,%d (%d<%d)\n",start%w,start/w,pl+jd,maxlen);
			maxlen=pl+jd-1;
		}
	}
	// Sort directions by distance to goal on path through jump point
	for (int i=0;i<n-1;i++)
		for (int j=i+1;j&&dists[j]<dists[j-1];j--) {
			swap(&dists[j-1],&dists[j]);
			swap(&dirs[j-1],&dirs[j]);
			swap(&jps[j-1],&jps[j]);
		}
	// Take jump point with shortest path to goal
	int ret=dists[0];
	if (ret>maxlen)
		ret=-1;
	else
		printf("path_len(%d,%d,%d) = %d\n",start%w,start/w,maxlen,ret);
	return ret;
}
int dir_offset(enum dir d,int w)
{
	if (d<1||d>9)
		return 0;
	d=9-d;
	return (1-d%3)+(d/3-1)*w;
}
int main(int argc,char **argv)
{
	static const int WIDTH=80,HEIGHT=24;
	static const int AREA=WIDTH*HEIGHT;
	static char map[AREA]=
"################################################################################"
"#                                                                              #"
"#                                                                              #"
"#                                                                              #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                                                                              #"
"#                                                                              #"
"#                                                                              #"
"################################################################################";
	static char disp[AREA];
	unsigned int seed=time(NULL);
	if (argc>1)
		sscanf(argv[1],"%u",&seed);
	srand(seed);

	//randomize_map(map,WIDTH,HEIGHT);
	//int start=rand()%AREA,goal=rand()%AREA;
	int start=20+12*WIDTH,goal=60+12*WIDTH;
	for (int i=0;i<AREA;i++)
		disp[i]=map[i];

	// Test new pathfinding algorithm
	int n=0;
	clock_t t=clock();
	disp[start]='O';
	/*
	int pos=start;
	while (pos!=goal) {
		enum dir d=path(map,WIDTH,HEIGHT,pos,goal);
		if (!d)
			break;
		pos+=dir_offset(d,WIDTH);
		printf("New step: %d,%d\n",pos%WIDTH,pos/WIDTH);
		disp[pos]='.';
		n++;
	}
	*/
	printf("Start coordinates: %d,%d\n",start%WIDTH,start/WIDTH);
	printf("Path length: %d\n",path_length(map,WIDTH,HEIGHT,start,goal,AREA));
	disp[goal]='X';
	t=clock()-t;
	print_map(disp,WIDTH,HEIGHT);
	printf("Pathfinding took:\n\tSum: %fms\n\tAvg: %fms\n",
			1000.0*t/CLOCKS_PER_SEC,
			1000.0*t/CLOCKS_PER_SEC/n);
	print_map(map,WIDTH,HEIGHT);

	printf("%u\n",seed);
	return 0;
}
