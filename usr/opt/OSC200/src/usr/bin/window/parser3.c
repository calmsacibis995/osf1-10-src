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
static char	*sccsid = "@(#)$RCSfile: parser3.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:11:29 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	parser3.c	3.7 (Berkeley) 6/29/88";
 */


#include "parser.h"

/*
 * =
 * ? :
 * ||
 * &&
 * |
 * ^
 * &
 * == !=
 * <= >=
 * << >>
 * + -
 * * / %
 * unary - + ~ !
 */
p_expr(v, flag)
register struct value *v;
char flag;
{
	struct value t;
	int ret;

	if (p_expr0(&t, flag) < 0)
		return -1;

	if (token != T_ASSIGN) {
		*v = t;
		return 0;
	}
	switch (t.v_type) {
	case V_NUM:
		p_error(MSGSTR(NOTVAR, "%d: Not a variable."), t.v_num);
	case V_ERR:
		t.v_str = 0;
		break;
	}
	ret = p_assign(t.v_str, v, flag);
	if (t.v_str != 0)
		str_free(t.v_str);
	return ret;
}

/*
 * ? :
 */
p_expr0(v, flag)
register struct value *v;
char flag;
{
	struct value t;
	char true;

	if (p_expr1(v, flag) < 0)
		return -1;
	if (token != T_QUEST)
		return 0;
	switch (v->v_type) {
	case V_NUM:
		true = v->v_num != 0;
		break;
	case V_STR:
		p_error(MSGSTR(NUMLEFT,"?: Numeric left operand required."));
		str_free(v->v_str);
		v->v_type = V_ERR;
	case V_ERR:
		flag = 0;
		break;
	}
	(void) s_gettok();
	v->v_type = V_ERR;
	if ((flag && true ? p_expr1(v, 1) : p_expr1(&t, 0)) < 0)
		return -1;
	if (token != T_COLON) {
		val_free(*v);
		p_synerror();
		return -1;
	}
	(void) s_gettok();
	return flag && !true ? p_expr1(v, 1) : p_expr1(&t, 0);
}

/*
 * ||
 */
p_expr1(v, flag)
register struct value *v;
char flag;
{
	char true = 0;

	if (p_expr2(v, flag) < 0)
		return -1;
	if (token != T_OROR)
		return 0;
	for (;;) {
		switch (v->v_type) {
		case V_NUM:
			v->v_num = true = true || v->v_num != 0;
			break;
		case V_STR:
			p_error(MSGSTR(NUMREQ, "||: Numeric operands required."));
			str_free(v->v_str);
			v->v_type = V_ERR;
		case V_ERR:
			flag = 0;
			break;
		}
		if (token != T_OROR)
			return 0;
		(void) s_gettok();
		if (p_expr2(v, flag && !true) < 0)
			return -1;
	}
}

/*
 * &&
 */
p_expr2(v, flag)
register struct value *v;
char flag;
{
	char true = 1;

	if (p_expr3_10(3, v, flag) < 0)
		return -1;
	if (token != T_ANDAND)
		return 0;
	for (;;) {
		switch (v->v_type) {
		case V_NUM:
			v->v_num = true = true && v->v_num != 0;
			break;
		case V_STR:
			p_error(MSGSTR(NUMREQ2, "&&: Numeric operands required."));
			str_free(v->v_str);
			v->v_type = V_ERR;
		case V_ERR:
			flag = 0;
			break;
		}
		if (token != T_ANDAND)
			return 0;
		(void) s_gettok();
		if (p_expr3_10(3, v, flag && true) < 0)
			return -1;
	}
	/*NOTREACHED*/
}
