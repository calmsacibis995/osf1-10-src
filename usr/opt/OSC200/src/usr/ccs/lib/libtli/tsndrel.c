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
static char rcsid[] = "@(#)$RCSfile: tsndrel.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:22:20 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** tsndrel.c 1.2, last change 1/29/90
 **/

#include <sys/stream.h>
#include <tli/common.h>
#include <stropts.h>
#include <tli/tihdr.h>

int
t_sndrel (
	int			fd)
{
	struct strbuf		ctlbuf;
	struct T_ordrel_req	tor;
	struct tli_st *		tli;
	int    			code;
	int 			ret;

        code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY)))
		goto badfd;
	
	if (tli->tlis_servtype != T_COTS_ORD) {
		_Set_terrno(TNOTSUPPORT);
		goto rtn;
	}
#ifdef XTI
	if (tli->tlis_state != T_DATAXFER  &&  tli->tlis_state != T_INREL) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}
#endif
	if (TLI_LOOK(fd, tli) == T_DISCONNECT) {
		_Set_terrno(TLOOK);
		goto rtn;
	}
	tor.PRIM_type = T_ORDREL_REQ;
	ctlbuf.buf = (char *)&tor;
	ctlbuf.len = sizeof(tor);
	if ((ret = putmsg(fd, &ctlbuf, nilp(struct strbuf), 0)) == -1) {
		t_unix_to_tli_error();
		if ( _Get_terrno() == TNODATA )
			_Set_terrno(TFLOW);
		goto rtn;
	}
	TLI_NEXTSTATE(tli, TLI_SNDREL);
	code = 0;
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_sndrel (fd, code);
#endif

badfd:
	return code;
}
