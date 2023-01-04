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
 *	@(#)$RCSfile: iostate.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:22:51 $
 */ 

/** Copyright (c) 1990  Mentat Inc.
 ** iostate.h 1.1, last change 4/14/90
 **/

#ifndef _IOSTATE_H
#define _IOSTATE_H

#ifndef	IOSTATE_LOCK
#define	IOSTATE_LOCK
#define	IOSTATE_UNLOCK
#endif

#define IOSTATE_LOOKUP          0x0001
#define IOSTATE_VERIFY  IOSTATE_LOOKUP
#define IOSTATE_CREATE          0x0011
#define IOSTATE_SETFLAG         0x0021
#define IOSTATE_CLEARFLAG       0x0041
#define IOSTATE_SET             0x0081
#define IOSTATE_USTATE          0x0101
#define IOSTATE_FREE            0x0100

#define IO_MORE_RUDATA		0x0020

#endif
