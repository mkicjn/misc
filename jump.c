int jump_north(char *m,const int w,const int h,int n)
{
	n-=w;
	while (n>0) {
		if (m[n]=='#')
			return -1;
		if (m[n+1]=='#'&&m[n+1-w]==' ')
			return n;
		if (m[n-1]=='#'&&m[n-1-w]==' ')
			return n;
		n-=w;
	}
	return -1;
}
int jump_south(char *m,const int w,const int h,int n)
{
	n+=w;
	while (n<w*h) {
		if (m[n]=='#')
			return -1;
		if (m[n+1]=='#'&&m[n+1+w]==' ')
			return n;
		if (m[n-1]=='#'&&m[n-1+w]==' ')
			return n;
		n+=w;
	}
	return -1;
}
int jump_west(char *m,const int w,const int h,int n)
{
	n--;
	while (n%w<w-1) {
		if (m[n]=='#')
			return -1;
		if (m[n+w]=='#'&&m[n+w-1]==' ')
			return n;
		if (m[n-w]=='#'&&m[n-w-1]==' ')
			return n;
		n++;
	}
	return -1;
}
int jump_east(char *m,const int w,const int h,int n)
{
	n++;
	while (n%w>0) {
		if (m[n]=='#')
			return -1;
		if (m[n+w]=='#'&&m[n+w+1]==' ')
			return n;
		if (m[n-w]=='#'&&m[n-w+1]==' ')
			return n;
		n++;
	}
	return -1;
}
int jump_northeast(char *m,const int w,const int h,int n)
{
	n+=1-w;
	while (n>0&&n%w>0) {
		printf("Searching NE:%d\n",n);
		if (m[n]=='#')
			return -1;
		if (jump_north(m,w,h,n)>=0)
			return n;
		if (jump_east(m,w,h,n)>=0)
			return n;
		n+=1-w;
	}
	return -1;
}
int jump_northwest(char *m,const int w,const int h,int n)
{
	n+=-1-w;
	while (n>0&&n%w<w-1) {
		printf("Searching NW:%d\n",n);
		if (m[n]=='#')
			return -1;
		if (jump_north(m,w,h,n)>=0)
			return n;
		if (jump_west(m,w,h,n)>=0)
			return n;
		n+=-1-w;
	}
	return -1;
}
int jump_southeast(char *m,const int w,const int h,int n)
{
	n+=1+w;
	while (n<w*h&&n%w>0) {
		printf("Searching SE:%d\n",n);
		if (m[n]=='#')
			return -1;
		if (jump_south(m,w,h,n)>=0)
			return n;
		if (jump_east(m,w,h,n)>=0)
			return n;
		n+=1+w;
	}
	return -1;
}
int jump_southwest(char *m,const int w,const int h,int n)
{
	n+=w-1;
	while (n<w*h&&n%w<w-1) {
		printf("Searching SW:%d\n",n);
		if (m[n]=='#')
			return -1;
		if (jump_south(m,w,h,n)>=0)
			return n;
		if (jump_west(m,w,h,n)>=0)
			return n;
		n+=w-1;
	}
	return -1;
}
int no_jump(char *m,const int w,const int h,int n)
{
	return -1;
}
int (*jump[10])(char *,const int,const int,int)={
	&no_jump,
	&jump_southwest,
	&jump_south,
	&jump_southeast,
	&jump_west,
	&no_jump,
	&jump_east,
	&jump_northwest,
	&jump_north,
	&jump_northeast,
};
