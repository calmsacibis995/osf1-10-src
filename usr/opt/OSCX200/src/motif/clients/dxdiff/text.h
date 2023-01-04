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
/* $Header: /usr/sde/x11/rcs/x11/src/./motif/clients/dxdiff/text.h,v 1.1 90/01/01 00:00:00 devrcs Exp $ */
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
 *	text.h - include file for text
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 29th April 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	17th August	1988	Laurence P. G. Cable
 *
 *	Experimental Horizontal Scroll stuff
 */

#ifndef	TEXT_H
#define	TEXT_H



/********************************
 *
 *      TextDisplay
 *
 ********************************/

typedef struct  _textdisplay	{
	CoreArgList		core;
	ADBConstraintArgList	constraints;
	TextArgList		text;
	FontArgList		font;

	Widget			textwidget;
	FileInfoPtr		file;
	XmFontList		fontlist;
	
	Widget			scrollwindow;
	int			scrollwidth,
				scrollheight;
	unsigned short		actualwidth;

} TextDisplay, *TextDisplayPtr;

#define	InitTextDisplayPtrArgList(p)				\
		InitCoreArgList((p)->core);			\
		InitADBConstraintArgList((p)_>contraints);	\
		InitTextArgList((p)->text);			\
		InitFontArgList((p)->font)

#define	InitTextDisplayArgList(p)	InitTextDisplayPtrArgList(&(p))

#define	TextDisplayPtrCoreArgList(p)	((p)->core)
#define	TextDisplayCoreArgList(p)	TextDisplayPtrCoreArgList(&(p))

#define	TextDisplayPtrConstraintArgList(p)	((p)->constraints)
#define	TextDisplayConstraintsArgList(p)	\
			TextDisplayPtrConstraintArgList(&(p))

#define	TextDisplayPtrTextArgList(p)	((p)->text)
#define	TextDisplayTextArgList(p)	TextDisplayPtrTextArgList(&(p))

#define	TextDisplayPtrFontArgList(p)	((p)->font)
#define	TextDisplayFontArgList(p)	TextDisplayPtrFontArgList(&(p))

#define	TextDisplayPtrTextWindowArgList(p) ((p)->textW)
#define	TextDisplayTextWindowArgList(p)	TextDisplayPtrTextWindowArgList(&(p))

#define	TextDisplayPtrWidget(p)		((p)->textwidget)
#define	TextDisplayWidget(p)		TextDisplayPtrWidget(&(p))

#define	TextDisplayPtrFile(p)		((p)->file)
#define	TextDisplayFile(p)		TextDisplayPtrFile(&(p))

#define	TextDisplayPtrFontList(p)	((p)->fontlist)
#define	TextDisplayFontList(p)		TextDisplayPtrFontList(&(p))

#define	TextDisplayPtrScrollWidget(p)	((p)->scrollwindow)
#define	TextDisplayScrollWidget(p)	TextDisplayPtrScrollWidget(&(p))

#define	TextDisplayPtrScrollWidth(p)	((p)->scrollwidth)
#define	TextDisplayScrollWidth(p)	TextDisplayPtrScrollWidth(&(p))

#define	TextDisplayPtrScrollHeight(p)	((p)->scrollheight)
#define	TextDisplayScrollHeight(p)	TextDisplayPtrScrollHeight(&(p))

#define	TextDisplayPtrActualWidth(p)	((p)->actualwidth)
#define	TextDisplayActualWidth(p)	TextDisplayPtrActualWidth(&(p))
#endif	TEXT_H
