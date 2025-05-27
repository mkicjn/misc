//usr/bin/env tcc $CFLAGS -run $0 $@; exit $?
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
	BLACK,RED,GREEN,YELLOW,BLUE,MAGENTA,CYAN,WHITE
};
enum object {
	NONE,FREE,PEASANT,CAPITAL,SPEARMAN,CASTLE,KNIGHT,BARON
};
char *obj_strs[]={
	"~~","()","[]","/\\","||","##","{}","<>"
};
struct hex {
	unsigned char team;
	unsigned char obj;
	bool mob;
};
void draw_tile(struct hex tile,int x,int y)
{
	x<<=1;
	x+=!(y&1);
	printf("\033[%d;%dH",y+1,x+1);
	printf("\033[1;%d;%dm",30+(tile.mob?CYAN:tile.team),40+tile.team);
	printf("%s",obj_strs[tile.obj]);
}
void draw_isle(struct hex *isle)
{
	for (int i=0;i<AREA;i++)
		draw_tile(isle[i],i%WIDTH,i/WIDTH);
}
int avg_around(int *elevs)
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
	static const struct hex WATER={BLUE,NONE,false};
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
		for (int x=1;x<WIDTH-1;x++)
		for (int y=1;y<HEIGHT-1;y++) {
			int i=x+y*WIDTH;
			tmp[i]=avg_around(&elevs[i]);
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
void init_territories(struct hex *isle,enum color *teams,int n_players)
{
	for (int i=0;i<AREA;i++)
		if (isle[i].team!=BLUE)
			isle[i].team=teams[rand()%n_players];
}
int main(int argc,char **argv)
{
	enum color teams[]={RED,GREEN,YELLOW,MAGENTA,CYAN,WHITE};
	printf("\033[2J");
	srand(time(NULL));
	struct hex isle[AREA];
	generate_isles(isle,3);
	init_territories(isle,teams,6);
	draw_isle(isle);
	printf("\033[m\n");
}
