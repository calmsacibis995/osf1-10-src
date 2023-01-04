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
static char rcsid[] = "@(#)$RCSfile: trcvcon.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:21:35 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** trcvcon.c 1.3, last change 10/16/89
 **/

#include <sys/stream.h>
#include <tli/common.h>
#include <stropts.h>
#include <tli/tihdr.h>

int
t_rcvconnect (
	int		fd,
reg	struct t_call * call)
{
	struct tli_st *	tli;
        int             code;

        code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY))) {
		goto badfd;
	}	
	if (tli->tlis_servtype == T_CLTS) {
		_Set_terrno(TNOTSUPPORT);
	        goto rtn;
	}
#ifdef XTI
	if (tli->tlis_state != T_OUTCON) {
		_Set_terrno(TOUTSTATE);
	        goto rtn;
	}
#endif
	switch (t_ircvconnect(fd, tli, call)) {
	case 0:
		code = 0;
		/* fallthru */
	case TBUFOVFLW:
		TLI_NEXTSTATE(tli, TLI_RCVCONN);
		break;
	case -1:
	default:
		break;
	}
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_rcvcon (fd, call, code);
#endif

badfd:
	return code;
}
