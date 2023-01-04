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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/dxdiff/hscrollbarcbs.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
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
 *
 *	dxdiff
 *
 *	hscrollbar.c - horizontal scroll bar code!
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 19th August 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 */


#include <sys/types.h>
#include <sys/stat.h>

#ifdef  DEBUG
#include <stdio.h>
#endif  DEBUG
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include "dxdiff.h"
#include "arglists.h"
#include "y.tab.h"
#include "filestuff.h"
#include "parsediff.h"
#include "alloc.h"
#include "differencebox.h"
#include "menu.h"
#include "text.h"
#include "display.h"

/*************************** Private Routines ****************************/

/********************************
 *
 *	HScrollBarUnitIncCallBack
 *
 ********************************/


HScrollBarUnitIncCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
	DoHorizontalScrollingFromCallBack(w, clientd, calld);
}

XtCallbackRec HScrollBarUnitIncCallbackList[] = {
	{ (VoidProc)HScrollBarUnitIncCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};


/********************************
 *
 *	HScrollBarUnitDecCallBack
 *
 ********************************/


HScrollBarUnitDecCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
	DoHorizontalScrollingFromCallBack(w, clientd, calld);
}

XtCallbackRec HScrollBarUnitDecCallbackList[] = {
	{ (VoidProc)HScrollBarUnitDecCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	HScrollBarPageIncCallBack
 *
 ********************************/


HScrollBarPageIncCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
	DoHorizontalScrollingFromCallBack(w, clientd, calld);
}

XtCallbackRec HScrollBarPageIncCallbackList[] = {
	{ (VoidProc)HScrollBarPageIncCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	HScrollBarPageDecCallBack
 *
 ********************************/


HScrollBarPageDecCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
	DoHorizontalScrollingFromCallBack(w, clientd, calld);
}

XtCallbackRec HScrollBarPageDecCallbackList[] = {
	{ (VoidProc)HScrollBarPageDecCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	HScrollBarToTopCallBack
 *
 ********************************/


HScrollBarToTopCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
}

XtCallbackRec HScrollBarToTopCallbackList[] = {
	{ (VoidProc)HScrollBarToTopCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	HScrollBarToBottomCallBack
 *
 ********************************/


HScrollBarToBottomCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
}

XtCallbackRec HScrollBarToBottomCallbackList[] = {
	{ (VoidProc)HScrollBarToBottomCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	HScrollBarDragCallBack
 *
 ********************************/


HScrollBarDragCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
	DoHorizontalScrollingFromCallBack(w, clientd, calld);
}

XtCallbackRec HScrollBarDragCallbackList[] = {
	{ (VoidProc)HScrollBarDragCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	HScrollBarValueChangedCallBack
 *
 ********************************/


HScrollBarValueChangedCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
}

XtCallbackRec HScrollBarValueChangedCallbackList[] = {
	{ (VoidProc)HScrollBarValueChangedCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};
