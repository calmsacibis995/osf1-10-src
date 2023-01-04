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
static char	sccsid[] = "@(#)$RCSfile: cdfs_lookup.c,v $ $Revision: 4.3.14.3 $ (DEC) $Date: 1993/07/27 20:23:32 $";
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
/********************************************************************
 *			Modification History
 *
 * 23-May-91 -- prs
 *	Initial Creation
 *
 ********************************************************************/

#include <sys/secdefines.h>

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <cdfs/cdfsnode.h>
#include <cdfs/cdfsmount.h>
#include <cdfs/cdfs.h>
#include <kern/kalloc.h>


/*
 * Convert a component of a pathname into a pointer to a locked cdnode.
 */
cdfs_lookup(vp, ndp)
	struct vnode *vp;
	register struct nameidata *ndp;
{
	register struct vnode *vdp;	/* vnode copy of dp */
	register struct cdnode *dp;	/* the directory we are searching */
	struct cdnode *pdp;		/* saved dp during symlink work */
	struct cdnode *tdp;
	int error;
	ino_t ino;
	int lbs;
	struct fs *fs;

	ndp->ni_dvp = vp;
	ndp->ni_vp = NULL;
	dp = VTOCD(vp);
	ASSERT(vp->v_usecount);
	/*
	 * Check accessiblity of directory.
	 */
	if ((dp->cd_mode&CDFMT) != CDFDIR)
		return (ENOTDIR);
	if (ndp->ni_isdotdot && (vp->v_mountedhere != NULLMOUNT) &&
	    !(ndp->ni_nameiop & NOCROSSMOUNT)) {
		/* no access checking */
	} else {	
	    if (error = cdnodeaccess(dp, CDEXEC, ndp->ni_cred))
		return (error);
	}
loop:
	/*
	 * We now have a segment name to search for, and a directory to search.
	 *
	 * Before tediously performing a linear scan of the directory,
	 * check the name cache to see if the directory/name pair
	 * we are looking for is known already.
	 */
	if (error = cache_lookup(ndp)) {
		int vpid;	/* capability number of vnode */

		if (error == ENOENT)
			return (error);
		if (vp == ndp->ni_rdir && ndp->ni_isdotdot)
			panic("cdfs_lookup: .. through root");
		/*
		 * Get the next vnode in the path.
		 * See comment below starting `Step through' for
		 * an explaination of the locking protocol.
		 */
		pdp = dp;
		dp = VTOCD(ndp->ni_vp);
		vdp = ndp->ni_vp;
#if	UNIX_LOCKS
		vpid = ndp->ni_vpid;
#else
		vpid = vdp->v_id;
#endif
		if (pdp == dp) {
			ASSERT(vdp->v_usecount);
			VREF(vdp);
			error = 0;
		} else {
			VN_LOCK(vdp);
			error = vget_nowait(vdp);
			VN_UNLOCK(vdp);
		}
		if (!error) {
			/*
			 * Check that the capability number did not change
			 * while we were waiting for the lock.
			 * For UNIX_LOCKS, we saved the capability number
			 * within cache_lookup (ndp->ni_vpid) to compare it
			 * post vget();  it's possible that vdp->v_tag!=VT_CDFS
			 * which would make use of dp invalid.
			 */
			if (vpid != vdp->v_id) {
				vrele(vdp);	/* we lost race to the vnode */
			} else {
				IN_LOCK(dp);
				/*
				 * If the inode is being inactivated, we must
				 * wait until the inactivation completes.
				 * Then we must try to find it in the cache
				 * again. The inode may have been reclaimed.
				 */
				if (dp->cd_flag & INACTIVATING) {
					dp->cd_flag |= INACTWAIT;
					assert_wait((vm_offset_t)&dp->cd_flag, FALSE);
					IN_UNLOCK(dp);
					thread_block();
					cdnodeput(dp);
					dp = pdp;	/* reset to parent */
					ndp->ni_vp = NULLVP;
					goto loop;
				}
				IN_UNLOCK(dp);
				return(0);
			}
		}
		ndp->ni_vp = NULL;
	}
	if (error = cdfs_getnumber(vp, ndp, &ino))
		return(error);
	if (ino == 0)
		return(ENOENT);
	if (error = cdnodeget(vp->v_mount, ino, &tdp))
		return (error);
	/*
	 * If a file was recorded in interleave mode, verify the
	 * file unit size and interleave gap size are both multiples
	 * of 2k.
	 */
	lbs = ISOFS_LBS(tdp->cd_fs);
	if (tdp->iso_dir_file_unit_size) {
		if ((tdp->iso_dir_file_unit_size * lbs) % ISO_SECSIZE) {
			printf("cdfs_lookup: cdnode number %d file unit size %d not 2k multiple\n", 
			       ino, (tdp->iso_dir_file_unit_size * lbs));
			cdnodeput(tdp);
			error = EIO;
			return(error);
		}
		if ((tdp->iso_dir_inger_gap_size * lbs) % ISO_SECSIZE) {
			printf("cdfs_lookup: cdnode number %d file gap size %d not 2k multiple\n", 
			       ino, (tdp->iso_dir_inger_gap_size * lbs));
			cdnodeput(tdp);
			error = EIO;
			return(error);
		}
		/*
		 * If a file unit contains an XAR, and it was recorded in 
		 * interleave mode, verify size of XAR is equal to the file 
		 * unit size.
		 */
		if (tdp->iso_dir_xar) {
			if (tdp->iso_dir_xar != 
			    tdp->iso_dir_file_unit_size) {
				printf("cdfs_lookup:  cdnode number %d xar size %d != file unit size\n", 
				       ino, tdp->iso_dir_xar,
				       tdp->iso_dir_file_unit_size);
				cdnodeput(tdp);
				error = EIO;
				return(error);
			}
		}
	} /* file unit size */
	ndp->ni_vp = CDTOV(tdp);

	/*
	 * Insert name into cache.
	 */
	if (ndp->ni_makeentry)
		cache_enter(ndp);
	return (0);
}



/*
 * XXX
 * We only use the first extent of multi-extent-on-one-volume files.
 */

int
cdfs_getnumber(vp, ndp, ino)
	struct vnode *vp;
	struct nameidata *ndp;
	ino_t *ino;
{
    char *nm = ndp->ni_ptr;
    int wasdot;
    struct buf *bp;
    struct fs *fs;
    int foundit;
    int gnumber;
    struct iso_dir *tmp_iso_dir;
    struct hsg_dir *tmp_hsg_dir;
    struct cdfsmount *mntp;
    struct cdnode *dgp = VTOCD(vp);
    struct iso_idir idir;
    int resid;
    int tsize;
    int lbn, bn;
    int datainbuf, offinbuf;
    unsigned int diskaddr;
    unsigned int isodir_offset;
    unsigned int isodir_reclen;
    union {
	unsigned char incoming[4];
	unsigned int  outgoing;
    } iso_convert_int;
    int rablock, rasize;
    int isiso, isrrip = 0;
    int length;
    int skip_file;
    int error = 0;
    int isfile;

    CDDEBUG1(GETNUMDEBUG,
	     printf("cdfs_getnumber: dgp = 0x%x nm = %s\n", dgp, nm));
    fs = dgp->cd_fs;

    if (ndp->ni_namelen == 1 && nm[0] == '.') {
	*ino = dgp->cd_number;
	return(0);
    }

    if (fs->fs_format == ISO_RRIP)
	isrrip = 1, isiso = 0;
    else if (fs->fs_format == ISO_9660)
	isiso = 1;
    else
	isiso = 0;

    resid = dgp->cd_size;
    mntp = VFSTOCDFS(vp->v_mount);

    isodir_offset = *ino = 0;
    diskaddr = ((unsigned int)dgp->iso_dir_extent +
		(unsigned int)dgp->iso_dir_xar) * (unsigned int)ISOFS_LBS(fs);
    tsize = 0;
    foundit = 0;
    while (resid && !error) {
	lbn = isodir_offset / ISOFS_LBS(fs);
	/*
	 * Since directories cannot be interleaved, datainbuf
	 * and tsize will both equal MAXBSIZE, We are only
	 * interested in bn and offinbuf.
	 */
	cdfs_ibmap(dgp, lbn, &bn, &datainbuf, &offinbuf);
	datainbuf = MIN(datainbuf, dgp->cd_size - isodir_offset);
	if (error = bread(dgp->cd_devvp, bn, fs->fs_ibsize, NOCRED, 
			  &bp)) {
	    brelse(bp);
	    return(error);
	}
	tsize = datainbuf;
	wasdot = 0;
	if (isiso || isrrip)
	    tmp_iso_dir = (struct iso_dir *)
		((unsigned long)bp->b_un.b_addr + offinbuf);
	else
	    tmp_hsg_dir = (struct hsg_dir *)
		((unsigned long)bp->b_un.b_addr + offinbuf);
	do {
	    unsigned char *name_check;
	    struct rrip_re *reptr;
	    struct rrip_cl *clptr = 0;
	    struct rrip_cl cl;
	    struct buf *nbp = 0, *nbp2;
	    int dealloc = 0;
	    /* ino 0 is out of band since ISO 9660 disks
	       leave the first 16k of the disk unspecified,
	       so it's not a block address that appears anywhere.
	       thus, we can use it to flag alternate inums for
	       things. */
	    int hold_ino = 0;
	    skip_file = 0;
	    /*
	     * This implementation chooses to ignore the
	     * "Existence" flag, and makes every directory
	     * entry visible (except for those hidden by RRIP)
	     */
	    switch (fs->fs_format) {
	    case ISO_RRIP:
		if (tmp_iso_dir->dir_file_flags&ISO_FLG_DIR) {
		    if (rrip_skipdir(fs, dgp->cd_devvp, tmp_iso_dir, 0)) {
			/* relocated directory.  It doesn't live here. */
			skip_file = 1;
			goto next_rrip;
		    }
		    isfile = 0;
		    clptr = 0;
		    if (tmp_iso_dir->dir_namelen == 1) {
			if (tmp_iso_dir->dir_name[0] == '\0') {
			    wasdot = 1;
			} else if (wasdot) {
			    wasdot = 0;
			    if (ndp->ni_isdotdot) {
				*ino = rrip_parent_num(fs, dgp->cd_devvp,
						       tmp_iso_dir, 0);
				foundit = 1;
				goto found;
			    }
			}
		    }
		} else {
		    /* not a directory...maybe */
		    hold_ino = rrip_child_num(fs, dgp->cd_devvp,
					      tmp_iso_dir, 0);
		    if (hold_ino) {
			isfile = 0;
		    } else 
			isfile = 1;
		}
		name_check = rrip_compose_altname(fs, tmp_iso_dir,
						  &nbp, dgp->cd_devvp,
						  &length, &dealloc);
		if (name_check) {
		    /* just compare components flat out */
		    if (ndp->ni_namelen == length &&
			!strncmp(ndp->ni_ptr, name_check, length)) {
			foundit = 1;
			if (!isfile) {
			    if (hold_ino)
				*ino = hold_ino;
			    else {
				CDFS_COPYINT(tmp_iso_dir->dir_extent_lsb,
					     iso_convert_int.incoming);
				*ino = (iso_convert_int.outgoing +
					tmp_iso_dir->dir_xar) *
					    (int)ISOFS_LBS(fs);
			    }
			} else
			    *ino = diskaddr + isodir_offset;
		    }
		next_rrip:
		    if (nbp)
			brelse(nbp);
		    nbp = 0;
		    if (dealloc)
			PN_DEALLOCATE(name_check);
		    if (foundit) goto found;
		    isodir_reclen = tmp_iso_dir->dir_len;
		    tmp_iso_dir = (struct iso_dir *)
			((unsigned long)tmp_iso_dir + 
			 isodir_reclen);
		    break;
		} else {
		    CDDEBUG1(RRIPDIRDEBUG, printf("no alternate name\n"));
		    /* end alternate name: if not found, fall through
		       to ISO_9660 */
		}
	    case ISO_9660:
		length = tmp_iso_dir->dir_namelen;
		if (length > 1 &&
		    strlen(tmp_iso_dir->dir_name) <
		    length) {
		    length = strlen(tmp_iso_dir->dir_name);
		    CDDEBUG1(GETNUMDEBUG,
			     printf("cdfs_getnumber: name %s length %d changed from %d\n", tmp_iso_dir->dir_name, length, tmp_iso_dir->dir_namelen));
		}
		if (tmp_iso_dir->dir_file_flags&ISO_FLG_DIR) {
		    isfile = 0;
		    if (tmp_iso_dir->dir_namelen == 1) {
			if (tmp_iso_dir->dir_name[0] == '\0') {
			    wasdot = 1;
			} else if (wasdot) {
			    wasdot = 0;
			    if (ndp->ni_isdotdot) {
				foundit = 1;
				CDFS_COPYINT(tmp_iso_dir->dir_extent_lsb,
					     iso_convert_int.incoming);
				*ino = (iso_convert_int.outgoing + tmp_iso_dir->dir_xar) * (int)ISOFS_LBS(fs);
				goto found;
			    }
			}
		    }
						
		} else {
		    if (hold_ino)
			isfile = 0;
		    else
			isfile = 1;
		    /*
		     * If associated file, skip over file.
		     */
		    if (tmp_iso_dir->dir_file_flags & ISO_FLG_ASSOC) {
			skip_file = 1;
			length = 0;
		    }
		}
		if (skip_file == 0 &&
		    cdfs_namematch(mntp, (char *)tmp_iso_dir->dir_name,
				   length, ndp, isfile)) {
		    foundit = 1;
		    if (!isfile) {
			if (hold_ino) {
			    *ino = hold_ino;
			} else {
			    CDFS_COPYINT(tmp_iso_dir->dir_extent_lsb,
					 iso_convert_int.incoming);
			    *ino = (iso_convert_int.outgoing +
				    tmp_iso_dir->dir_xar) *
					(int)ISOFS_LBS(fs);
			}
		    } else
			*ino = diskaddr + isodir_offset;
		    if (foundit)
			goto found;
		}
		isodir_reclen = tmp_iso_dir->dir_len;
		tmp_iso_dir = (struct iso_dir *)
		    ((unsigned long)tmp_iso_dir + 
		     isodir_reclen);
		break;
	    default:			/* HSG */
		length = tmp_hsg_dir->dir_namelen;
		if (length > 1 &&
		    strlen(tmp_hsg_dir->dir_name) <
		    length) {
		    length = strlen(tmp_hsg_dir->dir_name);
		    CDDEBUG1(GETNUMDEBUG,
			     printf("cdfs_getnumber: name %s length %d changed from %d\n", tmp_hsg_dir->dir_name, length, tmp_hsg_dir->dir_namelen));
		}
		if (tmp_hsg_dir->dir_file_flags&ISO_FLG_DIR) {
		    isfile = 0;
		    if (tmp_hsg_dir->dir_namelen == 1) {
			if (tmp_hsg_dir->dir_name[0] == '\0') {
			    wasdot = 1;
			} else if (wasdot) {
			    wasdot = 0;
			    if (ndp->ni_isdotdot) {
				foundit = 1;
				CDFS_COPYINT(tmp_hsg_dir->dir_extent_lsb,
					     iso_convert_int.incoming);
				*ino = (iso_convert_int.outgoing + tmp_hsg_dir->dir_xar)
				    * (int)ISOFS_LBS(fs);
				goto found;
			    }
			}
		    }
						
		} else {
		    isfile = 1;
		    /*
		     * If associated file, or volume seq
		     * number does not match file primary
		     * volume descriptor volume sequence
		     * number and multivolume set, skip
		     * over file.
		     */
		    if ((tmp_hsg_dir->dir_file_flags &
			 ISO_FLG_ASSOC) ||
			(ISOFS_SETSIZE(fs) > 1 &&
			 tmp_hsg_dir->dir_vol_seq_no_lsb !=
			 ISOFS_VOLSEQNUM(fs))) {
			skip_file = 1;
			length = 0;
		    }
		}
		if (skip_file == 0 &&
		    cdfs_namematch(mntp, (char *)tmp_hsg_dir->dir_name,
				   length, ndp, isfile)) {
		    foundit = 1;
		    if (!isfile) {
			CDFS_COPYINT(tmp_hsg_dir->dir_extent_lsb, 
				     iso_convert_int.incoming);
			*ino = (iso_convert_int.outgoing +
				tmp_hsg_dir->dir_xar) *
				    (int)ISOFS_LBS(fs);
		    } else
			*ino = diskaddr + isodir_offset;
		} else if (foundit)
		    goto found;
		isodir_reclen = tmp_hsg_dir->dir_len;
		tmp_hsg_dir = (struct hsg_dir *)
		    ((unsigned long)tmp_hsg_dir + 
		     isodir_reclen);
	    }				/* switch */

	    isodir_offset += isodir_reclen;
	    tsize -= isodir_reclen;
	    switch (fs->fs_format) {
	    case ISO_9660:
	    case ISO_RRIP:
		if ((tsize > 0) && 
		    (tmp_iso_dir->dir_len == 0)) {
		    isodir_reclen = ISO_SECSIZE - (isodir_offset % ISO_SECSIZE);
		    tsize -= isodir_reclen;
		    isodir_offset += isodir_reclen;
		    tmp_iso_dir = (struct iso_dir *) ((unsigned long)tmp_iso_dir +
						      isodir_reclen);
		}
		break;
	    default:
		if ((tsize > 0) && 
		    (tmp_hsg_dir->dir_len == 0)) {
		    isodir_reclen = ISO_SECSIZE - (isodir_offset % ISO_SECSIZE);
		    tsize -= isodir_reclen;
		    isodir_offset += isodir_reclen;
		    tmp_hsg_dir = (struct hsg_dir *) ((unsigned long)tmp_hsg_dir +
						      isodir_reclen);
		}
	    }				/* switch */
	    if (isodir_offset % ISO_SECSIZE == 0)
		resid -= ISO_SECSIZE;
	} while (tsize > 0);
    found:
	if (foundit) {
	    /*
	     * Found the correct directory entry.
	     * ino is already set.
	     */
	    brelse(bp);
	    CDDEBUG1(GETNUMDEBUG,
		     printf("cdfs_getnumber: entry %s number %d\n", nm, *ino));
	    return(0);
	}
	brelse(bp);
    } /* while */
    CDDEBUG1(GETNUMDEBUG,
	     printf("cdfs_getnumber: entry %s length = %d not found\n",
		    nm, ndp->ni_namelen));
    return(ENOENT);
}

char
cdfs_toupper(c)
	int c;
{
	int result;

	if (c >= 'a' && c <= 'z')
		result = c + 'A' - 'a';
	else
		result = c;
	return((char)result);
}

extern char cdfs_tolower(int);

/*
 * cdfs_old_namematch():	check if the directory name and requested name
 *	are the "same".  Apply M_NOVERSION ULTRIX semantics.
 *
 * ARGS:
 *	mntp:		(in)	cdfsmount pointer (to get flags)
 *	dir_name:	(in)	pointer to on-disk name
 *	dir_length:	(in)	length of on-disk name
 *	ndp:		(in)	nameidata for desired name
 *	isfile:		(in)	TRUE if this is a file (some translation rules
 *				are coded differently if the name is a
 *				directory or a file)
 * RETURNS:
 *	1 (true) if the names are equivalent
 *	0 otherwise
 */
static int
cdfs_old_namematch(mntp, dir_name, dir_length, ndp, isfile)
	struct cdfsmount *mntp;
	char *dir_name;
	int dir_length;
	struct nameidata *ndp;
	int isfile;
{
    int i, end;
    char *wanted_entry, *current_entry, *ptr;
    int has_enddot, wanted_entry_allocsz, current_entry_allocsz;
    int ret = 0;

    /* XXX todo: optimize this ? */
    /*
     * Convert names to upper case.
     */
    wanted_entry_allocsz = current_entry_allocsz = 0;
    wanted_entry_allocsz = ndp->ni_namelen + 3;
    wanted_entry = (char *)kalloc(wanted_entry_allocsz);
    if (wanted_entry == (char *)0) {
	printf("cdfs_namematch: kalloc failed\n");
	return (0);
    }

    current_entry_allocsz = dir_length + 1;
    current_entry = (char *)kalloc(current_entry_allocsz);
    if (current_entry == (char *)0) {
	printf("cdfs_namematch: kalloc failed 2\n");
	kfree(wanted_entry, wanted_entry_allocsz);
	return (0);
    }

    for (i = 0, ptr = wanted_entry; i < ndp->ni_namelen; i++, ptr++)
	*ptr = cdfs_toupper(ndp->ni_ptr[i]);

    for (i = 0, ptr = current_entry; i < dir_length; i++, ptr++)
	*ptr = cdfs_toupper(dir_name[i]);
    /*
     * If we match, we are done.
     */
    if (ndp->ni_namelen == dir_length &&
	!strncmp(wanted_entry, current_entry, dir_length)) {
	ret = 1;
	goto out;
    }
    /*
     * We didn't get a straight match. If current_entry
     * is a directory, then fail.
     */
    if (!isfile)
	goto out;
    /*
     * Subtract the version string from dir_length.
     * If current_entry ends with a "." set boolean.
     */
    has_enddot = 0;
    for (ptr = current_entry, i = 0; i < dir_length; i++, ptr++) {
	if (*ptr == ';') {
	    dir_length = i;
	    --ptr;
	    if (*ptr == '.') {
		has_enddot++;
	    }
	}
    }
    /*
     * Try to match without version
     */
    if (ndp->ni_namelen == dir_length &&
	!strncmp(wanted_entry, current_entry, dir_length)) {
	ret = 1;
	goto out;
    }
    /*
     * If current_entry ends with a ".", try to match without
     */
    if (has_enddot) {
	dir_length--;
	if (ndp->ni_namelen == dir_length &&
	    !strncmp(wanted_entry, current_entry, dir_length)) {
	    ret = 1;
	    goto out;
	}
    }
 out:
    kfree(wanted_entry, wanted_entry_allocsz);
    kfree(current_entry, current_entry_allocsz);
    return (ret);
}

/*
 * cdfs_namematch():	check if the directory name and requested name
 *	are the "same".  Apply any translation rules in effect.
 *
 * ARGS:
 *	mntp:		(in)	cdfsmount pointer (to get flags)
 *	dir_name:	(in)	pointer to on-disk name
 *	dir_length:	(in)	length of on-disk name
 *	ndp:		(in)	nameidata for desired name
 *	isfile:		(in)	TRUE if this is a file (some translation rules
 *				are coded differently if the name is a
 *				directory or a file)
 * RETURNS:
 *	1 (true) if the names are equivalent
 *	0 otherwise
 */
int
cdfs_namematch(mntp, dir_name, dir_length, ndp, isfile)
	struct cdfsmount *mntp;
	char *dir_name;
	int dir_length;
	struct nameidata *ndp;
	int isfile;
{
	int i, end;
	char *current_entry, *ptr;
	int has_enddot;
	int ret = 0;

	/* we usually don't come here if fs->fs_format == ISO_RRIP;
	   only in cases where there's no NM field for the file in question. */
	CDDEBUG1(MATCHDEBUG,
		 printf("namematch: '%.*s' vs '%.*s'\n",
			dir_length, dir_name, ndp->ni_namelen, ndp->ni_ptr));

	if (dir_length == 1) {
	    if (dir_name[0] == '\0') {
		current_entry = ".";
		goto check;
	    } else if (dir_name[0] == '\1') {
		current_entry = "..";
		dir_length = 2;
		goto check;
	    }
	}
	/* This code structuring is in preparation for XCDR support */
	/*
	 * If mounted in compatibility mode (weird name matching
	 * rules), apply those rules.
	 */
	if (RELAX_NAMES(mntp->um_flag))
	    return cdfs_old_namematch(mntp, dir_name, dir_length, ndp, isfile);

	current_entry = dir_name;

    check:
	CDDEBUG1(MATCHDEBUG,
		 printf("cdfs_namematch:  now '%.*s'\n", dir_length,
			current_entry));
	/* OK, we've now done any XCDR translation of
	   the on-disk name.  Run the comparison */
	if (ndp->ni_namelen == dir_length &&
	    !strncmp(ndp->ni_ptr, current_entry, dir_length))
	    ret = 1;
    out:
	CDDEBUG1(MATCHDEBUG,
		 printf("cdfs_namematch:  returns(%d)\n", ret));
	return (ret);
}
