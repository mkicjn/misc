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
	return dx*dx+dy*dy;//dx>dy?dx:dy;
}
// Map generation
void randomize_map(char *m,int w,int h)
{
	int a=w*h;
	for (int i=0;i<a;i++)
		m[i]='#';
	for (int x=1;x<w-1;x++)
	for (int y=1;y<h-1;y++)
		if (rand()%20)
			m[x+y*w]=' ';
}
void print_map(char *m,int w,int h)
{
	int a=w*h;
	for (int i=0;i<a;i++) {
		switch (m[i]) {
		case '#':
			printf("\033[1;30m#");
			break;
		case '.':
			printf("\033[1;33m.");
			break;
		case 'O':
			printf("\033[1;34mO");
			break;
		case 'X':
			printf("\033[1;31mX");
			break;
		default:
			printf("\033[m%c",m[i]);
		}
		if (i%w==w-1)
			putchar('\n');
	}
}
void swap(int *a,int *b)
{
	int c=*a;
	*a=*b;
	*b=c;
}
enum dir path(char *map,int w,int h,int start,int goal)
{
	int dirs[10],jps[10],dists[10];
	for (int i=0;i<10;i++) {
		dirs[i]=i;
		jps[i]=jump[i](map,w,h,start,goal);
		if (jps[i]>=0) {
			if (jps[i]==goal)
				return i;
			dists[i]=dist(jps[i],goal,w);
		} else
			dists[i]=-1;
	}
	// Sort jump points by increasing distance to goal
	for (int i=0;i<9;i++)
		for (int j=i+1;j&&dists[j]<dists[j-1];j--) {
			swap(&dists[j-1],&dists[j]);
			swap(&dirs[j-1],&dirs[j]);
			swap(&jps[j-1],&jps[j]);
		}
	// Print nodes
	for (int i=0;i<9;i++) {
		if (jps[i]>-1)
			printf("Jump point from %d: %d (%d)\n",start,jps[i],dists[i]);
	}
	// Check each jump point for viable path
	for (int i=0;i<9;i++) {
		if (jps[i]<0)
			continue;
		if (dists[i]==0)
			return dirs[i];
		if (path(map,w,h,jps[i],goal)!=0)
			return dirs[i];
		map[jps[i]]='C';
	}
	return 0;
}
int dir_offset(enum dir d,int w)
{
	d=9-d;
	return (1-d%3)+(d/3-1)*w;
}
int main(int argc,char **argv)
{
	static const int WIDTH=50,HEIGHT=25;
	static const int AREA=WIDTH*HEIGHT;
	static char map[AREA];
	srand(time(NULL));

	randomize_map(map,WIDTH,HEIGHT);
	int s=rand()%AREA,g=rand()%AREA;
	map[s]='O';
	map[g]='X';
	for (;;) {
		enum dir d=path(map,WIDTH,HEIGHT,s,g);
		s+=dir_offset(d,WIDTH);
		if (s!=g)
			map[s]='.';
		else
			break;
	}
	print_map(map,WIDTH,HEIGHT);
	return 0;
}
