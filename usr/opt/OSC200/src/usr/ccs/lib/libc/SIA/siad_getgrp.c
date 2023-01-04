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
static char *rcsid = "@(#)$RCSfile: siad_getgrp.c,v $ $Revision: 1.1.8.7 $ (DEC) $Date: 1993/12/17 17:10:50 $";
#endif

/*                                                               */
/* This module contains the following external routines:         */
/*                                                               */
/*   int siad_getgrent (struct group *result,                    */
/*                      char         *buffer,                    */
/*                      int           buflen)                    */
/*                                                               */
/*   int siad_getgrgid (gid_t         gid,                       */
/*                      struct group *result,                    */
/*                      char         *buffer,                    */
/*                      int           buflen)                    */
/*                                                               */
/*   int siad_getgrnam (char         *name,                      */
/*                      struct group *result,                    */
/*                      char         *buffer,                    */
/*                      int           buflen)                    */
/*                                                               */
/*   int siad_setgrent (void)                                    */
/*                                                               */
/*   int siad_endgrent (void)                                    */
/*                                                               */
/*                                                               */
/*   struct group *fgetgrent (FILE *fp)				 */
/*                                                               */
/*                                                               */
/* For use by siad_getpasswd.c routines:                         */
/*                                                               */
/*   int onminuslist    (char  *name,                            */
/*                       List  *minuslist)                       */
/*                                                               */
/*   int addtominuslist (char  *name,                            */
/*                       List **minuslist)                       */
/*                                                               */
/*   void freeminuslist (List **minuslist)                       */
/*                                                               */

/* Includes                                                      */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#define fgetgrent_r __fgetgrent_r
#define fgetgrent __fgetgrent
#if defined(_THREAD_SAFE) || defined(_REENTRANT)
#pragma weak fgetgrent_r = __fgetgrent_r
#else
#pragma weak fgetgrent = __fgetgrent
#endif
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak addtominuslist = __addtominuslist
#pragma weak bsd_siad_getgrgid = __bsd_siad_getgrgid
#pragma weak freeminuslist = __freeminuslist
#pragma weak onminuslist = __onminuslist
#pragma weak siad_endgrent = __siad_endgrent
#pragma weak siad_getgrent = __siad_getgrent
#pragma weak siad_getgrgid = __siad_getgrgid
#pragma weak siad_getgrnam = __siad_getgrnam
#pragma weak siad_setgrent = __siad_setgrent
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

/* Global variables and Externs                                  */

static char        *domain = NULL,         /* NIS domain                   */
                   *nis = NULL,	           /* pointer into the NIS group   */
                                           /* database file                */
                   *oldnis = NULL;
static int          nislen = 0,
                    oldnislen = 0,
                    setgrent_flag = 0,  /* has setgrent been called ?  */
                    nis_flag = 0;       /* set only in middle of NIS search */
static struct group NULLGROUP = {NULL, NULL, 0, NULL};
static List        *minuslist = NULL;	


extern char *yellowup (int flag);
void freeminuslist (List **minuslist);

/* Static routines: grpskip, save, interpret, interpretwithsave   */
/*                  getnamefromnis, getgidfromnis, gidof,         */ 
/*                  free_nis, getnextfromnis, getfirstfromnis,    */
/*                  matchname, matchgid                           */


/*                  
/* grpskip:                                                       */
/*         Move the pointer p beyond the character c in the       */
/*         string that begins at p.  Replace the character c in   */
/*         that string with a '\0'.                               */
/*         Return the pointer p.                                  */

static char *
  grpskip (char *p, char c)
{
  while (*p && *p != c && *p != '\n') 
    ++p;
  if (*p) 
    *p++ = 0;

  return (p);
}


/* save
/*         DESCRIPTION: Save the gr_passwd and gr_mem fields of a group   */
/*                      structure in the global group structure named     */
/*                      'savegroup'.                                      */ 
/*                      Use malloc for the strings to be saved, and free  */
/*                      the previous savegroup structure, if one exists.  */
/*                                                                        */
/*              INPUTS: group   Pointer to a group structure which        */
/*                              contains the values to be saved.          */
/*                                                                        */
/*             OUTPUTS: none                                              */
/*                                                                        */
/* 	 RETURN VALUES: A pointer to savegroup, the global group          */
/*                      structure which stores the input values.          */


static struct group *
  save (struct group *group)
{
  static struct group *savegroup;
  char **av, **q;
  int lnth;
	
  /*
   * free up stuff from last time around
   */
  if (savegroup) {
    for (av = savegroup -> gr_mem; *av != NULL; av++) {
      free (*av);
    }
    free (savegroup -> gr_passwd);
    free (savegroup -> gr_mem);
    free (savegroup);
  }
  if (group == NULL)
    return savegroup = NULL;
  if ((savegroup = (struct group *) malloc (sizeof (struct group))) == NULL)
    return ((struct group *) NULL);
  if ((savegroup -> gr_passwd = 
       (char *) malloc (strlen (group -> gr_passwd) + 1)) == NULL)
    return ((struct group *) NULL);

  strcpy (savegroup -> gr_passwd, group -> gr_passwd);
  for (lnth = 1, av = group -> gr_mem; *av != NULL; av++) {
    lnth++;
  }
  q = (char **) calloc(lnth, sizeof (char *));
  savegroup -> gr_mem = q;
  if (!q)
    return ((struct group *) NULL);
  for (av = group -> gr_mem; *av != NULL; av++) {
    if ((*q = (char *) malloc (strlen (*av) + 1)) == NULL)
      return ((struct group *) NULL);
    strcpy (*q, *av);
    q++;
  }
  *q = 0;
  return (savegroup);
}


/* interpret                                                              */
/*                                                                        */
/*     DESCRIPTION: Interpret a group entry and create a corresponding    */
/*                  group structure.                                      */
/*                                                                        */
/*          INPUTS: buffer   A group entry from the local or NIS group    */
/*                           file.                                        */
/*                                                                        */
/*         OUTPUTS: group    A pointer to a group structure which         */
/*                           corresponds to the input group entry.        */
/*                                                                        */
/*   RETURN VALUES: 1        Success                                      */
/*                  0        Invalid group entry                          */

static int
  interpret (char *buffer, struct group *group)
{
  char         *p, 
              **q,
	       *r;
  static char **gr_mem;
  static int  gr_mem_len = 0;
  int		lnth;
  int           colon_count = 0;

  p = buffer;

  group -> gr_name = p;
  GRSKIP(p);
  group -> gr_passwd = p;
  GRSKIP(p);
  group -> gr_gid = atoi (p);
  GRSKIP(p);

  grpskip (p,'\n');
  for (lnth = 1, r = p;  r && *++r;  r = strchr(r, ','))
    lnth++;
  if (lnth > gr_mem_len) {
    gr_mem_len = 0;
    free(gr_mem);
    gr_mem = (char **) malloc(sizeof (char *) * lnth);
    if (gr_mem)
      gr_mem_len = lnth;
    else
      return 0;
  }
  group -> gr_mem = q = gr_mem;
  while (*p)
    {
      *q++ = p;
      p = grpskip (p,',');
    }
  *q = NULL;

  if (colon_count < 3 && 
      (group->gr_name[0] != '+' && group->gr_name[0] != '-'))
    return (0);
  else 	
    return (1);
}


/* interpretwithsave                                                      */
/*                                                                        */
/*     DESCRIPTION: Interpret a group entry and create a corresponding    */
/*                  group structure.  Override the values for gr_passwd   */
/*                  and gr_mem with corresponding values from the global  */
/*                  group structure named 'savegroup', if savegroup       */
/*                  contains these values.                                */
/*                                                                        */
/*          INPUTS: buffer      A group entry from the local or NIS       */
/*                              group file.                               */
/*                  savegroup   Group structure containing saved fields   */
/*                                                                        */
/*         OUTPUTS: group       A pointer to a group structure which      */
/*                              corresponds to the input group entry      */
/*                              plus the overriding values from           */
/*                              savegroup.                                */
/*                                                                        */
/*   RETURN VALUES: 1           Success                                   */
/*                  0           Invalid group entry                       */

static int
  interpretwithsave (char *buffer, struct group *savegroup, 
		     struct group *group)
{
  /* Interpret one line of the NIS group database */
  if (!interpret (buffer, group))
    return (0);

  /* Set gr_passwd and gr_mem to the saved values from the local  */
  /* group database file                                          */
  if (savegroup -> gr_passwd && *(savegroup -> gr_passwd))
    group -> gr_passwd =  savegroup -> gr_passwd;
  if (savegroup -> gr_mem && *(savegroup -> gr_mem))
    group -> gr_mem = savegroup -> gr_mem;
  return (1);
}


/* getnamefromnis:                                                        */
/*                                                                        */
/*    DESCRIPTION: Search the NIS group database for a group withe the    */
/*                 group name 'name'.  If found, fill in the structure    */
/*                 'group' with the group information.                    */
/*                                                                        */
/*         INPUTS: name                                                   */
/*                 savegroup                                              */
/*                 buflen                                                 */
/*                                                                        */
/*        OUTPUTS: group                                                  */
/*                 buffer                                                 */
/*                                                                        */
/*  RETURN VALUES: 1            Success                                   */
/*                 0            Group not found in NIS                    */

static int
  getnamefromnis (char *name, struct group *savegroup, struct group *group,
		  char *buffer, int buflen)
{
  int   vallen = 0;
  char *val = NULL;
  
  if (yp_match (domain, "group.byname", name, 
		strlen (name), &val, &vallen)) 
    return (0);
  else {
    if (vallen < buflen - 1) {
      (void) strncpy (buffer, val, vallen);
      buffer[vallen] = '\n';
      buffer[vallen + 1] = '\0';
    } else {
      (void) strncpy (buffer, val, buflen - 2);
      buffer[buflen - 2] = '\n';
      buffer[buflen - 1] = '\0';
    }
    free (val);
    if (!interpretwithsave (buffer, savegroup, group))
      return (0);

    return (1);
  }
}


/* getgidfromnis:                                                 */
/*                 Search the NIS group database for a group      */
/*                 with gr_gid = gid.  If found, return a pointer */
/*                 to the group.  Otherwise, return NULL.         */

static int
  getgidfromnis (gid_t gid, struct group *savegroup, struct group *group,
		 char *buffer, int buflen)
{
  int           vallen = 0;
  char         *val    = NULL,
                gidstr[MAXGIDLEN];
	
  sprintf (gidstr, "%d", gid);
  if (yp_match (domain, "group.bygid", gidstr, strlen (gidstr), 
		&val, &vallen)) {
    return (0);
  }
  else {
    if (vallen < buflen - 1) {
      (void) strncpy (buffer, val, vallen);
      buffer[vallen] = '\n';
      buffer[vallen + 1] = '\0';
    } else {
      (void) strncpy (buffer, val, buflen - 2);
      buffer[buflen - 2] = '\n';
      buffer[buflen - 1] = '\0';
    }
    free (val);
    if (!interpretwithsave (buffer, savegroup, group))
      return (0);
    return (1);
  }
}


/* gidof:                                                       */
/*        Given the name of a group, return the group's gid.    */
/*        If the group is not found in the NIS database, return */
/*        MAXINT                                                */

static int
  gidof (char *name)
{
  struct group group,
               nullgroup;
  char         buffer[SIABUFSIZ];
  int          buflen = SIABUFSIZ;
  
  nullgroup = NULLGROUP;

  if (getnamefromnis (name, &nullgroup, &group, buffer, buflen))
    return (group.gr_gid);
  else
    return (MAXINT);
}


static void
  free_nis (void)
{
  if (nis)
    free (nis);
  nis = NULL;

  if (oldnis)
    free (oldnis);
  oldnis = NULL;

  (void) save((struct group *)NULL);	/* free old group list */

  (void) freeminuslist (&minuslist);
}


/* getnextfromnis:                                                  */
/*                 Set nis to point to the next entry in the NIS    */
/*                 group database.                                  */

static void
  getnextfromnis (char *buffer, int buflen)
{
  int   keylen = 0;
  char *key = NULL;
  
  if (yp_next (domain, "group.byname", oldnis, oldnislen, 
	       &key, &keylen, &nis, &nislen)) {
    nis = NULL;
    nis_flag = 0;
  } else {
    if (nislen < buflen - 1) {
      (void) strncpy (buffer, nis, nislen);
      buffer[nislen] = '\n';
      buffer[nislen + 1] = '\0';
    } else {
      (void) strncpy (buffer, nis, buflen - 2);
      buffer[buflen - 2] = '\n';
      buffer[buflen - 1] = '\0';
    }
    
    free (nis);
  }
    if (oldnis)
      free (oldnis);
    oldnis = key;
    oldnislen = keylen;
 
}


/* getfirstfromnis:                                                  */
/*                  Set nis to point to the first entry in the NIS   */
/*                  group database.                                  */

static void
  getfirstfromnis (char *buffer, int buflen)
{
  int   keylen = 0;
  char *key = NULL;
	
  if (yp_first (domain, "group.byname", &key, &keylen, &nis, &nislen)) {
    nis = NULL;
    nis_flag = 0;
  } else {
    if (nislen < buflen - 1) {
      (void) strncpy (buffer, nis, nislen);
      buffer[nislen] = '\n';
      buffer[nislen + 1] = '\0';
    } else {
      (void) strncpy (buffer, nis, buflen - 2);
      buffer[buflen - 2] = '\n';
      buffer[buflen - 1] = '\0';
    }
    free (nis);
  }
  if (oldnis)
    free (oldnis);
  oldnis = key;
  oldnislen = keylen;
}


/* matchname:                                                     */
/*             If parameter 'line1' is a local group entry,       */
/*             determine whether this is the matching entry.      */
/*             If so, return 1; if not, return 0.                 */
/*                                                                */
/*             If parameter 'line1' is an NIS group entry,        */
/*             determine whether there is a matching entry in the */
/*             NIS database.  If so, return 1; if not, return 0.  */
/*                                                                */
/*             Check NIS only if the domain variable is set.      */
/*             If domain is NULL, ignore NIS '+' entries.         */

static int
  matchname (char *buffer, int buflen, struct group *group, char *name)
{
  struct group *savegroup;
  
  switch (buffer[0]) {
  case '+':
    
    /* If NIS is not running, ignore this entry. */
    if (domain == (char *) NULL)
      break;

    /* Entry Type '+:' or '+name'                */

    if ((strcmp (group -> gr_name, "+") == 0) ||
	(strcmp (group -> gr_name + 1, name) == 0)) {

      /* Store the gr_passwd and gr_mem fields        */
      /* from the local passwd entry, if they exist.  */

      savegroup = save (group);

      /* Search the NIS group database for a match.   */

      if (getnamefromnis (name, savegroup, group, buffer, buflen))
	return (1);
      else
	return (0);
    }
   
    break;

  case '-':

    /* Entry Type '-name' */

    if (strcmp (group -> gr_name+1, name) == 0) {
      return (NIS_DISALLOW);
    }

    break;
   
  default:
    if (strcmp (group -> gr_name, name) == 0)
      return (1);
  }
  return (0);
}


/* matchgid:                                                   */
/*             Locate group with gid 'gid' in either the       */
/*             NIS group database or the local group database. */
/*             Check NIS only if the variable domain is set.   */
/*             If domain is NULL, ignore NIS entries           */
/*             Return 1 if a match is found;                   */
/*                    0 if a match is not found, or            */
/*   	              NIS_DISALLOW if this group is            */
/*                    disallowed by NIS.                       */

static int
  matchgid (char *buffer, int buflen, struct group *group, gid_t gid)
{
  struct group *savegroup;
  
  switch (buffer[0]) {
  case '+':
    
    /* If NIS is not running, ignore this entry */
    if (domain == (char *) NULL) 
      break;

    /* Entry Type: '+:' */
    
    if (strcmp (group -> gr_name, "+") == 0) {

      /* Store the gr_passwd and gr_mem fields          */
      /* from the local group entry, if they exist.     */
      
      savegroup = save (group);

      /* Search the NIS group database for a match.     */

      if (getgidfromnis (gid, savegroup, group, buffer, buflen))
	return (1);
      else 
	return (0);
    }

    /* Entry Type: '+name' */
    
    /* Store the gr_passwd and gr_mem fields          */
    /* from the group entry, if they exist.           */
      
    savegroup = save (group);
    if ((getnamefromnis (group -> gr_name + 1, savegroup, group, buffer,
			 buflen))
	&& (group -> gr_gid == gid))
      return (1);
    else 
      return (0);
    
    break;

  case '-':
    
    /* Entry Type: '-name' */

    if (gid == gidof (group -> gr_name + 1)) {
      return (NIS_DISALLOW);
    }
    
    break;

  default:
    if (group -> gr_gid == gid)
      return (1);
  }
  return (0);
}


/* External routines for siad_getpasswd.c to share:                */
/*                   onminuslist, addtominuslist, freeminuslist.   */


/* onminuslist:                                                    */
/*              Determine if a group is on the "minus" list.       */
/*              If so, return 1; if not, return 0.                 */

int
  onminuslist (char *name, List *minuslist)
{
  List *ls;
  
  for (ls = minuslist; ls != NULL; ls = ls -> nxt)
    if (strcmp (ls -> name, name) == 0)
      return (1);
  return (0);
}


/* addtominuslist:                                                  */
/*                 Add the group name 'name' to the "minuslist"     */
/*                 Return 1 if successful, NULL if error            */

int
  addtominuslist (char *name, List **minuslist)
{
  List *ls;
  char *buf;
	
  if ((ls = (List *) malloc (sizeof (List))) == NULL)
    return (0);
  if ((buf = (char *) malloc (strlen (name) + 1)) == NULL)
    return (0);
  strcpy (buf, name);
  ls -> name = buf;
  ls -> nxt = *minuslist;
  *minuslist = ls;
  return (1);
}


/* freeminuslist:                                                  */
/*                Free the entire "minuslist".                     */
 
void
  freeminuslist (List **minuslist) 
{
  List *ls;
	
  for (ls = *minuslist; ls != NULL; ls = ls -> nxt) {
    free (ls -> name);
    free (ls);
  }
  *minuslist = NULL;
}


/* siad_setgrent:                                                 */
/*                Prepare for search through local and NIS        */
/*                databases.                                      */
/*                Return SIADSUCCESS.                             */


int
  siad_setgrent (FILE **context)
{
return bsd_siad_setgrent(context);
}
static int
  bsd_siad_setgrent (FILE **context)
{
  /* Open the local group database file, if it isn't already open. */
  /* Otherwise, rewind the file.                                   */
  if (!*context)
    *context = fopen (GROUP, "r");
  else
    rewind (*context);

  /* Set NIS domain variable if NIS is configured and running */
  domain = yellowup (1);

  /* Clean up NIS data */
  free_nis ();
  
  setgrent_flag = 1;
  return (SIADSUCCESS);
}

/* siad_endgrent:                                                 */
/*                 Close the local group database file and clean  */
/*                 up the NIS data.                               */
/*                 Return SIADSUCCESS.                            */

int
  siad_endgrent (FILE **context)
{
return bsd_siad_endgrent(context);
}
static int
  bsd_siad_endgrent (FILE **context)
{
  /* Close the local group database file */
  if (*context) {
    fclose (*context);
    *context = NULL;
  }

  /* Clean up NIS data */
  free_nis ();
   
  setgrent_flag = 0;
  return (SIADSUCCESS);
}


/* siad_getgrent:                                                      */
/*                 Return values:                                      */
/*                    SIADSUCCESS if a group entry is returned.        */
/*                    SIADCONTINUE if there are no more group entries. */
/*                    SIADFAIL if a failure occurs.                    */

int
siad_getgrent (struct group *result, char *buffer, int buflen, FILE **context)
{
return bsd_siad_getgrent(result,buffer,buflen,context);
}

static int
  bsd_siad_getgrent (struct group *result, char *buffer, int buflen, FILE **context)
{
  static struct group *savegroup;
  
  /* Parameter checking */
  if (result == NULL || buffer == NULL || buflen < 1) {
    return (SIADFAIL);
  }

  /* If setgrent has not been called, call it */
#ifdef	_THREAD_SAFE
  if(!*context)
#else
  if (setgrent_flag == 0)
#endif	/* _THREAD_SAFE */
    (void) bsd_siad_setgrent(context);

  /* If the local group database file cannot be opened return SIADFAIL.  */
  if (!*context) {
    return (SIADFAIL);
  }

  for (;;) {

    /* If nis points to a line in the NIS group database, process  */
    /* that line.  Otherwise, process a line from the local group  */
    /* database file.                                              */

     if (nis_flag) {
       
       /* set nis to point to the next line in the NIS group database */
       (void) getnextfromnis (buffer, buflen);
       if (*buffer == NULL)
	  nis_flag = 0;

       /* if this group is excluded by the local group database file, */
       /* do not return this group                                    */

       if ((interpretwithsave (buffer, savegroup, result)) &&
	    (!onminuslist (result -> gr_name, minuslist)))
	 return (SIADSUCCESS);
       
     } else {
       result -> gr_name = (char *) NULL;
       while (result -> gr_name == (char *) NULL) {
	 if (fgets (buffer, buflen, *context) == NULL) {
	   return (SIADFAIL);
	 }
	 /* ignore bad entries */
	 if (!interpret (buffer, result))
	   result -> gr_name = (char *) NULL;
       }

       switch (buffer[0]) {
       case '+':
	 /* If NIS is not running, ignore this entry */
	 if (domain == (char *) NULL) 
	   break;

	 /* Entry Type '+:' */

	 if (strcmp (result -> gr_name, "+") == 0) {
	   savegroup = save (result);
	   (void) getfirstfromnis (buffer, buflen);
	   
	   if (*buffer != NULL) {
	      nis_flag = 1;
	      if ((interpretwithsave (buffer, savegroup, result)) &&
		  (!onminuslist (result -> gr_name, minuslist))) 
		return (SIADSUCCESS);
	    }
	 } else {

	   /* Entry Type '+name' */

	   savegroup = save (result);
	   if ((getnamefromnis (result -> gr_name + 1, savegroup, result, 
				buffer, buflen))
	     && (!onminuslist (result -> gr_name, minuslist)))
	     return (SIADSUCCESS);
	 }
      
	 break;

       case '-':

	 /* Entry Type '-name' */

	 if (addtominuslist (result -> gr_name + 1, &minuslist) == NULL) {
	   return (SIADFAIL);
	 }
       
	 break;

       default:
	 if (!onminuslist (result -> gr_name, minuslist)) 
	   return (SIADSUCCESS);

	 break;
       }    
     }
   }
}


/* siad_getgrgid:                                                       */
/*                 Return values:                                       */
/*                   SIADSUCCESS if a match is found in local/NIS       */
/*                   SIADCONTINUE if a match is not found in local/NIS  */
/*                   SIADFAIL if a failure occurs.                      */
 
int
  siad_getgrgid (gid_t gid, struct group *result, char *buffer, int buflen)
{
return(bsd_siad_getgrgid ( gid, result, buffer, buflen));
}
int
  bsd_siad_getgrgid (gid_t gid, struct group *result, char *buffer, int buflen)
{
  int res;
  FILE *context=NULL;

  /* Parameter checking */
  if (result == NULL || buffer == NULL || buflen < 1) {
    return (SIADFAIL);
  }

  /* Prepare for local/NIS search */
  (void) bsd_siad_setgrent(&context);

  /* Return SIADFAIL if the local group database file is not open */
  if (!context) {
    return (SIADFAIL);
  }

  /* Read the local group database file one line at a time */
  while (fgets (buffer, buflen, context) != NULL) {
    /* ignore invalid entries */
    if (interpret (buffer, result)) 
      {
	/* does this entry's gid match the gid we're looking for? */
	if ((res = matchgid (buffer, buflen, result, gid)) == 1) {
	  /* Clean up after the local/NIS search */
	  (void) bsd_siad_endgrent(&context);
	  return (SIADSUCCESS);
	} else if (res == NIS_DISALLOW) {
	  (void) bsd_siad_endgrent(&context);
	  return (SIADFAIL);
	}
      }
  }
  (void) bsd_siad_endgrent(&context);
  return (SIADFAIL);
}


/*****************************************************************************
* Usage:  int siad_getgrnam (char *name, struct group *result, char *buffer, 
*                            int buflen)
*
* Description: siad_getgrnam searches the local and NIS group databases
*              for an entry with a group name that matches the requested
*              group name.  siad_getgrnam is called from sia_getgroup
*              which is called from either getgrnam (libc) or 
*              getgrnam_r (libc_r). 
*
* Parameter Descriptions:  
*                         name: the name of the requested group
*                       result: a pointer to a group structure which will
*                               be filled in by this routine
*                       buffer: a pointer to a buffer which contains the
*                               strings pointed to by the group structure
*                          len: the length of the buffer
*
* Success return: SIADSUCCESS
*
* Error Conditions and returns:
*
*       Condition1: a match is not found in local/NIS
*       Return: SIADCONTINUE
*
*       Condition2: cannot open the local group database file
*       Return: SIADFAIL
*****************************************************************************/
int
siad_getgrnam (char *name, struct group *result, char *buffer, int buflen)
{
return(bsd_siad_getgrnam(name,result,buffer,buflen));
}
static int
  bsd_siad_getgrnam (char *name, struct group *result, char *buffer, int buflen)
{
  int res;
  FILE *context=NULL;

  /* Parameter checking */
  if (result == NULL || buffer == NULL || buflen < 1) {
    return (SIADFAIL);
  }

  /* Prepare for local/NIS search */
  (void) bsd_siad_setgrent (&context);

  /* Return SIADFAIL if the local group database file is not open */
  if (!context) {
    return (SIADFAIL);
  }

  /* Read the local group database file one line at a time */
  while (fgets (buffer, buflen, context) != NULL) {
    /* ignore invalid entries */
    if (interpret (buffer, result))
      {
	if ((res = matchname (buffer, buflen, result, name)) == 1) {
	  /* Clean up after the local/NIS search */
	  (void) bsd_siad_endgrent (&context);
	  return (SIADSUCCESS);
	} else if (res == NIS_DISALLOW) {
	  (void) bsd_siad_endgrent(&context);
	  return (SIADFAIL);
	}
      }
  }
  (void) bsd_siad_endgrent(&context);
  return (SIADFAIL);
}

/*****************************************************************************
* Usage:  struct group *fgetgrent (FILE *fp)
* Usage:  int fgetgrent_r(FILE *fp, struct group *gr, char *buf, int buflen)
*
* Description: fgetgrent returns a pointer to a static buffer containing the
*	       next group-file-formatted entry read from the file pointer
*	       provided, or NULL on error or EOF.
*	       fgetgrent_r returns 0 on success or -1 on error, with errno
*	       setup as appropriate.
*
* Parameter Descriptions:  
*			    fp: a pointer to a stdio stream for reading.
*			    gr: a pointer to where to write the result.
*			   buf: a pointer to a buffer for holding the strings
*				to which the resulting structure points.
*			   len: the size in bytes allocated to buf.
*
* Success return: A pointer to static data containing a group record.
*
* Error Conditions and returns:
*
*	Condition1: No (valid) next entry found
*       Return: NULL
*
*       Condition2: Cannot read from the passed file pointer
*       Return: NULL
*****************************************************************************/
#include "ts_supp.h"
static int
siad_fgetgrent(FILE *fp, struct group *result, char *buffer, int buflen)
{
  char *save_domain;
  int   save_flag;
  int   status;

  /* Check parameters */
  if (!fp || !result || !buffer || buflen <= 0) {
    TS_SETERR(EINVAL);
    return (-1);
  }
  
  /* Just in case, provide thread locking */
  SIATHREADLOCK(SIA_GROUP_LOCK);

  /* Fake a different setgrent environment, using the supplied fp and
     without NIS */
  save_flag = setgrent_flag;
  save_domain = domain;
  setgrent_flag = 1;
  domain = (char *) NULL;

  /* Get the next entry from the file */
  status = bsd_siad_getgrent(result, buffer, buflen, &fp);

  /* Put things back the way we found them */
  setgrent_flag = save_flag;
  domain = save_domain;

  /* Unlock if we locked */
  SIATHREADREL(SIA_GROUP_LOCK);

  /* Return as appropriate based on what getgrent gave us */
  if ((status & SIADSUCCESS) == SIADSUCCESS)
    return 0;
  if (!TS_GETERR())
    TS_SETERR(ESRCH);
  return (-1);
}
#if defined(_THREAD_SAFE) || defined(_REENTRANT)
int
fgetgrent_r(FILE *fp, struct group *result, char *buffer, int buflen)
{
  return siad_fgetgrent(fp, result, buffer, buflen);
}
#else /* not thread-safe */
struct group *
fgetgrent(FILE *fp)
{
  int   status;

  /* Fetch to the static buffer */
  status = siad_fgetgrent(fp, &siad_bsd_group, siad_bsd_getgrbuf, BUFSIZ);

  /* Return as appropriate based on what getgrent gave us */
  if (status == 0)
    return &siad_bsd_group;
  return (struct group *) NULL;
}
#endif /* _THREAD_SAFE || _REENTRANT */
