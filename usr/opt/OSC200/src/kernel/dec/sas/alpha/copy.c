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
#if !defined(lint) && defined(SECONDARY)
static char *sccsid = "@(#)copy.c	9.2	(ULTRIX)	3/11/91";
#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: /sys/sas/alpha/copy.c
 *
 * 02-feb-91 -- rjl
 *	Absolute smallest code to satisfy space problems
 *
 * 27-Sep-90 -- Tim Burke
 *	Created this file for Alpha primary bootstrap support.
 *	Copied "C" version of bcopy from Al's copy.c file.  This will be
 *	replaced by an optimized assembly version at a later date.
 */

#include <sys/types.h>

bcopy(src, dst, len)
	register caddr_t src;
	register caddr_t dst;
	register int len;
{
#ifdef SECONDARY
	if (len >= 8) {
		/*
		 * If not on a quadword boundary but both src and dst
		 * are the same distance from the quadword boundary
		 * then align the leading bytes.
		 * Note: we don't have to compare len to 0 since we
		 * know that we have at least 8 bytes to move.
		 */
		if ( (((long)src & 0x7) != 0)
				&& (((long)dst & 0x7) == ((long)src & 0x7)) ){
			while ( ((long)src & 0x7) != 0 ) {
				*dst++ = *src++;
				len--;
			}
		} else {
			/*
			 * If not on a quadword boundary but both src and dst
			 * are the same distance from the longword boundary
			 * then align the leading bytes.
			 * Note: we don't have to compare len to 0 since we
			 * know that we have at least 8 bytes to move.
			 */
			if((((long)src & 0x3) != 0)&&(((long)dst & 0x3) 
							== ((long)src & 0x3)))
				while ( ((long)src & 0x3) != 0 ) {
					*dst++ = *src++;
					len--;
				}
		}
		/*
		 * If both src & dst are quadword aligned, then move quadwords
		 */
		if ( (((long)src & 0x7) == 0) && (((long)dst & 0x7) == 0) ) {
			register long *qsrc;
			register long *qdst;
			qsrc = (long *)src;
			qdst = (long *)dst;
			/*
			 * Unroll the loop, try to move 8 quadwords (64 Bytes)
			 */
			while (len >= 64) {
				qdst[0] = qsrc[0];
				qdst[1] = qsrc[1];
				qdst[2] = qsrc[2];
				qdst[3] = qsrc[3];
				qdst[4] = qsrc[4];
				qdst[5] = qsrc[5];
				qdst[6] = qsrc[6];
				qdst[7] = qsrc[7];
				qdst += 8; qsrc += 8;
				len -= 64;
			}
			/*
			 * Now do any remaining quadwords
			 */
			while (len >= 8) {
				*qdst++ = *qsrc++;
				len -= 8;
			}
			src = (char *)qsrc;
			dst = (char *)qdst;
		} else {
			/*
			 * If both src & dst are longword aligned
			 */
			if((((long)src & 0x7) == 0)&&(((long)dst & 0x7) == 0)){
				register int *qsrc;
				register int *qdst;
				qsrc = (int *)src;
				qdst = (int *)dst;
				/*
				 * Unroll the loop, try to move 8 londwords 
				 */
				while (len >= 32) {
					qdst[0] = qsrc[0];
					qdst[1] = qsrc[1];
					qdst[2] = qsrc[2];
					qdst[3] = qsrc[3];
					qdst[4] = qsrc[4];
					qdst[5] = qsrc[5];
					qdst[6] = qsrc[6];
					qdst[7] = qsrc[7];
					qdst += 8; qsrc += 8;
					len -= 32;
				}
				/*
				 * Now do any remaining longwords
				 */
				while (len >= 4) {
					*qdst++ = *qsrc++;
					len -= 4;
				}
				src = (char *)qsrc;
				dst = (char *)qdst;
			}
		}
	} 
#endif /* SECONDARY */
	/*
	 * Non aligned & trailing bytes of quad aligned
	 */
	while (len > 0) {
		*dst++ = *src++;
		len--;
	}
	return (0);
}
