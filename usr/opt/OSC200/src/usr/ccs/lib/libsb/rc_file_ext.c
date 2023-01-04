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
static char	*sccsid = "@(#)$RCSfile: rc_file_ext.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/11/26 11:45:21 $";
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
**	1) get_current_sb_basedir ( sb, basedir,  sb_rcfile, usr_rcfile )
**	   args:
**	     char  sb [],		name of sandbox
**		   basedir [],		path to sandbox
**		   sb_rcfile [],	path and name to sb local, rc file
**		   usr_rcfile [];	opt path and name to user's rc file
**
**	   returns:
**	     0 if okay; -1 if an empty field cannot be filled,
**	       if sb and basedir are not empty and do not have a matching
**	       line in the usr_rcfile, or if basedir is not empty but sb is.
**          
**	   usage:
**	     always fills in sb if empty, reads usr_rcfile to get default value.
**		Leaves value alone if not empty.
**	     always fills in basedir if empty, reads usr_rcfile to get value,
**		no default.
**		If value not empty:
**		  if sb empty returns FALSE,
**		  if sb has value, sees if sb and basedir match in usr_rcfile,
**		    return TRUE or FALSE to match.
**	     always fills in sb_rcfile if empty, reads usr_rcfile to look
**		for value, default <sandbox>/rc_files/local.
**		If not empty and path is relative, makes it absolute.
**           always fills in usr_rcfile if empty, default $HOME/.sandboxrc
**		If not empty and path is relative, makes it absolute.
**          
**	2) get_current_set ( setname, setdir, sbname, rc_file )
**	   args:
**	     char  setname [],		string to hold setname
**		 * setdir;		string to hold set directory
**		   sbname [],		name of sandbox
**		   rc_file [];		opt path and name to user's rc file
**
**	   returns:
**	     0 if okay; -1 if an empty field cannot be filled,
**	       or if the setname is not in the sandbox sets file.
**          
**	   usage:
**	     always fills in setname if empty (this what routine does).
**		Leaves value alone if not empty, returns TRUE if it in
**		  a current set in the sandbox; FALSE otherwise.
**	     always fills in setdir if empty.
**	        Leaves value alone if not empty:
**		  if setname is not empty, returns TRUE if the setdir
**		  entered is the default setdir for the set entered;
**		  if setname is not entered, returns TRUE if the setdir
**		  entered is the default setdir for the default set.
**	     always fills in sbname if empty, reads rcfile to get default value.
**		Leaves value alone if not empty.
**           always fills in usr_rcfile if empty, default $HOME/.sandboxrc
**		If not empty and path is relative, makes it absolute.
**
**    functions called by lib functions:
**	1) get_current_sb_basedir
**	   a) get_default_usr_rcfile ( usr_rcfile )
**	   b) get_default_sb ( sb, rcfile )
**	   c) current_sb ( sbname, rcfile )
**	   d) get_basedir ( sb, basedir, rcfile )
**	   e) get_default_sb_rcfile ( sbname, basedir, sb_rcfile, usr_rcfile )
**	   f) match_sb_basedir ( sb, basedir, rcfile )
**	2) get_current_set
**	   a) get_default_usr_rcfile ( usr_rcfile )
**	   b) get_current_sb_basedir ( sb, basedir,  sb_rcfile, usr_rcfile )
**	   c) is_existing_set ( setname, sbname, rc_file )
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
#  include <sys/file.h>

extern char *nxtarg();


get_current_sb_basedir ( sb, basedir, sb_rcfile, usr_rcfile )

	/* This function checks to see what type of search is needed.
	   It then calls the appropriate functions to get the
	   information.  If the information is found, it returns
	   0, else -1. */

    char       	sb [],					   /* name of sandbox */
	       	basedir [],			    /* name of base directory */
	        sb_rcfile [],			  /* name of sandbox rc file. */
	        usr_rcfile [];			 /* name and path to rc file. */

{
    char	ab_path [ STRING_LEN ],			       /* misc string */
	      * env_ptr;
    BOOLEAN	debug;					     /* turn on debug */

  if (( env_ptr = getenv ( "EXT_RC_DEBUG" )) == NULL )
    debug = FALSE;
  else
    debug = TRUE;

  if ( usr_rcfile [ FIRST_FIELD ] == NUL ) {
    if  ( not get_default_usr_rcfile ( usr_rcfile ))
      return ( ERROR );
  } /* if */

  else if ( usr_rcfile [ FIRST_FIELD ] != SLASH ) {

    if ( abspath ( usr_rcfile, ab_path ) == ERROR ) {
      fprintf ( stderr, "ERROR: could not get cwd for rcfile %s\n",
			 usr_rcfile );
      return ( ERROR );
    } /* if */

    strcpy ( usr_rcfile, ab_path );
  } /* else if */

  if ( basedir [ FIRST_FIELD ] == NUL ) {
    if ( sb [ FIRST_FIELD ] == NUL ) {   	     /* get sb name if needed */
      if ( not get_default_sb ( sb, usr_rcfile, debug ))
	return ( ERROR );
    } /* if */

    if ( not current_sb ( sb, usr_rcfile, debug )) {       /* is sb name okay */
      return ( ERROR );
    } /* else if */

    if ( not get_basedir ( sb, basedir, usr_rcfile, debug ))   /* now get dir */
	return ( ERROR );
  } /* if */

  else if ( sb [ FIRST_FIELD ] != NUL ) {		/* dir and sb entered */
    if ( not match_sb_basedir ( sb, basedir, usr_rcfile, debug ))
      return ( ERROR );
  } /* else */

  else {				 /* dir entered, no sb - not possible */
    if ( debug )
      fprintf ( stderr, "EXT_RC: -1, dir, %s, entered, no sb name; illegal combination.\n", basedir );

    return ( ERROR );
  } /* else */

  if ( sb_rcfile [ FIRST_FIELD ] == NUL ) {
    if ( not get_default_sb_rcfile ( sb, basedir, sb_rcfile, usr_rcfile, debug))
      return ( ERROR );
  } /* if */
  
  if ( sb_rcfile [ FIRST_FIELD ] != SLASH ) {
    if ( concat ( ab_path, STRING_LEN,
		  basedir, "/", sb, "/", sb_rcfile, NULL ) == NULL) {
      fprintf ( stderr, "ERROR: no room in buffer for '%s/%s/%s'\n",
			 basedir, sb, sb_rcfile );
      return ( ERROR );
    } /* if */
    strcpy ( sb_rcfile, ab_path );
  } /* else if */

  return ( OK );
}						    /* get current sb basedir */



BOOLEAN	get_default_usr_rcfile ( usr_rcfile )

	/* This function gives the usr_rcfile the default value.
	   If it fails to find the file, it returns FALSE,
	   else TRUE. */

    char        usr_rcfile [];			 /* name and path to rc file. */

{
    char      * env_input;                        /* holds values from getenv */

  if (( env_input = getenv ( "HOME" )) == NULL ) {
    fprintf ( stderr, "ERROR: HOME not set in enviroment.\n" );
    return ( FALSE );
  } /* if */

  if ( concat ( usr_rcfile, STRING_LEN,
	        env_input, "/", SANDBOXRC, NULL) == NULL) {
    fprintf ( stderr, "ERROR: no room in buffer for '%s/%s'\n",
		       env_input, SANDBOXRC );
    return ( ERROR );
  } /* if */

  if ( access ( usr_rcfile, R_OK ) == ERROR ) {
    fprintf ( stderr, "ERROR: could not access rc file, %s, for reading.\n",
			usr_rcfile );
    return ( FALSE );
  }/* if */

  return ( TRUE );
}						    /* get default usr rcfile */



BOOLEAN get_default_sb ( sb, rcfile, debug )

	/* This function looks for the default sb value, first
	   as an environment variable, next in the rcfile.  If
	   it is in neither, it returns FALSE. */

    char      *	sb,					   /* name of sandbox */
	      * rcfile;				 /* name and path to rc file. */
    BOOLEAN	debug;					     /* turn on debug */

{
    FILE      * ptr_file;                                   /* ptr to rc file */
    char      * env_input,                        /* holds values from getenv */
		line [ STRING_LEN ];                           /* misc string */
    char      * line_ptr,
              * token;

  if (( env_input = getenv ( SANDBOX )) != NULL ) {
    strcpy ( sb, env_input );

    if ( debug )
      printf ( "EXT_RC: Found sb name in environment.  Name is: %s.\n", sb );

    return ( TRUE );
  } /* if */

  if (( ptr_file = fopen ( rcfile, READ )) == NULL ) {
    fprintf ( stderr, "ERROR: cannot read rc file %s.\n", rcfile );
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, STRING_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );
    if ( streq ( token, DEFAULT ))
      break;
  } /* while */

  fclose ( ptr_file );

  if ( line_ptr == NULL ) {
    if ( debug )
      fprintf ( stderr, "EXT_RC: -1, Did not find default sb name in rc file: %s.\n", rcfile );

    return ( FALSE );
  } /* if */

  else {
    token = nxtarg ( &line_ptr, WHITESPACE );
    strcpy ( sb, token );
    if ( debug )
      printf ( "EXT_RC: Found default sb name in rc file.  Name is: %s.\n", sb );

    return ( TRUE );
  } /* else */
}							    /* get default sb */



BOOLEAN current_sb ( sbname, rcfile, debug )

	/* This function checks to be sure the entered name of
	   the sandbox actually exists.  If it doesn't, it
	   returns FALSE, TRUE otherwise. */

    char      *	sbname,					   /* name of sandbox */
	      * rcfile;				 /* name and path to rc file. */
    BOOLEAN	debug;					     /* turn on debug */

{
    FILE      * ptr_file;                                   /* ptr to rc file */
    char	line [ STRING_LEN ];                           /* misc string */
    char      * line_ptr,
              * token;

  if (( ptr_file = fopen ( rcfile, READ )) == NULL ) {
    fprintf ( stderr, "ERROR: cannot read rc file %s.\n", rcfile );
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, STRING_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, SB )) {
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( streq ( token, sbname )) {
        if ( debug )
          printf ( "EXT_RC: Found sb name in rc file.  Name is: %s.\n", sbname );
  
        fclose ( ptr_file );
        return ( TRUE );
      } /* if */
    } /* if */
  } /* while */

  if ( debug )
    fprintf ( stderr, "EXT_RC: -1, Did not find sb name in rc file: %s.\n",
			 rcfile );
  fclose ( ptr_file );
  return ( FALSE );
}							        /* current sb */



BOOLEAN get_basedir ( sb, basedir, rcfile, debug )

	/* This function looks for a directory to match the given
	   sb in the rcfile.  If it finds one, it returns TRUE. */

    char      *	sb,					   /* name of sandbox */
	      *	basedir,			    /* name of base directory */
	      * rcfile;				 /* name and path to rc file. */
    BOOLEAN	debug;					     /* turn on debug */

{
    FILE      * ptr_file;                                   /* ptr to rc file */
    char	line [ STRING_LEN ];                           /* misc string */
    char      * line_ptr,
              * token;

  if (( ptr_file = fopen ( rcfile, READ )) == NULL ) {
    fprintf ( stderr, "ERROR: cannot read rc file %s.\n", rcfile );
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, STRING_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, BASE )) {
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( streq ( token, sb ))		   /* sandbox name is matched exactly */
        break;

      if ( strchr ( token, STAR ) == NULL )  	  /* no wild card so no match */
        continue;

      if ( streq ( token, STAR_ST ))	    /* "*" matches everything so okay */
        break;

      if ( gmatch ( sb, token ))	 /* if wild card and match, then okay */
        break;
    } /* if */
  } /* while */

  fclose ( ptr_file );

  if ( line_ptr == NULL ) {
    if ( debug )
      fprintf ( stderr, "EXT_RC: -1, Did not find base dir in rcfile: %s.\n",
			 rcfile );

    return ( FALSE );
  } /* if */

  token = nxtarg ( &line_ptr, WHITESPACE );
  strcpy ( basedir, token );

  if ( debug )
    printf ( "EXT_RC: Found base dir in rcfile. Dir is: %s.\n", basedir );

  return ( TRUE );
}							       /* get basedir */



BOOLEAN get_default_sb_rcfile ( sbname, basedir, sb_rcfile, usr_rcfile, debug )

	/* This procedure reads through the rc file, usr_rcfile,
	   looking for a match for the sbname.  When it finds it
	   it checks to see if there is a third field.  If there
	   is, it returns that information, else it returns the
	   path to the default sandbox rc file. */

    char      *	sbname,					   /* name of sandbox */
	      *	basedir,			    /* name of base directory */
	       	sb_rcfile [],			 /* string to hold sb rc file */
	      * usr_rcfile;			 /* name and path to rc file. */
    BOOLEAN	debug;					     /* turn on debug */

{
    FILE      * ptr_file;                                   /* ptr to rc file */
    char	line [ STRING_LEN ];                           /* misc string */
    char      * line_ptr,
              * token;

  if (( ptr_file = fopen ( usr_rcfile, READ )) == NULL ) {
    fprintf ( stderr, "ERROR: cannot read rc file %s.\n", usr_rcfile );
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, STRING_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, SB )) {			   /* key word to match is SB */
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( streq ( token, sbname )) {		      /* matches sandbox name */
        token = nxtarg ( &line_ptr, WHITESPACE );

        if ( token [ FIRST_FIELD ] == NUL )    /* no third field, use default */
	    break;

        if ( debug )
          printf ( "EXT_RC: Found sandbox rc file in usr rcfile. File is: %s.\n",
			  token );

        strcpy ( sb_rcfile, token );
        fclose ( ptr_file );
        return ( TRUE );
      } /* if */
    } /* if */
  } /* while */

  fclose ( ptr_file );

  if ( concat ( sb_rcfile, STRING_LEN,
	        basedir, "/", sbname, "/", LOCAL_RC, NULL ) == NULL ) {
    fprintf ( stderr, "ERROR: no room in buffer for '%s/%s/%s'\n",
		       basedir, sbname, LOCAL_RC );
    return ( ERROR );
  } /* if */

  if ( debug )
    printf ( "EXT_RC: Using default sandbox rc file: %s.\n", sb_rcfile );

  return ( TRUE );
}						     /* get default sb rcfile */



BOOLEAN match_sb_basedir ( sb, basedir, rcfile, debug )

	/* This function looks in the rcfile for a sb and basedir
	   pair that match the two entered.  If it finds a pair,
	   it returns TRUE. */

    char      *	sb,					   /* name of sandbox */
	      *	basedir,			    /* name of base directory */
	      * rcfile;				 /* name and path to rc file. */
    BOOLEAN	debug;					     /* turn on debug */

{
    FILE      * ptr_file;                                   /* ptr to rc file */
    char	line [ STRING_LEN ];                           /* misc string */
    char      * line_ptr,
              * token;

  if (( ptr_file = fopen ( rcfile, READ )) == NULL ) {
    fprintf ( stderr, "ERROR: cannot read rc file %s.\n", rcfile );
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, STRING_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, BASE )) {
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( not streq ( token, sb )) {	       /* if not sandbox name exactly */
        if ( strchr ( token, STAR ) == NULL )    /* no wild card, so no match */
	  continue;

        if ( not streq ( token, STAR_ST ) and  	    /* "*" matches everything */
	     not gmatch ( sb, token ))		      /* some wild card match */
	  continue;
      } /* if */

      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( streq ( token, basedir )) {
        if ( debug )
          printf ( "EXT_RC: sb, %s, and base, %s, match.\n", sb, basedir );

        return ( TRUE );
      } /* if */
    } /* if */
  } /* while */

  if ( debug )
    fprintf ( stderr, "EXT_RC: -1, sb, %s, and base, %s, don't match.\n",
		       sb, basedir );

  fclose ( ptr_file );
  return ( FALSE );
}							  /* match sb basedir */



get_current_set ( setname, setdir, sbname, rc_file )

	/* This procedure gets the current set name, first from the
	   environment variable, BCSSET, and then, if it is not set
	   from the sandbox set rc file.  If the sandbox name is
	   empty, it gets the default sandbox.  It returns -1 if
	   it still can't determine the setname.  If the setname
	   entered is not empty, it checks to see if it is in the
	   sandbox sets rc file.  It returns -1 if it is not. */

    char	setname [],			      /* the set name to fill */
	      * setdir,				      /* set directory t fill */
		sbname [],			       /* the current sandbox */
	       	rc_file [];			            /* rc file to use */

{
    FILE      * ptr_file;                                   /* ptr to rc file */
    char      * env_input,                        /* holds values from getenv */
		tmp [ STRING_LEN ],                           /* misc string */
		base [ STRING_LEN ],                           /* misc string */
		set_loc [ STRING_LEN ],                        /* misc string */
		line [ STRING_LEN ],                           /* misc string */
		set_dir [ STRING_LEN ], 	 /* used to input setdir info */
        	ab_path [ STRING_LEN ],			       /* misc string */
	      * env_ptr;
    BOOLEAN	debug;					     /* turn on debug */
    char      * line_ptr,
              * token;

  if ( setdir == NULL ) {			 /* if null, need local space */
    set_dir [ FIRST_FIELD ] = NUL;
    setdir = set_dir;
  } /* if */

  if (( env_input = getenv ( "EXT_RC_DEBUG" )) == NULL )
    debug = FALSE;
  else
    debug = TRUE;

  if ( rc_file [ FIRST_FIELD ] == NUL ) {    	   /* fix this no matter what */
    if  ( not get_default_usr_rcfile ( rc_file ))
      return ( ERROR );
  } /* if */

  else if ( rc_file [ FIRST_FIELD ] != SLASH ) {

    if ( abspath ( rc_file, ab_path ) == ERROR ) {
      fprintf ( stderr, "ERROR: could not get cwd for rcfile %s\n", rc_file );
      return ( ERROR );
    } /* if */

    strcpy ( rc_file, ab_path );
  } /* else if */

  if ( setname [ FIRST_FIELD ] != NUL ) {	   	     /* already given */
    if ( is_existing_set ( setname, setdir, sbname, rc_file, debug ))
      return ( OK );
    else
      return ( ERROR );
  } /* if */

  if (( env_input = getenv ( BCSSET )) != NULL ) {
    strcpy ( setname, env_input );  	      /* get it from the env variable */

    if ( is_existing_set ( setname, setdir, sbname, rc_file, debug ))
      return ( OK );
    else
      return ( ERROR );
  } /* if */

  base [ FIRST_FIELD ] = NUL;		  /* determine location of sb */
  tmp [ FIRST_FIELD ] = NUL;

  if ( get_current_sb_basedir ( sbname, base, tmp, rc_file ) == ERROR )
    return ( ERROR );

  if ( concat ( set_loc, STRING_LEN,
	        base, "/", sbname, "/", SET_RC, NULL ) == NULL ) {
    fprintf ( stderr, "ERROR: no room in buffer for '%s/%s/%s'\n",
		       base, sbname, SET_RC );
    return ( ERROR );
  } /* if */

  if (( ptr_file = fopen ( set_loc, READ )) == NULL ) {
    fprintf ( stderr, "ERROR: cannot read from set rc file\n  %s.\n", set_loc );
    return ( ERROR );
  } /* if */

  while (( line_ptr = fgets ( line, STRING_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, DEFAULT )) {
      token = nxtarg ( &line_ptr, WHITESPACE );
      strcpy ( setname, token );
      fclose ( ptr_file );

      if ( debug )
        printf ( "EXT_RC: found default set, %s, in sb rcfile.\n", setname );

      if ( is_existing_set ( setname, setdir, sbname, rc_file, debug ))
        return ( OK );
      else
        return ( ERROR );
    } /* if */
  } /* while */

  fclose ( ptr_file );

  if ( debug )
    fprintf ( stderr, "EXT_RC: -1, no default set in rcfile: %s.\n", set_loc );

  return ( ERROR );
}							   /* get current set */



BOOLEAN	is_existing_set ( setname, setdir, sbname, rc_file, debug )

	/* This function checks to see if the set name is in the
	   current sandbox rc_files/set file.  It returns TRUE
	   if it is, FALSE if not. It also checks the setdir,
	   filling it in if it is empty and checking for consistency
	   with the setname if it is not. */

    char	setname [],			      /* the set name to fill */
	      * setdir,				      /* set directory t fill */
		sbname [],			       /* the current sandbox */
	       	rc_file [];			            /* rc file to use */
    BOOLEAN	debug;					     /* turn on debug */

{
    FILE      * ptr_file;                                   /* ptr to rc file */
    char        tmp [ STRING_LEN ],                            /* misc string */
		base [ STRING_LEN ],                           /* misc string */
		set_loc [ STRING_LEN ],                        /* misc string */
		line [ STRING_LEN ];                           /* misc string */
    char      * line_ptr,
              * token;

  tmp [ FIRST_FIELD ] = NUL;
  base [ FIRST_FIELD ] = NUL;

  if ( get_current_sb_basedir ( sbname, base, tmp, rc_file ) == ERROR )
    return ( FALSE );

  if ( concat ( set_loc, STRING_LEN,
	        base, "/", sbname, "/", SET_RC, NULL ) == NULL ) {
    fprintf ( stderr, "ERROR: no room in buffer for '%s/%s/%s'\n",
		       base, sbname, SET_RC );
    return ( ERROR );
  } /* if */

  if (( ptr_file = fopen ( set_loc, READ )) == NULL ) {
    fprintf ( stderr, "ERROR: cannot read from set rc file\n  %s.\n", set_loc );
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, STRING_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, SET_KEY )) {
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( streq ( token, setname )) {
        token = nxtarg ( &line_ptr, WHITESPACE );
        fclose ( ptr_file );

	if ( setdir [ FIRST_FIELD ] == NUL )
	  strcpy ( setdir, token );

	else if ( not streq ( setdir, token )) {
          if ( debug )
            fprintf ( stderr, "EXT_RC: -1, could not match set, %s, to setdir, %s.\n", setname, setdir );

	  return ( FALSE );
	} /* else if */

        if ( debug )
          printf ( "EXT_RC: matched set, %s, and setdir, %s, in sb rcfile.\n",
			setname, setdir );

        return ( TRUE );
      } /* if */
    } /* if */
  } /* while */

  if ( debug )
    fprintf ( stderr, "EXT_RC: -1, could not find set, %s, in sb rcfile %s.\n",
		       setname, set_loc );

  fclose ( ptr_file );
  return ( FALSE );
} 							   /* is existing set */



/* ****************************************************************************
** The following match routine was taken from the "find" command
** and not edited in any way.  It returns true if string 's' is
** matched by pattern 'p' where 'p' is a string with wildcards in
** it.
** ************************************************************************** */

gmatch(s, p) /* string match as in glob */
register char *s, *p;
{
	if (*s=='.' && *p!='.' && *p!='{') return(0);
	return amatch(s, p);
}

amatch(s, p)
register char *s, *p;
{
	register cc;
	int scc, k;
	int c, lc;
	char *scopy;
	char schar;
	char *endbrace;
	char *endcomma;

	scc = *s;
	lc = 077777;
	switch (c = *p) {
	case '{':
		k = 0;
		endbrace = index(p, '}');
		if (endbrace == (char *) 0) return 0;	/* No closing '}' */
		schar = *(scopy = s);

		while (cc = *++p) {
			if (cc == ',' || cc == '}') {
				k |= amatch(scopy, endbrace + 1);
				if (k) return 1;
				if (cc == '}') return 0;
				schar = *(scopy = s);	/* Retry source */
			}
			else
			if (cc == schar) {
				/* Char match succeeded */
				schar = *++scopy;
			} else {
				/* Char match failed */
				endcomma = index(p, ',');
				if (endcomma == (char *) 0 ||
				    endcomma >= endbrace)
					return 0;

				p = endcomma;	/* Next choice */
				schar = *(scopy = s);	/* Retry source */
			}
		}
		return 0;

	case '[':
		k = 0;
		while (cc = *++p) {
			switch (cc) {

			case ']':
				if (k)
					return(amatch(++s, ++p));
				else
					return(0);

			case '-':
				cc = p[1];
				k |= lc <= scc && scc <= cc;
			}
			if (scc==(lc=cc)) k++;
		}
		return(0);

	case '?':
	caseq:
		if(scc) return(amatch(++s, ++p));
		return(0);
	case '*':
		return(umatch(s, ++p));
	case 0:
		return(!scc);
	}
	if (c==scc) goto caseq;
	return(0);
}

umatch(s, p)
register char *s, *p;
{
	if(*p==0) return(1);
	while(*s)
		if (amatch(s++, p)) return(1);
	return(0);
}

/* ****************************************************************************
** End of the match routines taken from find
** ************************************************************************** */
