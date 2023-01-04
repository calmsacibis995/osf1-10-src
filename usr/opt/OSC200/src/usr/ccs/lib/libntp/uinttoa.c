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
static char     *sccsid = "@(#)$RCSfile: uinttoa.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/07/07 16:12:35 $";
#endif
/*
 */

/*
 * uinttoa - return an asciized unsigned integer
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

#include "lib_strbuf.h"

char *
uinttoa(uval)
	u_int uval;
{
	register char *buf;

	LIB_GETBUF(buf);

	(void) sprintf(buf, "%u", uval);
	return buf;
}
