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
static char *rcsid = "@(#)$RCSfile: super.c,v $ $Revision: 1.2.2.6 $ (DEC) $Date: 1992/10/15 14:28:05 $";
#endif
#ifndef lint
static char     *sccsid = "@(#)super.c	9.2  (ULTRIX/OSF)    11/7/91";
#endif 
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from super.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/***********************************************************************
 *
 *		Modification History
 *
 * 25 Aug 87 -- prs
 *	Fixed a bug in ssblk that would update the wrong super block
 *	if it was mounted.
 *
 * 14 Jul 87 -- prs
 *	Fixed ssblk routine to update super blocks of all partitions for
 *	a device that start at block offset zero. Previously routine
 *	would only update the incore super block of an "a" partition.
 *
 * 11 Sep 86 -- koehler
 *	registerized a few things
 *
 ***********************************************************************/


#include <sys/secdefines.h>

#include <sys/mount.h>
#include <ufs/quota.h>
#include <ufs/ufsmount.h>
#include <ufs/fs.h>
/* #include <machine/fudge.xper.h> */
#include <io/dec/scsi/alpha/scsivar.h>
#include <sys/buf.h>
#include <sys/errno.h>
#include <sys/types.h>

/*
 *	This routine is called by any of the driver routines that need
 *	to handle the partition table.
 */

rsblk(strategy, dev, pt )
	register int (*strategy)();
	dev_t dev;
	register struct pt *pt;
{
	register union sblock {
		struct fs fs;
		char pad[MAXBSIZE];
	} *sblk_addr;				/* ptr to the superblock */
	register int unit = GETUNIT(dev);	/* device unit */
	register struct buf *bp;		/* buffer ptr for the I/O */

	/*
	 *	Get a buffer that will hold a supberblock
	 */
	bp = geteblk( SBSIZE );

	/*
	 *	Set up the buf struct to read the superblock from
	 *	the "a" partition which is located at SBLOCK
	 */

	bp->b_flags = B_READ;
	bp->b_bcount = SBSIZE;
	bp->b_dev = makedev( major(dev), MAKEMINOR(unit, 0));
	bp->b_blkno = SBLOCK;

	/*
	 *	Call the driver's strategy routine to read
	 *	in the superblock, if one exists
	 */

	(*strategy)(bp);		

	/*
	 *	Wait of the I/O to complete
	 */
	biowait(bp);

	/*
	 *	If we have an error just return
	 */

	if ( bp->b_flags & B_ERROR ) {
		bp->b_flags |= B_INVAL;
		brelse(bp);
	 	return (-1);
	}
	/*
	 *	If we have a  valid superblock and a valid partition
	 *	table return success
	 */

	sblk_addr = (union sblock *)bp->b_un.b_fs;

#if SEC_FSCHANGE
	if ( sblk_addr->fs.fs_magic == FS_MAGIC ||
	     sblk_addr->fs.fs_magic == FS_SEC_MAGIC)
#else
	if ( sblk_addr->fs.fs_magic == FS_MAGIC )
#endif
	{
		struct pt *scr_pt;

		/*
		 *	Get the pointer to the partitin table
		 */
	    	scr_pt = (struct pt*)&sblk_addr->pad[ SBSIZE - sizeof(struct pt ) ];

	    	if ( scr_pt->pt_magic == PT_MAGIC ) {
			*pt = *scr_pt;
			/*
			 *	Indicate that we have a valid partition tbl
			 */
			pt->pt_valid = PT_VALID;
			bp->b_flags |= B_INVAL;
			brelse( bp );		/* 001 release buffer */
			return(0);
	    	}
	 	else {
			brelse( bp );		/* 001 release buffer */
			bp->b_flags |= B_INVAL;
			return(-1);
		}
	}
	else {
		bp->b_flags |= B_INVAL;
		brelse( bp );		/* 001 release buffer */
		return(-1);
	}
}
/*
 *	The purpose of the routine is to make sure that any  partition
 *	changes will not corrupt the system.
 *
 *	There are cases in which a new partition table could corrupt
 *	the system.  The following are those cases
 *
 *	1. If a partition's starting location in the current partition table
 *	   differ from that of the new partition and there
 *	   is an open file descr. on the current partition an error
 *	   is returned.
 *
 *	2. If a new partition size decreases from that of the current
 *	   partition and there is an open file descr. on the current
 *	   partition an error is returned.
 *
 * NOTE:
 *	This routine should only be called if the device in questioned is
 *	a partitionable device (disk). The reason is that there are
 *	assumptions about the ioctl address being the same for a partitionable
 *	device be it a raw or block access of the same device.
 *
 */
ptcmp ( dev, cptbl, nptbl )
	dev_t dev;			/* major and minor number */
	register struct pt *cptbl, *nptbl;/* ptr to the current and new
					 * partition tables
					 */
{
	register int majno = major(dev);	/* major number */
	dev_t bdev, cdev;		/* char and block devices */
	int posserr[8];			/* possible error */
	register int part;		/* which partition */
	int bmajno, bunitno;		/* block's major and minor number */
	int cmajno, cunitno;		/* char's major and minor number */
	register struct file *fp;	/* pointer to incore file descp*/
	register struct ufsmount *ump;	/* ptr to mounted filesystem */
	int i;				/* temp variable */
	int anyerrors = 0;		/* if set then an error case was found */
	/*
	 *	Before we go through the incore file desc. we must
	 *      check to determine if any possible errors exists.
	 */

	for ( part = 0; part <= 'h' - 'a'; part++) {
		/*
		 *	If starting locations are different then
		 *	a possible error condition exist.
		 */

		if ( cptbl->pt_part[part].pi_blkoff !=
			 nptbl->pt_part[part].pi_blkoff ) {

			posserr[part] = -1;
			anyerrors = -1;
			continue;

		}
		/*
		 *	If current partition size is going to decrease
		 *	a possible error case exists
		 */
		
		if ( cptbl->pt_part[part].pi_nblocks >
			 nptbl->pt_part[part].pi_nblocks ) {

			posserr[part] = -1;
			anyerrors = -1;
			continue;
		}

		
		/*
		 *	Since we can not initialize local aggragates we
		 *	initialize it here.
		 */

		 posserr[part] = 0;
	}

	/*
	 *	If no error cases are found then no additional checking is
	 *	needed.
	 */

	if ( !anyerrors )
		return(0);

	/*
	 *	The major and minor number are not unique because the
	 *	raw and block device number are not the same for the same
	 *	device. We find the the equivalent block number for
	 *	the same device.
	 */
	cdev = dev;
	/*
 	 *	We know that the ioctl address will be the
 	 *	the same for the raw and block device.
 	 */
#if	!MACH
	for ( i = 0; i < nblkdev ; i++ ) {

            	if ( bdevsw[i].d_ioctl == cdevsw[majno].d_ioctl) {
		 	bdev = makedev( i, minor(dev) );
			break;
		}
	}

	/*
	 *	Do a sanity check to make sure that we did not go over
	 *	then end of the block device table
	 */
	if ( i >= nblkdev )
		panic("ptcmp: No matching ioctl address in block device table");
#endif	/* MACH */

	/*
	 *	Use local variables for optimization later
	 */
	cmajno = major(cdev);
	cunitno = GETUNIT(cdev);
	bmajno = major(bdev);
	bunitno = GETUNIT(bdev);

	/*
	 *	Go through the mounted table to see if we have
	 *	and error
	 */
	MOUNTTAB_LOCK();
	for (ump = &mounttab[0]; ump < &mounttab[nmount]; ump++) {
		/*
		 *	Use the major and unit number to determine
		 *	if we are looking at the right device
		 */
		if ( major(ump->um_dev) == bmajno  &&
		     (GETUNIT(ump->um_dev)) == bunitno ) {
			if ( posserr[ GETDEVS( ump->um_dev ) ] ) {
				MOUNTTAB_UNLOCK();
				return(EBUSY);
			}
		}
	}
	MOUNTTAB_UNLOCK();

	/*
	 *	Now go through the kernel open file descriptor table
	 *	looking to see if any of the descriptors are open on
	 *	the device in question.
	 */
#if	!MACH
	for ( fp = file; fp < fileNFILE; fp++) {
		struct gnode *gp;
		int majorno, unitno;


		/*
		 *	Make sure that we have an active gnode
		 */

		if ( fp->f_type == DTYPE_INODE && fp->f_count &&
		     (gp = (struct gnode *)fp->f_data ) ) {
		
			/*
		 	 *	We only need to check block or char file types
		 	 */
			if ( ( gp->g_mode & GFMT ) == GFCHR ||
				( gp->g_mode & GFMT ) == GFBLK ) {
				/*
			 	 *	Check to see if the file descriptor
			 	 *	in question is the right device by
			 	 *	checking the major and unit
			 	 *	number.
			 	 */
				if ( gp->g_mode & GFMT == GFBLK ) {
					majorno = bmajno;
					unitno = bunitno;
				}
				else {
					majorno = cmajno;
					unitno = cunitno;
				}

				/*
				 *	Everything is now ready to check to
				 *	see if we have an error
				 */
				if ( major(gp->g_rdev) == majorno &&
				     (GETUNIT(gp->g_rdev)) == unitno )
					if( posserr[ GETDEVS(gp->g_rdev ) ] )
						return(EBUSY);
				else
					continue;
			}
		}
	}
#endif /* MACH */
	return(0);
}
/*
 *	This routine is used to write out the partition table if the "a"
 *	paritition of the device is mounted.  The reason for this if
 *	the partition tables are changes via the "chpt" on the mounted
 *	"a" paritition the command has no way of changing the incore
 *	superblock of the "a" paritition.  This routine enables us to
 *	modified the superblock.
 */
ssblk ( dev, nptbl )
	dev_t dev;
	struct pt *nptbl;			/* New partition table */
{
	register int i;
	register majno = major(dev);		/* Major cdevsw number */
	register struct pt *pt;			/* ptr to the part tbl */
	int bdev;			/* block device number */
	register struct mount *mp;		/* ptr to the mount tbl */
	register union sblock {
		struct fs fs;
		char pad[MAXBSIZE];
	} *sblk_addr;
	register struct buf *bp;			/* buf ptr */
	int blk;

#if	MACH
	dprintf("ssblk: Write some code and try again\n");
#else	/* MACH */
	/*
	 *	We assume that the dev is of a character device.  We also
	 *	assume that the ioctl for both the raw and block device
	 *	are the same.
	 */
	for ( i = 0; i < nblkdev ; i++ ) {

            	if ( bdevsw[i].d_ioctl == cdevsw[majno].d_ioctl) {
			/*
			 *	Make a bdev with just a unit number
			 */
		 	bdev = makedev( i, MAKEMINOR(GETUNIT(dev),0) );
			break;
		}
	}

	for (mp = &mounttab[0]; mp < &mounttab[nmount]; mp++) {
	
		/*
		 *	See if any partition of the disk is mounted
		 */

		if ( (mp->m_dev & ~0x07) == bdev && mp->m_bufp != NULL ) {

			/*
			 * 	If the mounted partition does not start
			 *	at sector zero then continue.
			 */
			if (nptbl->pt_part[GETDEVS(mp->m_dev)].pi_blkoff != 0)
				continue;

			/*
			 *	We now know that we have a superblock
			 *	that must possible need updating. If
			 *	updating is required then get the superblock
			 *	and copy the user specified data
			 */
			bp = mp->m_bufp;

			sblk_addr = (union sblock *)bp->b_un.b_fs;
			pt = ( struct pt *)&sblk_addr->pad[ SBSIZE - sizeof(struct pt ) ];

			/*
			 *	If the new partition table will overwrite
			 *	the rotational table then remove it
			 *		Note
			 *	I assume that the user will be warned
			 *	of this case by a user level command.
			 */
			blk = sblk_addr->fs.fs_spc * sblk_addr->fs.fs_cpc /
				 NSPF(bp->b_un.b_fs);
			for (i = 0; i < blk; i += sblk_addr->fs.fs_frag)
				/* void */;
			if ((struct pt *)(&sblk_addr->fs.fs_rotbl[
			(i - sblk_addr->fs.fs_frag) /sblk_addr->fs.fs_frag])
				 >= pt) {
				sblk_addr->fs.fs_cpc = 0;
				mp->m_flags |= M_MOD;
			}

			/*
			 *	If the superblock size is less than the
			 *	max. superblock size we will not overwrite
			 *	what was put there, so just break out
			 */

			if( sblk_addr->fs.fs_sbsize < SBSIZE )
				break;
			/*
			 *	Copy the user specified table into the
			 *	superblock and indicate that we have
			 *	updated the superblock
			 */
			*pt = *nptbl;
			mp->m_flags |= M_MOD;
		}
	}
#endif	/* MACH */
}
