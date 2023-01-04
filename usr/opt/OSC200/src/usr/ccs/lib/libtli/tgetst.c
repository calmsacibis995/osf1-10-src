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
static char *rcsid = "@(#)$RCSfile: tgetst.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/05/12 16:26:28 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989  Mentat Inc.
 ** tgetst.c 1.2, last change 1/29/90
 **/

#include <tli/common.h>

int
t_getstate (
	int		fd)
{
	struct tli_st	*tli;
	int		state = -1;

	if(!(tli = iostate_sw(fd, IOSTATE_VERIFY))) {
                goto rtn;
        }
	if (tli->tlis_state == TLI_TSTATECHNG || tli->tlis_state == -1) {
		TLI_UNLOCK(tli);
		if (! (TLI_TSYNC(fd)))
               		 goto rtn;
	}
	state = tli->tlis_state;
	TLI_UNLOCK(tli);

rtn:
#ifdef XTIDBG
	tr_getstate (fd, ret);
#endif
	return state;
}
