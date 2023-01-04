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
/*   $RCSfile: DisplayI.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/15 16:17:15 $ */
/*
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmDisplayI_h
#define _XmDisplayI_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmDisplayEventQueryStruct {
    XmDisplay			dd;
    XmDragContext		dc;
    XmTopLevelEnterCallbackStruct	*enterCB;
    XmDragMotionCallbackStruct		*motionCB;
    XmTopLevelLeaveCallbackStruct	*leaveCB;
    XmDropStartCallbackStruct		*dropStartCB;
    Boolean			hasEnter;
    Boolean			hasMotion;
    Boolean			hasLeave;
    Boolean			hasDropStart;
} XmDisplayEventQueryStruct;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmDisplayI_h */
/* DON'T ADD STUFF AFTER THIS #endif */
