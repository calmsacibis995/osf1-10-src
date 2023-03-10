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
 * @(#)$RCSfile: app.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 18:36:51 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: app.h,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 18:36:51 $ */

/************************************************************
 *     app.h -- toolkit-independent code
 ************************************************************/

extern char *AppBufferName();

extern char *AppReadFile( );
extern void AppSaveFile( );
extern void AppTransferFile( );

extern void AppNewFile( );
extern AppRemoveFile();

extern AppOpenReadFile( );
extern AppOpenSaveFile( );
extern AppOpenTransferFile( );

extern void AppCompleteSaveAsFile( );
extern void AppCompleteCopyFile( );
extern AppCompleteMoveFile();


