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
{ // Chessboard distance
	int dx=p2%w-p1%w;
	int dy=p2/w-p1/w;
	return dx>dy?dx:dy;
}
// Map generation
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
		if (m[i]=='#')
			printf("\033[1;30m");
		else
			printf("\033[1;31m");
		putchar(m[i]);
		if (i%w==w-1)
			putchar('\n');
	}
}
int main(int argc,char **argv)
{
	static const int WIDTH=50,HEIGHT=25;
	static const int AREA=WIDTH*HEIGHT;
	static char map[AREA];
	srand(time(NULL));

	randomize_map(map,WIDTH,HEIGHT);
	int s=rand()%AREA;
	map[s]='5';
	printf("Start: %d\n",s);
	for (int i=2;i<=8;i+=2) {
		int n=jump[i](map,WIDTH,HEIGHT,s);
		if (n>=0)
			map[n]='0'+i;
	}
	for (int i=1;i<=9;i+=2) {
		int n=jump[i](map,WIDTH,HEIGHT,s);
		if (n>=0)
			map[n]='0'+i;
	}
	print_map(map,WIDTH,HEIGHT);
	return 0;
}
