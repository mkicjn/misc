#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#define WIDTH 30
#define HEIGHT 30
#define AREA (WIDTH*HEIGHT)
//  1 2 3
// 4 5 6
//  7 8 9
enum color {
	BLACK=0,RED,GREEN,YELLOW,BLUE,MAGENTA,CYAN,WHITE
};
enum object {
	NONE=0,FREE,PEASANT,CAPITAL,SPEARMAN,CASTLE,KNIGHT
};
char *obj_strs[KNIGHT+1]={
	"  ","()","[]","/\\","||","%%","<>"
};
struct hex {
	unsigned char team;
	unsigned char obj;
	bool mobile;
};
void draw_tile(struct hex tile,int x,int y)
{
	x<<=1;
	x+=!(y&1);
	printf("\033[%d;%dH",y+1,x+1);
	printf("\033[1;%d;%dm",30+(tile.mobile?CYAN:tile.team),40+tile.team);
	printf("%s",obj_strs[tile.obj]);
}
void draw_isle(struct hex *isle)
{
	for (int i=0;i<AREA;i++)
		draw_tile(isle[i],i%WIDTH,i/WIDTH);
}
int avg_around_hex(int *elevs)
{
	int sum=0;
	sum+=elevs[-WIDTH-1];
	sum+=elevs[-WIDTH];
	sum+=elevs[-1];
	sum+=elevs[0];
	sum+=elevs[1];
	sum+=elevs[WIDTH];
	sum+=elevs[WIDTH+1];
	return sum/7;
}
void generate_isles(struct hex *isle,int erosion)
{
	static const struct hex WATER={CYAN,NONE,false};
	static const struct hex LAND={GREEN,FREE,false};
	static const int WATER_LEVEL=0;
	int elevs[AREA];
	int tmp[AREA];
	for (int i=0;i<AREA;i++) {
		elevs[i]=-100;
		tmp[i]=-100;
	}
	for (int x=1;x<WIDTH-1;x++)
	for (int y=1;y<HEIGHT-1;y++)
		elevs[x+y*WIDTH]=-100+rand()%201;
	while (erosion-->0) {
		/**/
		for (int i=0;i<AREA;i++) {
			if (elevs[i]<WATER_LEVEL) {
				draw_tile(WATER,i%WIDTH,i/WIDTH);
			} else {
				draw_tile(LAND,i%WIDTH,i/WIDTH);
			}
		}
		putchar('\n');
		clock_t t=clock();
		while ((clock()-t)/CLOCKS_PER_SEC<0.5);
		printf("\033[2J");
		/**/
		for (int x=1;x<WIDTH-1;x++)
		for (int y=1;y<HEIGHT-1;y++) {
			int i=x+y*WIDTH;
			tmp[i]=avg_around_hex(&elevs[i]);
		}
		for (int i=0;i<AREA;i++)
			elevs[i]=tmp[i];
	}
	for (int i=0;i<AREA;i++) {
		if (elevs[i]<WATER_LEVEL)
			isle[i]=WATER;
		else
			isle[i]=LAND;
	}
}
int main(int argc,char **argv)
{
	printf("\033[2J");
	srand(time(NULL));
	struct hex isle[AREA];
	generate_isles(isle,3);
	draw_isle(isle);
	printf("\033[m\n");
}
