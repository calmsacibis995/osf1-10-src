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
static char	*sccsid = "@(#)$RCSfile: permiss.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:49:22 $";
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
 * FUNCTIONS: finduser, doflags, permiss, ck_lock
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
/* permiss.c 1.4 com/cmd/sccs/lib/comobj,3.1,9021 9/15/89 14:12:57"; */

# include	"defines.h"
# include	<grp.h>

finduser(pkt)
register struct packet *pkt;
{
	register char *p;
	char *user, *logname(), *fmalloc();
	char *strend(), *getline();
	int agid, i, ok_user, ngroups, none;
	int *groups = (int *)0;

	none = 1;
	user = logname();
	while ((p = getline(pkt)) != NULL && *p != CTLCHAR) {
		none = 0;
		ok_user = 1;
		repl(p,'\n','\0');	/* this is done for equal test below */
		if(*p == '!') {
			++p;
			ok_user = 0;
			}
		if (!pkt->p_user)
			if (equal(user,p))
				pkt->p_user = ok_user;
			else if (*p >= '0' && *p <= '9') { /* if a group-id number */ 
				int i, agid;
				agid=atoi(p);
				if (!groups) {
					groups = (int *)fmalloc(NGROUPS*sizeof(int));
					ngroups = getgroups(NGROUPS, groups);
				}
				for (i=0; i<ngroups; i++)
					if (agid == groups[i]) {
						pkt->p_user = ok_user;
						break;
					}
			}
		*(strend(p)) = '\n';	/* repl \0 end of line w/ \n again */
	}
	if (groups)
		ffree((char*)groups);
	if (none)
		pkt->p_user = 1;
	if (p == NULL || p[1] != EUSERNAM)
		fmterr(pkt);
}


char	*Sflags[NFLAGS];

doflags(pkt)
struct packet *pkt;
{
	register char *p;
	register int k;
	char *getline(), *fmalloc();

	for (k = 0; k < NFLAGS; k++)
		Sflags[k] = 0;
	while ((p = getline(pkt)) != NULL && *p++ == CTLCHAR && *p++ == FLAG) {
		NONBLANK(p);
		k = *p++ - 'a';
		NONBLANK(p);
		Sflags[k] = fmalloc(size(p));
		copy(p,Sflags[k]);
		for (p = Sflags[k]; *p++ != '\n'; )
			;
		*--p = 0;
	}
}


permiss(pkt)
register struct packet *pkt;
{
	extern char *Sflags[];
	register char *p;
	register int n;

	if (!pkt->p_user)
		fatal(MSGCO(NOTAUTH, 
                  "\nYou are not authorized to make deltas.(co14)\n"));  /* MSG */
	if (p = Sflags[FLORFLAG - 'a']) {
		if (((unsigned)pkt->p_reqsid.s_rel) < (n = patoi(p))) {
		    sprintf(Error,MSGCO(RLSLTFLR,"Release %1$u is less than the lowest allowed release %2$u.(co15)\n"), pkt->p_reqsid.s_rel,n);  /* MSG */
			fatal(Error);
		}
	}
	if (p = Sflags[CEILFLAG - 'a']) {
		if (((unsigned)pkt->p_reqsid.s_rel) > (n = patoi(p))) {
			sprintf(Error,MSGCO(RLSGTCLNG, "Release %1$u is greater than highest allowed release %2$u.(co16)\n"), pkt->p_reqsid.s_rel,n);  /* MSG */
			fatal(Error);
		}
	}
	/*
	check to see if the file or any particular release is
	locked against editing. (Only if the `l' flag is set)
	*/
	if ((p = Sflags[LOCKFLAG - 'a']))
		ck_lock(p,pkt);
}

char l_str[NL_TEXTMAX];

ck_lock(p,pkt)
register char *p;
register struct packet *pkt;
{
	int l_rel;
	int locked;

	sprintf(l_str,MSGCO(FILLCKD, 
             "The SCCS file is locked against editing.(co23)\n"));
	
	locked = 0;
	if (*p == 'a')
		locked++;
	else while(*p) {
		p = satoi(p,&l_rel);
		++p;
		if (l_rel == pkt->p_gotsid.s_rel || l_rel == pkt->p_reqsid.s_rel) {
			locked++;
			sprintf(l_str,MSGCO(RLSLCKD, 
                           "Release %d is locked against editing.(co23)\n"), l_rel);
			break;
		}
	}
	if (locked)
		fatal(l_str);
}
