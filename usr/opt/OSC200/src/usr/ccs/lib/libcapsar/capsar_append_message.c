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
/*LINTLIBRARY*/

#ifndef lint
static char *sccsid = "@(#)$RCSfile: capsar_append_message.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/03 13:01:47 $";
#endif lint

/*
 * capsar_append_message.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : capsar_append_message ( MM *m , MM *n )
 *
 * This routine appends the message n to the set of bodyparts on m
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <capsar.h>

capsar_append_message(m,n)
MM	*m;	/* pointer to message structure */
MM	*n;	/* pointer to message structure */
{
/*------*\
  Locals
\*------*/

	MM	*mprev,*m0,
	  	*mmpp,*n0,*nprev,
		*mnew,*mpar,
		*mmp,*mt;
	
	mt=m;
	mmp=m;
	mprev = NULL;
	m0=m->mm_next;

	/* find last body part of message */
	while(m0 != NULL){
		if(m0->parent == mmp){
			mprev = m0;
		}
		m0 = m0->mm_next;
	}

	/* find where to insert new mail messages */
	if(mprev){
		mt = mprev;
		m0 = mprev->mm_next;
	
		while(m0){
			mpar = m0->parent;
			while(!mpar && mpar != mmp)
				mpar = mpar->parent;
			if(!mpar)break;
			mt = m0;
			m0 = m0->mm_next;
		}
	}

	if(m->message_type == SIMPLE_MESSAGE){
		mnew = capsar_new();
		mmpp = mmp->parent;
		if(mmpp)mmpp->mm_next = mnew;
		mnew->message_type = COMPOUND_MESSAGE;
		mnew->parent = mmpp;
		mnew->mm_next = mmp;
		mmp->mm_next = n;
		for(n0=n;n0;n0=n0->mm_next)
			nprev = n0;
		if(m0 != NULL)
			nprev->mm_next = m0;
	}
	else {
		n->parent = mmp;
		for(n0 = n; n0 != NULL; n0 = n0->mm_next)
			nprev = n0;
		if(m0 != NULL)
			nprev->mm_next = m0;
		mt->mm_next = n;
	}
}
