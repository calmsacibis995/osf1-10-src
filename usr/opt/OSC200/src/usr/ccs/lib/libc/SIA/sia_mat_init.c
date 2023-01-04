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
static char *rcsid = "@(#)$RCSfile: sia_mat_init.c,v $ $Revision: 1.1.11.4 $ (DEC) $Date: 1993/08/04 21:21:04 $";
#endif
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_mat_init = __sia_mat_init
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"
/*****************************************************************************
* Usage:        int sia_mat_init()
*
* Description:  sia_mat_init() initializes the global SIA matrix (sia_mat)
*               from the file MATRIX_CONF.
*            
*               The SIA matrix is a matrix of structures which 
*               contain security mechanism package names, library names 
*               and function pointers.  The SIA matrix is used to sequence 
*               SIA calls to the available security mechanisms, per
*               capability.  This routine is only called from sia_init().
* 
*               If the SIA matrix has not been initialized or is out of date
*               with the most recent version of the MATRIX_CONF file,
*               sia_mat_init() will reread the MATRIX_CONF file.  Otherwise, 
*               sia_mat_init() will do nothing.
*
*               sia_mat_init sets all unused entries in the SIA matrix to NULL.
*
*               On successful completion, sia_mat_init() returns SIASUCCESS.
*               
* Assumed Input: A correctly formatted, readable MATRIX_CONF file.
*
*                sia_mat is a global struct shared by all threads
*		 within a process.
*
* Error Conditions and returns:
*
* 	Condition1: MATRIX_CONF file does not exist or 
*                   is incorrectly formatted
*	Return:     SIAFAIL
*
*****************************************************************************/

#define TRUE 1
#define FALSE 0
#define SIAEOF 0

int
sia_mat_init (void)
{
  /* If the MATRIX_CONF file exists, do nothing.   */
  /* Otherwise, log the error and exit             */

  if (stat (MATRIX_CONF, &sia_mat.matstat) < 0) {
    sia_log (MSGSTR (SIA_MSG_LOGERROR, "ERROR"),
	     MSGSTR (SIA_MSG_NOFILERR, "File %s not found"),
		     MATRIX_CONF);
    return (SIAFAIL);
  }

  /* If the MATRIX_CONF file has changed since it was last read by */
  /* sia_mat_init, reread the file.  Otherwise, do nothing.        */

  if (sia_mat.matstat.st_mtime != (time_t) sia_mat.matrix_date)
    if ((read_matrix()) == SIAFAIL) {
      return (SIAFAIL);
    } 
  return (SIASUCCESS);
}

static int
  read_matrix (void)
{
  FILE *matrix_conf_file;
  char one_line [SIABUFSIZ];
  int i, j;

  /* Initialize the SIA matrix with all NULL values */
  for (i=0; i < SIADCAPMAX; i++) {
    for (j=0; j < SIASWMAX; j++) {
      sia_mat.matrix_ent[i][j].pkgnam = (char *) NULL;
      sia_mat.matrix_ent[i][j].libnam = (char *) NULL;
      sia_mat.matrix_ent[i][j].fp = (void *) NULL;
    }
  }
    
  /* Open the MATRIX_CONF file and use it to fill in the SIA matrix */
  if ((matrix_conf_file = fopen (MATRIX_CONF, "r")) == 0) {
    sia_log (MSGSTR (SIA_MSG_LOGERROR, "ERROR"),
	     MSGSTR (SIA_MSG_MATOPENERR, "Can't open matrix file %s"),
		     MATRIX_CONF);
    return (SIAFAIL);
  } else {
    while (sia_getline (matrix_conf_file, one_line) != SIAEOF)
      if (sia_fill_matrix (one_line) == SIAFAIL) {
	if (fclose (matrix_conf_file) == EOF)
	  sia_log (MSGSTR (SIA_MSG_LOGERROR, "ERROR"),
		   MSGSTR (SIA_MSG_MATCLOSERR, 
			   "Can't close matrix file %s\n"), MATRIX_CONF);
	return (SIAFAIL);
      }
  }

#ifdef _SIA_DEBUG

  /* Print the SIA matrix values */
  for (i=0; i < SIADCAPMAX; i++) {
    if (!sia_caps[i])
      break;

    printf ("%s:\n", sia_caps[i]);
    for (j=0; j < SIASWMAX + 1; j++) {
      if (sia_mat.matrix_ent[i][j].pkgnam == (char *) NULL)
	break;
      printf ("\t\t%s", sia_mat.matrix_ent[i][j].pkgnam);
      printf ("\t\t%s\n", sia_mat.matrix_ent[i][j].libnam);
    }
  }

#endif /* _SIA_DEBUG */

  /* Close the MATRIX_CONF file */
  if (fclose (matrix_conf_file) == EOF)
	sia_log (MSGSTR (SIA_MSG_LOGERROR, "ERROR"),
		 MSGSTR (SIA_MSG_MATCLOSERR, "Can't close matrix file %s\n"), 
			 MATRIX_CONF);

  (time_t) sia_mat.matrix_date = sia_mat.matstat.st_mtime;

  return (SIASUCCESS);
}


/* sia_getline reads one line from MATRIX_CONF.  It returns the
 * line stripped of comments and white spaces.
 * It returns SIAEOF at end of file.
 */

static int
  sia_getline (FILE *matrix_conf_file, char *one_line)
{
  char *cp;
  int   i;

  /* Read first line, skipping blank lines and comments */
  one_line[0] = '\0';
  while (one_line[0] == '\0' || 
	 one_line[0] == '#' ||
	 one_line[0] == '\n') {
    if (fgets (one_line, SIABUFSIZ, matrix_conf_file) == NULL)
      return (SIAEOF);

    /* zap the newline returned by fgets */
    one_line[strlen (one_line) - 1] = '\0';
  }

  /* zap trailing comment and/or white space */
  cp = one_line;
  i = 0;
  while (*cp) {
    if ((*cp == ' ') || (*cp == '\t')) {
      cp++;	/* skip over space or tab */
    } else {
      if (*cp == '#')	/* stop if we reach a comment */
	break;
      one_line[i++] = *cp++;
    }
  }
  one_line[i] = '\0';
  
  if (i == 0) {
    sia_log (MSGSTR (SIA_MSG_LOGERROR, "ERROR"),
	     MSGSTR (SIA_MSG_MATBADFORMAT, 
		     "Bad matrix file format in %s\n"),
		     MATRIX_CONF);
    return (SIAFAIL);
  }
  
  return (SIASUCCESS);
}


/* sia_fill_matrix interprets one line of MATRIX_CONF
 * and fills in sia_mat with the information contained in the line.
 * Returns SIAFAIL if bad format, SIASUCCESS otherwise.
 */

static int
  sia_fill_matrix (char *one_line)
{
  char *mech, *lib, *tuple, *ptr;
  int cap_index, at_least_one_tuple=FALSE;
  
  /* Identify capability and set capability index, cap_index */

  if (identify_capability (one_line, &cap_index) == SIAFAIL)
    return (SIAFAIL);
  
  /* For each tuple, identify the mechanism and the library, verify the */
  /* mechanism, and fill in the matrix                                  */

  ptr = one_line;
  while ((tuple = strchr (ptr, '(')) != (char *)0) {
    ptr = tuple + 1;
    at_least_one_tuple = TRUE;
    if (identify_mech_and_lib (tuple, &mech, &lib) == SIAFAIL)
      return (SIAFAIL);
    if (fill_matrix (cap_index, mech, lib) == SIAFAIL)
      return (SIAFAIL);
  }
  if (at_least_one_tuple)
    return (SIASUCCESS);
  else
    return (SIAFAIL);
}

static int
  identify_capability (char *one_line, int *cap_index)
{
  char *rhs, *capability, cap[SIABUFSIZ];
  int i;
  
  strcpy (cap, one_line);

  if ((rhs = strchr (cap, '=')) == (char *)0) {
    sia_log (MSGSTR (SIA_MSG_LOGERROR, "ERROR"),
	     MSGSTR (SIA_MSG_MATBADFORMAT, 
		     "Bad matrix file format in %s\n"), MATRIX_CONF);
    return (SIAFAIL);
  }
  *rhs = '\0';
  capability = &cap[0];
  
  for (i=0; 
       sia_caps[i] != '\0' && strcmp (capability, sia_caps[i]) != 0; 
       i++) 
    ;

  if (sia_caps[i] == '\0') {
    sia_log (MSGSTR (SIA_MSG_LOGERROR, "ERROR"),
	     MSGSTR (SIA_MSG_MATBADFORMAT, 
		     "Bad matrix file format in %s\n"), MATRIX_CONF);
    return (SIAFAIL);
  }

  *cap_index = i;
  return (SIASUCCESS);
}
 
static int 
  identify_mech_and_lib (char *tuple, char **mech, char **lib)
{
  static char tup[SIABUFSIZ];
  char *rhs;
  int i;
  
  /* identify mechanism */
  strcpy (tup, tuple);
  *mech = &tup[1];
  rhs = strchr (tup, ',');
  *rhs = '\0';
  
  /* identify library */
  rhs++;
  *lib = rhs;
  rhs = strchr (rhs,')');
  *rhs = '\0';
  
#ifdef _SIA_VERIFY_MECH
  /* verify mechanism */
  for (i=0; 
       sia_mechs[i] != '\0' && strcmp (*mech, sia_mechs[i]) != 0; 
       i++) 
    ;;
    
  if (sia_mechs[i] == '\0') {
    sia_log (MSGSTR (SIA_MSG_LOGERROR, "ERROR"),
	     MSGSTR (SIA_MSG_MATBADFORMAT, 
		     "Bad matrix file format in %s\n"), MATRIX_CONF);
    return (SIAFAIL);
  }
#endif /* defined _SIA_VERIFY_MECH */

  return (SIASUCCESS);
}

static int
  fill_matrix (int cap_index, char *mech, char *lib)
{
  static int mech_order = 0, prev_cap = -1;
  
  if (cap_index == prev_cap)
    mech_order++;
  else mech_order = 0;

  if (mech_order >= SIASWMAX)
	return (SIAFAIL);

  /* Allocate memory for the package name and library name, and fill  */
  /* them in appropriately                                            */

  if ((sia_mat.matrix_ent[cap_index][mech_order].pkgnam = 
       (char *) malloc (strlen (mech) + 1)) == NULL)
    return(SIAFAIL);
  else strcpy (sia_mat.matrix_ent[cap_index][mech_order].pkgnam, mech);

  if ((sia_mat.matrix_ent[cap_index][mech_order].libnam = 
       (char *) malloc (strlen (lib) + 1)) == NULL)
    return(SIAFAIL);
  else strcpy (sia_mat.matrix_ent[cap_index][mech_order].libnam, lib);
  
  prev_cap = cap_index;
  return (SIASUCCESS);
}
