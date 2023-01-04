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
 * derived from uqsysap.h	4.1	(ULTRIX)	7/2/90
 */
/*
 *	Modification History
 *
 * 17-Mar-88 -- map
 *	Change unmapped buffer handle to simply be a longword.
 */
#ifndef _UQSYSAP_H_
#define _UQSYSAP_H_

#define	NCON	4		/* Number of possible connections	*/




/* UQ PPD Data Structure Definitions
 */

typedef struct _uqbhandle {			/* UQ buffer descriptor	    */
	union {					/* First word is overlaid   */
		struct {			/* Non-mapped descriptor    */
			u_int buf_add;		/* Buffer address to use    */
			u_int unused;		/* Unused longword MBZ      */
		}un_mapped;
		
		struct {			/* Mapped descriptor	    */
			u_int	map_idx;	/* Buf offset, map reg indx */
			u_int	map_base :30;	/* Base of map reg table    */
			u_int		:2;	/* MBZ		            */
		}mapped;

		struct {
			u_int		:31;	/* Don't care		    */
			u_int	map_flg	:1;	/* Mapping active flag	    */
		}mapflg;
	}bh;
} UQBHANDLE;



typedef struct _uqlpib {	/* UQ Local Port Information Block 	*/
	u_short	uq_state;	/* current state of port 		*/
	u_short	uq_credits[NCON]; /* uq port credits 			*/
	int	uq_type;	/* controller type 			*/
	u_short	uq_flags;	/* flag word				*/
	u_short	sa;		/* SA register contents for error log	*/
} UQLPIB;

#endif
