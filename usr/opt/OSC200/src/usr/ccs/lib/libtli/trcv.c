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
static char *rcsid = "@(#)$RCSfile: trcv.c,v $ $Revision: 4.3.7.4 $ (DEC) $Date: 1993/11/16 15:29:31 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989  Mentat Inc.
 ** trcv.c 1.1, last change 8/9/89
 **/

#include <tli/common.h>
#include <sys/stream.h>
#include <stropts.h>
#include <tli/tihdr.h>

int
t_rcv (
	int			fd,
	char *			buf,
	unsigned 		nbytes,
	int *			flags)
{
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	boolean			data_only;
	int			iflags;
	int			ret;
	struct T_data_ind *	tdi;
	struct tli_st	*	tli;
        int     		code;

        code = -1;
	if(!(tli = iostate_sw(fd, IOSTATE_VERIFY))) {
		goto badfd;
	}
#ifdef XTI
	if ( tli->tlis_state != T_DATAXFER  &&  tli->tlis_state != T_OUTREL ) {
		if (t_ilook(fd) == T_DISCONNECT) {
			_Set_terrno(TLOOK);
		} else {
			_Set_terrno(TOUTSTATE);
		}
		goto rtn;
	}
#endif
	if ( tli->tlis_servtype == T_CLTS ) {
		_Set_terrno(TNOTSUPPORT);
		goto rtn;
	}
	data_only = true;
	if (tli->tlis_flags & TLIS_SAVED_PROTO) {
		long    type = ((union T_primitives *)tli->tlis_proto_buf)->type;
		if (type != T_DATA_IND  &&  type != T_EXDATA_IND) {
			_Set_terrno(TLOOK);
			goto rtn;
		}
		tdi = (struct T_data_ind *)tli->tlis_proto_buf;
		data_only = false;
	}

	ctlbuf.buf = tli->tlis_proto_buf;
	ctlbuf.maxlen = _DEFAULT_STRCTLSZ;
	databuf.buf = buf;
	databuf.maxlen = nbytes;
	iflags = 0;
	ret = getmsg(fd, &ctlbuf, &databuf, &iflags);

	switch (ret) {
		case MORECTL:
		case MORECTL|MOREDATA:
		/* This should *never* happen... */
			_Set_terrno(TSYSERR);
			_Seterrno(EPROTO);
			goto rtn;
		case MOREDATA:
			if (ctlbuf.len != -1)
				tli->tlis_flags |= TLIS_SAVED_PROTO;
			*flags = T_MORE;
			break;
		case 0:
			tli->tlis_flags &= ~TLIS_SAVED_PROTO;
			*flags = 0;
			break;
		case -1:
		default:
			t_unix_to_tli_error();
			goto rtn;
	}

	 if (ctlbuf.len != -1) {
		tdi = (struct T_data_ind *)tli->tlis_proto_buf;
		if (ctlbuf.len != (int)sizeof(struct T_data_ind)
		|| (tdi->PRIM_type != T_DATA_IND  &&  tdi->PRIM_type != T_EXDATA_IND)) {
			tli->tlis_flags |= TLIS_SAVED_PROTO;
			_Set_terrno(TLOOK);
			goto rtn;
		}
		data_only = false;
	}

	if (!data_only) {
		if (tdi->MORE_flag & T_MORE)
			*flags |= T_MORE;
		if ( tdi->PRIM_type == T_EXDATA_IND )
			*flags |= T_EXPEDITED;
	}
	code = databuf.len;
rtn:

	TLI_UNLOCK(tli);

#ifdef XTIDBG
        tr_rcv (fd, buf, nbytes, flags, code);
#endif

badfd:
	return code;
}
