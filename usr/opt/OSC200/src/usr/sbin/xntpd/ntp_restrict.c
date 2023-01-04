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
static char     *sccsid = "@(#)$RCSfile: ntp_restrict.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 16:58:20 $";
#endif
/*
 */

/*
 * ntp_restrict.c - find out what restrictions this host is running under
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifdef convex
#include "/sys/sync/queue.h"
#include "/sys/sync/sema.h"
#endif
#include <net/if.h>
#include <netinet/in.h>

#include <ntp/ntp_syslog.h>
#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>


/*
 * This code keeps a simple address-and-mask list of hosts we want
 * to place restrictions on (or remove them from).  The restrictions
 * are implemented as a set of flags which tell you what the host
 * can't do.  There is a subroutine entry to return the flags.  The
 * list is kept sorted to reduce the average number of comparisons
 * and make sure you get the set of restrictions most specific to
 * the address.
 *
 * The algorithm is that, when looking up a host, it is first assumed
 * that the default set of restrictions will apply.  It then searches
 * down through the list.  Whenever it finds a match it adopts the match's
 * flags instead.  When you hit the point where the sorted address is
 * greater than the target, you return with the last set of flags you
 * found.  Because of the ordering of the list, the most specific match
 * will provide the final set of flags.
 *
 * This was originally intended to restrict you from sync'ing to your
 * own broadcasts when you are doing that, by restricting yourself
 * from your own interfaces.  It was also thought it would sometimes
 * be useful to keep a misbehaving host or two from abusing your primary
 * clock.  It has been expanded, however, to suit the needs of those
 * with more restrictive access policies.
 */

/*
 * Memory allocation parameters.  We allocate INITRESLIST entries
 * initially, and add INCRESLIST entries to the free list whenever
 * we run out.
 */
#define	INITRESLIST	10
#define	INCRESLIST	5

/*
 * The restriction list
 */
struct restrictlist *restrictlist;
int restrictcount;	/* count of entries in the restriction list */

/*
 * The free list and associated counters.  Also some uninteresting
 * stat counters.
 */
struct restrictlist *resfree;
int numresfree;		/* number of structures on free list */

u_int res_calls;
u_int res_found;
u_int res_not_found;
u_int res_timereset;

/*
 * Our initial allocation of list entries.
 */
struct restrictlist resinit[INITRESLIST];

/*
 * Imported from the timer module
 */
extern u_int current_time;


/*
 * init_restrict - initialize the restriction data structures
 */
void
init_restrict()
{
	register int i;

	/*
	 * Zero the list and put all but one on the free list
	 */
	resfree = 0;
	bzero((char *)resinit, sizeof resinit);

	for (i = 1; i < INITRESLIST; i++) {
		resinit[i].next = resfree;
		resfree = &resinit[i];
	}

	numresfree = INITRESLIST-1;

	/*
	 * Put the remaining item at the head of the
	 * list as our default entry.  Everything in here
	 * should be zero for now.
	 */
	resinit[0].addr = INADDR_ANY;
	resinit[0].mask = 0;
	restrictlist = &resinit[0];
	restrictcount = 1;


	/*
	 * fix up stat counters
	 */
	res_calls = 0;
	res_found = 0;
	res_not_found = 0;
	res_timereset = 0;
}


/*
 * restrictions - return restrictions for this host
 */
int
restrictions(srcadr)
	struct sockaddr_in *srcadr;
{
	register struct restrictlist *rl;
	register struct restrictlist *match;
	register u_int hostaddr;
	register int isntpport;

	res_calls++;
	/*
	 * We need the host address in host order.  Also need to know
	 * whether this is from the ntp port or not.
	 */
	hostaddr = SRCADR(srcadr);
	isntpport = (SRCPORT(srcadr) == NTP_PORT);

	/*
	 * Set match to first entry, which is default entry.  Work our
	 * way down from there.
	 */
	match = restrictlist;

	for (rl = match->next; rl != 0 && rl->addr <= hostaddr; rl = rl->next)
		if ((hostaddr & rl->mask) == rl->addr) {
			if ((rl->mflags & RESM_NTPONLY) && !isntpport)
					continue;
			match = rl;
		}

	match->count++;
	if (match == restrictlist)
		res_not_found++;
	else
		res_found++;
	
	return (int)match->flags;
}


/*
 * restrict - add/subtract/manipulate entries on the restrict list
 */
void
restrict(op, resaddr, resmask, mflags, flags)
	int op;
	struct sockaddr_in *resaddr;
	struct sockaddr_in *resmask;
	int mflags;
	int flags;
{
	register u_int addr;
	register u_int mask;
	register struct restrictlist *rl;
	register struct restrictlist *rlprev;
	int i;
	extern char *emalloc();

	/*
	 * Get address and mask in host byte order
	 */
	addr = SRCADR(resaddr);
	mask = SRCADR(resmask);
	addr &= mask;		/* make sure low bits are zero */

	/*
	 * If this is the default address, point at first on list.  Else
	 * go searching for it.
	 */
	if (addr == INADDR_ANY) {
		rlprev = 0;
		rl = restrictlist;
	} else {
		rlprev = restrictlist;
		rl = rlprev->next;
		while (rl != 0) {
			if (rl->addr > addr) {
				rl = 0;
				break;
			} else if (rl->addr == addr) {
				if (rl->mask == mask) {
					if ((mflags & RESM_NTPONLY)
					    == (rl->mflags & RESM_NTPONLY))
						break;	/* exact match */
					if (!(mflags & RESM_NTPONLY)) {
						/*
						 * No flag fits before flag
						 */
						rl = 0;
						break;
					}
					/* continue on */
				} else if (rl->mask > mask) {
					rl = 0;
					break;
				}
			}
			rlprev = rl;
			rl = rl->next;
		}
	}
	/*
	 * In case the above wasn't clear :-), either rl now points
	 * at the entry this call refers to, or rl is zero and rlprev
	 * points to the entry prior to where this one should go in
	 * the sort.
	 */

	/*
	 * Switch based on operation
	 */
	switch (op) {
	case RESTRICT_FLAGS:
		/*
		 * Here we add bits to the flags.  If this is a new
		 * restriction add it.
		 */
		if (rl == 0) {
			if (numresfree == 0) {
				rl = (struct restrictlist *) emalloc(
				    INCRESLIST*sizeof(struct restrictlist));
				bzero((char *)rl,
				    INCRESLIST*sizeof(struct restrictlist));

				for (i = 0; i < INCRESLIST; i++) {
					rl->next = resfree;
					resfree = rl;
					rl++;
				}
				numresfree = INCRESLIST;
			}

			rl = resfree;
			resfree = rl->next;
			numresfree--;

			rl->addr = addr;
			rl->mask = mask;
			rl->mflags = (u_short)mflags;

			rl->next = rlprev->next;
			rlprev->next = rl;
			restrictcount++;
		}
		rl->flags |= (u_short)flags;
		break;
	
	case RESTRICT_UNFLAG:
		/*
		 * Remove some bits from the flags.  If we didn't
		 * find this one, just return.
		 */
		if (rl != 0)
			rl->flags &= (u_short)~flags;
		break;
	
	case RESTRICT_REMOVE:
		/*
		 * Remove an entry from the table entirely if we found one.
		 * Don't remove the default entry and don't remove an
		 * interface entry.
		 */
		if (rl != 0
		    && rl->addr != INADDR_ANY
		    && !(rl->mflags & RESM_INTERFACE)) {
			rlprev->next = rl->next;
			restrictcount--;
			bzero((char *)rl, sizeof(struct restrictlist));

			rl->next = resfree;
			resfree = rl;
			numresfree++;
		}
		break;

	default:
		/* Oh, well */
		break;
	}

	/* done! */
}
