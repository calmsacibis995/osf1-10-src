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
 *	@(#)$RCSfile: ldr_macro_help.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:37:00 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_macro_help.h
 * Macros to assist in writing complex macros
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_LDR_MACRO_HELP
#define _H_LDR_MACRO_HELP

/* A multi-statement macro should always start with a MACRO_BEGIN and
 * end with a MACRO_END, to avoid possible dangling-else problems with
 * using the macro in "if" statements.
 */

#define		MACRO_BEGIN	do {
#define		MACRO_END	} while (0)

#endif /* _H_LDR_MACRO_HELP */
