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
 *	@(#)$RCSfile: rex.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:20:40 $
 */ 
/*
 */
/*
 * OSF/1 Release 1.0
 */

/* 
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 * @(#) from SUN 1.3
 */

/*
 * rex - remote execution server definitions
 */

#ifndef _rpcsvc_rex_h
#define _rpcsvc_rex_h

#define	REXPROG		100017
#define	REXPROC_NULL	0	/* no operation */
#define	REXPROC_START	1	/* start a command */
#define	REXPROC_WAIT	2	/* wait for a command to complete */
#define	REXPROC_MODES	3	/* send the tty modes */
#define REXPROC_WINCH	4	/* signal a window change */
#define REXPROC_SIGNAL	5	/* other signals */

#define	REXVERS	1

/* flags for rst_flags field */
#define REX_INTERACTIVE		1	/* Interative mode */

struct rex_start {
  /*
   * Structure passed as parameter to start function
   */
	char	**rst_cmd;	/* list of command and args */
	char	*rst_host;	/* working directory host name */
	char	*rst_fsname;	/* working directory file system name */
	char	*rst_dirwithin;	/* working directory within file system */
	char	**rst_env;	/* list of environment */
	u_short	rst_port0;	/* port for stdin */
	u_short	rst_port1;	/* port for stdin */
	u_short	rst_port2;	/* port for stdin */
	u_long	rst_flags;	/* options - see #defines above */
};

bool_t xdr_rex_start();

struct rex_result {
  /*
   * Structure returned from the start function
   */
   	int	rlt_stat;	/* integer status code */
	char	*rlt_message;	/* string message for human consumption */
};
bool_t xdr_rex_result();

struct rex_ttymode {
    /*
     * Structure sent to set-up the tty modes
     */
	struct sgttyb basic;	/* standard unix tty flags */
	struct tchars more;	/* interrupt, kill characters, etc. */
	struct ltchars yetmore;	/* special Bezerkeley characters */
	u_long andmore;		/* and Berkeley modes */
};

bool_t xdr_rex_ttymode();
bool_t xdr_rex_ttysize();

#endif /*!_rpcsvc_rex_h*/
