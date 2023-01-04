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
static char *rcsid = "@(#)$RCSfile: tfree.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/07/30 11:38:01 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
#include <tli/common.h>

#ifdef  XTI
#define T_BIND          T_BIND_STR
#define T_CALL          T_CALL_STR
#define T_OPTMGMT       T_OPTMGMT_STR
#define T_DIS           T_DIS_STR
#define T_UNITDATA      T_UNITDATA_STR
#define T_UDERROR       T_UDERROR_STR
#define T_INFO          T_INFO_STR
#endif

int
t_free (
	char *		ptr,
	int		struct_type)
{
reg	struct netbuf *	addrp = nilp(struct netbuf);
	struct netbuf * optp = nilp(struct netbuf);
	struct netbuf * datap = nilp(struct netbuf);
	union {
		struct t_bind	* t_b;
		struct t_call	* t_c;
		struct t_info	* t_i;
		struct t_optmgmt * t_o;
		struct t_discon	* t_d;
		struct t_unitdata * t_u;
		struct t_uderr	* t_ud;
	} up;
        int    		code;

        code = -1;
	if (!(up.t_b = (struct t_bind *)ptr)) {
	        code = 0;
		goto rtn;
	}
	switch (struct_type) {
	case T_BIND:
		addrp = &up.t_b->addr;
		break;
	case T_CALL:
		datap = &up.t_c->udata;
		optp = &up.t_c->opt;
		addrp = &up.t_c->addr;
		break;
	case T_OPTMGMT:
		optp = &up.t_o->opt;
		break;
	case T_DIS:
		datap = &up.t_d->udata;
		break;
	case T_INFO:
		break;
	case T_UNITDATA:
		datap = &up.t_u->udata;
		optp = &up.t_u->opt;
		addrp = &up.t_u->addr;
		break;
	case T_UDERROR:
		optp = &up.t_ud->opt;
		addrp = &up.t_ud->addr;
		break;
	default:
		_Seterrno(EINVAL);
		_Set_terrno(TSYSERR);
		goto rtn;
	}
	if (datap  &&  datap->buf)
		free(datap->buf);
	if (optp  &&  optp->buf)
		free(optp->buf);
	if (addrp  &&  addrp->buf)
		free(addrp->buf);
	free((char *)up.t_b);
        code = 0;
rtn:

#ifdef XTIDBG
	tr_free (struct_type, code);
#endif
	return code;
}
