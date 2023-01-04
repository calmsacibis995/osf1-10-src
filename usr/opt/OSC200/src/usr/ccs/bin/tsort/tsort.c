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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: tsort.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/10/07 19:16:20 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: CMDAOUT (tsort command)
 *
 * FUNCTIONS: anypred, cmp, error, findloop, index, mark, note, present
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.9  com/cmd/aout/tsort/tsort.c, 9121320k, bos320 5/7/91 17:15:31
 */

/*	topological sort
 *	input is sequence of pairs of items (blank-free strings)
 *	nonidentical pair is a directed edge in graph
 *	identical pair merely indicates presence of node
 *	output is ordered list of items consistent with
 *	the partial ordering specified by the graph
*/

#include <stdio.h>
#include <nl_types.h>
#include <locale.h>
#include "tsort_msg.h"

nl_catd  catd;
#define  MSGSTR(Num, Str) catgets(catd, MS_TSORT, Num, Str)

/*	the nodelist always has an empty element at the end to
 *	make it easy to grow in natural order
 *	states of the "live" field:*/
#define DEAD 0	/* already printed*/
#define LIVE 1	/* not yet printed*/
#define VISITED 2	/*used only in findloop()*/

#define	CANT_OPEN " Cannot open the specified file: "
#define	ODD_DATA  " Specify an even number of data fields."
#define	TOO_MANY  " There are too many items for available memory."
#define CYCLE     " The data contains a repeating cycle."
#define	PROG_ERR  " Internal software error."

struct nodelist {
	struct nodelist *nextnode;
	struct predlist *inedges;
	unsigned char *name;
	int live;
} firstnode = {NULL, NULL, NULL, DEAD};

/*	a predecessor list tells all the immediate
 *	predecessors of a given node
*/
struct predlist {
	struct predlist *nextpred;
	struct nodelist *pred;
};

struct nodelist *index();
struct nodelist *findloop();
struct nodelist *mark();
char *malloc();
char *empty = "";

/*	the first for loop reads in the graph,
 *	the second prints out the ordering
*/
main(argc,argv)
char **argv;
{
	register struct predlist *t;
	FILE *input = stdin;
	register struct nodelist *i, *j;
	int x;
	unsigned char precedes[LINE_MAX], follows[LINE_MAX];

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_TSORT, NL_CAT_LOCALE);

	if(argc>1) {
		int i = 1;
		/* Handle XPG4 '--' */
		if (strcmp(argv[1], "--") == 0)
			i = 2;

		if (argv[i]) {
			input = fopen(argv[i],"r");
			if(input==NULL)
			    error(MSGSTR(CANT_OPEN_MSG, CANT_OPEN), argv[i]);
		}
	}
	for(;;) {
		x = fscanf(input,"%s %s",precedes, follows);
		if(x==EOF)
			break;
		if(x!=2)
			error(MSGSTR(ODD_DATA_MSG, ODD_DATA),empty);
		i = index(precedes);
		j = index(follows);
		if(i==j||present(i,j)) 
			continue;
		t = (struct predlist *)malloc(sizeof(struct predlist));
		t->nextpred = j->inedges;
		t->pred = i;
		j->inedges = t;
	}
	for(;;) {
		x = 0;	/*anything LIVE on this sweep?*/
		for(i= &firstnode; i->nextnode!=NULL; i=i->nextnode) {
			if(i->live==LIVE) {
				x = 1;
				if(!anypred(i))
					break;
			}
		}
		if(x==0)
			break;
		if(i->nextnode==NULL)
			i = findloop();
		printf("%s\n",i->name);
		i->live = DEAD;
	}
	exit(0);	/* Ensure zero return on normal termination */
}

/*	is i present on j's predecessor list?
*/
present(i,j)
struct nodelist *i, *j;
{
	register struct predlist *t;
	for(t=j->inedges; t!=NULL; t=t->nextpred)
		if(t->pred==i)
			return(1);
	return(0);
}

/*	is there any live predecessor for i?
*/
anypred(i)
struct nodelist *i;
{
	register struct predlist *t;
	for(t=i->inedges; t!=NULL; t=t->nextpred)
		if(t->pred->live==LIVE)
			return(1);
	return(0);
}

/*	turn a string into a node pointer
*/
struct nodelist *
index(s)
register unsigned char *s;
{
	register struct nodelist *i;
	register unsigned char *t;
	for(i= &firstnode; i->nextnode!=NULL; i=i->nextnode)
		if(cmp(s,i->name))
			return(i);
	for(t=s; *t; t++) ;
	t = (unsigned char *)malloc((unsigned)sizeof(unsigned char) * (t+1-s));
	i->nextnode = (struct nodelist *)malloc(sizeof(struct nodelist));
	if(i->nextnode==NULL||t==NULL)
		error(MSGSTR(TOO_MANY_MSG, TOO_MANY),empty);
	i->name = t;
	i->live = LIVE;
	i->nextnode->nextnode = NULL;
	i->nextnode->inedges = NULL;
	i->nextnode->live = DEAD;
	while(*t++ = *s++);
	return(i);
}

cmp(s,t)
register unsigned char *s, *t;
{
	while(*s==*t) {
		if(*s==0)
			return(1);
		s++;
		t++;
	}
	return(0);
}

error(s,t)
char *s, *t;
{
	note(s,t);
	exit(1);
}

note(s,t)
char *s,*t;
{
	fprintf(stderr,"tsort: %s%s\n",s,t);
}

/*	given that there is a cycle, find some
 *	node in it
*/
struct nodelist *
findloop()
{
	register struct nodelist *i, *j;
	for(i= &firstnode; i->nextnode!=NULL; i=i->nextnode)
		if(i->live==LIVE)
			break;
	note(MSGSTR(CYCLE_MSG, CYCLE),empty);
	i = mark(i);
	if(i==NULL)
		error(MSGSTR(PROG_ERR_MSG, PROG_ERR),empty);
	for(j= &firstnode; j->nextnode!=NULL; j=j->nextnode)
		if(j->live==VISITED)
			j->live = LIVE;
	return(i);
}

/*	depth-first search of LIVE predecessors
 *	to find some element of a cycle;
 *	VISITED is a temporary state recording the
 *	visits of the search
*/
struct nodelist *
mark(i)
register struct nodelist *i;
{
	register struct nodelist *j;
	register struct predlist *t;
	if(i->live==DEAD)
		return(NULL);
	if(i->live==VISITED)
		return(i);
	i->live = VISITED;
	for(t=i->inedges; t!=NULL; t=t->nextpred) {
		j = mark(t->pred);
		if(j!=NULL) {
			note(i->name,empty);
			return(j);
		}
	}
	return(NULL);
}
