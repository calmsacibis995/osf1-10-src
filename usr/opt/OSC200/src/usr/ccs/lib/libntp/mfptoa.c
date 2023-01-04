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
static char     *sccsid = "@(#)$RCSfile: mfptoa.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:39:06 $";
#endif
/*
 */

/*
 * mfptoa - Return an asciized representation of a signed long fp number
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ntp/ntp_fp.h>

char *
mfptoa(fpi, fpf, ndec)
	u_int fpi;
	u_int fpf;
	int ndec;
{
	int isneg;
	extern char *dolfptoa();

	if (M_ISNEG(fpi, fpf)) {
		isneg = 1;
		M_NEG(fpi, fpf);
	} else
		isneg = 0;

	return dolfptoa(fpi, fpf, isneg, ndec, 0);
}
