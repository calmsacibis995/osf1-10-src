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
static char *rcsid = "@(#)$RCSfile: pickapart.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1994/01/03 20:21:32 $";
#endif

/*
 *  Modification History:  pickapart.c
 *
 *  09-Jan-92	Jeff Kenton
 * 		Initial version.
 * 		Modified and butchered disklabel.c
*/

/*

		HEY, LOOK OUT!

		This program should be merged with isl/cdl/cdl.c

		IF YOU'RE GOING TO WORK ON THIS PROGRAM, CONSIDER DOING
		THE MERGE. IF NOT, INSPECT isl/cdl/cdl.c TO MAKE SURE
		THAT CHANGES ARE NOT REQUIRED THERE ALSO

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

/*
 * pickapart: select a partition from specified disk.
 *
 *	Used by install.osf.  Avoids partitions which overlap
 *		those already in use.
*/

#ifdef vax
#define RAWPARTITION	'c'
#else
#define RAWPARTITION	'a'
#endif

#ifndef BBSIZE
#define	BBSIZE	8192			/* size of boot area, with label */
#endif

extern	int errno;
char	specname[80];
struct	disklabel lab;
struct	disklabel *readlabel();
void	display( FILE *, struct disklabel *);
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
	register struct disklabel *lp;
	int f, i, j;
	char	buf[BUFSIZ];

	if (argc < 2)
		usage();

	(void)sprintf(specname, "/dev/r%s%c", argv[1], RAWPARTITION);
	f = open(specname, O_RDONLY);
	if (f < 0)
		Perror(specname);

	lp = readlabel(f);
	if(checklabel(lp))
		exit(1);

	prune(argc, argv, lp);

	for(j = lp->d_npartitions; --j >= 0; )
		if(lp->d_partitions[j].p_size)
			break;

	if(j < 0)
	{

		fprintf(stderr, "\n\n*** No partitions available ***\n\n");
		exit(1);
	}

	while(1)
	{

		display(stderr, lp);

		fprintf(stderr,
			"\nEnter the letter specifying which partition to use: ");
		fflush(stderr);

		i = 0;
		gets( buf );
		sscanf(buf, "%1c", &i);
		if( isalpha(i) ) {

			i = tolower(i);
			j = i - 'a';

			if( (j < lp->d_npartitions) &&
						(lp->d_partitions[j].p_size) )
				break;
		}

		fprintf(stderr,
			"\n\n*** Please specify a valid partition. ***\n\n");
	}

	printf("%c\n", i);

	exit(0);
}


/*	Overlap()
 *		generate a list of overlapping partitions
 *
 *	given:	i - partition number
 *		lp - disk label pointer
 *	does:	checks for partition overlap
 *	return:	a string containing the partition names of the overlapping
 *		partitions
*/

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

		*bp++ = j + 'a';
	}
	*bp = '\0';
	return( outbuf );
}	



/*
 *	Prune partitions from disklabel which
 *		overlap those already used
*/
prune(argc, argv, lp)
int argc;
char *argv[];
register struct disklabel *lp;
{
	register struct partition *pp;
	int i, j, n;
	int start, end;
	register char **ap;

	n = strlen(argv[1]);

	for(argc -= 2, ap = argv + 2; argc > 0; --argc, ++ap) {

		if( strncmp(argv[1], *ap, n) )		/* different disk */
			continue;

		i = (*ap)[n];
		if( !isalpha(i) )
			continue;

		i = tolower(i) - 'a';
		if(i >= lp->d_npartitions)
			continue;

		/*
		 * Check for overlap with used partitions.
		 *	Zero partition size to ignore.
		 *
		 * Note:  if list of used partitions has overlaps,
		 *		this scheme isn't correct.
		 */
		start = lp->d_partitions[i].p_offset;
		end = start + lp->d_partitions[i].p_size;
		lp->d_partitions[i].p_size = 0;
		for(j = 0; j < lp->d_npartitions; ++j) {

			if(lp->d_partitions[j].p_offset >= end)
				continue;

			if(lp->d_partitions[j].p_offset +
					lp->d_partitions[j].p_size <= start)
				continue;

			lp->d_partitions[j].p_size = 0;
		}
	}
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



void display(f, lp)
FILE *f;
register struct disklabel *lp;
{
	register int i, j;
	register struct partition *pp;
	static char	*dl = "------------------";	/* 18 */
	double		x;		/* sector blocking factor */

	x = lp->d_secsize / 512.0;

	fprintf( f, "partition\t%9s%9s%9s\t%s\n",
		"start", "size", "end", "overlap" );

	fprintf( f, "%s%s%s%s\n", dl, dl, dl, dl );

	pp = lp->d_partitions;

	for (i = 0; i < lp->d_npartitions; i++, pp++)
	{
		if (pp->p_size)
		{
			double		offset = pp->p_offset * x;
			double		size = pp->p_size * x;

			fprintf(f, "    %c\t  ----\t%9.0f %9.0f %9.0f", 'a' + i,
				offset, size, offset+size );

			fprintf(f, "\t  %s\n", Overlap( i, lp ) );

		}
	}
	fprintf(f, "%s%s%s%s\n", dl, dl, dl, dl);
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
#ifdef DEBUG
	if(debug) {
		printf("label being checked:\n");
		dumplabel(lp);
	}
#endif

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
		part = 'a' + i;
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
		part = 'a' + i;
		pp = &lp->d_partitions[i];
		if (pp->p_size || pp->p_offset)
			Warning("unused partition %c: size %d offset %d\n",
			    'a' + i, pp->p_size, pp->p_offset);
	}
#ifdef DEBUG
	if(debug) {
		printf("label after checking:\n");
		dumplabel(lp);
	}
#endif
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
	fprintf(stderr, "usage: pickapart disk [partitions in use]\n");
	exit(1);
}

#ifdef DEBUG
dumplabel(lp)
register struct disklabel *lp;
{
register struct partition *pptr;
register int i;

	printf("d_magic: %x\n", lp->d_magic);
	printf("d_type: %d\n", lp->d_type);
	printf("d_subtype: %d\n", lp->d_subtype);
	printf("d_typename: %16s\n", lp->d_typename);
	printf("d_secsize: %d\n", lp->d_secsize);
	printf("d_nsectors: %d\n", lp->d_nsectors);
	printf("d_ntracks: %d\n", lp->d_ntracks);
	printf("d_ncylinders: %d\n", lp->d_ncylinders);
	printf("d_secpercyl: %d\n", lp->d_secpercyl);
	printf("d_secperunit: %d\n", lp->d_secperunit);
	printf("d_rpm: %d\n", lp->d_rpm);
	printf("d_npartitions: %d\n", lp->d_npartitions);
	printf("d_npartitions: %d\n", lp->d_npartitions);
	for (i=0; i< lp->d_npartitions; i++) {
		pptr = &(lp->d_partitions[i]);
		printf("partition %d\n", i);
		printf("        p_size %d\n", pptr->p_size);
		printf("        p_offset %d\n", pptr->p_offset);
		printf("        p_fsize %d\n", pptr->p_fsize);
		printf("        p_fstype %d\n", pptr->p_fstype);
		printf("        p_frag %d\n", pptr->p_frag);
		printf("        p_cpg %d\n", pptr->p_cpg);
		printf("\n");
	}
}
#endif

