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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: tclose.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:20:06 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** tclose.c 1.2, last change 1/29/90
 **/

#include <tli/common.h>
#include <sys/stream.h>

int
t_close (
	int		fd)
{
	int		retval;

	if (!iostate_sw(fd, IOSTATE_FREE))
		return  -1;
	retval = stream_close(fd);
	if (retval == -1)
		t_unix_to_tli_error();
rtn:

#ifdef XTIDBG
        tr_close (fd, retval);
#endif
	return retval;
}
