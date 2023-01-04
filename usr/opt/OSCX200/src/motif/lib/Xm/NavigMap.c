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
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/Xm/NavigMap.c,v 1.1.2.5 92/06/09 16:20:39 Dave_Hill Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)NavigMap.c	3.13.1.4 91/06/26";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, 1990 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY 
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
#include "TraversalI.h"
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/ShellP.h>

#ifdef VAXC
#pragma noinline (AddToImap,\
		  GetLevel,\
		  GetTopWidget,\
		  CreateInternalNavigMap,\
		  DestroyInternalNavigMap,\
		  DestroyNavigMap)
#endif

#define _XmNORMAL 1
#define _XmSPECIAL 2
#define _XmSELECTED 4

#define NEW_SORT

static void AddToImap(iMap, w, visRect, level, flags)
    XmNavigIMap	iMap;
    Widget	w;
    XRectangle	*visRect;
    Cardinal	level;
    unsigned char flags;
{
    XmNavigIMapEntry	iMapEntry;

    if (iMap->numEntries == iMap->maxEntries-1)
      {
	  iMap->maxEntries = 2 * iMap->maxEntries;
	  iMap->entries = (XmNavigIMapEntry)
	    XtRealloc((char*)iMap->entries, 
		      sizeof(XmNavigIMapEntryRec) * iMap->maxEntries);
      }
    iMapEntry 			= &(iMap->entries[iMap->numEntries++]);
    iMapEntry->widget		= w;
    iMapEntry->visRect 		= *visRect;
    iMapEntry->level 		= level;
    iMapEntry->flags		= flags;
}

static void FindItems(iMap, w, np, visRect, level)
    XmNavigIMap		iMap;
    Widget		w;
    WidgetNavigPtrs	np;
    XRectangle		*visRect;
    Cardinal		level;
{
    XRectangle		tmpRectRec;
    
    /* 
     * generate the visiblity rect to pass down (if needed) explicitly
     * and don't pass a visRect to _XmWidgetIsTraversable since it
     * would redo the same check. Do it directly since otherwise we
     * won't have an updated new rect to pass down that *doesn't munge
     * the existing one.
     */
    if (_XmIntersectRect(visRect, w, &tmpRectRec) &&
	_XmWidgetIsTraversable(w, np, XmNONE, NULL))
      {
	  CompositeWidget	cw = (CompositeWidget)w;
	  if (!XtIsComposite(w) || !(cw->composite.num_children))
	    {
		/*
		 * make level 0 so that all items are at same depth  |||
		 */
		AddToImap(iMap, w, &tmpRectRec, 0, _XmNORMAL);
	    }
	  else
	    {
		Cardinal		i;
		Widget		*child;
		for (i = 0, child = (cw->composite.children);
		     i < cw->composite.num_children;
		     i++, child++)
		  {
		      WidgetNavigPtrsRec	npRec;
		      WidgetNavigPtrs		npChild = &npRec;
		      
		      GetWidgetNavigPtrs(*child, npChild);
		      FindItems(iMap, *child, npChild, &tmpRectRec, level+1);
		  }
	    }
      }
}

static void FindItemSet(iMap, w, np, visRect, level)
    XmNavigIMap		iMap;
    Widget		w;
    WidgetNavigPtrs	np;
    XRectangle		*visRect;
    Cardinal		level;
{
    XRectangle		tmpRectRec;

    /* 
     * generate the visiblity rect to pass down (if needed) explicitly
     * and don't pass a visRect to _XmWidgetIsTraversable since it
     * would redo the same check. Do it directly since otherwise we
     * won't have an updated new rect to pass down that *doesn't munge
     * the existing one.
     */
    if (_XmIntersectRect(visRect, w, &tmpRectRec) &&
	_XmWidgetIsTraversable(w, np, XmTAB_ANY, NULL))
      {
	  if (XtIsComposite(w))
	    {
		CompositeWidget	cw = (CompositeWidget)w;
		Cardinal		i;
		Widget		*child;
		for (i = 0, child = (cw->composite.children);
		     i < cw->composite.num_children;
		     i++, child++)
		  {
		      WidgetNavigPtrsRec	npRec;
		      WidgetNavigPtrs		npChild = &npRec;
		      
		      GetWidgetNavigPtrs(*child, npChild);
		      FindItems(iMap, *child, npChild, &tmpRectRec, level+1);
		  }
	    }
	  else
	    /*
	     * make level 0 so that all items are at same depth  |||
	     */
	    AddToImap(iMap, w, &tmpRectRec, 0, _XmNORMAL);
      }
}

static void FindTabSet(iMap, w, np, visRect, level)
    XmNavigIMap		iMap;
    Widget		w;
    WidgetNavigPtrs	np;
    XRectangle		*visRect;
    Cardinal		level;
{
    XRectangle		tmpRectRec;
     /* 
     * generate the visiblity rect to pass down (if needed) explicitly
     * and don't pass a visRect to _XmWidgetIsTraversable since it
     * would redo the same check. Do it directly since otherwise we
     * won't have an updated new rect to pass down that *doesn't munge
     * the existing one.
     */
    if (_XmIntersectRect(visRect, w, &tmpRectRec) &&
	_XmWidgetIsTraversable(w, np, XmTAB_ANY, NULL))
      {
	  /*
	   * add it to the navigable set based on whether it matches
	   * the tab type. I'm assuming that only a non-composite can
	   * be a control.
	   */
	  
	  if (NavigIsTabGroup(np) &&
	      _XmPathIsTraversable(w, XmTAB_GROUP, BelowOnly, visRect))
	    {
		AddToImap(iMap, w, &tmpRectRec, level, _XmNORMAL);
	    }
	  
	  if (XtIsComposite(w))
	    {
		CompositeWidget	cw = (CompositeWidget)w;
		Cardinal		i;
		Widget		*child;
		for (i = 0, child = (cw->composite.children);
		     i < cw->composite.num_children;
		     i++, child++)
		  {
		      WidgetNavigPtrsRec	npRec;
		      WidgetNavigPtrs		npChild = &npRec;
		      
		      GetWidgetNavigPtrs(*child, npChild);
		      FindTabSet(iMap, *child, npChild, &tmpRectRec, level+1);
		  }
	    }
      }
}

static Widget	GetTopWidget(shell)
    ShellWidget	shell;
{
    Cardinal	i;

    for (i= 0; i < shell->composite.num_children; i++)
      if XtIsManaged(shell->composite.children[i])
	return shell->composite.children[i];
    return NULL;
}

static XmNavigIMap CreateInternalNavigMap(top, tabType)
    Widget		top;
    unsigned char	tabType;
{
    XmNavigIMap	navigIMap;
    
    navigIMap = XtNew(XmNavigIMapRec);
    
    navigIMap->numEntries = 0;
    navigIMap->maxEntries = 64;
    navigIMap->entries = (XmNavigIMapEntry)
      XtMalloc(sizeof(XmNavigIMapEntryRec) * navigIMap->maxEntries);
    return navigIMap;
}


static void DestroyInternalNavigMap(navigIMap)
    XmNavigIMap	navigIMap;
{
    XtFree((char*)navigIMap->entries);
    XtFree((char*)navigIMap);
}

static void DestroyNavigMap(navigMap)
    XmNavigMap	navigMap;
{
    XtFree((char*)navigMap->entries);
    XtFree((char*)navigMap->horizList);
    XtFree((char*)navigMap->vertList);
    XtFree((char*)navigMap);
}

#ifdef NEW_SORT

#define YOVERLAPS(a, c) \
  (((c->y < (a->y + (int)a->height)) && \
    ((c->y + (int)c->height) > a->y)))

#define OLDBETWEEN(x1, x2, a, b, c) \
  ((a->level <= c->level) && (b->level <= c->level) && \
   (\
    (((a->visRect.x1 < c->visRect.x1) || \
      ((a->visRect.x1 == c->visRect.x1) && \
       (a->visRect.x2 > c->visRect.x2))) && \
     ((c->visRect.x1 < b->visRect.x1) || \
      ((c->visRect.x1 == b->visRect.x1) && \
       (c->visRect.x2 > b->visRect.x2))) || \
     ((a->widget < b->widget) && (b->widget < c->widget)) \
     )))

#define BETWEEN(x1, x2, a, b, c) \
  ( \
   (((a)->level <= (c)->level) 			|| \
    (!IsAncestor(((a)->widget),((c)->widget))))	&& \
   (((c)->level <= (b)->level)			|| \
    (!IsAncestor(((b)->widget),((c)->widget))))	&& \
   (((a)->visRect.x1) < ((c)->visRect.x1))	&& \
   (((c)->visRect.x1) < ((b)->visRect.x1)))

#define XBETWEEN(a, b, c) BETWEEN(x, y, a, b, c)

static Boolean IsAncestor(first, second)
    Widget	first, second;
{
    Widget	w;

    for (w = second; !XtIsShell(w); w = XtParent(w))
      if (w == first)
	return True;
    return False;
}


#ifdef DEC_MOTIF_EXTENSION
static Cardinal HorizInBetween(iMap, above, below, betweenRtn, layout_rtol)
    XmNavigIMap	iMap;
    Dimension	above, below;
    Dimension	**betweenRtn;
    Boolean	layout_rtol;
#else
static Cardinal HorizInBetween(iMap, above, below, betweenRtn)
    XmNavigIMap	iMap;
    Dimension	above, below;
    Dimension	**betweenRtn;
#endif DEC_MOTIF_EXTENSION
{
    XmNavigIMapEntry	aE, bE, cE;
    Cardinal		i, numRtn = 0;
    XRectangle		*aR, *bR, *cR;

    aE = &(iMap->entries[above]);
    bE = &(iMap->entries[below]);
    
    aR = &(aE->visRect);
    bR = &(bE->visRect);

    for (i = 0; i < iMap->numEntries; i++)
      {
	  cE = &(iMap->entries[i]);
	  cR = &(cE->visRect);

#ifdef DEC_MOTIF_EXTENSION
/*
   BETWEEN should operate in opposite direction:
   in both cases XBTETWEEN(a,b,c) is TRUE and XBTETWEEN(b,a,c) is FALSE
	---------->X
	a  c  b  	LtoR 

	b  c  a		RtoL
   we just swap arguments of XBETWEEN
*/
	if ( layout_rtol ) {
	  if (!(cE->flags & _XmSELECTED) &&
	      !((i == above) || (i == below)) &&
	      (YOVERLAPS(aR, cR)) &&
	      (YOVERLAPS(bR, cR)) &&
	      (XBETWEEN(bE, aE, cE)))
	    (*betweenRtn)[numRtn++] = i;
	}
	else {
	  if (!(cE->flags & _XmSELECTED) &&
	      !((i == above) || (i == below)) &&
	      (YOVERLAPS(aR, cR)) &&
	      (YOVERLAPS(bR, cR)) &&
	      (XBETWEEN(aE, bE, cE)))
	    (*betweenRtn)[numRtn++] = i;
	}
#else
	  if (!(cE->flags & _XmSELECTED) &&
	      !((i == above) || (i == below)) &&
	      (YOVERLAPS(aR, cR)) &&
	      (YOVERLAPS(bR, cR)) &&
	      (XBETWEEN(aE, bE, cE)))
	    (*betweenRtn)[numRtn++] = i;
#endif DEC_MOTIF_EXTENSION
      }
    return numRtn;
}

#ifdef DEC_MOTIF_EXTENSION
static Cardinal HorizClosest(iMap, above, between, numBetween, layout_rtol)
    XmNavigIMap	iMap;
    Dimension	above;
    Dimension	*between;
    Cardinal	numBetween;
    Boolean	layout_rtol;
#else
static Cardinal HorizClosest(iMap, above, between, numBetween)
    XmNavigIMap	iMap;
    Dimension	above;
    Dimension	*between;
    Cardinal	numBetween;
#endif DEC_MOTIF_EXTENSION
{
    Dimension		min = between[0];
    XmNavigIMapEntry	aE, bE, cE;
    Cardinal		i;
    XRectangle		*aR, *bR, *cR;


    aE = &(iMap->entries[above]);
    aR = &(aE->visRect);

    bE = &(iMap->entries[min]);
    bR = &(bE->visRect);

    for (i = 1; i < numBetween; i++)
      {
	  cE = &(iMap->entries[between[i]]);
	  cR = &(cE->visRect);

#ifdef DEC_MOTIF_EXTENSION
/*
   In case of RtoL left rectangle is 'less' then right therefore we change
   condition (bR->x > cR->x) to (bR->x < cR->x)
   this together with modified HorizInBetween and exchanged values of
   of TOPPER & BOTTOMER will do sort from left to right and
   top down (hopefully).
*/
	if ( layout_rtol ) {
	  if (((bE->level > cE->level) &&
	       (IsAncestor(cE->widget, bE->widget))) ||
	      ((bR->y > cR->y) ||
	       ((bR->y == cR->y)  &&
		((bR->x < cR->x) ||
		 ((bR->x == cR->x))))))
	    {
		min = between[i];
		bE = &(iMap->entries[min]);
		bR = &(bE->visRect);
	    }
	}
	else {
	  if (((bE->level > cE->level) &&
	       (IsAncestor(cE->widget, bE->widget))) ||
	      ((bR->y > cR->y) ||
	       ((bR->y == cR->y)  &&
		((bR->x > cR->x) ||
		 ((bR->x == cR->x))))))
	    {
		min = between[i];
		bE = &(iMap->entries[min]);
		bR = &(bE->visRect);
	    }
	}
#else
	  if (((bE->level > cE->level) &&
	       (IsAncestor(cE->widget, bE->widget))) ||
	      ((bR->y > cR->y) ||
	       ((bR->y == cR->y)  &&
		((bR->x > cR->x) ||
		 ((bR->x == cR->x))))))
	    {
		min = between[i];
		bE = &(iMap->entries[min]);
		bR = &(bE->visRect);
	    }
#endif DEC_MOTIF_EXTENSION
      }
    return min;
}


#ifdef DEC_MOTIF_EXTENSION
static void HorizOverlapSort(iMap, TOPPER, BOTTOMER, mapRtn, layout_rtol)
    XmNavigIMap	iMap;
    Dimension	TOPPER, BOTTOMER;
    Dimension	**mapRtn;
    Boolean	layout_rtol;
#else
static void HorizOverlapSort(iMap, TOPPER, BOTTOMER, mapRtn)
    XmNavigIMap	iMap;
    Dimension	TOPPER, BOTTOMER;
    Dimension	**mapRtn;
#endif DEC_MOTIF_EXTENSION
{
    Dimension	above = TOPPER, *between ;
    Cardinal	curr = 0;

    between = (Dimension*)XtMalloc(sizeof(Dimension)*(iMap->numEntries));

    while (1)
      {
	  Dimension	below = BOTTOMER;
	  Cardinal	numBetween;
	  
#ifdef DEC_MOTIF_EXTENSION
	  while (numBetween = HorizInBetween(iMap, 
					     above, 
					     below, 
					     &between,
					     layout_rtol))
	    {
		below = HorizClosest(iMap, 
				     above, 
				     between, 
				     numBetween,
				     layout_rtol);
	    }
#else
	  while (numBetween = HorizInBetween(iMap, 
					     above, 
					     below, 
					     &between))
	    {
		below = HorizClosest(iMap, 
				     above, 
				     between, 
				     numBetween);
	    }
#endif DEC_MOTIF_EXTENSION
	  if (below == BOTTOMER)
	    {
		if (above != TOPPER)
		  above = TOPPER;
		else {
		    XtFree((char*)between);
		    return;
		}
	    }
	  else 
	    {
		above = (*mapRtn)[curr++] = below;
		iMap->entries[below].flags |= _XmSELECTED;
#ifdef notdef
		iMap->entries[below].visRect.x--;
#endif
	    }
      }
}

#define XOVERLAPS(a, c) \
  (((c->x < (a->x + (int)(a->width))) && \
    ((c->x + (int)(c->width)) > a->x)))

#define YBETWEEN(a, b, c) BETWEEN(y, x, a, b, c)

static Cardinal VertInBetween(iMap, above, below, betweenRtn)
    XmNavigIMap	iMap;
    Dimension	above, below;
    Dimension	**betweenRtn;
{
    XmNavigIMapEntry	aE, bE, cE;
    XRectangle		*aR, *bR, *cR;
    Cardinal		i, numRtn = 0;

    aE = &(iMap->entries[above]);
    bE = &(iMap->entries[below]);

    
    aR = &(aE->visRect);
    bR = &(bE->visRect);

    for (i = 0; i < iMap->numEntries; i++)
      {
	  cE = &(iMap->entries[i]);
	  cR = &(cE->visRect);
	  if (!(cE->flags & _XmSELECTED) &&
	      !((i == above) || (i == below)))
	    if (XOVERLAPS(aR, cR))
	      if (XOVERLAPS(bR, cR))
		if (YBETWEEN(aE, bE, cE))
		  (*betweenRtn)[numRtn++] = i;
      }
    return numRtn;
}

#ifdef DEC_MOTIF_EXTENSION
static Cardinal VertClosest(iMap, above, between, numBetween, layout_rtol)
    XmNavigIMap	iMap;
    Dimension	above;
    Dimension	*between;
    Cardinal	numBetween;
    Boolean	layout_rtol;
#else
static Cardinal VertClosest(iMap, above, between, numBetween)
    XmNavigIMap	iMap;
    Dimension	above;
    Dimension	*between;
    Cardinal	numBetween;
#endif DEC_MOTIF_EXTENSION
{
    Dimension		min = between[0];
    XmNavigIMapEntry	aE, bE, cE;
    Cardinal		i;
    XRectangle		*aR, *bR, *cR;

    aE = &(iMap->entries[above]);
    aR = &(aE->visRect);

    bE = &(iMap->entries[min]);
    bR = &(bE->visRect);

    for (i = 1; i < numBetween; i++)
      {
	  cE = &(iMap->entries[between[i]]);
	  cR = &(cE->visRect);

#ifdef DEC_MOTIF_EXTENSION
	if ( layout_rtol ) {
	  if (((bE->level > cE->level) &&
	       (IsAncestor(cE->widget, cE->widget))) ||
	      ((bR->x < cR->x) ||
	       ((bR->x == cR->x)  &&
		((bR->y > cR->y) ||
		 ((bR->y == cR->y))))))
	    {
		min = between[i];
		bE = &(iMap->entries[min]);
		bR = &(bE->visRect);
	    }
	}
	else {
	  if (((bE->level > cE->level) &&
	       (IsAncestor(cE->widget, cE->widget))) ||
	      ((bR->x > cR->x) ||
	       ((bR->x == cR->x)  &&
		((bR->y > cR->y) ||
		 ((bR->y == cR->y))))))
	    {
		min = between[i];
		bE = &(iMap->entries[min]);
		bR = &(bE->visRect);
	    }
	}
#else
	  if (((bE->level > cE->level) &&
	       (IsAncestor(cE->widget, cE->widget))) ||
	      ((bR->x > cR->x) ||
	       ((bR->x == cR->x)  &&
		((bR->y > cR->y) ||
		 ((bR->y == cR->y))))))
	    {
		min = between[i];
		bE = &(iMap->entries[min]);
		bR = &(bE->visRect);
	    }
#endif DEC_MOTIF_EXTENSION
      }
    return min;
}


#ifdef DEC_MOTIF_EXTENSION
static void VertOverlapSort(iMap, TOPPER, BOTTOMER, mapRtn, layout_rtol)
    XmNavigIMap	iMap;
    Dimension	TOPPER, BOTTOMER;
    Dimension	**mapRtn;
    Boolean	layout_rtol;
#else
static void VertOverlapSort(iMap, TOPPER, BOTTOMER, mapRtn)
    XmNavigIMap	iMap;
    Dimension	TOPPER, BOTTOMER;
    Dimension	**mapRtn;
#endif DEC_MOTIF_EXTENSION
{
    Dimension	below , *between ;
    Cardinal	numBetween;
    Dimension	above = TOPPER;
    Cardinal	curr = 0;

    between = (Dimension*)XtMalloc(sizeof(Dimension)*(iMap->numEntries));

    while (1)
      {
	  below = BOTTOMER;
	  while (numBetween = VertInBetween(iMap, 
					    above, 
					    below, 
					    &between))
	    {
#ifdef DEC_MOTIF_EXTENSION
		below = VertClosest(iMap, 
				    above, 
				    between, 
				    numBetween,
				    layout_rtol);
#else
		below = VertClosest(iMap, 
				    above, 
				    between, 
				    numBetween);
#endif DEC_MOTIF_EXTENSION
	    }
	  if (below == BOTTOMER)
	    {
		if (above != TOPPER)
		  above = TOPPER;
		else{
		    XtFree((char*)between);
		    return;
		}
	    }
	  else 
	    {
		above = (*mapRtn)[curr++] = below;
		iMap->entries[below].flags |= _XmSELECTED;
#ifdef notdef
		iMap->entries[below].visRect.y--;
#endif
	    }
      }
}

#else /* NEW_SORT */

static Boolean IsDirectRelation(first, second)
    Widget	first, second;
{
    Widget	w;

    for (w = first; !XtIsShell(w); w = XtParent(w))
      if (w == second)
	return True;

    for (w = second; !XtIsShell(w); w = XtParent(w))
      if (w == first)
	return True;
    
    return False;
}


static XmNavigIMapEntry	sortEntries = NULL;

static int VertItemCompare(uno, duo)
    unsigned short 	*uno, *duo;
{
    XmNavigIMapEntry	first, second;
    XRectangle		*firstRect, *secondRect;
    short		firstMidX, secondMidX;
    short		diffMidX, diffX;
    
    
    first 	= (XmNavigIMapEntry)&sortEntries[*uno];
    second 	= (XmNavigIMapEntry)&sortEntries[*duo];
    
    firstRect	= &(first->visRect);
    secondRect	= &(second->visRect);
    firstMidX	= firstRect->x + firstRect->width/2;
    secondMidX	= secondRect->x + secondRect->width/2;
    
    diffMidX 	= secondMidX - firstMidX;
    diffX 	= secondRect->x - firstRect->x;
    
    if (((diffMidX >= 0) && (diffX <= 0)) 
	||
	((diffMidX <= 0) && (diffX >= 0)))
      {
	  short		firstMidY, secondMidY;
	  short		diffMidY, diffY;
	  
	  firstMidY	= firstRect->y + firstRect->height/2;
	  secondMidY	= secondRect->y + secondRect->height/2;
	  diffMidY 	= secondMidY - firstMidY;
	  diffY	 	= secondRect->y - firstRect->y;
	  
	  if (diffMidY >= 0)
	    return -1;
	  else
	    return 1;
      }
    else if (diffMidX < 0)
      {
	  return 1;
      }
    else if (diffMidX > 0)
      {
	  return -1;
      }
    return 0;
}


static int VertTabCompare(uno, duo)
    unsigned short 	*uno, *duo;
{
    XmNavigIMapEntry	first, second;
    Widget		fw, sw;
    XRectangle		*firstRect, *secondRect;
    short		firstMidX, secondMidX;

    first 	= (XmNavigIMapEntry) &sortEntries[*uno];
    second 	= (XmNavigIMapEntry) &sortEntries[*duo];

    if ((first->level != second->level) &&
	IsDirectRelation(first->widget, second->widget))
      {
	  if (first->level < second->level)
	    return -1;
	  else if (first->level > second->level)
	    return  1;
      }
    else
      return VertItemCompare(uno, duo);
}



static int HorizItemCompare(uno, duo)
    unsigned short 	*uno, *duo;
{
    XmNavigIMapEntry	first, second;
    XRectangle		*firstRect, *secondRect;
    short		firstMidY, secondMidY;
    short		diffMidY, diffY;
    
    
    first 	= (XmNavigIMapEntry)&sortEntries[*uno];
    second 	= (XmNavigIMapEntry)&sortEntries[*duo];
    firstRect	= &(first->visRect);
    secondRect	= &(second->visRect);
    firstMidY	= firstRect->y + firstRect->height/2;
    secondMidY	= secondRect->y + secondRect->height/2;
    diffMidY 	= secondMidY - firstMidY;
    diffY 	= secondRect->y - firstRect->y;
    
    if (((diffMidY >= 0) && (diffY <= 0)) 
	||
	((diffMidY <= 0) && (diffY >= 0)))
      {
	  short		firstMidX, secondMidX;
	  short		diffMidX, diffX;
	  
	  firstMidX	= firstRect->x + firstRect->width/2;
	  secondMidX	= secondRect->x + secondRect->width/2;
	  diffMidX 	= secondMidX - firstMidX;
	  diffX	 	= secondRect->x - firstRect->x;
	  
	  if (diffMidX >= 0)
	    return -1;
	  else
	    return 1;
      }
    else if (diffMidY < 0)
      {
	  return 1;
      }
    else if (diffMidY > 0)
      {
	  return -1;
      }
    return 0;
}

static int HorizTabCompare(uno, duo)
    unsigned short 	*uno, *duo;
{
    XmNavigIMapEntry	first, second;
    Widget		fw, sw;
    XRectangle		*firstRect, *secondRect;
    short		firstMidX, secondMidX;

    first 	= (XmNavigIMapEntry) &sortEntries[*uno];
    second 	= (XmNavigIMapEntry) &sortEntries[*duo];


    if ((first->level != second->level) &&
	IsDirectRelation(first->widget, second->widget))
      {
	  if (first->level < second->level)
	    return -1;
	  else if (first->level > second->level)
	    return  1;
      }
    else
      return HorizItemCompare(uno, duo);
}
#endif /* NEW_SORT */

typedef enum { HorizList, VertList, BothLists } WhichLists;


static Cardinal GetLevel(group, w)
    Widget	group, w;
{
    Cardinal	i = 0;

    while (w  && (w != group))
      {
	  i++;
	  w = XtParent(w);
      }
    return i;
}


/*
 * this routine adds several widgets to the navigable set that are
 * needed for the selection of the item to traverse to. If the current
 * item if no longer traversable, then it is added to the set. We also
 * add special elements for outermost compares in the sort algorithm.
 * These are added as the last two elements in the iMap.
 */
static void AddSpecialsToImap(iMap, group, w, tabType, whichLists)
    XmNavigIMap		iMap;
    Widget		group;
    Widget		w;
    unsigned char	tabType;
    WhichLists		whichLists;
{ 
    Cardinal		i;
    Boolean		found = False;
    XRectangle	 	tmpRectRec;

    if (((tabType == XmTAB_GROUP) && 
	 (group == _XmGetTabGroup(group))) || 
	(group != w))
      {
	  for (i = 0; !found && (i < iMap->numEntries); i++)
	    if (iMap->entries[i].widget == w)
	      found = True;
	  
	  if (!found && w)
	    {
		_XmCreateVisibilityRect(w, &tmpRectRec);
		
		/* should be calculating a level ||| */
		AddToImap(iMap, 
			  w, 
			  &tmpRectRec, 
			  GetLevel(group, w),
			  _XmSPECIAL);
	    }
      }
#ifdef NEW_SORT
    tmpRectRec.y = -1;
    tmpRectRec.x = -1;
    tmpRectRec.width = 0;
    tmpRectRec.height = 0;
    switch (whichLists)
      {
	case HorizList:
	  tmpRectRec.height = 32000;
	  AddToImap(iMap, NULL, &tmpRectRec, 0,
		    _XmSELECTED | _XmSPECIAL);
	  tmpRectRec.x = 32000;
	  AddToImap(iMap, NULL, &tmpRectRec, 1023,
		    _XmSELECTED | _XmSPECIAL);
	  break;
	case VertList:
	  tmpRectRec.width = 32000;
	  AddToImap(iMap, NULL, &tmpRectRec, 0, 
		    _XmSELECTED | _XmSPECIAL);
	  tmpRectRec.y = 32000;
	  AddToImap(iMap, NULL, &tmpRectRec, 1023,
		    _XmSELECTED | _XmSPECIAL);
	  break;
      }
#endif /* NEW_SORT */
}


#define CopyImapToMap(iMapE, mapE) \
  mapE.widget = iMapE.widget;\

#ifdef DEC_MOTIF_EXTENSION
static XmNavigMap CalcNavigMap(iMap, group, w, tabType, whichLists, layout_rtol)
    XmNavigIMap		iMap;
    Widget		group;
    Widget		w;
    unsigned char	tabType;
    WhichLists		whichLists;
    Boolean		layout_rtol;
#else
static XmNavigMap CalcNavigMap(iMap, group, w, tabType, whichLists)
    XmNavigIMap		iMap;
    Widget		group;
    Widget		w;
    unsigned char	tabType;
    WhichLists		whichLists;
#endif DEC_MOTIF_EXTENSION
{ 
    XmNavigMap		map;
    Boolean		doVert, doHoriz;
    Cardinal		i;
    int			(*sortFunc)();

    AddSpecialsToImap(iMap,
		      group,
		      w, 
		      tabType, 
		      whichLists);

    switch (whichLists)
      {
	case BothLists:
	  doVert = doHoriz = True;
	  break;
	case HorizList:
	  doHoriz = True;
	  doVert = False;
	  break;
	case VertList:
	  doVert = True;
	  doHoriz = False;
	  break;
      }
    map = XtNew(XmNavigMapRec);
    map->numEntries = iMap->numEntries;


    if (doHoriz)
      map->horizList = 
	(unsigned short *) XtMalloc(sizeof(unsigned short) * iMap->numEntries);
    else
      map->horizList = NULL;

    if (doVert)
      map->vertList = 
	(unsigned short *) XtMalloc(sizeof(unsigned short) * iMap->numEntries);
    else
      map->vertList = NULL;

    map->entries = 
      (XmNavigMapEntry) XtMalloc(sizeof(XmNavigMapEntryRec) * iMap->numEntries);

    for (i = 0; i < map->numEntries; i++)
      {
	  if (doVert) map->vertList[i] = i;
	  if (doHoriz) map->horizList[i] = i;
	  CopyImapToMap(iMap->entries[i],map->entries[i]);
      }

#ifndef NEW_SORT
    /*
     * pass the sort routines the internal table containing the actual
     * data. 
     */
    sortEntries = iMap->entries;
#endif /* NEW_SORT */

    if (doHoriz)
      {
#ifdef NEW_SORT
#ifdef DEC_MOTIF_EXTENSION
	if ( layout_rtol )
	  HorizOverlapSort(iMap, 
			   iMap->numEntries-1,
			   iMap->numEntries-2,
			   &map->horizList,
			   layout_rtol);
	else
	  HorizOverlapSort(iMap, 
			   iMap->numEntries-2,
			   iMap->numEntries-1,
			   &map->horizList,
			   layout_rtol);
#else DEC_MOTIF_EXTENSION
	  HorizOverlapSort(iMap, 
			   iMap->numEntries-2,
			   iMap->numEntries-1,
			   &map->horizList);
#endif DEC_MOTIF_EXTENSION
#else
	  if (tabType == XmTAB_GROUP)
	    sortFunc = HorizTabCompare;
	  else
	    sortFunc = HorizItemCompare;

	  qsort((void *)map->horizList, 
		(size_t)map->numEntries, 
		(size_t)sizeof(unsigned short),
		sortFunc);
#endif /* NEW_SORT */
      }
    else if (doVert)
      {
#ifdef NEW_SORT
#ifdef DEC_MOTIF_EXTENSION
	  VertOverlapSort(iMap, 
			  iMap->numEntries-2,
			  iMap->numEntries-1,
			  &map->vertList,
			  layout_rtol);
#else DEC_MOTIF_EXTENSION
	  VertOverlapSort(iMap, 
			  iMap->numEntries-2,
			  iMap->numEntries-1,
			  &map->vertList);
#endif DEC_MOTIF_EXTENSION
#else
	  if (tabType == XmTAB_GROUP)
	    sortFunc = VertTabCompare;
	  else
	    sortFunc = VertItemCompare;
	  
  	  qsort((void *)map->vertList, 
		(size_t)map->numEntries, 
		(size_t)sizeof(unsigned short),
		sortFunc);
#endif /* NEW_SORT */	  
      }

#ifndef NEW_SORT
    sortEntries = NULL;
#else /* NEW_SORT */
    /*
     * make TOPPER and BOTTOMER invisible
     */
    map->numEntries =
      iMap->numEntries = 
	map->numEntries - 2;
#endif /* NEW_SORT */
    return map;
}
#undef CopyImapToMap


/*
 *  We are guaranteed that w is the current navigation widget
 */
#ifdef _NO_PROTO
Widget  _XmNavigate(w, direction)
    Widget	w;
    int		direction;
#else /* _NO_PROTO */
Widget _XmNavigate (Widget w, int direction)
#endif /* _NO_PROTO */
{
    WhichLists		whichLists;
    unsigned short	*list, curr;
    Widget		group = NULL;
    Widget		top;
    unsigned char	tabType;
    Widget		nw = NULL;
    Boolean		add;
    XmNavigMap		map = NULL;
    XmNavigIMap		iMap = NULL;
    WidgetNavigPtrsRec	npRec;
    WidgetNavigPtrs 	np = &npRec;
    XRectangle		visRectRec;
    Widget		shell = _XmFindTopMostShell(w);
    Boolean		bootstrap = False;
#ifdef DEC_MOTIF_EXTENSION
    Boolean		layout_rtol;
#endif DEC_MOTIF_EXTENSION

    switch (direction)
      {
	case XmTRAVERSE_NEXT_TAB_GROUP:
	case XmTRAVERSE_PREV_TAB_GROUP:
	  if (group = GetTopWidget(shell))
	    {
		if (w == shell)
		  {
		      w = group;
		      bootstrap = True;
		  }
		tabType = XmTAB_GROUP;
		whichLists = HorizList;
	    }
	  else
	    return NULL;
	  break;
	case XmTRAVERSE_RIGHT:
	case XmTRAVERSE_LEFT:
	  group = _XmFindTabGroup(w);
	  tabType = XmNONE;
	  whichLists = HorizList;
	  break;
	case XmTRAVERSE_UP:
	case XmTRAVERSE_DOWN:
	  group = _XmFindTabGroup(w);
	  tabType = XmNONE;
	  whichLists = VertList;
	  break;
	default:
	  XtWarning("_XmNavigate called with invalid direction");
	  return NULL;
	  break;
      }
#ifdef DEC_MOTIF_EXTENSION
      if ( XmIsManager(group) ) {
	XmManagerWidget w = (XmManagerWidget) group;
	layout_rtol = w->manager.dxm_layout_direction == DXmLAYOUT_LEFT_DOWN;
      }
#endif DEC_MOTIF_EXTENSION

    switch (direction)
      {
#ifdef DEC_MOTIF_EXTENSION
	case XmTRAVERSE_RIGHT:
	  if ( layout_rtol )
	     add = False;
	  else
	     add = True;
	  break;
#else
	case XmTRAVERSE_RIGHT:
#endif DEC_MOTIF_EXTENSION
	case XmTRAVERSE_NEXT_TAB_GROUP:
	case XmTRAVERSE_DOWN:
	  add = True;
	  break;
#ifdef DEC_MOTIF_EXTENSION
	case XmTRAVERSE_LEFT:
	  if ( layout_rtol )
	     add = True;
	  else
	     add = False;
	  break;
#else
	case XmTRAVERSE_LEFT:
#endif DEC_MOTIF_EXTENSION
	case XmTRAVERSE_PREV_TAB_GROUP:
	case XmTRAVERSE_UP:
	  add = False;
	  break;
      }

    /*
     * I use a simple algorithm for generating the navigation map. First
     * the entire hierarchy is traversed to generate the navigable set.
     * This is all widgets that are traversable and of appropriate type
     * for this context. An XmNavigIMap entry is added for each one that
     * can be used for the second pass.
     */
    GetWidgetNavigPtrs(group, np);
    (void)_XmCreateVisibilityRect((Widget)group, &visRectRec);
    
    iMap = CreateInternalNavigMap(group, tabType);
    
    if (tabType == XmTAB_GROUP)
      FindTabSet(iMap, group, np, &visRectRec, 0);
    else
      FindItemSet(iMap, group, np, &visRectRec, 0);
    
    if (iMap->numEntries)
      {
#ifdef DEC_MOTIF_EXTENSION
	  map = CalcNavigMap(iMap, group, w, tabType, whichLists, layout_rtol);
#else
	  map = CalcNavigMap(iMap, group, w, tabType, whichLists);
#endif DEC_MOTIF_EXTENSION
	  list = 
	    (whichLists == VertList) ? map->vertList : map->horizList;
      }
    else
      {
#ifdef DEBUG
	  XtWarning("null map in _XmNavigate");
#endif /* DEBUG */	  
	  DestroyInternalNavigMap(iMap);
	  return NULL;
      }

    for (curr = 0; curr < map->numEntries; curr++)
      if (map->entries[list[curr]].widget == w)
	break;

    if (curr == map->numEntries)
      {
	  if (add)
	    curr = 0;
	  else
	    curr = map->numEntries -1;
      }
    else if (!bootstrap)
      {
	  if (add)
	    {
		if (curr++ == map->numEntries - 1)
		  curr = 0;
	    }
	  else
	    {
		if (curr-- == 0)
		  curr = map->numEntries -1;
	    }
      }
    nw = map->entries[list[curr]].widget;

    DestroyInternalNavigMap(iMap);
    DestroyNavigMap(map);
    return nw;
}


