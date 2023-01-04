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
 *	@(#)$RCSfile: msg08.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:18:02 $
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

/* msg08.h	5.1 - 86/12/09 - 05:59:32 */
/* msg08.h	5.1 86/12/09 05:59:32 */
#ifndef _MSG08_H_
#define _MSG08_H_   

/************************************************************************
*  msg08.h - Structure Declarations for Message Table (used by msg07.h) *
************************************************************************/

/*** Format of message definition ***/
typedef struct {
       short helpindx;       /* help index #               */
       char sect_id[3];      /* Messages Manual section ID */
       char msg_id[3];       /* Messages Manual message ID */
       char *msgtextp;       /* ptr to message text        */
       } msg__msg;

/*** Format of text insert definition ***/
typedef char *msg__ins;

/*** Format of "Message Table" ***/
typedef struct {
       msg__msg *msgtabp;    /* ptr to message definition table        */
       msg__ins *instabp;    /* ptr to text insert definition table    */
       int  num_msg;         /* # of messages defined                  */
       int  num_ins;         /* # of text inserts defined              */
       char uniq[4];         /* unique ID ("MtAb") to verify table ptr */
       } msg__table;

#endif /* _MSG08_H_ */
