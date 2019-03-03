void print_distmap(int *m,const int w,const int h)
{
	for (int i=0;i<w*h;i++) {
		switch (m[i]) {
		case -2:
			printf("\033[1;30m#");
			break;
		case -1:
			putchar(' ');
			break;
		default:
			printf("\033[1;%dm%c",31+(m[i]/10)%7,'0'+m[i]%10);
		}
		if (i%w==w-1)
			putchar('\n');
	}
	printf("\033[m");
}
int *plan_path(char *map,const int w,const int h,int start,int goal)
{
	int *m=malloc(w*h*sizeof(int));
	for (int i=0;i<w*h;i++)
		m[i]=map[i]=='#'?-2:-1;
	// -2 for walls, -1 for unreached
	m[start]=-1;
	m[goal]=0;
	bool possible=true;
	for (int d=1;possible&&m[start]<0;d++) {
		if (VISUALIZE) {
			print_distmap(m,w,h);
			clkslp(200);
		}
		possible=false;
		for (int i=0;i<w*h;i++) {
			if (m[i]!=d-1)
				continue;
			for (int dx=-1;dx<=1;dx++)
			for (int dy=-1;dy<=1;dy++) {
				int n=i+dx+dy*w;
				int xd=n%w-i%w;
				if ((!dx&&!dy)||xd<-1||xd>1||n<0||n>=w*h)
					continue;
				if (m[n]==-1) {
					m[n]=d;
					possible=true;
				}
			}
		}
	}
	return m;
}
