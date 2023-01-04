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
static char rcsid[] = "@(#)$RCSfile: print.c,v $ $Revision: 4.2.2.2 $ (OSF) $Date: 1993/08/25 22:26:54 $";
#endif
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * print.c	5.3 (Berkeley) 6/1/90
 */


#include <sys/types.h>
#include <sys/file.h>
#include "ctags.h"

extern char	searchar;		/* ex search character */

/*
 * getline --
 *	get the line the token of interest occurred on,
 *	prepare it for printing.
 */
void
getline(void)
{
	register long	saveftell;
	register int	c,
			cnt;
	register char	*cp;

	saveftell = ftell(inf);
	(void)fseek(inf,lineftell,SEEK_SET);
	if (xflag)
		for (cp = lbuf;GETC(!=,'\n');*cp++ = c);
	/*
	 * do all processing here, so we don't step through the
	 * line more than once; means you don't call this routine
	 * unless you're sure you've got a keeper.
	 */
	else for (cnt = 0,cp = lbuf;GETC(!=,EOF) && cnt < ENDLINE;++cnt) {
		if (c == (int)'\\') {		/* backslashes */
			if (cnt > ENDLINE - 2)
				break;
			*cp++ = '\\'; *cp++ = '\\';
			++cnt;
		}
		else if (c == (int)searchar) {	/* search character */
			if (cnt > ENDLINE - 2)
				break;
			*cp++ = '\\'; *cp++ = c;
			++cnt;
		}
		else if (c == (int)'\n') {	/* end of keep */
			*cp++ = '$';		/* can find whole line */
			break;
		}
		else
			*cp++ = c;
	}
	*cp = EOS;
	(void)fseek(inf,saveftell,SEEK_SET);
}

/*
 * put_entries --
 *	write out the tags
 */
void
put_entries(NODE *node)
{
	extern FILE	*outf;		/* ioptr for tags file */
	extern int	vflag;		/* -v: vgrind style output */

	if (node->left)
		put_entries(node->left);
	if (vflag)
		printf("%s %s %d\n",
		    node->entry,node->file,(node->lno + 63) / 64);
	else if (xflag)
		printf("%-16s%4d %-16s %s\n",
		    node->entry,node->lno,node->file,node->pat);
	else
		fprintf(outf,"%s\t%s\t%c^%s%c\n",
		    node->entry,node->file,searchar,node->pat,searchar);
	if (node->right)
		put_entries(node->right);
}

char *
savestr(char *str)
{
	register u_int	len;
	register char	*space;
	char	*malloc();

	len = strlen(str) + 1;
	if (!(space = malloc((u_int)len))) {
		/*
		 * should probably free up the tree, here,
		 * we're just as likely to fail here as we
		 * are when getting the NODE structure
		 */
		fputs(MSGSTR(NOSPACE, "ctags: no more space.\n"),stderr);
		exit(1);
	}
	bcopy(str,space,len);
	return(space);
}
