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
/*   $RCSfile: ProtocolsP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:44:09 $ */
/*
*  (c) Copyright 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmProtocolsP_h
#define _XmProtocolsP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Protocols.h>
#include <Xm/ExtObjectP.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmProtocolClassPart{
    XtPointer	extension;
}XmProtocolClassPart;

typedef struct _XmProtocolClassRec{
    ObjectClassPart	object_class;
    XmExtClassPart	ext_class;
    XmProtocolClassPart	protocol_class;
}XmProtocolClassRec, *XmProtocolObjectClass;

typedef struct _XmProtocolPart{
    XtCallbackRec	pre_hook, post_hook;
    XtCallbackList	callbacks;
    Atom		atom;
    Boolean		active;
} XmProtocolPart, *XmProtocolPartPtr;

typedef struct _XmProtocolRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmProtocolPart		protocol;
}XmProtocolRec, *XmProtocol, **XmProtocolList;

#ifndef XmIsProtocol
#define XmIsProtocol(w) XtIsSubclass(w, xmProtocolObjectClass)
#endif /* XmIsProtocol */

/* Class record constants */

externalref XmProtocolClassRec 	xmProtocolClassRec;
externalref WidgetClass xmProtocolObjectClass;

typedef struct _XmProtocolMgrRec{
    Atom		property;
    XmProtocolList 	protocols;
    Cardinal		num_protocols;
    Cardinal		max_protocols;
}XmProtocolMgrRec, *XmProtocolMgr, **XmProtocolMgrList;


typedef struct _XmAllProtocolsMgrRec{
  XmProtocolMgrList	protocol_mgrs;
  Cardinal		num_protocol_mgrs;
  Cardinal		max_protocol_mgrs;
  Widget		shell;
}XmAllProtocolsMgrRec, *XmAllProtocolsMgr;
    

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmInstallProtocols() ;

#else

extern void _XmInstallProtocols( 
                        Widget w) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmProtocolsP_h */
