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
static char *rcsid = "@(#)$RCSfile: tgetaddr.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 16:26:21 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
#include <tli/common.h>
#include <sys/stream.h>
#include <tli/tihdr.h>
#include <tli/timod.h>

int
t_getprotaddr (
	int			fd, 
	struct t_bind *		boundaddr,
	struct t_bind *		peeraddr)
{
	char *			buf;
	char			stack_buf[TLI_STACK_BUF_SIZE];
	struct T_getaddr_req *	taddr_req;
	struct T_getaddr_ack *	taddr_ack;
	struct	tli_st *	tli;
	int			total_len;
	int			code;

	code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY))) {
		_Set_terrno(TBADF);
		goto badfd;
	}
	if (tli->tlis_state == T_UNINIT) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}
	if (tli->tlis_state == T_UNBND) {
		boundaddr->addr.len = 0;
		peeraddr->addr.len = 0;
		goto rtn;
	}
	
	total_len = sizeof(struct T_getaddr_ack) + boundaddr->addr.maxlen + 
	     peeraddr->addr.maxlen;
	if (total_len > sizeof(stack_buf)) {
		if (!(buf = (char *)malloc(total_len))) {
			_Set_terrno(TSYSERR);
			_Seterrno(ENOMEM);
			goto rtn;
		}
	} else
		buf = stack_buf;
		
	taddr_req = (struct T_getaddr_req *)&buf[0];
	taddr_req->PRIM_type = T_GETADDR_REQ;

	if (boundaddr)
		taddr_req->BNDADDR_maxlen = boundaddr->addr.maxlen;
	else
		taddr_req->BNDADDR_maxlen = 0;
	
	if (peeraddr)
		taddr_req->PEERADDR_maxlen = peeraddr->addr.maxlen;
	else
		taddr_req->PEERADDR_maxlen = 0;

	if (tli_ioctl(fd, TI_GETPEERNAME, buf, total_len) == -1)
		goto rtn;

	taddr_ack = (struct T_getaddr_ack *)buf;
	if (boundaddr) {
		boundaddr->addr.len = taddr_ack->BNDADDR_length;
		if (boundaddr->addr.len > 0)
			memcpy(boundaddr->addr.buf, 
			    &buf[taddr_ack->BNDADDR_offset],
			    taddr_ack->BNDADDR_length);
	}
	if (peeraddr) {
		if (tli->tlis_state == T_DATAXFER) {
			peeraddr->addr.len = taddr_ack->PEERADDR_length;
			if (peeraddr->addr.len > 0)
				memcpy(peeraddr->addr.buf, 
				    &buf[taddr_ack->PEERADDR_offset],
		    		    taddr_ack->BNDADDR_length);
		} else
			peeraddr->addr.len = 0;
	}
	code = 0;
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_bind (fd, req, ret, code);
#endif

badfd:
	return code;
}
