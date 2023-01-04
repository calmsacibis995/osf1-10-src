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
static char rcsid[] = "@(#)$RCSfile: truderr.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:21:56 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** truderr.c 1.5, last change 1/29/90
 **/

#include <sys/stream.h>
#include <tli/common.h>
#include <stropts.h>
#include <tli/tihdr.h>
#ifdef XTI
#include <tli/timod.h>
#endif

int
t_rcvuderr (
	int			fd,
	struct t_uderr * 	uderr)
{
	struct T_uderror_ind *	tudei;
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	int			iflags;
	int			ret;
        int     		code;
        int     		event;
	struct tli_st *		tli;

        code = -1;
	if (!(tli = iostate_sw(fd, IOSTATE_VERIFY)))
		goto badfd;

	if (tli->tlis_servtype != T_CLTS ) {
		_Set_terrno(TNOTSUPPORT);
		goto rtn;
	}
	event = TLI_ILOOK(fd, tli);	
	if (event != T_UDERR) {
		switch (event) {
#ifndef XTI
                case T_ERROR:
                        _Set_terrno(TSYSERR);
                        break;
#endif
                default:
                        _Set_terrno(TNOUDERR);
                        break;
		}
                goto rtn;
        }


	switch (t_get_primitive(fd, tli, T_UDERROR_IND,
	       (int)sizeof(struct T_uderror_ind), nilp(struct netbuf))) {
	case -1:
		if ((_Get_terrno() == TNODATA)  || (_Get_terrno() == TLOOK))
			_Set_terrno(TNOUDERR);
		goto rtn;
	case TBUFOVFLW:
		goto rtn;
	case 0:
	default:
		break;
	}

#ifdef XTI
	tli_ioctl(fd, TI_XTI_CLEAR_EVENT, nilp(char), 0);
#endif
	if (uderr) {
		tudei = (struct T_uderror_ind *)tli->tlis_proto_buf;
		if (uderr->addr.maxlen <= 0) {
			_Set_terrno(TBUFOVFLW);
			goto rtn;
		}
		uderr->error = tudei->ERROR_type;
		if (tudei->DEST_length > 0  &&  uderr->addr.maxlen > 0) {
			if (uderr->addr.maxlen < tudei->DEST_length) {
				_Set_terrno(TBUFOVFLW);
				goto rtn;
			}
			uderr->addr.len = tudei->DEST_length;
			memcpy(uderr->addr.buf, &tli->tlis_proto_buf[tudei->DEST_offset], uderr->addr.len);
		} else
			uderr->addr.len = 0;
		if (tudei->OPT_length > 0  &&  uderr->opt.maxlen > 0) {
			if (uderr->opt.maxlen < tudei->OPT_length) {
				_Set_terrno(TBUFOVFLW);
				goto rtn;
			}
			uderr->opt.len = tudei->OPT_length;
			memcpy(uderr->opt.buf, &tli->tlis_proto_buf[tudei->OPT_offset], uderr->opt.len);
		} else
			uderr->opt.len = 0;
	}
	code = 0;
rtn:
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_rcvuderr (fd, uderr, code);
#endif

badfd:
	return code;
}
