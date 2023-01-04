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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/dxdiff/vscrollbarcbs.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";	/* BuildSystemHeader */
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
 *	vscrollbar.c - vertical scroll bar code!
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 5th May 1988
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

static char sccsid[] = "@(#)vscrollbarcbs.c	1.5	17:45:56 2/21/89";

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
 *	VScrollBarUnitIncCallBack
 *
 ********************************/


VScrollBarUnitIncCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
	DoScrollFromCallBack(w, clientd, calld);
}

XtCallbackRec VScrollBarUnitIncCallbackList[] = {
	{ (VoidProc)VScrollBarUnitIncCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};


/********************************
 *
 *	VScrollBarUnitDecCallBack
 *
 ********************************/


VScrollBarUnitDecCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
	DoScrollFromCallBack(w, clientd, calld);
}

XtCallbackRec VScrollBarUnitDecCallbackList[] = {
	{ (VoidProc)VScrollBarUnitDecCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	VScrollBarPageIncCallBack
 *
 ********************************/


VScrollBarPageIncCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
	DoScrollFromCallBack(w, clientd, calld);
}

XtCallbackRec VScrollBarPageIncCallbackList[] = {
	{ (VoidProc)VScrollBarPageIncCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	VScrollBarPageDecCallBack
 *
 ********************************/


VScrollBarPageDecCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
	DoScrollFromCallBack(w, clientd, calld);
}

XtCallbackRec VScrollBarPageDecCallbackList[] = {
	{ (VoidProc)VScrollBarPageDecCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	VScrollBarToTopCallBack
 *
 ********************************/


VScrollBarToTopCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
}

XtCallbackRec VScrollBarToTopCallbackList[] = {
	{ (VoidProc)VScrollBarToTopCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	VScrollBarToBottomCallBack
 *
 ********************************/


VScrollBarToBottomCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
}

XtCallbackRec VScrollBarToBottomCallbackList[] = {
	{ (VoidProc)VScrollBarToBottomCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	VScrollBarDragCallBack
 *
 ********************************/


VScrollBarDragCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
	DoScrollFromCallBack(w, clientd, calld);
}

XtCallbackRec VScrollBarDragCallbackList[] = {
	{ (VoidProc)VScrollBarDragCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};



/********************************
 *
 *	VScrollBarValueChangedCallBack
 *
 ********************************/


VScrollBarValueChangedCallBack(w, clientd, calld)
	register Widget	w;
	caddr_t		clientd;
	caddr_t		calld;
{
	DoScrollFromCallBack(w, clientd, calld);
}

XtCallbackRec VScrollBarValueChangedCbkList[] = {
	{ (VoidProc)VScrollBarValueChangedCallBack, NULL },
	{ (VoidProc)NULL, NULL }
};
