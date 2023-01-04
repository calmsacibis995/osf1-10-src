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
static char *rcsid = "@(#)$RCSfile: scu_defects.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/11/03 21:21:33 $";
#endif
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * File:	scu_defects.c
 * Author:	Robin T. Miller
 * Date:	September 2, 1991
 *
 * Description:
 *	This file contains routines to handle defective media commands.
 * This includes the SCSI Read Defects and Reassign Blocks Commands.
 */

#include <stdio.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <io/common/iotypes.h>
#include <io/cam/rzdisk.h>
#include <io/cam/cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_direct.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"
#include "scu_device.h"

/*
 * External Declarations:
 */
extern int getuid();
extern int DoReadDirect (caddr_t buffer, U32 length, U32 lba);
extern int DoWriteDirect (caddr_t buffer, U32 length, U32 lba);
extern int IS_Mounted (struct scu_device *scu);
extern int OpenPager (char *pager);
extern void Printf();
extern void Puts (char *str);

/************************************************************************
 *									*
 * ShowDefects() - Display Disk Defect Lists.				*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ShowDefects (ce, ke)
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	extern void print_defects();
	struct defect_descriptors dd;
	struct read_defect_params defect_params;
	register struct read_defect_params *rdp = &defect_params;
	int status;

	(void) bzero ((char *)&dd, sizeof(dd));
	(void) bzero ((char *)rdp, sizeof(*rdp));

	/*
	 * This MUST all be rewritten to do the following:
	 *
	 *  1.	Read just the defect list header, then allocate the
	 *	required buffer based on the number of defects.
	 *
	 * The 'rzdisk' functionality provides:
	 *
	 *  1.	sizeof(dd) provides space for a maximum of 256 defects.
	 *  2.	Both primary and grown lists are always returned.
	 */
	rdp->rdp_format = DefectFormat;

	/*
	 * Determine which defect list(s) to request.
	 */
	if ( ISSET_KOPT (ce, K_VENDOR_DEFECTS) ) {
	    rdp->rdp_mdl = 1;
	} else if ( ISSET_KOPT (ce, K_GROWN_DEFECTS) ) {
	    rdp->rdp_gdl = 1;
	} else {
	    rdp->rdp_mdl = rdp->rdp_gdl = 1;
	}

	rdp->rdp_alclen = sizeof(dd);
	rdp->rdp_addr = (u_char *) &dd;

	status = DoIoctl (ke->k_cmd, (caddr_t)rdp, ke->k_msgp);
	if (status == SUCCESS) {
	    print_defects (&dd);
	}
	return (status);
}

/*
 * 'rzdisk' code until rewrite is done.
 */
static void
print_defects (ddp)
register struct defect_descriptors *ddp;
{
	int i, j, defect_length, ndefects;
	int cylinder, bfi, sector, lba;

	(void) OpenPager (NULL);
	Printf ("\nDefect List Header:\n");
	Printf ("  Format 0x%02x", ddp->dd_header.rdd_hdr.format);

	if (ddp->dd_header.rdd_hdr.mdl) {
	    	Printf (" MDL");
	}
	if (ddp->dd_header.rdd_hdr.gdl) {
	    	Printf (" GDL");
	}

	switch (ddp->dd_header.rdd_hdr.format) {
		case 4: Printf (" BYTES\n"); break;
		case 5: Printf (" SECTOR\n"); break;
		case 6: Printf (" VENDOR\n"); break;
		case 7: Printf (" RESERVED\n"); break;
		default:Printf (" BLOCK\n"); break;
	}

	defect_length = (ddp->dd_header.rdd_hdr.defect_len0 & 0x00ff) +
			((ddp->dd_header.rdd_hdr.defect_len1 << 8) & 0xff00);
	if (ddp->dd_header.rdd_hdr.format == BLK_FORMAT) {
		ndefects = defect_length / 4;
	} else {
		ndefects = defect_length / 8;
	}
	Printf ("  Defect list length %d number of defects %d\n",
			defect_length, ndefects);
	i = 0;
	j = 0;

	while(i < defect_length) {
		switch (ddp->dd_header.rdd_hdr.format) {
		case 0:
		case 1:
		case 2:
		case 3:
		    lba = (((int)ddp->BLK[j].lba3 << 24) & 0xff000000) +
			  (((int)ddp->BLK[j].lba2 << 16) & 0xff0000) +
			  (((int)ddp->BLK[j].lba1 << 8) & 0xff00) +
			  (((int)ddp->BLK[j].lba0 << 0) & 0xff);
		    Printf (" Block %8d\n", lba);
		    i += 4;
		    break;

		case 4:
		    cylinder = (((int)ddp->BFI[j].cyl2 << 16) & 0xff0000) +
			       (((int)ddp->BFI[j].cyl1 << 8) & 0xff00) +
			       (((int)ddp->BFI[j].cyl0 << 0) & 0xff);
		    bfi = (((int)ddp->BFI[j].bfi3 << 24) & 0xff000000) +
			  (((int)ddp->BFI[j].bfi2 << 16) & 0xff0000) +
			  (((int)ddp->BFI[j].bfi1 << 8) & 0xff00) +
			  (((int)ddp->BFI[j].bfi0 << 0) & 0xff);
		    Printf (" Cylinder %6d  Head %2d  Byte %6d\n",
				cylinder, ddp->BFI[j].head, bfi);
		    i += 8;
		    break;

		case 5:
		    cylinder = (((int)ddp->PHY[j].cyl2 << 16) & 0xff0000) +
			       (((int)ddp->PHY[j].cyl1 << 8) & 0xff00) +
			       (((int)ddp->PHY[j].cyl0 << 0) & 0xff);
		    sector = (((int)ddp->PHY[j].sector3 << 24) & 0xff000000) +
			     (((int)ddp->PHY[j].sector2 << 16) & 0xff0000) +
			     (((int)ddp->PHY[j].sector1 << 8) & 0xff00) +
			     (((int)ddp->PHY[j].sector0 << 0) & 0xff);
		    Printf (" Cylinder %6d  Head %2d  Sector %4d\n",
				cylinder, ddp->PHY[j].head, sector);
		    i += 8;
		    break;

		case 6:
		case 7:
		    return;
		}
		++j;
	}
}

/************************************************************************
 *									*
 * ReassignBlock() - Reassign A Defective Disk Block.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ReassignBlock (ce)
register struct cmd_entry *ce;
{
	struct scu_device *scu = ScuDevice;
	struct reassign_params reassign_params;
	register struct reassign_params *rp = &reassign_params;
	register U32 lba = LogicalBlock;
	int read_status, status;

	/*
	 * We allow root to do reassign to mounted file systems so
	 * the root file system can be repaired (if necessary).
	 */
	if (getuid()) {
	    if ((status = IS_Mounted (scu)) != FALSE) {
		return (FAILURE);
	    }
	}

	/*
	 * Attempt to read the block being reassigned.
	 */
	read_status = DoReadDirect (scu->scu_data_buffer,
						scu->scu_device_size, lba);
	if (read_status == FAILURE) {
	    Puts ("WARNING, unable to read data from block being reassigned.");
	}

	(void) bzero ((char *)rp, sizeof(*rp));
	/*
	 * Must add read/write of block data being reassigned.
	 */
	rp->rp_lbn3 = ((lba >> 24) & 0xff);
	rp->rp_lbn2 = ((lba >> 16) & 0xff);
	rp->rp_lbn1 = ((lba >> 8) & 0xff);
	rp->rp_lbn0 = ((lba >> 0) & 0xff);
	rp->rp_header.defect_len1 = 0;
	rp->rp_header.defect_len0 = 4;

	status = DoIoctl (ce->c_cmd, (caddr_t)rp, ce->c_msgp);

	/*
	 * If the block being reassigned was read successfully, then
	 * write out the previous data from that block.  Some controllers
	 * rewrite the data on reassign data command but others do not.
	 */
	if ( (status == SUCCESS) && (read_status != FAILURE) ) {
	    status = DoWriteDirect (scu->scu_data_buffer,
						scu->scu_device_size, lba);
	}

	return (status);
}
