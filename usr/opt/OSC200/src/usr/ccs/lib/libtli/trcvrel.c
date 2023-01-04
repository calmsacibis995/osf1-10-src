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
static char rcsid[] = "@(#)$RCSfile: trcvrel.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:21:46 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** trcvrel.c 1.3, last change 1/29/90
 **/

#include <sys/stream.h>
#include <tli/common.h>
#include <stropts.h>
#include <tli/tihdr.h>
#ifdef XTI
#include <tli/timod.h>
#endif

int
t_rcvrel (
	int			fd)
{
	struct T_ordrel_ind	toi;
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	int			event;
	int			iflags;
	int			ret;
	struct tli_st *		tli;
        int     		code;

        code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY)))
		goto badfd;
	
	if (tli->tlis_servtype != T_COTS_ORD) {
		_Set_terrno(TNOTSUPPORT);
		goto rtn;
	}
#ifdef XTI
	if (tli->tlis_state != T_OUTREL &&  tli->tlis_state != T_INREL) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}
#endif
	event = TLI_ILOOK(fd, tli);
	if (event != T_ORDREL) {
		switch (event) {
		case 0:
			_Set_terrno(TNOREL);
			break;
		case -1:
			break;
		default:
			_Set_terrno(TLOOK);
			break;
		}
		goto rtn;
	}

	switch (t_get_primitive(fd, tli, T_ORDREL_IND, 
	       (int)sizeof(struct T_ordrel_ind), nilp(struct netbuf))) {
	case -1:
		goto rtn;
	case TBUFOVFLW:
		TLI_NEXTSTATE(tli, TLI_RCVREL);
		goto rtn;
	case 0:
	default:
		break;
	}

#ifdef XTI
	tli_ioctl(fd, TI_XTI_CLEAR_EVENT, nilp(char), 0);
#endif

	TLI_NEXTSTATE(tli, TLI_RCVREL);
	code = 0;
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_rcvrel (fd, code);
#endif

badfd:
	return code;
}
