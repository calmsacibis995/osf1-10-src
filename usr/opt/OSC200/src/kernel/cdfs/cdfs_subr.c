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
static char	sccsid[] = "@(#)$RCSfile: cdfs_subr.c,v $ $Revision: 4.3.9.4 $ (DEC) $Date: 1993/08/03 18:11:19 $";
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
 * Copyright (c) 1989 The Regents of the University of California.
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
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <sys/secdefines.h>

#include <sys/param.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/mode.h>
#include <sys/kernel.h>
#include <sys/limits.h>
#include <cdfs/cdfsnode.h>
#include <cdfs/cdfsmount.h>
#include <cdfs/cdfs.h>
#include <kern/kalloc.h>

#ifdef _KERNEL

/*
 * XXX
 * We only use the first extent of multi-extent-on-one-volume files.
 */

/*
 * Translate on-disk directory record into an in-memory record.
 * only copy the proper byte order stuff.
 */
cdfs_isodir_to_idir(fs, iso_dir, hsg_dir, idir)
struct fs *fs;
struct iso_dir *iso_dir;
struct hsg_dir *hsg_dir;
struct iso_idir *idir;
{
	union {
		unsigned char incoming[4];
		unsigned int  outgoing;
	} iso_convert_int;

	if (fs->fs_format == ISO_9660 || fs->fs_format == ISO_RRIP) {
		idir->dir_len = iso_dir->dir_len;
		idir->dir_xar = iso_dir->dir_xar;
		bcopy(iso_dir->dir_extent_lsb, 
		      iso_convert_int.incoming, sizeof(int));
		idir->dir_extent = iso_convert_int.outgoing;
		bcopy(iso_dir->dir_dat_len_lsb, 
		      iso_convert_int.incoming, sizeof(int));
		idir->dir_dat_len = iso_convert_int.outgoing;
		bcopy(iso_dir->dir_dt, idir->dir_dt, 
		      sizeof(iso_dir->dir_dt));
		idir->dir_file_flags = iso_dir->dir_file_flags;
		idir->dir_file_unit_size = 
			iso_dir->dir_file_unit_size;
		idir->dir_inger_gap_size = 
			iso_dir->dir_inger_gap_size;
		idir->dir_vol_seq_no = iso_dir->dir_vol_seq_no_lsb;
	} else {
		idir->dir_len = hsg_dir->dir_len;
		idir->dir_xar = hsg_dir->dir_xar;
		bcopy(hsg_dir->dir_extent_lsb, 
		      iso_convert_int.incoming, sizeof(int));
		idir->dir_extent = iso_convert_int.outgoing;
		bcopy(hsg_dir->dir_dat_len_lsb, 
		      iso_convert_int.incoming, sizeof(int));
		idir->dir_dat_len = iso_convert_int.outgoing;
		bcopy(hsg_dir->dir_dt, idir->dir_dt, 
		      sizeof(hsg_dir->dir_dt));
		idir->dir_file_flags = hsg_dir->dir_file_flags;
		idir->dir_file_unit_size = 
			hsg_dir->dir_file_unit_size;
		idir->dir_inger_gap_size = 
			hsg_dir->dir_inger_gap_size;
		idir->dir_vol_seq_no = hsg_dir->dir_vol_seq_no_lsb;
	}
}

/*
 * Read an iso directory record. Stuff record into file system
 * specific portion of gnode. gp->g_number is set to the
 * disk location of the directory record we will read.
 */

cdfs_readisodir(cdp, ret_isodirp)
	register struct cdnode *cdp;
	struct iso_dir	*ret_isodirp;
{
    return cdfs_readisodir_int(cdp->cd_fs,
			       cdp->cd_number,
			       cdp->cd_devvp,
			       cdp->cd_dev,
			       &cdp->cd_cdin, ret_isodirp);
}

/*
 * Read a directory entry off the disc.
 * 
 * ARGS:
 *	fs	(in)		ptr to superblock for relevant file system
 *	num	(in)		inode number (== byte offset within file system
 *				 to directory entry) of file whose entry
 *				 is desired
 *	devvp	(in)		device vnode pointer for bread()
 *	dev	(in)		dev_t of devvp (for debugging)
 *	ret_isoidir	(out)	where to put translated directory entry
 *	ret_isodirp	(out)	where to put raw directory
 *
 * if RRIP or ISO9660 and ret_isodirp is non-null,
 * the raw entry is copied to it (it must be at least as big as the entry;
 * ISO_MAXDIRENTLEN is a good size)
 *
 * otherwise, the converted entry is placed in *ret_isoidir.
 */
cdfs_readisodir_int(fs, num, devvp, dev, ret_isoidir, ret_isodirp)
	struct fs *fs;
	ino_t num;
	struct vnode *devvp;
	dev_t dev;
	struct iso_idir *ret_isoidir;
	struct iso_dir	*ret_isodirp;
{
	struct buf *bp;
	unsigned int loc;
	unsigned int off;
	int error;
	struct iso_dir *iso_dir = 0;
	struct hsg_dir *hsg_dir = 0;
	union {
		unsigned char incoming[4];
		unsigned int  outgoing;
	} iso_convert_int;

	CDDEBUG1(ISODEBUG, printf("cdfs_readisodir: Inside\n"));
	/*
	 * Since a directory record cannot span a logical sector,
	 * read in the entire sector.
	 */
	off = num % fs->fs_ibsize;
	loc = num - off;
	if (error = bread(devvp, btodb(loc), fs->fs_ibsize, NOCRED,
			  &bp)) {
		printf("cdfs_readisodir: Cannot read block %d of dev 0x%x\n",
		       btodb(loc), dev);
		brelse(bp);
		return(error);
	}

	if (fs->fs_format == ISO_9660 || fs->fs_format == ISO_RRIP)
		iso_dir = (struct iso_dir *)(bp->b_un.b_addr + off);
	else
		hsg_dir = (struct hsg_dir *)(bp->b_un.b_addr + off);

	/*
	 * if we only want to read the iso_dir, don't translate
	 * it (again), just copy it and we're done.
	 */
	if (ret_isodirp && (fs->fs_format == ISO_9660 ||
			    fs->fs_format == ISO_RRIP)) {
	    CDDEBUG1(ISODEBUG,
		     printf("cdfs_readisodir: copy %d bytes\n",
			    iso_dir->dir_len));
	    /* copy whole thing (including any SUA): */
	    bcopy(iso_dir, ret_isodirp, iso_dir->dir_len);
	} else
	    cdfs_isodir_to_idir(fs, iso_dir, hsg_dir, ret_isoidir);
	brelse(bp);
	CDDEBUG1(ISODEBUG, printf("cdfs_readisodir: Leaving\n"));
	return(0);
}



/*
 * cdfs_readxar():	read in an XAR from disc and copy relevant fields to
 *			cdnode
 *
 * ARGS:
 *	cdp		(in/out) cd node of file for which XAR should be
 *					interpreted
 *	cdmntp		(in)	cdfs mount structure pointer, accessed to
 *					get mount/XCDR options
 *	uio		(in/out) where to put a copy of the XAR (if non-null)
 */
/*
 * cdfs_readxar() may be called if RRIP PX field was found on the file,
 * in which case the caller is only interested in timestamps.
 */
int
cdfs_readxar(cdp, cdmntp, uio)
	struct cdnode *cdp;
	struct cdfsmount *cdmntp;	/* needed for mount options */
	struct uio *uio;		/* optionally return xar to here */
{
	struct fs *fs;
	int bn;
	int loc;
	int off;
	int lbs;
	struct buf *bp = 0;
	struct iso_xar *iso_xarp;
	struct hsg_xar *hsg_xarp;
	int error = 0;
	int noprot = 0;

	fs = cdp->cd_fs;
	if (cdp->iso_dir_xar) {
	    lbs = ISOFS_LBS(fs);

	    off = (cdp->iso_dir_extent * lbs) % fs->fs_ibsize;
	    loc = (cdp->iso_dir_extent * lbs) - off;
	    
	    bn = btodb(loc);
	    if (error = bread(cdp->cd_devvp, bn, fs->fs_ibsize, NOCRED, &bp))
		goto out;
	    /*
	     * if not (using DEFPERM mode and protection info in XAR)
	     * don't interpret protection flags.
	     */
	    if (!(cdmntp->um_flag & M_NODEFPERM &&
		  cdp->iso_dir_file_flags&ISO_FLG_PROTECT))
		noprot = 1;
	} else {
	    /* there is no XAR to read. */
	    noprot = 1;
	    goto out;
	}
	if (fs->fs_format == ISO_RRIP && isdone(cdp, PX))
	    noprot = 1;			/* just do timestamps */
	if (fs->fs_format == ISO_9660 || fs->fs_format == ISO_RRIP) {
	    iso_xarp = (struct iso_xar *)(bp->b_un.b_addr + off);
	    if (iso_xarp->iso_xar_version != 1) {
		printf("cdfs_readxar: XAR has %d for version\n",
		       iso_xarp->iso_xar_version);
		goto out;
	    }
	    if (fs->fs_format != ISO_RRIP || !isdone(cdp, TF)) {
		/* we need some timestamps */
		/*
		 * pull up timestamps, per XCDR.
		 */
		cdp->cd_ctime = cdfs_longdate(iso_xarp->iso_xar_dtcre);
		cdp->cd_mtime = cdfs_longdate(iso_xarp->iso_xar_dtmod);
		cdp->cd_atime = cdp->cd_mtime;
	    }
	    /*
	     * see if copy of xar desired.
	     */
	    if (uio) {
		/* to do: XCDR copies of XAR */
	    }
	    if (noprot)
		goto out;
#ifndef __alpha
#error Make sure uid_t and gid_t can hold any on-disk format.
#endif	   
	    /*
	     * since our uid_t/gid_t can hold 16-bits, we don't need to
	     * worry about on-disk identifiers being out-of-band.
	     * see XCDR XAR Fields table, sec 4.2.
	     */
	    if (iso_xarp->iso_xar_oid)
		cdp->cd_uid = (uid_t) iso_xarp->iso_xar_oid;
	    if (iso_xarp->iso_xar_gid_lsb)
		cdp->cd_gid = (gid_t) iso_xarp->iso_xar_gid_lsb;
	    /* note that all perms were already turned on by the caller;
	       this code turns off bits that should not be on */
	    if (iso_xarp->iso_xar_perm & ISO_NOT_OWN_READ)
		cdp->cd_mode &= ~S_IRUSR;
	    if (iso_xarp->iso_xar_perm & ISO_NOT_OWN_EXEC)
		cdp->cd_mode &= ~S_IXUSR;
	    if (iso_xarp->iso_xar_perm & ISO_NOT_GRP_READ)
		cdp->cd_mode &= ~S_IRGRP;
	    if (iso_xarp->iso_xar_perm & ISO_NOT_GRP_EXEC)
		cdp->cd_mode &= ~S_IXGRP;
	    if (iso_xarp->iso_xar_perm & ISO_NOT_OTH_READ)
		cdp->cd_mode &= ~S_IROTH;
	    if (iso_xarp->iso_xar_perm & ISO_NOT_OTH_EXEC)
		cdp->cd_mode &= ~S_IXOTH;
	} else {
	    hsg_xarp = (struct hsg_xar *)(bp->b_un.b_addr + off);
	    if (hsg_xarp->iso_xar_version != 1) {
		printf("cdfs_readxar: XAR has %d for version\n",
		       hsg_xarp->iso_xar_version);
		goto out;
	    }
	    /*
	     * pull up timestamps, per XCDR (even though HSG cannot be
	     * XCDR compliant, we do it)
	     */
	    /* don't know format without spec. XXX */

	    if (noprot)
		goto out;

	    if (hsg_xarp->iso_xar_oid)
		cdp->cd_uid = (uid_t) hsg_xarp->iso_xar_oid;
	    if (hsg_xarp->iso_xar_gid_lsb)
		cdp->cd_gid = (gid_t) hsg_xarp->iso_xar_gid_lsb;
	    if (hsg_xarp->iso_xar_perm & ISO_NOT_OWN_READ)
		cdp->cd_mode &= ~S_IRUSR;
	    if (hsg_xarp->iso_xar_perm & ISO_NOT_OWN_EXEC)
		cdp->cd_mode &= ~S_IXUSR;
	    if (hsg_xarp->iso_xar_perm & ISO_NOT_GRP_READ)
		cdp->cd_mode &= ~S_IRGRP;
	    if (hsg_xarp->iso_xar_perm & ISO_NOT_GRP_EXEC)
		cdp->cd_mode &= ~S_IXGRP;
	    if (hsg_xarp->iso_xar_perm & ISO_NOT_OTH_READ)
		cdp->cd_mode &= ~S_IROTH;
	    if (hsg_xarp->iso_xar_perm & ISO_NOT_OTH_EXEC)
		cdp->cd_mode &= ~S_IXOTH;
	}
out:
	if (bp)
	    brelse(bp);
	return(error);
}

static	int	dmsize[12] =
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
extern struct timezone tz;
/* leap year is something that == 0 mod 4 and != 0 mod 100,
   or == 0 mod 400 */
#define	dysize(A) (((A)%4)? 365 : (!((A) % 100) && ((A) % 400)) ? 365 : 366)

/*
 * cdfs_tounixdate():	convert 7-byte ISO date format to UNIX timestamp.
 *
 * ARGS:
 *	ret_time:	(out)	fill in the result time here
 *	y,m,d,h,mi,s:	(in)	one-byte quantities representing year, month,
 *				day, hour, minutes, seconds
 *	o:		(in)	signed quantity of 15-minute intervals
 *				from GMT.
 */
cdfs_tounixdate(ret_time,yrs,mon,dys,hrs,mins,secs,gmtoff)
time_t *ret_time;
char yrs;
char mon;
char dys;
char hrs;
char mins;
char secs;
char gmtoff;
{
	register int i;
#ifdef __alpha
	register unsigned long val = 0;
#else
#error  You need to figure out how to detect timeval overflow
#endif
	int iy;
	int im;


	*ret_time = (time_t)0;
	if (yrs < 0 || yrs > 100 || mon < 1 || mon > 12 ||
	    dys < 1 || dys > 31 || hrs < 0 || hrs > 23 ||
	    mins < 0 || mins > 59 || secs < 0 || secs > 59)
		
		return(0);
	/* XCDR says dates before the oldest representable time
	   are to be returned as (time_t) 0.
	   1970 is the witching hour.
	   XXX gmt offset? */
	if (yrs >= 69) {
	    iy = yrs + 1900;
	    for (i = 1970; i < iy; i++)
		val += dysize(i);
	    im = mon;
	    /* 
	     * Leap year 
	     */
	    if (dysize(iy) == 366 && im >= 3)
		val++;
	    /*
	     * Do the current year
	     */
	    while(--im)
		val += dmsize[im-1];
	    val += dys-1;
	    val = 24*val + hrs;
	    val = 60*val + mins;
	    val = 60*val + secs;
	    /* o is signed value, in 15-minute increments.
	       negative is west (behind GMT)
	       positive is east (ahead of GMT).
	       subtract so that, e.g. noon US EST (which is GMT-5h, or -20)
	       becomes 5pm GMT */
	    val -= gmtoff * 15 * 60;	
	    /* now, if it was 1969, take off the year and see if we
	       still fit */
	    if (yrs == 69) {
		/* 1969 was not a leap year */
		if (365*24*60*60 < val)
		    val -= 365*24*60*60;
		else
		    val = 0;		/* too early */
	    }
	}
#ifdef __alpha
	/* depends on time_t being typedef'd as an int */
	if (val > INT_MAX)
	    *ret_time = INT_MAX;
	else
#endif
	*ret_time = (time_t)val;
	return (0);
}

/*
 * cdfs_longdate(): convert 8.4.26.1-style long time format to time_t.
 *
 * ARGS:
 *	tstring:	(in)	bytes of the timestamp.
 * RETURNS:
 *	the time_t representation of that time.
 */

time_t
cdfs_longdate(signed char *tstring)
{
    time_t retval;
    register int i;
#ifdef __alpha
    register unsigned long val = 0;
#else
#error  You need to figure out how to detect timeval overflow
#endif
    int iy;
    int im;
    /* The long form is YYYYMMDDHHMMSSo, with YMDHMS as ASCII digits,
       and o as an unsigned GMT offset */

    /*
     * Added up the seconds since the epoch
     *  val = o - tz.tz_minuteswest;  should divide into 24 hour periods
     */
#define digitval(c) (c - '0')
#define twodigits(cp) (digitval((cp)[0])*10+digitval((cp)[1]))
    iy = twodigits(tstring);
    tstring += 2;
    iy = iy*100 + twodigits(tstring);
    tstring += 2;
    /* per XCDR, old dates are time_t 0 */
    if (iy < 1970)
	return (time_t) 0;
    for (i = 1970; i < iy; i++)
	val += dysize(i);
    im = twodigits(tstring);
    tstring += 2;
    /* 
     * Leap year 
     */
    if (dysize(iy) == 366 && im >= 3)
	val++;
    /*
     * Do the current year
     */
    while(--im)
	val += dmsize[im-1];
    val += twodigits(tstring)-1;
    tstring += 2;
    val = 24*val + twodigits(tstring);	/* hours */
    tstring += 2;
    val = 60*val + twodigits(tstring);	/* minutes */
    tstring += 2;
    val = 60*val + twodigits(tstring);	/* secs */
    tstring += 2;
    tstring += 2;			/* can't resolve hundredths */
    /* o is signed value, in 15-minute increments.
       negative is west (behind GMT)
       positive is east (ahead of GMT).
       subtract so that, e.g. noon US EST (which is GMT-5h)
       becomes 5pm GMT */
    val -= *tstring * 15 * 60;	
#ifdef __alpha
    /* depends on time_t being typedef'd as an int */
    if (val > INT_MAX)
	retval = INT_MAX;
    else
#endif
	retval = (time_t)val;
    return retval;
}

/*
 * rrip_convert_tf_ts():	convert RRIP timestamp to UNIX time.
 *				either a 7-byte or a 17-byte quantity.
 *
 * ARGS:
 *
 *	longform:	(in)	is this a long-form timestamp?
 *	tstring: 	(in)	bytes of the timestamp
 *
 * RETURNS:
 *	the time_t representation of that time.
 */
time_t
rrip_convert_tf_ts(int longform,
		   signed char *tstring)
{
    time_t retval;
    if (longform)
	retval = cdfs_longdate(tstring);
    else
	cdfs_tounixdate(&retval,
			tstring[0],
			tstring[1],
			tstring[2],
			tstring[3],
			tstring[4],
			tstring[5],
			tstring[6]);
    return retval;
}

/*
 * susp_* are a collection of routines to read and return pointers to
 * SUSP fields.
 *
 * susp_search() does most of the work, including searching of
 * 	continuation areas and recording RRIP shortcut pointers.
 *
 * susp_compute_diroff() sets up a count and pointer to the SUSP part of
 * 	a directory record's SUA.
 *
 * susp_search_dir() searches a directory entry for a particular SUSP field.
 *
 * susp_search_cached() uses cached cdnode pointers, if available, to
 *	search a directory for a particular field.  The only things cached
 *	currently are pointers to RRIP fields that live in the directory
 *	entry.
 */

/*
 * susp_search(): Search in system use area for a given SUSP field.
 *
 * Also records location of RRIP fields found along the way, if a cdnode
 * is passed in.
 * 
 * Even if we don't see an RR field, we deduce one if we run out
 * of SUSP entries.
 *
 * ARGS:
 *	fs:	(in)		cdfs superblock pointer
 *	devvp:	(in)		device vnode for bread()ing blocks
 *	inbuf:	(in/out)	pointer to initial SUA to search for
 *				the desired record.
 *				on exit, it's modified to point to the
 *				next place to search, if we need to continue
 *				the search for another entry of the same type.
 *				it *MAY* point into a new buffer from bread()
 *	count:	(in/out)	count of bytes of SUA at *inbuf.
 *				on exit, modified to count remaining bytes
 *				at *inbuf
 *	sigmatch: (in)		TRUE if signature should be matched
 *	searchfor: (in)		signature of what we're looking for
 *	bpp:	(in/out)	buf pointer of buffer into which *inbuf points.
 *				only set on entry if it should be brelse()d
 *				if we move to another block.
 *				on exit, *bpp is set to point the buf into
 *				which *inbuf points (if non-zero).
 *				caller should brelse() it when done, or pass
 *				it back into a subsequent call to
 *				susp_search().
 *	cdp:	(in/out)	cdnode pointer; if non-NULL, record shortcut
 *				info in the cdnode.
 *	cont:	(in/out)	hold continuation area copy, for routines
 *				which do detailed start/stop examinations
 *				of SUSP fields and need to restart continuation
 *				area lookups.  If a CE is found, it's filled
 *				into this pointer, if non-NULL
 *
 * RETURNS:	pointer to the desired SUF, or NULL if not found.
 *		also sets *bpp, *count, *inbuf, *cont as appropriate.
 *		if returns NULL, *bpp is zero.
 */


struct cd_suf_header *
susp_search(struct fs *fs,
	    struct vnode *devvp,
	    unsigned char **inbuf,
	    int *count,
	    int sigmatch,
	    const char *searchfor,
	    struct buf **bpp,
	    struct cdnode *cdp,
	    struct cd_suf_ce *cont)
{
    struct cd_suf_header *hdr;
    static struct cd_suf_ce cont_area = { {SUF_CONTINUATION_AREA,
					       SUF_CE_LEN, SUF_CE_VERS} };
    static struct cd_suf_st terminator = { {SUF_SUSP_TERMINATOR,
						SUF_ST_LEN, SUF_ST_VERS} };
    static unsigned char rrip_rr_sig[2] = RRIP_SIG_RR;

    int found_cont = 0;
    unsigned char *first_sua = *inbuf;	/* is this the first SUA we
					   are searching? */
    int i;
    int error;
    daddr_t lbn;
    union {
	unsigned char incoming[4];
	unsigned int outgoing;
    } convert_extent;
    int lbs = ISOFS_LBS(fs);

    if (cont && SUSP_SHORTIFY(cont_area.hdr.suf_sig_word) ==
	SUSP_SHORTIFY(cont->hdr.suf_sig_word)) {
	/* caller passed in a continuation area pointer */
	bcopy(cont, &cont_area, sizeof(cont_area));
	/* and erase the copy so we don't try to use it again */
	bzero(cont, sizeof(*cont));
#ifdef CDFSDEBUG
	if (SUSPDEBUG) {
	    CDFS_COPYINT(cont_area.cont_lbn_lsb, convert_extent.incoming);
	    printf("susp_search: cont'd at user-specified 0x%x\n",
		   convert_extent.outgoing);
	}
#endif /* CDFSDEBUG */
	found_cont = 1;
    }
    for (;;) {
	while (*count >= sizeof(*hdr)) {
	    hdr = (struct cd_suf_header *)*inbuf;
	    CDDEBUG1(SUSPDEBUG,
		     printf("record %c%c (%d bytes)\n",
			    hdr->suf_sig_word[0], hdr->suf_sig_word[1],
			    hdr->suf_length));
	    if (hdr->suf_length > *count) {
		CDDEBUG1(WEIRDDEBUG || SUSPDEBUG,
			 printf("susp_search: record too long\n"));
		break;
	    }
	    if (!hdr->suf_length) {
		CDDEBUG1(WEIRDDEBUG || SUSPDEBUG,
			 printf("susp_search: 0-len record, no terminator\n"));
		break;			/* from while loop */
	    }
	    if (SUSP_SHORTIFY(hdr->suf_sig_word) == RRIP_SHORTIFY_SIG(RR)
		&& cdp && !(cdp->cd_rrip_fields & CDNODE_RRIP_RR_SEEN)) {
		/* if this is a field indicating what RRIP elements are
		   recorded, then keep track to help other code
		   avoid some extra work */
		cdp->cd_rrip_fields |=
		    ((struct rrip_rr *)hdr)->rr_present | CDNODE_RRIP_RR_SEEN;
		cdp->cd_rrip_fields &= ~CDNODE_RRIP_RR_NOT;
	    }
	    if (!bcmp(hdr, &terminator, sizeof(*hdr))) {
		break;			/* from the while loop */
	    }
	    if (cdp) {			/* if we can record things, do so: */
		switch(SUSP_SHORTIFY(hdr->suf_sig_word)) {
#define docase(xx) case RRIP_SHORTIFY_SIG(xx): \
		    i = CDNODE_ ## xx ## _OFF; break
		    docase(PX);
		    docase(PN);
		    docase(SL);
		    docase(NM);
		    docase(CL);
		    docase(PL);
		    docase(RE);
		    docase(TF);
#undef docase
		default:
		    i = -1;
		}

		if (i >= 0 &&
		    !(cdp->cd_rrip_fields & (1 << (i + CDNODE_SEEN_SHIFT)))) {
		    /* this is the first encounterd thing of a known type
		       record its sighting, and maybe location */
		    cdp->cd_rrip_fields |= (1 << i);
		    if (first_sua) {
			CDDEBUG1(SUSPDEBUG,
				 printf("saving offset %d\n",
					*inbuf - first_sua));
			cdp->cd_rrip_fldoffset[i] = *inbuf - first_sua;
			cdp->cd_rrip_fields |= (1 << (i + CDNODE_SEEN_SHIFT));
		    }
		}
	    }
	    if (!found_cont && !bcmp(hdr, &cont_area.hdr, sizeof(*hdr))) {
		bcopy(hdr, &cont_area, sizeof(cont_area));
		/* and copy it back to the caller if desired: */
		if (cont) {
		    bcopy(hdr, cont, sizeof(*cont));
		}
#ifdef CDFSDEBUG
		if (SUSPDEBUG) {
		    CDFS_COPYINT(cont_area.cont_lbn_lsb,
				 convert_extent.incoming);
		    printf("susp_search: cont'd at 0x%x\n",
			   convert_extent.outgoing);
		}
#endif /* CDFSDEBUG */
		found_cont = 1;
	    }
	    if (!sigmatch ||		/* match anything if sigmatch == 0 */
		(SUSP_SHORTIFY(hdr->suf_sig_word) == SUSP_SHORTIFY(searchfor)
		 && hdr->suf_version == 1)) {
		/* XXX other versions? */
		*count -= hdr->suf_length;
		*inbuf += hdr->suf_length;
		/* if the caller did not look for a CE, then zap
		   any stored CE (the other ptrs will Do the Right Thing) */
		if (cont && sigmatch &&
		    SUSP_SHORTIFY(searchfor) !=
		    SUSP_SHORTIFY(cont_area.hdr.suf_sig_word))
		    bzero(cont, sizeof(*cont));
		return hdr;
	    }
	    *count -= hdr->suf_length;
	    *inbuf += hdr->suf_length;
	} /* end while (space_left */
	if (*bpp)
	    brelse(*bpp);
	*bpp = 0;
	if (!found_cont) {
	    /* no terminator, but we ran out of things to look through */
	    if (cdp) {
		cdp->cd_rrip_fields &= ~CDNODE_RRIP_RR_NOT;
		/* haven't really seen it, but have seen everything there
		   is to see: */
		cdp->cd_rrip_fields |= CDNODE_RRIP_RR_SEEN;
	    }
	    return 0;
	}
	first_sua = 0;
	found_cont = 0;
	/* go fetch the continuation area */
	CDFS_COPYINT(cont_area.cont_lbn_lsb, convert_extent.incoming);
	/* addresses are in lbs chunks, but we read blocks in ISO_SECSIZE
	   chunks */
	lbn = convert_extent.outgoing * (lbs / DEV_BSIZE);
	if (error = bread(devvp, lbn, ISO_SECSIZE, NOCRED, bpp)) {
	    CDDEBUG1(SUSPDEBUG,
		     printf("susp_search: read failed at lbn %x\n", lbn));
	    return 0;
	}
	/* got the block.  continue the search */
	CDFS_COPYINT(cont_area.cont_len_lsb, convert_extent.incoming);
	*count = convert_extent.outgoing;

	CDFS_COPYINT(cont_area.cont_offset_lsb, convert_extent.incoming);
	if (convert_extent.outgoing + *count  > ISO_SECSIZE) {
	    int size = roundup(*count, DEV_BSIZE);
	    CDDEBUG1(SUSPDEBUG,
		     printf("susp_search: continuation area overflows sector\n"));
	    /* The SUSP spec is unclear in 5.1.  While you could interpret it
	       to mean that the continuation area should not span logical
	       sectors, the intent was that any logical sector containing
	       continuation areas should have no other data in that sector.

"What we intended was that AT LEAST a full sector will be allocated
for continuation area(s), even if not completely consumed."

               (per e-mail discussions with Andy Young <ayoung@ymi.com>).
	       
	       In the present case, a CE spans sectors so we need to do some
	       gross hackery to get at this continuation area.
	       */
	    if (size > MAXBSIZE) {
		printf("susp_search: continuation area too big\n");
		brelse(*bpp);
		*bpp = 0 ;
		return 0;
	    } else {
		int lefthere, copied;
		struct buf *bogusbuf;

		/* geteblk() gets a block buffer, and marks it invalid, so
		   brelse() will just trash it completely. */
		bogusbuf = geteblk(size);

		/* now copy what portions we have into the block, and
		   then get the rest */

		/* lefthere == how much to copy from this sector */
		lefthere = ISO_SECSIZE - convert_extent.outgoing;
		bcopy(((unsigned char *)(*bpp)->b_un.b_addr) +
		      convert_extent.outgoing,
		      bogusbuf->b_un.b_addr, lefthere);

		copied = lefthere;
		while (copied < *count) {
		    /* more to go: get next sector */
		    brelse(*bpp);
		    *bpp = 0;
		    lbn += ISO_SECSIZE / DEV_BSIZE;
		    if (error = bread(devvp, lbn, ISO_SECSIZE, NOCRED, bpp)) {
			CDDEBUG1(SUSPDEBUG,
				 printf("susp_search: read failed at lbn %x\n",
					lbn));
			brelse(bogusbuf);
			return 0;
		    }
		    bcopy((*bpp)->b_un.b_addr,
			  (unsigned char *)bogusbuf->b_un.b_addr + copied,
			  MIN(ISO_SECSIZE, *count - copied));
		    copied += MIN(ISO_SECSIZE, *count - copied);
		}
		brelse(*bpp);
		*bpp = bogusbuf;
	    }
	    *inbuf = ((unsigned char *)(*bpp)->b_un.b_addr);
	} else
	    *inbuf = ((unsigned char *)(*bpp)->b_un.b_addr) +
		convert_extent.outgoing;

	/* go around again ... */
    }
}

/*
 * susp_compute_diroff(): compute count & pointers in preparation for calling
 *	susp_search().
 *
 * ARGS:
 *	dirp:	(in)	ptr to iso directory in which to start search
 *	skip:	(in)	SUSP SUA skip parameter (from fs superblock)
 *	ctp:	(out)	filled in with count for susp_search()
 *	bufpp:	(out)	filled in with ptr to SUSP area, for susp_search()
 */
void
susp_compute_diroff(struct iso_dir *dirp,
		    int skip,
		    int *ctp,
		    unsigned char **bufpp)
{
    int namelen = dirp->dir_namelen;
    if (!(namelen & 0x1))
	namelen++;	/* round up if even. */
    /* what's available? */
    *ctp = dirp->dir_len - (sizeof(*dirp) - 1) - namelen - skip; 
    *bufpp = &dirp->dir_name[namelen + skip];
}

/*
 * susp_search_cached(): return cached offset into SUA of a given cdnode, if
 *	present; else call susp_search()
 *
 * ARGS:
 * [NOTE: all of these are passed along to susp_search(); see its description
 * for details]
 *	fs:	(in)		cdfs superblock pointer
 *	devvp:	(in)		device vnode for bread()ing blocks
 *	inbuf:	(in/out)	pointer to SUA to search for
 *				the desired record. modified to point to next
 *				record.
 *	count:	(in/out)	count of bytes of SUA at *inbuf.
 *				on exit, modified to count remaining bytes
 *				at *inbuf
 *	sigmatch: (in)		TRUE if signature should be matched
 *	searchfor: (in)		signature of what we're looking for
 *	bpp:	(in/out)	buf pointer of buffer into which *inbuf points.
 *	cdp:	(in/out)	cdnode pointer, look here for hints
 *	cont:	(in/out)	CE pointer (saved/reused by susp_search())
 *
 * RETURNS:	pointer to the desired SUF, or NULL if not found.
 *		also sets *bpp, *count, *inbuf as appropriate.
 *		if returns NULL, *bpp is zero.
 */
struct cd_suf_header *
susp_search_cached(struct fs *fs,
		   struct vnode *devvp,
		   unsigned char **inbuf,
		   int *count,
		   int sigmatch,
		   const char *searchfor,
		   struct buf **bpp,
		   struct cdnode *cdp,
		   struct cd_suf_ce *cont)
{
    int i;
    /* take any shortcuts possible, if we know anything other than NOT: */
    if (cdp->cd_rrip_fields & ~CDNODE_RRIP_RR_NOT) {
	switch(SUSP_SHORTIFY(searchfor)) {
#define docase(xx) case RRIP_SHORTIFY_SIG(xx): \
	    i = CDNODE_ ## xx ## _OFF; break
	    docase(PX);
	    docase(PN);
	    docase(SL);
	    docase(NM);
	    docase(CL);
	    docase(PL);
	    docase(RE);
	    docase(TF);
#undef docase
	default:
	    i = -1;
	}
	if (i >= 0 && (cdp->cd_rrip_fields & (1 << i)) &&
	    (cdp->cd_rrip_fields & (1 << (CDNODE_SEEN_SHIFT + i)))) {
	    /* just return the known offset */
	    struct cd_suf_header * ret;
	    CDDEBUG1(SUSPDEBUG,
		     printf("returning offset %d\n",
			    cdp->cd_rrip_fldoffset[i]));

	    ret = (struct cd_suf_header *)(*inbuf + cdp->cd_rrip_fldoffset[i]);
	    /* and advance pointers */
	    *inbuf += cdp->cd_rrip_fldoffset[i] + ret->suf_length;
	    *count -= cdp->cd_rrip_fldoffset[i] + ret->suf_length;
	    if (count == 0) {
		/* XXX what if last part of SUA, but continued?  do we lose? */
		panic("ran out of SUA!");
	    }
	    return ret;
	}
    }
    /* no shortcut */
    return susp_search(fs, devvp, inbuf, count, sigmatch, searchfor, bpp, cdp,
		       cont);
}

/*
 * susp_search_dir():	search an ISO directory entry for a particular
 *	SUSP field.  return pointers to allow continuation of the search,
 *	for those things that want more than one of the same type (SL, NM)
 *
 * ARGS:
 *	fs:	(in)		CDFS superblock pointer
 *	devvp:	(in)		device to read from for bread()
 *	dirp:	(in)		pointer to directory entry in question
 *	skip:	(in)		SUSP skip offset for the SUA
 *	searchfor:	(in)	SUSP signature (what to look for)
 *	sigmatch:	(in)	TRUE if signature should be matched
 *	bpp:	(out)		buf structure pointer, filled in if
 *				the search took us to another block.
 *				if *bpp non-zero on return, the caller
 *				must brelse() the block when done with
 *				the SUSP field
 *	cdp:	(in/out)	cdnode pointer corresponding to this
 *				diretory entry.  if non-NULL, then
 *				store RRIP hints in the node as fields are
 *				discovered.
 * RETURNS:	pointer to the desired SUF, or NULL if not found.
 *		also sets *bpp if it's in a different block
 *		also fills in RRIP hints in *cdp, if cdp non-NULL.
 */
struct cd_suf_header *
susp_search_dir(struct fs *fs,
		struct vnode *devvp,
		struct iso_dir *dirp,
		int skip,
		int sigmatch,
		const char *searchfor,
		struct buf **bpp,
		struct cdnode *cdp)
{
    int count;
    unsigned char *inbuf;

    *bpp = 0;
    susp_compute_diroff(dirp, skip, &count, &inbuf);
    if (cdp) {
	return susp_search_cached(fs, devvp, &inbuf, &count,
				  sigmatch, searchfor, bpp, cdp, 0);
    }
    return susp_search(fs, devvp, &inbuf, &count, sigmatch,
		       searchfor, bpp, cdp, 0);
}


#ifdef CDFSDEBUG
#define LEN_CHECK(max, len, new, action)	if ((len) + (new) > (max)-1){ \
					if (RRIPDEBUG || WEIRDDEBUG) \
					 printf("cdfs: RRIP name too big\n"); \
					action \
					goto cleanup; }
#else
#define LEN_CHECK(max, len, new, action)	if ((len) + (new) > (max)-1){ \
					action \
					goto cleanup; }
#endif /* CDFSDEBUG */
/*
 * rrip_compose_altname():	given a directory entry, use the RRIP NM
 *	fields to construct the alternate name by which to know this entry.
 *	
 * ARGS:
 *	fs:	(in)		CDFS superblock pointer
 *	dirp:	(in)		pointer to directory entry
 *	bpp:	(out)		buf structure pointer, filled in if
 *				the search took us to another block.
 *				if *bpp non-zero on return, the caller
 *				must brelse() the block when done with
 *				the name.
 *	devvp:	(in)		device to read from for bread()
 *	retlength:	(out)	filled in with length of extended
 *				name (if found)
 *	dealloc:	(out)	filled in to 1 if the returned name
 *				should be deallocated with PN_DEALLOCATE()
 * RETURNS:	pointer to new name, or NULL if no alternate name
 *		also sets *bpp if it's in a different block
 *		also sets *dealloc appropriately
 */
unsigned char *
rrip_compose_altname(struct fs *fs,
		     struct iso_dir *dirp,
		     struct buf **bpp,
		     struct vnode *devvp,
		     int *retlength,
		     int *dealloc)
{

    struct rrip_nm *alt_name;
    unsigned char *name_check = dirp->dir_name;
    unsigned char *susp_bufp = 0;
    unsigned char *assemble_name = 0;
    int assemble_len = 0, continuing = 0;
    int count, length;
    struct cd_suf_ce cont;

    if ((dirp->dir_file_flags&ISO_FLG_DIR) &&
	dirp->dir_namelen == 1 &&
	/* ignore NM field if this is special . or .. entry */
	(dirp->dir_name[0] == '\0' ||
	 dirp->dir_name[0] == '\1'))
	return 0;
    susp_compute_diroff(dirp, fs->rrip_susp_offset,
			&count, &susp_bufp);
    *bpp = 0;
    bzero(&cont, sizeof(cont));
    do {
	alt_name = (struct rrip_nm *) susp_search(fs, devvp,
						  &susp_bufp,
						  &count, TRUE,
						  RRIP_SIG_NM, bpp, 0, &cont);
	if (continuing && !alt_name) {
	    CDDEBUG1(WEIRDDEBUG || RRIPDEBUG,
		     printf("cdfs_altname: RRIP continued name missing\n"));
	goodbye:
	    if (assemble_name)
		PN_DEALLOCATE(assemble_name);
	    if (*bpp)
		brelse(*bpp);
	    *bpp = 0;
	    return 0;
	}
	if (!alt_name)
	    return 0;

	if (alt_name->hdr.suf_length < 5) {
	    CDDEBUG1(WEIRDDEBUG || RRIPDEBUG,
		     printf("cdfs_altname: RRIP name component malformed\n"));
	    goto goodbye;
	}	    

	if (!continuing && alt_name->nm_flags & RRIP_NM_CONTINUE) {
	    /* first component, with continuation to come */
	    PN_ALLOCATE(assemble_name);
	    assemble_name[0] = '\0';
	}

	switch(alt_name->nm_flags) {
	case RRIP_NM_CURRENT:
	case RRIP_NM_CURRENT|RRIP_NM_CONTINUE:
	    if (assemble_name) {
		LEN_CHECK(NAME_MAX+1, assemble_len, 1, );
		assemble_name[assemble_len++] = '.';
		name_check = assemble_name;
		length = assemble_len;
	    } else {
		name_check = (unsigned char *)".";
		length = 1;
	    }
	    break;
	case RRIP_NM_PARENT:
	case RRIP_NM_PARENT|RRIP_NM_CONTINUE:
	    if (assemble_name) {
		LEN_CHECK(NAME_MAX+1,assemble_len, 2, );
		assemble_name[assemble_len++] = '.';
		assemble_name[assemble_len++] = '.';
		name_check = assemble_name;
		length = assemble_len;
	    } else {
		name_check = (unsigned char *)"..";
		length = 2;
	    }
	    break;
	case RRIP_NM_HOST:
	case RRIP_NM_HOST|RRIP_NM_CONTINUE:
	    /* find the hostname and insert it.  whee! */
	    if (!assemble_name) {
		PN_ALLOCATE(assemble_name);
		assemble_name[0] = '\0';
	    }
	    HOSTNAME_READ_LOCK();
	    LEN_CHECK(NAME_MAX+1,assemble_len, hostnamelen,
		      HOSTNAME_READ_UNLOCK(););
	    strncpy(&assemble_name[assemble_len], hostname, hostnamelen);
	    assemble_len += hostnamelen;
	    HOSTNAME_READ_UNLOCK();
	    length = assemble_len;
	    name_check = assemble_name;
	    break;
	case RRIP_NM_CONTINUE:
	    LEN_CHECK(NAME_MAX+1,assemble_len, alt_name->hdr.suf_length - 5, );
	    strncpy(&assemble_name[assemble_len], alt_name->nm_component,
		    alt_name->hdr.suf_length - 5);
	    assemble_len += alt_name->hdr.suf_length - 5;
	    /* on to the next one */
	    break;
	case 0:
	    /* normal.  just use it */
	    if (assemble_name) {
		LEN_CHECK(NAME_MAX+1,assemble_len,
			  alt_name->hdr.suf_length - 5, );
		strncpy(&assemble_name[assemble_len], alt_name->nm_component,
			alt_name->hdr.suf_length - 5);
		assemble_len += alt_name->hdr.suf_length - 5;
		length = assemble_len;
		name_check = assemble_name;
	    } else {
		name_check = alt_name->nm_component;
		length = alt_name->hdr.suf_length - 5;
	    }
	    break;
	default:
	    CDDEBUG1(WEIRDDEBUG,
		     printf("cdfs_altname: unknown flags 0x%x\n",
			    alt_name->nm_flags));
	    break;
	}
	if (alt_name->nm_flags & RRIP_NM_CONTINUE) {
	    continuing = 1;
	    continue;
	}
	CDDEBUG1(RRIPDIRDEBUG, printf("found entry '%.*s'\n",
				      length, name_check));
	break;			/* do it once, unless continued */
    } while (1);
    if (assemble_name)
	*dealloc = 1;
    else
	*dealloc = 0;
    *retlength = length;
    return name_check;
 cleanup:
    PN_DEALLOCATE(assemble_name);
    if (*bpp)
	brelse(*bpp);
    *bpp = 0;
    return 0;
}
    
/*
 * rrip_pullup_symlink():	given a directory entry and cdnode,
 *	construct the symbolic link target and return it.
 *	
 * ARGS:
 *	fs:	(in)		CDFS superblock pointer
 *	mntp:	(in)		pointer to filesystem's mount structure
 *				(used for volume-relative symlinks)
 *	isodir:	(in)		pointer to directory entry
 *	cdp:	(in)		ptr to cdnode on which we're operating.
 *
 * RETURNS:	pointer to symlink name, or NULL if some problem occurred
 *		the returned pointer must be released with PN_DEALLOCATE()
 *		when the caller is finished.
 */
unsigned char *
rrip_pullup_symlink(struct fs *fs,
		    struct mount *mntp,
		    struct cdnode *cdp,
		    struct iso_dir *isodir)
{
    struct buf *bp;
    unsigned char *symname;
    int symlen = 0;
    int continuing = 0;
    struct rrip_sl *sl;
    int suspcount;
    unsigned char *susp_bufp;
    int juststarted = 1;
    struct cd_suf_ce cont;

    susp_compute_diroff(isodir, fs->rrip_susp_offset, &suspcount, &susp_bufp);

    /* there may be multiple SLs to concatenate */
    PN_ALLOCATE(symname);
    symname[0] = '\0';
    symlen = 0;

    bp = 0;
    bzero(&cont, sizeof(cont));
    /*
     * look through the recorded SL fields and accumulate the symlink target.
     */
    if (cached(cdp, SL)) 
	sl = (struct rrip_sl *)
	    susp_search_cached(fs, cdp->cd_devvp, &susp_bufp,
			       &suspcount, TRUE, RRIP_SIG_SL, &bp,
			       cdp, &cont);
    else
	sl = (struct rrip_sl *) susp_search(fs, cdp->cd_devvp, &susp_bufp,
					    &suspcount, TRUE, RRIP_SIG_SL,
					    &bp, cdp, &cont);
    while (sl) {
	unsigned char *slc_cp = &sl->sl_components[0];
	struct rrip_sl_component *slc;
	int count = sl->hdr.suf_length - 5;
	/*
	 * For each SL field, combine the constituent components.
	 */

	slc = (struct rrip_sl_component *) slc_cp;
	while (count > 0) {
	    /* SLC_CONTINUE means that this component of the symlink target
	       continues in the next component record. */
	    switch (slc->slc_flags) {
	    case RRIP_SLC_CURRENT:
	    case RRIP_SLC_CURRENT|RRIP_SLC_CONTINUE:
		if (!juststarted && !continuing) {
		    LEN_CHECK(MAXPATHLEN,symlen, 2, );
		    symname[symlen++] = '/';
		} else {
		    LEN_CHECK(MAXPATHLEN,symlen, 1, );
		}
		symname[symlen++] = '.';
		CDDEBUG1(RRIPSLDEBUG, printf("symcomp CURRENT\n"));
		break;
	    case RRIP_SLC_PARENT:
	    case RRIP_SLC_PARENT|RRIP_SLC_CONTINUE:
		if (!juststarted && !continuing) {
		    LEN_CHECK(MAXPATHLEN,symlen, 3, );
		    symname[symlen++] = '/';
		} else {
		    LEN_CHECK(MAXPATHLEN,symlen, 2, );
		}
		symname[symlen++] = '.';
		symname[symlen++] = '.';
		CDDEBUG1(RRIPSLDEBUG, printf("symcomp PARENT\n"));
		break;
	    case RRIP_SLC_ROOT:
		if (!juststarted) {
	    case RRIP_SLC_ROOT|RRIP_SLC_CONTINUE:
		    CDDEBUG1(WEIRDDEBUG || RRIPSLDEBUG,
			     printf("rrip_pullup_symlink: senseless SL_ROOT node %d\n",
				    cdp->cd_number));
		    goto cleanup;
		}
		symname[symlen++] = '/';
		CDDEBUG1(RRIPSLDEBUG, printf("symcomp ROOT\n"));
		break;
	    case RRIP_SLC_VOLROOT:
		if (!juststarted) {
	    case RRIP_SLC_VOLROOT|RRIP_SLC_CONTINUE:
		    CDDEBUG1(WEIRDDEBUG || RRIPSLDEBUG,
			     printf("rrip_pullup_symlink: senseless SL_VOLROOT node %d\n",
			       cdp->cd_number));
		    goto cleanup;
		}
		strncpy(symname, mntp->m_stat.f_mntonname,
			MIN(MNAMELEN, MAXPATHLEN)-1);
		symname[MIN(MAXPATHLEN,MNAMELEN)-1] = '\0';
		symlen = strlen(symname);
		/* if the mntonname doesn't end in "/", then we add it,
		   so that we treat VOLROOT and ROOT similarly below */
		if (symlen < MAXPATHLEN &&
		    symname[symlen-1] != '/') {
		    symname[symlen++] = '/';
		    symname[symlen] = '\0';
		}		    
		CDDEBUG1(RRIPSLDEBUG, printf("symcomp VOLROOT\n"));
		break;
	    case RRIP_SLC_HOST:
	    case RRIP_SLC_HOST|RRIP_SLC_CONTINUE:
		/* XXX We depend here on the caller to hold onto the pathname
		   for a while, since the hostname might change and alter the
		   target.  The alternative is to cache the hostname at
		   mount time, which might be the wrong thing to do if
		   hostname isn't yet set at that time (e.g. mounted from
		   /etc/fstab before the hostname program is called at
		   system startup time */
		/* find the hostname and insert it.  whee! */
		HOSTNAME_READ_LOCK();
		LEN_CHECK(MAXPATHLEN,symlen,
			  symlen ? hostnamelen + 1 : hostnamelen,
			  HOSTNAME_READ_UNLOCK(););
		if (!juststarted && !continuing) {
		    symname[symlen++] = '/';
		}
		strncpy(&symname[symlen], hostname, hostnamelen);
		symlen += hostnamelen;
		HOSTNAME_READ_UNLOCK();
		CDDEBUG1(RRIPSLDEBUG, printf("symcomp HOST\n"));
		break;
	    case 0:
	    case RRIP_SLC_CONTINUE:
		if (!juststarted && !continuing) {
		    LEN_CHECK(MAXPATHLEN,symlen, slc->slc_len + 1, );
		    symname[symlen++] = '/';
		} else
		    LEN_CHECK(MAXPATHLEN,symlen, slc->slc_len, );
		bcopy(&slc->slc_component[0], &symname[symlen],
		      slc->slc_len);
		symlen += slc->slc_len;
		CDDEBUG1(RRIPSLDEBUG, printf("symcomp '%.*s'\n", slc->slc_len,
					     slc->slc_component));
		break;
	    default:
		CDDEBUG1(WEIRDDEBUG || RRIPSLDEBUG,
			 printf("rrip_pullup_symlink: unknown slc flag 0x%x\n",
				slc->slc_flags));
		goto cleanup;
		break;
	    }
	    if (slc->slc_flags != RRIP_SLC_ROOT &&
		slc->slc_flags != RRIP_SLC_VOLROOT)
		juststarted = 0;
	    /* move along to next one */
	    if (slc->slc_flags & RRIP_SLC_CONTINUE) {
	    /* this symlink component continues into the next record;
	       arrange for that if needed. */
		continuing = 1;
	    } else
		/* the component might be bigger than NAME_MAX,
		   but let namei() complain about that. */
		continuing = 0;

	    count -= (slc->slc_len + 2);
	    slc = (struct rrip_sl_component *) ((unsigned char *)slc+slc->slc_len + 2);
	} /* end of while (count > 0) */
	/* If we come here, there are no more components in this SL.
	   If we should not look for more, leave, otherwise continue &
	   fetch next one. */
	if (!(sl->sl_flags & RRIP_SL_CONTINUE))
	    break;
	/* on to the next:  we know it's not the first, so don't pass cdp */
	sl = (struct rrip_sl *) susp_search(fs, cdp->cd_devvp, &susp_bufp,
					    &suspcount, TRUE,
					    RRIP_SIG_SL, &bp, 0, &cont);
    }
    if (continuing) {
	/* got to the last SL, but a component record wasn't finished! */
	CDDEBUG1(WEIRDDEBUG || RRIPSLDEBUG,
		 printf("rrip_pullup_symlink: orphaned component record node %d\n",
			cdp->cd_number));
	/* fall through to cleanup case below: */
    } else {
	if (bp)
	    brelse(bp);
	symname[symlen] = '\0';
	return symname;
    }
 cleanup:
    PN_DEALLOCATE(symname);
    if (bp)
	brelse(bp);
    return(0);
}

/*
 * cdfs_adjust_dirent_name():	given an ISO-9660 filename, apply
 *	current XCDR options to convert the name.
 *	(M_LOWERCASE and/or M_DROPVERS translations)
 *	(also used when M_NOVERSION specified, since that translates
 *	 to M_LOWERCASE|M_DROPVERS)
 * ARGS:
 *	mntp:	(in)		CDFS mount point pointer (to get name
 *				conversion flags)
 *	name:	(in/out)	pointer to the name
 *	lngth:	(in/out)	pointer to name length
 *
 * RETURNS:
 *	modifies name in place to apply the rules.
 *	modifies *lngth to reflect the new length
 */
void
cdfs_adjust_dirent_name(struct cdfsmount *mntp,
			char *name,
			unsigned short *lngth)
{
    int len;
    int length;
    if ((mntp->um_flag & M_DROPVERS) || (mntp->um_flag & M_LOWERCASE)) {
	for (length = 0; length < *lngth; length++) {
	    if (name[length] == ';') {
		/*
		 * XCDR: drop version suffix.
		 */
		if (mntp->um_flag & M_DROPVERS) {
		    len = length;
		}
		/*
		 * XCDR: drop null extension
		 */
		if ((mntp->um_flag & M_LOWERCASE) &&
		    (name[length-1] == '.')) {
		    /* strip off "." */
		    if (mntp->um_flag & M_DROPVERS)
			len--;
		    else {		/* shift left */
			strcpy(&name[length-1], &name[length]);
			(*lngth)--;
		    }
		}
		if (mntp->um_flag & M_DROPVERS) {
		    *lngth = len;
		    name[len] = '\0';
		    break;
		}
	    } else {			/* not ";" */
		/*
		 * XCDR: map upper to lower.
		 */
		if (mntp->um_flag & M_LOWERCASE) {
		    name[length] = cdfs_tolower((int) name[length]);
		}
	    }
	}
    }
    return;
}

/*
 * rrip_parent_num():	given a directory entry for `..', look to see
 *	if it should be altered to reflect a relocated directory entry.
 *
 * ARGS:
 *	fs: 	(in)	CDFS superblock pointer
 *	devvp:	(in)	device from which to read blocks if needed
 *	dirp:	(in)	pointer to directory entry in question
 *	cdp:	(in)	pointer to node in question (for hint caching
 *			by susp_* functions)
 * RETURNS:
 *	inode number of logical parent (either from PL field if present,
 *	or from directory entry)
 */
ino_t
rrip_parent_num(struct fs *fs,
		struct vnode *devvp,
		struct iso_dir *dirp,
		struct cdnode *cdp)
{
    struct rrip_pl *pl;
    struct buf *nbp;
    union {
	unsigned char incoming[4];
	unsigned int  outgoing;
    } iso_convert_int;

    /* Look for "PL": is this a substitute parent link? */
    pl = (struct rrip_pl *) susp_search_dir(fs, devvp, dirp,
					    fs->rrip_susp_offset, TRUE,
					    RRIP_SIG_PL, &nbp, cdp);
    if (pl) {
	CDDEBUG1(RRIPDIRDEBUG, printf("found PL\n"));
	CDFS_COPYINT(pl->pl_parent_lbn_lsb, iso_convert_int.incoming);
    } else {
	CDFS_COPYINT(dirp->dir_extent_lsb, iso_convert_int.incoming);
    }
    if (nbp)
	brelse(nbp);
    return iso_convert_int.outgoing * (int)ISOFS_LBS(fs);
}

/*
 * rrip_skipdir():	given a directory entry, look to see if it's a
 *		relocated child which should not be seen here.
 *
 * ARGS:
 *	fs: 	(in)	CDFS superblock pointer
 *	devvp:	(in)	device from which to read blocks if needed
 *	dirp:	(in)	pointer to directory entry in question
 *	cdp:	(in)	pointer to node in question (for hint caching
 *			by susp_* functions)
 * RETURNS:
 *	1 if the entry should be skipped. 
 *	0 otherwise.
 */
int
rrip_skipdir(struct fs *fs,
	     struct vnode *devvp,
	     struct iso_dir *dirp,
	     struct cdnode *cdp)
{
    struct rrip_re *reptr;
    struct buf *nbp;
    if (reptr = (struct rrip_re *) susp_search_dir(fs, devvp, dirp,
						   fs->rrip_susp_offset,
						   TRUE,
						   RRIP_SIG_RE, &nbp, cdp)) {
	/* relocated directory.  It doesn't live here. */
	CDDEBUG1(RRIPDIRDEBUG, printf("cdfs_readdir: skipping RE dirent\n"));
	if (nbp)
	    brelse(nbp);
	return 1;
    }
    return 0;
}

/*
 * rrip_child_num():	given a directory entry, look to see if
 *	if it is a placeholder for a relocated directory.
 *
 * ARGS:
 *	fs: 	(in)	CDFS superblock pointer
 *	devvp:	(in)	device from which to read blocks if needed
 *	dirp:	(in)	pointer to directory entry in question
 *	cdp:	(in)	pointer to node in question (for hint caching
 *			by susp_* functions)
 * RETURNS:
 *	inode number of logical child, if there is one,
 *	else 0
 */
ino_t
rrip_child_num(struct fs *fs,
	       struct vnode *devvp,
	       struct iso_dir *dirp,
	       struct cdnode *cdp)
{
    struct rrip_cl *clptr;
    struct buf *nbp;
    union {
	unsigned char incoming[4];
	unsigned int  outgoing;
    } iso_convert_int;

    /* look for CL: indicates file is really a directory which has
       been relocated due to ISO-9660 depth restrictions */

    clptr = (struct rrip_cl *) susp_search_dir(fs, devvp, dirp,
					       fs->rrip_susp_offset, TRUE,
					       RRIP_SIG_CL,
					       &nbp, cdp);
    if (clptr) {
	/* found it. */
	CDDEBUG1(RRIPDIRDEBUG, printf("found CL\n"));
	/* XXX is the block the first of the extent? or an xar? */
	CDFS_COPYINT(clptr->cl_child_lbn_lsb,
		     iso_convert_int.incoming);
	if (nbp)
	    brelse(nbp);
	/* guaranteed non-zero by ISO-9660 block allocation rules: */
	return iso_convert_int.outgoing * (int)ISOFS_LBS(fs);
    }
    return 0;
}

/*
 * rrip_getnodedevmap():	given a node and a superblock, return the dev_t
 * 	to which that node is mapped, if any, else return ENODEV.
 *
 * ARGS:
 *	fs:	(in)	superblock pointer
 *	cdp:	(in)	cdnode pointer
 *
 * RETURNS: dev_t mapping, if present, else NODEV
 */

dev_t
rrip_getnodedevmap(struct fs *fs, struct cdnode *cdp)
{
    int i;
    for (i = 0; i < fs->rrip_numdevs; i++)
	if (fs->rrip_devmap[i].ino == cdp->cd_number)
	    return fs->rrip_devmap[i].newdev;
    return NODEV;
}

/*
 * rrip_mapdev():	given a node, a superblock, and a dev_t,
 *	add that node mapping to the filesystem's mapping table.
 *
 * ARGS:
 *	fs:	(in/out)	superblock pointer
 *	cdp:	(in)	cdnode pointer
 *	dev:	(in)	dev_t to which the node should map
 *	path:	(in)	kalloc'ed copy of pathname
 *	pathlen:	(in)	length of kalloc'ed pathname
 *
 * RETURNS: 0 if successful, error code otherwise
 *
 * does NOT set cdp->cd_rdev; that needs to be done by the caller
 * due to vgone()/clearalias() vnode issues.
 */
int
rrip_mapdev(struct fs *fs, struct cdnode *cdp, dev_t dev,
	    char *path, int pathlen)
{
    int i;
    if (!CD_ISBLK(cdp->cd_mode) && !CD_ISCHR(cdp->cd_mode))
	return EINVAL;

    for (i = 0; i < fs->rrip_numdevs; i++)
	if (fs->rrip_devmap[i].ino == cdp->cd_number) {
	    fs->rrip_devmap[i].newdev = dev;
	    kfree(fs->rrip_devmap[i].path, fs->rrip_devmap[i].pathlen);
	    fs->rrip_devmap[i].path = path;
	    fs->rrip_devmap[i].pathlen = pathlen;
	    return 0;
	}
    /* not already mapped. */
    if (fs->rrip_numdevs) {
	/* table exists */
	if (fs->rrip_numdevs == CD_MAXDMAP)
	    return E2BIG;		/* flag to user-level XXX */
	i = fs->rrip_numdevs;
	fs->rrip_numdevs++;
    } else {
	/* allocate a table */
	fs->rrip_devmap = (struct rrip_devmap *)
	    kalloc(sizeof(struct rrip_devmap) * CD_MAXDMAP);
	if (!fs->rrip_devmap)
	    return ENOMEM;
	fs->rrip_numdevs = 1;
	i = 0;
    }
    fs->rrip_devmap[i].ino = cdp->cd_number;
    fs->rrip_devmap[i].newdev = dev;
    fs->rrip_devmap[i].path = path;
    fs->rrip_devmap[i].pathlen = pathlen;
    /* XXX spec_init ? */
    return 0;
}

/*
 * rrip_unmapdev():	given a node, a superblock, and a dev_t,
 *	remove that node mapping from the filesystem's mapping table.
 *
 * ARGS:
 *	fs:	(in/out)	superblock pointer
 *	cdp:	(in)	cdnode pointer
 *
 * RETURNS: 0 if successful, error code otherwise
 */
int
rrip_unmapdev(struct fs *fs, struct cdnode *cdp)
{
    int i;
    if (!CD_ISBLK(cdp->cd_mode) && !CD_ISCHR(cdp->cd_mode))
	return EINVAL;

    if (!fs->rrip_numdevs)
	return ESRCH;			/* XXX? ENOENT? */
    for (i = 0; i < fs->rrip_numdevs; i++)
	if (fs->rrip_devmap[i].ino == cdp->cd_number) {
	    /* free the path */
	    kfree(fs->rrip_devmap[i].path, fs->rrip_devmap[i].pathlen);

	    /* copy the last entry on top of this one, and decrement count */
	    fs->rrip_devmap[i] = fs->rrip_devmap[--(fs->rrip_numdevs)];
	    break;
	}
    if (!fs->rrip_numdevs) {
	kfree(fs->rrip_devmap, sizeof(struct rrip_devmap) * CD_MAXDMAP);
	fs->rrip_devmap = 0;
    }
    return 0;
}

/*
 * rrip_dealloc_devmap():	deallocate device map table and its
 *	chained pointers.
 * ARGS:
 *	fs:	(in)		superblock pointer to fs in question
 */
void
rrip_dealloc_devmap(struct fs *fs)
{
    /* clear device mappings */
    int i;
    for (i = 0; i < fs->rrip_numdevs; i++)
	kfree(fs->rrip_devmap[i].path, fs->rrip_devmap[i].pathlen);
    if (fs->rrip_devmap)
	kfree(fs->rrip_devmap, sizeof(struct rrip_devmap) *  CD_MAXDMAP);
    return;
}

/*
 * rrip_getnthdevmap():	get nth entry in the table, if it exists
 *
 * ARGS:
 *	fs	(in)	superblock pointer
 *	n:	(in)	index into table (1-based index)
 *
 * RETURNS:	NULL if no such entry
 */
struct rrip_devmap *
rrip_getnthdevmap(struct fs *fs,
		  int n)
{
    if (n > fs->rrip_numdevs)
	return 0;			/* no more */
    return &fs->rrip_devmap[n-1];
}

#endif /* _KERNEL */
