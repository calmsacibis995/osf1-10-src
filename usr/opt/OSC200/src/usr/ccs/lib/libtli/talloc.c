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
static char *rcsid = "@(#)$RCSfile: talloc.c,v $ $Revision: 4.2.6.4 $ (DEC) $Date: 1993/11/02 17:35:33 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989  Mentat Inc.
 ** talloc.c 1.4, last change 12/20/89
 **/

#include <tli/common.h>
#include <errno.h>

#ifdef XTI
#include <xti.h>
#define T_BIND          T_BIND_STR
#define T_CALL          T_CALL_STR
#define T_OPTMGMT       T_OPTMGMT_STR
#define T_DIS           T_DIS_STR
#define T_UNITDATA      T_UNITDATA_STR
#define T_UDERROR       T_UDERROR_STR
#define T_INFO          T_INFO_STR
#else
#include <tiuser.h>
#endif

#ifdef XTIDBG
#include <tli/tdbg.h>
#endif

extern	char	* calloc(   int elem_cnt, int elem_size   );


static int
netbuf_alloc (nb, len)
	struct netbuf	* nb;
	long		len;
{
reg	uint ulen = (uint)len;

	if (len < 0L) {
		nb->buf = nilp(char);
		nb->len = 0;
		nb->maxlen = 0;
		return 1;
	}
	if ((long)ulen != len) {
		_Seterrno(EINVAL);
		return 0;
	}
	if (ulen > 0  &&  !(nb->buf = calloc(ulen, (uint)sizeof(char)))) {
		_Seterrno(ENOMEM);
		return 0;
	}
	nb->len = 0;
	nb->maxlen = ulen;
	return 1;
}

char *
t_alloc (fd, struct_type, fields)
	int	fd;
	int	struct_type;
	int	fields;
{
	struct netbuf	* addrp = nilp(struct netbuf);
	struct netbuf	* datap = nilp(struct netbuf);
	long		len;
	struct netbuf	* optp = nilp(struct netbuf);
	struct t_info	tinfo;
	union {
		struct t_bind	* t_b;
		struct t_call	* t_c;
		struct t_info	* t_i;
		struct t_optmgmt * t_o;
		struct t_discon	* t_d;
		struct t_unitdata * t_u;
		struct t_uderr	* t_ud;
	} up;
	char *  code = nilp(char);

	up.t_b = nilp(struct t_bind);
	switch (struct_type) {
	case T_BIND:
		up.t_b = newa(struct t_bind, 1);
		addrp = &up.t_b->addr;
		break;
	case T_CALL:
		up.t_c = newa(struct t_call, 1);
		datap = &up.t_c->udata;
		optp = &up.t_c->opt;
		addrp = &up.t_c->addr;
		break;
	case T_DIS:
		up.t_d = newa(struct t_discon, 1);
		datap = &up.t_d->udata;
		break;
	case T_INFO:
		up.t_i = newa(struct t_info, 1);
		break;
	case T_OPTMGMT:
		up.t_o = newa(struct t_optmgmt, 1);
		optp = &up.t_o->opt;
		break;
	case T_UDERROR:
		up.t_ud = newa(struct t_uderr, 1);
		optp = &up.t_ud->opt;
		addrp = &up.t_ud->addr;
		break;
	case T_UNITDATA:
		up.t_u = newa(struct t_unitdata, 1);
		datap = &up.t_u->udata;
		optp = &up.t_u->opt;
		addrp = &up.t_u->addr;
		break;
	default:
		_Seterrno(EINVAL);
#ifdef XTI
		_Set_terrno(TNOSTRUCTYPE);
#else
		_Set_terrno(TSYSERR);
#endif
		code =  nilp(char);
		goto rtn;
	}
	if (!up.t_b) {
		_Seterrno(ENOMEM);
		_Set_terrno(TSYSERR);
		code = nilp(char);
		goto rtn;
	}
	if (!(fields & T_ALL )) {
		code =  (char *)up.t_b;
		goto rtn;
	}
	if (t_getinfo(fd, &tinfo) != 0) {
		(void)t_free((char *)up.t_b, struct_type);
		code =  nilp(char);
		goto rtn;
	}
	if (((fields == T_ALL) || (fields & T_ADDR))  &&  addrp) {
		if (!netbuf_alloc(addrp, tinfo.addr))
			goto alloc_err;
	}
	if (((fields == T_ALL) || (fields & T_OPT))  &&  optp) {
		if (!netbuf_alloc(optp, tinfo.options))
			goto alloc_err;
	}
	if (((fields == T_ALL) || (fields & T_UDATA))  &&  datap) {
		switch (struct_type) {
		case T_UNITDATA:
			len = tinfo.tsdu;
			break;
		case T_CALL:
			len = tinfo.connect;
			break;
		case T_DIS:
			len = tinfo.discon;
			break;
		default:
			len = -1;
			break;
		}
		if (!netbuf_alloc(datap, len))
			goto alloc_err;
	}
	code = (char *)up.t_b;
	goto rtn;
alloc_err:
	(void)t_free((char *)up.t_b, struct_type);
	_Set_terrno(TSYSERR);
	code = nilp(char);
rtn:
#ifdef XTIDBG
        tr_allocate (fd, struct_type, fields, code);
#endif
	return code;

}
