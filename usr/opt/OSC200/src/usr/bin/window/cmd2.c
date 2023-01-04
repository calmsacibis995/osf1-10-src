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
static char	*sccsid = "@(#)$RCSfile: cmd2.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/14 13:14:34 $";
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
 * 	cmd2.c	3.36 (Berkeley) 6/29/88
 */


#include "defs.h"

#define LINELENGTH 	80	/* max number od chars in a helpline */

char *shortcmd_default[] = {
	"#       Select window # and return to conversation mode",
	"%#      Select window # but stay in command mode",
	"escape  Return to conversation mode without changing window",
	"^^      Return to conversation mode and change to previous window",
	"c#      Close window #",
	"w       Open a new window",
	"m#      Move window #",
	"M#      Move window # to its previous position",
	"s#      Change the size of window #",
	"S#      Change window # to its previous size",
	"^Y      Scroll up one line",
	"^E      Scroll down one line",
	"^U      Scroll up half a window",
	"^D      Scroll down half a window",
	"^B      Scroll up a full window",
	"^F      Scroll down a full window",
	"h       Move cursor left",
	"j       Move cursor down",
	"k       Move cursor up",
	"l       Move cursor right",
	"^S      Stop output in current window",
	"^Q      Restart output in current window",
	"^L      Redraw screen",
	"^Z      Suspend",
	"q       Quit",
	":       Enter a long command",
	0
};

#define NSHRT ((SHELP_LAST - SHELP01) + 1)
static char help_shortcmd[NSHRT+1][LINELENGTH];

char *longcmd_default[] = {
	":alias name string ...  Make `name' an alias for `string ...'",
	":alias                  Show all aliases",
	":close # ...            Close windows",
	":close all              Close all windows",
	":cursor modes           Set the cursor modes",
	":echo # string ...      Print `string ...' in window #",
	":escape c               Set escape character to `c'",
	":foreground # flag      Make # a foreground window, if `flag' is true",
	":label # string         Set label of window # to `string'",
	":list                   List all open windows",
	":nline lines            Set default window buffer size to `lines'",
	":select #               Select window #",
	":shell string ...       Set default shell program to `string ...'",
	":smooth # flag          Set window # to smooth scroll mode",
	":source filename        Execute commands in `filename'",
	":terse flag             Set terse mode",
	":unalias name           Undefine `name' as an alias",
	":unset variable         Deallocate `variable'",
	":variable               List all variables",
	":window [row col nrow ncol nline label pty frame mapnl keepopen smooth shell]",
	"                        Open a window at `row', `col' of size `nrow', `ncol',",
	"                        with `nline' lines in the buffer, and `label'",
	":write # string ...     Write `string ...' to window # as input",
	0
};

#define NLONG ((LHELP_LAST - LHELP01) + 1)
static char help_longcmd[NLONG+1][LINELENGTH];


c_help()
{
	register struct ww *w;
	int count;
	static int read_messages=0;

	if (!read_messages) {
		for (count=0; count< NSHRT; count++)
			strcpy(help_shortcmd[count], 
				MSGSTR(SHELP01+count, shortcmd_default[count]));
		help_shortcmd[NSHRT][0] = NULL;

		for (count=0; count < NLONG; count++)
			strcpy(help_longcmd[count],
				MSGSTR(LHELP01+count, longcmd_default[count]));
		help_longcmd[NLONG][0] = NULL;
		read_messages=1;
	}

	if ((w = openiwin(wwnrow - 3, MSGSTR(HELP, "Help"))) == 0) {
		error(MSGSTR(CANTOPENHELP, "Can't open help window: %s."), wwerror());
		return;
	}
	wwprintf(w, MSGSTR(ESCCHAR, "The escape character is %c.\n"), escapec);
	wwprintf(w, MSGSTR(NUMEXP, "(# represents one of the digits from 1 to 9.)\n\n"));
	if (help_print(w, MSGSTR(SHRTCMDS, "Short commands"), help_shortcmd) >= 0)
		(void) help_print(w, MSGSTR(LONGCMDS, "Long commands"), help_longcmd);
	closeiwin(w);
}

help_print(w, name, list)
register struct ww *w;
char *name;
register char *list;
{
	wwprintf(w, "%s:\n\n", name);
	while (*list)
		switch (more(w, 0)) {
		case 0:
			wwputs(list, w);
			wwputc('\n', w);
			list += LINELENGTH;		/* next line */
			break;
		case 1:
			wwprintf(w, MSGSTR(CONT, "%s: (continued)\n\n"), name);
			break;
		case 2:
			return -1;
		}
	return more(w, 1) == 2 ? -1 : 0;
}

c_quit()
{
	char oldterse = terse;
	char ans[2];

	setterse(0);
	wwputs(MSGSTR(REALLYQ, "Really quit [yn]? "), cmdwin);
	wwcurtowin(cmdwin);
	while (wwpeekc() < 0)
		wwiomux();
	ans[0] = wwgetc();	/* Take 1-char response, and	*/
	ans[1] = '\0';		/* build a string for NLyesno	*/

	if (NLyesno(ans) == 1) {
		wwputs(MSGSTR(YES, "Yes"), cmdwin);
		quit++;
	} else
		wwputc('\n', cmdwin);
	setterse(!quit && oldterse);
}
