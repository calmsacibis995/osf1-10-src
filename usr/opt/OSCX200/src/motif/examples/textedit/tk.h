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
 * @(#)$RCSfile: tk.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 18:54:29 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: tk.h,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 18:54:29 $ */
/************************************************************
 *     tk.h -- toolkit-specific dialogue layer
 ************************************************************/

#include "tkdef.h"

extern void TkBeep();
extern void TkExit();
extern void TkUpdateStatus();

extern TkTextChanged();
extern void TkTextActUnchangedSince();
extern TkTextChangedSince();

extern void TkTextClear();
extern void TkTextStore( );
extern char *TkTextRetrieve();

extern void TkAskFileToOpen();
extern void TkAskFileToSave();
extern void TkAskFileToCopy();
extern void TkAskFileToMove();
extern void TkDoneAskingFile();
extern void TkArrangeToOpen();

extern void TkAskSave();
extern void TkDoneAskingSave();

extern void TkWarn();
extern void TkWarnAndAskFileToSave();
extern void TkQuestionRemove();







