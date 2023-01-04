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
 * @(#)$RCSfile: DropTrans.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:36:56 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: DropTrans.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:36:56 $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDropTrans_h
#define _XmDropTrans_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XmTRANSFER_FAILURE 0
#define XmTRANSFER_SUCCESS 1

externalref WidgetClass xmDropTransferObjectClass;

typedef struct _XmDropTransferClassRec * XmDropTransferObjectClass;
typedef struct _XmDropTransferRec      * XmDropTransferObject;

#ifndef XmIsDropTransfer
#define XmIsDropTransfer(w) \
	XtIsSubclass((w), xmDropTransferObjectClass)
#endif /* XmIsDropTransfer */

typedef struct _XmDropTransferEntryRec {
	XtPointer	client_data;
	Atom		target;
} XmDropTransferEntryRec, * XmDropTransferEntry;

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmDropTransferStart() ;
extern void XmDropTransferAdd() ;

#else

extern Widget XmDropTransferStart( 
                        Widget refWidget,
                        ArgList args,
                        Cardinal argCount) ;
extern void XmDropTransferAdd( 
                        Widget widget,
                        XmDropTransferEntry transfers,
                        Cardinal num_transfers) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmDropTrans_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
