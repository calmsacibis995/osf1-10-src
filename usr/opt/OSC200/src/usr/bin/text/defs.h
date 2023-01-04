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
/* @(#)$RCSfile: defs.h,v $ $Revision: 4.3 $ (DEC) $Date: 1991/09/21 17:11:49 $ */
/*
 */
/*
 *
 *   File name: defs.h
 *
 *   Source file description:
 *	A useful header file for the string extraction/merge tools
 *
 *   Modification history:
 *	Tom Woodburn, 16-Apr-1991
 *		Changed CATSUFFIX from .msf to .msg to be consistent
 *		with OSF/1 convention.  Then had to change MSGSUFFIX
 *		from .msg to something else -- chose .str.
 *
 *		Changed LIB_DIR from /usr/lib/intln to /usr/lib/nls and
 *		HELPFILE from /usr/lib/intln/help to /usr/lib/nls/help.
 *
 *		Changed copyright notice to DIGITAL_COPYRIGHT macro.
 *
 *	Andy Gadsby, 20-Jan-1987.
 *		Created.
 *
 */

/* some very useful standard definitions				*/

#define ERROR     (-1)		/* standard error return		*/
#ifdef OK
#undef OK
#endif /* OK */
#define OK	  (0)		/* standard success return		*/
#ifdef TRUE
#undef TRUE
#endif /* TRUE */
#define TRUE	  (1)
#ifdef FALSE
#undef FALSE
#endif /* FALSE */
#define FALSE     (0)	

#ifndef NULL
# define NULL 	  0
#endif

#define STREXTRACT 	"strextract"

#define IGNFILE	   	"ignore"
#define PATTERN_FILE 	"patterns"

#define LIB_DIR		"/usr/lib/nls/"
#define HELPFILE   	"/usr/lib/nls/help"

#define MSGSUFFIX ".str"	/* the message file suffix		*/
#define CATSUFFIX ".msg"	/* the catalogue file suffix		*/
#define INTPREFIX "nl_"		/* natural language prefix string	*/

#define LINESIZE  256		/* maximum size of a text line		*/
#define REWLEN   1024		/* maximum size of a rewrite string	*/

/* some general purpose function definitions				*/
char *fixsuffix();
char *fixprefix();

/* string ignore/duplicate matching definitions				*/

#define HASHMASK	63		/* a useful number		*/

struct element {			/* the information we save	*/
	struct element *next;		/* a flink			*/
	int    len;			/* the length of the string	*/
	long   linenum;			/* the first line number	*/
	long   msgnum;			/* message num in catalogue	*/
	int    flags;			/* useful flags			*/
	char   string[1];		/* start of string		*/
};

struct element *lookupstr();

					/* setting of the flags		*/
#define STR_IGNORE	1		/* ignore this string		*/
