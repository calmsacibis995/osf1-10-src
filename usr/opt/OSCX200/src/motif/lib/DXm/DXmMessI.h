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
 * @(#)$RCSfile: DXmMessI.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/07 01:31:48 $
 */

/*
*****************************************************************************

              Copyright (c) Digital Equipment Corporation, 1990,1991,1992,1993
              All Rights Reserved.  Unpublished rights reserved
              under the copyright laws of the United States.

              The software contained on this media is proprietary
              to and embodies the confidential technology of
              Digital Equipment Corporation.  Possession, use,
              duplication or dissemination of the software and
              media is authorized only pursuant to a valid written
              license from Digital Equipment Corporation.

              RESTRICTED RIGHTS LEGEND   Use, duplication, or
              disclosure by the U.S. Government is subject to
              restrictions as set forth in Subparagraph (c)(1)(ii)
              of DFARS 252.227-7013, or in FAR 52.227-19, as
              applicable.

*****************************************************************************
**++
**  FACILITY:
**
**      DECwindows Toolkit
**
**  ABSTRACT:
**
**	DXm Message facilty for characters arrays to replace statically
**	defined messages      
**
**
**  MODIFICATION HISTORY:
**
**      1 Feb 1993  Begin work from MessagesI.h in Motif 1.2.1
**
**
**--
**/

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


/* Declare DXm character arrays. */

extern _XmConst char _DXmMsgColorMix_0000[] ;
extern _XmConst char _DXmMsgPane_0000[] ;
extern _XmConst char _DXmMsgPane_0001[] ;
extern _XmConst char _DXmMsgPrintWgt_0000[] ;
extern _XmConst char _DXmMsgPrintWgt_0001[] ;
extern _XmConst char _DXmMsgPrintWgt_0002[] ;
extern _XmConst char _DXmMsgPrintWgt_0003[] ;
extern _XmConst char _DXmMsgPrintWgt_0004[] ;
extern _XmConst char _DXmMsgPrintWgt_0005[] ;
extern _XmConst char _DXmMsgPrintWgt_0006[] ;
extern _XmConst char _DXmMsgPrintWgt_0007[] ;
extern _XmConst char _DXmMsgPWgtName_0000[] ;
extern _XmConst char _DXmMsgPWgtName_0001[] ;
extern _XmConst char _DXmMsgPWgtName_0002[] ;
extern _XmConst char _DXmMsgPWgtName_0003[] ;
extern _XmConst char _DXmMsgPWgtName_0004[] ;
extern _XmConst char _DXmMsgPWgtName_0005[] ;
extern _XmConst char _DXmMsgPWgtName_0006[] ;
extern _XmConst char _DXmMsgPWSend_0000[] ;
extern _XmConst char _DXmMsgPWSend_0001[] ;
extern _XmConst char _DXmMsgPWSend_0002[] ;
extern _XmConst char _DXmMsgPWSendName_0000[] ;
extern _XmConst char _DXmMsgPWSendName_0001[] ;
extern _XmConst char _DXmMsgPWSendName_0002[] ;
extern _XmConst char _DXmMsgFAO_0000[] ;
extern _XmConst char _DXmMsgFAOName_0000[] ;
extern _XmConst char _DXmMsgHlpShl_0000[] ;
extern _XmConst char _DXmMsgHlpShlName_0000[] ;
extern _XmConst char _DXmMsgHlpWid_0000[] ;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif

#endif /* _XmMessagesI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
