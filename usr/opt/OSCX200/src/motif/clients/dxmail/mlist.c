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
static char rcs_id[] = "@(#)$RCSfile: mlist.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:58:42 $";
#endif

/*
 *                     Copyright (c) 1987, 1991 by
 *              Digital Equipment Corporation, Maynard, MA
 *                      All rights reserved.
 *
 *   This software is furnished under a license and may be used and
 *   copied  only  in accordance with the terms of such license and
 *   with the  inclusion  of  the  above  copyright  notice.   This
 *   software  or  any  other copies thereof may not be provided or
 *   otherwise made available to any other person.  No title to and
 *   ownership of the software is hereby transferred.
 *
 *   The information in this software is subject to change  without
 *   notice  and should not be construed as a commitment by Digital
 *   Equipment Corporation.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 */

/* mlist.c -- functionss to deal with message lists. */

#include "decxmail.h"
#include "mlist.h"
#include "toc.h"

/* Create a message list containing no messages. */

MsgList MakeNullMsgList()
{
    MsgList mlist;
    mlist = (MsgList) XtNew(MsgListRec);
    mlist->nummsgs = 0;
    mlist->msglist = (Msg *) XtNew(Msg);
    mlist->msglist[0] = NULL;
    return mlist;
}


/* Append a message to the given message list. */

void AppendMsgList(mlist, msg)
  MsgList mlist;
  Msg msg;
{
    if (msg == NULL) return;
    mlist->nummsgs++;
    mlist->msglist =
	(Msg *) XtRealloc((char *) mlist->msglist,
			  (unsigned) (mlist->nummsgs + 1) * sizeof(Msg));
    mlist->msglist[mlist->nummsgs - 1] = msg;
    mlist->msglist[mlist->nummsgs] = 0;
}



/* Delete a message from a message list. */

void DeleteMsgFromMsgList(mlist, msg)
MsgList mlist;
Msg msg;
{
    register int i;
    for (i=0 ; i<mlist->nummsgs ; i++) {
	if (mlist->msglist[i] == msg) {
	    mlist->nummsgs--;
	    for (; i<mlist->nummsgs ; i++)
		mlist->msglist[i] = mlist->msglist[i+1];
	    return;
	}
    }
}



/* Create a new messages list containing only the one given message. */

MsgList MakeSingleMsgList(msg)
  Msg msg;
{
    MsgList result;
    result = MakeNullMsgList();
    AppendMsgList(result, msg);
    return result;
}


/* We're done with this message list; free it's storage. */

void FreeMsgList(mlist)
   MsgList mlist;
{
    XtFree((char *) mlist->msglist);
    XtFree((char *) mlist);
}



/* Parse the given string into a message list.  The string contains mh-style
   message numbers.  This routine assumes those messages numbers refer to
   messages in the given toc. */

MsgList StringToMsgList(toc, str)
Toc toc;
char *str;
{
    MsgList mlist;
    char *ptr;
    register int first, second, i;
    Msg msg;
    mlist = MakeNullMsgList();
    while (*str) {
        while (*str == ' ')
            str++;
        first = second = atoi(str);
        str++;
        for (ptr = str; *ptr >= '0' && *ptr <= '9'; ptr++) ;
        if (*ptr == '-')
            second = atoi(ptr + 1);
        if (first > 0) {
            for (i = first; i <= second; i++) {
		msg = TocMsgFromId(toc, i);
                if (msg) AppendMsgList(mlist, msg);
	    }
	}
        str = ptr;
    }
    return mlist;
}
