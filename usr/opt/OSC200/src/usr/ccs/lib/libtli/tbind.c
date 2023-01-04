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
static char rcsid[] = "@(#)$RCSfile: tbind.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/08/18 21:51:16 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** tbind.c 1.3, last change 10/16/89
 **/

#include <tli/common.h>
#include <sys/stream.h>
#include <tli/tihdr.h>
#include <tli/timod.h>

int
t_bind (
	int			fd,
	struct t_bind * 	req,
	struct t_bind * 	ret)
{
	char *			buf;
	char			stack_buf[TLI_STACK_BUF_SIZE];
	struct T_bind_req *	tbindr;
	struct T_bind_ack *	tbinda;
	int			total_len;
	struct	tli_st *	tli;
	int			code;

	code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY))) {
		goto badfd;
	}
	if (tli->tlis_state != T_UNBND) {
		_Set_terrno(TOUTSTATE);
		goto rtn;
	}
		
	total_len = req ? req->addr.len : 0;
	if (ret  &&  ret->addr.maxlen > total_len)
		total_len = ret->addr.maxlen;
	total_len += sizeof(struct T_bind_req);
	if (total_len > sizeof(stack_buf)) {
		if (!(buf = (char *)malloc(total_len))) {
			_Set_terrno(TSYSERR);
			_Seterrno(ENOMEM);
			goto rtn;
		}
	} else
		buf = stack_buf;

	tbindr = (struct T_bind_req *)buf;
	tbindr->PRIM_type = T_BIND_REQ;
	if (req) {
		tbindr->CONIND_number = req->qlen;
		tbindr->ADDR_length = req->addr.len;
		tbindr->ADDR_offset = (char *)&tbindr[1] - (char *)tbindr;
		memcpy((char *)&tbindr[1], req->addr.buf, req->addr.len);
	} else {
		tbindr->CONIND_number = 0;
		tbindr->ADDR_length = 0;
		tbindr->ADDR_offset = 0;
	}
	if (tli_ioctl(fd, TI_BIND, (char *)tbindr, total_len) == -1)
		goto rtn;
	
	if (ret) {
		tbinda = (struct T_bind_ack *)buf;
		ret->qlen = tbinda->CONIND_number;
		if (tbinda->ADDR_length > 0  &&  ret->addr.maxlen > 0) {
			if (ret->addr.maxlen < tbinda->ADDR_length) {
				_Set_terrno(TBUFOVFLW);
				goto rtn1;
			}
			ret->addr.len = tbinda->ADDR_length;
			memcpy(ret->addr.buf, &buf[tbinda->ADDR_offset], ret->addr.len);
		} else {
			ret->addr.len = 0;
#ifndef XTI
			/*
			 * Although XPG3 doesn't say so, it implies that if
			 * "ret" is specified, then you must receive the
			 * addr component as well.  So setting the maxlen of 0
			 * is treated as falling short of the available length
			 * and hence TBUFOVFLW error
			 *
			 * For TLI generate this error even if maxlen = 0.
			 *
			 */
			if (ret->addr.maxlen <= 0) {
				_Set_terrno(TBUFOVFLW);
				goto rtn1;
			}
#endif /* XTI */
			
		}
	}
	code = 0;
rtn1:
	TLI_NEXTSTATE(tli, TLI_BIND);
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_bind (fd, req, ret, code);
#endif

badfd:
	return code;
}
