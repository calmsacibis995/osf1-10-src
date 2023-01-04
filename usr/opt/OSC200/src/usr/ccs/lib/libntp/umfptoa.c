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
static char     *sccsid = "@(#)$RCSfile: umfptoa.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:40:41 $";
#endif
/*
 */

/*
 * umfptoa - Return an asciized representation of an unsigned long fp number
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ntp/ntp_fp.h>

char *
umfptoa(fpi, fpf, ndec)
	u_int fpi;
	u_int fpf;
	int ndec;
{
	extern char *dolfptoa();

	return dolfptoa(fpi, fpf, 0, ndec, 0);
}
