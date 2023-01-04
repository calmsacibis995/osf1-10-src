/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: s5mkfs.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:27:31 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*	mkfs	COMPILE:	cc -O mkfs.c -s -i -o mkfs
 * Make a file system prototype.
 * usage: mkfs filsys blocksize:size[:inodes] [gap blocks/cyl]
 *        mkfs filsys proto [gap blocks/cyl]
 */

#include <stdio.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <s5fs/s5param.h>
#include <s5fs/filsys.h>
#include <s5fs/s5ino.h>
#include <s5fs/s5dir.h>

#include <s5fs/s5inode.h>
#include <s5fs/s5fblk.h>


#define MAX_BSIZE 2048		/* max BSIZE on OSF/1 */
#define MID_BSIZE 1024		/* another BSIZE on OSF/1 */
#define MIN_BSIZE 512		/* min BSIZE on OSF/1 */


/* file system block size passed in as parameter.
	FSBSIZE needs to be a variable and assigned to this blk size 
		parameter
	NIDIR
	NFB
	NDIRECT
	NBINODE
*/


/* boot-block size */
#define BBSIZE	512
/* super-block size */
#define SBSIZE	512

#define	LADDR	10
#define	STEPSIZE	7
#define	CYLSIZE		400
#define	MAXFN	1000

long	FSBSIZE;		/* keep the same name - all caps -
				   less changes */
long	NBINODE	;
long	NDIRECT;
long	NIDIR;
long	NFB;

time_t	utime;
FILE 	*fin;
int	fsi;
int	fso;
char	*charp;
char	buf[MAX_BSIZE ];	/* was sized by FSBSIZE */

int work0[MAX_BSIZE * 4/sizeof(int)];	/* was sized by FSBSIZE */
struct fblk *fbuf = (struct fblk *)work0;


char	string[512];

int work1[MAX_BSIZE /sizeof(int)]; /* was sized by FSBSIZE */
struct filsys *filsys = (struct filsys *)work1;
char	*fsys;
char	*proto;
int	f_n = CYLSIZE;
int	f_m = STEPSIZE;
int	error;
ino_t	ino;
long	getnum();
daddr_t	alloc();


main(argc, argv)
int argc;
char *argv[];
{
	int f, c;
	long n, nb, blocksize;
	int getting_blksize, filsys_type;
	struct stat statarea;
	struct {
		daddr_t tfree;
		ino_t tinode;
		char fname[6];
		char fpack[6];
	} ustatarea;

	/*
	 * open relevent files
	 */

	time(&utime);
	if(argc < 3) {
		printf("usage: %s filsys proto [gap blocks/cyl]\n       %s filsys blocksize:blocks[:inodes] [gap blocks/cyl]\n", argv[0], argv[0]);
		exit(1);
	}
	fsys = argv[1];
	if(stat(fsys, &statarea) < 0) {
		printf("%s: cannot stat\n",fsys);
		exit(1);
	}
	proto = argv[2];
	fsi = open(fsys, 0);
	if(fsi < 0) {
		printf("%s: cannot open\n", fsys);
		exit(1);
	}

#ifdef CHECK_MOUNT	  /* we are not checking mounted fs for now */
	if((statarea.st_mode & S_IFMT) == S_IFBLK)
		if(ustat(statarea.st_rdev,&ustatarea) >= 0) { 
			printf("*** MOUNTED FILE SYSTEM\n");
			exit(1);
		}
#endif CHECK_MOUNT

	printf("Mkfs: %s? \n(DEL if wrong)\n", fsys);
	sleep(10);	/* 10 seconds to DEL */
	fso = creat(fsys, 0666);
	if(fso < 0) {
		printf("%s: cannot create\n", fsys);
		exit(1);
	}
	fin = fopen(proto, "r");
	if(fin == NULL) {
		getting_blksize = 1;
		nb = n = 0;
		for(f=0; c=proto[f]; f++) {
			if(c<'0' || c>'9') {
				if(c == ':') {
				    if (getting_blksize ) {
					getting_blksize =0;
					FSBSIZE = n;
					n = 0;
					continue;
				    }
				    else {
					nb = n;
					n = 0;
					continue;
				    }
				}
				printf("%s: cannot open\n", proto);
				exit(1);
			}
			n = n*10 + (c-'0');
		}
		if (!FSBSIZE) {
			printf("default block size is %d\n",MIN_BSIZE);
			FSBSIZE=MIN_BSIZE;
		}
		switch (FSBSIZE) {
		case MAX_BSIZE:
			filsys_type = Fs3b;
			break;
		case MID_BSIZE:
			filsys_type = Fs2b;
			break;
		case MIN_BSIZE:
			filsys_type = Fs1b;
			break;
		default:
			printf("illegal block size, default blk size used is %d\n",MIN_BSIZE);
			FSBSIZE = MIN_BSIZE;
			filsys_type = Fs1b;
			break;
		}
		NBINODE	=FSBSIZE/sizeof(struct s5dinode);
		NDIRECT= FSBSIZE/sizeof(struct s5direct);
		NIDIR	= FSBSIZE/sizeof(daddr_t);
		NFB	= NIDIR+500;	/* NFB must be greater than NIDIR+LADDR */
		if (!nb){
			nb = n;
			n = nb/(NBINODE*4);
		}
		else {
			n /= NBINODE;
		}
		filsys->s_fsize = nb;
		if(n <= 0)
			n = 1;
		if(n > 65500/NBINODE)
			n = 65500/NBINODE;
		filsys->s_isize = n + 2;

		charp = "d--777 0 0 $ ";
		goto f3;
	}

	/*
	 * get name of boot load program
	 * and read onto block 0
	 * NOTE: not doing anything with the boot block right now
	 */

	printf("Not doing anything with boot load program\n");
	getstr();

	/*
	 * get total disk size
	 * and inode block size
	 */

f2:
	FSBSIZE = getnum();

	if (!FSBSIZE) {
		printf("default block size is %d\n",MIN_BSIZE);
		FSBSIZE=MIN_BSIZE;
	}
	switch (FSBSIZE) {
	case MAX_BSIZE:
		filsys_type = Fs3b;
		break;
	case MID_BSIZE:
		filsys_type = Fs2b;
		break;
	case MIN_BSIZE:
		filsys_type = Fs1b;
		break;
	default:
		printf("illegal block size, default blk size used is %d\n",MIN_BSIZE);
		FSBSIZE = MIN_BSIZE;
		filsys_type = Fs1b;
		break;
	}

	NBINODE	=FSBSIZE/sizeof(struct s5dinode);
	NDIRECT= FSBSIZE/sizeof(struct s5direct);
	NIDIR	= FSBSIZE/sizeof(daddr_t);
	NFB	= NIDIR+500;	/* NFB must be greater than NIDIR+LADDR */

	nb = getnum();
	filsys->s_fsize = nb ;
	n = getnum();
	n /= NBINODE;  
	filsys->s_isize = n + 2;

f3:
	/* set magic number for file system type */
	filsys->s_magic = FsMAGIC;
	filsys->s_type = filsys_type;
	if(argc >= 5) {
		f_m = atoi(argv[3]);
		f_n = atoi(argv[4]);
		if(f_n <= 0 || f_n >= MAXFN)
			f_n = CYLSIZE;
		if(f_m <= 0 || f_m > f_n)
			f_m = STEPSIZE;
	}
	filsys->s_dinfo[0] = f_m;
	filsys->s_dinfo[1] = f_n;

	printf("bytes per logical block = %d\n", FSBSIZE);
	printf("total logical blocks = %ld\n", filsys->s_fsize);
	printf("total inodes = %ld\n", n*NBINODE);
	printf("gap (physical blocks) = %d\n", filsys->s_dinfo[0]);
	printf("cylinder size (physical blocks) = %d \n", filsys->s_dinfo[1]);

	if(filsys->s_isize >= filsys->s_fsize) {
		printf("%ld/%ld: bad ratio\n", filsys->s_fsize, filsys->s_isize-2);
		exit(1);
	}
	filsys->s_tinode = 0;
	for(c=0; c<FSBSIZE; c++)
		buf[c] = 0;
	for(n=2; n!=filsys->s_isize; n++) {
		wtfs(n, buf);
		filsys->s_tinode += NBINODE;
	}
	ino = 0;

	bflist();

	cfile((struct s5inode *)0);

	filsys->s_time = utime;

/* write super-block onto file system */
	lseek(fso, (long)SUPERBOFF, 0);
	if(write(fso, (char *)filsys, SBSIZE) != SBSIZE) {
		printf("write error: super-block\n");
		exit(1);
	}

	exit(error);
}

cfile(par)
struct s5inode *par;
{
	struct s5inode in;
	daddr_t bn, nblk;
	int dbc, ibc;
	char db[MAX_BSIZE];		/* was sized by FSBSIZE */
	daddr_t ib[MAX_BSIZE/sizeof(daddr_t) + 500];	/* was sized by NFB */
	int i, f, c;

	/*
	 * get mode, uid and gid
	 */

	getstr();

	in.i_mode  = get_file_mode(string[0]);
	in.i_mode |= get_setuid(string[1]);
	in.i_mode |= get_setgid(string[2]);

	for(i=3; i<6; i++) {
		c = string[i];
		if(c<'0' || c>'7') {
			printf("%c/%s: bad octal mode digit\n", c, string);
			error = 1;
			c = 0;
		}
		in.i_mode |= (c-'0')<<(15-3*i);
	}
	in.i_uid = getnum();
	in.i_gid = getnum();

	/*
	 * general initialization prior to
	 * switching on format
	 */

	ino++;
	in.i_number = ino;
	for(i=0; i<FSBSIZE; i++)
		db[i] = 0;
	for(i=0; i<NFB; i++)
		ib[i] = (daddr_t)0;
	in.i_nlink = 1;
	in.i_size = 0;
	for(i=0; i<NADDR; i++)
		in.i_faddr[i] = (daddr_t)0;
	if(par == (struct s5inode *)0) {
		par = &in;
		in.i_nlink--;
	}
	dbc = 0;
	ibc = 0;
	switch(in.i_mode&S5IFMT) {

	case S5IFREG:
		/*
		 * regular file
		 * contents is a file name
		 */

		getstr();
		f = open(string, 0);
		if(f < 0) {
			printf("%s: cannot open\n", string);
			error = 1;
			break;
		}
		while((i=read(f, db, FSBSIZE)) > 0) {
			in.i_size += i;
			newblk(&dbc, db, &ibc, ib);
		}
		close(f);
		break;

	case S5IFBLK:
	case S5IFCHR:
		/*
		 * special file
		 * content is maj/min types
		 */

		i = getnum() & 0377;
		f = getnum() & 0377;
		in.i_faddr[0] = (i<<8) | f;
		break;

	case S5IFDIR:
		/*
		 * directory
		 * put in extra links
		 * call recursively until
		 * name of "$" found
		 */

		par->i_nlink++;
		in.i_nlink++;
		entry(par->i_number, "..", &dbc, db, &ibc, ib);
		in.i_size = 2*sizeof(struct s5direct);
		entry(in.i_number, ".", &dbc, db, &ibc, ib);
		for(;;) {
			getstr();
			if(string[0]=='$' && string[1]=='\0')
				break;
			entry(ino+1, string, &dbc, db, &ibc, ib);
			in.i_size += sizeof(struct s5direct);
			cfile(&in);
		}
		break;

	}
	if(dbc != 0)
		newblk(&dbc, db, &ibc, ib);
	iput(&in, &ibc, ib);
}

get_file_mode(s)
char s;
{
      if (s == '-')
	return(S5IFREG);
      if (s == 'b')
	return(S5IFBLK);
      if (s == 'c')
	return(S5IFCHR);
      if (s == 'd')
	return(S5IFDIR);

	return(0);

}


get_setuid(s)
char s;
{
  if (s == 'u')
        return (S5ISUID);
  return(0);
}

get_setgid(s)
char s;
{
  if (s == 'g')
        return(S5ISGID);
  return(0);
}


long getnum()
{
	int i, c;
	long n;

	getstr();
	n = 0;
	for(i=0; c=string[i]; i++) {
		if(c<'0' || c>'9') {
			printf("%s: bad number\n", string);
			error = 1;
			return((long)0);
		}
		n = n*10 + (c-'0');
	}
	return(n);
}

getstr()
{
	int i, c;

loop:
	switch(c=getch()) {

	case ' ':
	case '\t':
	case '\n':
		goto loop;

	case '\0':
		printf("EOF\n");
		exit(1);

	case ':':
		while(getch() != '\n');
		goto loop;

	}
	i = 0;

	do {
		string[i++] = c;
		c = getch();
	} 
	while(c!=' '&&c!='\t'&&c!='\n'&&c!='\0');
	string[i] = '\0';
}

rdfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	lseek(fsi, (long)(bno*FSBSIZE), 0);
	n = read(fsi, bf, FSBSIZE);
	if(n != FSBSIZE) {
		printf("read error: %ld\n", bno);
		exit(1);
	}
}

wtfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	lseek(fso, (long)(bno*FSBSIZE), 0);
	n = write(fso, bf, FSBSIZE);
	if(n != FSBSIZE) {
		printf("write error: %ld\n", bno);
		exit(1);
	}
}

daddr_t alloc()
{
	int i;
	daddr_t bno;

	filsys->s_tfree--;
	bno = filsys->s_free[--filsys->s_nfree];
	if(bno == 0) {
		printf("out of free space\n");
		exit(1);
	}
	if(filsys->s_nfree <= 0) {
		rdfs(bno, (char *)fbuf);
		filsys->s_nfree = fbuf->df_nfree;
		for(i=0; i<NICFREE; i++)
			filsys->s_free[i] = fbuf->df_free[i];
	}
	return(bno);
}


bfree(bno)
daddr_t bno;
{
	int i;

	filsys->s_tfree++;
	if(filsys->s_nfree >= NICFREE) {
		fbuf->df_nfree = filsys->s_nfree;
		for(i=0; i<NICFREE; i++)
			fbuf->df_free[i] = filsys->s_free[i];
		wtfs(bno, (char *)fbuf);
		filsys->s_nfree = 0;
	}
	filsys->s_free[filsys->s_nfree++] = bno;
}

entry(in, str, adbc, db, aibc, ib)
ino_t in;
char *str;
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	struct s5direct *dp;
	int i;

	dp = (struct s5direct *)db;
	dp += *adbc;
	(*adbc)++;
	dp->d_ino = in;
	for(i=0; i<s5DIRSIZ; i++)
		dp->d_name[i] = 0;
	for(i=0; i<s5DIRSIZ; i++)
		if((dp->d_name[i] = str[i]) == 0)
			break;
	if(*adbc >= NDIRECT)
		newblk(adbc, db, aibc, ib);
}

newblk(adbc, db, aibc, ib)
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	int i;
	daddr_t bno;

	bno = alloc();
	wtfs(bno, db);
	for(i=0; i<FSBSIZE; i++)
		db[i] = 0;
	*adbc = 0;
	ib[*aibc] = bno;
	(*aibc)++;
	if(*aibc >= NFB) {
		printf("file too large\n");
		error = 1;
		*aibc = 0;
	}
}

getch()
{

	if(charp)
		return(*charp++);
	return(getc(fin));
}

bflist()
{
	struct s5inode in;
	daddr_t ib[MAX_BSIZE/sizeof(daddr_t) + 500];	/* was sized by NFB */
	int ibc;
	char flg[MAXFN];
	int adr[MAXFN];
	int i, j;
	daddr_t f, d;

	for(i=0; i<f_n; i++)
		flg[i] = 0;
	i = 0;
	for(j=0; j<f_n; j++) {
		while(flg[i])
			i = (i+1)%f_n;
		adr[j] = i+1;
		flg[i]++;
		i = (i+f_m)%f_n;
	}

	ino++;
	in.i_number = ino;
	in.i_mode = S5IFREG;
	in.i_uid = 0;
	in.i_gid = 0;
	in.i_nlink = 0;
	in.i_size = 0;
	for(i=0; i<NADDR; i++)
		in.i_faddr[i] = (daddr_t)0;

	for(i=0; i<NFB; i++)
		ib[i] = (daddr_t)0;
	ibc = 0;
	bfree((daddr_t)0);
	filsys->s_tfree = 0;
	d = filsys->s_fsize-1;
	while(d%f_n)
		d++;
	for(; d > 0; d -= f_n)
		for(i=0; i<f_n; i++) {
			f = d - adr[i];
			if(f < filsys->s_fsize && f >= filsys->s_isize)
				if(badblk(f)) {
					if(ibc >= NIDIR) {
						printf("too many bad blocks\n");
						error = 1;
						ibc = 0;
					}
					ib[ibc] = f;
					ibc++;
				} else {
					bfree(f);
				}
		}
	iput(&in, &ibc, ib);
}

iput(ip, aibc, ib)
register struct s5inode *ip;
register int *aibc;
daddr_t *ib;
{
	register struct s5dinode *dp;
	register char *p1, *p2;
	daddr_t d;
	register int i,j,k;
	daddr_t ib2[MAX_BSIZE/sizeof(daddr_t)];	/* a double indirect block */
						/* was sized by NIDIR */
	daddr_t ib3[MAX_BSIZE/sizeof(daddr_t)];	/* a triple indirect block */
						/* was sized by NIDIR */

	filsys->s_tinode--;
	d = (daddr_t)FsITOD(FSBSIZE,ip->i_number);
	if(d >= filsys->s_isize) {
		if(error == 0)
			printf("ilist too small\n");
		error = 1;
		return;
	}
	rdfs(d, buf);
	dp = (struct s5dinode *)buf;
	dp += FsITOO(FSBSIZE,ip->i_number);
	dp->di_mode = ip->i_mode;
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	dp->di_atime = utime;
	dp->di_mtime = utime;
	dp->di_ctime = utime;

	switch(ip->i_mode&S5IFMT) {

	case S5IFDIR:
	case S5IFREG:
		/* handle direct pointers */
		for(i=0; i<*aibc && i<LADDR; i++) {
			ip->i_faddr[i] = ib[i];
			ib[i] = 0;
		}
		/* handle single indirect block */
		if(i < *aibc)
		{
			for(j=0; i<*aibc && j<NIDIR; j++, i++)
				ib[j] = ib[i];
			for(; j<NIDIR; j++)
				ib[j] = 0;
			ip->i_faddr[LADDR] = alloc();
			wtfs(ip->i_faddr[LADDR], (char *)ib);
		}
		/* handle double indirect block */
		if(i < *aibc)
		{
			for(k=0; k<NIDIR && i<*aibc; k++)
			{
				for(j=0; i<*aibc && j<NIDIR; j++, i++)
					ib[j] = ib[i];
				for(; j<NIDIR; j++)
					ib[j] = 0;
				ib2[k] = alloc();
				wtfs(ib2[k], (char *)ib);
			}
			for(; k<NIDIR; k++)
				ib2[k] = 0;
			ip->i_faddr[LADDR+1] = alloc();
			wtfs(ip->i_faddr[LADDR+1], (char *)ib2);
		}
		/* handle triple indirect block */
		if(i < *aibc)
		{
			printf("triple indirect blocks not handled\n");
		}
		break;

	case S5IFBLK:
		break;

	case S5IFCHR:
		break;


	default:
		printf("bad mode %o\n", ip->i_mode);
		exit(1);
	}

	/*	ltol3(dp->di_addr, ip->i_faddr, NADDR);	*/



	p1 = (char *)dp->di_addr;
	p2 = (char *)ip->i_addr;

	for(i=0; i<NADDR; i++) {
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2++;
		if(*p2++ != 0)
			printf("iaddress > 2^24\n");
	}

	wtfs(d, buf);
}

badblk(bno)
daddr_t bno;
{

	return(0);
}

