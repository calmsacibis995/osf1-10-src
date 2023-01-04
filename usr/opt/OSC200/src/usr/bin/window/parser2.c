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
static char	*sccsid = "@(#)$RCSfile: parser2.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:11:26 $";
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
 * 	parser2.c	3.12 (Berkeley) 6/29/88";
 */


#include "parser.h"
#include "var.h"
#include "lcmd.h"
#include "alias.h"

/*
 * name == 0 means we don't have a function name but
 * want to parse the arguments anyway.  flag == 0 in this case.
 */
p_function(name, v, flag)
char *name;
register struct value *v;
{
	struct value t;
	register struct lcmd_tab *c = 0;
	register struct alias *a = 0;
	register struct lcmd_arg *ap;		/* this arg */
	struct lcmd_arg *lp = 0;		/* list arg */
	register i;
	struct value av[LCMD_NARG + 1];
	register struct value *vp;

	if (name != 0)
		if (c = lcmd_lookup(name))
			name = c->lc_name;
		else if (a = alias_lookup(name))
			name = a->a_name;
		else {
			p_error(MSGSTR(UNKNOWN, "%s: No such command or alias."), name);
			flag = 0;
		}

	for (vp = av; vp < &av[LCMD_NARG + 1]; vp++)
		vp->v_type = V_ERR;

	if (token == T_LP)
		(void) s_gettok();
	i = 0;
	for (;;) {
		ap = 0;
		vp = 0;
		if (token == T_COMMA)		/* null argument */
			t.v_type = V_ERR;
		else {
			if (p_expr0(&t, flag) < 0)
				break;
			if (t.v_type == V_ERR)
				flag = 0;
		}
		if (token != T_ASSIGN) {
			if (i >= LCMD_NARG ||
			    c != 0 && (ap = lp) == 0 &&
			    (ap = c->lc_arg + i)->arg_name == 0) {
				p_error(MSGSTR(TOOMANY, "%s: Too many arguments."), name);
				flag = 0;
			} else
				vp = &av[i++];
		} else {
			char *tmp;
			if (p_convstr(&t) < 0)
				goto abort;
			tmp = t.v_type == V_STR ? t.v_str : 0;
			(void) s_gettok();
			if (p_expr(&t, flag) < 0) {
				if (tmp)
					str_free(tmp);
				p_synerror();
				goto abort;
			}
			if (t.v_type == V_ERR)
				flag = 0;
			if (tmp) {
				if (c == 0) {
					/* an aliase */
					p_error(MSGSTR(BADSYN, "%s: Bad alias syntax."), name);
					flag = 0;
				} else {
					for (ap = c->lc_arg, vp = av;
					     ap != 0 && ap->arg_name != 0 &&
					     (*ap->arg_name == '\0' ||
					      !str_match(tmp, ap->arg_name,
							ap->arg_minlen));
					     ap++, vp++)
						;
					if (ap == 0 || ap->arg_name == 0) {
						p_error(MSGSTR(UNKNOWNARG, "%s: Unknown argument \"%s\"."),
							name, tmp);
						flag = 0;
						ap = 0;
						vp = 0;
					}
				}
				str_free(tmp);
			}
		}
		if (ap != 0) {
			if (ap->arg_flags & ARG_LIST) {
				i = vp - av + 1;
				lp = ap;
			}
			if (vp->v_type != V_ERR) {
				if (*ap->arg_name)
					p_error(MSGSTR(DUPARG, "%s: Argument %d (%s) duplicated."),
						name, vp - av + 1,
						ap->arg_name);
				else
					p_error(MSGSTR(DUPARG2, "%s: Argument %d duplicated."),
						name, vp - av + 1);
				flag = 0;
				vp = 0;
			} else if (t.v_type == V_ERR) {
				/* do nothing */
			} else if ((ap->arg_flags&ARG_TYPE) == ARG_NUM &&
				   t.v_type != V_NUM ||
				   (ap->arg_flags&ARG_TYPE) == ARG_STR &&
				   t.v_type != V_STR) {
				if (*ap->arg_name)
					p_error(MSGSTR(TYPEER, "%s: Argument %d (%s) type mismatch."),
						name, vp - av + 1,
						ap->arg_name);
				else
					p_error(MSGSTR(TYPEERR2, "%s: Argument %d type mismatch."),
						name, vp - av + 1);
				flag = 0;
				vp = 0;
			}
		}
		if (vp != 0)
			*vp = t;
		else
			val_free(t);
		if (token == T_COMMA)
			(void) s_gettok();
	}

	if (p_erred())
		flag = 0;
	if (token == T_RP)
		(void) s_gettok();
	else if (token != T_EOL && token != T_EOF)
		flag = 0;		/* look for legal follow set */
	v->v_type = V_ERR;
	if (flag)
		if (c != 0)
			(*c->lc_func)(v, av);
		else
			if (a->a_flags & A_INUSE)
				p_error(MSGSTR(RECURALIAS, "%s: Recursive alias."), a->a_name);
			else {
				a->a_flags |= A_INUSE;
				if (dolongcmd(a->a_buf, av, i) < 0)
					p_memerror();
				a->a_flags &= ~A_INUSE;
			}
	if (p_abort()) {
		val_free(*v);
		v->v_type = V_ERR;
		goto abort;
	}
	for (vp = av; vp < &av[LCMD_NARG]; vp++)
		val_free(*vp);
	return 0;
abort:
	for (vp = av; vp < &av[LCMD_NARG]; vp++)
		val_free(*vp);
	return -1;
}

p_assign(name, v, flag)
char *name;
struct value *v;
char flag;
{
	(void) s_gettok();

	if (p_expr(v, flag) < 0) {
		p_synerror();
		return -1;
	}
	switch (v->v_type) {
	case V_STR:
	case V_NUM:
		if (flag && var_set(name, v) == 0) {
			p_memerror();
			val_free(*v);
			return -1;
		}
		break;
	}
	return 0;
}
