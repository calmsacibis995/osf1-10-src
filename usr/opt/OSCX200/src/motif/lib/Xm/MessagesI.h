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
 * @(#)$RCSfile: MessagesI.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:42:48 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
/*   $RCSfile: MessagesI.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:42:48 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmMessagesI_h
#define _XmMessagesI_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* The symbol _XmConst is used for constant data that cannot be
 * declared const in the header file because of usage as arguments to
 * routines which have string arguments that are not declared const.
 *
 * So, _XmConst is always defined to be nothing in header files.
 * In the source file, however, _XmConst is defined to be const,
 * so as to allow shared data in a shared library environment.
 */

#ifndef _XmConst
#define _XmConst
#endif


extern _XmConst char _XmMsgBaseClass_0000[] ;
extern _XmConst char _XmMsgBaseClass_0001[] ;
extern _XmConst char _XmMsgBaseClass_0002[] ;
extern _XmConst char _XmMsgBulletinB_0001[] ;
extern _XmConst char _XmMsgCascadeB_0000[] ;
extern _XmConst char _XmMsgCascadeB_0001[] ;
extern _XmConst char _XmMsgCascadeB_0002[] ;
extern _XmConst char _XmMsgCascadeB_0003[] ;
extern _XmConst char _XmMsgCommand_0000[] ;
extern _XmConst char _XmMsgCommand_0001[] ;
extern _XmConst char _XmMsgCommand_0002[] ;
extern _XmConst char _XmMsgCommand_0003[] ;
extern _XmConst char _XmMsgCommand_0004[] ;
extern _XmConst char _XmMsgCommand_0005[] ;
extern _XmConst char _XmMsgCutPaste_0000[] ;
extern _XmConst char _XmMsgCutPaste_0001[] ;
extern _XmConst char _XmMsgCutPaste_0002[] ;
extern _XmConst char _XmMsgCutPaste_0003[] ;
extern _XmConst char _XmMsgCutPaste_0004[] ;
extern _XmConst char _XmMsgCutPaste_0005[] ;
extern _XmConst char _XmMsgCutPaste_0006[] ;
extern _XmConst char _XmMsgCutPaste_0007[] ;
extern _XmConst char _XmMsgCutPaste_0008[] ;
extern _XmConst char _XmMsgCutPaste_0009[] ;
extern _XmConst char _XmMsgDialogS_0000[] ;
extern _XmConst char _XmMsgDragBS_0000[] ;
extern _XmConst char _XmMsgDragBS_0001[] ;
extern _XmConst char _XmMsgDragBS_0002[] ;
extern _XmConst char _XmMsgDragBS_0003[] ;
extern _XmConst char _XmMsgDragBS_0004[] ;
extern _XmConst char _XmMsgDragBS_0005[] ;
extern _XmConst char _XmMsgDragBS_0006[] ;
extern _XmConst char _XmMsgDragICC_0000[] ;
extern _XmConst char _XmMsgDragICC_0001[] ;
extern _XmConst char _XmMsgDragIcon_0000[] ;
extern _XmConst char _XmMsgDragIcon_0001[] ;
extern _XmConst char _XmMsgDragIcon_0002[] ;
extern _XmConst char _XmMsgDragOverS_0000[] ;
extern _XmConst char _XmMsgDragOverS_0001[] ;
extern _XmConst char _XmMsgDragOverS_0002[] ;
extern _XmConst char _XmMsgDragOverS_0003[] ;
extern _XmConst char _XmMsgDragUnder_0000[] ;
extern _XmConst char _XmMsgDragUnder_0001[] ;
extern _XmConst char _XmMsgForm_0000[] ;
extern _XmConst char _XmMsgForm_0002[] ;
extern _XmConst char _XmMsgForm_0003[] ;
extern _XmConst char _XmMsgForm_0004[] ;
extern _XmConst char _XmMsgGetSecRes_0000[] ;
extern _XmConst char _XmMsgLabel_0003[] ;
extern _XmConst char _XmMsgLabel_0004[] ;
extern _XmConst char _XmMsgList_0000[] ;
extern _XmConst char _XmMsgList_0005[] ;
extern _XmConst char _XmMsgList_0006[] ;
extern _XmConst char _XmMsgList_0007[] ;
extern _XmConst char _XmMsgList_0008[] ;
extern _XmConst char _XmMsgList_0009[] ;
extern _XmConst char _XmMsgList_0010[] ;
extern _XmConst char _XmMsgList_0011[] ;
extern _XmConst char _XmMsgList_0012[] ;
extern _XmConst char _XmMsgList_0013[] ;
extern _XmConst char _XmMsgMainW_0000[] ;
extern _XmConst char _XmMsgMainW_0001[] ;
extern _XmConst char _XmMsgManager_0000[] ;
extern _XmConst char _XmMsgMenuShell_0000[] ;
extern _XmConst char _XmMsgMenuShell_0001[] ;
extern _XmConst char _XmMsgMessageB_0003[] ;
extern _XmConst char _XmMsgMessageB_0004[] ;
extern _XmConst char _XmMsgNavigMap_0000[] ;
extern _XmConst char _XmMsgPanedW_0000[] ;
extern _XmConst char _XmMsgPanedW_0001[] ;
extern _XmConst char _XmMsgPanedW_0002[] ;
extern _XmConst char _XmMsgPanedW_0003[] ;
extern _XmConst char _XmMsgPanedW_0004[] ;
extern _XmConst char _XmMsgPanedW_0005[] ;
extern _XmConst char _XmMsgProtocols_0000[] ;
extern _XmConst char _XmMsgProtocols_0001[] ;
extern _XmConst char _XmMsgProtocols_0002[] ;
extern _XmConst char _XmMsgRegion_0000[] ;
extern _XmConst char _XmMsgResConvert_0000[] ;
extern _XmConst char _XmMsgRowColumn_0000[] ;
extern _XmConst char _XmMsgRowColumn_0001[] ;
extern _XmConst char _XmMsgRowColumn_0002[] ;
extern _XmConst char _XmMsgRowColumn_0003[] ;
extern _XmConst char _XmMsgRowColumn_0004[] ;
extern _XmConst char _XmMsgRowColumn_0005[] ;
extern _XmConst char _XmMsgRowColumn_0007[] ;
extern _XmConst char _XmMsgRowColumn_0008[] ;
extern _XmConst char _XmMsgRowColumn_0015[] ;
extern _XmConst char _XmMsgRowColumn_0016[] ;
extern _XmConst char _XmMsgRowColumn_0017[] ;
extern _XmConst char _XmMsgRowColumn_0018[] ;
extern _XmConst char _XmMsgRowColumn_0019[] ;
extern _XmConst char _XmMsgRowColumn_0020[] ;
extern _XmConst char _XmMsgRowColumn_0022[] ;
extern _XmConst char _XmMsgRowColumn_0023[] ;
extern _XmConst char _XmMsgRowColText_0024[] ;
extern _XmConst char _XmMsgRowColumn_0025[] ;
extern _XmConst char _XmMsgRowColumn_0026[] ;
extern _XmConst char _XmMsgRowColumn_0027[] ;
extern _XmConst char _XmMsgScale_0000[] ;
extern _XmConst char _XmMsgScale_0001[] ;
extern _XmConst char _XmMsgScale_0002[] ;
extern _XmConst char _XmMsgScaleScrBar_0004[] ;
extern _XmConst char _XmMsgScale_0005[] ;
extern _XmConst char _XmMsgScale_0006[] ;
extern _XmConst char _XmMsgScale_0007[] ;
extern _XmConst char _XmMsgScale_0008[] ;
extern _XmConst char _XmMsgScreen_0000[] ;
extern _XmConst char _XmMsgScrollBar_0000[] ;
extern _XmConst char _XmMsgScrollBar_0001[] ;
extern _XmConst char _XmMsgScrollBar_0002[] ;
extern _XmConst char _XmMsgScrollBar_0003[] ;
extern _XmConst char _XmMsgScrollBar_0004[] ;
extern _XmConst char _XmMsgScrollBar_0005[] ;
extern _XmConst char _XmMsgScrollBar_0006[] ;
extern _XmConst char _XmMsgScrollBar_0007[] ;
extern _XmConst char _XmMsgScrollBar_0008[] ;
extern _XmConst char _XmMsgScrolledW_0004[] ;
extern _XmConst char _XmMsgScrolledW_0005[] ;
extern _XmConst char _XmMsgScrolledW_0006[] ;
extern _XmConst char _XmMsgScrolledW_0007[] ;
extern _XmConst char _XmMsgScrolledW_0008[] ;
extern _XmConst char _XmMsgScrolledW_0009[] ;
extern _XmConst char _XmMsgScrollVis_0000[] ;
extern _XmConst char _XmMsgSelectioB_0001[] ;
extern _XmConst char _XmMsgSelectioB_0002[] ;
extern _XmConst char _XmMsgText_0000[] ;
extern _XmConst char _XmMsgText_0002[] ;
extern _XmConst char _XmMsgTextF_0000[] ;
extern _XmConst char _XmMsgTextF_0001[] ;
extern _XmConst char _XmMsgTextF_0002[] ;
extern _XmConst char _XmMsgTextF_0003[] ;
extern _XmConst char _XmMsgTextF_0004[] ;
extern _XmConst char _XmMsgTextF_0005[] ;
extern _XmConst char _XmMsgTextF_0006[] ;
extern _XmConst char _XmMsgTextFWcs_0000[] ;
extern _XmConst char _XmMsgTextFWcs_0001[] ;
extern _XmConst char _XmMsgTextIn_0000[] ;
extern _XmConst char _XmMsgTextOut_0000[] ;
extern _XmConst char _XmMsgVendor_0000[] ;
extern _XmConst char _XmMsgVendorE_0000[] ;
extern _XmConst char _XmMsgVendorE_0005[] ;
extern _XmConst char _XmMsgVisual_0000[] ;
extern _XmConst char _XmMsgVisual_0001[] ;
extern _XmConst char _XmMsgVisual_0002[] ;
extern _XmConst char _XmMsgXmIm_0000[] ;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmMessagesI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
