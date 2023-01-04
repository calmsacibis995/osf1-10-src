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
static char	*sccsid = "@(#)$RCSfile: par_cmd_line.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/11/26 11:44:52 $";
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
**	1) parse_cmd_line ( argc, argv, arga, max1 )
**           -> argc, int with command line argument count;
**           -> argv, string array with command line arguments;
**	     -> arga, an array to hold the results of the parsing;
**	     -> maxl, an int with the size of array arga.
**           <- returns '0' if parsing was successfully completed,
**		returns '-1' if not successful.
**
**	   usage:
**	     The routine takes argv and argc and checks syntax, completeness,
**	     and if there are duplications.  Though it checks for the accuracy
**	     of the option flags, it does not check for correctness of the
**	     arguments themselves.  It cannot handle ambiguity in the design
**	     of the command line syntax.
**
**	     It works by taking an initialized array of all the possible
**	     command line options and their specifications and checking
**	     the command line input against the array specs.  It also
**	     combines any arguments with the flags they are associated
**	     so this information can be extracted logically within the
**	     program.
**
**		NOTE: see parse_cmd_line.h for example use.
**
**    functions called by lib functions:
**	1) parse_cmd_line
**	   a) put_in_arg_list ( argv, count, arga, maxl )
**	   b) put_in_flag_list ( argc, argv, &count, arga, maxl )
**	   c) check_final_args ( arga, maxl )
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

#  include <sdm/parse_cmd_line.h>
#  include <sdm/std_defs.h>


int	parse_cmd_line ( argc, argv, arga, maxl )

  	/* This function takes argv and argc and checks syntax,
	   completeness, and if there are duplications.  Though it
	   checks for the accuracy of the option flags, it does not
	   check for correctness of the arguments themselves.  It
	   cannot handle ambiguity in the design of the command
	   line syntax. The function returns a 0 if successful,
	   it prints to stderr and returns a -1 if not. */

    int		argc;                 /* the number of command line arugments */
    char      *	argv [ ];                       /* strings with each argument */
    ARGS        arga [ ];		      /* array of argument structures */
    int		maxl;					/* size of array arga */

{
    int		count = FIRST_ARG;		       /* counts current argv */

  while ( count < argc ) {
    if ( argv [ count ] [ FIRST_FIELD ] != DASH ) { 	 /* an arg not a flag */
      if ( put_in_arg_list ( argv, count, arga, maxl ) == FALSE )
	return ( ERROR );
    } /* if */

    else {						 /* a flag not an arg */
      if ( put_in_flag_list ( argc, argv, &count, arga, maxl ) == FALSE )
	return ( ERROR );
    } /* else */

    count++;
  } /* while */

  if ( check_final_args ( arga, maxl ) == FALSE )
    return ( ERROR );

  return ( OK );
}						            /* parse cmd line */



BOOLEAN	put_in_arg_list ( argv, count, arga, maxl )

	/* This function puts the argument in argv into arga
	   after first creating the space for it.  If arga [ maxl ]
	   is not suppose to have this many arguments, it returns
	   FALSE otherwise TRUE. */

    char      *	argv [ ];                       /* strings with each argument */
    int		count;		 		       /* counts current argv */
    ARGS        arga [ ];		      /* array of argument structures */
    int		maxl;					/* size of array arga */

{
    VALUES    *	arg_ptr;		    /* points to end of argument list */

  arga [ maxl ]. is_set = TRUE;
  ( arga [ maxl ]. field_ct )++;

  if (( not arga [ maxl ]. max_num_fields == UNLIMITED ) and
      ( arga [ maxl ]. field_ct > arga [ maxl ]. max_num_fields )) {
    fprintf ( stderr, "ERROR: exceeded argument count\n" );
    return ( FALSE );
  } /* if */

  if ( arga [ maxl ]. value == NULL ) {
    arga [ maxl ]. value = ( VALUES * ) malloc ( sizeof ( VALUES ));
    arg_ptr = arga [ maxl ]. value;
  } /* if */

  else {
    arg_ptr = arga [ maxl ]. value;

    while ( arg_ptr-> nextfield != NULL )
      arg_ptr = arg_ptr-> nextfield;
  
    arg_ptr-> nextfield = ( VALUES * ) malloc ( sizeof ( VALUES ));
    arg_ptr = arg_ptr-> nextfield;
  } /* else */

  arg_ptr-> field = argv [ count ];       	    /* put arg in arga [maxl] */
  arg_ptr-> nextfield = NULL;
  return ( TRUE );
}							   /* put in arg list */


BOOLEAN	put_in_flag_list ( argc, argv, pcount, arga, maxl )

	/* This function puts the field in argv into arga after
	   first creating the space for it.  If arga [ maxl ]
	   is not suppose to have this many fields, it returns
	   FALSE otherwise TRUE. */

    int		argc;                 /* the number of command line arugments */
    char      *	argv [ ];                       /* strings with each argument */
    int	      * pcount;		 		       /* counts current argv */
    ARGS        arga [ ];		      /* array of argument structures */
    int		maxl;					/* size of array arga */

{
    BOOLEAN	found = FALSE;				      /* misc boolean */
    int 	counter = FIRST_FIELD;			      /* misc counter */

  while (( not found ) and
	 ( counter < maxl )) {

    if ( streq ( arga [ counter ]. flag, argv [ *pcount ] )) { /* found match */
      if ( arga [ counter ]. is_set ) {		           /* but already set */
        fprintf ( stderr, "ERROR: %s option already set.\n",
		  argv [ *pcount ] );
	return ( FALSE );
      } /* if */

      if ( not load_min_fields ( argc, argv, pcount, arga, counter ))
	return ( FALSE );

      if (( arga [ counter ]. max_num_fields == UNLIMITED ) or
          ( arga [counter]. max_num_fields > arga [counter]. min_num_fields ))
        load_max_fields ( argc, argv, pcount, arga, counter );

      arga [ counter ]. is_set = TRUE;
      found = TRUE;
    } /* if */

    else
      counter++;
  } /* while */

  if ( not found ) {
    fprintf ( stderr, "ERROR: %s option not found.\n", argv [ *pcount ] );
    return ( FALSE );
  } /* if */

  return ( TRUE );
}							   /* put in arg list */



BOOLEAN	load_min_fields ( argc, argv, pcount, arga, ct )

	/* This function attempts to load the number of required
	   minimum number of fields into the flag.  If it succeeds,
	   it returns TRUE, else FALSE. */

    int		argc;                 /* the number of command line arugments */
    char      *	argv [ ];                       /* strings with each argument */
    int	      * pcount;		 		       /* counts current argv */
    ARGS        arga [ ];		      /* array of argument structures */
    int		ct;					 /* arga working with */

{
    VALUES    *	field_ptr;		          /* points to fields in arga */
    int		ctr = FIRST_ARG,			      /* misc counter */
		save = *pcount;			     /* original pcount value */

  while ( ctr <= arga [ ct ]. min_num_fields ) {     /* get min number fields */
    if ( *pcount + NEXT < argc )
      (*pcount)++;

    else {
      if ( arga [ ct ]. min_num_fields == 1 )
        fprintf ( stderr, "ERROR: option %s requires 1 argument.\n",
                  argv [ save ] );
      else
        fprintf ( stderr, "ERROR: option %s requires %d arguments.\n",
                  argv [ save ], arga [ ct ]. min_num_fields );

      return ( FALSE );
    } /* else */

    if ( argv [ *pcount ] [ FIRST_FIELD ] == DASH ) {/* ran into another flag */
      if ( arga [ ct ]. min_num_fields == 1 )
        fprintf ( stderr, "ERROR: option %s requires 1 argument.\n",
                  argv [ save ] );
      else
        fprintf ( stderr, "ERROR: option %s requires %d arguments.\n",
                  argv [ save ], arga [ ct ]. min_num_fields );

      return ( FALSE );
    } /* if */

    if ( arga [ ct ]. value == NULL ) {
      arga [ ct ]. value = ( VALUES * ) malloc ( sizeof ( VALUES ));
      field_ptr = arga [ ct ]. value;
    } /* if */

    else {
      field_ptr = arga [ ct ]. value;

      while ( field_ptr-> nextfield != NULL )
        field_ptr = field_ptr-> nextfield;
  
      field_ptr-> nextfield = ( VALUES * ) malloc ( sizeof ( VALUES ));
      field_ptr = field_ptr-> nextfield;
    } /* else */

    field_ptr-> field = argv [ *pcount ];
    field_ptr-> nextfield = NULL;
    ctr++;
    ( arga [ ct ]. field_ct )++;
  } /* while */

  return ( TRUE );
}							   /* load min fields */


load_max_fields ( argc, argv, pcount, arga, ct )

	/* This procedure loads the rest of the fields which the
	   user has entered up to the maximum number for this flag. */

    int		argc;                 /* the number of command line arugments */
    char      *	argv [ ];                       /* strings with each argument */
    int	      * pcount;		 		       /* counts current argv */
    ARGS        arga [ ];		      /* array of argument structures */
    int		ct;					 /* arga working with */

{
    VALUES    *	field_ptr;		          /* points to fields in arga */
    int		ctr = FIRST_ARG,			      /* misc counter */
		remaining;		     /* fields remaining to be filled */
    BOOLEAN	no_flag = TRUE;			      /* still getting fields */

  if ( arga [ ct ]. max_num_fields == UNLIMITED )
    remaining = UNLIMITED;
  else
    remaining = arga [ ct ]. max_num_fields - arga [ ct ]. min_num_fields;

  field_ptr = arga [ ct ]. value;

  while ( field_ptr != NULL )		  /* bring field_ptr to end of fields */
    field_ptr = field_ptr-> nextfield;

  while ((( remaining == UNLIMITED ) or
	  ( ctr <= remaining )) and
	 ( no_flag )) {

    if ( *pcount + NEXT < argc ) {
      (*pcount)++;

      if ( argv [ *pcount ] [ FIRST_FIELD ] == DASH ) { /* ran into next flag */
        no_flag = FALSE;
        (*pcount)--;
      } /* if */

      else {
        if ( arga [ ct ]. value == NULL ) {
          arga [ ct ]. value = ( VALUES * ) malloc ( sizeof ( VALUES ));
          field_ptr = arga [ ct ]. value;
        } /* if */

        else {
          field_ptr = arga [ ct ]. value;
  
          while ( field_ptr-> nextfield != NULL )
            field_ptr = field_ptr-> nextfield;
    
          field_ptr-> nextfield = ( VALUES * ) malloc ( sizeof ( VALUES ));
          field_ptr = field_ptr-> nextfield;
        } /* else */
  
        field_ptr-> field = argv [ *pcount ];
        field_ptr-> nextfield = NULL;
        ctr++;
        ( arga [ ct ]. field_ct )++;
      } /* else */
    } /* if */

    else
      no_flag = FALSE;
  } /* while */
}							   /* load max fields */


BOOLEAN	check_final_args ( arga, maxl )

	/* This function makes sure the number of arguments
	   in the non-option field is correct.  It returns
	   TRUE if they are, FALSE otherwise. */

    ARGS        arga [ ];		      /* array of argument structures */
    int		maxl;				         /* max value of arga */

{
    int		ctr = 0,				      /* misc counter */
		arg_fields = 0;			       /* count fields in arg */
    VALUES    *	arg_ptr;		   /* points to end of arg field list */

  arg_ptr = arga [ maxl ]. value;

  while ( arg_ptr != NULL ) {
    arg_ptr = arg_ptr-> nextfield;
    arg_fields++;
  } /* while */

  if ( arg_fields < arga [ maxl ]. min_num_fields )  {
    if ( arga [ maxl ]. min_num_fields == 1 ) {
      fprintf ( stderr, "ERROR: 1 argument required.\n" );
    } /* if */

    else {
      fprintf ( stderr, "ERROR: %d arguments required.\n",
	        arga [ maxl ]. min_num_fields );
    } /* else */

    return ( FALSE );
  } /* if */

  if (( arga [ maxl ]. max_num_fields != ERROR ) and
      ( arg_fields > arga [ maxl ]. max_num_fields ))  {
    
    if ( arga [ maxl ]. max_num_fields == 0 ) {
      fprintf ( stderr, "ERROR: no arguments allowed.\n" );
    } /* if */

    else if ( arga [ maxl ]. max_num_fields == 1 ) {
      fprintf ( stderr, "ERROR: only 1 argument allowed.\n" );
    } /* else if */

    else {
      fprintf ( stderr, "ERROR: only %d arguments allowed.\n",
	        arga [ maxl ]. max_num_fields );
    } /* else */

    return ( FALSE );
  } /* if */

  return ( TRUE );
}							  /* check final args */
