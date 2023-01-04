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
static char	*sccsid = "@(#)$RCSfile: catgets.c,v $ $Revision: 4.2.8.5 $ (DEC) $Date: 1993/11/18 15:39:22 $";
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
 * FUNCTIONS: catgets
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.19  com/lib/c/msg/catgets.c, libcmsg, bos320,bosarea.9125 6/19/91 15:04:47
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak NLcatgets = __catgets
#pragma weak catgets = __catgets
#endif
#include <nl_types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "catio.h"

/*
 * NAME: catgets
 *                                                                    
 * FUNCTION: Get a pointer to a message from a message catalog.
 *                                                                    
 * ARGUMENTS:
 *      catd            - catalog descripter obtained from catopen()
 *      setno           - message catalogue set number
 *      msgno           - message numner within setno
 *      def             - default message text
 *
 * RETURNS: Returns a pointer to the message on success.
 *      On any error, the default string is returned.
 *
 */  

char *
catgets(nl_catd catd,int setno,int msgno, const char *def)
{
    int 	errno_save;	/* preserve errno for caller */
    struct _catset *set;	/* ptr to set's _catset structure */
    struct _msgptr *msg;	/* ptr to msg's _msgptr structure */
    
    
    /*
     * return default text if catd is invalid or no message catalogue
     */

    if (catd == CATD_ERR || catd == NULL)
      return ((char *)def);
    LOCK;
    if(catd->_fd == FILE_UNUSED || catd->_magic != CAT_MAGIC) {
	UNLOCK;
	return ((char *)def);
    }
    
    errno_save = _Geterrno();
    
    /*
     * perform deferred open now
     * return default text if no message catalogue file
     */
    
    
    if (catd->_fd == FILE_CLOSED) {
	if ( !_cat_do_open(catd)) {
	    catd->_fd = FILE_UNUSED;
	    UNLOCK;
	    RETURN (errno_save, (char *)def);
	}
    }
    
    set = __cat_get_catset(catd, setno);
    if (set) {
	msg = __cat_get_msgptr(set, msgno);
	if (msg) {
	    UNLOCK;
	    RETURN(errno_save, catd->_mem + msg->_offset);
	}
    }

    UNLOCK;
    RETURN(errno_save, (char *)def);
}


/*
 * NAME: compare_sets
 *                                                                    
 * FUNCTION: Compare function used by bsearch() in __cat_get_catset().
 *                                                                    
 * ARGUMENTS:
 *      key             - pointer to set number we're searching for
 *      element         - pointer to current _catset structure
 *
 * RETURNS: Returns -1, 0, or 1, depending on whether the set number
 *          is less than, equal to, or greater than the set number of
 *          the _catset structure.
 *
 */  

static int
compare_sets(const void *key, const void *element)
{
	int *setno = (int *) key;
	struct _catset *set = (struct _catset *) element;

	if (*setno < set->_setno)
		return -1;
	if (*setno > set->_setno)
		return  1;

	return 0;
}


/*
 * NAME: __cat_get_catset
 *                                                                    
 * FUNCTION: Find a set in the catd->_set array.  Assumes that the
 *           sets in the array are sorted by increasing set number.
 *                                                                    
 * ARGUMENTS:
 *      catd            - catalog descripter obtained from catopen()
 *      setno           - message catalogue set number
 *
 * RETURNS: Returns a pointer to the set on success.
 *          On any error, returns NULL.
 *
 */  

struct _catset *
__cat_get_catset(nl_catd catd, int setno)
{
	struct _catset *set;

	if ((catd == (nl_catd) NULL) || (catd == CATD_ERR))
		return (struct _catset *) NULL;

	if (catd->_sets_expanded) {
		if ((setno < 1) || (setno > catd->_n_sets))
			return (struct _catset *) NULL;

		set = &catd->_set[setno];

		/*
		 * Catch empty elements in the array.  They aren't
		 * real sets.
		 */

		if (set->_mp == (struct _msgptr *) NULL)
			return (struct _catset *) NULL;
	}
	else {
		set = (struct _catset *) bsearch((void *) &setno,
						 catd->_set, catd->_n_sets + 1,
						 sizeof(struct _catset),
						 compare_sets);

		/*
		 * Since the sets are compacted, there aren't any
		 * empty elements in the array to check for.
		 */
	}

	return set;
}


/*
 * NAME: compare_msgs
 *                                                                    
 * FUNCTION: Compare function used by bsearch() in __cat_get_msgptr().
 *                                                                    
 * ARGUMENTS:
 *      key             - pointer to message number we're searching for
 *      element         - pointer to current _msgptr structure
 *
 * RETURNS: Returns -1, 0, or 1, depending on whether the message
 *          number is less than, equal to, or greater than the message
 *          number of the _msgptr structure.
 *
 */  

static int
compare_msgs(const void *key, const void *element)
{
	int *msgno = (int *) key;
	struct _msgptr *msg = (struct _msgptr *) element;

	if (*msgno < msg->_msgno)
		return -1;
	if (*msgno > msg->_msgno)
		return  1;

	return 0;
}


/*
 * NAME: __cat_get_msgptr
 *                                                                    
 * FUNCTION: Find a message in a set's set->_mp array.  Assumes that
 *           the messages in the array are sorted by increasing
 *           message number.
 *                                                                    
 * ARGUMENTS:
 *      set             - ptr to _catset structure
 *      msgno           - message catalogue message number
 *
 * RETURNS: Returns a pointer to the message on success.
 *          On any error, returns NULL.
 *
 */  

struct _msgptr *
__cat_get_msgptr(struct _catset *set, int msgno)
{
	struct _msgptr *msg;

	if (set == (struct _catset *) NULL)
		return (struct _msgptr *) NULL;

	if (set->_mp == (struct _msgptr *) NULL) /* empty set */
		return (struct _msgptr *) NULL;

	if (set->_msgs_expanded) {
		if ((msgno < 1) || (msgno > set->_n_msgs))
			return (struct _msgptr *) NULL;

		msg = &set->_mp[msgno];

		/*
		 * Catch empty elements in the array.  They aren't
		 * real messages.
		 */

		if (!msg->_offset)
			return (struct _msgptr *) NULL;
	}
	else {
		msg = (struct _msgptr *) bsearch((void *) &msgno,
						 set->_mp, set->_n_msgs + 1,
						 sizeof(struct _msgptr),
						 compare_msgs);

		/*
		 * Since the messages are compacted, there aren't any
		 * empty elements in the array to check for.
		 */
	}

	return msg;
}
