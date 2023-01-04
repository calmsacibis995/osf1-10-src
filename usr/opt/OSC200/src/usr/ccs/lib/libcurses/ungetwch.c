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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile$ $Revision$ (DEC) $Date$";
#endif
/*
 * HISTORY
 */

# include       "cursesext.h"

/* Include file for wchar_t */
#include	<locale.h>
#include	<wchar.h>

#define	BUF_LEN	16
/*
 *	This routine puts back a wchar_t character onto the input queue.
 *
 */

int	ungetwch( int wch )
{
	int i, j, mb_len;
	unsigned char mbuf[8];

	for (i=0; i<BUF_LEN; i++) {
		if (SP->input_queue[i] < 0) {
			mb_len = wctomb(mbuf, wch);
			if (mb_len < 1 || (i+mb_len > BUF_LEN))
				return(ERR);
			for (j = 0; j < mb_len; j++, i++)
				SP->input_queue[i] = mbuf[j];
			return(OK);
		}
	}
	return(ERR);
}
