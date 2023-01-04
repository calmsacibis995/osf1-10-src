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
static char rcsid[] = "@(#)$RCSfile: tunbind.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:22:34 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** tunbind.c 1.2, last change 11/8/89
 **/

#include <sys/stream.h>
#include <tli/common.h>
#include <tli/tihdr.h>
#include <tli/timod.h>

int
t_unbind (
	int			fd)
{
	union T_primitives	tunbindr;
        int   			code;
	struct tli_st *		tli;

        code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY)))
		goto badfd;
	
	if (tli->tlis_state == T_UNBND) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}
	tunbindr.type = T_UNBIND_REQ;
	code = tli_ioctl(fd, TI_UNBIND, &tunbindr, sizeof(struct T_unbind_req));
	TLI_NEXTSTATE(tli, TLI_UNBIND);
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_unbind (fd, code);
#endif

badfd:
	return code;
}
