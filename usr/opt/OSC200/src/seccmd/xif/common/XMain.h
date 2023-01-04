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
 *	@(#)$RCSfile: XMain.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:05:11 $
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

#if SEC_BASE

#ifndef __MAIN__
#define __MAIN__

/*
	filename:
	    XMain.h
	
	copyright:
	    Copyright (c) 1989-1990 SKM, L.P.
	    Copyright (c) 1989-1990 SecureWare Inc.
	    ALL RIGHTS RESERVED
	
	function:
	    include file for ISSO role program
*/

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

#define ISSO			1
#define SYS_ADMIN		2
#define OPERATOR		3

/* Load in main variables that are accessed by all files */
#ifndef MAIN
extern
#endif
int     
	no_form_present,		/* Used by error code */
	role_program,		/* Set to the role we are in */
	always_confirm,		/* If true, always confirm, otherwise never */
	save_memory;		/* If true destroy widgets on exit */

#ifndef MAIN
extern
#endif
Widget
	main_form,
	main_menubar,
	main_shell,
	mother_form;

#ifdef MAIN
XmStringCharSet charset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
#else
extern XmStringCharSet charset;
#endif

	/* Device size was changed with SW_3000 */
#define HOST_NAME_LEN		13
#define PRINTER_NAME_LEN	15
#define TAPE_NAME_LEN		15
#define DEVICE_NAME_LEN		100
#define TERMINAL_NAME_LEN	13	/* See prot.h for this field */

#define UNAMELEN		8
#define NGROUPNAME		9

#endif /* __MAIN__ */
#endif /* SEC_BASE */
