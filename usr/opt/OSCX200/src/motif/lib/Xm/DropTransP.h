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
/*   $RCSfile: DropTransP.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/15 16:28:18 $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDropTransferP_h
#define _XmDropTransferP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/DropTrans.h>
#include <Xm/XmP.h>


#ifdef __cplusplus
extern "C" {
#endif

/*  DropTransfer class structure  */

#ifdef _NO_PROTO
typedef Widget (*XmDropTransferStartTransferProc)();
typedef void (*XmDropTransferAddTransferProc)();
#else
typedef Widget (*XmDropTransferStartTransferProc)(Widget,
	ArgList, Cardinal);
typedef void (*XmDropTransferAddTransferProc)(Widget,
	XmDropTransferEntry, Cardinal);
#endif

typedef struct _XmDropTransferClassPart
{
	XmDropTransferStartTransferProc	start_drop_transfer;
	XmDropTransferAddTransferProc	add_drop_transfer;
	XtPointer extension;
} XmDropTransferClassPart;

/*  Full class record declaration */

typedef struct _XmDropTransferClassRec
{
   ObjectClassPart        object_class;
   XmDropTransferClassPart dropTransfer_class;
} XmDropTransferClassRec;

#ifdef DEC_MOTIF_BUG_FIX
externalref XmDropTransferClassRec xmDropTransferClassRec;
#else
extern XmDropTransferClassRec xmDropTransferClassRec;
#endif


typedef struct _XmDropTransferListRec {
	XmDropTransferEntry	transfer_list;
	Cardinal		num_transfers;
} XmDropTransferListRec, * XmDropTransferList;


/*  The DropTransfer instance record  */

typedef struct _XmDropTransferPart
{
    XmDropTransferEntry		drop_transfers;
    Cardinal			num_drop_transfers;
    Atom			selection;
    Widget			dragContext;
    Time			timestamp;
    Boolean			incremental;
    Window			source_window;
    unsigned int		tag;
    XtSelectionCallbackProc 	transfer_callback;
    unsigned char		transfer_status;

    Atom 			motif_drop_atom;
    
    XmDropTransferList		drop_transfer_lists;
    Cardinal			num_drop_transfer_lists;
    Cardinal			cur_drop_transfer_list;
    Cardinal			cur_xfer;
    Atom *			cur_targets;
    XtPointer *			cur_client_data;
} XmDropTransferPart;

/*  Full instance record declaration  */

typedef struct _XmDropTransferRec
{
	ObjectPart	object;
	XmDropTransferPart dropTransfer;
} XmDropTransferRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#if defined(__cplusplus) || defined(c_plusplus)
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmDropTransferP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
