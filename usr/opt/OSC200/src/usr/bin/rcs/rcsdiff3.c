/*
 * *********************************************************************
 * *                                                                   *
 * *       Modified by Digital Equipment Corporation, 1991, 1994       *
 * *                                                                   *
 * *       This file no longer matches the original Free Software      *
 * *       Foundation file.                                            *
 * *                                                                   *
 * *********************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: rcsdiff3.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/05/28 02:24:44 $";
#endif
/*
 * Revision 1.3  91/01/07  16:49:40  devrcs
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  18:52:20  dwm]
 * 
 * Revision 1.2  90/10/07  21:44:49  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/29  13:55:15  gm]
 * 
 * Revision 1.1  89/12/26  10:43:52  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * Revision 1.3  89/05/23  20:49:01  bww
 * 	Added RCSid.
 * 	[89/05/23            bww]
 * 
 * Revision 1.2  89/05/23  16:45:09  bww
 * 	CMU CS as of 89/05/15
 * 	[89/05/23  16:39:10  bww]
 * 
 * Revision 1.3  89/01/25  16:12:55  gm0w
 * 	Added code to exit when fopen of a diff file fails.
 * 	[89/01/25            gm0w]
 * 
 */
/*** "diff3.c	4.4 (Berkeley) 8/27/85"; ***/

#include <stdio.h>
#include <ctype.h>

#ifdef debug
#define dprintf(x) printf x
#else
#define dprintf(x)
#endif /* debug */

/* diff3 - 3-way differential file comparison*/

/* diff3 [-ex3EX] d13 d23 f1 f2 f3 [m1 m3]
 *
 * d13 = diff report on f1 vs f3
 * d23 = diff report on f2 vs f3
 * f1, f2, f3 the 3 files
 * if changes in f1 overlap with changes in f3, m1 and m3 are used
 * to mark the overlaps; otherwise, the file names f1 and f3 are used
 * (only for options E and X).
*/

struct  range {int from,to; };
	/* from is first in range of changed lines
	 * to is last+1
	 * from=to=line after point of insertion
	* for added lines
	*/
struct diff {struct range old, new;};

#define NC 1024
struct diff d13[NC];
struct diff d23[NC];
/* de is used to gather editing scripts,
 * that are later spewed out in reverse order.
 * its first element must be all zero
 * the "new" component of de contains line positions
 * or byte positions depending on when you look(!?)
 * array overlap indicates which sections in de correspond to
 * lines that are different in all three files.
*/
struct diff de[NC];
char overlap[NC];
int  overlapcnt =0;

char line[2048];
FILE *fp[3];
/*	the number of the last-read line in each file
 *	is kept in cline[0-2]
*/
int cline[3];
/*	the latest known correspondence between line
 *	numbers of the 3 files is stored in last[1-3]
*/
int last[4];
int eflag;
int oflag;      /* indicates whether to mark overlaps (-E or -X)*/
int rflag;
int iflag;
#ifdef debug
int debug = 1;
#else
int debug = 0;
#endif /* debug */
char f1mark[100], f3mark[100]; /*markers for -E and -X*/
char * def = NULL;
char * defbegin = "#if	%s\n";
char * defelse = "#else	%s\n";
char * defend = "#endif	%s\n";


main(argc,argv)
char **argv;
{
	register i,m,n;
        eflag=0; oflag=0; rflag=0; iflag=0;

	while((argv[1] != NULL) && (*argv[1]=='-')) {
		switch(argv[1][1]) {
		default:
			eflag = 3;
			break;
		case '3':
			eflag = 2;
			break;
		case 'x':
			eflag = 1;
                        break;
                case 'E':
                        eflag = 3;
                        oflag = 1;
                        break;
                case 'X':
                        oflag = eflag = 1;
                        break;
		case 'i':
			iflag = 1;
		case 'r':
			rflag = 1;
			oflag = 1;
			break;
	        case 'D':
			def = argv[1]+2;
			break;
		}
		argv++;
		argc--;
	}
	if(argc<6) {
		fprintf(stderr,"diff3: arg count\n");
		exit(1);
	}
        if (oflag) { 
                (void)sprintf(f1mark,"<<<<<<< %s",argc>=7?argv[6]:argv[3]);
                (void)sprintf(f3mark,">>>>>>> %s",argc>=8?argv[7]:argv[5]);
        }

	m = readin(argv[1],d13);
	n = readin(argv[2],d23);
	for(i=0;i<=2;i++)
		if((fp[i] = fopen(argv[i+3],"r")) == NULL) {
			printf("diff3: can't open %s\n",argv[i+3]);
			exit(1);
		}
	merge(m,n);

	if( rflag )
		exit( overlapcnt > 254 ? 254 : overlapcnt );
	else
		exit(0);
}

/*pick up the line numbers of all changes from
 * one change file
 * (this puts the numbers in a vector, which is not
 * strictly necessary, since the vector is processed
 * in one sequential pass. The vector could be optimized
 * out of existence)
*/

readin(name,dd)
char *name;
struct diff *dd;
{
	register i;
	int a,b,c,d;
	char kind;
	char *p;
	fp[0] = fopen(name,"r");
	if(fp[0] == NULL) {
		printf("diff3: can't open %s\n",name);
		exit(1);
	}
	for(i=0;getchange(fp[0]);i++) {
		if(i>=NC) {
			fprintf(stderr,"diff3: too many changes\n");
			exit(1);
		}
		p = line;
		a = b = number(&p);
		if(*p==',') {
			p++;
			b = number(&p);
		}
		kind = *p++;
		c = d = number(&p);
		if(*p==',') {
			p++;
			d = number(&p);
		}
		if(kind=='a')
			a++;
		if(kind=='d')
			c++;
		b++;
		d++;
		dd[i].old.from = a;
		dd[i].old.to = b;
		dd[i].new.from = c;
		dd[i].new.to = d;
	}
	dd[i].old.from = dd[i-1].old.to;
	dd[i].new.from = dd[i-1].new.to;
	(void)fclose(fp[0]);
	return(i);
}

number(lc)
char **lc;
{
	register nn;
	nn = 0;
	while(digit(**lc))
		nn = nn*10 + *(*lc)++ - '0';
	return(nn);
}

digit(c)
{
	return(c>='0'&&c<='9');
}

getchange(b)
FILE *b;
{
	while(getline(b))
		if(digit(line[0]))
			return(1);
	return(0);
}

getline(b)
FILE *b;
{
	register i, c;
	for(i=0;i<sizeof(line)-1;i++) {
		c = getc(b);
		if(c==EOF)
			break;
		line[i] = c;
		if(c=='\n') {
			line[++i] = 0;
			return(i);
		}
	}
	return(0);
}

merge(m1,m2)
{
	register struct diff *d1, *d2, *d3;
	int dup;
	int j;
	int t1,t2;
	d1 = d13;
	d2 = d23;
	j = 0;
	for(;(t1 = d1<d13+m1) | (t2 = d2<d23+m2);) {
		if(debug) {
			printf("%d,%d=%d,%d %d,%d=%d,%d\n",
			d1->old.from,d1->old.to,
			d1->new.from,d1->new.to,
			d2->old.from,d2->old.to,
			d2->new.from,d2->new.to);
		}
/*			first file is different from others*/
		if(!t2||t1&&d1->new.to < d2->new.from) {
/*			stuff peculiar to 1st file */
dprintf( ("first file change\n") );
			if (rflag) {
				print_to(3, &d1->new);
				if (def) printf( defbegin, def );
				print_range(1, &d1->old);
				if (def) printf( defend, def );
			} else
			if(eflag==0) {
				separate("1");
				change(1,&d1->old,0);
				keep(2,&d1->new);
				change(3,&d1->new,0);
			}
			d1++;
			continue;
		}
/*			second file is different from others*/
		if(!t1||t2&&d2->new.to < d1->new.from) {
dprintf( ("second file change\n") );
			if (rflag) {
				print_to(3, &d2->new);
				print_range(2, &d2->old);
			} else
			if(eflag==0) {
				separate("2");
				keep(1,&d2->new);
				change(2,&d2->old,0);
				change(3,&d2->new,0);
			}
			d2++;
			continue;
		}
/*			merge overlapping changes in first file
 *			this happens after extension see below*/
		if(d1+1<d13+m1 &&
		   d1->new.to>=d1[1].new.from) {
			d1[1].old.from = d1->old.from;
			d1[1].new.from = d1->new.from;
			d1++;
			continue;
		}
/*			merge overlapping changes in second*/
		if(d2+1<d23+m2 &&
		   d2->new.to>=d2[1].new.from) {
			d2[1].old.from = d2->old.from;
			d2[1].new.from = d2->new.from;
			d2++;
			continue;
		}
/*			stuff peculiar to third file or different in all*/
		if(d1->new.from==d2->new.from&&
		   d1->new.to==d2->new.to) {
dprintf( ("overlapping change\n") );
			dup = duplicate(&d1->old,&d2->old);
/*				dup=0 means all files differ
 *				dup =1 meands files 1&2 identical*/
			if(rflag) {
				print_to( 3, &d2->new );
				if( dup )
					print_range(2,&d2->old);
				else {

					if( ! iflag ) goto non_interactive;
					narrate( "%s (%d,%d)\n", f1mark, d1->new.to, 
						d1->new.from );
					narrate_range( 1, &d1->old );
					narrate( "======\n" );
					narrate_range( 2, &d2->old );
					narrate( "%s\n", f3mark );
					switch( inquire( "<>mq[b]? ", "bm<>q" ) ) {
					    case '<':
						if (def) printf( defbegin, def );
						print_range( 1, &d1->old );
						if (def) printf( defend, def );
						break;

					    case '>':
						print_range( 2, &d2->old );
						break;

					    case 'q':
						exit( -1 );

					    case 'b':
						if (def) printf( defbegin, def );
						print_range( 1, &d1->old );
						if (def) printf( defelse, def );
						print_range( 2, &d2->old );
						if (def) printf( defend, def );
						break;

					    case 'm':
					    non_interactive:
						overlapcnt++;
						printf( "%s\n", f1mark );
						print_range( 1, &d1->old );
						printf( "======\n" );
						print_range( 2, &d2->old );
						printf( "%s\n", f3mark );
						break;
					}
				}
			} else
			if(eflag==0) {
				separate(dup?"3":"");
				change(1,&d1->old,dup);
				change(2,&d2->old,0);
				d3 = d1->old.to>d1->old.from?d1:d2;
				change(3,&d3->new,0);
			} else
				j = edit(d1,dup,j);
			d1++;
			d2++;
			continue;
		}
/*			overlapping changes from file1 & 2
 *			extend changes appropriately to
 *			make them coincide*/
		 if(d1->new.from<d2->new.from) {
			d2->old.from -= d2->new.from-d1->new.from;
			d2->new.from = d1->new.from;
		}
		else if(d2->new.from<d1->new.from) {
			d1->old.from -= d1->new.from-d2->new.from;
			d1->new.from = d2->new.from;
		}
		if(d1->new.to >d2->new.to) {
			d2->old.to += d1->new.to - d2->new.to;
			d2->new.to = d1->new.to;
		}
		else if(d2->new.to >d1->new.to) {
			d1->old.to += d2->new.to - d1->new.to;
			d1->new.to = d2->new.to;
		}
	}

	if(rflag)
		print_to_eof(3);
	if(eflag)
		edscript(j);
}

separate(s)
char *s;
{
	printf("====%s\n",s);
}

/*	the range of ines rold.from thru rold.to in file i
 *	is to be changed. it is to be printed only if
 *	it does not duplicate something to be printed later
*/
change(i,rold,dup)
struct range *rold;
{
	printf("%d:",i);
	last[i] = rold->to;
	prange(rold);
	if(dup)
		return;
	if(debug)
		return;
	i--;
	(void)skip(i,rold->from,(char *)0);
	(void)skip(i,rold->to,"  ");
}

/*	print the range of line numbers, rold.from  thru rold.to
 *	as n1,n2 or n1
*/
prange(rold)
struct range *rold;
{
	if(rold->to<=rold->from)
		printf("%da\n",rold->from-1);
	else {
		printf("%d",rold->from);
		if(rold->to > rold->from+1)
			printf(",%d",rold->to-1);
		printf("c\n");
	}
}

/*	no difference was reported by diff between file 1(or 2)
 *	and file 3, and an artificial dummy difference (trange)
 *	must be ginned up to correspond to the change reported
 *	in the other file
*/
keep(i,rnew)
struct range *rnew;
{
	register delta;
	struct range trange;
	delta = last[3] - last[i];
	trange.from = rnew->from - delta;
	trange.to = rnew->to - delta;
	change(i,&trange,1);
}

print_to(i,rold)
struct range *rold;
{
	(void)skip( i-1, rold->from, "" );
	(void)skip( i-1, rold->to, (char *)0 );
}

print_to_eof(i)
{
       	while(getline(fp[i-1])!=0) {
		printf("%s", line);
	}
}

print_range(i,rold)
struct range *rold;
{
	(void)skip( i-1, rold->from, (char *)0 );
	(void)skip( i-1, rold->to, "" );
}

narrate_range(i,rold)
struct range *rold;
{

	long pos;
	int line;

	pos = ftell( fp[i-1] );
	line = cline[i-1];
	(void)skip_fd( i-1, rold->from, (char *)0, stderr );
	(void)skip_fd( i-1, rold->to, "", stderr );
	fseek( fp[i-1], pos, 0 );
	cline[i-1] = line;

}

/*	skip to just before line number from in file i
 *	if "pr" is nonzero, print all skipped stuff
 * 	with string pr as a prefix
*/
skip_fd(i,from,pr,fd)
char *pr;
FILE *fd;
{
	register j,n;
	for(n=0;cline[i]<from-1;n+=j) {
		if((j=getline(fp[i]))==0) {
			trouble();
		}
		if(pr)
			fprintf(fd,"%s%s",pr,line);
		cline[i]++;
	}
	return(n);
}

skip(i,from,pr)
char *pr;
{
	return( skip_fd(i,from,pr,stdout) );
}

/*	return 1 or 0 according as the old range
 *	(in file 1) contains exactly the same data
 *	as the new range (in file 2)
*/
duplicate(r1,r2)
struct range *r1, *r2;
{
	register c,d;
	register nchar;
	int nline;
	if(r1->to-r1->from != r2->to-r2->from)
		return(0);
	(void)skip(0,r1->from,(char *)0);
	(void)skip(1,r2->from,(char *)0);
	nchar = 0;
	for(nline=0;nline<r1->to-r1->from;nline++) {
		do {
			c = getc(fp[0]);
			d = getc(fp[1]);
			if(c== -1||d== -1)
				trouble();
			nchar++;
			if(c!=d) {
				repos(nchar);
				return(0);
			}
		} while(c!= '\n');
	}
	repos(nchar);
	return(1);
}

repos(nchar)
{
	register i;
	for(i=0;i<2;i++) 
		(void)fseek(fp[i], (long)-nchar, 1);
}

trouble()
{
	fprintf(stderr,"diff3: logic error\n");
	abort();
}

narrate( fmt, a, b, c, d, e, f )
char *fmt;
{
	fprintf( stderr, fmt, a, b, c, d, e, f );
}

inquire( fmt, vals )
char *fmt;
char *vals;
{
	char * v;
	char buf[256];

	narrate( fmt );
	gets( buf );

	for( v=vals; *v; v++ )
		if( *v == buf[0] ) return( *v );

	return( *vals );
}

/*	collect an editing script for later regurgitation
*/
edit(diff,dup,j)
struct diff *diff;
{
	if(((dup+1)&eflag)==0)
		return(j);
	j++;
        overlap[j] = !dup;
        if (!dup) overlapcnt++;
	de[j].old.from = diff->old.from;
	de[j].old.to = diff->old.to;
	de[j].new.from = de[j-1].new.to
	    +skip(2,diff->new.from,(char *)0);
	de[j].new.to = de[j].new.from
	    +skip(2,diff->new.to,(char *)0);
	return(j);
}

/*		regurgitate */
edscript(n)
{
	register j,k;
	char block[BUFSIZ];
	for(n=n;n>0;n--) {
                if (!oflag || !overlap[n]) 
                        prange(&de[n].old);
                else
                        printf("%da\n=======\n", de[n].old.to -1);
		(void)fseek(fp[2], (long)de[n].new.from, 0);
		for(k=de[n].new.to-de[n].new.from;k>0;k-= j) {
			j = k>BUFSIZ?BUFSIZ:k;
			if(fread(block,1,j,fp[2])!=j)
				trouble();
			(void)fwrite(block, 1, j, stdout);
		}
                if (!oflag || !overlap[n]) 
                        printf(".\n");
                else {
                        printf("%s\n.\n",f3mark);
                        printf("%da\n%s\n.\n",de[n].old.from-1,f1mark);
                }
	}
        exit(overlapcnt > 254 ? 254 : overlapcnt);
}
