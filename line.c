#include <stdio.h>
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
	int dx=x2-x1,dy=y2-y1;
	if (dx>dy) {
		float slope=(float)dy/dx;
		for (int x=x1;x<=x2;x++) {
			float y=slope*(x-x1)+y1;
			f(x,y-(int)y>0.5?1+(int)y:(int)y);
		}
	} else {
		float slope=(float)dx/dy;
		for (int y=y1;y<=y2;y++) {
			float x=slope*(y-y1)+x1;
			f(x-(int)x>0.5?1+(int)x:(int)x,y);
		}
	}
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
