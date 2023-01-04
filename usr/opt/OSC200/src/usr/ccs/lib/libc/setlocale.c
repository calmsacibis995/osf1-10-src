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
static char	*sccsid = "@(#)$RCSfile: setlocale.c,v $ $Revision: 4.4.12.5 $ (DEC) $Date: 1993/12/21 23:07:51 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: setlocale
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.12  com/lib/c/loc/setlocale.c, libcloc, bos320, 9132320m 8/11/91 14:14:10
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/limits.h>
#include <sys/localedef.h>
#include <locale.h>
#include <langinfo.h>

/*
** global variables set up by setlocale()
**
**	INVARIANT:	If any global variable changes, setlocale() returns
**			successfully.
*/
extern _LC_charmap_t  * __lc_charmap;
extern _LC_ctype_t    * __lc_ctype;
extern _LC_collate_t  * __lc_collate;
extern _LC_numeric_t  * __lc_numeric;
extern _LC_monetary_t * __lc_monetary;
extern _LC_time_t     * __lc_time;
extern _LC_resp_t     * __lc_resp;
extern _LC_locale_t   * __lc_locale;

extern _LC_locale_t	*_C_locale;	/* Hardwired default */

extern void	*__lc_load( const char *, void *(*)());

static _LC_locale_t *load_all_locales(const char *, const char *, const char *);
static char *locale_name(int category, const char *name, const char *lc_all);
static _LC_locale_t *load_locale(const char *name, const char *locpath);
static char *saved_category_name(int category, const char *name);
static int copy_locale_name(int category, const char *name);

/*
** Static strings to record locale names used to load
** specific locale categories.
**
** This data is used to build the locale state string.
*/

static const char POSIX[] = "POSIX";
static char C[] = "C";

static char *locale_names[_LastCategory+1] = { C, C, C, C, C, C };
static char	*locbuf = NULL;	/* Holds last allocated locale string buffer */
static int	loclen = 0;	/* Remembers maximum length of locbuf */

static struct lconv local_lconv;

static _LC_locale_t locale;

/*
*  FUNCTION: setlocale
*
*  DESCRIPTION:
*  Loads the specified 'category' from the locale database 'locname'.
*
*  RETURNS:
*  A space separated string of names which represents the effective locale 
*  names for each of the categories affected by 'category'.
*/
char *
setlocale(int category, const char *locname)
{
    _LC_locale_t *lp;			/* locale temporory */
   
    char *lc_all;			/* value of LC_ALL environment */
					/* variable  */
    
    char *locpath;			/* value of LOCPATH */
					/* environment variable */
    
    char *s;				/* temporary string pointer */

    /* 
     ** Verify category parameter
     */
    if (!_ValidCategory(category)) 
	return NULL;
    

    /* 
     ** Check special locname parameter overload which indicates that current
     ** locale state is to be saved.
     */
    if (locname == NULL)
	goto getlocale;

    
    /* 
     ** get values of environment variables which are globally usefull to
     ** avoid repeated calls to getenv() 
     */
    locpath = getenv("LOCPATH");
    if (locpath==NULL) locpath = "";
    lc_all  = getenv("LC_ALL");
    if (lc_all==NULL) lc_all = "";
    
    /*
     ** Split logic for loading all categories versus loading a single 
     ** category.
     */
    if (category == LC_ALL) {
	/*
	 ** load all locale categories
	 */
	lp = load_all_locales(lc_all, (char *)locname, locpath);
	if (!lp) return NULL;
	
	__lc_collate  = lp->lc_collate;
	__lc_ctype    = lp->lc_ctype;
	__lc_charmap  = lp->lc_charmap;
	__lc_monetary = lp->lc_monetary;
	__lc_numeric  = lp->lc_numeric;
	__lc_time     = lp->lc_time;
	__lc_resp     = lp->lc_resp;
	__lc_locale   = &locale;

	locale.core.hdr	    = lp->core.hdr;
	locale.core.nl_langinfo = lp->core.nl_langinfo;
	locale.core.localeconv  = lp->core.localeconv;
	locale.core.data        = lp->core.data;
	locale.core.init        = lp->core.init;

	locale.nl_lconv         = &local_lconv;

	locale.lc_collate  = __lc_collate;
	locale.lc_ctype    = __lc_ctype;
	locale.lc_charmap  = __lc_charmap;
	locale.lc_monetary = __lc_monetary;
	locale.lc_numeric  = __lc_numeric;
	locale.lc_time     = __lc_time;
	locale.lc_resp     = __lc_resp;

    } else {
	/*
	 ** load a specific category of locale information
	 */

	lp = load_locale(s=locale_name(category, (char *)locname, lc_all), locpath);

	if ( !lp )
	    return NULL;

	if (copy_locale_name(category, s) == -1)
	    return NULL;

	/* call init method for category changed */
	locale.lc_charmap       = __lc_charmap;
	locale.lc_collate       = __lc_collate;
	locale.lc_ctype         = __lc_ctype;
	locale.lc_monetary      = __lc_monetary;
	locale.lc_numeric       = __lc_numeric;
	locale.lc_time          = __lc_time;
	locale.lc_resp          = __lc_resp;

	locale.core.hdr	    = lp->core.hdr;
	locale.core.nl_langinfo = lp->core.nl_langinfo;
	locale.core.localeconv  = lp->core.localeconv;
	locale.core.data        = lp->core.data;
	locale.core.init        = lp->core.init;

	locale.nl_lconv         = &local_lconv;

#define FILL(category)				\
    locale.lc_##category = lp->lc_##category;	\
    __lc_##category = locale.lc_##category;


	switch (category) {
	  case LC_COLLATE:
	    FILL(collate);
	    FILL(charmap);
	    break;
	  case LC_CTYPE:
	    FILL(ctype);
	    FILL(charmap);
	    break;
	  case LC_MONETARY:
	    FILL(monetary);
	    break;
	  case LC_NUMERIC:
	    FILL(numeric);
	    break;
	  case LC_TIME:
	    FILL(time);
	    break;
	  case LC_MESSAGES:
	    FILL(resp);
	    break;
	}

	__lc_locale = &locale;
    }
#undef FILL

    IF_METHOD(lp,init)
	METHOD(lp,init)(&locale);

    /*
     * Locale is loaded, now return descriptor that shows current settings
     */

getlocale:

    if (category != LC_ALL) {
        return (locale_names[category]);
    } else {

	int	i;
	size_t	len=0;

	for(i=0; i<=_LastCategory; i++)
	  len += strlen(locale_names[i])+1;	/* category plus blank or NUL */

	if (len > loclen) {
	    free(locbuf);
	    loclen = 0;
	    locbuf = malloc(len);
	    if (!locbuf)			/* Partial failure */
	        return(NULL);			/*  on no storage */
	    loclen = len;			/* Remember space for string */
	}

	sprintf(locbuf,
		"%s %s %s %s %s %s",
		locale_names[LC_COLLATE],
		locale_names[LC_CTYPE],
		locale_names[LC_MONETARY],
		locale_names[LC_NUMERIC],
		locale_names[LC_TIME],
		locale_names[LC_MESSAGES]);

	return (locbuf);
    }
}


/*
*  FUNCTION: load_all_locales
*
*  DESCRIPTION:
*  loads all of the locales from the appropriate locale databases.
*
*  RETURNS:
*  a pointer to a locale handle containing an object for each of the 
*  locale categories, or NULL on errors
*
*  This function sets up the locale name for each of the categories
*/
static _LC_locale_t *
load_all_locales(const char *lc_all, const char *locname, const char *locpath)
{
    int	i;

    /* 
     * special case for C locale 
     */
    if ((locname[0] == 'C' && locname[1] == '\0') ||
	!strcmp(locname, "C C C C C C") ) {

	locname = C;
	for (i=0;i<=_LastCategory; i++)
	    copy_locale_name(i, (char *)locname);
	return (_C_locale);

    } else if ((strcmp(locname,POSIX)==0) || 
	       !strcmp(locname, "POSIX POSIX POSIX POSIX POSIX POSIX")) {
	
	locname = POSIX;
	for (i=0; i<=_LastCategory; i++)
	    copy_locale_name(i, (char *)locname);
	return (_C_locale);

    } else {

	char	*s;
	int	diff_flag = FALSE;
	_LC_locale_t	*lp;
	static	_LC_locale_t	locale;

	/*
	  build locale piecemeal from each of the categories
	*/

	s=locale_name(LC_COLLATE, locname, lc_all);
	lp = load_locale(s, locpath);
	if (!lp) 
	    return lp;

	if (copy_locale_name(LC_COLLATE, s) == -1)
	    return NULL;
	locale.lc_collate = lp->lc_collate;
	
	s=locale_name(LC_CTYPE, locname, lc_all);
	if (strcmp(s, locale_names[LC_COLLATE])!=0) {
	    diff_flag = TRUE;
	    lp = load_locale(s, locpath);
	    if (!lp) 
		return lp;
	}

	if (copy_locale_name(LC_CTYPE, s) == -1)
	    return NULL;
	locale.lc_ctype = lp->lc_ctype;
	locale.lc_charmap = lp->lc_charmap;
	
	s=locale_name(LC_MONETARY, locname, lc_all);
	if (strcmp(s, locale_names[LC_CTYPE]) != 0) {
	    diff_flag = TRUE;
	    lp = load_locale(s, locpath);
	    if (!lp) 
		return lp;
	}
	if (copy_locale_name(LC_MONETARY, s) == -1)
	    return NULL;
	locale.lc_monetary = lp->lc_monetary;
	
	s=locale_name(LC_NUMERIC, locname, lc_all);
	if (strcmp(s, locale_names[LC_MONETARY]) != 0) {
	    diff_flag = TRUE;
	    lp = load_locale(s, locpath);
	    if (!lp) 
		return lp;
	}
	if (copy_locale_name(LC_NUMERIC, s) == -1)
	    return NULL;
	locale.lc_numeric = lp->lc_numeric;
	
	s=locale_name(LC_TIME, locname, lc_all);
	if (strcmp(s, locale_names[LC_NUMERIC]) != 0) {
	    diff_flag = TRUE;
	    lp = load_locale(s, locpath);
	    if (!lp) 
		return lp;
	}
	if (copy_locale_name(LC_TIME, s) == -1)
	    return NULL;
	locale.lc_time = lp->lc_time;

	s=locale_name(LC_MESSAGES, locname, lc_all);
	if (strcmp(s, locale_names[LC_TIME]) != 0) {
	    diff_flag = TRUE;
	    lp = load_locale(s, locpath);
	    if (!lp) 
		return lp;
	}
	if (copy_locale_name(LC_MESSAGES, s) == -1)
	    return NULL;
	locale.lc_resp = lp->lc_resp;
	
	if (!diff_flag)
	    return lp;
	else {
	    /* 
	     * set up core part of locale container object 
	     */
	    locale.core.hdr	    = lp->core.hdr;
	    locale.core.nl_langinfo = lp->core.nl_langinfo;
	    locale.core.localeconv  = lp->core.localeconv;
	    locale.core.data        = lp->core.data;
	    locale.nl_lconv         = &local_lconv;

	    locale.core.init = lp->core.init;

	    /* 
	     * call init method which should populate lc_info and lc_lconv
	     * from locale category handles
	     */
	    if (lp->core.init != 0)
		METHOD(lp, init)(&locale);

	    return &locale;
	}
    }
}


/*
*  FUNCTION: locale_name
*
*  DESCRIPTION:
*  This function returns the name which should be used to load a 
*  locale category given the 'name' parameter passed to setlocale() 
*  the value of the LC_ALL environment variable.
*
*  RETURNS:
*  Name for the specified category.
*/
static char *
locale_name(int category, const char *name, const char *lc_all)
{
    char *env_name;

    static const char *category_name[]={
	"LC_COLLATE", "LC_CTYPE", "LC_MONETARY", "LC_NUMERIC", 
	"LC_TIME",  "LC_MESSAGES"
	};
    
    /* 
     ** specified locale name overrides all environment variables 
     */
    if (name[0] != '\0')
	/* 
	 ** check if name is a saved locale state string.
	 */
	return saved_category_name(category, name);
    
    /*
     ** The value of LC_ALL overrides all other environment variables
     */
    if (lc_all[0] != '\0') return (char *)lc_all;
    
    /* 
     ** check environment variable for specified category
     */
    env_name = getenv(category_name[category]);
    
    /* 
     ** use it if it is specified, otherwise use value of LANG
     */
    if (env_name==NULL || env_name[0] == '\0')
	env_name = getenv("LANG");
    
    /*
     ** if even LANG is undefined, then use C
     */
    if (env_name==NULL || env_name[0]=='\0')
	env_name = C;
    
    return env_name;
}


/*
*  FUNCTION: saved_category_name
*
*  DESCRIPTION: 
*  Returns the locale name associated with 'category'.  If 'name' is a saved
*  category string then there will be list of locale names separated by 
*  space characters.  This is decomposed by using strchr() until the 
*  category(th) string is found.  If there are no spaces in the string, 
*  then the 'name' is returned.
*
*  RETURNS:
*  locale name from 'name' string.
*/
static char *
saved_category_name(int category, const char *name)
{
    const char	*endptr=NULL, *headptr=NULL;
    static char	*saved = 0;
    static int incr = 64;
    int  i, len;
    
    for (i=0; i<=category; i++) {
	headptr = name;
	endptr = strchr(headptr, ' ');
	if (endptr == NULL) {
	    break;
	}
	name = endptr+1;	/* Advance over blank */
    }

    if (endptr == NULL)
        endptr = headptr + strlen(headptr);

    /*
     * Now headptr addresses the beginning of a locale-name, and
     * endptr addresses the blank or NUL that terminates it.
     */
    len = endptr - headptr;

    if ((!saved) || (len+1 > incr)) {
	if (len+1 > incr)
	    incr = len + 1;
	if (saved)
	    free(saved);
        saved = (char *)malloc(incr);
    }
    if ( !saved ) return (NULL);

    strncpy(saved,headptr,len);
    saved[len] = '\0';
	
    return saved;
}


/*
*  FUNCTION: copy_locale_name
*
*  DESCRIPTION: 
*  Maintains a static set of increments containing the
*  amount of memory allocated for each category string.
*  This allows re-use of the space used for keeping the
*  the locale names set for each category.
*
*  RETURNS:
*  0 if successful, -1 if otherwise.
*/
static int
copy_locale_name(int category, const char *name)
{
    static int incr[_LastCategory+1];
    int len;
    
    len = strlen(name);
    if (len+1 > incr[category]) {
	incr[category] = len + 1;
	if ((locale_names[category] = (char *)malloc(incr[category])) == NULL)
	    return -1;
    }
    strcpy(locale_names[category], name);
    return 0;
}	


/* 
*  FUNCTION: load_locale 
*
*  DESCRIPTION:
*  This function loads the locale specified by the locale name 'locale',
*  from the list of pathnames provided in 'locpath'.
*
*  RETURNS:
*  Pointer to locale database found in 'locpath'.
*/
static _LC_locale_t *
load_locale(const char *name, const char *locpath)
{
    char path[PATH_MAX+1];
    int	privileged;
    _LC_object_t *handle;

    /*
     * Check for special case.
     */
    if ((name[0] == 'C' && name[1] == '\0') || (strcmp(name,"POSIX")==0))
        return (_C_locale);

    /*
     * check if this is a privileged program.  If so, don't load untrusted code
     */

    privileged = __ispriv();

    if (privileged) {
	if ( strchr(name, '/') )		/* Explicit path */
	    return (NULL);			/* Not legal in privileged program */

	if ( locpath )
	    locpath = _DFLT_LOC_PATH;		/* Ignore non-default paths */
    }


    if (strchr(name, '/')) {
        handle = __lc_load(name, (void *(*)())0);
        if (handle && handle->type_id == _LC_LOCALE)
          return (_LC_locale_t *)(handle);
        else
          return NULL;
    }


    if ( !*locpath )
        locpath = _DFLT_LOC_PATH;


    while ( *locpath ) {
	int i;

	for (i=0; *locpath != ':' && *locpath != '\0' && i<= PATH_MAX+1;i++)
	  path[i] = *locpath++;

	if (*locpath==':')
	  locpath++;

	if (i==0) path[i++] = '.';	
	/* append '/' */
	path[i++] = '/';
	path[i] = '\0';

	/* append locale name */
	strncat(path+i,		/* We know where string-end is */
		name,
		PATH_MAX-i);	/* Remaining space without the trailing null */


	handle = __lc_load(path, (void *(*)())0);
	if (handle && handle->type_id == _LC_LOCALE)
	  return (_LC_locale_t *)(handle);
    } 

    /*
     * No locale loaded!
     */
    return (NULL);	
}
