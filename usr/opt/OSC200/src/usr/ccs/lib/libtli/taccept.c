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
static char *rcsid = "@(#)$RCSfile: taccept.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/05/12 16:24:55 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/** Copyright (c) 1989  Mentat Inc.
 ** taccept.c 1.2, last change 6/4/90
 **/

#include <sys/stream.h>
#include <tli/common.h>
#include <stropts.h>
#include <tli/tihdr.h>
#include <sys/poll.h>

int
t_accept (
	int			fd,
	int			resfd,
	struct t_call *		call)
{
	char *			buf;
	int			event;
	char			stack_buf[TLI_STACK_BUF_SIZE];
	struct strfdinsert	strfd;
	struct T_conn_res *	tcr;
	int			ret;
	int			total_len;
	struct tli_st *		tli, * tli2;
	int     		code;
	struct pollfd		pollfd[1];

	code = -1;
	if (!(tli = iostate_sw(fd,IOSTATE_VERIFY)))
		goto badfd;

	if (fd != resfd) {
		if (!(tli2 = iostate_sw(resfd,IOSTATE_VERIFY))) {
			TLI_UNLOCK(tli);
			goto badfd;
		}
	}

	if (tli->tlis_servtype == T_CLTS ) {
		_Set_terrno(TNOTSUPPORT);
		goto rtn;
	}
	event = TLI_LOOK(fd, tli);

	if (event) {
		if (event == T_LISTEN &&  fd == resfd)
			_Set_terrno(TBADF);
		else
			_Set_terrno(TLOOK);
		goto rtn;
	}

	if (tli->tlis_state != T_INCON) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}
	if ((fd != resfd) && (tli2->tlis_state != T_IDLE)) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}

	total_len = (call->opt.len >= 0 ? call->opt.len : 0);
	total_len += sizeof(struct T_conn_res);
	if (total_len > sizeof(stack_buf)) {
		if (!(buf = (char *)malloc(total_len))) {
			_Set_terrno(TSYSERR);
			_Seterrno(ERANGE);
			goto rtn;
		}
	} else
		buf = stack_buf;

	tcr = (struct T_conn_res *)&buf[0];
	tcr->PRIM_type = T_CONN_RES;
	if (call->opt.len > 0) {
		tcr->OPT_length = call->opt.len;
		tcr->OPT_offset = sizeof(struct T_conn_res);
		memcpy(&buf[tcr->OPT_offset], call->opt.buf, tcr->OPT_length);
	} else {
		tcr->OPT_length = 0;
		tcr->OPT_offset = 0;
	}
	tcr->SEQ_number = call->sequence;
	strfd.ctlbuf.buf = (char *)tcr;
	strfd.ctlbuf.len = total_len;

	/*
	 * Only send data if the length is greater than 0.
	 * You can't send 0 bytes.
	 */
	if (call->udata.len > 0) {
		strfd.databuf.buf = call->udata.buf;
		strfd.databuf.len = call->udata.len;
	} else
		strfd.databuf.len = -1;

	strfd.flags = 0;
	strfd.fildes = resfd;
	strfd.offset = sizeof(long);
	ret = stream_ioctl(fd, I_FDINSERT, (char *)&strfd);

	if (buf != stack_buf)
		free(buf);

	if (ret == -1) {
		t_unix_to_tli_error();
		goto rtn;
	}
	/*
	 * Before we are ready to get acknowledgement make sure that we
	 * have something on the read queue.  If not and if fd is non-blocking
	 * we will erroneously return ENODATA to the unsuspecting caller.
	 */
	pollfd[0].fd     = fd;
	pollfd[0].events = POLLIN|POLLPRI;
	/* Now we wait until ack is guaranteed */
	if (poll(pollfd, 1, -1) < 0) {
		t_unix_to_tli_error();
		goto rtn;
	}
	/* Check for accept acknowledgment */
	if (t_chk_ack(fd, tli, T_CONN_RES) == -1 )
		goto rtn;

	TLI_TSYNC(fd);
	if ( resfd != fd ) {
		TLI_TSYNC(resfd);
	}
	return 0;
rtn:   
	TLI_UNLOCK(tli);
	if ( resfd != fd )
		TLI_UNLOCK(tli2);

#ifdef XTIDBG
        tr_accept (fd, resfd, call, code);
#endif

badfd:
	return code;
}
