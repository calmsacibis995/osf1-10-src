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
static char *rcsid = "@(#)$RCSfile: tfixup.c,v $ $Revision: 4.2.8.4 $ (DEC) $Date: 1993/11/23 21:36:17 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989  Mentat Inc.
 ** tfixup.c 1.3, last change 1/29/90
 **/

#include <tli/common.h>
#include <sys/stream.h>
#include <stropts.h>
#include <tli/tihdr.h>

void
t_unix_to_tli_error () {
	switch (_Geterrno()) {
	case EBADF:
	case ENOSTR:
		_Set_terrno(TBADF);
		break;
	case EACCES:
		_Set_terrno(TACCES);
		break;
	case EAGAIN:
/** EAGAIN == EWOULDBLOCK **/
	case ENODATA:
		_Set_terrno(TNODATA);
		break;
	default:
		_Set_terrno(TSYSERR);
		break;
	}
}

int
t_chk_ack (
	int			fd,
	struct tli_st *		tli,
	int			prim_type)
{
	struct strbuf		ctlbuf;
	int			iflags;
	struct T_error_ack *	tea;
	struct T_ok_ack	*	toa;
	int ret;


/* NOTE: should we check for saved primitives here?? */
/* TODO: poll here so that we wait even if we're in non-blocking mode. */

	ctlbuf.buf = (char *)tli->tlis_proto_buf;
	ctlbuf.maxlen = _DEFAULT_STRCTLSZ;
	ctlbuf.len = 0;
	/* 
	 * Following is counter-intuitive because the expected msg is RS_HIPRI,
	 * however, if the message is low-priority (can happen if a 
	 * Disconnect indication sneaked in, forcing a purge of OK ack in
	 * xtiso), then it won't be retrieved, making the connection hang
	 *
	 */
	iflags = 0;

	/*
	 * Since we're reading a maximum size control buffer and no data,
	 * this getmsg should return 0.  MORECTL would mean that the
	 * transport provider created a M_PROTO block larger than the
	 * maximum allowed for writing.  MOREDATA can't happen.
	 * getmsg may return -1 if the stream is in non-blocking mode
	 * and there is no message available.
	 */
	toa = (struct T_ok_ack *)tli->tlis_proto_buf;
	/*
	 * If getmsg returns non-zero positive integer, it's not an error
	 * condition per se.  The decision about that will be made by the
	 * xtiso/transport provider and will be reflected in the t_errno
	 * appropriately
	 */
	if ( getmsg(fd, &ctlbuf, nilp(struct strbuf), &iflags) < 0 ) {
		t_unix_to_tli_error();
		return -1;
	}

	if ( ctlbuf.len < (int)sizeof(toa->PRIM_type) ) {
		/* Transport provider error -- illegal TPI message */
		_Set_terrno(TSYSERR);
		_Seterrno(EPROTO);
		return -1;
	}

	toa = (struct T_ok_ack *)tli->tlis_proto_buf;
	if ( toa->PRIM_type == T_ERROR_ACK ) {
		struct T_error_ack * tea = (struct T_error_ack *)tli->tlis_proto_buf;
		if ( ctlbuf.len < (int)sizeof(struct T_error_ack)
		||  tea->ERROR_prim != prim_type ) {
			/* NOTE: how can this happen?? */
			tli->tlis_flags |= TLIS_SAVED_PROTO;
			_Set_terrno(TLOOK);
			return -1;
		}
		_Set_terrno(tea->TLI_error);
		_Seterrno(tea->UNIX_error);
		return -1;
	}
	if ( ctlbuf.len < (int)sizeof(struct T_ok_ack)
	||  toa->PRIM_type != T_OK_ACK
	||  toa->CORRECT_prim != prim_type ) {
		tli->tlis_flags |= TLIS_SAVED_PROTO;
		_Set_terrno(TLOOK);
		return -1;
	}
	return 0;
}

/*
 * t_get_primitive()
 *		return -1:
 *			fatal error
 *		return TBUFOVFLW:
 *			caller should return -1 and t_errno = TBUFOVFLW
 *			this will allow state to transition to next state
 *		return 0:
 *			no error
 */
int
t_get_primitive (
	int			fd,
	struct tli_st *		tli,
	long			expected_primitive,
	int			expected_len,
	struct netbuf *		netbufp)
{
	char			buf1[64];
	char			buf2[64];
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	struct strpeek		strp;
	int			iflags, len;
	int			ret = 0;
	union T_primitives *	prim;

	
	prim = (union T_primitives *)tli->tlis_proto_buf;
	if (!(tli->tlis_flags & TLIS_SAVED_PROTO)) {

		/*
		 * If there is no primitive already waiting in the library,
		 * then call getmsg to retrieve just the M_PROTO part of the
		 * next message.
		 */
		ctlbuf.buf = tli->tlis_proto_buf;
		ctlbuf.len = 0;
		ctlbuf.maxlen = _DEFAULT_STRCTLSZ;

		iflags = 0;
		ret = getmsg(fd, &ctlbuf, nilp(struct strbuf), &iflags);
		switch (ret) {
		case MORECTL:
		case MORECTL|MOREDATA:
			/* This should *never* happen...
			 * MORECTL means that the transport provider
			 * created a M_PROTO block larger than the
			 * maximum allowed for writing.
			 */
			_Set_terrno(TSYSERR);
			_Seterrno(EPROTO);
			return -1;
		case MOREDATA:
		case 0:
			if (ctlbuf.len != -1 && ctlbuf.len < expected_len) {
				if (prim->type != expected_primitive)
					goto tlook_error;
				_Set_terrno(TSYSERR);
				_Seterrno(EPROTO);
				return -1;
			}
			break;
		case -1:
		default:
			t_unix_to_tli_error();
			return -1;
		}
	}

	if (prim->type != expected_primitive)
		goto tlook_error;

	/* Check for data associated with this primitive. */
	strp.ctlbuf.buf = buf1;
	strp.ctlbuf.maxlen = sizeof(buf1);
	strp.databuf.buf = buf2;
	strp.databuf.maxlen = sizeof(buf2);
	strp.flags = 0;
	switch (stream_ioctl(fd, I_PEEK, (char *)&strp)) {
	case -1:
#ifndef XTI
		if (_Geterrno() == EPROTO)
			return T_ERROR;
#endif
		t_unix_to_tli_error();
		return -1;
	case 1:
		if ( strp.ctlbuf.len <= 0
		&&   strp.databuf.len > 0 ) {
			break;
		}
		fallthru;
	case 0:
		/* No data, we're all done. */
		if (ret == MOREDATA)
			break;
		if (netbufp)
			netbufp->len = 0;
		tli->tlis_flags &= ~TLIS_SAVED_PROTO;
		return 0;
	}

	/* There's data, so do a second getmsg to retrieve all data. */

	tli->tlis_flags &= ~TLIS_SAVED_PROTO;
	
	if ((databuf.buf = (char *)malloc(len = (_DEFAULT_STRMSGSZ))) ==
								(char *)NULL) {
		_Set_terrno(TSYSERR);
		_Seterrno(ENOMEM);
		return -1;
	}

	databuf.maxlen = len;
	iflags = 0;
	ret = getmsg(fd, nilp(struct strbuf), &databuf, &iflags);

	if (netbufp) {
		netbufp->len = 0;
		if ((databuf.len > 0) && (netbufp->maxlen >= databuf.len)) {
			memmove(netbufp->buf, databuf.buf,
				(size_t)(netbufp->len = databuf.len));
		}
	}
	free(databuf.buf);		/* get rid of the temp storage */
	/* 
	 * if there was some data received, then make sure that caller had
	 * space for it 
	 */
	if ((databuf.len>0) && (netbufp && (netbufp->maxlen < databuf.len))) {
		_Set_terrno(TBUFOVFLW);
		return TBUFOVFLW;
	}

	tli->tlis_flags &= ~TLIS_SAVED_PROTO;
	return 0;

tlook_error:
	tli->tlis_flags |= TLIS_SAVED_PROTO;
	_Set_terrno(TLOOK);
	return -1;
}
