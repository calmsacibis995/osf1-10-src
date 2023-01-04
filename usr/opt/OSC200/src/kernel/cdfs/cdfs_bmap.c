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
static char	sccsid[] = "@(#)$RCSfile: cdfs_bmap.c,v $ $Revision: 4.3.8.2 $ (DEC) $Date: 1993/07/16 13:04:40 $";
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
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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
/***********************************************************************
 *			Modification History
 *
 * 23-May-91 -- prs
 *	Initial Creation
 *
 ***********************************************************************/
#if	MACH
#include <mach_nbc.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <cdfs/cdfsnode.h>
#include <cdfs/cdfs.h>


cdfs_ibmap(cdp, lbn, bn, datainbuf, offsetinbuf)
register struct cdnode *cdp;
daddr_t lbn;
int *bn;
int *datainbuf;
int *offsetinbuf;
{
	/*
	 * PRS - Do I have to lock fs here ??
	 */

	*datainbuf = *offsetinbuf = *bn = 0;;
	return(cdfs_setuptransfer(cdp, lbn, bn, datainbuf, offsetinbuf, 
				 (cdp->cd_fs)->fs_ibsize));
}

cdfs_setuptransfer(cdp, lbn, bn, datainbuf, offsetinbuf, bsize)
	register struct cdnode *cdp;
	int lbn;		/* Logical block number uio_offset refers to */
	int *bn;
	int *datainbuf;		/* Number of data bytes in buffer */
	int *offsetinbuf;	/* Offset within buffer data resides */
	int bsize;		/* block size */
{
	struct fs *fs;
	int offset_lbn;
	int offset;
	int tmp;
	int file_unit_size = 0;
	int lbs;
	int lbs_bn;

	/*
	 * PRS - Do I have to lock fs here ?
	 */
	fs = (struct fs *)cdp->cd_fs;
	lbs = ISOFS_LBS(fs);

	if (cdp->iso_dir_file_unit_size)
		file_unit_size = cdp->iso_dir_file_unit_size * lbs;
	/*
	 * Set offset to byte offset within file, lbn refers to.
	 */
	offset = lbn * lbs;
	/*
	 * If an XAR exists, skip over it by incrementing offset. Note,
	 * iso_dir_xar is the number of logical blocks XAR is recorded
	 * over.
	 */
	if (cdp->iso_dir_xar)
		offset += (cdp->iso_dir_xar * lbs);
	/*
	 * For initialization purposes.
	 */
	*datainbuf = bsize;
	/*
	 * Calculate logical block number offset resides at.
	 */
	lbs_bn = offset / lbs;
	/*
	 * If recorded in interleave mode, calculate logical block number
	 * offseted by file unit and gap size.
	 */
	if (file_unit_size) {
		/*
		 * tmp is set to which file unit offset exists in.
		 */
		tmp = offset / file_unit_size;
		/*
		 * offset_lbn is set to the logical block number offset
		 * resides at, relative to beginning of file.
		 */
		offset_lbn = 
			(lbs_bn % 
			 (int)cdp->iso_dir_file_unit_size)
			+ (tmp * ((int)cdp->iso_dir_file_unit_size +
				  (int)cdp->iso_dir_inger_gap_size));
		/*
		 * file_unit_size has to be in 2K increments. We can calculate
		 * datainbuf by subtracting the modula of lbs_bn and 
		 * iso_dir_file_unit_size, taking note that both values are
		 * in logical block size units. This value can be coverted
		 * into bytes and subtracted from the size of a file unit,
		 * resulting in the maximum number of bytes remaining in the
		 * file unit.
		 */
		*datainbuf = file_unit_size - 
			((lbs_bn % (int)cdp->iso_dir_file_unit_size)
			 * lbs);
		/*
		 * Since datainbuf is the maximum number of bytes remaining in
		 * the file unit, set value to be amount which can fit in a
		 * buffer.
		 */
		*datainbuf = MIN(*datainbuf, bsize);
	} else
		/*
		 * If the file was not recorded in interleave mode,
		 * offset_lbn is set to the value of lbs_bn.
		 */
		offset_lbn = (int)lbs_bn;

	/*
	 * offset_lbn and iso_dir_extent are in logical block size
	 * units; increment offset_lbn by iso_dir_extent so value
	 * is relative to beginning of volume.
	 */
	offset_lbn += cdp->iso_dir_extent;
	/*
	 * offset_lbn is the logical block number where transfer should
	 * start.
	 *
	 * Now calculate disk block number to begin transfer. Buffers will
	 * begin on bsize increments, and obviously be bsize in length.
	 * We may have to recalculate offsetinbuf and datainbuf to properly 
	 * offset uiomove.
	 */
	*bn = (offset_lbn * lbs) / DEV_BSIZE;
	if ((*bn * DEV_BSIZE) % bsize) {
		unsigned int data_begin;
		unsigned int data_end;

		data_begin = *bn * DEV_BSIZE;
		data_end = data_begin + *datainbuf;

		*offsetinbuf += ((*bn * DEV_BSIZE) % bsize);
		*bn = ((*bn * DEV_BSIZE) - ((*bn * DEV_BSIZE) % bsize)) 
			/ DEV_BSIZE;

		if ((*bn * DEV_BSIZE) + bsize < data_begin) {
			printf("cdfs_setuptransfer: buffer remap messup\n");
			*bn = 0;
			return(0);
		}
		if ((*bn * DEV_BSIZE) + bsize < data_end)
			*datainbuf -= (data_end -
				       ((*bn * DEV_BSIZE) + bsize));
	} else
		*offsetinbuf = 0;

	CDDEBUG1(ISODEBUG,
		printf("cdfs_setuptransfer: cdp 0x%x lbn %d bn %d bsize %d offsetinbuf %d datainbuf %d\n",
		       cdp, lbn, *bn, bsize, *offsetinbuf, *datainbuf));
	return(0);
}
