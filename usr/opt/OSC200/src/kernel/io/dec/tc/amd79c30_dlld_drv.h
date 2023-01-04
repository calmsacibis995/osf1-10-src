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
 * @(#)$RCSfile: amd79c30_dlld_drv.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/10/06 13:51:27 $
 */

/*
 * OLD HISTORY
 * Revision 1.1.2.2  92/04/19  17:59:38  Ron_Bhanukitsiri
 * 	"BL2 - Bellcore Certification"
 * 	[92/04/19  17:58:56  Ron_Bhanukitsiri]
 * 
 */


#ifndef	_DLLD_DRV_H
#define	_DLLD_DRV_H

/*
 *		ISDN Device Driver Definitions
 */

#if defined(KERNEL) || defined (_KERNEL)
struct DRV_funcs {
        void (*df_wr_dsc) ();
        int (*df_rd_dsc) ();
        void (*df_qenable) ();		/* Enable D-Chan Q		*/
        void (*df_put_dchan_rdq) ();	/* Put message on D-Chan Q	*/
	int (*df_enable) ();		/* Enable the B-Channell	*/
};
#endif /* KERNEL */

#endif	/* _DLLD_DRV_H */


