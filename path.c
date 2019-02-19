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
			printf("(%d,%d): Found goal (i=%d)\n",start%w,start/w,i);
			return sjd;
		}
		printf("(%d,%d): Found jump point %d,%d (i=%d)\n",start%w,start/w,j%w,j/w,i);
		if (sjd>maxlen) {
			printf("(%d,%d): Forgetting %d,%d; jump is too long\n",start%w,start/w,j%w,j/w);
			continue;
		}
		int jgd=dist(j,goal,w); // Jump->Goal dist
		if (sjd+jgd>maxlen) {
			printf("(%d,%d): Forgetting %d,%d; shortest possible path is too long\n",start%w,start/w,j%w,j/w);
			continue;
		}
		jps[n]=j;
		dists[n]=sjd+jgd; // Minimum possible path length
		dirs[n]=i;
		n++;
		map[j]='?';
	}
	if (n<1) {
		printf("(%d,%d): No viable jump points\n",start%w,start/w);
		return -1;
	}
	// Update dist[] with actual path lengths
	// jps[] should only contain coordinates for viable jump points
	for (int i=0;i<n;i++) {
		printf("(%d,%d): Analyzing path through %d,%d (maxlen=%d)\n",start%w,start/w,jps[i]%w,jps[i]/w,maxlen);
		int jd=dist(start,jps[i],w); // Jump distance
		int pl=path_length(map,w,h,jps[i],goal,maxlen); // Path length
		if (pl<0||jd+pl>maxlen) { // If no path or path too long
			printf("(%d,%d): Pruning jump point %d,%d (pl=%d,jd=%d)\n",start%w,start/w,jps[i]%w,jps[i]/w,pl,jd);
			/*
			// Discard jump point
			n--;
			jps[i]=jps[n];
			dirs[i]=dirs[n];
			dists[i]=dists[n];
			i--;
			*/
		} else if (jd+pl<maxlen) {
			maxlen=jd+pl;
			printf("(%d,%d): New best path through %d,%d (%d)\n",start%w,start/w,jps[i]%w,jps[i]/w,maxlen);
		}
	}
	return maxlen;
	// Return minimum distance
	int min=dists[0];
	for (int i=1;i<n;i++)
		if (dists[i]<min)
			min=dists[i];
	return min;
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
"#                      #               ##                                      #"
"#                                      ##                                      #"
"#                                      ##                                      #"
"#                         #            ##                                      #"
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
"################################################################################";
	static char disp[AREA];
	unsigned int seed=time(NULL);
	if (argc>1)
		sscanf(argv[1],"%u",&seed);
	srand(seed);

	//randomize_map(map,WIDTH,HEIGHT);
	int start=20+12*WIDTH,goal=60+12*WIDTH;
	//int start=rand()%AREA,goal=rand()%AREA;
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
	map[start]='O';
	map[goal]='X';
	print_map(map,WIDTH,HEIGHT);

	printf("%u\n",seed);
	return 0;
}
