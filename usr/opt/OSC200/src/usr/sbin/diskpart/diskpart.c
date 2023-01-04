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
static char	*sccsid = "@(#)$RCSfile: diskpart.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:03:30 $";
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
/*
 * Copyright (c) 1983, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983, 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */


/*
 * Program to calculate standard disk partition sizes.
 */
#include <sys/param.h>
#define DKTYPENAMES
#include <sys/disklabel.h>

#include <sys/secdefines.h>
#include <stdio.h>
#include <ctype.h>

#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif

#define	for_now			/* show all of `c' partition for disklabel */
#define	NPARTITIONS	8
#define	PART(x)		(x - 'a')

/*
 * Default partition sizes, where they exist.
 */
#if	CMU_DISKPART
#define	NDEFAULTS	5
int	defpart[NDEFAULTS][NPARTITIONS] = { 
/*
 *  All disks use a common A (root), B (paging), and D (system) partitioning of
 *  the initial cylinders of the disk along with the standard C (full disk)
 *  partition.  Beyond this there are three basic layouts:
 *
 *  Small disks:   E maps space remaining after D
 *		   F is unused
 *		   G maps all usable space from the full disk
 *		   H maps space remaining after B
 *
 *  Medium disks:  E maps space in F remaining after D
 *		   F maps first half space remaining after B (floor)
 *		   G maps last half space remaining after B (ceiling)
 *		   H maps space remaining after B
 *    
 *  Large disks:   E maps space in F remaining after D
 *		   F maps first third space remaining after B (floor/ceiling)
 *		   G maps second third space remaining after B (floor)
 *		   H maps last third space remaining after B (ceiling)
 *
 *  The template sizes listed below guarantee minimum sizes for the partitions
 *  in each layout.  Small disks have no minimum E partition size.  Medium
 *  disks have a minimum E partition size but no minimum H partition size.
 *  Large disks have both minimum E and H partition sizes.
 */
#ifdef	vax
/* large: ~ 466+ Mbytes */
   { 15884, 66880, 0, 193920, 193920/2, 0, 3*193920/2, 3*193920/2 },
/* medium: ~ 308-466 Mbytes */
   { 15884, 33440, 0, 193920, 193920/2, 0, 3*193920/2,        0   },
/* medium: ~ 166-308 Mbytes */
   { 15884, 33440, 0,  96960,  96960/2, 0, 3* 96960/2,        0   },
/* small: ~ 40-166 Mbytes */
   { 15884, 33440, 0,  34398,        0, 0,        0  ,        0   },
/* small: ~ 26-40 Mbytes */
   { 15884, 10032, 0,  27852,        0, 0,        0  ,        0   },
/*
 *  The template sizes listed above have the following historical significant:
 *
 *   15884 is the size of partition A on the RP06
 *   10032 is the size of partition B on the RK07
 *   33440 is the size of partition B on the RP06
 *   27852 is the space remaining for partition H on the RK07
 *   34398 is the space remaining for partition H on the RD52
 *   96960 is the size of partition D on older Eagles
 *  193920 is the size of partition D on newer Eagles (older doubled)
 */
#endif	/* vax */
#ifdef	sun
/* large: ~ 472+ Mbytes */
   { 27797, 66880, 0, 193920, 193920/2, 0, 3*193920/2, 3*193920/2 },
/* medium: ~ 313-472 Mbytes */
   { 27797, 33440, 0, 193920, 193920/2, 0, 3*193920/2,        0   },
/* medium: ~ 171-313 Mbytes */
   { 27797, 33440, 0,  96960,  96960/2, 0, 3* 96960/2,        0   },
/* small: ~ 55-171 Mbytes */
   { 27797, 33440, 0,  51597,        0, 0,        0  ,        0   },
/* small: ~ 42-55 Mbytes */
   { 27797, 17556, 0,  41778,        0, 0,        0  ,        0   },
/*
 *  The template sizes listed above which differ from those on VAX are adjusted
 *  to account for the relative binary size difference by an expansion factor
 *  of 1.75 (determined experimentally).  All root (A) partitions and the
 *  smaller paging (B) and system (D) partitions are adjusted by this factor.
 *  The other areas are left untouched since they are probably already oversize
 *  for their current use on the VAX.
 */
#endif	/* sun */
#ifdef	ibmrt
/* large: ~ 472+ Mbytes */
   { 27797, 66880, 0, 193920, 193920/2, 0, 3*193920/2, 3*193920/2 },
/* medium: ~ 313-472 Mbytes */
   { 27797, 33440, 0, 193920, 193920/2, 0, 3*193920/2,        0   },
/* medium: ~ 171-313 Mbytes */
   { 27797, 33440, 0,  96960,  96960/2, 0, 3* 96960/2,        0   },
/* small: ~ 55-171 Mbytes */
   { 27797, 33440, 0,  51597,        0, 0,        0  ,        0   },
/* small: ~ 42-55 Mbytes */
   { 27797, 17556, 0,  41480,        0, 0,        0  ,        0   },
/*
 *  The template sizes listed above which differ from those on VAX are adjusted
 *  to account for the relative binary size difference by an expansion factor
 *  of 1.75 (determined experimentally).  All root (A) partitions and the
 *  smaller paging (B) and system (D) partitions are adjusted by this factor.
 *  The other areas are left untouched since they are probably already oversize
 *  for their current use on the VAX.  The size of the smallest system (D)
 *  partitions is slightly less to accomodate HD40M disks.
 */
#endif	/* ibmrt */
};
#else	/* CMU_DISKPART */
#define	NDEFAULTS	4
int	defpart[NDEFAULTS][NPARTITIONS] = {
   { 15884, 66880, 0, 15884, 307200, 0, 0, 291346 },	/* ~ 356+ Mbytes */
   { 15884, 33440, 0, 15884, 55936, 0, 0, 291346 },	/* ~ 206-355 Mbytes */
   { 15884, 33440, 0, 15884, 55936, 0, 0, 0 },		/* ~ 61-205 Mbytes */
   { 15884, 10032, 0, 15884, 0, 0, 0, 0 },		/* ~ 20-60 Mbytes */
};
#endif	/* CMU_DISKPART */

/*
 * Each array defines a layout for a disk;
 * that is, the collection of partitions totally
 * covers the physical space on a disk.
 */
#if	CMU_DISKPART
/*
 *  Layouts not used in a given configuration size are flagged here so that
 *  they can be dynamically deleted from the layout table once the
 *  configuration and cylinder sizes have been determined.
 */
#define	NLAYOUTS	8
char	layouts[NLAYOUTS][NPARTITIONS] = { 
   { 'a', 'b', 'h' },			/* small, medium */
#define	LAYOUT1_NLARGE	0
   { 'a', 'b', 'f', 'g' },		/* medium */
#define	LAYOUT1_NSMALL	1
#define	LAYOUT2_NLARGE	1
   { 'a', 'b', 'f', 'g', 'h' },		/* large */
#define	LAYOUT2_NSMALL	2
#define	LAYOUT1_NMEDIUM	2
   { 'a', 'b', 'd', 'e' },		/* small */
#define	LAYOUT2_NMEDIUM	3
#define	LAYOUT3_NLARGE	3
   { 'a', 'b', 'd', 'e', 'g' },		/* medium */
#define	LAYOUT3_NSMALL	4
#define	LAYOUT4_NLARGE	4
   { 'a', 'b', 'd', 'e', 'g', 'h' },	/* large */
#define	LAYOUT4_NSMALL	5
#define	LAYOUT3_NMEDIUM	5
   { 'c' },				/* small, medium, large */
   { 'g' },				/* small */
#define	LAYOUT4_NMEDIUM	7
#define	LAYOUT5_NLARGE	7
};
#else	/* CMU_DISKPART */
#define	NLAYOUTS	3
char	layouts[NLAYOUTS][NPARTITIONS] = {
   { 'a', 'b', 'h', 'g' },
   { 'a', 'b', 'h', 'd', 'e', 'f' },
   { 'c' },
};
#endif	/* CMU_DISKPART */

/*
 * Default disk block and disk block fragment
 * sizes for each file system.  Those file systems
 * with zero block and frag sizes are special cases
 * (e.g. swap areas or for access to the entire device).
 */
struct	partition defparam[NPARTITIONS] = {
	{ 0, 0, 1024, FS_UNUSED, 8, 0 },		/* a */
	{ 0, 0, 1024, FS_SWAP,   8, 0 },		/* b */
	{ 0, 0, 1024, FS_UNUSED, 8, 0 },		/* c */
#if	CMU
	{ 0, 0, 1024, FS_UNUSED, 8, 0 },		/* d */
#else
	{ 0, 0,  512, FS_UNUSED, 8, 0 },		/* d */
#endif
	{ 0, 0, 1024, FS_UNUSED, 8, 0 },		/* e */
	{ 0, 0, 1024, FS_UNUSED, 8, 0 },		/* f */
	{ 0, 0, 1024, FS_UNUSED, 8, 0 },		/* g */
	{ 0, 0, 1024, FS_UNUSED, 8, 0 }			/* h */
};

#if	CMU && defined(ibmrt)
/* 
 *  IBM appears to reserve 1000 sectors for bad block forwarding
 */
int	badsecttable = 1000;	/* # sectors */
#else
/*
 * Each disk has some space reserved for a bad sector
 * forwarding table.  DEC standard 144 uses the first
 * 5 even numbered sectors in the last track of the
 * last cylinder for replicated storage of the bad sector
 * table; another 126 sectors past this is needed as a
 * pool of replacement sectors.
 */
int	badsecttable = 126;	/* # sectors */
#endif

int	pflag;			/* print device driver partition tables */
int	dflag;			/* print disktab entry */

struct	disklabel *promptfordisk();

#if SEC_BASE
static privvec_t additional_privs;
#endif

main(argc, argv)
	int argc;
	char *argv[];
{
	struct disklabel *dp;
	register int curcyl, spc, def, part, layout, j;
	int threshhold, numcyls[NPARTITIONS], startcyl[NPARTITIONS];
	int totsize = 0;
	char *lp, *tyname;
#if	CMU
	int reserved;	/* initial reserved cylinders (RT) */
#endif

#if SEC_BASE && !defined(SEC_STANDALONE)
	privvec_t saveprivs;

	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, "%s: need sysadmin authorization\n",
			command_name);
		exit(1);
	}
	setprivvec(additional_privs, SEC_ALLOWDACACCESS, SEC_FILESYS,
#if SEC_MAC
			SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
			SEC_ALLOWNCAVACCESS,
#endif
			-1);
#endif /* SEC_BASE && !defined(SEC_STANDALONE) */

	argc--, argv++;
	if (argc < 1) {
		fprintf(stderr,
		    "usage: disktab [ -p ] [ -d ] [ -s size ] disk-type\n");
		exit(1);
	}
	if (argc > 0 && strcmp(*argv, "-p") == 0) {
		pflag++;
		argc--, argv++;
	}
	if (argc > 0 && strcmp(*argv, "-d") == 0) {
		dflag++;
		argc--, argv++;
	}
	if (argc > 1 && strcmp(*argv, "-s") == 0) {
		totsize = atoi(argv[1]);
		argc += 2, argv += 2;
	}
	dp = getdiskbyname(*argv);
	if (dp == NULL) {
		if (isatty(0))
			dp = promptfordisk(*argv);
		if (dp == NULL) {
			fprintf(stderr, "%s: unknown disk type\n", *argv);
			exit(2);
		}
	} else {
		if (dp->d_flags & D_REMOVABLE)
			tyname = "removable";
		else if (dp->d_flags & D_RAMDISK)
			tyname = "simulated";
		else
			tyname = "winchester";
	}
	spc = dp->d_secpercyl;
	/*
	 * Bad sector table contains one track for the replicated
	 * copies of the table and enough full tracks preceding
	 * the last track to hold the pool of free blocks to which
	 * bad sectors are mapped.
	 * If disk size was specified explicitly, use specified size.
	 */
	if (dp->d_type == DTYPE_SMD && dp->d_flags & D_BADSECT &&
	    totsize == 0) {
#if	CMU && defined(ibmrt)
		/* 
		 * Kernel has bug that causes badsecttable to start one
		 * cylinder too early.  Therefore add the number of blocks
		 * in one cylinder to the bad sector table size.
		 */
		badsecttable += spc;
#else
		badsecttable = dp->d_nsectors +
		    roundup(badsecttable, dp->d_nsectors);
#endif
		threshhold = howmany(spc, badsecttable);
	} else {
		badsecttable = 0;
		threshhold = 0;
	}
	/*
	 * If disk size was specified, recompute number of cylinders
	 * that may be used, and set badsecttable to any remaining
	 * fraction of the last cylinder.
	 */
	if (totsize != 0) {
		dp->d_ncylinders = howmany(totsize, spc);
		badsecttable = spc * dp->d_ncylinders - totsize;
	}

	/* 
	 * Figure out if disk is large enough for
	 * expanded swap area and 'd', 'e', and 'f'
	 * partitions.  Otherwise, use smaller defaults
	 * based on RK07.
	 */
	for (def = 0; def < NDEFAULTS; def++) {
		curcyl = 0;
		for (part = PART('a'); part < NPARTITIONS; part++)
			curcyl += howmany(defpart[def][part], spc);
#if	CMU
		/* bug in orginal? */
		if (curcyl <= dp->d_ncylinders - threshhold)
#else
		if (curcyl < dp->d_ncylinders - threshhold)
#endif
			break;
	}
	if (def >= NDEFAULTS) {
		fprintf(stderr, "%s: disk too small, calculate by hand\n",
			*argv);
		exit(3);
	}

	/*
	 * Calculate number of cylinders allocated to each disk
	 * partition.  We may waste a bit of space here, but it's
	 * in the interest of (very backward) compatibility
	 * (for mixed disk systems).
	 */
	for (curcyl = 0, part = PART('a'); part < NPARTITIONS; part++) {
		numcyls[part] = 0;
		if (defpart[def][part] != 0) {
			numcyls[part] = howmany(defpart[def][part], spc);
			curcyl += numcyls[part];
		}
	}
#if	CMU && defined(ibmrt)
	reserved = 1;	/*  all known RT disks reserve cylinder 0 */
#else
	reserved = 0;
#endif
#if	CMU_DISKPART
	/*
	 *  Calculate full disk size as partition C and space remaining after A
	 *  and B partitions temporarily as partition F.
	 */
	numcyls[PART('c')] = dp->d_ncylinders;
	numcyls[PART('f')] = (numcyls[PART('c')] - numcyls[PART('a')] -
			      numcyls[PART('b')] - reserved);
	if (defpart[def][PART('e')]) {
		if (defpart[def][PART('h')]) {
			/*
			 *  Calculate large disk partition sizes.
			 */
			numcyls[PART('h')] = howmany(numcyls[PART('f')],3);
			numcyls[PART('g')] = numcyls[PART('f')]/3;
			numcyls[PART('f')] -= (numcyls[PART('g')] +
					       numcyls[PART('h')]);
			/*
			 *  Remove non-large layouts from table.
			 */
			layouts[LAYOUT1_NLARGE][0] = 0;
			layouts[LAYOUT2_NLARGE][0] = 0;
			layouts[LAYOUT3_NLARGE][0] = 0;
			layouts[LAYOUT4_NLARGE][0] = 0;
			layouts[LAYOUT5_NLARGE][0] = 0;
			/*
			 *  Remember partitions in large layouts which extend
			 *  to the end of the disk for bad sector forwarding
			 *  adjustment.
			 */
			layout = (1<<PART('h'));
		} else {
			/*
			 *  Calculate medium disk partition sizes.
			 */
			numcyls[PART('h')] = numcyls[PART('f')];
			numcyls[PART('g')] = howmany(numcyls[PART('f')],2);
			numcyls[PART('f')] -= numcyls[PART('g')];
			/*
			 *  Remove non-medium layouts from table.
			 */
			layouts[LAYOUT1_NMEDIUM][0] = 0;
			layouts[LAYOUT2_NMEDIUM][0] = 0;
			layouts[LAYOUT3_NMEDIUM][0] = 0;
			layouts[LAYOUT4_NMEDIUM][0] = 0;
			/*
			 *  Remember partitions in medium layouts which extend
			 *  to the end of the disk for bad sector forwarding
			 *  adjustment.
			 */
			layout = (1<<PART('g'))|(1<<PART('h'));
		}
		numcyls[PART('e')] = numcyls[PART('f')] - numcyls[PART('d')];
	} else {
		/*
		 *  Calculate small disk partition sizes.
		 */
		numcyls[PART('h')] = numcyls[PART('f')];
		numcyls[PART('g')] = dp->d_ncylinders - reserved;
		numcyls[PART('f')] = 0;
		numcyls[PART('e')] = numcyls[PART('h')] - numcyls[PART('d')];
		/*
		 *  Remove non-small layouts from table.
		 */
		layouts[LAYOUT1_NSMALL][0] = 0;
		layouts[LAYOUT2_NSMALL][0] = 0;
		layouts[LAYOUT3_NSMALL][0] = 0;
		layouts[LAYOUT4_NSMALL][0] = 0;
		/*
		 *  Remember partitions in small layouts which extend to the
		 *  end of the disk for bad sector forwarding adjustment.
		 */
		layout = (1<<PART('e'))|(1<<PART('g'))|(1<<PART('h'));
	}
	/*
	 *  Update all partition sizes to cylinder boundaries.
 	 */
	defpart[def][PART('h')] = numcyls[PART('h')] * spc;
	defpart[def][PART('g')] = numcyls[PART('g')] * spc;
	defpart[def][PART('f')] = numcyls[PART('f')] * spc;
	defpart[def][PART('e')] = numcyls[PART('e')] * spc;
	defpart[def][PART('d')] = numcyls[PART('d')] * spc;
	defpart[def][PART('c')] = numcyls[PART('c')] * spc;
	defpart[def][PART('b')] = numcyls[PART('b')] * spc;
	defpart[def][PART('a')] = numcyls[PART('a')] * spc;
	/*
	 *  Adjust sizes of all partitions which extend to the end of the disk
	 *  to account for the bad sector forwarding area.
 	 */
	part = 0;
	for (; layout; layout>>=1, part++)
		if ((layout&1) && defpart[def][part])
			defpart[def][part] -= badsecttable;
#else	/* CMU_DISKPART */
	numcyls[PART('f')] = dp->d_ncylinders - curcyl;
	numcyls[PART('g')] =
		numcyls[PART('d')] + numcyls[PART('e')] + numcyls[PART('f')];
	numcyls[PART('c')] = dp->d_ncylinders;
	defpart[def][PART('f')] = numcyls[PART('f')] * spc - badsecttable;
	defpart[def][PART('g')] = numcyls[PART('g')] * spc - badsecttable;
	defpart[def][PART('c')] = numcyls[PART('c')] * spc;
#endif	/* CMU_DISKPART */
#if	CMU && defined(ibmrt)
	/*
	 *  When the 'c' partition is used on the IBM-RT, this already destroys
	 *  the disk configuration and forwarding table in cylinder 0 so there
	 *  is no point in reserving replacement block space at the end of the
	 *  disk either.
	 */
#else
#ifndef for_now
	if (totsize || !pflag)
#else
	if (totsize)
#endif
		defpart[def][PART('c')] -= badsecttable;
#endif

	/*
	 * Calculate starting cylinder number for each partition.
	 * Note the 'h' partition is physically located before the
	 * 'g' or 'd' partition.  This is reflected in the layout
	 * arrays defined above.
	 */
	for (layout = 0; layout < NLAYOUTS; layout++) {
		curcyl = 0;
#if	CMU
		/*
		 *  Adjust starting cylinder by reserved count for all layouts
		 *  except the one which maps the entire disk.
		 */
		if (layouts[layout][0] != 'c')
			curcyl += reserved;
#endif
		for (lp = layouts[layout]; *lp != 0; lp++) {
			startcyl[PART(*lp)] = curcyl;
			curcyl += numcyls[PART(*lp)];
		}
	}

	if (pflag) {
		printf("}, %s_sizes[%d] = {\n", dp->d_typename, NPARTITIONS);
		for (part = PART('a'); part < NPARTITIONS; part++) {
			if (numcyls[part] == 0) {
				printf("\t0,\t0,\n");
				continue;
			}
			if (dp->d_type != DTYPE_MSCP) {
#if	CMU_DISKPART
				int nc = roundup(defpart[def][part],spc)/spc;
 
				printf(dp->d_ncylinders>1000?
					"\t%d,\t%d,\t\t/* %c=cyl%5d thru%5d":
					"\t%d,\t%d,\t\t/* %c=cyl%4d thru%4d",
 					defpart[def][part], startcyl[part],
 					'A' + part, startcyl[part],
					startcyl[part] + nc - 1);
				if (defpart[def][part]/spc == nc)
					printf(" */\n");
				else
					printf(" (-%d sectors) */\n",
					       nc*spc-defpart[def][part]);
#else
			       printf("\t%d,\t%d,\t\t/* %c=cyl %d thru %d */\n",
					defpart[def][part], startcyl[part],
					'A' + part, startcyl[part],
					startcyl[part] + numcyls[part] - 1);
#endif
				continue;
			}
			printf("\t%d,\t%d,\t\t/* %c=sectors %d thru %d */\n",
				defpart[def][part], spc * startcyl[part],
				'A' + part, spc * startcyl[part],
				spc * startcyl[part] + defpart[def][part] - 1);
		}
		exit(0);
	}
	if (dflag) {
		int nparts;

		/*
		 * In case the disk is in the ``in-between'' range
		 * where the 'g' partition is smaller than the 'h'
		 * partition, reverse the frag sizes so the /usr partition
		 * is always set up with a frag size larger than the
		 * user's partition.
		 */
		if (defpart[def][PART('g')] < defpart[def][PART('h')]) {
			int temp;

			temp = defparam[PART('h')].p_fsize;
			defparam[PART('h')].p_fsize =
				defparam[PART('g')].p_fsize;
			defparam[PART('g')].p_fsize = temp;
		}
		printf("%s:\\\n", dp->d_typename);
		printf("\t:ty=%s:ns#%d:nt#%d:nc#%d:", tyname,
			dp->d_nsectors, dp->d_ntracks, dp->d_ncylinders);
		if (dp->d_secpercyl != dp->d_nsectors * dp->d_ntracks)
			printf("sc#%d:", dp->d_secpercyl);
		if (dp->d_type == DTYPE_SMD && dp->d_flags & D_BADSECT)
			printf("sf:");
		printf("\\\n\t:dt=%s:", dktypenames[dp->d_type]);
		for (part = NDDATA - 1; part >= 0; part--)
			if (dp->d_drivedata[part])
				break;
		for (j = 0; j <= part; j++)
			printf("d%d#%d:", j, dp->d_drivedata[j]);
		printf("\\\n");
		for (nparts = 0, part = PART('a'); part < NPARTITIONS; part++)
			if (defpart[def][part] != 0)
				nparts++;
		for (part = PART('a'); part < NPARTITIONS; part++) {
			if (defpart[def][part] == 0)
				continue;
			printf("\t:p%c#%d:", 'a' + part, defpart[def][part]);
			printf("o%c#%d:b%c#%d:f%c#%d:",
			    'a' + part, spc * startcyl[part],
			    'a' + part,
			    defparam[part].p_frag * defparam[part].p_fsize,
			    'a' + part, defparam[part].p_fsize);
			if (defparam[part].p_fstype == FS_SWAP)
				printf("t%c=swap:", 'a' + part);
			nparts--;
			printf("%s\n", nparts > 0 ? "\\" : "");
		}
#ifdef for_now
		defpart[def][PART('c')] -= badsecttable;
		part = PART('c');
		printf("#\t:p%c#%d:", 'a' + part, defpart[def][part]);
		printf("o%c#%d:b%c#%d:f%c#%d:\n",
		    'a' + part, spc * startcyl[part],
		    'a' + part,
		    defparam[part].p_frag * defparam[part].p_fsize,
		    'a' + part, defparam[part].p_fsize);
#endif
		exit(0);
	}
	printf("%s: #sectors/track=%d, #tracks/cylinder=%d #cylinders=%d\n",
		dp->d_typename, dp->d_nsectors, dp->d_ntracks,
		dp->d_ncylinders);
	printf("\n    Partition\t   Size\t Offset\t   Range\n");
	for (part = PART('a'); part < NPARTITIONS; part++) {
		printf("\t%c\t", 'a' + part);
		if (numcyls[part] == 0) {
			printf(" unused\n");
			continue;
		}
#if	CMU_DISKPART
		{
		int nc = roundup(defpart[def][part],spc)/spc;

		printf(dp->d_ncylinders>1000?
			"%7d\t%7d\t%5d -%5d":
			"%7d\t%7d\t%4d -%4d",
			defpart[def][part], startcyl[part] * spc,
			startcyl[part], startcyl[part] + nc - 1);
		if (defpart[def][part]/spc == nc)
			printf("\n");
		else
			printf(" (-%d sectors)\n",
			       nc*spc-defpart[def][part]);
		}
#else
		printf("%7d\t%7d\t%4d - %d%s\n",
			defpart[def][part], startcyl[part] * spc,
			startcyl[part], startcyl[part] + numcyls[part] - 1,
			defpart[def][part] % spc ? "*" : "");
#endif
	}
}

struct disklabel disk;

struct	field {
	char	*f_name;
	char	*f_defaults;
	u_long	*f_location;
} fields[] = {
	{ "sector size",		"512",	&disk.d_secsize },
	{ "#sectors/track",		0,	&disk.d_nsectors },
	{ "#tracks/cylinder",		0,	&disk.d_ntracks },
	{ "#cylinders",			0,	&disk.d_ncylinders },
	{ 0, 0, 0 },
};

struct disklabel *
promptfordisk(name)
	char *name;
{
	register struct disklabel *dp = &disk;
	register struct field *fp;
	register i;
	char buf[BUFSIZ], **tp, *cp, *gets();

	strncpy(dp->d_typename, name, sizeof(dp->d_typename));
	fprintf(stderr,
		"%s: unknown disk type, want to supply parameters (y/n)? ",
		name);
	(void) gets(buf);
	if (*buf != 'y')
		return ((struct disklabel *)0);
	for (;;) {
		fprintf(stderr, "Disk/controller type (%s)? ", dktypenames[1]);
		(void) gets(buf);
		if (buf[0] == 0)
			dp->d_type = 1;
		else
			dp->d_type = gettype(buf, dktypenames);
		if (dp->d_type >= 0)
			break;
		fprintf(stderr, "%s: unrecognized controller type\n", buf);
		fprintf(stderr, "use one of:\n", buf);
		for (tp = dktypenames; *tp; tp++)
			if (index(*tp, ' ') == 0)
				fprintf(stderr, "\t%s\n", *tp);
	}
gettype:
	dp->d_flags = 0;
	fprintf(stderr, "type (winchester|removable|simulated)? ");
	(void) gets(buf);
	if (strcmp(buf, "removable") == 0)
		dp->d_flags = D_REMOVABLE;
	else if (strcmp(buf, "simulated") == 0)
		dp->d_flags = D_RAMDISK;
	else if (strcmp(buf, "winchester")) {
		fprintf(stderr, "%s: bad disk type\n", buf);
		goto gettype;
	}
	strncpy(dp->d_typename, buf, sizeof(dp->d_typename));
	fprintf(stderr, "(type <cr> to get default value, if only one)\n");
	if (dp->d_type == DTYPE_SMD)
#if	CMU && defined(ibmrt)
	   fprintf(stderr, "Do %ss support IBM bad block forwarding (yes)? ",
#else
	   fprintf(stderr, "Do %ss support bad144 bad block forwarding (yes)? ",
#endif
		dp->d_typename);
	(void) gets(buf);
	if (*buf != 'n')
		dp->d_flags |= D_BADSECT;
	for (fp = fields; fp->f_name != NULL; fp++) {
again:
		fprintf(stderr, "%s ", fp->f_name);
		if (fp->f_defaults != NULL)
			fprintf(stderr, "(%s)", fp->f_defaults);
		fprintf(stderr, "? ");
		cp = gets(buf);
		if (*cp == '\0') {
			if (fp->f_defaults == NULL) {
				fprintf(stderr, "no default value\n");
				goto again;
			}
			cp = fp->f_defaults;
		}
		*fp->f_location = atol(cp);
		if (*fp->f_location == 0) {
			fprintf(stderr, "%s: bad value\n", cp);
			goto again;
		}
	}
	fprintf(stderr, "sectors/cylinder (%d)? ",
	    dp->d_nsectors * dp->d_ntracks);
	(void) gets(buf);
	if (buf[0] == 0)
		dp->d_secpercyl = dp->d_nsectors * dp->d_ntracks;
	else
		dp->d_secpercyl = atol(buf);
	fprintf(stderr, "Drive-type-specific parameters, <cr> to terminate:\n");
	for (i = 0; i < NDDATA; i++) {
		fprintf(stderr, "d%d? ", i);
		(void) gets(buf);
		if (buf[0] == 0)
			break;
		dp->d_drivedata[i] = atol(buf);
	}
	return (dp);
}

gettype(t, names)
	char *t;
	char **names;
{
	register char **nm;

	for (nm = names; *nm; nm++)
		if (ustrcmp(t, *nm) == 0)
			return (nm - names);
	if (isdigit(*t))
		return (atoi(t));
	return (-1);
}

ustrcmp(s1, s2)
	register char *s1, *s2;
{
#define	lower(c)	(islower(c) ? (c) : tolower(c))

	for (; *s1; s1++, s2++) {
		if (*s1 == *s2)
			continue;
		if (isalpha(*s1) && isalpha(*s2) &&
		    lower(*s1) == lower(*s2))
			continue;
		return (*s2 - *s1);
	}
	return (0);
}
