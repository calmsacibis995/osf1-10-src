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
 *	@(#)$RCSfile: msg04.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:17:56 $
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

/* msg04.h	5.1 - 86/12/09 - 05:59:15 */
/* msg04.h	5.1 86/12/09 05:59:15 */
#ifndef  _MSG04_H_
#define  _MSG04_H_ 

/***********************************************************************
*  msg04.h - Message Services Error Return Code Definitions            *
***********************************************************************/

#define  MSG_COMP -9001    /* component file could not be located     */
#define  MSG_INVL -9002    /* component file has invalid format       */
#define  MSG_MTCH -9003    /* component file contains wrong component */
#define  MSG_NONE -9004    /* req. msg/insert/help not in comp. file  */
#define  MSG_REFN -9005    /* text reference to invalid descriptor    */
/* (Above errors cause msg # 090-001 or 090-002 to be substituted.)   */

#define  MSG_TABI -9009    /* req. msg/insert not in message table    */

#define  MSG_CPID -9010    /* component ID is not six characters long */
#define  MSG_INDX -9011    /* index is not in the range 1-999         */
#define  MSG_TABP -9012    /* MSGFLTAB flag specified with help req.  */
#define  MSG_ALLO -9013    /* unable to allocate minimum work area    */
#define  MSG_SREG -9014    /* segment reg. not avail. for file mapping*/
#define  MSG_BADP -9015    /* msg table ptr doesn't point to msg table*/
#define  MSG_EXEC -9016    /* execl to /qmsg upd pgm (msgoutq) failed */
#define  MSG_QMSG -9017    /* can't open /qmsg, or its fmt is invalid */

#endif /* _MSG04_H_ */

