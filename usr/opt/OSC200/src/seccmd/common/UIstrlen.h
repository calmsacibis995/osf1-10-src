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
 *	@(#)$RCSfile: UIstrlen.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:01:03 $
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
#ifdef SEC_BASE
#ifndef _UIstrlen_h_
#define _UIstrlen_h_



/*
	filename:
	    UIstrlen.h
	
	copyright:
	    Copyright (c) 1989-1990 SKM, L.P.
	    Copyright (c) 1989-1990 SecureWare Inc.
	    Copyright (c) 1989-1990 MetaMedia Inc.
	    ALL RIGHTS RESERVED
	
	function:
	    lengths of various string components of UI programs
*/

	/* Device size was changed with SW_3000 */

#define HOST_NAME_LEN		13
#define PRINTER_NAME_LEN	15
#define TAPE_NAME_LEN		15
#define DEVICE_NAME_LEN		100
#define TERMINAL_NAME_LEN	13	/* See prot.h for this field */

/* limits for user account fields */

#define UNAMELEN	8
#define NUSERNAME	(UNAMELEN+1)

#define GNAMELEN	8
#define NGROUPNAME	(GNAMELEN+1)

#define NHOMEDIR	40
#define NSHELLNAME	40
#define NGECOS		40


#endif /* _UIstrlen_h_ */
#endif /* SEC_BASE */
