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
 *	@(#)$RCSfile: vgsa.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:23:28 $
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

#ifndef _VGSA_H_
#define _VGSA_H_

/*
 * This file is derived from the IBM file vgsa.h.
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	Logical Volume Manager Status Area data structures.
 */


/*
 * The volume group status area header.
 */
struct	vgsaheader {
struct	timeval		sa_h_timestamp;	/* The header timestamp.	    */
	ushort_t	sa_maxpxs;	/* The maximum physical extents.    */
	ushort_t	sa_maxpvs;	/* The maximum physical volumes.    */
};

/*
 * The volume group status area trailer.
 */
struct	vgsatrailer {
struct	timeval		sa_t_timestamp;	/* The trailer timestamp.	    */
};

/*
 * Array of pointers to the volume group information.
 */
struct	vgsa {
struct	vgsaheader	*sa_header;	/* Pointer to the VGSA header.	    */
	uint_t		*sa_pvmiss;	/* Pointer to the PV missing array. */
	uchar_t		*sa_pxstale;	/* Pointer to the PX stale array.   */
struct	vgsatrailer	*sa_trailer;	/* Pointer to the VGSA trailer.	    */
};

/*
 * Macros used to set/clear/test the missing and stale bits in the VGSA.
 */
#define	SA_SET_PVMISSING(vg, pvnum)	\
	((*(vg->vg_sa_ptr.sa_pvmiss + (pvnum >> 5))) |= (1 << (pvnum & 0x1f)))
#define	SA_CLR_PVMISSING(vg, pvnum)	\
	((*(vg->vg_sa_ptr.sa_pvmiss + (pvnum >> 5))) &= ~(1 << (pvnum & 0x1f)))
#define	SA_TST_PVMISSING(vg, pvnum)	\
	((*(vg->vg_sa_ptr.sa_pvmiss + (pvnum >> 5))) & (1 << (pvnum & 0x1f)))

#define	SA_SET_PXSTALE(vg, pvnum, xno)					\
	((*(vg->vg_sa_ptr.sa_pxstale +					\
	  ((SA_MAXPXS(vg) >> 3) * pvnum) + (xno >> 3))) |= (1 << (xno & 0x07)))
#define	SA_CLR_PXSTALE(vg, pvnum, xno)					\
	((*(vg->vg_sa_ptr.sa_pxstale +					\
	  ((SA_MAXPXS(vg) >> 3) * pvnum) + (xno >> 3))) &= ~(1 << (xno & 0x07)))
#define	SA_TST_PXSTALE(vg, pvnum, xno)					\
	((*(vg->vg_sa_ptr.sa_pxstale +					\
	  ((SA_MAXPXS(vg) >> 3) * pvnum) + (xno >> 3))) & (1 << (xno & 0x07)))

/*
 * Macros to get the sequence number and physical sector numbers from the VG.
 */
#define	SA_SEQNUM(vg,idx)    ((vg)->pvols[(idx) >> 1]->pv_sa_seqnum[(idx) & 1])
#define	SA_PV_TIMESTAMP(vg,idx)	((vg)->pvols[(idx) >> 1]->pv_vgsats[(idx) & 1])
#define	SA_PSN(vg,idx)		((vg)->pvols[(idx) >> 1]->pv_sa_psn[(idx) & 1])
#define	SA_LSN(vg,idx)		(SA_PSN(vg, idx)			\
				+ EXT2BLK(vg,((idx) >> 1))		\
				- (vg)->pvols[(idx) >> 1]->pv_vgra_psn)

/*
 * Abbreviations.
 */
#define	SA_H_TIMESTAMP(vg)	(vg->vg_sa_ptr.sa_header->sa_h_timestamp)
#define	SA_T_TIMESTAMP(vg)	(vg->vg_sa_ptr.sa_trailer->sa_t_timestamp)
#define	SA_MAXPXS(vg)		(vg->vg_sa_ptr.sa_header->sa_maxpxs)
#define	SA_MAXPVS(vg)		(vg->vg_sa_ptr.sa_header->sa_maxpvs)

/*
 * Configuration command types.
 */
#define CNFG_INPROGRESS -1	/* Operation started			*/
#define	CNFG_NOP	0	/* No operation in progress		*/
#define	CNFG_PVMISSING	1	/* Mark a set of PVs as missing.	*/
#define	CNFG_INSTALLPV	2	/* Mark a set of PVs as installed.	*/
#define	CNFG_FRESHPX	3	/* Mark a set of PXs as fresh.		*/
#define	CNFG_STALEPX	4	/* Mark a set of PXs as stale.		*/
#define	CNFG_SYNCWRITE	5	/* Perform a synchronous update of the VGSA. */

/*
 * Structure used to pass physical extent information to config.
 */
struct	sa_px_info {
	uchar_t 	pxi_pvnum;
	ushort_t	pxi_pxnum;
};
typedef	struct sa_px_info sa_px_info_t;

/*
 * Status area manager definitions.  At some point these should
 * be split out to their own file
 */
#define	SA_STALEPX	'S'	/* Stale physical extent buffer.    */
#define	SA_FRESHPX	'F'	/* Physical extent is now fresh.    */

#endif  /* _VGSA_H_ */
