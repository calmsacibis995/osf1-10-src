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
 * @(#)$RCSfile: CallbackI.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:29:17 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: CallbackI.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:29:17 $ */
/*
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmCallbackI_h
#define _XmCallbackI_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include "XmI.h"

#ifdef __cplusplus
extern "C" {
#endif


#define _XtCBCalling 1
#define _XtCBFreeAfterCalling 2

typedef struct internalCallbackRec {
    unsigned short count;
    char           is_padded;   /* contains NULL padding for external form */
    char           call_state;  /* combination of _XtCB{FreeAfter}Calling */
#ifdef ALPHA_64BIT
    unsigned int  more_padding; /* pad to Alpha %8 boundry */
    /* 
     This is needed for ToList() macro to work properly, otherwise
     ToList generates an unaligned address for the following struct.
     ddhill
    */
#endif
    /* XtCallbackList */
} InternalCallbackRec, *InternalCallbackList;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmAddCallback() ;
extern void _XmRemoveCallback() ;
extern void _XmCallCallbackList() ;
extern void _XmRemoveAllCallbacks() ;

#else

extern void _XmAddCallback( 
                        InternalCallbackList *callbacks,
                        XtCallbackProc callback,
                        XtPointer closure) ;
extern void _XmRemoveCallback( 
                        InternalCallbackList *callbacks,
                        XtCallbackProc callback,
                        XtPointer closure) ;
extern void _XmCallCallbackList( 
                        Widget widget,
                        XtCallbackList callbacks,
                        XtPointer call_data) ;
extern void _XmRemoveAllCallbacks(
                        InternalCallbackList *callbacks) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmCallbackI_h */
/* DON'T ADD STUFF AFTER THIS #endif */
