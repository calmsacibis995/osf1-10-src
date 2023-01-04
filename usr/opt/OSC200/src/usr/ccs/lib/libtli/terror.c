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
static char *rcsid = "@(#)$RCSfile: terror.c,v $ $Revision: 4.2.6.4 $ (DEC) $Date: 1993/09/23 18:30:37 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989  Mentat Inc.
 ** terror.c 1.3, last change 12/20/89
 **/

#include <tli/common.h>
#include <stdio.h>
#include <nl_types.h>
#include "libtli_msg.h"


void
t_error (msg)
char	* msg;
{
	int err;
	char *c;
	nl_catd catd;

	catd = catopen(MF_LIBTLI, NL_CAT_LOCALE);
#ifndef	XTI
	if (!msg)
		msg = catgets(catd, MS_LIBTLI, TERROR_MSG, "t_error");
#endif

	if ((err = _Get_terrno()) > 0  &&  err < t_nerr) {
#ifdef	XPG4
		if (err >= TINDOUT)	/* Skip 3 extra catalog entries */
			c = catgets(catd, MS_LIBTLI, err+3, t_errlist[err]);
		else
#endif
		c = catgets(catd, MS_LIBTLI, err, t_errlist[err]);
		if (err == TSYSERR) {
			if( msg == NULL || *msg == 0)
				fprintf(stderr, "%s, %s\n", c, 
				    strerror(_Geterrno()));
			else
				fprintf(stderr, "%s: %s, %s\n", msg, c, 
				    strerror(_Geterrno()));
		} else if( msg == NULL || *msg == 0)
			fprintf(stderr, "%s\n", c);
		else
			fprintf(stderr, "%s: %s\n", msg, c);
	} else
		fprintf(stderr, catgets(catd, MS_LIBTLI, TERROR_ERR, 
		    "%s: Error %d occured\n"), msg, err);
	catclose(catd);
	return;
}
