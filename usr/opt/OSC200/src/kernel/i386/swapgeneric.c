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
static char	*sccsid = "@(#)$RCSfile: swapgeneric.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:20:04 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
 
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <hd.h>
#include <fd.h>
#include <wt.h>

#include <cputypes.h>

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/systm.h>
#include <sys/reboot.h>
#include <i386/AT386/disk.h>

int boottype = 0;
int boothowto = 0;

/*
 * Generic configuration;  all in one
 */
#if	AT386
/* dev_t	rootdev = makedev(0, 1);		/* disk */
/* dev_t	rootdev = makedev(1, 19);		/* floppy */
#endif	AT386
#if	EXL
/*
 *	Make the compile-time rootdev (0, 0) for EXL; rootdev is replaced
 *	at boot time in i386_init() for EXL.
 */
dev_t	rootdev = makedev(0, 0);

#endif	EXL
/*
 * Generic configuration;  all in one
 */
dev_t	rootdev = NODEV;
dev_t	dumpdev = NODEV;
long	dumplo;

#ifdef	someday
#if	NHD > 0
extern struct at_driver hddriver;
#endif	NHD
#if	NFD > 0
extern struct at_driver fddriver;
#endif	NFD
#if	NWT > 0
extern struct at_driver qtdriver;
#endif	NWT
#else	someday
#define driver dumplo		/* non zero */
#define hddriver driver
#define fddriver driver
#define qtdriver driver
#endif	someday

hd_makedev(maj, unit, slice)
{
	int ret;

#if	PART_DISK == 2 || PS2
	ret = makedev(maj, unit*16+slice);		/* a */
#else	PART_DISK == 2
	ret = makedev(maj, unit*16+slice+1);		/* b --- stupid */
#endif	PART_DISK == 2

	return ret;
}

fd_makedev(maj, unit, slice)
{
	int ret;
	char space[128];

	/*
	 *  the boot program initializes the slice to be the proper
	 *  minor number for the given floppy type
	 */

	ret = makedev(maj, unit*64+slice);

	gets(space);
	return ret;
}


qt_makedev(maj, unit, slice)
{
	int ret;
printf("qt_makedev(major %x, unit %x, slice %x)", maj, unit, slice);
	ret = makedev(maj, unit*8+slice);
printf(" == %x\n", ret);
	return ret;
}

struct	genericconf {			/* block or char */
	caddr_t	gc_driver;
	char	*gc_name;
	dev_t	gc_root;
	int	(*gc_makedev)();
} genericconf[] = {
#if	NHD > 0
	{ (caddr_t)&hddriver,	"hd",	0, hd_makedev},
#endif	NHP
#if	NFD > 0
	{ (caddr_t)&fddriver,	"fd",	1, fd_makedev},
#endif	NFD
#if	NWT > 0
	{ (caddr_t)&qtdriver,	"qt",	2, qt_makedev},
#endif	NWT
	{ 0 },
};

setconf()
{
	register struct mba_device *mi;
	register struct uba_device *ui;
	register struct genericconf *gc;
	int unit, swaponroot = 0;
	int slice;
	char name[128];
	int bootdev;

	slice = (boottype>>B_PARTITIONSHIFT)&B_PARTITIONMASK;
	bootdev = (boottype>>B_TYPESHIFT)&B_TYPEMASK;

	printf("setconf: bootdev=%x slice=%x\n",
		bootdev, slice);

	if (rootdev != NODEV)
		goto doswap;
	if (boothowto & RB_ASKNAME) {
retry:
		printf("root device? ");
		gets(name);
		for (gc = genericconf; gc->gc_driver; gc++)
			if (gc->gc_name[0] == name[0] &&
			    gc->gc_name[1] == name[1])
				goto gotit;
		goto bad;
gotit:
		if (name[3] == '*') {
			name[3] = name[4];
			swaponroot++;
		}
		if (name[2] >= '0' && name[2] <= '7') {
			if (name[3] >= 'a' && name[3] <= 'h') {
				slice = name[3] - 'a';
			} else if (name[3]) {
				printf("bad partition number\n");
				goto bad;
			}
			unit = name[2] - '0';
			goto found;
		}
		printf("bad/missing unit number\n");
bad:
		for (gc = genericconf; gc->gc_driver; gc++)
			printf("%s%s%%d",
			       (gc == genericconf)?"use ":
				    (((gc+1)->gc_driver)?", ":" or "),
			       gc->gc_name);
		printf("\n");
		goto retry;
	}

	unit = 0;
	for (gc = genericconf; gc->gc_driver; gc++) {
		if (gc->gc_root == bootdev) {
			printf("root on %s0\n",
				    gc->gc_name);
			goto found;
		}
	}
	printf("no suitable root\n");
#if PS2
	inline_hlt();
#else
	asm("hlt");
#endif
found:
	gc->gc_root = (gc->gc_makedev)(gc->gc_root, unit, slice);
	rootdev = gc->gc_root;
doswap:
	if (dumpdev == NODEV)	/* default to 'b' partition */
		dumpdev = (gc->gc_makedev)(rootdev, unit, slice+1);
	/* swap size and dumplo set during autoconfigure */
	if (swaponroot)
		rootdev = dumpdev;
}

gets(cp)
	char *cp;
{
	register char *lp;
	register c;

strcpy(cp,"hd0");
return;

	lp = cp;
	for (;;) {
		c = cngetc() & 0177;
		switch (c) {
		case '\n':
		case '\r':
			*lp++ = '\0';
			return;
		case '\b':
		case '#':
		case '\177':
			lp--;
			if (lp < cp)
				lp = cp;
			continue;
		case '@':
		case 'u'&037:
			lp = cp;
			cnputc('\n');
			continue;
		default:
			*lp++ = c;
		}
	}
}
