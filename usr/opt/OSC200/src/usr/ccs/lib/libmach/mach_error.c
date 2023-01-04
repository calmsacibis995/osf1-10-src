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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: mach_error.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:10:59 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *   mach_error.c
 *	interprets structured mach error codes and prints
 *	or returns a descriptive string. Will map old style
 *	unstructured error codes to new style if necessary.
 */

#include <stdio.h>
#include <mach/error.h>
#include <mach/message.h>	/* compatibility */
#include <mig_errors.h>		/* compatibility */
#include <servers/netname.h>	/* compatibility */
#include <servers/env_mgr.h>	/* compatibility */
#include <servers/ipcx_types.h>	/* compatibility */
#include <servers/errorlib.h>
#include <mach_error.h>

static
do_compat( org_err )
	mach_error_t		* org_err;
{
	mach_error_t		err = *org_err;

	/* 
	 * map old error numbers to 
	 * to new error sys & subsystem 
	 */
	/* -100 */
	if( (err <= SEND_ERRORS_START) && (err > SEND_ERRORS_START-100) ) {
		err = -(err - SEND_ERRORS_START) | IPC_SEND_MOD;
	} else	/* -200 */
	if( (err <= RCV_ERRORS_START) && (err > RCV_ERRORS_START-100) ) {
		err = -(err - RCV_ERRORS_START) | IPC_RCV_MOD;
	} else	/* -300 */
	if( (err <= MIG_TYPE_ERROR) && (err > MIG_TYPE_ERROR-100) ) {
		err = -(err - MIG_TYPE_ERROR) | IPC_MIG_MOD;
	} else	/* 1000 */
	if( (err >= NAME_NOT_YOURS) && (err < NAME_NOT_YOURS+100) ) {
		err = (err - NAME_NOT_YOURS) | SERV_NETNAME_MOD;
	} else  /* 1600  used to be environment manager codes */
	if( (err >= 1600) && (err < 1700) ) {
		err = (err - 1600) | SERV_ENV_MOD;
	} else	/* 27600 */
	if( (err >= IPC_ERROR_BASE) && (err < IPC_ERROR_BASE+100) ) {
		err = (err - IPC_ERROR_BASE) | SERV_EXECD_MOD;
	};
	   

	*org_err = err;
}


void
mach_error( str, err )	
	char	*str;
	mach_error_t		err;
{
	char * err_str;
	char buf[1024];

	fputs( str, stderr );
	putc( ' ', stderr );

	do_compat( &err );

	if ( strcmp( err_str=mach_error_string(err), NO_SUCH_ERROR ) == 0 ) {
		sprintf( buf, "%s %s (%x)", mach_error_type(err), err_str, err );
		err_str = buf;
	}

	fputs( err_str, stderr );
	putc( '\n', stderr );
	
}


char *
mach_error_type( err )
	mach_error_t		err;
{
	int sub, system;

	do_compat( &err );

	sub = err_get_sub(err);
	system = err_get_system(err);

	if (system > err_max_system) return( "(?/?)" );
	if (sub >= errors[system].max_sub) return( errors[system].bad_sub );
	return( errors[system].subsystem[sub].subsys_name );
}

char *
mach_error_string( err )
	mach_error_t		err;
{
	int sub, system, code;

	do_compat( &err );

	sub = err_get_sub(err);
	system = err_get_system(err);
	code = err_get_code(err);

	if (system > err_max_system) return( "(?/?) unknown error system" );
	if (sub >= errors[system].max_sub) return( errors[system].bad_sub );
	if (code >= errors[system].subsystem[sub].max_code) return ( NO_SUCH_ERROR );
	return( errors[system].subsystem[sub].codes[code] );
}


