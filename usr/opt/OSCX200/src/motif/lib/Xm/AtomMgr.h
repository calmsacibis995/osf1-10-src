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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: AtomMgr.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:27:57 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmAtomMgr_h
#define _XmAtomMgr_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XM_ATOM_CACHE

#ifdef XM_ATOM_CACHE
/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Atom XmInternAtom() ;
extern String XmGetAtomName() ;

#else

extern Atom XmInternAtom( 
                        Display *display,
                        String name,
#if NeedWidePrototypes
                        int only_if_exists) ;
#else
                        Boolean only_if_exists) ;
#endif /* NeedWidePrototypes */
extern String XmGetAtomName( 
                        Display *display,
                        Atom atom) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/
#else /* XM_ATOM_CACHE */
#define XmInternAtom(display, name, only_if_exists) \
		XInternAtom(display, name, only_if_exists)
#define XmGetAtomName(display, atom) \
		XGetAtomName(display, atom)
#endif /* XM_ATOM_CACHE */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#define XmNameToAtom(display, atom) \
        XmGetAtomName(display, atom)

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmAtomMgr_h */
