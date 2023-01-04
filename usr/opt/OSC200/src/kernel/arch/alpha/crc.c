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
static char *rcsid = "@(#)$RCSfile: crc.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1992/08/31 12:14:08 $";
#endif
#include <sys/types.h>

crc(ctp, inicrc, len, dp)
	register char *ctp;
	register u_int inicrc;
	register u_int len;
	register char *dp;
{
	register u_int index;

	while (len > 0) {
	    inicrc = (((char)inicrc ^ *dp++) & 0x0ff) | (inicrc & 0xffffff00);
	    index = 0x0f & inicrc;
	    inicrc = inicrc >> 4;
	    inicrc ^= *((u_int *)ctp + index);
	    index = 0x0f & inicrc;
	    inicrc = inicrc >> 4;
	    inicrc ^= *((u_int *)ctp + index);
	    --len;
	}
	return(inicrc);
}
