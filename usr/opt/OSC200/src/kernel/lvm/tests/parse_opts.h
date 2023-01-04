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
 *	@(#)$RCSfile: parse_opts.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:29:15 $
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

struct values {
	char *keyword;
	int value;
	int mask;
};
struct option {
	char *name;
	int minlength;
	int *value;
	int *mask;
	int arg_type;
	struct values *values;
};

#if __STDC__
#define name_value(X) { #X, X, (~0) }
#define bitname_value(X) { #X, X, X }
#else
#define name_value(X) { "X", X, (~0) }
#define bitname_value(X) { "X", X, X }
#endif

#define ARG_BOOL 0
#define ARG_INT 1

extern int optind;
extern struct option opts[];
