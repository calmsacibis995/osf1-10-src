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
static char rcsid[] = "@(#)$RCSfile: tconnect.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/08/18 21:51:34 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** tconnect.c 1.4, last change 1/29/90
 **/

#include <tli/common.h>
#include <sys/stream.h>
#include <stropts.h>
#include <tli/tihdr.h>

int
t_connect (
	int			fd,
	struct t_call * 	sndcall,
	struct t_call * 	rcvcall)
{
	char *			buf;
	char			stack_buf[TLI_STACK_BUF_SIZE];
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	struct strbuf *		dataptr;
	int			ret;
	struct T_conn_con *	tcc;
	struct T_conn_req *	tcr;
	int			ctl_len;
	int     		code;
	struct	tli_st *	tli;
	int			iflags;

	code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY)))
		goto badfd;

	
	if (tli->tlis_state == T_UNBND) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}
	if (TLI_ILOOK(fd, tli) == T_DISCONNECT) {
		_Set_terrno(TLOOK);
		goto rtn;
	}
	if (!sndcall) {
		_Set_terrno(TBADADDR);
		goto rtn;
	}
	ctl_len = sizeof(struct T_conn_req);
	if (sndcall->addr.len > 0)
		ctl_len += sndcall->addr.len;
	if (sndcall->opt.len > 0)
		ctl_len += sndcall->opt.len;

	if (ctl_len > sizeof(stack_buf)) {
		if (!(buf = (char *)malloc(ctl_len))) {
			_Set_terrno(TSYSERR);
			_Seterrno(ENOMEM);
			goto rtn;
		}
	} else
		buf = stack_buf;

	tcr = (struct T_conn_req *)&buf[0];
	tcr->PRIM_type = T_CONN_REQ;
	if (sndcall->addr.len > 0) {
		tcr->DEST_length = sndcall->addr.len;
		tcr->DEST_offset = sizeof(struct T_conn_req);
		memcpy(&buf[tcr->DEST_offset], sndcall->addr.buf, tcr->DEST_length);
	} else {
		tcr->DEST_length = 0;
		tcr->DEST_offset = 0;
	}
	if (sndcall->opt.len > 0) {
		tcr->OPT_length = sndcall->opt.len;
		tcr->OPT_offset = tcr->DEST_offset + tcr->DEST_length;
		memcpy(&buf[tcr->OPT_offset], sndcall->opt.buf, tcr->OPT_length);
	} else {
		tcr->OPT_length = 0;
		tcr->OPT_offset = 0;
	}
	ctlbuf.buf = (char *)tcr;
	ctlbuf.len = ctl_len;

	/*
	 * Only send data if the length is greater than 0.
	 * You can't send 0 bytes.
	 */
	if (sndcall->udata.len > 0) {
		databuf.buf = sndcall->udata.buf;
		databuf.len = sndcall->udata.len;
	} else
		databuf.len = -1;

	ret = putmsg(fd, &ctlbuf, &databuf, 0);

	if (buf != stack_buf)
		free(buf);

	if (ret == -1) {
		t_unix_to_tli_error();
		goto rtn;
	}

	/* Check for connection request acknowledgment */
	if ( t_chk_ack(fd, tli, T_CONN_REQ) == -1 )
		goto rtn;	
	
	/* If the stream is in non-blocking mode, get out now. */
	if (t_is_nonblocking(fd)) {
		_Set_terrno( TNODATA);
		TLI_NEXTSTATE(tli, TLI_CONNECT2);
		goto rtn;
	}

	switch (t_ircvconnect(fd, tli, rcvcall)) {
	case 0:
		code = 0;
		/* fallthru */
	case TBUFOVFLW:
		TLI_NEXTSTATE(tli, TLI_CONNECT1);
		break;
	case -1:
	default:
		break;
	}
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_connect (fd, sndcall, rcvcall, code);
#endif

badfd:
	return code;
}
/*
 * t_ircvconnect() 
 *		return -1:
 *			fatal error
 *		return TBUFOVFLW:
 *			caller should return -1 and t_errno = TBUFOVFLW
 *			this will allow state to transition to next state
 *		return 0:
 *			no error
 */

int
t_ircvconnect (
	int			fd,
	struct tli_st *		tli,
	struct t_call *		rcvcall)
{
	int			ctl_len;
	int			iflags;
	struct netbuf *		netbufp;
	int			ret;
	struct T_conn_con *	tcc;

	/* Get the connection confirm */
	netbufp = rcvcall ? &rcvcall->udata : nilp(struct netbuf);
	switch (t_get_primitive(fd, tli, T_CONN_CON, 
	    (int)sizeof(struct T_conn_con), netbufp)) {
		case TBUFOVFLW:
			return TBUFOVFLW;
		case -1:
			return -1;
		default:
			break;
	}

	if ( rcvcall ) {
		tcc = (struct T_conn_con *)tli->tlis_proto_buf;
		if (tcc->RES_length > 0  &&  rcvcall->addr.maxlen > 0) {
			if (rcvcall->addr.maxlen < tcc->RES_length) {
				_Set_terrno(TBUFOVFLW);
				return TBUFOVFLW;
			}
			rcvcall->addr.len = tcc->RES_length;
			memcpy(rcvcall->addr.buf, &tli->tlis_proto_buf[tcc->RES_offset], rcvcall->addr.len);
		} else {
#ifndef XTI
			/*
			 * Although XPG3 doesn't say so, it implies that if
			 * rcvcall is specified, then you must receive the
			 * addr component as well.  So setting the maxlen of 0
			 * is treated as falling short of the available length
			 * and hence TBUFOVFLW error
			 *
			 * For TLI generate this error even if maxlen = 0.
			 *
			 */
			if (rcvcall->addr.maxlen <= 0) {
				_Set_terrno(TBUFOVFLW);
				return TBUFOVFLW;
			}
#endif /* XTI */
			rcvcall->addr.len = 0;
		}	
		if (tcc->OPT_length > 0  &&  rcvcall->opt.maxlen > 0) {
			if (rcvcall->opt.maxlen < tcc->OPT_length) {
				_Set_terrno(TBUFOVFLW);
				return TBUFOVFLW;
			}
			rcvcall->opt.len = tcc->OPT_length;
			memcpy(rcvcall->opt.buf, &tli->tlis_proto_buf[tcc->OPT_offset], rcvcall->opt.len);
		} else
				rcvcall->opt.len = 0;
	}
	return 0;
}
