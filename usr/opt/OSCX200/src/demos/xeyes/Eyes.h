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
* $XConsortium: Eyes.h,v 1.7 90/12/01 13:04:51 rws Exp $
*/

#ifndef _XtEyes_h
#define _XtEyes_h

/***********************************************************************
 *
 * Eyes Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		pixel		White
 border		     BorderColor	pixel		Black
 borderWidth	     BorderWidth	int		1
 foreground	     Foreground		Pixel		Black
 outline	     Outline		Pixel		Black
 height		     Height		int		120
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 reverseVideo	     ReverseVideo	Boolean		False
 width		     Width		int		120
 x		     Position		int		0
 y		     Position		int		0

*/

#define XtNoutline	"outline"
#define XtNcenterColor	"center"

#define XtNshapeWindow	"shapeWindow"
#define XtCShapeWindow	"ShapeWindow"

typedef struct _EyesRec *EyesWidget;  /* completely defined in EyesPrivate.h */
typedef struct _EyesClassRec *EyesWidgetClass;    /* completely defined in EyesPrivate.h */

extern WidgetClass eyesWidgetClass;

#endif /* _XtEyes_h */
/* DON'T ADD STUFF AFTER THIS #endif */
