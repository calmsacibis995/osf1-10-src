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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: AtomMgr.c,v $ $Revision: 1.1.6.3 $ $Date: 1993/07/15 15:54:10 $"
#endif
#endif
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#include <Xm/XmP.h>
#include <X11/Xresource.h>
#include "AtomMgrI.h"
  

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


static XContext 	atomToNameContext = (XContext) NULL;
static XContext		nameToAtomContext = (XContext) NULL;
static Boolean		firstTime = True;

/*****************************************************************************
 *
 *  XmInternAtom()
 *
 ****************************************************************************/

Atom 
#ifdef _NO_PROTO
XmInternAtom( display, name, only_if_exists )
        Display *display ;
        String name ;
        Boolean only_if_exists ;
#else
XmInternAtom(
        Display *display,
        String name,
#if NeedWidePrototypes
        int only_if_exists )
#else
        Boolean only_if_exists )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
    XrmQuark		atomQuark;
    Atom		atomReturn = None;

    if (name == NULL) return(None);
        
    if (firstTime) {
        firstTime = False;
	_XmInitAtomPairs (display);
    }

    if (nameToAtomContext == (XContext) NULL)
      nameToAtomContext = XUniqueContext();
    
    if (atomToNameContext == (XContext) NULL)
      atomToNameContext = XUniqueContext();
    
    atomQuark = XrmStringToQuark(name);
    
    if (XFindContext(display, 
		     (Window) atomQuark,  
		     nameToAtomContext, 
		     (char **)&atomReturn))
      {
	  atomReturn = XInternAtom(display, name, only_if_exists);
	  
	  if ((!only_if_exists) || (atomReturn != None))
	    {
		(void) XSaveContext(display, 
				    (Window) atomQuark, 
				    nameToAtomContext, 
				    (XPointer) atomReturn);
		(void) XSaveContext(display, 
				    (Window) atomReturn, 
				    atomToNameContext, 
				    (XPointer) atomQuark);
	    }
      }
    return atomReturn;
}

/*****************************************************************************
 *
 *  XmGetAtomName()
 *
 ****************************************************************************/
    
String 
#ifdef _NO_PROTO
XmGetAtomName( display, atom )
        Display *display ;
        Atom atom ;
#else
XmGetAtomName(
        Display *display,
        Atom atom )
#endif /* _NO_PROTO */
{
    XrmQuark	atomQuark;
    String	atomName, nameRtn;
    
    if (nameToAtomContext == (XContext) NULL)
      nameToAtomContext = XUniqueContext();
    
    if (XFindContext(display, 
		     (Window) atom,
		     atomToNameContext, 
		     (char **)&atomQuark))	
      {
	  atomName = XGetAtomName(display, atom);
	  
	  /* get it from the server and register it */
	  
	  atomQuark = XrmStringToQuark(atomName);
	  
	  (void) XSaveContext(display, 
			      (Window) atomQuark, 
			      nameToAtomContext, 
			      (XPointer) atom);
	  (void) XSaveContext(display, 
			      (Window) atom, 
			      atomToNameContext, 
			      (XPointer) atomQuark);
	  
	  nameRtn = XtNewString(atomName);
	  XFree(atomName);
      }
    else 
      nameRtn = XtNewString(XrmQuarkToString(atomQuark));
    
    return (nameRtn);
}    

/*****************************************************************************
 *
 *  _XmInternAtomAndName()
 *
 ****************************************************************************/

void
#ifdef _NO_PROTO
_XmInternAtomAndName( display, atom, name )
        Display *display ;
        Atom atom ;
        String name ;
#else
_XmInternAtomAndName(
        Display *display,
	Atom atom,
        String name )
#endif /* _NO_PROTO */
{
    XrmQuark		atomQuark;
    Atom		atomReturn = None;
    
    if (nameToAtomContext == (XContext) NULL)
      nameToAtomContext = XUniqueContext();
    
    if (atomToNameContext == (XContext) NULL)
      atomToNameContext = XUniqueContext();
    
    atomQuark = XrmStringToQuark(name);
    
    if (XFindContext(display, 
		     (Window) atomQuark,  
		     nameToAtomContext, 
		     (char **)&atomReturn))
    {
	(void) XSaveContext(display, 
			    (Window) atomQuark, 
			    nameToAtomContext, 
			    (XPointer) atom);
	(void) XSaveContext(display, 
			    (Window) atom, 
			    atomToNameContext, 
			    (XPointer) atomQuark);
    }
}

