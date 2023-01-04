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
/*	
 *	@(#)$RCSfile: pvres.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:21:57 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _PVRES_H_
#define _PVRES_H_

/*
 *	Physical Volume Reserve Area Definitions.
 */

#include <sys/types.h>
#include <lvm/lvm.h>

/*
 * Layout of the LVM record in the physical volume reserved area.  In the
 * following structure all lengths are in DEV_BSIZE units.  All PSNs are in
 * DEV_BSIZE units.  pxsize and pxspace are in bytes.
 */
struct lv_lvmrec {
	char		lvm_id[8];	/* Should be LVMRECXX.		*/
	lv_uniqueID_t	pv_id;		/* The physical volume ID.	*/
	lv_uniqueID_t	vg_id;		/* The volume group ID.		*/
	uint_t		last_psn;	/* Last physical sector number.	*/
	uint_t		pv_num;		/* Physical Volume Number in VG */
	uint_t		vgra_len;	/* Length of the VGRA.		*/
	uint_t		vgra_psn;	/* PSN of the VGRA.		*/
	uint_t		vgda_len;	/* Length of the VGDA.		*/
	uint_t		vgsa_len;	/* Length of the VGSA.		*/
	uint_t		vgda_psn1;	/* PSN of the primary VGDA.	*/
	uint_t		vgda_psn2;	/* PSN of the secondary VGDA.	*/
	uint_t		mcr_len;	/* Length of the mirror		*/
					/*    consistency record (MCR).	*/
	uint_t		mcr_psn1;	/* PSN of the primary MCR.	*/
	uint_t		mcr_psn2;	/* PSN of the secondary MCR.	*/
	uint_t		data_len;	/* Length of the user data.	*/
	uint_t		data_psn;	/* The start of the user data.	*/
	uint_t		pxsize;		/* Size of each physical extent.*/
	uint_t		pxspace;	/* Size of physical space.	*/
	uint_t		altpool_len;	/* Alternate block pool length. */
	uint_t		altpool_psn;	/* Alternate block pool start.  */
};
typedef struct lv_lvmrec lv_lvmrec_t;

/*
 * Physical Volume Reserved Area (PVRA) Definitions.
 */
#define	PVRA_SIZE		128	/* Size of the PVRA in sectors.	*/
#define	PVRA_LVM_REC_SN1	8	/* Sector of LVM record.	*/
#define	PVRA_LVM_REC_SN2	72	/* Sector of backup LVM record.	*/
#define	PVRA_BBDIR_SN1		9	/* Sector of bad block dir.	*/
#define	PVRA_BBDIR_SN2		73	/* Sector of backup bad block dir. */
#define	PVRA_BBDIR_LENGTH	55	/* The number of sectors of the bad */
					/* block directory */

#endif  /* _PVRES_H_ */
