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
static char	*sccsid = "@(#)$RCSfile: uquit.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/11/26 11:46:04 $";
#endif 
/*
********************************************************************************
**                                                                            **
**                 (c) Copyright 1990, Open Software Foundation               **
**                             All rights reserved                            **
**    No part of this program may be photocopied, reproduced or translated    **
**    to another programming language or natural language without prior       **
**    written consent of Open Software Foundation.                            **
**                                                                            **
********************************************************************************
**
**    Description:
**	These are functions for library librad.a.
**
**    written by:
**                   Randy J. Barbano
**                Open Software Foundation
**                    Cambridge, MA
**                     April 1990
**
**    lib functions and their usage:
**	1) uquit ( exit_value, usage, function_name, format, arg1, arg2... )
**	   args:
**	     int      exit_value;		-1, 0, 1, etc
**	     boolean  usage; 			true or false - 1 or 0
**	     char     function_name [], 	where error was found
**	     	      format [],	 	"%s has value %d"
**	     	      args1 [], arg2 []...;	arguments to format
**
**	   returns:
**	     does not returns, calls exit with exit_value.
**
**	   usage:
**	     This procedure prints out an error message and exits the
**	     the program that called it.  It uses function_name to 
**	     indicate what was running when the failure occured, and
**	     prints out the error message from format.  Format is like
**	     a printf statement in that it can take a varying number of
**	     arguments.
**
**	     If usage is not set to 0, uquit will call the print_usage
**	     program before it exits with the value in exit_value.
**
**	     NOTE: print_usage is a routine called by uquit.  The
**	           user must create this procedure to use uquit.
**
**    functions called by lib functions:
**	1) uquit
**	   a) print_usage ()		user must create one
**
**    known limitations/defects:
**
**    copyright
**
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
**
**    modification history:
**
 * OSF/1 Release 1.0
**



                                                                              */
#  include <sdm/std_defs.h>
#  include <stdio.h>
#  include <varargs.h>


uquit ( va_alist )

	/* This procedure takes a variable length argument list and
	   prints out the name of the function that failed, the
	   error message, usage if asked for, and then exits with
	   the code entered. */

va_dcl

{
    va_list 	args;		 /* see vprintf(3) and varargs(5) for details */
    int		status,				       /* status to exit with */
        	usage;					/* do you print usage */
    char      * fmt;					     /* format string */

  fflush ( stdout );

  va_start ( args );

  status = va_arg ( args, int );	  /* gets the first argument and type */
  usage = va_arg ( args, int );		 /* gets the second argument and type */

  ( void ) fprintf ( stderr, "ERROR in %s:\n", va_arg ( args, char * ));
					       /* print out location of error */
  fmt = va_arg ( args, char * );
  ( void ) vfprintf ( stderr, fmt, args ); 	   /* print out error message */

  if ( usage )
    print_usage ();

  va_end ( args );
  exit ( status );
}								     /* uquit */



/* print_usage () 

	 * Dummy routine.  User should create a real one that
	   prints real usage information. * 

{
}							        * print usage */
