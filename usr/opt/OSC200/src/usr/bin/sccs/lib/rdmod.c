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
static char	*sccsid = "@(#)$RCSfile: rdmod.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:49:32 $";
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
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: readmod, addq, remq, setkeep, apply, chkix, msg
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* rdmod.c 1.6 com/cmd/sccs/lib/comobj,3.1,9021 1/5/90 10:56:38"; */

# include	"defines.h"
# define msg(s,help)	fprintf(pkt->p_stdout,MSGCO(INEXCNFLT, msgstr),s,pkt->p_glnno,help)

#define msgstr "An include or exclude conflict %1$s at line %u (%2$s)\n"

char    *fmalloc();

readmod(pkt)
register struct packet *pkt;
{
	extern	char	*satoi();
	char *getline();
	register char *p;
	int tmpvar;
	int ser;
	int iord;
	int oldixmsg;
	register struct apply *ap;

	oldixmsg = pkt->p_ixmsg;
	while (getline(pkt) != NULL) {
		p = pkt->p_line;
		if (*p++ != CTLCHAR) {
			if (pkt->p_keep == YES) {
				pkt->p_glnno++;
				if (pkt->p_verbose) {
					if (pkt->p_ixmsg && oldixmsg == 0) {
						msg(MSGCO(BEGINS,"begins"),"co25");
					}
					else if (pkt->p_ixmsg == 0 && oldixmsg) {
						msg(MSGCO(ENDS,"ends"),"co26");
					}
				}
				return(1);
			}
		}
		else {
			if (!((iord = *p++) == INS || iord == DEL || iord == END))
				fmterr(pkt);
			NONBLANK(p);
			satoi(p,&ser);
			if (!(ser > 0 && ser <= maxser(pkt)))
				fmterr(pkt);
			if (iord == END)
				remq(pkt,ser);
			else if ((ap = &pkt->p_apply[ser])->a_code == APPLY)
			{
				if (iord == INS) 
					tmpvar = YES;
				else tmpvar = NO;
				addq(pkt,ser,tmpvar,iord,
							ap->a_reason & USER);
			}
			else
			{
				if (iord == INS)
					tmpvar = NO;
				else tmpvar = NULL;
				addq(pkt,ser,tmpvar,iord,
							ap->a_reason & USER);
			}
		}
	}
	if (pkt->p_q)
		fatal(MSGCO(PRMTREOF, 
                    "\nThe end of the file was premature.(co5)\n"));  /* MSG */
	return(0);
}


addq(pkt,ser,keep,iord,user)
struct packet *pkt;
int ser;
int keep;
int iord;
{
	register struct queue *cur, **prev, *q;

	for (prev = &pkt->p_q; cur = *prev; prev = &cur->q_next)
		if (cur->q_sernum <= ser) {
			if (cur->q_sernum == ser)
				fmterr(pkt);
			break;
		}
	*prev = q = (struct queue *) fmalloc(sizeof(*q));
	q->q_next = cur;
	q->q_sernum = ser;
	q->q_keep = keep;
	q->q_iord = iord;
	q->q_user = user;
	if (pkt->p_ixuser && (q->q_ixmsg = chkix(q,pkt->p_q)))
		++(pkt->p_ixmsg);
	else
		q->q_ixmsg = 0;

	setkeep(pkt);
}


remq(pkt,ser)
register struct packet *pkt;
int ser;
{
	register struct queue *cur, **prev;

	for (prev = &pkt->p_q; cur = *prev; prev = &cur->q_next)
		if (cur->q_sernum == ser)
			break;
	if (cur) {
		if (cur->q_ixmsg)
			--(pkt->p_ixmsg);
		*prev = cur->q_next;
		ffree((char *)cur);
		setkeep(pkt);
	}
	else
		fmterr(pkt);
}


setkeep(pkt)
register struct packet *pkt;
{
	register struct queue *q;
	register struct sid *sp;

	for (q = pkt->p_q; q; q = q->q_next)
		if (q->q_keep != NULL) {
			if ((pkt->p_keep = q->q_keep) == YES) {
				sp = &pkt->p_idel[q->q_sernum].i_sid;
				pkt->p_inssid.s_rel = sp->s_rel;
				pkt->p_inssid.s_lev = sp->s_lev;
				pkt->p_inssid.s_br = sp->s_br;
				pkt->p_inssid.s_seq = sp->s_seq;
			}
			return;
		}
	pkt->p_keep = NO;
}


# define apply(qp)	((qp->q_iord == INS &&((unsigned char)  qp->q_keep) == ((char) YES)) || \
				(qp->q_iord == DEL &&((unsigned char)  qp->q_keep) == (((char) NO) & 0377)))

chkix(new,head)
register struct queue *new;
struct queue *head;
{
	register int retval;
	register struct queue *cur;
	int firstins, lastdel;

	if (!apply(new))
		return(0);
	for (cur = head; cur; cur = cur->q_next)
		if (cur->q_user)
			break;
	if (!cur)
		return(0);
	retval = 0;
	firstins = 0;
	lastdel = 0;
	for (cur = head; cur; cur = cur->q_next ) {
		if (apply(cur)) {
			if (cur->q_iord == DEL)
				lastdel = cur->q_sernum;
			else if (firstins == 0)
				firstins = cur->q_sernum;
		}
		else if (cur->q_iord == INS)
			retval++;
	}
	if (retval == 0) {
		if (lastdel && (new->q_sernum > lastdel))
			retval++;
		if (firstins && (new->q_sernum < firstins))
			retval++;
	}
	return(retval);
}
