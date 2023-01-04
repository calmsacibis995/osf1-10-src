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
static char	*sccsid = "@(#)$RCSfile: wwgets.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:13:45 $";
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
 * wwgets.c	3.12 (Berkeley) 6/29/88
 */


#include "ww.h"
#include "char.h"

static rub();

wwgets(buf, n, w)
char *buf;
int n;
register struct ww *w;
{
	register char *p = buf;
	register char c;
	char uc = w->ww_unctrl;

	w->ww_unctrl = 0;
	for (;;) {
		wwcurtowin(w);
		while ((c = wwgetc()) < 0)
			wwiomux();
#ifndef POSIX_TTY
                if (c == wwoldtty.ww_sgttyb.sg_erase)
#else
                if (c == wwoldtty.ww_termios.c_cc[VERASE])
#endif
		{
			if (p > buf)
				rub(*--p, w);
		} else 
#ifndef POSIX_TTY
                if (c == wwoldtty.ww_sgttyb.sg_kill)
#else
                if (c == wwoldtty.ww_termios.c_cc[VKILL])
#endif
		{
			while (p > buf)
				rub(*--p, w);
		} else 
#ifndef POSIX_TTY
                if (c == wwoldtty.ww_ltchars.t_werasc)
#else
                if (c == wwoldtty.ww_termios.c_cc[VWERASE])
#endif
		{
			while (--p >= buf && (*p == ' ' || *p == '\t'))
				rub(*p, w);
			while (p >= buf && *p != ' ' && *p != '\t')
				rub(*p--, w);
			p++;
		} else if (c == '\r' || c == '\n') {
			break;
		} else {
			if (p >= buf + n - 1)
				wwputc(ctrl('g'), w);
			else
				wwputs(unctrl(*p++ = c), w);
		}
	}
	*p = 0;
	w->ww_unctrl = uc;
}

static
rub(c, w)
struct ww *w;
{
	register i;

	for (i = strlen(unctrl(c)); --i >= 0;)
		(void) wwwrite(w, "\b \b", 3);
}
