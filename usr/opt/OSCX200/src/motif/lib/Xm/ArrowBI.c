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
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/dec/Xm/ArrowBI.c,v 1.1.2.4 92/04/15 16:26:39 Dave_Hill Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)ArrowBI.c	3.8 91/01/10";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
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
#include <Xm/XmP.h>

/* functions shared by ArrowB and ArrowBG */

/************************************************************************
 *
 *	Calculate the drawing rectangles and draw rectangles.
 *
 ************************************************************************/

#ifdef _NO_PROTO
void _XmGetArrowDrawRects (highlight_thickness, shadow_thickness,
	 	direction, core_width, core_height, top_count, cent_count, 
		bot_count, top, cent, bot)

int  highlight_thickness;
int  shadow_thickness;
unsigned char  direction;
int  core_width;
int  core_height;
short  *top_count;
short  *cent_count;
short  *bot_count;
XRectangle **top;
XRectangle **cent;
XRectangle **bot;

#else /* _NO_PROTO */
void _XmGetArrowDrawRects (int highlight_thickness, int shadow_thickness, unsigned int direction, int core_width, int core_height, short *top_count, short *cent_count, short *bot_count, XRectangle **top, XRectangle **cent, XRectangle **bot)
#endif /* _NO_PROTO */
{
   /*  Arrow rectangle generation function  */

   int size, width, start;
   register int y;
   XRectangle *tmp;
   register int temp;
   short t = 0;
   short b = 0;
   short c = 0;
   int xOffset = 0;
   int yOffset = 0;


   /*  Free the old lists  */

   if (*top != NULL)
   {
      XtFree ((char*)*top);   *top  = NULL;
      XtFree ((char*)*cent);  *cent = NULL;
      XtFree ((char*)*bot);   *bot  = NULL;
      *top_count = 0;
      *cent_count = 0;
      *bot_count = 0;
   }


   /*  Get the size and allocate the rectangle lists  */

   if (core_width > core_height) 
   {
      size = core_height - 2 - 
	     2 * (highlight_thickness + shadow_thickness);
      xOffset = (core_width - core_height) / 2;
   }
   else
   {
      size = core_width - 2 - 
	     2 * (highlight_thickness + shadow_thickness);
      yOffset = (core_height - core_width) / 2;
   }

   if (size < 1) return;


   if (direction == XmARROW_RIGHT ||
       direction == XmARROW_LEFT)
   {
      temp = xOffset;
      xOffset = yOffset;
      yOffset = temp;
   }

   *top  = (XRectangle *) XtMalloc (sizeof (XRectangle) * (size / 2 + 6));
   *cent = (XRectangle *) XtMalloc (sizeof (XRectangle) * (size  / 2 + 6));
   *bot  = (XRectangle *) XtMalloc (sizeof (XRectangle) * (size / 2 + 6));

   /*  Set up a loop to generate the segments.  */

   width = size;
   y = size + highlight_thickness + shadow_thickness - 1 + yOffset;

   start = highlight_thickness + shadow_thickness + 1 + xOffset;

   while (width > 0)
   {

      if (width == 1)
      {
         (*top)[t].x = start; (*top)[t].y = y + 1;
         (*top)[t].width = 1; (*top)[t].height = 1;
         t++;
      }
      else if (width == 2)
      {
         if (size == 2 || 
             (direction == XmARROW_UP ||
              direction == XmARROW_LEFT))
         {
            (*top)[t].x = start; (*top)[t].y = y;
            (*top)[t].width = 2; (*top)[t].height = 1;
            t++;
            (*top)[t].x = start; (*top)[t].y = y + 1;
            (*top)[t].width = 1; (*top)[t].height = 1;
            t++;
            (*bot)[b].x = start + 1; (*bot)[b].y = y + 1;
            (*bot)[b].width = 1; (*bot)[b].height = 1;
            b++;
         }
         else if (direction == XmARROW_UP ||
                  direction == XmARROW_LEFT)
         {
            (*top)[t].x = start; (*top)[t].y = y;
            (*top)[t].width = 2; (*top)[t].height = 1;
            t++;
            (*bot)[b].x = start; (*bot)[b].y = y + 1;
            (*bot)[b].width = 2; (*bot)[b].height = 1;
            b++;
         }
      }
      else
      {
         if (start == highlight_thickness +
                      shadow_thickness + 1 + xOffset)
         {
            if (direction == XmARROW_UP ||
                direction == XmARROW_LEFT)
            {
               (*top)[t].x = start; (*top)[t].y = y;
               (*top)[t].width = 2; (*top)[t].height = 1;
               t++;
               (*top)[t].x = start; (*top)[t].y = y + 1;
               (*top)[t].width = 1; (*top)[t].height = 1;
               t++;
               (*bot)[b].x = start + 1; (*bot)[b].y = y + 1;
               (*bot)[b].width = 1; (*bot)[b].height = 1;
               b++;
               (*bot)[b].x = start + 2; (*bot)[b].y = y;
               (*bot)[b].width = width - 2; (*bot)[b].height = 2;
               b++;
            }
            else
            {
               (*top)[t].x = start; (*top)[t].y = y;
               (*top)[t].width = 2; (*top)[t].height = 1;
               t++;
               (*bot)[b].x = start; (*bot)[b].y = y + 1;
               (*bot)[b].width = 2; (*bot)[b].height = 1;
               b++;
               (*bot)[b].x = start + 2; (*bot)[b].y = y;
               (*bot)[b].width = width - 2; (*bot)[b].height = 2;
               b++;
            }
         }
         else
         {
            (*top)[t].x = start; (*top)[t].y = y;
            (*top)[t].width = 2; (*top)[t].height = 2;
            t++;
            (*bot)[b].x = start + width - 2; (*bot)[b].y = y;
            (*bot)[b].width = 2; (*bot)[b].height = 2;
            if (width == 3)
            {
               (*bot)[b].width = 1;
               (*bot)[b].x += 1;
            }
            b++;
            if (width > 4)
            {
               (*cent)[c].x = start + 2; (*cent)[c].y = y;
               (*cent)[c].width = width - 4; (*cent)[c].height = 2;
               c++;
            }
         }
      }
      start++;
      width -= 2;
      y -= 2;
   }

   if (direction == XmARROW_UP ||
       direction == XmARROW_LEFT)
   {
      *top_count = t;
      *cent_count = c;
      *bot_count = b;
   }
   else
   {   
      tmp = *top;
      *top = *bot;
      *bot = tmp;
      *top_count = b;
      *cent_count = c;
      *bot_count = t;
   }


   /*  Transform the "up" pointing arrow to the correct direction  */

   switch (direction)
   {
      case XmARROW_LEFT:
      {
          register int i; 

          i = -1;
          do
          {
             i++;
             if (i < *top_count)
             {
                temp = (*top)[i].y; (*top)[i].y =
		    (*top)[i].x; (*top)[i].x = temp;
                temp = (*top)[i].width; 
                (*top)[i].width = (*top)[i].height; (*top)[i].height = temp;
             }             
             if (i < *bot_count)
             {
                temp = (*bot)[i].y; (*bot)[i].y =
		    (*bot)[i].x; (*bot)[i].x = temp;
                temp = (*bot)[i].width; 
                (*bot)[i].width = (*bot)[i].height; (*bot)[i].height = temp;
             }             
             if (i < *cent_count)
             {
                temp = (*cent)[i].y; (*cent)[i].y =
		    (*cent)[i].x; (*cent)[i].x = temp;
                temp = (*cent)[i].width; 
                (*cent)[i].width = (*cent)[i].height; (*cent)[i].height = temp;
             }             
          }
          while (i < *top_count || i < *bot_count || i < *cent_count);
      }
      break;

      case XmARROW_RIGHT:
      {
          register int h_right = core_height - 2;
          register int w_right = core_width - 2;
          register int i; 

          i = -1;
          do
          {
             i++;
             if (i < *top_count)
             {
                temp = (*top)[i].y; (*top)[i].y = (*top)[i].x; 
		(*top)[i].x = temp; 
		temp = (*top)[i].width; (*top)[i].width = (*top)[i].height; 
		(*top)[i].height = temp;
                (*top)[i].x = w_right - (*top)[i].x - (*top)[i].width + 2;
                (*top)[i].y = h_right - (*top)[i].y - (*top)[i].height + 2;
             }             
             if (i < *bot_count)
             {
                temp = (*bot)[i].y; (*bot)[i].y = (*bot)[i].x; 
		(*bot)[i].x = temp; 
		temp = (*bot)[i].width; (*bot)[i].width = (*bot)[i].height; 
		(*bot)[i].height = temp;
                (*bot)[i].x = w_right - (*bot)[i].x - (*bot)[i].width + 2;
                (*bot)[i].y = h_right - (*bot)[i].y - (*bot)[i].height + 2;
             }             
             if (i < *cent_count)
             {
                temp = (*cent)[i].y; (*cent)[i].y = (*cent)[i].x; 
		(*cent)[i].x = temp; 
		temp = (*cent)[i].width; (*cent)[i].width = (*cent)[i].height;
		(*cent)[i].height = temp;
                (*cent)[i].x = w_right - (*cent)[i].x - (*cent)[i].width + 2;
                (*cent)[i].y = h_right - (*cent)[i].y - (*cent)[i].height + 2;
             }
          }
          while (i < *top_count || i < *bot_count || i < *cent_count);
      }
      break;

      case XmARROW_UP:
      {
      }
      break;

      case XmARROW_DOWN:
      {
          register int w_down = core_width - 2;
          register int h_down = core_height - 2;
          register int i; 

          i = -1;
          do
          {
             i++;
             if (i < *top_count)
             {
                (*top)[i].x = w_down - (*top)[i].x - (*top)[i].width + 2;
                (*top)[i].y = h_down - (*top)[i].y - (*top)[i].height + 2;
             }
             if (i < *bot_count)
             {
                (*bot)[i].x = w_down - (*bot)[i].x - (*bot)[i].width + 2;
                (*bot)[i].y = h_down - (*bot)[i].y - (*bot)[i].height + 2;
             }
             if (i < *cent_count)
             {
                (*cent)[i].x = w_down - (*cent)[i].x - (*cent)[i].width + 2;
                (*cent)[i].y = h_down - (*cent)[i].y - (*cent)[i].height + 2;
             }
          }

          while (i < *top_count || i < *bot_count || i < *cent_count);
      }
      break;
   }
}


/************************************************************************
 *
 *  _XmOffsetArrow
 *	Offset the arrow drawing rectangles, if needed, by the difference
 *	of the current x, y and the saved x, y);
 *
 ************************************************************************/
 
#ifdef _NO_PROTO
void _XmOffsetArrow (diff_x, diff_y, top, cent, bot, 
                     top_count, cent_count, bot_count)
register int diff_x;
register int diff_y;
XRectangle * top;
XRectangle * bot;
XRectangle * cent;
int top_count;
int cent_count;
int bot_count;

#else /* _NO_PROTO */
void _XmOffsetArrow (int diff_x, int diff_y, XRectangle *top, XRectangle *cent, XRectangle *bot, int top_count, int cent_count, int bot_count)
#endif /* _NO_PROTO */
{
   register int i;

   if (diff_x != 0 || diff_y != 0)
   {
      for (i = 0; i < top_count; i++)
      {
         (top + i)->x += diff_x;
         (top + i)->y += diff_y;
      }

      for (i = 0; i < cent_count; i++)
      {
         (cent + i)->x += diff_x;
         (cent + i)->y += diff_y;
      }

      for (i = 0; i < bot_count; i++)
      {
         (bot + i)->x += diff_x;
         (bot + i)->y += diff_y;
      }
   }
}

