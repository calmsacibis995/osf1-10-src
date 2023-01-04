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
static char *rcsid = "@(#)$RCSfile: toptmgmt.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/07/14 12:24:02 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989  Mentat Inc.
 ** toptmgmt.c 1.3, last change 10/16/89
 **/

#include <tli/common.h>
#include <sys/stream.h>
#include <tli/tihdr.h>
#include <tli/timod.h>

int
t_optmgmt (
	int			fd,
	struct t_optmgmt * 	req,
	struct t_optmgmt * 	ret)
{
	char *			buf;
	char			stack_buf[TLI_STACK_BUF_SIZE];
	struct T_optmgmt_req *	toptmgmtr;
	struct T_optmgmt_ack *	toptmgmta;
	struct	tli_st *	tli;
	int			total_len;
        int     		code;

        code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY))) {
		goto badfd;
	}
#ifdef	XTI
	if (tli->tlis_state == T_UNINIT)
#else
	if (tli->tlis_state == T_UNBND)
#endif
	{
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}

	total_len = req ? req->opt.len : 0;
	if (ret  &&  ret->opt.maxlen > total_len)
		total_len = ret->opt.maxlen;
	total_len += sizeof(struct T_optmgmt_req);

	if (total_len > sizeof(stack_buf)) {
		if (!(buf = (char *)malloc(total_len))) {
			_Set_terrno(TSYSERR);
			_Seterrno(ENOMEM);
			goto rtn;
		}
	} else
		buf = stack_buf;

	toptmgmtr = (struct T_optmgmt_req *)&buf[0];
	toptmgmtr->PRIM_type = T_OPTMGMT_REQ;
	if (req) {
		toptmgmtr->MGMT_flags = req->flags;
		toptmgmtr->OPT_length = req->opt.len;
		toptmgmtr->OPT_offset = (char *)&toptmgmtr[1] - (char *)toptmgmtr;
		memcpy((char *)&toptmgmtr[1], req->opt.buf, req->opt.len);
	} else {
		toptmgmtr->MGMT_flags = 0;
		toptmgmtr->OPT_length = 0;
		toptmgmtr->OPT_offset = 0;
	}
	if (tli_ioctl(fd, TI_OPTMGMT, (char *)toptmgmtr, total_len) == -1)
		goto rtn;
	if (ret) {
		toptmgmta = (struct T_optmgmt_ack *)&buf[0];
		ret->flags = toptmgmta->MGMT_flags;
		if (toptmgmta->OPT_length > 0  &&  ret->opt.maxlen >= 0) {
			if (ret->opt.maxlen < toptmgmta->OPT_length) {
				_Set_terrno(TBUFOVFLW);
				goto rtn;
			}
			ret->opt.len = toptmgmta->OPT_length;
			memcpy(ret->opt.buf, &buf[toptmgmta->OPT_offset], ret->opt.len);
		} else
			ret->opt.len = 0;
	}
	TLI_NEXTSTATE(tli, TLI_OPTMGMT);
        code = 0;
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_optmgmt (fd, req, ret, code);
#endif

badfd:
	return code;
}
