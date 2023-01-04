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
static char	*sccsid = "@(#)$RCSfile: s5fs_bmap.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:51:41 $";
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
 *
 * 	ADK:
 *		original OSF/1 Release 1.0
 *
 *
 */

#include <sysv_fs.h> 
#include <sys/types.h> 
#include <s5fs/s5param.h>
#include <s5fs/filsys.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/buf.h>
#include <sys/vnode.h>
#include <sys/uio.h>
#include <s5fs/s5inode.h>
#include <s5fs/s5dir.h>


/*
 * s5bmap defines the structure of file system storage based on
 * returning the physical block number on a device given the
 * inode and the logical block number in a file.
 *
 */
s5bmap(ip, lbn, bnp, flags, readflg)
	register struct s5inode *ip;
	register daddr_t lbn;
	daddr_t  *bnp;
	int  flags, readflg;
{
	struct buf *bp;
	register j, sh;
	struct vnode *vp = S5ITOV(ip);
	daddr_t llbn;
	daddr_t nb, *bap;
	int i, bsize;
	int error = 0;


	bsize = FsBSIZE(ip->i_s5fs);

	/*
	 * blocks 0..NADDR-4 are direct blocks
	 */
	if (lbn < NADDR-3) {
		i = lbn;
		nb = ip->i_addr[i];
		if (nb == 0) {
			if (readflg ) {
				*bnp = (daddr_t)-1;
				return(0);
			}	
			if (error = s5alloc(ip, &nb)) {
				*bnp = (daddr_t)-1;
				return(error);
			}
			bp = getblk(vp, lbn, bsize);
			clrbuf(bp);
			bp->b_blkno = FsLTOP(bsize, nb);

			bdwrite(bp, vp);
			ip->i_addr[i] = nb;
			ip->i_flag |= S5IUPD|S5ICHG;
		}
		*bnp = FsLTOP(bsize, nb);
		return(0);
	}
	/*
	 * addresses NADDR-3, NADDR-2, and NADDR-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	llbn = lbn;
	sh = 0;
	nb = 1;
	lbn -= NADDR-3;
	for(j=3; j>0; j--) {
		sh += FsNSHIFT(bsize);
		nb <<= FsNSHIFT(bsize);
		if (lbn < nb)
			break;
		lbn -= nb;
	}
	if (j == 0) {
		*bnp = (daddr_t)-1;
		return(EFBIG);

	}

	/*
	 * fetch the address from the inode
	 * NOTE: an indirect block is owned by a device not by a file. 
	 */
	nb = ip->i_addr[NADDR-j];
	if (nb == 0) {
		if (readflg) { 
			*bnp = (daddr_t)-1;
			return(0);
		}	
		if (error = s5alloc(ip, &nb)) {
			*bnp = (daddr_t)-1;
			return(error);
		}
		bp = getblk (ip->i_devvp, FsLTOP(bsize, nb), bsize);
		clrbuf(bp);
		bp->b_blkno = FsLTOP(bsize, nb);
		bwrite(bp);  		/* force write the indirect block */
		ip->i_addr[NADDR-j] = nb;
		ip->i_flag |= S5IUPD|S5ICHG;
	}
	/*
	 * fetch through the indirect blocks
	 */
	for(; j<=3; j++) {
		error = bread(ip->i_devvp, FsLTOP(bsize, nb), bsize, 
                              NOCRED, &bp);
		if (error) {
			brelse(bp);
			return(error);
		}
		bap = bp->b_un.b_daddr;
		sh -= FsNSHIFT(bsize);
		i = (lbn>>sh) & FsNMASK(bsize);
		nb = bap[i];
		if (nb == 0) {
			register struct buf *nbp;

			if (readflg) { 
				brelse(bp);
				*bnp = (daddr_t)-1;
				return(0);
			}
			if (error = s5alloc(ip, &nb) ) {
				brelse(bp);
				*bnp = (daddr_t)-1;
				return(error);
			}
			if (j < 3 ) {
			  /* Get a buffer frame for the indirect block. */
 			        nbp = getblk(ip->i_devvp, FsLTOP(bsize, nb),
					     bsize);
			        nbp->b_blkno = FsLTOP(bsize, nb); 
				clrbuf(nbp);
				bwrite(nbp);
			      }
			else  {
			  /* Get a buffer frame for the regular block. */
 			        nbp = getblk(vp, llbn, bsize);
			        nbp->b_blkno = FsLTOP(bsize, nb); 
			        if (flags & B_CLRBUF) 
				         clrbuf(nbp);
				bdwrite(nbp, vp);
			      }
			bap[i] = nb;
			if (flags&B_SYNC)
				bwrite(bp);	
			else
				bdwrite(bp, vp);
		} else
			brelse(bp);
	}
	*bnp = FsLTOP(bsize, nb);
	return(0);
}

