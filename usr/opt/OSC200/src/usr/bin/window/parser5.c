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
static char	*sccsid = "@(#)$RCSfile: parser5.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:11:35 $";
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
 * 	parser5.c	3.11 (Berkeley) 6/29/88
 */


#include "parser.h"
#include "var.h"

/*
 * unary $ $? + - ! ~
 */
p_expr11(v, flag)
register struct value *v;
char flag;
{
	int op;
	char *opname;

	switch (token) {
	case T_DOLLAR:
		opname = "$";
		break;
	case T_DQ:
		opname = "$?";
		break;
	case T_PLUS:
		opname = "unary +";
		break;
	case T_MINUS:
		opname = "unary -";
		break;
	case T_NOT:
		opname = "!";
		break;
	case T_COMP:
		opname = "~";
		break;
	default:
		return p_expr12(v, flag);
	}
	op = token;
	(void) s_gettok();
	if (p_expr11(v, flag) < 0)
		return -1;
	switch (v->v_type) {
	case V_NUM:
		break;
	case V_STR:
		switch (op) {
		case T_MINUS:
		case T_NOT:
		case T_COMP:
			p_error(MSGSTR(NUMREQ3, "%s: Numeric operand required."), opname);
			str_free(v->v_str);
			v->v_type = V_ERR;
			return 0;
		}
		break;
	case V_ERR:
		return 0;
	}
	switch (op) {
	case T_DOLLAR:
	case T_DQ:
		if (v->v_type == V_NUM) {
			int tmp = cx.x_type == X_BUF && cx.x_arg != 0 &&
				v->v_num > 0 && v->v_num <= cx.x_narg;
			if (op == T_DQ)
				v->v_num = tmp;
			else if (tmp)
				*v = cx.x_arg[v->v_num - 1];
			else {
				p_error(MSGSTR(NOARG, "%d: No such argument."), v->v_num);
				v->v_type = V_ERR;
			}
		} else {
			char *name = v->v_str;
			struct var *r = var_lookup(name);
			if (op == T_DQ) {
				v->v_type = V_NUM;
				v->v_num = r != 0;
			} else if (r != 0)
				*v = r->r_val;
			else {
				p_error(MSGSTR(UNDEFVAR, "%s: Undefined variable."), name);
				v->v_type = V_ERR;
			}
			str_free(name);
		}
		if (v->v_type == V_STR && (v->v_str = str_cpy(v->v_str)) == 0) {
			p_memerror();
			return -1;
		}
		break;
	case T_MINUS:
		v->v_num = - v->v_num;
		break;
	case T_NOT:
		v->v_num = ! v->v_num;
		break;
	case T_COMP:
		v->v_num = ~ v->v_num;
		break;
	}
	return 0;
}

/*
 * string, number, ( expr )
 * Plus function calls.
 *
 * Always return v_type == V_ERR when flag == 0.
 */
p_expr12(v, flag)
register struct value *v;
char flag;
{
	v->v_type = V_ERR;
	switch (token) {
	case T_NUM:
		if (flag) {
			v->v_type = V_NUM;
			v->v_num = token_num;
		}
		(void) s_gettok();
		break;
	case T_STR:
		if (flag) {
			v->v_type = V_STR;
			v->v_str = token_str;
		} else
			str_free(token_str);
		(void) s_gettok();
		break;
	case T_LP:
		(void) s_gettok();
		if (p_expr(v, flag) < 0) {
			p_synerror();
			return -1;
		}
		if (token != T_RP) {
			p_synerror();
			val_free(*v);
			return -1;
		}
		(void) s_gettok();
		break;
	default:
		return -1;
	}
	while (token == T_LP) {
		char *cmd;

		if (p_convstr(v) < 0)
			return -1;
		cmd = v->v_type == V_STR ? v->v_str : 0;
		if (p_function(cmd, v, flag) < 0) {
			if (cmd)
				str_free(cmd);
			return -1;
		}
		if (cmd)
			str_free(cmd);
	}
	return 0;
}
