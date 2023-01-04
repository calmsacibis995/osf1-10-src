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
 *	@(#)$RCSfile: msg06.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:17:59 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/* msg06.h	5.1 - 86/12/09 - 05:59:23 */
/* msg06.h	5.1 86/12/09 05:59:23 */
#ifndef  _MSG06_H_
#define  _MSG06_H_

/***********************************************************************
*  msg06.h - Message Services External Variable Declarations           *
***********************************************************************/

    extern int   msgvi1;           /* integer variable 1              */
    extern int   msgvi2;           /* integer variable 2              */
    extern long  msgvl1;           /* long integer variable 1         */
    extern long  msgvl2;           /* long integer variable 2         */
    extern char  *msgvc1;          /* pointer to variable string 1    */
    extern char  *msgvc2;          /* pointer to variable string 2    */
    extern char  *msgvc3;          /* pointer to variable string 3    */
    extern int   msgvt1;           /* index to variable text insert 1 */
    extern int   msgvt2;           /* index to variable text insert 2 */
    extern int   msgvt3;           /* index to variable text insert 3 */
    extern char  *msgpath;         /* pointer to optional path prefix */

#endif /* _MSG06_H_ */
