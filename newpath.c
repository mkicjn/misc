#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// TODO: Jump point search or best-first
void dist(int p1,int p2,int w)
{ // Chessboard distance
	int dx=p2%w-p1%w;
	int dy=p2/w-p1/w;
	return dx>dy?dx:dy;
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
		putchar(m[i]);
		if (i%w==w-1)
			putchar('\n');
	}
}
int main(int argc,char **argv)
{
	static const int WIDTH=80,HEIGHT=24;
	static const int AREA=WIDTH*HEIGHT;
	static char map[AREA];
	srand(time(NULL));
	randomize_map(map,WIDTH,HEIGHT);
	print_map(map,WIDTH,HEIGHT);
	return 0;
}
