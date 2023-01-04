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
static char	*sccsid = "@(#)$RCSfile: disklabel.c,v $ $Revision: 4.3.7.3 $ (DEC) $Date: 1993/10/05 21:00:47 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1983, 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 * disklabel.c	5.13 (Berkeley) 6/1/90
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak dkcksum = __dkcksum
#if defined(_THREAD_SAFE)
#pragma weak creatediskbyname_r = __creatediskbyname_r
#pragma weak getdiskbyname_r = __getdiskbyname_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak creatediskbyname = __creatediskbyname
#pragma weak getdiskbyname = __getdiskbyname
#endif
#endif
#include <sys/param.h>
#include <ufs/fs.h>
#include <sys/file.h>
#define DKTYPENAMES
#include <sys/disklabel.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include <errno.h>
/*
 * The main problem with making this code thread safe was that nearly
 * every function used a global pointer to refer to a buffer.
 * Instead of putting all the local function calles into #ifdefs,
 * I defined these macros.
 */
#define DNAMATCH(name, buf)	dnamatch(name, buf)
#define DGETNUM(id, buf)	dgetnum(id, buf)
#define DGETFLAG(id, buf)	dgetflag(id, buf)
#define DGETSTR(id, area, buf)	dgetstr(id, area, buf)
#else
#define DNAMATCH(name, buf)	dnamatch(name)
#define DGETNUM(id, buf)	dgetnum(id)
#define DGETFLAG(id, buf)	dgetflag(id)
#define DGETSTR(id, area, buf)	dgetstr(id, area)
#endif /* _THREAD_SAFE */

static	dgetent();
static	dnamatch();
static	dgetnum();
static	dgetflag();
static	gettype();
static	char *dgetstr();

#ifdef _THREAD_SAFE
int
getdiskbyname_r(name, disk, boot, boot_len)
	char *name;
	struct disklabel *disk;
	char *boot;
	int boot_len;
#else
struct disklabel *
getdiskbyname(name)
	char *name;
#endif /* _THREAD_SAFE */
{

#ifndef _THREAD_SAFE
	static struct	disklabel disk;
	static char 	*boot = NULL;
#endif /* _THREAD_SAFE */

	char	localbuf[BUFSIZ];
	char	buf[BUFSIZ];
	char	*cp, *cq;	/* can't be register */

#ifdef _THREAD_SAFE
	register struct	disklabel *dp = disk;
#else
	register struct	disklabel *dp = &disk;
#endif /* _THREAD_SAFE */

	register struct partition *pp;
	char	p, max, psize[3], pbsize[3],
		pfsize[3], poffset[3], ptype[3];
	u_int	*dx;

#ifdef _THREAD_SAFE
	if (disk == NULL || boot == NULL || boot_len != BUFSIZ) {
		_Seterrno(EINVAL);
		return(-1);
	}
#endif /* _THREAD_SAFE */

	if (dgetent(buf, name) <= 0)
#ifdef _THREAD_SAFE
		return(-1);
#else
		return ((struct disklabel *)0);
#endif /* _THREAD_SAFE */

#ifdef _THREAD_SAFE
	bzero((char *)disk, sizeof(disk));
#else
	bzero((char *)&disk, sizeof(disk));
#endif /* _THREAD_SAFE */
	/*
	 * typename
	 */
	cq = dp->d_typename;
	cp = buf;
	while (cq < dp->d_typename + sizeof(dp->d_typename) - 1 &&
	    (*cq = *cp) && *cq != '|' && *cq != ':')
		cq++, cp++;
	*cq = '\0';
	/*
	 * boot name (optional)  xxboot, bootxx
	 */
	if(boot == NULL) {
	    boot = (char *)malloc(BUFSIZ);
            assert(boot);
        }
	cp = boot;

	dp->d_boot0 = DGETSTR("b0", &cp, buf);
	dp->d_boot1 = DGETSTR("b1", &cp, buf);
	cp = localbuf;
	cq = DGETSTR("ty", &cp, buf);
	if (cq && strcmp(cq, "removable") == 0)
		dp->d_flags |= D_REMOVABLE;
	else  if (cq && strcmp(cq, "simulated") == 0)
		dp->d_flags |= D_RAMDISK;
	else  if (cq && strcmp(cq, "dynamic_geometry") == 0)
		dp->d_flags |= D_DYNAM_GEOM;

	if (DGETFLAG("sf", buf))
		dp->d_flags |= D_BADSECT;

#define getnumdflt(field, dname, dflt) \
	{ int f = DGETNUM(dname, buf); \
	(field) = f == -1 ? (dflt) : f; }

	getnumdflt(dp->d_secsize, "se", DEV_BSIZE);
	dp->d_ntracks = DGETNUM("nt", buf);
	dp->d_nsectors = DGETNUM("ns", buf);
	dp->d_ncylinders = DGETNUM("nc", buf);
	cq = DGETSTR("dt", &cp, buf);

	if (cq)
		dp->d_type = gettype(cq, dktypenames);
	else
		getnumdflt(dp->d_type, "dt", 0);
	getnumdflt(dp->d_secpercyl, "sc", dp->d_nsectors * dp->d_ntracks);
	getnumdflt(dp->d_secperunit, "su", dp->d_secpercyl * dp->d_ncylinders);
	getnumdflt(dp->d_rpm, "rm", 3600);
	getnumdflt(dp->d_interleave, "il", 1);
	getnumdflt(dp->d_trackskew, "sk", 0);
	getnumdflt(dp->d_cylskew, "cs", 0);
	getnumdflt(dp->d_headswitch, "hs", 0);
	getnumdflt(dp->d_trkseek, "ts", 0);
	getnumdflt(dp->d_bbsize, "bs", BBSIZE);
	getnumdflt(dp->d_sbsize, "sb", SBSIZE);
	strcpy(psize, "px");
	strcpy(pbsize, "bx");
	strcpy(pfsize, "fx");
	strcpy(poffset, "ox");
	strcpy(ptype, "tx");
	max = 'a' - 1;
	pp = &dp->d_partitions[0];
	for (p = 'a'; p < 'a' + MAXPARTITIONS; p++, pp++) {
		psize[1] = pbsize[1] = pfsize[1] = poffset[1] = ptype[1] = p;
		pp->p_size = DGETNUM(psize, buf);
		if (pp->p_size == -1)
			pp->p_size = 0;
		else {
			pp->p_offset = DGETNUM(poffset, buf);
			getnumdflt(pp->p_fsize, pfsize, 0);
			if (pp->p_fsize)
				pp->p_frag = DGETNUM(pbsize, buf) / pp->p_fsize;
			getnumdflt(pp->p_fstype, ptype, 0);
			if (pp->p_fstype == 0 && 
				(cq = DGETSTR(ptype, &cp, buf)))
				pp->p_fstype = gettype(cq, fstypenames);
		}
		/* ULTRIX/OSF:  This is a bit strange.
		 * Originally "max" was updated inside the "else"
		 * part of the preceeding if statement.  However,
		 * because pp is updated on each turn of the "for",
		 * "max" also needs to be updated when p_size = -1.
		 * Result -- d_npartitions is the number of partition
		 * entries used in the structure and some of these
		 * entries may have a partition size of 0.
		 */
		max = p;
	}
	dp->d_npartitions = max + 1 - 'a';
	(void)strcpy(psize, "dx");
	dx = dp->d_drivedata;
	for (p = '0'; p < '0' + NDDATA; p++, dx++) {
		psize[1] = p;
		getnumdflt(*dx, psize, 0);
	}
	dp->d_magic = DISKMAGIC;
	dp->d_magic2 = DISKMAGIC;

#ifdef _THREAD_SAFE
	return (0);
#else
	return (dp);
#endif
}

#include <ctype.h>

#ifndef _THREAD_SAFE
static	char *tbuf;
#endif
static	char *dskip();
static	char *ddecode();

/*
 * Get an entry for disk name in buffer bp,
 * from the diskcap file.  Parse is very rudimentary;
 * we just notice escaped newlines.
 */
static
dgetent(bp, name)
	char *bp, *name;
{
	register char *cp;
	register int c;
	register int i = 0, cnt = 0;
	char ibuf[BUFSIZ];
	int tf;

#ifndef _THREAD_SAFE
	tbuf = bp;
#endif
	tf = open(DISKTAB, 0);
	if (tf < 0)
		return (-1);
	for (;;) {
		cp = bp;
		for (;;) {
			if (i == cnt) {
				cnt = read(tf, ibuf, BUFSIZ);
				if (cnt <= 0) {
					close(tf);
					return (0);
				}
				i = 0;
			}
			c = ibuf[i++];
			if (c == '\n') {
				if (cp > bp && cp[-1] == '\\'){
					cp--;
					continue;
				}
				break;
			}
			if (cp >= bp+BUFSIZ) {
				write(2,"Disktab entry too long\n", 23);
				break;
			} else
				*cp++ = c;
		}
		*cp = 0;

		/*
		 * The real work for the match.
		 */
		if (DNAMATCH(name, bp)) {
			close(tf);
			return (1);
		}
	}
}

/*
 * Dnamatch deals with name matching.  The first field of the disktab
 * entry is a sequence of names separated by |'s, so we compare
 * against each such name.  The normal : terminator after the last
 * name (before the first field) stops us.
 */
static

#ifdef _THREAD_SAFE
dnamatch(np, tbuf)
	char *np;
	char *tbuf;
#else
dnamatch(np)
	char *np;
#endif /* _THREAD_SAFE */
{
	register char *Np, *Bp;

	Bp = tbuf;
	if (*Bp == '#')
		return (0);
	for (;;) {
		for (Np = np; *Np && *Bp == *Np; Bp++, Np++)
			continue;
		if (*Np == 0 && (*Bp == '|' || *Bp == ':' || *Bp == 0))
			return (1);
		while (*Bp && *Bp != ':' && *Bp != '|')
			Bp++;
		if (*Bp == 0 || *Bp == ':')
			return (0);
		Bp++;
	}
}

/*
 * Skip to the next field.  Notice that this is very dumb, not
 * knowing about \: escapes or any such.  If necessary, :'s can be put
 * into the diskcap file in octal.
 */
static char *
dskip(bp)
	register char *bp;
{

	while (*bp && *bp != ':')
		bp++;
	while (*bp == ':')
		bp++;
	return (bp);
}

/*
 * Return the (numeric) option id.
 * Numeric options look like
 *	li#80
 * i.e. the option string is separated from the numeric value by
 * a # character.  If the option is not found we return -1.
 * Note that we handle octal numbers beginning with 0.
 */
static
#ifdef _THREAD_SAFE
dgetnum(id, tbuf)
	char *id;
	char *tbuf;
#else
dgetnum(id)
	char *id;
#endif /* _THREAD_SAFE */
{
	register int i, base;
	register char *bp = tbuf;

	for (;;) {
		bp = dskip(bp);
		if (*bp == 0)
			return (-1);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return (-1);
		if (*bp != '#')
			continue;
		bp++;
		base = 10;
		if (*bp == '0')
			base = 8;
		i = 0;
		while (isdigit(*bp))
			i *= base, i += *bp++ - '0';
		return (i);
	}
}

/*
 * Handle a flag option.
 * Flag options are given "naked", i.e. followed by a : or the end
 * of the buffer.  Return 1 if we find the option, or 0 if it is
 * not given.
 */
static
#ifdef _THREAD_SAFE
dgetflag(id, tbuf)
	char *id;
	char *tbuf;
#else
dgetflag(id)
	char *id;
#endif /* _THREAD_SAFE */
{
	register char *bp = tbuf;

	for (;;) {
		bp = dskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ == id[0] && *bp != 0 && *bp++ == id[1]) {
			if (!*bp || *bp == ':')
				return (1);
			else if (*bp == '@')
				return (0);
		}
	}
}

/*
 * Get a string valued option.
 * These are given as
 *	cl=^Z
 * Much decoding is done on the strings, and the strings are
 * placed in area, which is a ref parameter which is updated.
 * No checking on area overflow.
 */
static char *

#ifdef _THREAD_SAFE
/*
 * The thrad safe interface can still return a character pointer
 * since it's a pointer into the "area" argument.
 */
dgetstr(id, area, tbuf)
	char *id, **area, *tbuf;
#else
dgetstr(id, area)
	char *id, **area;
#endif /* _THREAD_SAFE */
{

	register char *bp = tbuf;

	for (;;) {
		bp = dskip(bp);
		if (!*bp)
			return (0);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return (0);
		if (*bp != '=')
			continue;
		bp++;
		return (ddecode(bp, area));
	}
}

/*
 * Tdecode does the grung work to decode the
 * string capability escapes.
 */
static char *
ddecode(str, area)
	register char *str;
	char **area;
{
	register char *cp;
	register int c;
	register char *dp;
	int i;

	cp = *area;
	while ((c = *str++) && c != ':') {
		switch (c) {

		case '^':
			c = *str++ & 037;
			break;

		case '\\':
			dp = "E\033^^\\\\::n\nr\rt\tb\bf\f";
			c = *str++;
nextc:
			if (*dp++ == c) {
				c = *dp++;
				break;
			}
			dp++;
			if (*dp)
				goto nextc;
			if (isdigit(c)) {
				c -= '0', i = 2;
				do
					c <<= 3, c |= *str++ - '0';
				while (--i && isdigit(*str));
			}
			break;
		}
		*cp++ = c;
	}
	*cp++ = 0;
	str = *area;
	*area = cp;
	return (str);
}

static
gettype(t, names)
	char *t;
	char **names;
{
	register char **nm;

	for (nm = names; *nm; nm++)
		if (strcasecmp(t, *nm) == 0)
			return (nm - names);
	if (isdigit(*t))
		return (atoi(t));
	return (0);
}

dkcksum(lp)
	register struct disklabel *lp;
{
	register u_short *start, *end;
	register u_short sum = 0;

	start = (u_short *)lp;
	end = (u_short *)&lp->d_partitions[lp->d_npartitions];
	while (start < end)
		sum ^= *start++;
	return (sum);
}

/*
 * This routine creates a disklabel for a unit by obtaining as much 
 * information from the disk as possible.  The intent is that programs like
 * newfs will first search the disktab file for an entry.  If there is no
 * entry corresponding to this disk then try to construct one.
 */
#ifdef	RETURN_FAIL
#undef	RETURN_FAIL
#endif
#ifdef	_THREAD_SAFE
#define	RETURN_FAIL	return TS_FAILURE
#else
#define	RETURN_FAIL	return (struct disklabel *)0
#endif	/* _THREAD_SAFE */

#ifdef	_THREAD_SAFE
int creatediskbyname_r(char *file_name, struct disklabel *dp)
{
#else
struct disklabel *
creatediskbyname(file_name)
	char *file_name;		/* Device special file, ie rrz0a */
{
	static struct disklabel disk;
	register struct	disklabel *dp = &disk;
#endif	/* _THREAD_SAFE */
	struct pt_tbl pt_struct;
	register struct partition *pp;
	int fd;
	DEVGEOMST devgeom;
	struct devget devget_st;
	int i;

	/*
	 * If there is no disktab file at all then do not try to fabricate
	 * an entry.  The disktab file may have been lost and valid entries
	 * could possibly exist for this name.
	 */
	fd = open(DISKTAB, 0);
	if (fd < 0)
		RETURN_FAIL;
	close(fd);

	fd = open(file_name, O_RDONLY);
	if(fd < 0) {
		RETURN_FAIL;
	}
	bzero(dp, sizeof (struct disklabel));

	/*
	 * Call DEVIOCGET to obtain the disk name; ie RA90.
	 */
	if (ioctl(fd, DEVIOCGET, (char *)&devget_st) < 0) {
		strcpy(dp->d_typename,"UNKNOWN"); 
	}
	else {
		strcpy(dp->d_typename,devget_st.device); 
        	if (strcmp(devget_st.dev_name,"rz") == NULL) 
			dp->d_type = DTYPE_SCSI;
        	else if (strcmp(devget_st.dev_name,"ra") == NULL) 
			dp->d_type = DTYPE_MSCP;
        	else 
		/* UNKNOWN device */
			dp->d_type = NULL;
	}

	/*
	 * DEVGETGEOM will attempt to get device geometry information
	 * for this device.  If we can't get back geometry information,
	 * we do not continue.
	 */
	if (ioctl(fd, DEVGETGEOM, (char *)&devgeom) < 0) {
		RETURN_FAIL;
	}
	dp->d_ntracks = devgeom.geom_info.ntracks;
	dp->d_nsectors = devgeom.geom_info.nsectors;
	dp->d_ncylinders = devgeom.geom_info.ncylinders; 
	dp->d_secperunit = devgeom.geom_info.dev_size;
	dp->d_flags = 0; 
	if (devgeom.geom_info.attributes & DEVGEOM_REMOVE) {
		dp->d_flags |= D_REMOVABLE;
	}
	if (devgeom.geom_info.attributes & DEVGEOM_DYNAMIC) {
		dp->d_flags |= D_DYNAM_GEOM;
	}

	/*
	 * Assign default values for other parameters of the disk
	 * label.  These are the same defaults assigned in the
	 * getdiskbyname() routine above (which assigns these
	 * as defaults when it doesn't find the values in the
	 * device's disktab entry).
	 */
	dp->d_secsize = DEV_BSIZE;
	dp->d_secpercyl = dp->d_nsectors * dp->d_ntracks;
	dp->d_rpm = 3600;
	dp->d_interleave = 1;
	dp->d_trackskew = 0;
	dp->d_cylskew = 0;
	dp->d_headswitch = 0;
	dp->d_trkseek = 0;
	dp->d_bbsize = BBSIZE;
	dp->d_sbsize = SBSIZE;

	/*
	 * DIOCGDEFPT will get the driver's default partition table
	 * values for the media which will then be used to plug default
	 * partition info into the disk label.
	 */
	if (ioctl(fd, DIOCGDEFPT, (char *)&pt_struct) < 0) {
		RETURN_FAIL;
	}
	dp->d_npartitions = MAXPARTITIONS;
	for (i = 0; i < MAXPARTITIONS; i++) {
		pp = &dp->d_partitions[i];
		pp->p_size = pt_struct.d_partitions[i].p_size;
		pp->p_offset = pt_struct.d_partitions[i].p_offset;
		pp->p_fsize = 0;
	}
	dp->d_magic = DISKMAGIC;
	dp->d_magic2 = DISKMAGIC;

#ifdef	_THREAD_SAFE
	return TS_SUCCESS;
#else
	return dp;
#endif	/* _THREAD_SAFE */
}
