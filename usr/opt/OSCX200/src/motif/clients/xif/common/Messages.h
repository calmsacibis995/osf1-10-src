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
 * @(#)$RCSfile: Messages.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/03 13:00:54 $
 */

/******************************************************************************* *
 * File: Messages.h
 *
 *
 *
 ******************************************************************************/

/*
 * Public Exported Functions
 */
extern void MessagesSetMrmHierarchy(MrmHierarchy herarchy);

extern void PostWorkingDialog(char    *msgname, 
                              Widget   parent,
                              void   (*okCB)(),
                              XtPointer okData,
                              void   (*cancelCB)(),
                              XtPointer cancelData,
                              void   (*helpCB)(),
                              XtPointer helpData);

extern void UnPostWorkingDialog(Widget parent);

extern void PostErrorDialog(char    *msgname, 
                            Widget   parent,
                            void   (*okCB)(),
                            XtPointer okData,
                            void   (*cancelCB)(),
                            XtPointer cancelData,
                            void   (*helpCB)(),
                            XtPointer helpData);

extern void UnPostErrorDialog(Widget parent);

extern void PostQuestionDialog(char    *msgname, 
                              Widget   parent,
                              void   (*okCB)(),
                              XtPointer okData,
                              void   (*cancelCB)(),
                              XtPointer cancelData,
                              void   (*helpCB)(),
                              XtPointer helpData);

extern void UnPostQuestionDialog(Widget parent);

/*
 * List of Error Message Indexs
 */
#define ERR_SAVE                  0
#define ERR_UNKNOWN_USERNAME      1
#define ERR_UNKNOWN_TMPLTNAME     2
#define LAST_ERROR_MSG            ERR_UNKNOWN_TMPLTNAME

#define WRK_SAVE_USER_ACC         LAST_ERROR_MSG+1
