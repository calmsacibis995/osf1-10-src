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
static char rcsid[] = "@(#)$RCSfile: tsudata.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:22:25 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** tsudata.c 1.2, last change 1/29/90
 **/

#include <sys/stream.h>
#include <tli/common.h>
#include <stropts.h>
#include <tli/tihdr.h>

int
t_sndudata (
	int			fd,
	struct t_unitdata * 	ud)
{
	char *			buf;
	char    		stack_buf[TLI_STACK_BUF_SIZE];
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	struct T_unitdata_req * tudr;
	int			len;
	struct tli_st *		tli;
        int     		code;
	int			ret;

        code = -1;
	if(!(tli = iostate_sw(fd, IOSTATE_VERIFY)))
		goto badfd;
	
	if ( tli->tlis_servtype != T_CLTS ) {
		_Set_terrno(TNOTSUPPORT);
		goto rtn;
	}
#ifdef	XTI
	if ( tli->tlis_state != T_IDLE ) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}
#else
	if ( tli->tlis_state != T_IDLE ) {
		_Seterrno( EPROTO);
		_Set_terrno(TSYSERR);
		goto rtn;
	}
#endif
	if ( ud->udata.len == 0 ) {
		_Set_terrno(TBADDATA);
		goto rtn;
	}

	/* NOTE: this check may need to be against tsdu for XTI */
	if ( ud->udata.len > tli->tlis_tidu_size ) {
		_Set_terrno(TSYSERR);
		_Seterrno(EPROTO);
		goto rtn;
	}
	len = sizeof(struct T_unitdata_req) + ud->addr.len + ud->opt.len;
	if (len > (int)sizeof(stack_buf)) {
		buf = (char *)malloc(len);
		if (!buf) {
			_Set_terrno(TBUFOVFLW);
			_Seterrno(ENOMEM);
			goto rtn;
		}
	} else
		buf = stack_buf;

	tudr = (struct T_unitdata_req *)&buf[0];
	ctlbuf.len = len;
	ctlbuf.buf = buf;
	tudr->PRIM_type = T_UNITDATA_REQ;
	databuf.buf = ud->udata.buf;
	databuf.len = ud->udata.len;
	len = ud->addr.len;
	if (len > 0) {
		tudr->DEST_offset = sizeof(struct T_unitdata_req);
		tudr->DEST_length = len;
		memcpy(&buf[sizeof(struct T_unitdata_req)], ud->addr.buf, len);
	} else {
		tudr->DEST_offset = 0;
		tudr->DEST_length = 0;
	}
	len = ud->opt.len;
	if (len > 0) {
		tudr->OPT_length = len;
		tudr->OPT_offset = sizeof(struct T_unitdata_req) + tudr->DEST_length;
		memcpy(&buf[tudr->OPT_offset], ud->opt.buf, len);
	} else {
		tudr->OPT_length = 0;
		tudr->OPT_offset = 0;
	}
	ret = putmsg(fd, &ctlbuf, &databuf, 0);

	if (buf != stack_buf)
		free(buf);

	if (ret == -1) {
#ifdef	XTI
		if ( _Geterrno() == ERANGE ) {
			/* Secret message from TIMOD!  T_UDERR waiting. */
			_Set_terrno(TLOOK);
			goto rtn;
		}
#endif
		t_unix_to_tli_error();
		if ( _Get_terrno() == TNODATA ) {
#ifdef XTI
			tli->tlis_flags |= TLIS_DATA_STOPPED;
#endif
			_Set_terrno(TFLOW);
			goto rtn;
		}
		goto rtn;
	}
	code = 0;
#ifdef XTI
	tli->tlis_flags &= ~TLIS_DATA_STOPPED;
#endif

rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_sndudata(fd, ud, code);
#endif

badfd:
	return code;
}
