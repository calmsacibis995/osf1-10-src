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
static char *rcsid = "@(#)$RCSfile: cdl.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/12/21 18:52:21 $";
#endif


/*

        HEY, LOOK OUT!

        This program should be merged with isl/pickapart/pickapart.c

        IF YOU'RE GOING TO WORK ON THIS PROGRAM, CONSIDER DOING
        THE MERGE. IF NOT, INSPECT isl/pickapart/pickapart.c TO
		MAKE SURE THAT CHANGES ARE NOT REQUIRED THERE ALSO

*/


#include <sys/secdefines.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#if	BSD > 43
#include <ufs/fs.h>
#else
#include <sys/fs.h>
#endif
#include <strings.h>
#define DKTYPENAMES
#include <sys/disklabel.h>

#ifdef vax
#define RAWPARTITION	'c'
#else
#define RAWPARTITION	'a'
#endif

#ifndef BBSIZE
#define	BBSIZE	8192			/* size of boot area, with label */
#endif

#define	P(X)	('a'+(X))

void	PrintLabel();
char	*Overlap();

extern	int errno;
char	specname[80];
struct	disklabel lab;
struct	disklabel *readlabel();
char	bootarea[BBSIZE];
char	boot0[MAXPATHLEN];
char	boot1[MAXPATHLEN];

#ifdef DEBUG
int	debug;
#endif

main(argc, argv)
int argc;
char *argv[];
{
	register struct disklabel *dp;
	int	f;

	if (argc < 3)
		usage();

	switch (argv[1][1])
	{
		case 'D':
			/*
			 * We check getdiskbyname() to see if an entry is in
			 * /etc/disktab.  If not, we use the device specified
			 * to attempt to create a disk label using the
			 * creatediskbyname() routine.  
			 */
			if (dp = getdiskbyname(argv[2])) {
				PrintLabel(dp);
			} else {
				(void)sprintf(specname, "/dev/r%s%c", argv[3],
					RAWPARTITION);
				if (dp = creatediskbyname(specname)) 
					PrintLabel(dp);
			}
			break;

		case 'E':
			(void)sprintf(specname, "/dev/r%s%c", argv[2],
					RAWPARTITION);
			f = open(specname, O_RDONLY);
			if (f < 0)
				Perror(specname);
			dp = readlabel(f);
			if(checklabel(dp))
				exit(1);

			PrintLabel( dp );
			break;
		default:
			usage();
	}
	exit(0);
}


void PrintLabel( dp )
struct disklabel *dp;
{
	register int			i;
	register struct partition	*pt;
	static char			*dl = "------------------";		/* '-' * 18 */
	double				x;	/* sector blocking factor */

	x = dp->d_secsize / 512.0;
	printf( "partition\t%9s%9s%9s\t%s\n",
		"start", "size", "end", "overlap" );

	printf( "%s%s%s%s\n", dl, dl, dl, dl );

	pt = dp->d_partitions;

	for (i = 0; i < dp->d_npartitions; i++, pt++)
	{
		double	offset = pt->p_offset * x;
		double	size = pt->p_size * x;

		printf( "    %c\t  ---\t%9.0f %9.0f%9.0f", P(i),
			offset, size, offset+size );

		printf( "\t  %s\n", Overlap( i, dp ) );

	}
	printf( "%s%s%s%s\n", dl, dl, dl, dl);
}



char *Overlap( i, lp )
u_short i;
struct disklabel *lp;
{
	struct partition	*pp;
	u_short			j;
	static char		outbuf[BUFSIZ];
	char			*bp;
	u_long			start, end;
	short			comma = 0;

	bp = outbuf;
	pp = lp->d_partitions;

	start = pp[i].p_offset;
	end = start + pp[i].p_size;

	for( j = 0; j < MAXPARTITIONS; ++j )
	{
		if( j == i )
			continue;

		if( end <= pp[j].p_offset ||
			start >= (pp[j].p_offset + pp[j].p_size) )
		{
			/* no overlap */
			continue;
		}
		if( comma )
		{
			*bp++ = ',';
			*bp++ = ' ';
		}
		else
			++comma;

		*bp++ = P(j);
	}
	*bp = '\0';
	return( outbuf );
}	

/*
 * Fetch disklabel for disk.
*/

struct disklabel *readlabel(f)
int f;
{
	register struct disklabel *lp;

	if (read(f, bootarea, BBSIZE) < BBSIZE)
		Perror(specname);

	for (lp = (struct disklabel *)bootarea;
		lp <= (struct disklabel *)(bootarea + BBSIZE - sizeof(*lp));
				lp = (struct disklabel *)((char *)lp + 16))
		if (lp->d_magic == DISKMAGIC &&
					lp->d_magic2 == DISKMAGIC)
				break;

	if (lp > (struct disklabel *)(bootarea+BBSIZE-sizeof(*lp)) ||
		    lp->d_magic != DISKMAGIC || lp->d_magic2 != DISKMAGIC ||
		    dkcksum(lp) != 0) {
			fprintf(stderr,
	"Bad pack magic number (label is damaged, or pack is unlabeled)\n");
			/* lp = (struct disklabel *)(bootarea + LABELOFFSET); */
			exit (1);
	}

	return (lp);
}


/*
 * Check disklabel for errors and fill in
 * derived fields according to supplied values.
 */
checklabel(lp)
register struct disklabel *lp;
{
	register struct partition *pp;
	int i, errors = 0;
	char part;

	if (lp->d_secsize == 0) {
		fprintf(stderr, "sector size %d\n", lp->d_secsize);
		return (1);
	}
	if (lp->d_nsectors == 0) {
		fprintf(stderr, "sectors/track %d\n", lp->d_nsectors);
		return (1);
	}
	if (lp->d_ntracks == 0) {
		fprintf(stderr, "tracks/cylinder %d\n", lp->d_ntracks);
		return (1);
	}
	if  (lp->d_ncylinders == 0) {
		fprintf(stderr, "cylinders/unit %d\n", lp->d_ncylinders);
		errors++;
	}
	if (lp->d_rpm == 0)
		Warning("revolutions/minute %d\n", lp->d_rpm);
	if (lp->d_secpercyl == 0)
		lp->d_secpercyl = lp->d_nsectors * lp->d_ntracks;
	if (lp->d_secperunit == 0)
		lp->d_secperunit = lp->d_secpercyl * lp->d_ncylinders;
	if (lp->d_bbsize == 0) {
		fprintf(stderr, "boot block size %d\n", lp->d_bbsize);
		errors++;
	} else if (lp->d_bbsize % lp->d_secsize)
		Warning("boot block size %% sector-size != 0\n");
	if (lp->d_sbsize == 0) {
		fprintf(stderr, "super block size %d\n", lp->d_sbsize);
		errors++;
	} else if (lp->d_sbsize % lp->d_secsize)
		Warning("super block size %% sector-size != 0\n");
	if (lp->d_npartitions > MAXPARTITIONS)
		Warning("number of partitions (%d) > MAXPARTITIONS (%d)\n",
		    lp->d_npartitions, MAXPARTITIONS);
	for (i = 0; i < lp->d_npartitions; i++) {
		part = P(i);
		pp = &lp->d_partitions[i];
		if (pp->p_size == 0 && pp->p_offset != 0)
			Warning("partition %c: size 0, but offset %d\n",
			    part, pp->p_offset);
		if (pp->p_offset > lp->d_secperunit) {
			fprintf(stderr,
			    "partition %c: offset past end of unit\n", part);
			errors++;
		}
		if (pp->p_offset + pp->p_size > lp->d_secperunit) {
			fprintf(stderr,
			    "partition %c: partition extends past end of unit\n", part);
			errors++;
		}
	}
	for (; i < MAXPARTITIONS; i++) {
		part = P(i);
		pp = &lp->d_partitions[i];
		if (pp->p_size || pp->p_offset)
			Warning("unused partition %c: size %d offset %d\n",
			    P(i), pp->p_size, pp->p_offset);
	}
	return (errors);
}

/*VARARGS1*/
Warning(fmt, a1, a2, a3, a4, a5)
char *fmt;
{

	fprintf(stderr, "Warning, ");
	fprintf(stderr, fmt, a1, a2, a3, a4, a5);
	fprintf(stderr, "\n");
}

Perror(str)
char *str;
{
	fputs("pickapart: ", stderr); perror(str);
	exit(4);
}

usage()
{
	fprintf(stderr, "usage: cdl [-D disk_type] [-E device]\n");
	exit(1);
}

