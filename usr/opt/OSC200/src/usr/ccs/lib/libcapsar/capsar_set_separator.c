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
/*LINTLIBRARY*/

#ifndef lint
static char *sccsid = "@(#)$RCSfile: capsar_set_separator.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/03 15:26:47 $";
#endif lint

/*
 * capsar_set_separator.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : capsar_set_separator ( MM *m , char *sep)
 *
 * Set the line separator for the message m to sep.
 * The line separator is used to terminate lines when unparsing
 * a message. The default is LF, for SMTP, the separator should be CRLF.
 * The separator is inherited by child messages when set.
 */

#include <stdio.h>
#include <ctype.h>
#include <capsar.h>

capsar_set_separator(m,sep)
MM	*m;	/* pointer to message structure */
char	*sep;
{
/*------*\
  Locals
\*------*/

	MM	*m0;

	for(m0=m;m0;m0=m0->mm_next){
		if(m->separator)free(m->separator);
		m0->separator = getcpy(sep);
	}
}
