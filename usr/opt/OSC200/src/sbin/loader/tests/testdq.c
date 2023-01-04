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
static char	*sccsid = "@(#)$RCSfile: testdq.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:46:13 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* testdq.c
 *
 * OSF/1 Release 1.0
 */

#include	<stdio.h>

#include	"dqueue.h"

#define	NELT	20

struct	dqueue_elem	array[NELT];		/* the queue elements */
struct	dqueue_elem	hdr;			/* the queue header */
char *functs = "functions are: <ah>|<at>|<aa>|<ab>|<rh>|<rt>|<d>|<n> <elt> [<prev>]\n";
main (argc, argv)
int	argc;
char	**argv;
{
	char	func[80];
	int	elt, prev;
	struct	dqueue_elem	*e;
	
	dq_init(&hdr);
	for (elt = 0; elt < NELT; elt++)
		dq_init(&array[elt]);
	printf ("%s", functs);
	for (;;) {
		scanf ("%s", func);
		if (strcmp (func, "?") == 0) {
			printf ("%s", functs);
			continue;
		} else if (strcmp (func, "q") == 0) {
			exit (0);
		}
		scanf ("%d", &elt);
		if (elt < 0 || elt >= NELT) {
			printf ("no such element\n");
			continue;
		}
		if (strcmp (func, "ah") == 0) {
			dq_ins_head(&hdr, &array[elt]);
		} else if (strcmp (func, "at") == 0) {
			dq_ins_tail(&hdr, &array[elt]);
		} else if (strcmp (func, "aa") == 0) {
			scanf ("%d", &prev);
			dq_ins_after(&array[elt], &array[prev]);
		} else if (strcmp (func, "ab") == 0) {
			scanf ("%d", &prev);
			dq_ins_before(&array[elt], &array[prev]);
		} else if (strcmp (func, "d") == 0) {
			dq_rem_elem(&array[elt]);
		} else if (strcmp (func, "rh") == 0) {
			e = dq_rem_head(&hdr, struct dqueue_elem *);
			if (e != NULL)
				printf("Removed %d\n", (e - array));
			else
				printf("Queue empty\n");
		} else if (strcmp (func, "rt") == 0) {
			e = dq_rem_tail(&hdr, struct dqueue_elem *);
			if (e != NULL)
				printf("Removed %d\n", (e - array));
			else
				printf("Queue empty\n");
		} else {
			printf ("no such function\n");
			continue;
		}
		show_que(&hdr);
	}
}


show_que(q)
register struct dqueue_elem	*q;
{
	register struct dqueue_elem	*ent;
	register int	len;
	
	for (ent = q->dq_forw, len = 0; ent != q;
	     ent = ent->dq_forw, len++) {
		if (len > NELT) {
			printf("looping, break...\n");
			return;
		}
		printf ("\telement %3d", (ent - array));
		if (ent->dq_back != q)
			printf("  back %3d", (ent->dq_back - array));
		else
			printf("          ");
		if (ent->dq_forw != q)
			printf("  forw %3d\n", (ent->dq_forw - array));
		else
			printf("\n");
	}
}
