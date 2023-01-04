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
static char	*sccsid = "@(#)$RCSfile: s5fs_alloc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:51:38 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * History:
 *
 *	August 20, 1991: vsp
 *		OSF/1 Release 1.0.1 bug fixes.
 *		filesystem and inode locking problems fixed.
 *
 * 	ADK:
 *		original OSF/1 Release 1.0
 *
 *
 */


#include <sysv_fs.h>
#include <sys/types.h>
#include <s5fs/s5param.h>
#include <sys/syslog.h>
#include <sys/user.h>
#include <sys/kernel.h> 
#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/buf.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <s5fs/filsys.h>
#include <s5fs/s5fblk.h>
#include <s5fs/s5inode.h>
#include <s5fs/s5dir.h>
#include <s5fs/s5ino.h>
#include <s5fs/s5mount.h>

#include <sys/signal.h>

#include <sys/errno.h>

typedef	struct fblk *FBLKP;
/*
 * alloc will obtain the next available free disk block from the free list
 * of the specified device.
 * The super block has up to NICFREE remembered free blocks;
 * the last of these is read to obtain NICFREE more . . .
 *
 * NOTE: It is the caller that may call getblk() to get an associated 
 *       buffer frame for this new block. 
 *       
 *       No space on dev x/y -- when the free list is exhausted.
 */
s5alloc(ip, bnp)
struct s5inode *ip;
daddr_t *bnp;
{
	daddr_t bno;
	register struct filsys *fp;
	struct buf *bp;
	dev_t dev = ip->i_dev;
	int bsize;
	int error = 0;
	struct s5fsmount *ump;

	*bnp = 0;
	ump = VFSTOS5FS((S5ITOV(ip))->v_mount);
	fp = ip->i_s5fs;
	bsize = FsBSIZE(fp);


	s5FS_FLOCK(ump);
	do {
		if (fp->s_nfree <= 0)

			goto nospace;
 		bno = fp->s_free[--fp->s_nfree];
		if (bno == 0)
			goto nospace;
	} while (s5badblock(fp, bno, dev));
	if (fp->s_nfree <= 0) {
		error = bread(ip->i_devvp, FsLTOP(bsize, bno), bsize, 
			      NOCRED, &bp); 
		if (error == 0) {
			fp->s_nfree = ((FBLKP)(bp->b_un.b_addr))->df_nfree;
			bcopy((caddr_t)((FBLKP)(bp->b_un.b_addr))->df_free,
			    (caddr_t)fp->s_free, sizeof(fp->s_free));
		}
		bp->b_flags =  B_AGE; 
		brelse(bp);
	}
	if (fp->s_nfree <= 0 || fp->s_nfree > NICFREE) {
		s5fserr(fp, "Bad free count");
		goto nospace;
	}
	if (fp->s_tfree) fp->s_tfree--;
	fp->s_fmod = 1;
	s5FS_FUNLOCK(ump);
	*bnp = bno;
	return(0);

nospace:
	fp->s_nfree = 0;
	fp->s_tfree = 0;
/*
	delay(5*HZ);
 */
	s5fserr(fp, "no space");
	s5FS_FUNLOCK(ump);
	return(ENOSPC);
}

/*
 * place the specified disk block back on the free list of the
 * specified device.
 */
s5free(ip, bno)
struct s5inode *ip;
daddr_t bno;
{
	register struct filsys *fp;
	register struct buf *bp;
	dev_t dev = ip->i_dev;
	int bsize;
	struct s5fsmount *ump;


	fp = ip->i_s5fs;  /* get the pointer to the superblock */
	bsize = FsBSIZE(fp);
	if (s5badblock(fp, bno, dev))
		return;
	ump = VFSTOS5FS((S5ITOV(ip))->v_mount);
	s5FS_FLOCK(ump);
	fp->s_fmod = 1;
	if (fp->s_nfree <= 0) {
		fp->s_nfree = 1;
		fp->s_free[0] = 0;
	}
	if (fp->s_nfree >= NICFREE) {
		bp = getblk(ip->i_devvp,  FsLTOP(bsize, bno), bsize);
		((FBLKP)(bp->b_un.b_addr))->df_nfree = fp->s_nfree;
		bcopy((caddr_t)fp->s_free,
			(caddr_t)((FBLKP)(bp->b_un.b_addr))->df_free,
			sizeof(fp->s_free));
		fp->s_nfree = 0;
		bp->b_blkno = FsLTOP(bsize, bno);
		bwrite(bp);
	}
	s5FS_FUNLOCK(ump);
	fp->s_free[fp->s_nfree++] = bno;
	fp->s_tfree++;
	fp->s_fmod = 1;
}

/*
 * Check that a block number is in the range between the I list
 * and the size of the device.
 * This is used mainly to check that a
 * garbage file system has not been mounted.
 *
 * bad block on dev x/y -- not in range
 */
s5badblock(fp, bn, dev)
register struct filsys *fp;
daddr_t bn;
dev_t dev;
{

	if (bn < fp->s_isize || bn >= fp->s_fsize) {
		s5fserr(fp, "bad block");
		return(1);
	}
	return(0);
}
/*
 * Allocate an unused I node on the specified device.
 * Used with file creation.
 * The algorithm keeps up to NICINOD spare I nodes in the
 * super block. When this runs out, a linear search through the
 * I list is instituted to pick up NICINOD more.
 *
 * NOTE: It is the caller that continues the inode initialization work and 
 *	 then does the force write, if necessary,  by calling s5iupdate().
 * 	 The new inode is locked at iget().
 */
s5ialloc(dip, mode, ipp)
register struct s5inode *dip;
int	mode;
register struct s5inode **ipp;	
{
	register struct filsys *fp;
	register i;
	struct buf *bp;
	struct s5inode *ip;
	struct s5dinode *dp;
	s5ino_t ino;
	daddr_t adr;
	int bsize, error = 0;
	struct s5fsmount *ump;

	*ipp = 0;
	fp = dip->i_s5fs; 
	bsize = FsBSIZE(fp);
	ump = VFSTOS5FS((S5ITOV(dip))->v_mount); 

	s5FS_ILOCK(ump);

loop:
	if (fp->s_ninode > 0
	    && (ino = fp->s_inode[--fp->s_ninode])) {
		if (error = s5iget(dip, ino, &ip)) {
		        s5FS_IUNLOCK(ump);
			return(error);
		}
		if (ip->i_mode == 0) {
			/* found inode: update now to avoid races */
			ip->i_mode = mode;
			ip->i_flag |= S5IACC|S5IUPD|S5ICHG|S5ISYN;
			ip->i_size = 0;
			for (i=0; i<NADDR; i++)
				ip->i_addr[i] = 0;
			if (fp->s_tinode) 
				fp->s_tinode--;
			fp->s_fmod = 1;
	                *ipp = ip; 
			/*
			 * Set up a new generation number for this inode.
			 */
			if (++nextgennumber < (u_long)time.tv_sec)
			  nextgennumber = time.tv_sec;
			ip->i_gen = nextgennumber;
			s5FS_IUNLOCK(ump);
			return(0);

		}
		/*
		 * Inode was allocated by someone else after all.
		 * Look some more.
		 */
		if (error = s5iupdat(ip, &time, &time)) {
		        s5FS_IUNLOCK(ump);
			return(error); 
		}
		s5iput(ip);
		goto loop;
	}
	fp->s_ninode = NICINOD;
	ino = FsINOS(bsize, fp->s_inode[0]);
/* Do we need to call FsLTOP for ino here?? XXX */


	for (adr = FsITOD(bsize, ino); adr < fp->s_isize; adr++)  {
		error = bread(dip->i_devvp, FsLTOP(bsize, adr), bsize, 
				NOCRED,&bp);
		if (error) {
			brelse(bp);
			ino += FsINOPB(bsize);
			continue;
		}
		dp = bp->b_un.b_s5dino;
		for(i=0; i<FsINOPB(bsize); i++,ino++,dp++) {
			if (fp->s_ninode <= 0)
				break;
			if (dp->di_mode == 0)
				fp->s_inode[--fp->s_ninode] = ino;
		}
		brelse(bp);
		if (fp->s_ninode <= 0)
			break;
	} /* for loop */
	if (fp->s_ninode > 0) {
		fp->s_inode[fp->s_ninode-1] = 0;
		fp->s_inode[0] = 0;
	}
	if (fp->s_ninode != NICINOD) {
		fp->s_ninode = NICINOD;
		goto loop;
	}
	fp->s_ninode = 0;
	s5fserr(fp, "Out of inodes");
	error = ENOSPC;
	fp->s_tinode = 0;
	s5FS_IUNLOCK(ump);
	return(error);
}

/*
 * Free the specified I node on the specified device.
 * The algorithm stores up to NICINOD I nodes in the super
 * block and throws away any more.
 */
s5ifree(ip, ino)
struct s5inode *ip;
s5ino_t ino;
{
	register struct filsys *fp;
	struct s5fsmount *ump;

	fp = ip->i_s5fs;
	ump = VFSTOS5FS((S5ITOV(ip))->v_mount);
	fp->s_tinode++;
	if (!lock_try_write(&(ump)->um_fsilock))
		return;
	lock_done(&(ump)->um_fsilock);
	fp->s_fmod = 1;
	if (fp->s_ninode >= NICINOD) {
		if (ino < fp->s_inode[0])
			fp->s_inode[0] = ino;
		return;
	}
	fp->s_inode[fp->s_ninode++] = ino;
}



/*
 * update is the internal name of 'sync'. It goes through the disk
 * queues to initiate sandbagged IO; goes through the I nodes to write
 * modified nodes; and it goes through the mount table to initiate modified
 * super blocks.
 */


/* 
 * The update routine is replaced by s5fs_sync in s5fs_vfsop.sc 
 */

/****** Commnet it Out

s5update()
{
	register struct s5inode *ip;
	register struct mount *mp;
	struct filsys *fp;
	static struct s5inode uinode;

	if (uinode.i_flag)
		return;
	uinode.i_flag++;
	uinode.i_mode = IFBLK;
	for(mp = &mount[0]; mp < (struct mount *)v.ve_mount; mp++)
		if (mp->m_flags == MINUSE) {
			fp = mp->m_bufp->b_un.b_filsys;
			if (fp->s_fmod==0 || fp->s_ilock!=0 ||
			   fp->s_flock!=0 || fp->s_ronly!=0)
				continue;
			fp->s_fmod = 0;
			fp->s_time = time;
			uinode.i_rdev = mp->m_dev;
			u.u_error = 0;
			u.u_offset = SUPERBOFF;
			u.u_count = sizeof(struct filsys);
			u.u_base = (caddr_t)fp;
			u.u_segflg = 1;
			u.u_fmode = FWRITE|FSYNC;
			writei(&uinode);
		}
	for(ip = &inode[0]; ip < (struct inode *)v.ve_inode; ip++)
		if ((ip->i_flag&ILOCK)==0 && ip->i_count!=0
		&& (ip->i_flag&(S5IACC|IUPD|S5ICHG))) {
			ip->i_flag |= ILOCK;
			ip->i_count++;
			s5iupdat(ip, &time, &time);
			s5iput(ip);
		}
	bflush(NODEV);
	uinode.i_flag = 0;
}

************/

/*
 * s5fserr prints the name of a file system with an error diagnostic.
 * 
 * The form of the error message is:
 *	fs: error message
 */

s5fserr(fp, cp)
	struct filsys *fp;
	char *cp;
{

	log(LOG_ERR, "%s: %s\n", fp->s_fname, cp);
}
