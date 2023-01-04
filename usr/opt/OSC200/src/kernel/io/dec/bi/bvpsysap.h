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
	@(#)bvpsysap.h	4.1	(ULTRIX)	7/2/90
*/
/*	BVP Constants
 */

#ifndef _BVPSYSAP_H_
#define _BVPSYSAP_H_

#define	BVP_NOLOG	32		/* Number of logout entries	*/
#define	BVP_LEV_14	1		/* IPL 14  for PIV		*/
#define	BVP_LEV_15	2		/* IPL 15  for PIV		*/
#define	BVP_LEV_16	4		/* IPL 16  for PIV		*/
#define	BVP_LEV_17	8		/* IPL 17  for PIV		*/

/*	BVP flag word definitions
 */

#define	BVP_TIM		1		/* Timer on/off			*/


/* BVP Data Structure Definitions.
 */
typedef struct _bvp_ssplpib	{	/* BVP SSP local port information   */
	u_long	bvp_type;		/* Port device type		    */
	u_short	bvp_flags;		/* Flag word			    */
    } BVPSSPLPIB;

#endif
