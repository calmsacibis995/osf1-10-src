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
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/lib/Xm/ToggleBI.c,v 1.1.4.3 1993/04/13 20:48:52 Kenneth_Miller Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)ToggleBI.c	3.9.1.2 91/05/03";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/
/*
 * Include files & Static Routine Definitions
 */
#include <Xm/XmP.h>



/*************************************<->*************************************
 *
 *  DrawSquareButton()
 *
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void _XmDrawSquareButton (w, x, y, size, topGC, bottomGC, centerGC, fill)
Widget w;
int x, y, size;
GC topGC, bottomGC, centerGC;
Boolean fill;
#else /* _NO_PROTO */
void _XmDrawSquareButton (Widget w, int x, int y, int size, GC topGC, GC bottomGC, GC centerGC, Boolean fill)
#endif /* _NO_PROTO */
{
#ifdef DEC_MOTIF_BUG_FIX
   if(size <= 0)
      return;
#endif
   _XmDrawShadow (XtDisplay (w), XtWindow (w), 
		  topGC, bottomGC,
		  2, x, y, size, size);

   if (size > 6)
     XFillRectangle (XtDisplay ((Widget) w), 
		     XtWindow ((Widget) w),
		     centerGC, 
		     ((fill) ? x+2 : x+3),
		     ((fill) ? y+2 : y+3),
		     ((fill) ? size-4 : size-6),
		     ((fill) ? size-4 : size-6));
} 


/************************************************************************
 *
 *  DrawDiamondButton()
 *	The dimond drawing routine.  Used in place of  widgets or gadgets
 *	draw routine when toggleButton's indicatorType is one_of_many.
 *
 ************************************************************************/


#ifdef _NO_PROTO
void _XmDrawDiamondButton (tw, x, y, size, topGC, bottomGC, centerGC, fill)
Widget tw;
int x, y, size;
GC topGC, bottomGC, centerGC;
Boolean fill;
#else /* _NO_PROTO */
void _XmDrawDiamondButton (Widget tw, int x, int y, int size, GC topGC, GC bottomGC, GC centerGC, Boolean fill)
#endif /* _NO_PROTO */
{
   XSegment seg[12];
   XPoint   pt[5];
   int midX, midY;

#ifdef DEC_MOTIF_BUG_FIX
   if(size <= 0)
       return;
#endif

   if (size % 2 == 0)
      size--;

   midX = x + (size + 1) / 2;
   midY = y + (size + 1) / 2;

   /* COUNTER REVERSE DRAWING EFFECT ON TINY ToggleButtonS */
   if (size <= 3)
    {
       /*  The top shadow segments  */

       seg[0].x1 = x + size - 1;	/*  1  */
       seg[0].y1 = midY - 1;
       seg[0].x2 = midX - 1;		/*  2  */
       seg[0].y2 = y + size - 1;

       seg[1].x1 = x + size - 2;	/*  3  */
       seg[1].y1 = midY - 1;
       seg[1].x2 = midX - 1;		/*  4  */
       seg[1].y2 = y + size - 2;

       seg[2].x1 = x + size - 3;	/*  3  */
       seg[2].y1 = midY - 1;
       seg[2].x2 = midX - 1;		/*  4  */
       seg[2].y2 = y + size - 3;

       /*--*/

       seg[3].x1 = midX - 1;		/*  5  */
       seg[3].y1 = y + size - 1;
       seg[3].x2 = x;			/*  6  */
       seg[3].y2 = midY - 1;

       seg[4].x1 = midX - 1;		/*  7  */
       seg[4].y1 = y + size - 2;
       seg[4].x2 = x + 1;		/*  8  */
       seg[4].y2 = midY - 1;

       seg[5].x1 = midX - 1;		/*  7  */
       seg[5].y1 = y + size - 3;
       seg[5].x2 = x + 2;		/*  8  */
       seg[5].y2 = midY - 1;

       /*  The bottom shadow segments  */

       seg[6].x1 = x + size - 1;	/*  9  */
       seg[6].y1 = midY - 1;
       seg[6].x2 = midX - 1;		/*  10  */
       seg[6].y2 = y;

       seg[7].x1 = x + size - 2;	/*  11  */
       seg[7].y1 = midY - 1;
       seg[7].x2 = midX - 1;		/*  12  */
       seg[7].y2 = y + 1;

       seg[8].x1 = x + size - 3;	/*  11  */
       seg[8].y1 = midY - 1;
       seg[8].x2 = midX - 1;		/*  12  */
       seg[8].y2 = y + 2;

       /*--*/

       seg[9].x1 = midX - 1;		/*  13  */
       seg[9].y1 = y;
       seg[9].x2 = x;			/*  14  */
       seg[9].y2 = midY - 1;

       seg[10].x1 = midX - 1;		/*  15  */
       seg[10].y1 = y + 1;
       seg[10].x2 = x + 1;		/*  16  */
       seg[10].y2 = midY - 1;

       seg[11].x1 = midX - 1;		/*  15  */
       seg[11].y1 = y + 2;
       seg[11].x2 = x + 2;		/*  16  */
       seg[11].y2 = midY - 1;

    }
  else    /* NORMAL SIZED ToggleButtonS */
    {
       /*  The top shadow segments  */

       seg[0].x1 = x;			/*  1  */
       seg[0].y1 = midY - 1;
       seg[0].x2 = midX - 1;		/*  2  */
       seg[0].y2 = y;

       seg[1].x1 = x + 1;		/*  3  */
       seg[1].y1 = midY - 1;
       seg[1].x2 = midX - 1;		/*  4  */
       seg[1].y2 = y + 1;

       seg[2].x1 = x + 2;		/*  3  */
       seg[2].y1 = midY - 1;
       seg[2].x2 = midX - 1;		/*  4  */
       seg[2].y2 = y + 2;

       /*--*/

       seg[3].x1 = midX - 1;		/*  5  */
       seg[3].y1 = y;
       seg[3].x2 = x + size - 1;	/*  6  */
       seg[3].y2 = midY - 1;

       seg[4].x1 = midX - 1;		/*  7  */
       seg[4].y1 = y + 1;
       seg[4].x2 = x + size - 2;	/*  8  */
       seg[4].y2 = midY - 1;

       seg[5].x1 = midX - 1;		/*  7  */
       seg[5].y1 = y + 2;
       seg[5].x2 = x + size - 3;	/*  8  */
       seg[5].y2 = midY - 1;


       /*  The bottom shadow segments  */
    
       seg[6].x1 = x;			/*  9  */
       seg[6].y1 = midY - 1;
       seg[6].x2 = midX - 1;		/*  10  */
       seg[6].y2 = y + size - 1;

       seg[7].x1 = x + 1;		/*  11  */
       seg[7].y1 = midY - 1;
       seg[7].x2 = midX - 1;		/*  12  */
       seg[7].y2 = y + size - 2;

       seg[8].x1 = x + 2;		/*  11  */
       seg[8].y1 = midY - 1;
       seg[8].x2 = midX - 1;		/*  12  */
       seg[8].y2 = y + size - 3;

       /*--*/

       seg[9].x1 = midX - 1;		/*  13  */
       seg[9].y1 = y + size - 1;
       seg[9].x2 = x + size - 1;	/*  14  */
       seg[9].y2 = midY - 1;

       seg[10].x1 = midX - 1;		/*  15  */
       seg[10].y1 = y + size - 2;
       seg[10].x2 = x + size - 2;	/*  16  */
       seg[10].y2 = midY - 1;

       seg[11].x1 = midX - 1;		/*  15  */
       seg[11].y1 = y + size - 3;
       seg[11].x2 = x + size - 3;	/*  16  */
       seg[11].y2 = midY - 1;
    }

   XDrawSegments (XtDisplay ((Widget) tw), XtWindow ((Widget) tw),
                  topGC, &seg[3], 3);

   XDrawSegments (XtDisplay ((Widget) tw), XtWindow ((Widget) tw),
                  bottomGC, &seg[6], 6);

   XDrawSegments (XtDisplay ((Widget) tw), XtWindow ((Widget) tw),
                  topGC, &seg[0], 3);

  
/* For Fill */
   if (fill)
   {
      pt[0].x = x + 3;
      pt[0].y = midY - 1;
      pt[1].x = midX - 1 ;
      pt[1].y = y + 2;
      pt[2].x = x + size - 3;
      pt[2].y = midY - 1;
      pt[3].x = midX - 1 ;
      pt[3].y = y + size - 3;
   }
   else
   {
      pt[0].x = x + 4;
      pt[0].y = midY - 1;
      pt[1].x = midX - 1;
      pt[1].y = y + 3;
      pt[2].x = x + size - 4;
      pt[2].y = midY - 1;
      pt[3].x = midX - 1;
      pt[3].y = y + size - 4;
   }


   /* NOTE: code which handled the next two ifs by setting pt[1-3]
      to match pt[0] values was replaced with return statements because
      passing 4 identical coordinates to XFillPolygon caused the PMAX
      to give a bus error.  Dana@HP reports that the call is legitimate
      and that the error is in the PMAX server.  The return statements
      will stay until the situation with the PMAX is resolved. (mitch) */

   /* COUNTER REVERSE DRAWING EFFECT ON TINY ToggleButtonS */
   if (pt[0].x > pt[1].x)
     {
#ifdef CORRECT
       pt[1].x = pt[0].x;
       pt[2].x = pt[0].x;
       pt[3].x = pt[0].x;
#else
       return;
#endif
     }

   if (pt[0].y < pt[1].y)
     {
#ifdef CORRECT
       pt[1].x = pt[0].x;
       pt[2].x = pt[0].x;
       pt[3].x = pt[0].x;
#else
       return;
#endif
     }

   XFillPolygon (XtDisplay ((Widget) tw), XtWindow ((Widget) tw),
                 centerGC, pt, 4, Convex, CoordModeOrigin);
}
