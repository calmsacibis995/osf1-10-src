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
 *	@(#)$RCSfile: parse_cmd_line.h,v $ $Revision: 4.3 $ (DEC) $Date: 1991/09/21 17:12:38 $
 */ 
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
**	This header file is for the par_cmd_line library routine which
**	parses the command line arguments in a standard way.
**
**  NOTE: this file needs a full description and example put right here!
**
**    written by:
**                   Randy J. Barbano
**                Open Software Foundation
**                    Cambridge, MA
**                     April 1990
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
/*                       DEFINES					      */

#  define  UNLIMITED	-1      /* if the number of fiels should be unlimited */

/*                       GLOBAL DECLARATIONS				      */

	/* A structure to hold each argument as a string and a pointer
	   to the next arguments. NOTE: names are purposely misspelled
	   and long so they won't likely be used else where. */

  typedef struct _argmentvalues {
    char      *	field;				      /* string with argument */
    struct	_argmentvalues	* nextfield;	  /* pointer to next argument */
  } VALUES;


	/* An array of this structure is created, one per flag plus
	   one for the arguments without dashes.  This structure holds
	   the information about each flag after it has been parsed. */

  typedef struct _argmentfields {
    char      *	flag;		       		       /* string listing flag */
    int		is_set,				  /* boolean, is the flag set */
		field_ct,	 /* number of fields filled for option or arg */
		min_num_fields, /* if used, minimum # fields following option */
		max_num_fields; /* if used, maximum # fields following option */
    VALUES    *	value;				 /* the strings with the args */
  } ARGS;
  
