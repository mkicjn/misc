#include <stdio.h>
#include <math.h>
void cls(void)
{
	printf("\033[2J");
}
void mark(int x,int y)
{
	printf("\033[%d;%dH#",y+1,x+1);
}
void line(void (*f)(int,int),int x1,int y1,int x2,int y2)
{
	float dx=x2-x1,dy=y2-y1;
	float m=sqrt(dx*dx+dy*dy);
	dx/=m,dy/=m;
	float x=x1,y=y1;
	for (int i=0;i<m;i++)
		f(round(x+=dx),round(y+=dy));
}
int main(int argc,char **argv)
{
	int x1,y1,x2,y2;
	if (argc<4)
		return 1;
	sscanf(argv[1],"%d",&x1);
	sscanf(argv[2],"%d",&y1);
	sscanf(argv[3],"%d",&x2);
	sscanf(argv[4],"%d",&y2);
	cls();
	line(mark,x1,y1,x2,y2);
	putchar('\n');
	putchar('\n');
	return 0;
}
