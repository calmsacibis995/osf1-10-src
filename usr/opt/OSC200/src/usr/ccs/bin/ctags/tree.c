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
static char rcsid[] = "@(#)$RCSfile: tree.c,v $ $Revision: 4.2.2.2 $ (OSF) $Date: 1993/08/25 22:27:01 $";
#endif
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * tree.c	5.4 (Berkeley) 6/1/90
 */


#include "ctags.h"
#include <strings.h>

void	add_node(NODE *, NODE *);
void	free_tree(NODE *);

/*
 * pfnote --
 *	enter a new node in the tree
 */
void
pfnote(char *name, int ln)
{
	extern NODE	*head;		/* head of the sorted binary tree */
	extern char	*curfile;	/* current input file name */
	register NODE	*np;
	register char	*fp;
	char	nbuf[MAXTOKEN],
		*malloc(), *savestr();

	/*NOSTRICT*/
	if (!(np = (NODE *)malloc(sizeof(NODE)))) {
		fputs(MSGSTR(TOOMANY, 
			"ctags: too many entries to sort\n"),stderr);
		put_entries(head);
		free_tree(head);
		/*NOSTRICT*/
		if (!(head = np = (NODE *)malloc(sizeof(NODE)))) {
			fputs(MSGSTR(NOSPACE2, 
				"ctags: out of space.\n"),stderr);
			exit(1);
		}
	}
	if (!xflag && !strcmp(name,"main")) {
		if (!(fp = rindex(curfile,'/')))
			fp = curfile;
		else
			++fp;
		(void)sprintf(nbuf,"M%s",fp);
		fp = rindex(nbuf,'.');
		if (fp && !fp[2])
			*fp = EOS;
		name = nbuf;
	}
	np->entry = savestr(name);
	np->file = curfile;
	np->lno = ln;
	np->left = np->right = 0;
	np->pat = savestr(lbuf);
	if (!head)
		head = np;
	else
		add_node(np,head);
}

void
add_node(NODE *node, NODE *cur_node)
{
	extern int	wflag;			/* -w: suppress warnings */
	register int	dif;

	dif = strcmp(node->entry,cur_node->entry);
	if (!dif) {
		if (node->file == cur_node->file) {
			if (!wflag)
				fprintf(stderr,MSGSTR(DUPENTRY, 
	"Duplicate entry in file %s, line %d: %s\nSecond entry ignored\n"),
				node->file,lineno,node->entry);
			return;
		}
		if (!cur_node->been_warned)
			if (!wflag)
				fprintf(stderr,MSGSTR(DUPENTRY2,
	"Duplicate entry in files %s and %s: %s (Warning only)\n"),
				node->file,cur_node->file,node->entry);
		cur_node->been_warned = YES;
	}
	else if (dif < 0)
		if (cur_node->left)
			add_node(node,cur_node->left);
		else
			cur_node->left = node;
	else if (cur_node->right)
		add_node(node,cur_node->right);
	else
		cur_node->right = node;
}

void
free_tree(NODE *node)
{
	while (node) {
		if (node->right)
			free_tree(node->right);
		cfree(node);
		node = node->left;
	}
}
