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
static char *rcsid = "@(#)$RCSfile: tsnddis.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/12/21 16:07:27 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989  Mentat Inc.
 ** tsnddis.c 1.4, last change 1/29/90
 **/

#include <sys/stream.h>
#include <tli/common.h>
#include <stropts.h>
#include <tli/tihdr.h>
#include <sys/poll.h>

int
t_snddis (
	int			fd,
	struct t_call * 	call)
{
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	struct T_discon_req	tdr;
	struct tli_st *		tli;
        int    			code;
	int			ret;
	struct pollfd		pollfd[1];

        code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY)))
		goto badfd;
	
	if ( tli->tlis_servtype == T_CLTS ) {	
		_Set_terrno(TNOTSUPPORT);
		goto rtn;
	}
	if (TLI_ILOOK(fd, tli) == T_DISCONNECT) {
		_Set_terrno( TLOOK);
		goto rtn;
	}
	if (tli->tlis_state == T_UNBND || tli->tlis_state == T_IDLE) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}

	tdr.PRIM_type = T_DISCON_REQ;
	ctlbuf.buf = (char *)&tdr;
	ctlbuf.len = sizeof(struct T_discon_req);
	if ( call ) {
		tdr.SEQ_number = call->sequence;
		databuf.buf = call->udata.buf;
		databuf.len = call->udata.len ? call->udata.len: -1;
	} else {
		tdr.SEQ_number = -1;
		databuf.len = -1;
	}
	/*
	 * The following needs to be High priority message despite TPI
	 * specs that says otherwise.  If not, the disconnect requests
	 * in a non-blocking env would hang indefinitely in a flow
	 * controlled stream.  That would violate the XTI specs.
	 *
	 */
	if (putmsg(fd, &ctlbuf, &databuf, RS_HIPRI) == -1) {
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
	code = t_chk_ack(fd, tli, T_DISCON_REQ);

	TLI_TSYNC(fd);
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_snddis (fd, call, code);
#endif

badfd:
	return code;
}

