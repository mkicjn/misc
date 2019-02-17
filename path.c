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
	int dy=p2/w-p1/w;
	return dx*dx+dy*dy; // Euclidean
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
enum dir path(char *map,int w,int h,int start,int goal)
{ // TODO: Tidy up, move to jump.c
	int dirs[10],jps[10],dists[10];
	for (int i=0;i<10;i++) { // For each direction
		dirs[i]=i;
		// Record jump point in that direction
		jps[i]=jump[i](map,w,h,start,goal);
		if (jps[i]>=0) {
			// Record its distance to the goal
			dists[i]=dist(jps[i],goal,w);
			if (!dists[i]) { // If it is the goal
				// Clean up
				for (int i=0;i<w*h;i++)
					if (map[i]=='?')
						map[i]=' ';
				return i;
			}
		} else
			dists[i]=-1;
	}
	// Sort jump points/directions/distances by increasing jump point distance to goal
	for (int i=0;i<9;i++)
		for (int j=i+1;j&&dists[j]<dists[j-1];j--) {
			swap(&dists[j-1],&dists[j]);
			swap(&dirs[j-1],&dirs[j]);
			swap(&jps[j-1],&jps[j]);
		}
	// Check each jump point for viable path
	for (int i=0;i<10;i++) {
		if (jps[i]<0)
			continue;
		printf("Jump point from %d,%d =%d=> %d,%d (%d)\n",start%w,start/w,dirs[i],jps[i]%w,jps[i]/w,dists[i]);
		if (dists[i]==0)
			return dirs[i];
		map[jps[i]]='?';
		if (path(map,w,h,jps[i],goal)!=0)
			return dirs[i];
	}
	return 0;
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
	static char map[AREA];
	static char disp[AREA];
	unsigned int seed=time(NULL);
	if (argc>1)
		sscanf(argv[1],"%u",&seed);
	srand(seed);

	randomize_map(map,WIDTH,HEIGHT);
	int start=rand()%AREA,goal=rand()%AREA;
	for (int i=0;i<AREA;i++)
		disp[i]=map[i];

	// Test new pathfinding algorithm
	int n=0;
	clock_t t=clock();
	disp[start]='O';
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
	disp[goal]='X';
	t=clock()-t;
	print_map(disp,WIDTH,HEIGHT);
	printf("Pathfinding took:\n\tSum: %fms\n\tAvg: %fms\n",
			1000.0*t/CLOCKS_PER_SEC,
			1000.0*t/CLOCKS_PER_SEC/n);

	printf("%u\n",seed);
	return 0;
}
