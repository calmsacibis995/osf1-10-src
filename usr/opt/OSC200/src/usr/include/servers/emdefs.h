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
 *	@(#)$RCSfile: emdefs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:11:25 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*  
 *  File: emdefs
 *	Exported definitions for Environment Maganger
 *  Author: Mary Thompson
 */

#ifndef _env_mgr_defs
#define _env_mgr_defs

#include <servers/errorlib.h>

#define env_name_size		(80)
#define env_val_size		(256)

#define ENV_SUCCESS			(KERN_SUCCESS)

#define ENV_VAR_NOT_FOUND	(SERV_ENV_MOD | 0)
#define ENV_WRONG_VAR_TYPE	(SERV_ENV_MOD | 1)
#define ENV_UNKNOWN_PORT	(SERV_ENV_MOD | 2)
#define ENV_READ_ONLY		(SERV_ENV_MOD | 3)
#define ENV_NO_MORE_CONN	(SERV_ENV_MOD | 4)
#define ENV_PORT_TABLE_FULL	(SERV_ENV_MOD | 5)
#define ENV_PORT_NULL		(SERV_ENV_MOD | 6)

typedef char	env_name_t[env_name_size];

typedef char	env_str_val_t[env_val_size];

typedef env_name_t *env_name_list; 	/* Variable sized array */

typedef env_str_val_t *env_str_list; 	/* Variable sized array */

#endif /* _env_mgr_defs */
