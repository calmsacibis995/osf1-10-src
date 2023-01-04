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
 * @(#)$RCSfile: dialog.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 22:04:42 $
 */
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif Release 1.2
 */


/* values which can be or'ed together */
#define RET_NONE        0
#define RET_OK          1
#define RET_CANCEL      2
#define RET_HELP        4
#define RET_DONE	8
#define RET_SAVE	16
#define RET_DISCARD	32
#define RET_APPLY       64

/* amount of time for working dialogs to remain on screen */
#define WORKING_TIME	5000

extern int 	Question();
extern int 	Warning();
extern int      Information() ;
extern int 	Error();
extern int      Selection();
extern int 	Working();
extern int 	SaveWarning();
extern int      PromptDialog();
extern int      FileSelectionDialog();


