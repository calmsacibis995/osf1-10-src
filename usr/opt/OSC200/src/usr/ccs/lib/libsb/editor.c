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
static char	*sccsid = "@(#)$RCSfile: editor.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/11/26 11:43:51 $";
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
/*
 *  editor  --  fork editor to edit some text file
 *
 *  Usage:
 *	i = editor(file, prompt);
 *	char *file, *prompt;
 *	int i;
 *
 *  The editor() routine is used to fork the user's favorite editor.
 *  There is assumed to be an environment variable named "EDITOR" whose
 *  value is the name of the favored editor.  If the EDITOR parameter is
 *  missing, some default (see DEFAULTED below) is assumed.  The runp()
 *  routine is then used to find this editor on the searchlist specified
 *  by the PATH variable (or the default path).  "file" is the name of
 *  the file to be edited and "prompt" is a string (of any length) which
 *  will be printed in a such a way that the user can see it at least at
 *  the start of the editing session.  editor() returns the value of the
 *  runp() call.
 */

#include <stdio.h>

char *getenv();

#define DEFAULTED "vi"

int
editor(file, prompt)
register char *file, *prompt;
{
	register char *editor;

	if ((editor = getenv("EDITOR")) == NULL)
		editor = DEFAULTED;
	if (*prompt) printf("%s\n", prompt);
	return(runp(editor, editor, file, 0));
}
