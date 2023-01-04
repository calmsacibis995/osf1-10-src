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
static char	*sccsid = "@(#)$RCSfile: blocktest.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:28:35 $";
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
#include <signal.h>
#include <setjmp.h>

#include "block.h"

#include "alloc.h"
#include "utilproto.h"

#define	b_options	b_driver_un_1.longvalue

extern struct lv_queue *newqueue();
struct buf *work_Q[WORKQ_SIZE];
struct lv_queue *busy_Q;
struct lv_queue *done_Q;
jmp_buf env;
void segvhandler();
struct sigvec sigsegv = { segvhandler, 0, 0 };

int printflag = 0;
int requestcount = 0;
#define	PR_SUMMARY	(1<<0)
#define	PR_VERBOSE	(1<<1)
#define	PR_DEBUG	(1<<2)

#define volatile

int
main(argc, argv)
int argc;
char **argv;
{
char buffer[256];
char *p;
int c;
struct lv_queue *q = NULL;
struct buf *bp;
int blockno, length, rno;
int fromfile = 0;
int sumflag = 0;
int requestno = 0;
	
	initialize();

	if (isatty(fileno(stdin)))
		fromfile = 0;
	else 
		fromfile = 1;

	while ((fromfile || printf("Command > ")),
			fgets(buffer, sizeof(buffer), stdin)) {
		if (setjmp(env)) {
			printf("Fatal error in test program.\n");
		}
		p = skipwhite(buffer);
		c = *p;
		c = isupper(c)?tolower(c):c;
		switch (c) {
		    case 'a':	/* Assert the current state of a request */
				/* A requestno [Waiting|Blocked|Busy|Done] */
				if (p = nextword(p)) {
				    rno = atoi(p);
				} else {
				    printf("Must specify request number.\n");
				    break;
				}
				if ((p = nextword(p)) == NULL) {
				    printf("Must specify state.\n");
				    break;
				}
				assert_state(rno, p);
				break;

		    case 'c':	/* Complete a selected item */
				/* C requestno */
				if (p = nextword(p)) {
				    rno = atoi(p);
				} else {
				    printf("Must specify request number.\n");
				    break;
				}
				complete(rno);
				break;

		    case 'd':	printflag ^= PR_DEBUG;
				printf("Debugging printouts are now ");
				if (printflag&PR_DEBUG)
					printf("on.\n");
				else
					printf("off.\n");
				break;

		    case 'i':	/* Initiate all requests */
				if (!q) {
					printf("No requests to initiate.\n");
					break;
				}
				bp = q->lv_head;
				free(q); q = NULL;
				initiate(bp);
				break;

		    case 'r':	/* Build a request. */
				/* R blockno length */
				if (p = nextword(p)) {
				    blockno = atoi(p);
				} else {
				    printf("Must specify request number.\n");
				    break;
				}
				if (p = nextword(p)) {
				    length = atoi(p);
				} else {
				    length = 512;
				}
				bp = new(struct buf);
				bp->b_blkno = blockno;
				bp->b_bcount = length;
				bp->av_forw = NULL;
				bp->b_options = requestno++;
				if (!q) q = newqueue();
				LV_QUEUE_APPEND(q,bp);
				requestcount++;
				printrequest(bp);
				break;

		    case 's':	/* print summary of queue structures */
				sumflag = PR_SUMMARY;
				/* FALLTHROUGH */

		    case 'p':	/* Print all queue structures */
				/* p [all|requests|work|busy|done] */
				if ((p = nextword(p)) == NULL) {
					p = "all";
				}
				print_all(q,p);
				printflag &= ~sumflag;
				sumflag = 0;
				break;

		    case 'v':	/* verboseness */
				printflag ^= PR_VERBOSE;
				printf("Verbose printouts are now ");
				if (printflag&PR_VERBOSE)
					printf("on.\n");
				else
					printf("off.\n");
				break;

		    case '?':	/* Print help (list of commands) */
				break;

		    default:	printf("Unknown command %c: ? for help.\n", c);
				break;
		}
	}
	printf("\nExiting\n");

	return(0);
}

initialize()
{
	sigvec(SIGSEGV, &sigsegv, (struct sigvec *)NULL);
	busy_Q = newqueue();
	done_Q = newqueue();
	return;
}

assert_state()
{
}

initiate(bp)
struct buf *bp;
{
int hash, error;
struct buf **hashchain;
struct buf *next;
volatile struct lv_queue *q;
jmp_buf saveenv;

	q = newqueue();

	savejmp(saveenv,env);
	if (setjmp(env)) {
		restorejmp(saveenv,env);
		printf("Fatal error. Initiate operation aborted.\n");
		free(q);
		return;
	} 
	for ( ; bp != NULL; bp = next) {
		next = bp->av_forw;
		bp->av_forw = NULL;

		hash = LV_HASH(bp);
		hashchain = &(work_Q[hash]);
		if (error = lv_block(hashchain, q, bp, 0)) {
			printf("lv_block failed: errno %d\n", error);
			continue;
		}
		printf("Initiated ");
		printrequest(bp);
		/* <<INVARIANT>> New items are always added at the
		 * tail of the work_Q hash chains, indicated by the
		 * av_back field set to NULL */
		if (bp->av_back != NULL) {
			printf("Failure: Request not added to work_Q tail.\n");
		}
		/* <<INVARIANT>> Unblocked items are always added to the 'q'
		 * tail */
		if ((bp->b_error != ELBBLOCKED) &&
			(q->lv_tail != &(bp->av_forw))) {
			printf("Failure: Unblocked request not ready\n");
		}
	}
	/* <<INVARIANT>> Items that are placed on the q as 'not blocked'
	 * must not be marked 'blocked' */
	for (bp = q->lv_head; bp != NULL; bp = bp->av_forw) {
		if (bp->b_error == ELBBLOCKED) {
			printf("Failure: Unblocked request marked blocked.\n");
		}
	}
	lv_queue_concat(busy_Q,q);

	free(q);
	return;
}

savejmp(savearea,buf)
jmp_buf savearea, buf;
{
	bcopy(buf, savearea, sizeof(jmp_buf));
}

restorejmp(buf,savearea)
jmp_buf savearea, buf;
{
	bcopy(savearea, buf, sizeof(jmp_buf));
}

#define REQUEST_MASK	(1<<0)
#define WORK_MASK	(1<<1)
#define	BUSY_MASK	(1<<2)
#define	DONE_MASK	(1<<3)

struct pr_opts {
	char *name;
	int bitmask;
} print_opts[] = {
	{ "all",	REQUEST_MASK|WORK_MASK|BUSY_MASK|DONE_MASK },
	{ "requests",	REQUEST_MASK },
	{ "work",	WORK_MASK },
	{ "busy",	BUSY_MASK },
	{ "done",	DONE_MASK },
	{ NULL, 0 }
};

print_all(q,type)
struct lv_queue *q;
char *type;
{
int mask = 0;
struct pr_opts *opts;

	for (;type; type = nextword(type)) {
		for (opts = print_opts; opts->name; opts++) {
			if (strcmp(type,opts->name) == 0)
				mask |= opts->bitmask;
		}
	}
	if (mask & REQUEST_MASK) printrequests(q);
	if (mask & WORK_MASK) printworkq();
	if (mask & BUSY_MASK) printq_items(busy_Q,"busy_Q");
	if (mask & DONE_MASK) printq_items(done_Q,"done_Q");
}

printrequest(bp)
struct buf *bp;
{
	printf("Request [%03d]:", bp->b_options);
	printf(" blkno %d, length %d bytes (%d blocks)",
		bp->b_blkno, bp->b_bcount, bp->b_bcount/512);
	if (bp->b_error==ELBBLOCKED)
		printf(" [blocked]");
	printf("\n");
	if (printflag&PR_VERBOSE) {
		printf("         b_work 0x%08x b_error %d\n",
			bp->b_work, bp->b_error);
		printf("         av_forw: ");
		if (bp->av_forw) {
			printf("req[%03d] ", bp->av_forw->b_options);
		} else {
			printf("NULL ");
		}
		printf(" av_back: ");
		if (bp->av_back) {
			printf("req[%03d]\n", bp->av_back->b_options);
		} else {
			printf("NULL\n");
		}
	}
	return;
}

printworkq()
{
struct buf *bp;
int hash;
int banner = 0;
int count;

	for (hash = 0; hash < WORKQ_SIZE; hash++) {
		if (bp = work_Q[hash]) {
			if (!banner) {
				printf("Work Q contains:\n");
				banner++;
			}
			printf("Hash class %d:", hash);
			if (!(printflag&PR_SUMMARY)) printf("\n");
			count = 0;
			for (; bp != NULL; bp = bp->av_back) {
				if (printflag&PR_SUMMARY)
					count++;
				else
					printrequest(bp);
			}
			if (printflag&PR_SUMMARY)
				printf(" %d items.", count);
			printf("\n");
		}
	}
}

printq_head(q,name)
struct lv_queue *q;
char *name;
{
	printf("%s: 0x%08x\n", name, q);
	printf("%s->lv_head: 0x%08x\n", name, q->lv_head);
	printf("%s->lv_tail: 0x%08x\n", name, q->lv_tail);
	printf("%s->lv_count: %d\n", name, q->lv_count);

	return;
}

printq_items(Q,name)
struct lv_queue *Q;
char *name;
{
struct buf *bp;

	if (printflag&PR_DEBUG)
		printq_head(Q,name);

	if (bp = Q->lv_head) {
		printf("%s contains %d items:\n", name, Q->lv_count);
		if (!(printflag&PR_SUMMARY)) {
			for (; bp != NULL; bp = bp->av_forw) {
				printrequest(bp);
			}
			printf("\n");
		}
	}
	return;
}

printrequests(q)
register struct lv_queue *q;
{
register struct buf *bp;

	if (q && (bp = q->lv_head)) {
		printf("Request Q contains %d items:\n", q->lv_count);
		if (!(printflag&PR_SUMMARY)) {
			for (; bp != NULL; bp = bp->av_forw) {
				printrequest(bp);
			}
		}
		printf("\n");
	}
	return;
}

panic(s)
{
	printf("panic: %s\n");
	longjmp(env,1);
	/* NOTREACHED */
}

void
segvhandler()
{
	printf("segmentation fault.\n");
	longjmp(env,1);
	/* NOTREACHED */
}

void
inthandler()
{
	printf("interrupt received.\n");
	longjmp(env,1);
	/* NOTREACHED */
}

complete(requestno)
int requestno;
{
struct buf *bp, **bpp;
struct buf **hashchain;
volatile struct lv_queue *q;
int hash;
jmp_buf saveenv;
int ltgno;

	savejmp(saveenv,env);
	if (setjmp(env)) {
		restorejmp(saveenv,env);
		printf("Fatal error. Completion operation aborted.\n");
		return;
	}
	/*
	 * Locate the first item in the busy_Q whose beginning block
	 * was given.
	 */

	for (bpp = &(busy_Q->lv_head);
			*bpp != NULL;
			bpp = &((*bpp)->av_forw)) {
		if ((*bpp)->b_options == requestno) break;
	}
	if (*bpp == NULL) {
		printf("Operation failed: request not found on busy_Q\n");
		return;
	}
	/* bpp points to the field that points to the buf to extract */

	bp = *bpp;
	/* bp points to the buf to extract */

	busy_Q->lv_count--;

	/* extract from busy_Q */
	/* If tail pointer points to my link, then I am last. Adjust
	   tail pointer to point the link that pointed to me. */
	if (busy_Q->lv_tail == &(bp->av_forw)) busy_Q->lv_tail = bpp;

	/* adjust the guy who points to me to point to whatever I pointed to */
	*bpp = bp->av_forw;

	/* forget who I pointed to */
	bp->av_forw = NULL;

	if (printflag&PR_DEBUG)
		printq_head(busy_Q,"busy_Q");

	printf("Completing ");
	printrequest(bp);

	hash = LV_HASH(bp);
	hashchain = &(work_Q[hash]);

	q = newqueue();

	lv_unblock(hashchain,q,bp,0);

	/* <<INVARIANT>> Completed request must not be on work_Q */

	requestcount--;
	ltgno = BLK2TRK(bp->b_blkno);

	LV_QUEUE_APPEND(done_Q,bp);
	
	if (bp = q->lv_head) {
		printf("The following requests are now unblocked:\n");
		for (; bp != NULL; bp = bp->av_forw) {
			printrequest(bp);
			/* <<INVARIANT>> Only requests in the same LTG can
			 * be unblocked by a completed request */
			if (ltgno != BLK2TRK(bp->b_blkno)) {
	printf("Failure: request unblocked by different track group\n");
			}
		}
	}
	lv_queue_concat(busy_Q,q);

	free(q);
	return;
}

struct lv_queue *
newqueue()
{
struct lv_queue *q;

	if (q = new(struct lv_queue)) {
		LV_QUEUE_INIT(q);
	}
	return(q);
}

lv_queue_concat(q1,q2)
register struct lv_queue *q1, *q2;
{	/* LV_QUEUE_CONCAT */

	if (q2->lv_tail == &(q2->lv_head))
		/* q2 is empty */
		return;

	*(q1->lv_tail) = q2->lv_head;
	q1->lv_tail    = q2->lv_tail;
	q1->lv_count  += q2->lv_count;

	q2->lv_head = NULL;
	q2->lv_tail = &(q2->lv_head);
	q2->lv_count = 0;

	return;
}
