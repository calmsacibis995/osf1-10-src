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
static char *rcsid = "@(#)$RCSfile: trcvdis.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/09/29 20:51:11 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989  Mentat Inc.
 ** trcvdis.c 1.4, last change 1/29/90
 **/

#include <tli/common.h>
#include <sys/stream.h>
#include <stropts.h>
#include <tli/tihdr.h>
#ifdef XTI
#include <tli/timod.h>
#endif


int
t_rcvdis (
	int			fd,
reg	struct t_discon * 	discon)
{
	struct T_discon_ind *	tdi;
	struct tli_st *		tli;
	struct netbuf *		netbufp;
        int     		code;
	int 			event;

        code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_SYNC)))
		goto badfd;
	
	if (tli->tlis_servtype == T_CLTS) {
		_Set_terrno(TNOTSUPPORT);
		goto rtn;
	}
#ifdef XTI
	if (tli->tlis_state != T_IDLE) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}
#endif
	if ((event = TLI_ILOOK(fd, tli)) != T_DISCONNECT) {
		if (event != -1)
			_Set_terrno(TNODIS);
		goto rtn;
	}


	netbufp = discon ? &discon->udata : nilp(struct netbuf);
	switch (t_get_primitive(fd, tli, T_DISCON_IND, 
	       (int)sizeof(struct T_discon_ind), netbufp)) {
	case -1:
		if (_Get_terrno() == TNODATA  ||  _Get_terrno() == TLOOK)
			_Set_terrno(TNODIS);
		goto rtn;
	case TBUFOVFLW:
		TLI_NEXTSTATE(tli, TLI_RCVDIS1);
		goto rtn;
	case 0:
	default:
		break;
	}

	if ( discon ) {
		tdi = (struct T_discon_ind *)tli->tlis_proto_buf;
		discon->reason = tdi->DISCON_reason;
		discon->sequence = tdi->SEQ_number;
	}

#ifdef XTI
	tli_ioctl(fd, TI_XTI_CLEAR_EVENT, nilp(char), 0);
#endif
	TLI_TSYNC(fd);
	code = 0;
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_rcvdis (fd, discon, code);
#endif

badfd:
	return code;
}

