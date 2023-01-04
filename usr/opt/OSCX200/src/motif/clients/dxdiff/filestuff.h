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
/* BuildSystemHeader added automatically */
/* $Header: /usr/sde/x11/rcs/x11/src/./motif/clients/dxdiff/filestuff.h,v 1.1 90/01/01 00:00:00 devrcs Exp $ */
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
 *	filestuff.h - include file for file handling
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

#ifndef	FILESTUFF.H
#define	FILESTUFF.H


typedef	struct _highlightinfo	{
	XmTextPosition	left,
			right;
	XmHighlightMode	mode;
} HighLightInfo, *HighLightInfoPtr;

typedef	struct _edc *ForwardEdcPtr;	/* we need this forward dec'l */

typedef	struct _fileinfo	{
	char			*path;		/* file name */
	char			*data;		/* ptr to the data */

	ForwardEdcPtr		dlhead,
				dltail;		/* the diff list */
	HighLightInfoPtr	hlp;		/* highlight list */

	off_t			filesize;	/* how long ? */
	time_t			modtime;	/* not really interesting ? */
	unsigned int		numlines;	/* number of lines */
	unsigned short		widestline;	/* width of widest */
}	FileInfo, *FileInfoPtr;

#endif	FILESTUFF.H
