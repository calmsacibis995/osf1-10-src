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
static char	*sccsid = "@(#)$RCSfile: catopen.c,v $ $Revision: 4.3.11.5 $ (DEC) $Date: 1993/10/18 15:34:23 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: LIBCMSG
 *
 * FUNCTIONS: catopen, _cat_do_open, make_set, _cat_openfile,
 *    cat_already_open, add_open_cat, catclose
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   1.40  com/lib/c/msg/catopen.c, libcmsg, bos320, 9132320b 8/1/91 10:03:14
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak catclose = __catclose
#pragma weak catopen = __catopen
#endif
#include <locale.h>
#include "catio.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>

static CATD *catdtbl[NL_MAXOPEN];       /* list of open catalogue pointers */

static int make_sets(nl_catd);		/* create text tables */
static void add_open_cat(nl_catd);      /* remember which catalogue is open */
static nl_catd cat_already_open(char*, char*, char*); /* find open catalogue */
static int _cat_openfile(nl_catd);
static char *c = "C";
static char *empty_string = "";

/*
 * NAME: catopen
 *                                                                    
 * FUNCTION: Set up a deferred open for a message catalog. If a catalog message
 *      is requested by catgets(), the partial open started here
 *      will be completed by _cat_do_open.
 *
 * ARGUMENTS:
 *      catname         - name of message catalog to be opened
 *      oflag           - "officially" unused, but required by X/Open.
 *
 * EXECUTION ENVIRONMENT:
 *
 * 	catopen executes under a process.	
 *
 * RETURNS: Returns nl_catd, which is a pointer to a CATD
 *
 */  

nl_catd
catopen (const char *catname, int oflag)
{
    int		errno_save = _Geterrno();
    nl_catd	catd;
    char	*lc_message;
    char	*nlspath;

    if(!catname)
	RETURN(errno_save,CATD_ERR);
      
    /*
     * Return existing CATD if catalog already open
     * Just increment catopen() count
     */

    LOCK;

    if (oflag == NL_CAT_LOCALE)
	/*
	 * XPG4 behavior desired
	 */
	lc_message =  setlocale(LC_MESSAGES, NULL);
    else {
	/*
	 * XPG3 behavior desired.  Get the LANG environment variable
	 * and use it.
	 */
	if((lc_message = getenv("LANG")) == NULL)
	  lc_message = empty_string;
    }

    if((nlspath  = getenv("NLSPATH")) == NULL)
       nlspath = empty_string;


    if ((catd = cat_already_open((char *)catname, lc_message, nlspath)) != NULL) {
	/*
	 * CACHE HIT!  Bump reference count and return quickly
	 */
	catd->_count++;
	UNLOCK;
	RETURN(errno_save,catd);
    }
    
    /*
     * Allocate new CATD and add to catdtbl list - for future catopen()
     * of the same catalogue
     */


    if((catd=(CATD *)calloc(1, sizeof(CATD))) == NULL) {
	UNLOCK;
	RETURN(errno_save,CATD_ERR);
    }
    

    catd->_name = strdup(catname);

    if(lc_message[0] == 'C' && lc_message[1] == '\0')  /* memory saver */
        catd->_lc_message = c;

/* for some reason, the session manager sometimes sets the LANG env */
/* variable to "", so we have to check for this as well.            */

    else if(lc_message == empty_string || strcmp(lc_message,"") == 0)
        catd->_lc_message = empty_string;
    else
        catd->_lc_message = strdup(lc_message);

    if(nlspath == empty_string)                         /* memory saver */
        catd->_nlspath = empty_string;
    else
        catd->_nlspath = strdup(nlspath);

    if ( !catd->_name || !catd->_lc_message || !catd->_nlspath) {
	/*
	 * No memory for one or more of the strings
	 */
	UNLOCK;

	free(catd->_name);
	if(catd->_lc_message != empty_string && (catd->_lc_message != c))
	     free(catd->_lc_message); 
	                              
	if(catd->_nlspath != empty_string)
	     free(catd->_nlspath);
	free(catd);

	RETURN(errno_save,CATD_ERR);
    }

    catd->_magic = CAT_MAGIC;		/* Mark as valid descriptor */    
    catd->_count = 1;			/* Reference count prevents preemptive close */
    catd->_fd = FILE_CLOSED;		/* Indicate not opened yet... */
    add_open_cat(catd);
    UNLOCK;
    RETURN(errno_save,catd);
}



/*
 * NAME: NLcatopen
 *                                                                    
 * FUNCTION: dummy function for catopen to provide binary
 *           compatibility. 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *      NLcatopen executes under a process.
 *
 * RETURNS: Returns a pointer to CATD
 *
 */  

#pragma weak NLcatopen = __catopen

/* 
nl_catd NLcatopen(char *cat, int dummy)
*/
                /*---- name of the catalog to be opened ----*/
                /*----  dummy variable  ----*/
/*
{
        return catopen(cat, dummy);
}
*/


/*
 * 
 * NAME: _cat_do_open
 *                                                                    
 * FUNCTION: Opens a catalog file, reads in and builds an index table.
 *                                                                    
 * ARGUMENTS:
 *      catd            - catalog descripter obtained from catopen
 *
 * RETURNS: Returns 1 if catalogue opened successfully
 *      If any error, returns 0
 *
 */  

#ifdef	_THREAD_SAFE
/*
 * Caller should take _catalog_rmutex lock before calling this routine.
 */
#endif	/* _THREAD_SAFE */

/*
 * This macro estimates whether a fully expanded set or message array
 * would have too many holes (unused elements).  Too many holes wastes
 * memory.
 */

#define TOO_MANY_HOLES(num_holes, num_non_holes) \
    (((num_holes) > 100) && ((num_holes) > (num_non_holes)))

int
_cat_do_open(nl_catd catd)
{
    int i;			/*---- Misc counter(s) used for loop */
    struct _catset *cs;
    struct stat	statbuf;
    off_t	catlen;
    int errno_save;
    caddr_t	p;		/* Compute address of message set within catalog */
    int num_holes;		
    
    errno_save = _Geterrno();
    
    catd->_fd = _cat_openfile( catd );
    if (catd->_fd == FILE_UNUSED || catd->_magic != CAT_MAGIC) {
	RETURN(errno_save, 0);
    }
    
    if (fstat(catd->_fd, &statbuf) == -1)
      RETURN(errno_save, 0);
    
    catd->_catlen = catlen = statbuf.st_size;
    if (catlen < sizeof(struct _header))
      /*
       * This was TOO small to be a message catalog!
       */
      RETURN(errno_save, 0);
    
    p = mmap( NULL, catlen, 	/* Any address, whole file */
	     PROT_READ,		/* Only reading */
	     MAP_FILE|MAP_VARIABLE|MAP_PRIVATE,
	     catd->_fd,
	     (off_t) 0 );
    
    close (catd->_fd);

    if (p == (void *) -1) {		/* Could not MMAP it */
	catd->_fd = FILE_UNUSED;
	RETURN(errno_save, 0);
    }
    
    catd->_hd = (struct _header *) p;
    
    if (catd->_hd->_magic != CAT_MAGIC)
      goto error1;
    
    cs = (struct _catset *) (p + sizeof(struct _header));

    for (i = 0 ; i < catd->_hd->_n_sets ; i++) {
	cs = (void *) ((caddr_t) cs + (off_t)(2*sizeof(unsigned short) +
			  cs->_n_msgs * sizeof(struct _msgptr)));
	
	if ( (caddr_t)cs > (p+catlen))
	  goto error1;
    }
    
    catd->_mem = (char *)catd->_hd;

    /*
     * If there aren't many holes in the set numbers, fully expand the
     * compacted set array from the catalog.  Then in catgets(), we'll
     * be able to use the set number to index directly into the
     * expanded array.
     *
     * If there are a lot of holes, leave the set array compacted.  In
     * catgets(), we'll search through it for the requested set.
     */

    num_holes = catd->_hd->_setmax - catd->_hd->_n_sets;
    if (!TOO_MANY_HOLES(num_holes, catd->_hd->_n_sets)) {
	catd->_sets_expanded = TRUE;
	catd->_n_sets = catd->_hd->_setmax;
    }
    else {
	catd->_sets_expanded = FALSE;
	catd->_n_sets = catd->_hd->_n_sets - 1;
    }
    
    catd->_set = (struct _catset *) calloc(1 + catd->_n_sets,
					   sizeof (struct _catset));
    
    if ( catd->_set == NULL )
      goto error1;
    
    /* save the max. set number in catd->_setmax */

    catd->_setmax = catd->_hd->_setmax;
    
    if (!make_sets(catd))
        RETURN(errno_save, 1);

    /*
     * Couldn't complete setup of message catalogs
     */

error1:

    catd->_fd = FILE_UNUSED;			/* Mark as not open */
    munmap((caddr_t) catd->_hd, catlen);	/* Release mapped addr space */

    RETURN (errno_save, 0);			/* Tell the world it failed */
}



/*
 * 
 * NAME: make_sets
 *
 * FUNCTION: Expands the compacted version of the catalog index table into
 *	the fast access memory version.
 *
 * ARGUMENTS:
 *	catd		- Catalog descriptor for mapped file
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Make_set executes under a process.	
 *
 * RETURNS: int
 */

static int
make_sets(nl_catd catd)
{
    struct _catset *cset;
    char	   *base = catd->_mem;
    int		   n_sets = catd->_hd->_n_sets;
    int 	i;		/*---- Misc counter(s) used for loops ----*/
    int	j;			/*---- Misc counter(s) used for loops ----*/
    int 	msgmax;		/*---- The maximum number of _messages in a set ----*/
    char 	*cmpct_set_ptr;	/*---- pointer into the index table ----*/
    struct _catset	cs;	/*---- used to look at the sets in the table -*/
    int		num_holes;
    
    cmpct_set_ptr = base + sizeof(struct _header);
    
    for (i = 0 ; i < n_sets ; i++) {
	/* loop through each compacted set */
	
	cs._setno =  ((struct _catset *)cmpct_set_ptr)->_setno;	
	cs._n_msgs = ((struct _catset *)cmpct_set_ptr)->_n_msgs;	
	/* set the _catset ptr to the base of the current 
	   compacted set.        */
	
	cs._mp = (struct _msgptr *)(cmpct_set_ptr +
				    2 * sizeof(unsigned short));
	/* set the ms array ptr to the base of
	   compacted array of _msgptr's     */

	cset = (catd->_sets_expanded) ?
	    &catd->_set[cs._setno] : &catd->_set[i];

	/*
	 * If there aren't many holes in the message numbers, fully
	 * expand the compacted message array from the catalog.  Then
	 * in catgets(), we'll be able to use the message number to
	 * index directly into the expanded array.
	 *
	 * If there are many holes, leave the message array compacted.
	 * In catgets(), we'll search through it for the requested
	 * message.
	 */

	msgmax = cs._mp[cs._n_msgs - 1]._msgno;
	num_holes = msgmax - cs._n_msgs;
	if (!TOO_MANY_HOLES(num_holes, cs._n_msgs)) {
	    cset->_msgs_expanded = TRUE;
	    cset->_n_msgs = msgmax;
	    cset->_mp = calloc(1 + msgmax, sizeof(struct _msgptr));
	    if (cset->_mp == NULL)
		return (-1);
	
	    for (j = 0; j < cs._n_msgs; j++)
		cset->_mp[cs._mp[j]._msgno] = cs._mp[j];
	}
	else {
	    cset->_msgs_expanded = FALSE;
	    cset->_n_msgs = cs._n_msgs - 1;
	    cset->_mp = cs._mp;
	}

	cset->_setno = cs._setno;

	/* Superfluous but should have the correct data. Increment 
	   the base of the set pointer.          */
	
	cmpct_set_ptr += 2 * sizeof(unsigned short) + cs._n_msgs *
	  sizeof(struct _msgptr);
    }
    return(0);
}



/*
 * 
 * NAME: _cat_openfile
 *
 * FUNCTION: Opens a catalog file, looking in the language path first (if 
 *	there is no slash) and returns a pointer to the file stream.
 *                                                                    
 * ARGUMENTS:
 * 	catd		- message catalog descriptor
 *
 * NOTE: The C locale will always return an error so force programs to
 *      use their default messages. NOTE: this is still not true as
 *      some programs still install default messages in the C message
 *      directory.
 *
 * RETURNS:  Returns file descriptor
 */

static int
_cat_openfile( nl_catd catd )
{
    char 	*nlspath;	/*---- pointer to the nlspath val ----*/
    int		fp;		/* file descriptor */
    char	cpth[PATH_MAX+1]; /*---- current value of nlspath ----*/
    char    	*p,*np;
    char	*fulllang;	/* %L language value */
    char	lang[PATH_MAX+1]; /* %l language value */
    char	*territory;	/* %t language value */
    char	*codeset;	/* %c language value */
    char	*ptr;		/* for decompose of $LANG */
    const char 	*str;
    char	*optr;
    int		nchars;
    int 	lenstr;
    char 	outptr[PATH_MAX+1];
    int		privileged;
    
    privileged = __ispriv();/* Is this a ROOT program? */


    /*
     * Simplest case: message catalog has an explicit path.
     */

    if (strchr(catd->_name,'/')) {
	if ((fp = open(catd->_name, O_RDONLY)) != -1) {
	    return (fp);
	} else {
	    return (FILE_UNUSED);
	}
    }

    /*
     * Use catalog search rules - since someone could change the LC_MESSAGES
     * variable from the time of the open til now, we have to use the
     * saved value from the catalog.
     */
    
    fulllang = basename(catd->_lc_message);

    if ((nlspath = catd->_nlspath) == NULL || *nlspath == '\0')
      /*
       * The NLSPATH variable is not set.  
       * Set nlspath to system default.
       */
      nlspath = PATH_FORMAT; 
    else {
	/*
	 * If the requestor is executing a privileged program,
	 * then don't believe the environment variable
	 * NLSPATH, only search the default path.
	 */
	if (privileged)
	    nlspath = PATH_FORMAT;
    }
    
    /*
     ** LC_MESSAGES is a composite of three fields:
     ** language_territory.codeset
     ** and we're going to break it into those
     ** three fields.
     */
    
    strcpy(lang, fulllang);
    
    territory = "";
    codeset = "";
    
    ptr = (char*) strchr(lang, '_' );
    if (ptr != NULL) {
	territory = ptr+1;
	*ptr = '\0';
	ptr = (char*) strchr(territory, '.');
	if (ptr != NULL) {
	    codeset = ptr+1;
	    *ptr = '\0';
	}
    } else {
	ptr = (char*) strchr( lang, '.' );
	if (ptr != NULL) {
	    codeset = ptr+1;
	    *ptr = '\0';
	}
    }

    if (*codeset) {
	ptr = strchr(codeset,'@');	/* Find special modifier */
	if (ptr) *ptr = '\0';		/*  and chop it off */
    }

    np = nlspath;
    while (*np) {
	p = cpth;
	while (*np && *np != ':')
	  *p++ = *np++;
	*p = '\0';
	if (*np)	/*----  iff on a colon then advance --*/
	  np++;

	if (strlen(cpth)) {
	    ptr = cpth;
	    optr = outptr;
	    
	    nchars = 0;
	    while (*ptr != '\0') {
		while ((*ptr != '\0') && (*ptr != '%') 
		       && (nchars < PATH_MAX+1)) {
		    *(optr++) = *(ptr++);
		    nchars++;
		}
		if (*ptr == '%') {
		    switch (*(++ptr)) {
		      case '%':
			str = "%";
			break;
		      case 'L':
			str = fulllang;
			break;
		      case 'N':
 			str = catd->_name;
			break;
		      case 'l':
			str = lang;
			break;
		      case 't':
			str = territory;
			break;
		      case 'c':
			str = codeset;
			break;
		      default:
			str = "";
			break;
		    }
		    lenstr = strlen(str);
		    nchars += lenstr;
		    if (nchars < PATH_MAX+1) {
			strcpy(optr, str);
			optr += lenstr;
		    } else {	
			break;
		    } 
		    ptr++;
		} else {
		    if (nchars >= PATH_MAX+1) {
			break;
		    }
		}
	    }
	    *optr = '\0';
	    strcpy(cpth, outptr);
	}
	else {		/*----  iff leading | trailing | 
			  adjacent colons ... --*/
	    strcpy(cpth,catd->_name);
	}

	if ( (fp = open(cpth,O_RDONLY))!=-1) {
	    return(fp);
	 }
    }

    if (privileged)
        return (FILE_UNUSED);

    /*
     * Last desperate chance for non-privileged programs - look in the
     * working directory (it might not have been in the default search path
     */

    if ((fp = open(catd->_name, O_RDONLY)) != -1) {
	return(fp);
    }

    return (FILE_UNUSED);
}


/*
 * 
 * NAME: cat_already_open
 *
 * FUNCTION: Check to see if a message catalog has already been opened.
 *                                                                    
 * ARGUMENTS:
 *      catname         - name of message catalog to be opened
 *      lc_message      - value of LC_MESSAGES env variable
 *      nlspath         - value of NLSPATH env variable
 *
 * EXECUTION ENVIRONMENT:
 *
 * 	cat_already_open executes under a process.
 *
 * RETURNS: Returns a pointer to the existing CATD if one exists, and 
 *	a NULL pointer if no CATD exists.
 */

static nl_catd
cat_already_open(char *catname, char *lc_message, char *nlspath)
{
    int 	i;
    
    for (i = 0 ; i < NL_MAXOPEN; i++)  {
	if ((catdtbl[i] != NULL) && !strcmp(catname, catdtbl[i]->_name) &&
	       !strcmp(catdtbl[i]->_lc_message, lc_message) &&
	       !strcmp(catdtbl[i]->_nlspath, nlspath))
	    /*
	     * MATCH!
	     */
			       return(catdtbl[i]);
    }
    return(NULL);
}



/*
 * 
 * NAME: add_open_cat
 *
 * FUNCTION: Add a catalog to the list of already opened catalogs.
 *                                                                    
 * ARGUMENTS:
 *      catd            - catalog descripter returned from catopen
 *
 * NOTE:
 *      If this catalogue descriptor cannot be added to catdtbl[],
 *      future catopen() of the same catalogue will result in a new
 *      catalogue descriptor.
 *
 * EXECUTION ENVIRONMENT:
 *
 * 	add_open_cat executes under a process.	
 *
 * RETURNS: void
 */

static void
add_open_cat(nl_catd catd)
{
    int i;
    
    for (i = 0; i < NL_MAXOPEN && catdtbl[i] != NULL; i++)
      ;
    if (i < NL_MAXOPEN)
      catdtbl[i] = catd;
    return;
}



/*
 * 
 * NAME: catclose
 *                                                                    
 * FUNCTION: Close a message catalog.
 *
 * ARGUMENTS:
 *      catd            - catalog descripter returned from catopen
 *
 * EXECUTION ENVIRONMENT:
 *
 * 	catclose executes under a process.	
 *
 * NOTES: Catclose closes the stream and frees the memory of the catalog
 *      when "open" count is zero.  Otherwise the catalog file is left
 *      open for continued access.
 *
 * RETURNS: 0 on success, -1 on failure.
 *
 */  


int
catclose(nl_catd catd)
{
    int i;

    /*
     * Hide behavior in C locale, by treating a CATD_ERR as "ok"
     */
    if (catd == CATD_ERR)
        return (0);

    /*
     * return error if catd is invalid
     */
    if (catd == NULL)
	return (-1);
    
    LOCK;

    /*
     * return error if catalogue already closed
     */
    if (catd->_count <= 0 || catd->_magic != CAT_MAGIC) {
	UNLOCK;
	return (-1);
    }
    
    /*
     * return ok if catalogue opened more than once
     */
    if (--catd->_count > 0) {
	UNLOCK;
	return (0);
    }
    
    /*
     * return ok after deleting catdtbl entry and closing catalogue
     */
    for (i = 0; i < NL_MAXOPEN; i++)
      if (catdtbl[i] == catd) {
	  catdtbl[i] = NULL;
	  break;
      }


    /*
     * free memory associated with catalogue descriptor
     */

    if (catd->_set) {
	/*
	 * free message offset table and set table
	 */
	for (i = 0; i <= catd->_n_sets; i++) {
	    if (catd->_set[i]._msgs_expanded && catd->_set[i]._mp)
		free(catd->_set[i]._mp);
	}

        free(catd->_set);
    }

    if (catd->_hd)	/* Release mapped addr space */
    	munmap((caddr_t) catd->_hd, catd->_catlen); 
    free(catd->_name);

    catd->_magic = 0;		/* Void the descriptor */
    if(catd->_lc_message != empty_string && (catd->_lc_message != c)) 
        free(catd->_lc_message);
    if(catd->_nlspath != empty_string)
        free(catd->_nlspath);
    free(catd);


    UNLOCK;

    return (0);
}
