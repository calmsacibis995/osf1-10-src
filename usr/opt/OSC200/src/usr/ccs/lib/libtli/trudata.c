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
static char rcsid[] = "@(#)$RCSfile: trudata.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/05/13 12:11:25 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** trudata.c 1.3, last change 10/16/89
 **/

#include <tli/common.h>
#include <sys/stream.h>
#include <stropts.h>
#include <tli/tihdr.h>

int
t_rcvudata (
	int			fd,
reg	struct t_unitdata * 	ud,
	int *			flags)
{
reg	struct T_unitdata_ind * tudi;
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	int			iflags;
	int			len;
	int			ret;
	struct tli_st *		tli;
        int     		code;

        code = -1;
	if(!(tli = iostate_sw(fd, IOSTATE_VERIFY)))
		goto badfd;

	if ( tli->tlis_servtype != T_CLTS ) {
		_Set_terrno(TNOTSUPPORT);
		goto rtn;
	}
#ifdef XTI
	if ( tli->tlis_state != T_IDLE ) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}
#endif
#ifdef OBSOLETE
	if((ud->addr.maxlen == 0) || (ud->opt.maxlen == 0)) {
		_Set_terrno(TBUFOVFLW);
		goto rtn;
	}
#else
	/* 
	 * Changed from above to allow opt.maxlen be allowed to be 0.
	 * Actually, XPG3 spec says this *should be* 0 for UDP. We 
	 * don't require this (should we?)
	 * How about ISO CLTS ?
	 */
	/* 
	 * TBD - if maxlen is 0, shouldn't the message be deq'ed 
	 *       anyways ?
	 */
	if(ud->addr.maxlen == 0) {
		_Set_terrno(TBUFOVFLW);
		goto rtn;
	}
	ud->opt.len = 0; /* Initialize the option length field since it is
			    modified below only if opt.maxlen was non-0 */
#endif

	if (tli->tlis_flags & TLIS_SAVED_PROTO) {
		_Set_terrno(TLOOK);
		goto rtn;
	}

	ctlbuf.buf = tli->tlis_proto_buf;
	ctlbuf.maxlen = _DEFAULT_STRCTLSZ;

	databuf.buf = ud->udata.buf;
	databuf.maxlen = ud->udata.maxlen;
	
	iflags = 0;
	ret = getmsg(fd, &ctlbuf, &databuf, &iflags);
	switch (ret) {
	case MORECTL:
	case MORECTL|MOREDATA:
		/* This should *never* happen... */
		_Set_terrno(TSYSERR);
		errno = EPROTO;
		goto rtn;
	case MOREDATA:
		if (flags)
			*flags = T_MORE;
		break;
	case 0:
		if (flags)
			*flags = 0;
		break;
	case -1:
	default:
		t_unix_to_tli_error();
		goto rtn;	
	}

	ud->udata.len = databuf.len > 0 ? databuf.len : 0;

	len = ctlbuf.len;
	if (len >= (int)sizeof(long)) {
		if (((union T_primitives *)tli->tlis_proto_buf)->type != T_UNITDATA_IND) {
			tli->tlis_flags |= TLIS_SAVED_PROTO;
			_Set_terrno(TLOOK);
			goto rtn;
		}
	}
	if (len >= (int)sizeof(struct T_unitdata_ind)) {
		tudi = (struct T_unitdata_ind *)tli->tlis_proto_buf;
		len = tudi->SRC_length;
		if (len > 0  &&  ud->addr.maxlen > 0) {
			if (ud->addr.maxlen < len) {
				_Set_terrno(TBUFOVFLW);
				goto rtn;
			}
			ud->addr.len = len;
			memcpy(ud->addr.buf, &tli->tlis_proto_buf[tudi->SRC_offset], len);
		}
		len = tudi->OPT_length;
		if (len > 0  &&  ud->opt.maxlen > 0) {
			if (ud->opt.maxlen < len) {
				_Set_terrno(TBUFOVFLW);
				goto rtn;
			}
			ud->opt.len = len;
			memcpy(ud->opt.buf, &tli->tlis_proto_buf[tudi->OPT_offset], len);
		}
	 } else if (databuf.len == 0) {
                _Set_terrno(TNODATA);
                goto rtn;
        }
	code = 0;
	if (*flags & T_MORE) {
		if (! (tli->tlis_flags & TLIS_MORE_RUDATA))
			tli->tlis_flags |= TLIS_MORE_RUDATA;
		else {
			ud->addr.len = 0;
			ud->opt.len = 0;
		}
	} else {
		if (tli->tlis_flags & TLIS_MORE_RUDATA) {
			ud->addr.len = 0;
			ud->opt.len = 0;
		}
		tli->tlis_flags &= ~TLIS_MORE_RUDATA;
	}
		
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_rcvudata (fd, ud, flags, code);
#endif

badfd:
	return code;
}
