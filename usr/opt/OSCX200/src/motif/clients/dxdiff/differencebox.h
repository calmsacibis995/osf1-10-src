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
/* $Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxdiff/differencebox.h,v 1.1.4.2 1993/09/03 21:03:22 Lynda_Rice Exp $ */
/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
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
 *	differencebox.h - include file for difference box
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 24th March 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	16th April 1988	Laurence P. G. Cable
 *
 *	Added the line number display code support!
 *
 *	22nd April 1988 Laurence P. G. Cable
 *
 *	Added support for dragging optimisation and kine number painting
 *
 *	6th May 1988	Laurence P. G. Cable
 *
 *	changed interface to support the dxdiff homogenous design
 *
 *	1 Sep 1993	Lynda Rice
 *
 *	Added the Slave macro for code clarity; added the display variable
 *	to the differencebox structure in order to locate a scrollbar widget
 *	in GetVerticalSliderSize(), used to determine the number of rows on
 *	a screen; and defined the scrolling boolean to determine whether the
 *	scrollbars or the Next/Prev Diff buttons are being used.
 */

#ifndef	DIFFERENCEBOX_H
#define	DIFFERENCEBOX_H


#ifndef	XtOffset
#define	XtOffset(type, field)	((unsigned int)&(((type)NULL)->field))
#endif	XtOffset

typedef	enum _whichfile		{
				  LeftFile,
				  RightFile
				} WhichFile;

#define Slave(file)		\
	((file == LeftFile) ? RightFile : LeftFile)

typedef	enum _diffscrollmode	{ NotScrolling,
				  ScrollOnlyOne,
				  ScrollBoth
				} DiffScrollMode;

#define	SetDifferenceBoxScrollOff(db)	((db)->scrollmode = NotScrolling)
#define	SetDifferenceBoxScrollOne(db)	((db)->scrollmode = ScrollOnlyOne)
#define	SetDifferenceBoxScrollBoth(db)	((db)->scrollmode = ScrollBoth)


typedef	enum _diffdrawmode	{ DrawDiffsAsLines,
				  DrawDiffsAsFilledPolygons
				} DiffDrawMode;


#define	SetDifferenceBoxToDrawDiffsAsFilledPolygons(db)		\
		((db)->drawdiffsas = DrawDiffsAsFilledPolygons)

#define	SetDifferenceBoxToDrawDiffsAsLines(db)		\
		((db)->drawdiffsas = DrawDiffsAsLines)

#define	SetDifferenceBoxDraggingOn(db)		((db)->dragging = True)
#define	SetDifferenceBoxDraggingOff(db)		((db)->dragging = False)

#define	SetDifferenceBoxDrawDiffsOn(db)		((db)->painting = True)
#define	SetDifferenceBoxDrawDiffsOff(db)	((db)->painting = False)

#define	SetDifferenceBoxPaintLineNumbersOn(db)	((db)->linenumbers = True)
#define	SetDifferenceBoxPaintLineNumbersOff(db)	((db)->linenumbers = False)

/* typedef for new point list stuff */

typedef	struct	_pointlist	{
	int	numpoints;
	XPoint	points[5];	/* should be large enough to hold all the points ever needed */
}	PointList, *PointListPtr;

/* struct def for difference line number data */

typedef struct	_linenumber	{
	int	x,y;		/* pixel postition */
	int	len;		/* of string */
	char	number[9];	/* we should never be larger than this!! */
} LineNumber, *LineNumberPtr;

typedef	struct	_differencebox	*DifferenceBoxPtr;

typedef struct	_differencebox	{
	CoreArgList		core;		/* core */
	ADBConstraintArgList	constraints;	/* constraints */
	Arg			exposecallback;	/* guess ! */

	Widget	window;
	caddr_t display ;	/* DxDiffDisplayPtr */

	int	width,height;	/* pixels */
	int	rows;		/* height in rows of text = slider_size */
	int	fontheight;	/* height of font bbox */
	int	vmargins;	/* height of vertical margins in pixels */
	int	lw;		/* width of lines */

	Boolean		 resizenotifypending;	/* please do one !! */
	void		 (*resizenotify)();	/* notify someone that I have been resized */
	caddr_t		 clientdata;		/* client parameter to resize notify */
	DifferenceBoxPtr prevdb;		/* old geometry */

	GC	gc;		/* gc of gc for window */
	GC	draggc;		/* gc for line drawing during drags */

	XColor		*lnforeground;		/* foreground line no pixel */
	void		(*drawstring)();	/* image or not ? */

	GC		lnfgc;	/* gc for painting line numbers */
	XFontStruct	*font;	/* the font info for paint line numbers */
	LineNumberPtr	lnos;	/* the table of line numbers ! */
	int		tfw;	/* total field width in chars needed to 
				   display the line numbers for both files */

	WhichFile		lastscrolled;	/* who scrolled last ? */
	DiffScrollMode		scrollmode;	/* scrolling mode of diff box */
	DiffDrawMode		drawdiffsas;	/* lines or filled polygons */

	Boolean	scrolling;	/* are we using scrollbars */
	Boolean	dragging;	/* are we dragging */
	Boolean	painting;	/* are we drawing this */
	Boolean linenumbers;	/* are we painting line numbers */

	int	toplnolf,	/* line number at top of 'left' file */
		ptoplnolf;	/* previous */
	int	toplnorf,	/* line number at top of 'right' file */
		ptoplnorf;
	int	lflen,		/* length of left file in lines */
		rflen;		/* length of right file in lines */

	EdcPtr	head,tail,	/* the difference list for this box */
		vdtop,vdbottom,	/* the list of visible differences */
		viewprtl,viewprtr;

	int	dllen;		/* number of nodes between top and bottom */

	int		nstrs;	/* the number of line numbers */
	int		npoints;/* the number of points */
	PointListPtr	points;	/* the points themselves */
} DifferenceBox /* ,*DifferenceBoxPtr */ ;

#endif	DIFFERENCEBOX_H		/* add nothing after this line */
