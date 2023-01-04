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
static char rcsid[] = "@(#)$RCSfile: tlook.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/03/16 23:20:59 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** tlook.c 1.2, last change 1/29/90
 **/

#include <tli/common.h>
#include <sys/stream.h>
#include <stropts.h>
#include <tli/tihdr.h>
#ifdef XTI
#include <sys/poll.h>
#endif

int
t_ilook (
	int		fd)
{
	long		type;
	struct strpeek	strp;
	struct tli_st *	tli;
        int    		code;

        code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY)))
		goto rtn;

	if (!(tli->tlis_flags & TLIS_SAVED_PROTO)) {
		strp.ctlbuf.buf = (char *)&type;
		strp.ctlbuf.maxlen = sizeof(type);
		strp.databuf.buf = nilp(char);
		strp.databuf.maxlen = -1;
		strp.flags = 0;
		switch (stream_ioctl(fd, I_PEEK, (char *)&strp)) {
		case -1:
#ifndef XTI
			if (_Geterrno() == EPROTO)
				code = T_ERROR;
#endif
	        	t_unix_to_tli_error();
			goto rtn;
		case 0:
			code = 0;
			goto rtn;
		case 1:
			break;
		}
		if (strp.ctlbuf.len != sizeof(type)) {
			code = 0;
			goto rtn;
        	}
	} else
		type = ((union T_primitives *)tli->tlis_proto_buf)->type;

	TLI_UNLOCK(tli);
	switch (type) {
	case T_CONN_CON:
		code = T_CONNECT;
		break;
	case T_CONN_IND:
		code = T_LISTEN;
		break;
	case T_DATA_IND:
		code = T_DATA;
		break;
	case T_DISCON_IND:
		code = T_DISCONNECT;
		break;
	case T_EXDATA_IND:
		code = T_EXDATA;
		break;
	case T_ORDREL_IND:
		code = T_ORDREL;
		break;
	case T_UNITDATA_IND:
		code = T_DATA;
		break;
	case T_UDERROR_IND:
		code = T_UDERR;
		break;
	default:
		_Seterrno(EPROTO);
		_Set_terrno(TSYSERR);
		break;
	}
rtn:

#ifdef XTIDBG
	tr_look (fd, code);
#endif /* XTIDBG */

	return code;
}

#ifdef	XTI
int
t_look (
	int		fd)
{
	int		i1;
	struct tli_st *	tli;
	struct pollfd	fds[1];

	i1 = t_ilook(fd);
	if ( i1 != 0 )
		return i1;
	if ((tli = iostate_sw(fd, IOSTATE_VERIFY)) 
	&&  (tli->tlis_flags & (TLIS_DATA_STOPPED | TLIS_EXDATA_STOPPED))) {
		fds[0].fd = fd;
		fds[0].events = POLLOUT;
		if (poll(fds, 1L, 0L) == 1
		&&  fds[0].revents == POLLOUT) {
			if (tli->tlis_flags & TLIS_EXDATA_STOPPED) {
				tli->tlis_flags &= ~TLIS_EXDATA_STOPPED;
				i1 = T_GOEXDATA;
			}
			else {
				tli->tlis_flags &= ~TLIS_DATA_STOPPED;
				i1 = T_GODATA;
		        }
		}
		TLI_UNLOCK(tli);
		
	}
#ifdef XTIDBG
	tr_look (fd, i1);
#endif /* XTIDBG */

	return i1;
}
#endif

#if defined(_THREAD_SAFE) || defined(_REENTRANT)

int
__t_ilook (
	int		fd,
	struct tli_st *	tli)
{
	long		type;
	struct strpeek	strp;
        int    		code;

        code = -1;
	if (!(tli->tlis_flags & TLIS_SAVED_PROTO)) {
		strp.ctlbuf.buf = (char *)&type;
		strp.ctlbuf.maxlen = sizeof(type);
		strp.databuf.buf = nilp(char);
		strp.databuf.maxlen = -1;
		strp.flags = 0;
		switch (stream_ioctl(fd, I_PEEK, (char *)&strp)) {
		case -1:
#ifndef XTI
			if (_Geterrno() == EPROTO)
				code = T_ERROR;
#endif
	        	t_unix_to_tli_error();
			goto rtn;
		case 0:
			code = 0;
			goto rtn;
		case 1:
			break;
		}
		if (strp.ctlbuf.len != sizeof(type)) {
			code = 0;
			goto rtn;
        	}
	} else
		type = ((union T_primitives *)tli->tlis_proto_buf)->type;

	switch (type) {
	case T_CONN_CON:
		code = T_CONNECT;
		break;
	case T_CONN_IND:
		code = T_LISTEN;
		break;
	case T_DATA_IND:
		code = T_DATA;
		break;
	case T_DISCON_IND:
		code = T_DISCONNECT;
		break;
	case T_EXDATA_IND:
		code = T_EXDATA;
		break;
	case T_ORDREL_IND:
		code = T_ORDREL;
		break;
	case T_UNITDATA_IND:
		code = T_DATA;
		break;
	case T_UDERROR_IND:
		code = T_UDERR;
		break;
	default:
		_Seterrno(EPROTO);
		_Set_terrno(TSYSERR);
		break;
	}
rtn:
#ifdef XTIDBG
	tr_look (fd, code);
#endif /* XTIDBG */
	return code;
}

#ifdef	XTI
int
__t_look (
	int		fd,
	struct tli_st *	tli)
{
	int		i1;
	struct pollfd	fds[1];

	i1 = __t_ilook(fd, tli);
	if ( i1 != 0 )
		return i1;
	if (tli->tlis_flags & (TLIS_DATA_STOPPED | TLIS_EXDATA_STOPPED)) {
		fds[0].fd = fd;
		fds[0].events = POLLOUT;
		if (poll(fds, 1L, 0L) == 1
		&&  fds[0].revents == POLLOUT) {
			if (tli->tlis_flags & TLIS_EXDATA_STOPPED) {
				tli->tlis_flags &= ~TLIS_EXDATA_STOPPED;
				i1 = T_GOEXDATA;
			}
			else {
				tli->tlis_flags &= ~TLIS_DATA_STOPPED;
				i1 = T_GODATA;
		        }
		}
	}
#ifdef XTIDBG
	tr_look (fd, i1);
#endif /* XTIDBG */
	return i1;
}
#endif

#endif /* _THREAD_SAFE || _REENTRANT */
