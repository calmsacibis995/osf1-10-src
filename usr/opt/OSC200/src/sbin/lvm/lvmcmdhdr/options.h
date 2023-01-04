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
 *	@(#)$RCSfile: options.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:47:56 $
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
 *   options.h
 *   
 *   Contents:
 *	Command line parsing has been highly automatised, in the sense
 *	that all options and arguments legal to each command have been
 *	extracted by the same source of information from which the manual
 *	pages were generated. Here are the functions to handle this
 *	information.
 *	See the LVM commands library module "options.c".
 */

int parse_args(int *argcp, char ***argvp, char *opt_without_val,
   	       char *opt_with_val, unsigned int required_args);
int used_opt(char key);
char *value_of_opt(char key);
char *program_name();
char *next_arg();
char **left_arg(int *numptr);
int bad_int_arg_value(char *flagptr, int *valptr, char flag, char *flagmeaning);
int bad_char_arg_value(char *flagptr, char *valptr, char flag,
		       char *flagmeaning, char *goodvalues);
int usage_error(char *bad_inp, char *err_msg);
int print_arg_error();
int print_usage(register char *usage_msg);
