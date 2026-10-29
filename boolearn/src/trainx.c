#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <float.h>

#include "net.h"

char solfile[20],*datafile,cmdfile[20],runfile[20],gapfile[20],genfile[20];

int nodes,innodes,outnodes,edges;
int *indeg,*outdeg,**inedge,**outedge,*innode,*outnode;

int fileitems,items,indata,outdata,intype,outtype;

double **w,**x,**y,**wA,**xA,**yA,**wR,**xR,**yR,*wB,**yB;

double **gnode,**gapnode;

double beta,gam;

double acctrain=0.,acctest=0.,conftrain=0.,conftest=0.;

double sqrt2;
	
void allocvars()
	{
	int i;
	
	w=malloc(items*sizeof(double*));
	x=malloc(items*sizeof(double*));
	y=malloc(items*sizeof(double*));
	
	wA=malloc(items*sizeof(double*));
	xA=malloc(items*sizeof(double*));
	
	wR=malloc(items*sizeof(double*));
	xR=malloc(items*sizeof(double*));
	yR=malloc(items*sizeof(double*));
	
	wB=malloc(edges*sizeof(double));
	
	for(i=0;i<items;++i)
		{
		w[i]=malloc(edges*sizeof(double));
		x[i]=malloc(edges*sizeof(double));
		y[i]=malloc(nodes*sizeof(double));
		
		wA[i]=malloc(edges*sizeof(double));
		xA[i]=malloc(edges*sizeof(double));
		
		wR[i]=malloc(edges*sizeof(double));
		xR[i]=malloc(edges*sizeof(double));
		yR[i]=malloc(nodes*sizeof(double));
		}
		
	gnode=malloc(items*sizeof(double*));
	gapnode=malloc(items*sizeof(double*));
	
	for(i=0;i<items;++i)
		{
		gnode[i]=malloc(nodes*sizeof(double));
		gapnode[i]=malloc(nodes*sizeof(double));
		}
	}
	
	
int getdata()
	{
	FILE *fp;
	int i,n,yb,class;
	double yf;
	
	fp=fopen(datafile,"r");
	if(!fp)
		{
		printf("datafile not found\n");
		
		return 0;
		}
		
	fscanf(fp,"%d",&fileitems);
	
	if(items>fileitems)
		{
		printf("datafile has only %d items\n",fileitems);
		
		fclose(fp);
		
		return 0;
		}
		
	fscanf(fp,"%d%d",&intype,&indata);
	
	if(!(intype==0 || intype==1 || intype==2))
		{
		printf("intype must be 0 (analog), 1 (1-hot) or 2 (binary)\n");
		
		fclose(fp);
		
		return 0;
		}
			
	if(indata && (1+indata!=innodes))
		{
		printf("1+indata != innodes\n");
			
		fclose(fp);
			
		return 0;
		}
		
	fscanf(fp,"%d%d",&outtype,&outdata);
	
	if(outtype<2)
		{
		printf("outtype must be 2 (binary outputs) or greater than 2 (class labels)\n");
		
		fclose(fp);
		
		return 0;
		}
	
	if(outtype==2 && outdata!=outnodes)
		{
		printf("outdata != outnodes\n");
		
		fclose(fp);
		
		return 0;
		}
		
	if(outtype>2 && outdata!=1)
		{
		printf("outdata must be 1 (class label) when outtype is greater than 2\n");
		
		fclose(fp);
		
		return 0;
		}
		
	if(outtype>2 && outtype!=outnodes)
		{
		printf("when outtype is greater than 2 it must equal outnodes\n");
		
		fclose(fp);
		
		return 0;
		}
	
	yA=malloc(fileitems*sizeof(double*));
	yB=malloc(fileitems*sizeof(double*));
	
	for(i=0;i<fileitems;++i)
		{
		yA[i]=malloc(nodes*sizeof(double));
		yB[i]=malloc(nodes*sizeof(double));
		}
	
	for(i=0;i<fileitems;++i)
		{
		yA[i][0]=-1.;
		
		if(indata)
			{
			for(n=1;n<innodes;++n)
				{
				if(intype==0)
					{
					fscanf(fp,"%lf",&yf);
				
					yA[i][n]=2.*yf-1.;
					}
				else
					{
					fscanf(fp,"%d",&yb);
				
					yA[i][n]=yb ? 1. : -1.;
					}
				}
			}
			
		if(outtype==2)
			{
			for(n=0;n<outnodes;++n)
				{
				fscanf(fp,"%d",&yb);
			
				yB[i][n+nodes-outnodes]=yb ? 1. : -1.;
				}
			}
		else
			{
			for(n=0;n<outnodes;++n)
				yB[i][n+nodes-outnodes]=-1.;
				
			fscanf(fp,"%d",&class);
			
			yB[i][class+nodes-outnodes]=1.;
			}
		}
			
	fclose(fp);
	
	return 1;
	}

	
double normboundproj(int deg,double v[WMAX],double bs)
	{
	int d;
	double b,vn,s;
	
	vn=0.;
	for(d=0;d<deg;++d)
		vn+=v[d]*v[d];
		
	b=fabs(bs);
	
	if((bs>0. && vn>b) || (bs<0. && vn<b))
		return 0.;
	else
		{
		s=sqrt(b/vn);
		
		for(d=0;d<deg;++d)
			v[d]*=s;
			
		return (s-1.)*(s-1.)*vn;
		}
	}
	
	
double marginproj(int deg,double wn[WMAX],double xn[WMAX],double mus)
	{
	int d;
	double p[WMAX],q[WMAX],mu,dist;
	
	for(d=0;d<deg;++d)
		{
		p[d]=(wn[d]+xn[d])/sqrt2;
		q[d]=(wn[d]-xn[d])/sqrt2;
		}
		
	mu=fabs(mus);
	
	if(mus>0.)
		dist=normboundproj(deg,p,deg+mu)+normboundproj(deg,q,-(deg-mu));
	else
		dist=normboundproj(deg,p,-(deg-mu))+normboundproj(deg,q,deg+mu);
		
	for(d=0;d<deg;++d)
		{
		wn[d]=(p[d]+q[d])/sqrt2;
		xn[d]=(p[d]-q[d])/sqrt2;
		}
	
	return dist;
	}


void neuronproj(int deg,double wn[WMAX],double xn[WMAX],double *yn)
	{
	int d;
	double del,dot,dist1,dist0,w1[WMAX],w0[WMAX],x1[WMAX],x0[WMAX];
	
	del=sqrt(deg/sig);
	
	dot=0.;
	for(d=0;d<deg;++d)
		dot+=wn[d]*xn[d];
	
	if((*yn)*dot>0.)
		{
		if(*yn>0.)
			{
			*yn=1.;
			
			if(dot<del)
				marginproj(deg,wn,xn,del);
			}
		else
			{
			*yn=-1.;
			
			if(dot>-del)
				marginproj(deg,wn,xn,-del);
			}
			
		return;
		}
		
	for(d=0;d<deg;++d)
		{
		w1[d]=wn[d];
		x1[d]=xn[d];
			
		w0[d]=wn[d];
		x0[d]=xn[d];
		}
	
	dist1=(1.-*yn)*(1.-*yn);
	dist0=(1.+*yn)*(1.+*yn);
	
	if(dot<del)
		dist1+=marginproj(deg,w1,x1,del);
	
	if(dot>-del)
		dist0+=marginproj(deg,w0,x0,-del);
	
	if(dist1<dist0)
		{
		*yn=1.;
		
		for(d=0;d<deg;++d)
			{
			wn[d]=w1[d];
			xn[d]=x1[d];
			}
		}
	else
		{
		*yn=-1.;
		
		for(d=0;d<deg;++d)
			{
			wn[d]=w0[d];
			xn[d]=x0[d];
			}
		}
	}


void projA()
	{
	int i,n,d,e,nmax;
	double wn[WMAX],xn[WMAX],yn,ymax;
	
	for(i=0;i<items;++i)
		{
		if(!indata)
			{
			if(intype==2)
				{
				for(n=1;n<innodes;++n)
					yA[i][n]=(y[i][n]<0.) ? -1. : 1.;
				}
			else
				{
				for(n=1;n<innodes;++n)
					yA[i][n]=-1.;
					
				ymax=y[i][1];
				nmax=1;
				
				for(n=2;n<innodes;++n)
					if(y[i][n]>ymax)
						{
						ymax=y[i][n];
						nmax=n;
						}
				
				yA[i][nmax]=1.;
				}
			}
	
		for(n=innodes;n<nodes;++n)
			{
			for(d=0;d<indeg[n];++d)
				{
				e=inedge[n][d];
				wn[d]=w[i][e];
				xn[d]=x[i][e];
				}
			
			yn=y[i][n];
	
			neuronproj(indeg[n],wn,xn,&yn);
			
			for(d=0;d<indeg[n];++d)
				{
				e=inedge[n][d];
				wA[i][e]=wn[d];
				xA[i][e]=xn[d];
				}
			
			yA[i][n]=yn;
			}
		}
	}
	
	
void reflA()
	{
	int i,e,n;
	
	for(i=0;i<items;++i)
		{	
		for(e=0;e<edges;++e)
			{
			wR[i][e]=2.*wA[i][e]-w[i][e];
			xR[i][e]=2.*xA[i][e]-x[i][e];
			}
		
		for(n=0;n<nodes;++n)
			yR[i][n]=2.*yA[i][n]-y[i][n];
		}
	}
	
	
void projB()
	{
	int e,i,n,d,out,in;
	double wnorm,gw,gz,z;
	
	for(e=0;e<edges;++e)
		{
		in=innode[e];
		
		wB[e]=0.;
		gw=0.;
		
		for(i=0;i<items;++i)
			{
			wB[e]+=gnode[i][in]*wR[i][e];
			gw+=gnode[i][in];
			}
			
		wB[e]/=gw;
		}
		
	for(n=innodes;n<nodes;++n)
		{
		wnorm=0.;
		
		for(d=0;d<indeg[n];++d)
			{
			e=inedge[n][d];
			wnorm+=wB[e]*wB[e];
			}
			
		wnorm=sqrt(wnorm/indeg[n]);
		
		for(d=0;d<indeg[n];++d)
			{
			e=inedge[n][d];
			wB[e]/=wnorm;
			}
		}
	
	for(i=0;i<items;++i)
	for(out=0;out<nodes-outnodes;++out)
		{
		gz=gnode[i][out];
		z=gnode[i][out]*yR[i][out];
		
		for(d=0;d<outdeg[out];++d)
			{
			e=outedge[out][d];
			in=innode[e];
			
			gz+=gnode[i][in];
			z+=gnode[i][in]*xR[i][e];
			}
			
		z/=gz;
		
		yB[i][out]=z;
		}
	}
	
	
double RRR(double *gapw,double *gapx,double *gapy)
	{
	int i,n,d,e,out;
	double diff,gap;
	
	projA();
	reflA();
	projB();
	
	*gapw=0.;
	*gapx=0.;
	*gapy=0.;
	
	gap=0.;
	
	for(i=0;i<items;++i)
	for(n=0;n<nodes;++n)
		{
		diff=yB[i][n]-yA[i][n];
		gapnode[i][n]=diff*diff;
		y[i][n]+=beta*diff;
		
		*gapy+=diff*diff;
		
		if(n>=innodes)
			{
			for(d=0;d<indeg[n];++d)
				{
				e=inedge[n][d];
				out=outnode[e];
			
				diff=wB[e]-wA[i][e];
				gapnode[i][n]+=diff*diff;
				w[i][e]+=beta*diff;
				
				*gapw+=diff*diff;
			
				diff=yB[i][out]-xA[i][e];
				gapnode[i][n]+=diff*diff;
				x[i][e]+=beta*diff;
				
				*gapx+=diff*diff;
				}
			
			gap+=gapnode[i][n];
			}
		}
	
	gap/=items*(nodes-innodes);
	
	for(i=0;i<items;++i)
	for(n=innodes;n<nodes;++n)
		gnode[i][n]+=gam*(gapnode[i][n]/gap-gnode[i][n]);
		
	*gapw/=items*edges;
	*gapx/=items*edges;
	*gapy/=items*nodes;
	
	*gapw=sqrt(*gapw);
	*gapx=sqrt(*gapx);
	*gapy=sqrt(*gapy);
	
	return sqrt(gap);
	}
	
	
double urand()
	{
	return ((double)rand())/RAND_MAX;
	}
	

void init()
	{
	int i,e,n;
	
	for(i=0;i<items;++i)
		{
		for(e=0;e<edges;++e)
			{
			w[i][e]=2.*urand()-1.;
			x[i][e]=2.*urand()-1.;
			}
		
		y[i][0]=-1.;
		
		for(n=1;n<innodes;++n)
			y[i][n]=indata ? yA[i][n] : 2.*urand()-1.;
			
		for(n=innodes;n<nodes-outnodes;++n)
			y[i][n]=2.*urand()-1.;
			
		for(n=nodes-outnodes;n<nodes;++n)
			y[i][n]=yB[i][n];
		}
		
	for(i=0;i<items;++i)
	for(n=0;n<nodes;++n)
		gnode[i][n]=1.;
	}
		

void accuracy(int train,double *acc,double *confave)
	{
	int i,i0,i1,n,nmax;
	double accitem,ymax;
	
	if(train)
		{
		i0=0;
		i1=items;
		}
	else
		{
		if(items==fileitems)
			{
			*acc=0.;
			*confave=0.;
			
			return;
			}
			
		i0=items;
		i1=fileitems;
		}
		
	*acc=0.;
	*confave=0.;

	for(i=i0;i<i1;++i)
		{	
		for(n=0;n<innodes;++n)
			yinfer[n]=yA[i][n];
			
		infer(wB);
		
		*confave+=conf;

		if(outtype==2)
			{
			accitem=0.;
			
			for(n=0;n<outnodes;++n)
				accitem+=(yinfer[n+nodes-outnodes]>0.)==(yB[i][n+nodes-outnodes]>0.);
			
			accitem/=outnodes;
			}
		else
			{
			ymax=-DBL_MAX;
			nmax=0;
			
			for(n=0;n<outnodes;++n)
				if(yinfer[n+nodes-outnodes]>ymax)
					{
					ymax=yinfer[n+nodes-outnodes];
					nmax=n;
					}
			
			accitem=yB[i][nmax+nodes-outnodes]>0. ? 1. : 0.;
			}
						
		*acc+=accitem;
		}
			
	*acc/=i1-i0;
	*confave/=i1-i0;
	}
	
	
void printsol()
	{
	FILE *fp;
	int e,in,out,codes,usedcodes,*counts,c,code,i,n,bit,nmax;
	double ymax;

	fp=fopen(solfile,"w");
	
	fprintf(fp,"%d  %d  %d  %d\n",nodes,innodes,outnodes,edges);
	
	for(e=0;e<edges;++e)
		{
		in=innode[e];
		out=outnode[e];
		
		fprintf(fp,"%5d%5d%14.8lf\n",in,out,wB[e]);
		}
		
	fclose(fp);
	
	if(!indata && intype==1)
		{
		counts=malloc((innodes-1)*sizeof(int));
		
		for(n=1;n<innodes;++n)
			counts[n]=0;
		
		for(i=0;i<items;++i)
			{
			ymax=yB[i][1];
			nmax=1;
			
			for(n=2;n<innodes;++n)
				if(yB[i][n]>ymax)
					{
					ymax=yB[i][n];
					nmax=n;
					}
			
			++counts[nmax];
			}
			
		fp=fopen(genfile,"w");
		
		fprintf(fp,"%d  %d\n\n",innodes-1,innodes-1);
		
		for(c=1;c<innodes;++c)
			{
			for(n=1;n<innodes;++n)
				fprintf(fp,"%2d",n==c);
				
			fprintf(fp,"\t%d\n",counts[c]);
			}
			
		fclose(fp);		
		}
		
	if(!indata && intype==2)
		{
		codes=1<<(innodes-1);
		
		counts=malloc(codes*sizeof(int));
		
		for(c=0;c<codes;++c)
			counts[c]=0;
		
		for(i=0;i<items;++i)
			{
			c=0;
			for(n=1;n<innodes;++n)
				c+=(yB[i][n]>0.)<<(n-1);
			
			++counts[c];
			}
		
		usedcodes=0;
		for(c=0;c<codes;++c)
			if(counts[c])
				++usedcodes;
				
		fp=fopen(genfile,"w");
		
		fprintf(fp,"%d  %d\n\n",usedcodes,innodes-1);
		
		for(c=0;c<codes;++c)
			if(counts[c])
				{
				code=c;
				
				for(n=1;n<innodes;++n)
					{
					bit=code%2;
					
					fprintf(fp,"%2d",bit);
					
					code/=2;
					}
					
				fprintf(fp,"\t%d\n",counts[c]);
				}
				
		fclose(fp);		
		}
	}
	
	
int solve(double *gap,double itermax,double itermult,double gapstop)
	{
	FILE *fp;
	int iter;
	double gapw,gapx,gapy,gapmin,iterprint;
	double acctrainmax;
	
	fp=fopen(gapfile,"w");
	fclose(fp);
			
	init();
	gapmin=1e10;
	
	iterprint=itermult;
	
	acctrainmax=0.;
	
	for(iter=1;iter<=itermax;++iter)
		{
		*gap=RRR(&gapw,&gapx,&gapy);
		
		if(*gap<gapmin)
			gapmin=*gap;
			
		if(iter>iterprint || gapmin<gapstop || iter==itermax)
			{
			if(indata)
				{
				accuracy(1,&acctrain,&conftrain);
				
				if(acctrain>acctrainmax)
					{
					acctrainmax=acctrain;
					printsol();
					}
					
				accuracy(0,&acctest,&conftest);
				}
			
			fp=fopen(gapfile,"a");
			fprintf(fp,"%10d%14.8lf%14.8lf%14.8lf%14.8lf%14.8lf%10.3lf %%%10.3lf %%%10.3lf %%%10.3lf %%\n",iter,gapw,gapx,gapy,*gap,gapmin,100.*acctrain,100.*acctest,100.*conftrain,100.*conftest);
			fclose(fp);
		
			iterprint*=itermult;
			}
			
		if(gapmin<gapstop)
			return iter;
		}
			
	return 0;
	}
	
	
void run(int trials,double itermax,double itermult,double gapstop)
	{
	FILE *fp;
	int success,t,iter;
	double gap,iterave,iter2ave,diter,itererr;
	
	fp=fopen(runfile,"w");
	fclose(fp);
		
	success=0;
	iterave=0.;
	iter2ave=0.;
	
	for(t=1;t<=trials;++t)
		{
		iter=solve(&gap,itermax,itermult,gapstop);
		
		fp=fopen(runfile,"a");
		fprintf(fp,"%4d%12d%14.8lf%10.3lf %%%10.3lf %%%10.3lf %%%10.3lf %%\n",t,iter,gap,100.*acctrain,100.*acctest,100.*conftrain,100.*conftest);
		fclose(fp);
		
		if(iter)
			{
			++success;
			
			if(!indata)
				printsol();
			
			diter=(double)iter;
			iterave+=diter;
			iter2ave+=diter*diter;
			}
		}
	
	if(!success)
		{
		if(!indata)
			printsol();
		
		fp=fopen(runfile,"a");
		
		fprintf(fp,"\n no solutions found\n");
		
		fclose(fp);
		
		return;
		}
	
	iterave/=(double)success;
	iter2ave/=(double)success;
	itererr=sqrt((iter2ave-iterave*iterave)/(double)success);
	
	fp=fopen(runfile,"a");
	fprintf(fp,"\n%d / %d solutions   %lf +/- %lf iterations/solution\n",success,trials,iterave,itererr);
	fclose(fp);
	}
	
	
int main(int argc,char* argv[])
	{
	char *id;
	int epochs,a,trials;
	double gapstop,itermax,itermult;
	FILE *fp;
	
	if(argc==12)
		{
		netfile=argv[1];
		datafile=argv[2];
		items=atoi(argv[3]);
		sig=atof(argv[4]);
		beta=atof(argv[5]);
		gam=atof(argv[6]);
		epochs=atoi(argv[7]);
		itermax=atof(argv[8]);
		gapstop=atof(argv[9]);
		trials=atoi(argv[10]);
		id=argv[11];
		}
	else
		{
		printf("expected 11 arguments: netfile datafile items sig beta gam epochs itermax gapstop trials id\n");
		return 1;
		}
	
	if(!getnet())
		return 1;
		
	allocvars();
	
	if(!getdata())
		return 1;
	
	sprintf(cmdfile,"%s.cmd",id);
	fp=fopen(cmdfile,"w");
	for(a=0;a<argc;++a)
    	fprintf(fp,"%s ",argv[a]);
    fprintf(fp,"\n");
	fclose(fp);
	
	sprintf(runfile,"%s.run",id);
	sprintf(gapfile,"%s.gap",id);
	sprintf(solfile,"%s.sol",id);
	
	if(!indata)
		sprintf(genfile,"%s.gen",id);
	
	srand(time(NULL));
	
	sqrt2=sqrt(2.);
	
	itermult=pow(itermax,1./((double)epochs));

	run(trials,itermax,itermult,gapstop);
	
	return 0;
	}
	