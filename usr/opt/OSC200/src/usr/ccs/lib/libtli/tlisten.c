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
static char rcsid[] = "@(#)$RCSfile: tlisten.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/05/20 22:03:37 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** tlisten.c 1.5, last change 1/29/90
 **/

#include <tli/common.h>
#include <sys/stream.h>
#include <stropts.h>
#include <tli/tihdr.h>
#ifdef XTI
#include <tli/timod.h>
#endif

int
t_listen (
	int			fd,
reg	struct t_call *		call)
{
	char			buf[TLI_STACK_BUF_SIZE];
	struct T_conn_ind *	tci = (struct T_conn_ind *)buf;
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	int			iflags;
	int			ret;
	struct t_info		tinfo;
	struct tli_st *		tli;
#ifdef XTI
	XTIS			xtis;
#endif
        int    			code;
	struct netbuf *		netbufp;

        code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY)))
		goto badfd;
	
	if ( tli->tlis_servtype == T_CLTS ) {
		_Set_terrno(TNOTSUPPORT);
		goto rtn;
	}
#ifdef XTI
	if (tli_ioctl(fd, TI_XTI_GET_STATE, (char *)&xtis, sizeof(xtis)) == -1
	||  xtis.xtis_qlen <= 0) {
		_Set_terrno(TBADQLEN);
		goto rtn;
	}
	if (tli->tlis_state != T_IDLE &&  tli->tlis_state != T_INCON) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}
#endif

	netbufp = call ? &call->udata : nilp(struct netbuf);
	switch (t_get_primitive(fd, tli, T_CONN_IND, 
	       (int)sizeof(struct T_conn_ind), netbufp)) {
		case -1:
		case TBUFOVFLW:
			goto rtn;
		case 0:
		default:
			break;
	}

	tci = (struct T_conn_ind *)tli->tlis_proto_buf;
	call->sequence = tci->SEQ_number;
	if (tci->SRC_length > 0  &&  call->addr.maxlen > 0) {
		if (call->addr.maxlen < tci->SRC_length) {
			_Set_terrno(TBUFOVFLW);
			goto rtn;
		}
		call->addr.len = tci->SRC_length;
		memcpy(call->addr.buf, &tli->tlis_proto_buf[tci->SRC_offset], call->addr.len);
	} else {
		if(call->addr.maxlen <= 0) {
			_Set_terrno(TBUFOVFLW);
			goto rtn;
		}
		call->addr.len = 0;
	}
	if (tci->OPT_length > 0  &&  call->opt.maxlen > 0) {
		if (call->opt.maxlen < tci->OPT_length) {
			_Set_terrno(TBUFOVFLW);
			goto rtn;
		}
		call->opt.len = tci->OPT_length;
		memcpy(call->opt.buf, &tli->tlis_proto_buf[tci->OPT_offset], call->opt.len);
	} else
		call->opt.len = 0;
        code = 0;
rtn:
	if (code == 0 || ((code == -1) && (_Get_terrno() == TBUFOVFLW)))
		TLI_NEXTSTATE(tli, TLI_LISTEN);
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_listen (fd, call, code);
#endif

badfd:
	return code;
}
