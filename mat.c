#include <stdio.h>
#include <stdlib.h>
struct matrix {
	int rows,cols;
	double **data;
};
struct matrix *new_matrix(int rows,int cols)
{
	struct matrix *m=malloc(sizeof(struct matrix));
	m->rows=rows;
	m->cols=cols;
	m->data=malloc(rows*sizeof(double *));
	for (int i=0;i<rows;i++)
		m->data[i]=calloc(cols,sizeof(double));
	return m;
}
void free_matrix(struct matrix *m)
{
	for (int i=0;i<m->rows;i++)
		free(m->data[i]);
	free(m->data);
	free(m);
}
struct matrix *identity(int k)
{
	struct matrix *m=new_matrix(k,k);
	for (int i=0;i<k;i++)
		m->data[i][i]=1;
	return m;
}
void rowmult(double k,double *row,int cols)
{
	for (int i=0;i<cols;i++)
		row[i]*=k;
}
void rowadd(double k,double *row1,double *row2,int cols)
{ // add k*row1 to row2
	if (k==0.0)
		return;
	for (int i=0;i<cols;i++)
		row2[i]+=k*row1[i];
}
void rowswap(double *row1,double *row2,int cols)
{
	for (int i=0;i<cols;i++) {
		double c=row1[i];
		row1[i]=row2[i];
		row2[i]=c;
	}
}
struct matrix *transpose(struct matrix *m)
{
	struct matrix *t=new_matrix(m->cols,m->rows);
	for (int i=0;i<m->rows;i++)
		for (int j=0;j<m->cols;j++)
			t->data[j][i]=m->data[i][j];
	return t;
}
void rref(struct matrix *a,struct matrix *b)
{
	for (int i=0;i<a->rows;i++) {
		int l=0; // leading index
		for (;!a->data[i][l]&&l<a->cols;l++);
		if (l==a->cols)
			return;
		rowmult(1/a->data[i][l],a->data[i],a->cols);
		rowmult(1/a->data[i][l],b->data[i],b->cols);
		for (int j=0;j<a->rows;j++) {
			if (j==i)
				continue;
			rowadd(-(a->data[j][l]),a->data[i],a->data[j],a->cols);
			rowadd(-(a->data[j][l]),b->data[i],b->data[j],b->cols);
		}
	}
}
void matprint(struct matrix *m)
{
	for (int i=0;i<m->rows;i++) {
		putchar('[');
		for (int j=0;j<m->cols;j++) {
			printf("%.3f",m->data[i][j]);
			if (j<m->cols-1)
				printf(", ");
		}
		puts("]");
	}
}
int main(int argc,char **argv)
{
	struct matrix *a=new_matrix(2,2);
	a->data[0][0]=1;
	a->data[0][1]=2;
	a->data[1][0]=3;
	a->data[1][1]=4;
	struct matrix *b=new_matrix(2,1);
	b->data[0][0]=5;
	b->data[1][0]=11;

	rref(a,b);
	matprint(a);
	putchar('\n');
	matprint(b);
	free_matrix(a);
	free_matrix(b);
}
