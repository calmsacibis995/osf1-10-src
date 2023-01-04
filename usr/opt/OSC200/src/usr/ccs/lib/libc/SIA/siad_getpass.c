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
static char *rcsid = "@(#)$RCSfile: siad_getpass.c,v $ $Revision: 1.1.13.9 $ (DEC) $Date: 1994/02/21 21:51:18 $";
#endif
/*                                                            */
/* This module contains the following external routines and   */
/*  external variables:                                       */
/*                                                            */
/*   int  siad_getpwent (struct passwd *result,               */
/*                       char          *buffer,               */
/*                       int            buflen)               */
/*                                                            */
/*   int  siad_getpwuid (uid_t          uid,                  */
/*                       struct passwd *result,               */
/*                       char          *buffer,               */
/*                       int            buflen)               */
/*                                                            */
/*   int  siad_getpwnam (char          *name,                 */
/*                       struct passwd *result,               */
/*                       char          *buffer,               */
/*                       int            buflen)               */
/*                                                            */
/*   int  siad_setpwent (void)                                */
/*                                                            */
/*   int  siad_endpwent (void)                                */
/*                                                            */
/*  void  setpwfile (char *file)                              */
/*                                                            */
/*  struct passwd *getpwent_local (void)                      */
/*                                                            */
/*  struct passwd *getpwuid_local (uid_t uid)                 */
/*                                                            */
/*  struct passwd *getpwnam_local (char *name)                */
/*                                                            */
/*  int _pw_stayopen                                          */
/*                                                            */


/* Includes and Defines                                       */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#define fgetpwent_r __fgetpwent_r
#define fgetpwent __fgetpwent
#if defined(_THREAD_SAFE) || defined(_REENTRANT)
#pragma weak fgetpwent_r = __fgetpwent_r
#else
#pragma weak fgetpwent = __fgetpwent
#endif
#pragma weak bsd_siad_getpwnam = __bsd_siad_getpwnam
#pragma weak getpwent_local = __getpwent_local
#pragma weak getpwnam_local = __getpwnam_local
#pragma weak getpwuid_local = __getpwuid_local
#pragma weak pw_file = __pw_file
#pragma weak setpwfile = __setpwfile
#pragma weak siad_endpwent = __siad_endpwent
#pragma weak siad_getpwent = __siad_getpwent
#pragma weak siad_getpwnam = __siad_getpwnam
#pragma weak siad_getpwuid = __siad_getpwuid
#pragma weak siad_setpwent = __siad_setpwent
#endif
#include "siad.h"
#include "siad_bsd.h"
#include <ndbm.h>

/* The following line is because <rpcsvc/yp_prot.h> will redefine the */
/* typedef datum if we don't define DATUM.  datum is defined in       */
/* <ndbm.h>                                                           */
#define	DATUM

/* Externs                                                    */

extern char *yellowup       (int   flag);
extern int   onminuslist    (char  *name, 
			     List  *minuslist); 
extern int   addtominuslist (char  *name, 
			     List **minuslist);
extern void  freeminuslist  (List **minuslist);  
extern struct passwd siad_bsd_passwd;
extern char siad_bsd_getpwbuf[];

/* Global variables                                           */

int _pw_stayopen	= 0;
DBM *_pw_db = NULL;	                /* pointer into passwd data base */

FILE   *pw_file = NULL;                    /* local passwd file ptr  */

static	char    passwd_file[MAXPATHLEN] = PASSWD,  /* local passwd file name */
              *domain = NULL,                     /* NIS domain             */
              *nis = NULL,	                  /* NIS current entry ptr  */
              *oldnis = NULL;                     /* NIS previous key ptr   */
static	int    	nislen = 0,                        /* NIS current entry size */
               oldnislen = 0,                     /* NIS previous key size  */
               setpwent_flag = 0,                 /* setpwent status        */
               nis_flag = 0,                      /* NIS search status      */
               use_nis = 1;                       /* NIS include/exclude    */
static struct  passwd NULLPASSWD =                /* NULL passwd structure  */
                          {NULL, NULL, 0, NULL};
static List   *minuslist;	                  /* NIS exclusion list     */
static char emp[] = "";                           /* empty string           */


/* Static routines: save, interpret, interpretwithsave,             */
/*                  getnamefromnis, getuidfromnis, uidof, free_nis, */ 
/*                  getnextfromnis, getfirstfromnis,                */
/*                  matchname, matchuid, fetchpw                    */


/* save
/*         DESCRIPTION: Save the pw_passwd, pw_gecos, pw_dir, and         */
/*                      pw_shell fields of a passwd structure in the      */
/*                      global passwd structure named 'savepasswd'.       */ 
/*                      Use malloc for the strings to be saved, and free  */
/*                      the previous savepasswd structure, if one exists. */
/*                                                                        */
/*              INPUTS: passwd   Pointer to a passwd structure which      */
/*                               contains the values to be saved.         */
/*                                                                        */
/*             OUTPUTS: none                                              */
/*                                                                        */
/* 	 RETURN VALUES: A pointer to savepasswd, the global passwd        */
/*                      structure which stores the input values.          */


static struct passwd *
  save (struct passwd *passwd)
{
  static struct passwd *savepasswd;
  
  /*
   * free up stuff from last time around
   */
  if (savepasswd) {
    free (savepasswd -> pw_passwd);
    free (savepasswd -> pw_gecos);
    free (savepasswd -> pw_dir);
    free (savepasswd -> pw_shell);
    free ((char *) savepasswd);
  }
  if (passwd == NULL)
    return savepasswd = NULL;

  if ((savepasswd = 
       (struct passwd *) malloc (sizeof (struct passwd))) == NULL)
    return ((struct passwd *) NULL);

  if ((savepasswd -> pw_passwd = 
       (char *) malloc ((unsigned) strlen (passwd -> pw_passwd) + 1)) 
      == NULL)
    return ((struct passwd *) NULL);
  (void) strcpy (savepasswd -> pw_passwd, passwd -> pw_passwd);

  if ((savepasswd -> pw_gecos = 
       (char *) malloc ((unsigned) strlen (passwd -> pw_gecos) + 1)) == NULL)
    return ((struct passwd *) NULL);
  (void) strcpy (savepasswd -> pw_gecos, passwd -> pw_gecos);
  
  if ((savepasswd -> pw_dir = 
       (char *) malloc ((unsigned) strlen (passwd -> pw_dir) + 1)) == NULL)
    return ((struct passwd *) NULL);
  (void) strcpy(savepasswd -> pw_dir, passwd -> pw_dir);

  if ((savepasswd -> pw_shell = 
       (char *) malloc ((unsigned) strlen (passwd -> pw_shell) + 1)) == NULL)
    return ((struct passwd *) NULL);
  (void) strcpy (savepasswd -> pw_shell, passwd -> pw_shell);
 
  return (savepasswd);
}


/* interpret                                                              */
/*                                                                        */
/*     DESCRIPTION: Interpret a passwd entry and create a corresponding   */
/*                  passwd structure.                                     */
/*                                                                        */
/*          INPUTS: buffer   A passwd entry from the local or NIS passwd  */
/*                           file.                                        */
/*                                                                        */
/*         OUTPUTS: passwd   A pointer to a passwd structure which        */
/*                           corresponds to the input passwd entry.       */
/*                                                                        */
/*   RETURN VALUES: 1        Success                                      */
/*                  0        Invalid passwd entry                         */

static int
  interpret (char *buffer, struct passwd *passwd)
{
  char  *p,
        *v;
  int    colon_count = 0;

  p = buffer;

  passwd -> pw_name = p;
  PWSKIP(p);
  passwd -> pw_passwd = p;
  PWSKIP(p);
  if (colon_count < 2 && 
      (passwd->pw_name[0] != '+' && passwd->pw_name[0] != '-'))
    passwd->pw_uid = NOBODY;
  else ATOI(p,passwd->pw_uid);
  PWSKIP(p);
  if (colon_count < 3 && 
      (passwd->pw_name[0] != '+' && passwd->pw_name[0] != '-'))
    passwd->pw_gid = NOBODY;
  else
    ATOI(p,passwd->pw_gid);

  /* The only difference in the pwd structure between DEC OSF/1
   * and System V is in the definition of the following field.
   * However, both systems ignore it because it is not present
   * in the actual password file.
   */
#ifndef SYSTEM_FIVE
  passwd->pw_quota = 0;
#else	SYSTEM_FIVE
  passwd->pw_age = (char *) 0;
#endif	SYSTEM_FIVE

  passwd->pw_comment = emp;
  PWSKIP(p);
  passwd->pw_gecos = p;
  PWSKIP(p);
  passwd->pw_dir = p;
  PWSKIP(p);
  passwd->pw_shell = p;
  while(*p && *p != '\n') p++;
  *p = '\0';
  if (colon_count < 4 && (passwd->pw_name[0] != '+' && passwd->pw_name[0] != '-'))
    return (0);
  else 	return(1);
}

/* interpretwithsave                                                      */
/*                                                                        */
/*     DESCRIPTION: Interpret a passwd entry and create a corresponding   */
/*                  passwd structure.  Override the values for pw_passwd, */
/*                  pw_gecos, pw_dir, and pw_shell with corresponding     */
/*                  values from the global passwd structure named         */
/*                  'savepasswd', if savepasswd contains these values.    */
/*                                                                        */
/*          INPUTS: buffer       A passwd entry from the local or NIS     */
/*                               passwd file.                             */
/*                  savepasswd   Passwd structure containing fields which */
/*                               will override NIS values.                */
/*                                                                        */
/*         OUTPUTS: passwd       A pointer to a passwd structure which    */
/*                               corresponds to the input passwd entry    */
/*                               plus the overriding values from          */
/*                               savepasswd.                              */
/*                                                                        */
/*   RETURN VALUES: 1            Success                                  */
/*                  0            Invalid passwd entry                     */


static int
  interpretwithsave (char *buffer, struct passwd *savepasswd, 
		     struct passwd *passwd)
{
  struct passwd temp;

  /* Interpret one line of the NIS passwd database */
  if (interpret (buffer, passwd) == 0)
    return (0);

  /* Set pw_passwd, pw_gecos, pw_dir, pw_shell to the saved values */
  /* from the local passwd database file                           */
  if (savepasswd->pw_passwd && *(savepasswd->pw_passwd))
    passwd->pw_passwd =  savepasswd->pw_passwd;
  if (savepasswd->pw_gecos && *(savepasswd->pw_gecos))
    passwd->pw_gecos = savepasswd->pw_gecos;
  if (savepasswd->pw_dir && *(savepasswd->pw_dir))
    passwd->pw_dir = savepasswd->pw_dir;
  if (savepasswd->pw_shell && *(savepasswd->pw_shell))
    passwd->pw_shell = savepasswd->pw_shell;
  return (1);
}


/* getnamefromnis                                                         */
/*                                                                        */
/*     DESCRIPTION: Search the NIS passwd database for a passwd entry     */
/*                  with pw_name 'name'.  If found, fill in the structure */
/*                  'passwd' with the passwd entry, overriding the        */
/*                  appropriate NIS fields with those stored in           */
/*                  savepasswd.                                           */
/*                                                                        */
/*          INPUTS: name         User name of the requested entry         */
/*                  savepasswd   Passwd structure containing fields which */
/*                               will override NIS values.                */
/*		    buflen       Size of output buffer                    */
/*                                                                        */
/*         OUTPUTS: passwd       A pointer to a passwd structure which    */
/*                               corresponds to the NIS passwd entry plus */
/*                               the overriding values from savepasswd.   */
/*                  buffer       The passwd entry from NIS                */
/*                                                                        */
/*   RETURN VALUES: 1            Success                                  */
/*                  0            Failure, NIS entry not found             */

static int
  getnamefromnis (char *name, struct passwd *savepasswd, 
		  struct passwd *passwd, char *buffer, int buflen)
{
  int   vallen =0;
  char *val = NULL;
  
  if (yp_match (domain, "passwd.byname", name, 
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
    if (interpretwithsave (buffer, savepasswd, passwd) == 0)
      return (0);
    return (1);
  }
}


/* getuidfromnis                                                          */
/*                                                                        */
/*     DESCRIPTION: Search the NIS passwd database for a passwd entry     */
/*                  with pw_uid 'uid'.  If found, fill in the structure   */
/*                  'passwd' with the passwd entry, overriding the        */
/*                  appropriate NIS fields with those stored in           */
/*                  savepasswd.                                           */
/*                                                                        */
/*          INPUTS: uid          User ID of the requested entry           */
/*                  savepasswd   Passwd structure containing fields which */
/*                               will override NIS values.                */
/*		    buflen       Size of output buffer                    */
/*                                                                        */
/*         OUTPUTS: passwd       A pointer to a passwd structure which    */
/*                               corresponds to the NIS passwd entry plus */
/*                               the overriding values from savepasswd.   */
/*                  buffer       The passwd entry from NIS                */
/*                                                                        */
/*   RETURN VALUES: 1            Success                                  */
/*                  0            Failure, NIS entry not found             */

static int
  getuidfromnis (gid_t uid, struct passwd *savepasswd, struct passwd *passwd,
		 char *buffer, int buflen)
{
  int           vallen = 0;
  char         *val    = NULL,
                uidstr[MAXUIDLEN];
	
  sprintf (uidstr, "%d", uid);
  if (yp_match (domain, "passwd.byuid", uidstr, strlen (uidstr), 
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
    if (interpretwithsave (buffer, savepasswd, passwd) == 0)
      return (0);
    return (1);
  }
}


/* uidof                                                                  */
/*                                                                        */
/*     DESCRIPTION:  Given a user name, return the corresponding uid.     */
/*                                                                        */
/*          INPUTS: name         User name                                */
/*                                                                        */
/*         OUTPUTS: none                                                  */
/*                                                                        */
/*   RETURN VALUES: uid          Success                                  */
/*                  MAXINT       Failure, user's passwd entry not found   */
/*                               in NIS                                   */

static int
  uidof (char *name)
{
  struct passwd passwd,
                nullpasswd;
  char          buffer[SIABUFSIZ];
  int           buflen = SIABUFSIZ;
  
  nullpasswd = NULLPASSWD;

  if (getnamefromnis (name, &nullpasswd, &passwd, buffer, buflen))
    return (passwd.pw_uid);
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

  (void) save((struct passwd *)NULL);

  (void) freeminuslist (&minuslist);
}


/* getnextfromnis:                                                  */
/*                 Set nis to point to the next entry in the NIS    */
/*                 passwd database.                                  */

static void
  getnextfromnis (char *buffer, int buflen)
{
  int   keylen = 0;
  char *key = NULL;
  
  if (yp_next (domain, "passwd.byname", oldnis, oldnislen, 
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
/*                  passwd database.                                  */

static void
  getfirstfromnis (char *buffer, int buflen)
{
  int   keylen = 0;
  char *key = NULL;
	
  if (yp_first (domain, "passwd.byname", &key, &keylen, &nis, &nislen)) {
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
/*             If parameter 'line1' is a local passwd entry,      */
/*             determine whether this is the matching entry.      */
/*             If so, return 1; if not, return 0.                 */
/*                                                                */
/*             If parameter 'line1' is an NIS passwd entry,       */
/*             determine whether there is a matching entry in the */
/*             NIS database.  If so, return 1; if not, return 0.  */
/*                                                                */
/*             Check NIS only if the domain variable is set.      */
/*             If domain is NULL, ignore NIS '+' entries.         */

static int
  matchname (char *buffer, int buflen, struct passwd *passwd, char *name)
{
  struct passwd *savepasswd;
  
  switch (buffer[0]) {
  case '+':
    /* If NIS is not running, ignore this entry. */
    if (domain == (char *) NULL)
      break;

    /* Entry Type '+:' */

    if (strcmp (passwd -> pw_name, "+") == 0) {

      /* Save the pw_passwd, pw_gecos, pw_dir, and pw_shell fields      */
      /* from the local passwd entry, if they exist.                    */

      savepasswd = save (passwd);

      /* Search the NIS passwd database for a match.                 */

      if (getnamefromnis (name, savepasswd, passwd, buffer, buflen))
	return (1);
      else
	return (0);
    }
   
    /* Entry Type '+@netgroup' */

    if (buffer[1] == '@') {
      if (innetgr (passwd -> pw_name + 2, (char *) NULL, name, domain)) {
	savepasswd = save (passwd);
	if (getnamefromnis (name, savepasswd, passwd, buffer, buflen))
	  return (1);
      }
      return (0);
    }

    /* Entry Type '+name' */

    if (strcmp (passwd -> pw_name + 1, name) == 0) {
      savepasswd = save (passwd);
      if (getnamefromnis (passwd -> pw_name + 1, savepasswd, passwd, 
			  buffer, buflen))
	return (1);
      else
	return (0);
    }
    
    break;

  case '-':
    
    /* Entry Type '-@netgroup' */

    /* If the requested name is in an excluded netgroup, set passwd */
    /* to NULL and return.                                          */

    if (buffer[1] == '@') {
      /* If NIS is not running, ignore this entry */
      if (domain == (char *) NULL) 
	break;

      if (innetgr (passwd -> pw_name + 2, (char *) NULL, name, domain)) {
	return (NIS_DISALLOW);
      }
    }

    /* Entry Type '-name' */
    /* If this entry excludes the requested name, set passwd to NULL */
    /* and return.                                                   */

    else if (strcmp (passwd -> pw_name + 1, name) == 0) {
      return (NIS_DISALLOW);
    }
   
    break;

  default:
    if (strcmp (passwd -> pw_name, name) == 0)
      return (1);
  }
  return (0);
}


/* matchuid:                                                      */
/*             Locate passwd with uid 'uid' in either the         */
/*             NIS passwd database or the local passwd database.  */
/*             Check NIS only if the variable domain is set.      */
/*             If domain is NULL, ignore NIS entries.             */
/*             Return 1 if a match is found;                      */
/*                    0 if a match is not found;                  */
/*                      or if this entry is excluded by           */
/*                      an NIS entry.                             */

static int
  matchuid (char *buffer, int buflen, struct passwd *passwd, uid_t uid)
{
  struct passwd *savepasswd;
  char netgroup[MAX_NETGRNAME];

  switch (buffer[0]) {
  case '+':
    /* If NIS is not running, ignore this entry */
    if (domain == (char *) NULL) 
      break;

    /* Entry Type: '+:' */

    if (strcmp (passwd -> pw_name, "+") == 0) {

      /* Save the pw_passwd, pw_gecos, pw_dir, and pw_shell fields      */
      /* from the local passwd entry, if they exist.                    */

      savepasswd = save (passwd);

      /* Search the NIS passwd database for a match.                 */

      if (getuidfromnis (uid, savepasswd, passwd, buffer, buflen))
	return (1);
      else return (0);
    }

    /* Entry Type: '+@netgroup' */

    if (buffer[1] == '@') {
      (void) strcpy (netgroup, passwd -> pw_name + 2);
      savepasswd = save (passwd);
      if (getuidfromnis (uid, savepasswd, passwd, buffer, buflen)) {
	if (innetgr (netgroup, (char *) NULL, passwd -> pw_name, domain))
	  return (1);
      }
      return (0);
    }

    /* Entry Type: '+name' */

    savepasswd = save (passwd);
    if ((getnamefromnis (passwd -> pw_name + 1, savepasswd, passwd, 
			 buffer, buflen))
	&& (passwd -> pw_uid == uid))
      return (1);
    else return (0);
    
    break;

  case '-':

    /* Entry Type: '-@netgroup' */

    if (buffer[1] == '@') {
      /* If NIS is not running ignore this entry */
      if (domain == (char *) NULL) 
	break;

      (void) strcpy (netgroup, passwd -> pw_name + 2);
      if (getuidfromnis (uid, &NULLPASSWD, passwd, buffer, buflen)) {
	if (innetgr (netgroup, (char *) NULL, passwd -> pw_name, domain)) 
	  return (NIS_DISALLOW);
      } else
	return (0);
    
    /* Entry Type: '-name' */

    } else if (uid == uidof (passwd -> pw_name + 1)) {
      return (NIS_DISALLOW);
    }
    
    break;
    
  default:
    if (passwd -> pw_uid == uid)
      return (1);
  }
  return (0);
}

/* fetchpw                                               */
/*          Find passwd entry that matches the parameter */
/*          'key' in the local passwd database           */

static int
fetchpw (DBM *_pw_db, datum key, struct passwd *passwd, char *line, int len)
{
        register char *cp, *tp;
        int i;

        if ((passwd == NULL) || (line == NULL) || (len < 1)) {
#ifdef _THREAD_SAFE
	  seterrno (EINVAL);
#endif /* _THREAD_SAFE */
	  return (-1);
        }
        
        if (key.dptr == 0) {
#ifdef _THREAD_SAFE
	  seterrno (EINVAL);
#endif /* _THREAD_SAFE */
	  return (-1);
        }

	key = dbm_fetch(_pw_db, key);
        if (key.dptr == 0) {
#ifdef _THREAD_SAFE
	  seterrno(EINVAL);
#endif /* _THREAD_SAFE */
	  return(-1);
        }

	cp = key.dptr;
        tp = line;

	passwd->pw_name = tp; 
	while (*tp++ = *cp++);

	passwd->pw_passwd = tp; 
	while (*tp++ = *cp++);

	bcopy(cp, (char *)&i, sizeof(i));
        cp += sizeof(i);
        passwd->pw_uid = i;
        bcopy(cp, (char *)&i, sizeof(i));
        cp += sizeof(i);
        passwd->pw_gid = i;

	/* The only difference in the pwd structure between DEC OSF/1
	 * and System V is in the definition of the following field.
	 * However, both systems ignore it because it is not present
	 * in the actual password file.
	 */
#ifndef SYSTEM_FIVE
        passwd->pw_quota = 0;
#else
	passwd->pw_age = (char *) 0;
#endif

	cp += sizeof(i);
        
	passwd->pw_comment = tp; 
	while (*tp++ = *cp++);

        passwd->pw_gecos = tp; 
	while (*tp++ = *cp++);

        passwd->pw_dir = tp; 
	while (*tp++ = *cp++);

        passwd->pw_shell = tp; 
	while (*tp++ = *cp++);

        return(0);
}

/* bsd_siad_setpwent:                                                 */
/*                Prepare for search through local and NIS        */
/*                databases.                                      */

int
siad_setpwent (FILE **context) /* external SIA interface */
{
return bsd_siad_setpwent(context);
}

static	int
  bsd_siad_setpwent (FILE **context)
{
  /* Open the local passwd database file, if it isn't already open. */
  /* Otherwise, rewind the file.                                    */
  if (!*context)
	{
    *context = fopen (passwd_file, "r");
    pw_file = *context;
	}
  else
    rewind (*context);

  /* Set NIS domain variable if NIS is configured and running.  */
  /* 'use_nis' is used by _local routines to exclude NIS.       */
  if (use_nis)
    domain = yellowup (1);
  else 
    domain = NULL;

  /* Clean up NIS data */
  free_nis();

  setpwent_flag = 1;
}

/* bsd_siad_endpwent:                                                  */
/*                 Close the local passwd database file and clean  */
/*                 up the NIS data.                                */
/*                 Return SIADSUCCESS.                             */

int
siad_endpwent (FILE **context)	/* external SIA interface */
{
return bsd_siad_endpwent(context);
}

static	int
  bsd_siad_endpwent (FILE **context)
{
  _pw_stayopen = 0;

  /* Close the local passwd database file */
  if (*context) {
    (void) fclose (*context);
    *context = NULL;
    pw_file = NULL;
  }

  /* Clean up NIS data */
  free_nis();
  
  endnetgrent();
  setpwent_flag = 0;
  return (SIADSUCCESS);
}


/* bsd_siad_getpwent:                                                       */
/*                 Return values:                                       */
/*                    SIADSUCCESS if a passwd entry is returned.        */
/*                    SIADCONTINUE if there are no more passwd entries. */
/*                    SIADFAIL if a failure occurs.                     */

int
siad_getpwent(struct passwd *result,char *buffer,int buflen, FILE **context)
{
return bsd_siad_getpwent(result,buffer,buflen,context);
}

static 	int
  bsd_siad_getpwent(struct passwd *result, char *buffer, int buflen, FILE **context)
{
  static struct passwd *savepasswd;
  char *user = NULL; 
  char *mach = NULL;
  char *dom = NULL;

  /* Parameter checking */
  if (result == NULL || buffer == NULL || buflen < 1) {
    return (SIADFAIL);
  }
  /* If setpwent has not been called, call it */
#ifdef	_THREAD_SAFE
  if(!*context)
#else
  if (setpwent_flag == 0)
#endif	/* _THREAD_SAFE */
    (void) bsd_siad_setpwent(context);

  /* If the local passwd database file cannot be opened return SIADFAIL. */
  if (!*context) {
    return (SIADFAIL);
  }

  for (;;) {
      
      /* If nis points to a line in the NIS passwd database, process  */
      /* that line.  Otherwise, process a line from the local passwd  */
      /* database file.                                               */

      if (nis_flag) {
	
	/* set buffer to point to the next line in the NIS passwd database */
	(void) getnextfromnis (buffer, buflen);
	if (*buffer == NULL)
	  nis_flag = 0;

	/* if this passwd is excluded by the local passwd database file, */
	/* do not return this passwd                                     */

	if ((interpretwithsave (buffer, savepasswd, result)) &&
	    (!onminuslist (result -> pw_name, minuslist))) 
	  return (SIADSUCCESS);
	
      } else if (getnetgrent (&mach, &user, &dom)) {
	if (user) {
	  if ((getnamefromnis (user, savepasswd, result, buffer, buflen))
	      && !onminuslist (result -> pw_name, minuslist)) {
	    return (SIADSUCCESS);
	  }
	}
      } else {
	endnetgrent();
	result -> pw_name = (char *) NULL;
	while (result -> pw_name == (char *) NULL) {
	  if (fgets (buffer, buflen, *context) == NULL) { 
	    return (SIADFAIL);
	  }
	  /* ignore bad entries */
	  if (interpret (buffer, result) == 0)
	    result->pw_name = (char *) NULL;
	}
	
	switch (buffer[0]) {
	case '+':

	  /* If NIS is not running, ignore this entry */
	  if (domain == (char *) NULL) 
	    break;
	  
	  /* Entry Type '+:' */

	  if (strcmp (result -> pw_name, "+") == 0) {
	    savepasswd = save (result);
	    (void) getfirstfromnis (buffer, buflen);
	    
	    if (*buffer != NULL) {
	      nis_flag = 1;
	      if ((interpretwithsave (buffer, savepasswd, result)) &&
		  (!onminuslist (result -> pw_name, minuslist))) 
		return (SIADSUCCESS);
	    }

	  } else if (buffer[1] == '@') {
	    
	    /* Entry Type '+@' */

	    savepasswd = save (result);
	    if (innetgr (result -> pw_name + 2, (char *) NULL, "*", domain)) 
	      {
		/* include the whole NIS database */
		(void) getfirstfromnis (buffer, buflen);
		if (*buffer != NULL) {
		  nis_flag = 1;
		  if ((interpretwithsave (buffer, savepasswd, result)) &&
		      (!onminuslist (result -> pw_name, minuslist))) 
		    return (SIADSUCCESS);
		}

	      } else {
		setnetgrent (result -> pw_name + 2);
	      } 

	  } else {

	    /* Entry Type '+name' */

	    savepasswd = save (result);
	    if ((getnamefromnis (result -> pw_name + 1, savepasswd, result,
				 buffer, buflen))
		&& (!onminuslist (result -> pw_name, minuslist)))
	      return (SIADSUCCESS);
	  }

	  break;

	case '-':

	  /* Entry Type '-@' */

	  if (buffer[1] == '@') {
	    /* If NIS is not running, ignore this entry */
	    if (domain == NULL) 
	      break;

	    if (innetgr (result -> pw_name + 2, (char *) NULL, "*", domain)) 
	      {
		/* everybody was subtracted */
		return (SIADFAIL);
	      }
	    setnetgrent (result -> pw_name + 2);
	    while (getnetgrent (&mach, &user, &dom)) {
	      if (user) {
		addtominuslist (user, &minuslist);
	      }
	    }
	    endnetgrent();
	  } else { 

	    /* Entry Type '-name' */

	    if (addtominuslist (result -> pw_name + 1, &minuslist) == NULL) {
	      return (SIADFAIL);
	    }
	  }
	  break;

	default:
	  if (!onminuslist (result -> pw_name, minuslist)) 
	    return (SIADSUCCESS);
	    
	  break;
	}    
      }
    }
}

/* bsd_siad_getpwuid:                                                       */
/*                 Return values:                                       */
/*                   SIADSUCCESS if a match is found in local/NIS       */
/*                   SIADCONTINUE if a match is not found in local/NIS  */
/*                   SIADFAIL if a failure occurs OR this entry is      */
/*                            disallowed.                               */
int
siad_getpwuid(uid_t uid, struct passwd *result, char *buffer, int buflen)
{
return(bsd_siad_getpwuid(uid,result,buffer,buflen));
}

static	int
  bsd_siad_getpwuid (uid_t uid, struct passwd *result, char *buffer, int buflen)
{
  int res;
  datum key;
  FILE *context=NULL;
#ifdef	_THREAD_SAFE
  DBM *_pw_db = NULL;
#endif	/* _THREAD_SAFE */

  /* Parameter checking */
  if (result == NULL || buffer == NULL || buflen < 1) {
    return (SIADFAIL);
  }

  /* Check if a dbm local passwd database exists.  If so, search it */
  /* first.  Otherwise, go immediately to the local/NIS search.     */

  if (_pw_db != (DBM *)0 ||
      (_pw_db = dbm_open(passwd_file, O_RDONLY, 0)) != (DBM *)0) {

    if (flock (dbm_dirfno (_pw_db), LOCK_SH) < 0) {
      dbm_close (_pw_db);
      _pw_db = (DBM *) 0;
    } else {
      key.dptr = (char *) &uid;
      key.dsize = sizeof uid;
      if (fetchpw (_pw_db, key, result, buffer, buflen) == 0) {
	(void) flock (dbm_dirfno(_pw_db), LOCK_UN);
	if (/*!_pw_stayopen*/ 1) {
	  dbm_close (_pw_db);
	  _pw_db = (DBM *) 0;
	  (void) bsd_siad_endpwent(&context);
	}
	return (SIADSUCCESS);
      } else {
	(void) flock (dbm_dirfno(_pw_db), LOCK_UN);
        if (/*!_pw_stayopen*/ 1) {
          dbm_close (_pw_db);
          _pw_db = (DBM *) 0;
        }
      }
    }
  }


  /* The entry was not found in a local dbm passwd database file.  */
  
  /* Prepare for local/NIS search */
  (void) bsd_siad_setpwent(&context);
  
  /* Return SIADFAIL if the local passwd database file is not open */
  if (!context) {
    return (SIADFAIL);
  }

  /* Read the local passwd database file one line at a time */
  while (fgets (buffer, buflen, context) != NULL) {
    /* ignore invalid entries */
    if (interpret (buffer, result))
      {
	/* does this entry's uid match the uid we're looking for? */
	if ((res = matchuid (buffer, buflen, result, uid)) == 1) {
	  /* Clean up after the local/NIS search */
	  if (/*!_pw_stayopen*/ 1)
	    (void) bsd_siad_endpwent(&context);
	  return (SIADSUCCESS);
	} else if (res == NIS_DISALLOW) {
	  if (/*!_pw_stayopen*/ 1)
	    (void) bsd_siad_endpwent(&context);
	  return (SIADFAIL);
	}
      }
  }
  if (/*!_pw_stayopen*/ 1)
    (void) bsd_siad_endpwent(&context);
  return (SIADFAIL);
}


/*****************************************************************************
* Usage:  int bsd_siad_getpwnam (char *name, struct passwd *result, char *buffer, 
*                            int len)
*
* Description: bsd_siad_getpwnam searches the local and NIS passwd databases
*              for an entry with a passwd name that matches the requested
*              passwd name.  siad_getpwnam is called from sia_getpasswd
*              which is called from either getpasswdnam (libc) or 
*              getpasswdnam_r (libc_r). 
*
* Parameter Descriptions:  
*                         name: the name of the requested passwd
*                       result: a pointer to a passwd structure which will
*                               be filled in by this routine
*                       buffer: a pointer to a buffer which contains the
*                               strings pointed to by the passwd structure
*                          len: the length of the buffer
*
* Success return: SIADSUCCESS
*
* Error Conditions and returns:
*
*       Condition1: a match is not found in local/NIS
*       Return: SIADCONTINUE
*
*       Condition2: cannot open the local passwd database file
*       Return: SIADFAIL
*****************************************************************************/
int
  siad_getpwnam (char *name, struct passwd *result, char *buffer, int buflen)
{
return(bsd_siad_getpwnam(name,result,buffer,buflen));
}
int	/* global for other siad references within libc */
  bsd_siad_getpwnam (char *name, struct passwd *result, char *buffer, int buflen)
{
  int res;
  datum key;
  FILE *context=NULL;
#ifdef	_THREAD_SAFE
  DBM *_pw_db = NULL;
#endif	/* _THREAD_SAFE */

  /* Parameter checking */
  if (result == NULL || buffer == NULL || buflen < 1) {
    return (SIADFAIL);
  }

  /* Check if a dbm local passwd database exists.  If so, search it */
  /* first.  Otherwise, go immediately to the local/NIS search.     */


  if (_pw_db != (DBM *)0 ||
	    (_pw_db = dbm_open(passwd_file, O_RDONLY, 0)) != (DBM *)0) {


    if (flock (dbm_dirfno (_pw_db), LOCK_SH) < 0) {
      dbm_close (_pw_db);
      _pw_db = (DBM *) 0;
    } else {
      key.dptr = name;
      key.dsize = strlen (name);
      if (fetchpw (_pw_db, key, result, buffer, buflen) == 0) {
	(void) flock (dbm_dirfno(_pw_db), LOCK_UN);
	if (/*!_pw_stayopen*/ 1) {
	  dbm_close (_pw_db);
	  _pw_db = (DBM *) 0;
	  (void) bsd_siad_endpwent(&context);
	}
	return (SIADSUCCESS);
      } else {
	(void) flock (dbm_dirfno(_pw_db), LOCK_UN);
        if (/*!_pw_stayopen*/ 1) {
          dbm_close (_pw_db);
          _pw_db = (DBM *) 0;
        }
      } 
    }
  }


  /* The entry was not found in a local dbm passwd database file.  */
  
  /* Prepare for local/NIS search */
  (void) bsd_siad_setpwent(&context);
  
  /* Return SIADFAIL if the local passwd database file is not open */
  if (!context) {
    return (SIADFAIL);
  }

  /* Read the local passwd database file one line at a time */
  while (fgets (buffer, buflen, context) != NULL) {
    /* ignore invalid entries */
    if (interpret (buffer, result)) 
      {
	if ((res = matchname (buffer, buflen, result, name)) == 1) {
	  /* Clean up after the local/NIS search */
	  if (/*!_pw_stayopen*/ 1)
	    (void) bsd_siad_endpwent(&context);
	  return (SIADSUCCESS);
	} else if (res == NIS_DISALLOW) {
	  if (/*!_pw_stayopen*/ 1)
	    (void) bsd_siad_endpwent(&context);
	  return (SIADFAIL);
	}
      }
  }
 
  if (/*!_pw_stayopen*/ 1)
    (void) bsd_siad_endpwent(&context);
  return (SIADFAIL);
}

/* setpwfile:                                                             */  
/*            Sets the pathname of the passwd file and optional hashed    */
/*            database to be used for local passwd lookups.  If a passwd  */
/*            file has been left open by a call to setpwent or getpwent,  */
/*            setpwfile will close it first.                              */

void
  setpwfile (char *file)
{
  FILE *context=NULL;
#ifdef _THREAD_SAFE
  SIATHREADLOCK (SIA_PASSWD_LOCK);
#endif /* _THREAD_SAFE */

  (void) bsd_siad_endpwent(&context);
  strncpy (passwd_file, file, sizeof passwd_file);

#ifdef _THREAD_SAFE
  SIATHREADREL (SIA_PASSWD_LOCK);
#endif /* _THREAD_SAFE */
}


struct passwd *
  getpwent_local (void)
{
  static FILE *context=NULL;
  use_nis = 0;

  if (bsd_siad_getpwent (&siad_bsd_passwd, siad_bsd_getpwbuf, SIABUFSIZ, &context) == 0) {
	use_nis = 1;
        return (NULL);
  }

  use_nis = 1;
  return (&siad_bsd_passwd); 
}

struct passwd *
  getpwuid_local (uid_t uid)
{
  use_nis = 0;

  if (bsd_siad_getpwuid (uid, &siad_bsd_passwd, siad_bsd_getpwbuf, 
		     SIABUFSIZ) == 0) {
	use_nis = 1;
        return (NULL);
  }	

  use_nis = 1;
  return (&siad_bsd_passwd); 
}

struct passwd *
  getpwnam_local (char *name)
{
  use_nis = 0;

  if (bsd_siad_getpwnam (name, &siad_bsd_passwd, siad_bsd_getpwbuf, 
		     SIABUFSIZ) == 0) {
	use_nis = 1;
        return (NULL);
  }

  use_nis = 1;
  return (&siad_bsd_passwd); 
}

/*****************************************************************************
* Usage:  struct group *fgetpwent (FILE *fp)
* Usage:  int fgetpwent_r(FILE *fp, struct passwd *pw, char *buf, int buflen)
*
* Description: fgetpwent returns a pointer to a static buffer containing the
*	       next passwd-file-formatted entry read from the file pointer
*	       provided, or NULL on error or EOF.
*	       fgetpwent_r returns 0 on success or -1 on error, with errno
*	       setup as appropriate.
*
* Parameter Descriptions:  
*			    fp: a pointer to a stdio stream for reading.
*			    pw: a pointer to where to write the result.
*			   buf: a pointer to a buffer for holding the strings
*				to which the resulting structure points.
*			   len: the size in bytes allocated to buf.
*
* Success return: A pointer to static data containing a passwd record.
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
siad_fgetpwent(FILE *fp, struct passwd *result, char *buffer, int buflen)
{
  char *save_domain;
  int   save_flag;
  int   save_nis;
  int   status;

  /* Check parameters */
  if (!fp || !result || !buffer || buflen <= 0) {
    TS_SETERR(EINVAL);
    return (-1);
  }
  
  /* Just in case, provide thread locking */
  SIATHREADLOCK(SIA_PASSWD_LOCK);

  /* Fake a different setpwent environment, using the supplied fp and
     without NIS */
  save_flag = setpwent_flag;
  save_domain = domain;
  save_nis = use_nis;
  setpwent_flag = 1;
  domain = (char *) NULL;
  use_nis = 0;

  /* Get the next entry from the file */
  status = bsd_siad_getpwent(result, buffer, buflen, &fp);

  /* Put things back the way we found them */
  setpwent_flag = save_flag;
  domain = save_domain;
  use_nis = save_nis;

  /* Unlock if we locked */
  SIATHREADREL(SIA_PASSWD_LOCK);

  /* Return as appropriate based on what getpwent gave us */
  if ((status & SIADSUCCESS) == SIADSUCCESS)
    return 0;
  if (!TS_GETERR())
    TS_SETERR(ESRCH);
  return (-1);
}
#if defined(_THREAD_SAFE) || defined(_REENTRANT)
int
fgetpwent_r(FILE *fp, struct passwd *result, char *buffer, int buflen)
{
  return siad_fgetpwent(fp, result, buffer, buflen);
}
#else /* not thread-safe */
struct passwd *
fgetpwent(FILE *fp)
{
  int   status;

  /* Fetch to the static buffer */
  status = siad_fgetpwent(fp, &siad_bsd_passwd, siad_bsd_getpwbuf, SIABUFSIZ);

  /* Return as appropriate based on what getpwent gave us */
  if (status == 0)
    return &siad_bsd_passwd;
  return (struct passwd *) NULL;
}
#endif /* _THREAD_SAFE || _REENTRANT */
