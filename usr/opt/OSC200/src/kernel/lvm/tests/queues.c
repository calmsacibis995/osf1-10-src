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
static char	*sccsid = "@(#)$RCSfile: queues.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:29:47 $";
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "queuedefs.h"
#include "queueproto.h"

#include "alloc.h"
#include "utilproto.h"

char *reasons[] = {
	"Valid Queue",
	"Queue does not exist",
	"Nonzero count for empty queue",
	"Incorrect tail pointer for empty queue",
	"Incorrect tail pointer for non-empty queue",
	"Incorrect count"
	};

int
main(int argc, char **argv)
{
char buffer[256];
char *p;
int n = 0;
int val, c;
struct queue *q;
struct item *I;
int code;

extern int optind;
extern char *optarg;

	if ((q = newqueue()) == NULL) {
		printf("Unable to allocate queue.\n");
		return(1);
	}

	while( printf("Command > "), fgets(buffer, sizeof(buffer), stdin) ) {
		p = skipwhite(buffer);
		c = *p;
		c = isupper(c)?tolower(c):c;
		switch( c ) {
		    case 'a':	printf("Allocating new item and appending.\n");
				addqueue(q, n++);
				break;

		    case 'e':	if (I = fetchqueue(q)) {
					printf("Extracted element: %d\n", 
							I->value);
					release(I);
				} else {
					printf("Extract from empty queue!\n");
				}
				break;
		    case 'n':	flushqueue(q);
				n = 0;
				while (addqueue(q,n++));
				printf("Filled all memory: %d objects\n", n-1);
				n = 0;
				while (I = fetchqueue(q)) {
				    if (n++ != I->value) {
					printf("Retrieved value screwup: %d\n",
						I->value);
				    }
				    release(I);
				}
				flushqueue(q);
				break;

		    case 'p':	printqueue(stdout,q);
				break;

		    case 'v':	if (code = notvalidqueue(q)) {
					printf("Queue not valid: %s\n",
						reasons[code]);
				} else {
					printf("Queue is valid.\n");
				}
				break;

		    default:	printf("Unknown command %c\n", c);
				break;
		}
	}
	printf("\nEOF\n");
	printf("Freeing Queue.\n");
	flushqueue(q);
	release(q);

	return(0);
}

struct queue *
newqueue(void)
{
struct queue *q;

	if (q = new(struct queue)) {
		Q_INIT(q);
	}
	return(q);
}

void
flushqueue(struct queue *q)
{
struct item *I;

	while ((Q_FETCH(q,I), I)) {
		release(I);
	}
	return;
}

int
notvalidqueue(struct queue *q)
{
struct item *I;
int count;

	if (q != NULL) {
		count = q->count;
		if ((I = q->head) != NULL) {
			/* queue is not empty */
			while (I) {
				count--;
				if (((I->next!=NULL)&&(q->tail==&(I->next)))
				  ||((I->next==NULL)&&(q->tail!=&(I->next)))) {
					/* tail pointer pointer points to
					   link that is not in last element */
					return(4);
				}
				I = I->next;
			}
			if (count) {
				/* count is incorrect */
				return(5);
			}
			return(0);
		} else {
			/* queue is empty (according to head) */
			if (q->tail != &(q->head)) {
				/* tail pointer is wrong for empty Q */
				return(2);
			}
			if (count != 0) {
				/* count is incorrect. */
				return(3);
			}
			return(0);
		}
	} else {
		/* queue does not exist */
		return(1);
	}
}

struct item *
fetchqueue(struct queue *q)
{
struct item *I;

	Q_FETCH(q,I);
	return(I);
}

int
addqueue(struct queue *q, int val)
{
struct item *I;

	if (I = new(struct item)) {
		I->next = NULL;
		I->value = val;
	}

	Q_APPEND(q,I);

	return(I != NULL);
}

void
printqueue(FILE *f, struct queue *q)
{
	struct item *I;

	fprintf(f,"Queue contains %d elements:\n", q->count);
	fprintf(f,"Head: 0x%x\n", q->head );
	fprintf(f,"Tail Pointer: 0x%x\n", q->tail );

	I = q->head;
	while (I) {
		fprintf(f,"Element: addr 0x%x, value %d, next 0x%x\n", 
			I, I->value, I->next);
		I = I->next;
	}
	return;
}
