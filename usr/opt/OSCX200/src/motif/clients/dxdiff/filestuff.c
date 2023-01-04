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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/dxdiff/filestuff.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";	/* BuildSystemHeader */
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
 *	filestuff.c - file handling code
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 25th April 1988
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

static char sccsid[] = "@(#)filestuff.c	1.11	17:45:20 2/21/89";


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/file.h>

#include <X11/Xlib.h>
#include <Xm/Xm.h>

#include "y.tab.h"
#include "filestuff.h"
#include "parsediff.h"
#include "alloc.h"
#include "arglists.h"
#include "differencebox.h"

#define	FormPointer(base, offset, ptrtype) ptrtype((char *)(base) + offset)

extern  void XmTextSetHighlight();

/********************** Public Routines ************************/


/********************************
 *
 *	InitializeHighLightInfo
 *
 ********************************/

InitializeHighLightInfo(filep, head, tail, number, whichfile)
	FileInfoPtr filep;
	EdcPtr	     head,
		     tail;
	int	     number;
	WhichFile    whichfile;
{
	register char		  *cp,*top;
	register EdcPtr		  edcp;
	register HighLightInfoPtr hlp;
	register unsigned int	  nsoff;
	register unsigned int	  hloff;
	int		  	  n;
	Boolean			  isleft;

	if (head == (EdcPtr)NULL) {
		return;	/* no diffs ?? */
	}

	if (isleft = (whichfile == LeftFile)) {
		hloff = XtOffset(EdcPtr, highl);
		nsoff = XtOffset(EdcPtr, ns1);
	} else {
		hloff = XtOffset(EdcPtr, highr);
		nsoff = XtOffset(EdcPtr, ns2);
	}


	if (number <= 0) {		/* we dont know - so find out */
		for (n = 0, edcp = head; edcp != tail->next; n++, edcp = edcp->next)
		;
		number = n;
	}

	if (filep->hlp != (HighLightInfoPtr)NULL)
		XtFree((char *)filep->hlp);

	if ((filep->hlp = hlp =
	     (HighLightInfoPtr)XtMalloc(number * sizeof (HighLightInfo))) == 
	     (HighLightInfoPtr)NULL) {	/* error */
		return;
	}

	filep->dlhead = (ForwardEdcPtr)head;
	filep->dltail = (ForwardEdcPtr)tail;

	for (edcp = head, n = 1, cp = filep->data,
	     top = filep->data + filep->filesize; edcp !=  tail->next && cp < top;
	     hlp++, edcp = edcp->next) {
#ifdef	USESECONDARYSELECTION
		if ((edcp->et == EAppend && isleft) ||
		    (edcp->et == EDelete && !isleft))
			hlp->mode = XmHIGHLIGHT_SECONDARY_SELECTED;
		else
			hlp->mode = XmHIGHLIGHT_SELECTED;
#else
		hlp->mode = XmHIGHLIGHT_SELECTED;
#endif

		*FormPointer(edcp, hloff, (HighLightInfoPtr *)) = hlp;

		number = (*FormPointer(edcp, nsoff, (NumberSequencePtr *)))->numbers[0];
		while (n < number && cp < top) {
			while (*cp++ != '\n' && cp < top)
			;
			n++;
		}

		hlp->left = (XmTextPosition)(cp - filep->data);

		number = (*FormPointer(edcp, nsoff, (NumberSequencePtr *)))->numbers[1];

		while (n <= number && cp < top) {
			while (*cp++ != '\n' && cp < top)
			;
			n++;
		}

		hlp->right = (XmTextPosition)(cp /* - 1 */ - filep->data);
	}
}






/********************************
 *
 *	FreeFile
 *
 ********************************/


FreeFile(filep)
	FileInfoPtr filep;
{
	register EdcPtr	p,head,tail;
	unsigned char	mask;
	unsigned int	hloff;
	

	if (filep->data != (char *)NULL)
		XtFree(filep->data);

	head = (EdcPtr)filep->dlhead;
	tail = (EdcPtr)filep->dltail;

	if (head) {
		if (head->highl == filep->hlp) {
			mask = ~HIGHLEFT;
			hloff = XtOffset(EdcPtr, highl);
		} else {
			mask = ~HIGHRIGHT;
			hloff = XtOffset(EdcPtr, highr);
		}

		for (p = head; p != tail->next; p = p->next) {
			p->common.flags &= mask;
			*FormPointer(p, hloff, (HighLightInfoPtr *)) = 
				(HighLightInfoPtr)NULL;
		}
	}
		
	if (filep->hlp != (HighLightInfoPtr)NULL)
		XtFree((char *)filep->hlp);

	XtFree((char *)filep);
}


/********************************
 *
 *	LoadNewFile
 *
 ********************************/

FileInfoPtr
LoadNewFile(file)
	char		*file;
{
	extern int	     errno;
	register FileInfoPtr filep;
	register char	     *cp,*ocp,*top;
	struct stat	     statbuf;
	int		     fd;
	register int	     t;
	
	if ((filep = (FileInfoPtr)XtMalloc(sizeof (FileInfo))) ==
	    (FileInfoPtr)NULL)	{	/* error */
		return filep;
	}

	filep->data = (char *)NULL;
	filep->hlp = (HighLightInfoPtr)NULL;

	filep->path = file;
	filep->filesize = filep->widestline = filep->modtime =
	filep->numlines = 0;

	filep->dlhead = filep->dltail = (ForwardEdcPtr)NULL;

	if (stat(file, &statbuf) == -1) {	/* error */
		perror("bad stat(2) in LoadNewFile");
		return (FileInfoPtr)NULL;
	}

	filep->filesize = statbuf.st_size;
	filep->modtime = statbuf.st_mtime;

	if ((filep->data = cp = XtMalloc(filep->filesize + 1)) == (char *)NULL) {
		return (FileInfoPtr)NULL;
	}

	cp[filep->filesize] = '\0';

	if ((fd = open(file, O_RDONLY)) == -1) {	/* error */
		perror("bad open(2)in LoadNewFile");
		return (FileInfoPtr)NULL;
	}

	if (read(fd, cp, filep->filesize) != filep->filesize) {
			perror("bad read(2) in LoadNewFile");
			XtFree(filep->data);
			close(fd);
			return (FileInfoPtr)NULL;
	}
	close(fd);

	for (top = (cp = ocp = filep->data) + filep->filesize; cp < top;)
		if (*cp++ == '\n') {
			filep->numlines++;
			if ((t = cp - ocp - 2) > (int)(filep->widestline))
				filep->widestline = t;
			ocp = cp;
		}

	if (*(cp - 1) != '\n') {	/* incomplete last line */
			filep->numlines++;
			if ((t = cp - ocp) > (int)(filep->widestline))
				filep->widestline = t;
	}

	return filep;
}
