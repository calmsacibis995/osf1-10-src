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
/*
 * $XConsortium: Menu.h,v 1.2 89/07/21 14:22:10 jim Exp $
 */

#ifndef _XtMenu_h
#define _XtMenu_h

/***********************************************************************
 *
 * Menu Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		pixel		White
 border		     BorderColor	pixel		Black
 borderWidth	     BorderWidth	int		1
 height		     Height		int		120
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 reverseVideo	     ReverseVideo	Boolean		False
 width		     Width		int		120
 x		     Position		int		0
 y		     Position		int		0

*/

#define XtNmenuEntries		"menuEntries"
#define XtNhorizontalPadding	"horizontalPadding"
#define XtNverticalPadding	"verticalPadding"
#define XtNselection		"Selection"

#define XtCMenuEntries		"MenuEntries"
#define XtCPadding		"Padding"
#define XtCSelection		"Selection"

typedef struct _MenuRec *MenuWidget;  /* completely defined in MenuPrivate.h */
typedef struct _MenuClassRec *MenuWidgetClass;    /* completely defined in MenuPrivate.h */

extern WidgetClass menuWidgetClass;

extern Widget	XawMenuCreate ();
#endif /* _XtMenu_h */
/* DON'T ADD STUFF AFTER THIS #endif */
