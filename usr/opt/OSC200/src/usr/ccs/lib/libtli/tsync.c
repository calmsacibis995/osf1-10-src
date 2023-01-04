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
static char rcsid[] = "@(#)$RCSfile: tsync.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:22:30 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** tsync.c 1.1, last change 8/9/89
 **/

#include <tli/common.h>

int
t_sync (
	int		fd)
{
	struct t_info	tinfo;
	struct tli_st *	tli;
        int     	code;

	code = -1;
	if(!(tli = iostate_sw(fd, IOSTATE_SYNC)))
		goto rtn;

	code = tli->tlis_state;
	TLI_UNLOCK(tli);
rtn:
#ifdef XTIDBG
	tr_sync (fd, code);
#endif
	return code;
}
