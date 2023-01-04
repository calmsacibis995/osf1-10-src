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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: dspcat.c,v $ $Revision: 4.3.11.5 $ (DEC) $Date: 1993/10/11 17:26:14 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: (CMDMSG) Message Catalogue Facilities
 *
 * FUNCTIONS: Main, make_msg, unpack
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.18  com/cmd/msg/dspcat.c, , bos320, 9134320 8/13/91 09:58:49
 */

 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "catio.h"
#include "msgfac_msg.h" 

void	make_msg(), 	/*---- prints ._msg format messages ----*/
	unpack();	/*--- reformats string into ._msg (gencat) format ----*/

/*
 * Get the index of the first valid set in a catalog's set array.  The
 * array's first element may have set number 0, which is not a valid
 * set number.  There are two ways this could happen:
 *
 *     1.  The message catalog was created with the 1.0 gencat.  That
 *         version of gencat allowed set number 0.
 *     2.  The set array is expanded.  In that case, element 0 is not
 *         used. 
 *
 * Similarly, get the index of the first valid message in a set's
 * message array.
 */

#define FIRST_SET(catd)         (((catd)->_set[0]._setno == 0) ? 1 : 0)
#define FIRST_MSG(setptr)       ((((setptr)->_mp == NULL) || \
				  ((setptr)->_mp[0]._msgno == 0)) ? 1 : 0)

/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 	standard library functions
 */


/*
 * NAME: main
 *                                                                    
 * FUNCTION: Decides what it has been asked to do (from the input arguments)
 *           and does it. (It either prints out a single message, a set, or the
 * 	     whole catalog.)
 *
 * EXECUTION ENVIRONMENT:
 *    	User mode.
 *                                                                    
 * RETURNS:   exit's(0);
 *
 */  

void
main(int argc, char *argv[]) 

{
	int 	set = ERR,	/*---- set to be printed ----*/
		msg = ERR,	/*---- msg to be printed ----*/
		msgout = FALSE;	/*---- TRUE for ._msg style output format ----*/
	int 	i,
		j;
	int	c;
	nl_catd catd;
	nl_catd catderr;	/* catalog descriptor for error messages */
	char	*catfile;
	struct _catset *setptr = NULL;
	char	*cp;

	setlocale (LC_ALL,"");
	catderr = catopen(MF_MSGFAC, NL_CAT_LOCALE);

	while ( (c = getopt(argc, argv, "g")) != EOF) {
	    switch(c) {
	      case 'g':
		msgout = TRUE;
		break;

	      default:
		die( catgets(catderr, MS_DSPCAT, M_USAGE, "Usage:  dspcat [-g] catname [set#] [msg#]") );
	    }
	}

	argc -= optind;		/* Skip past flags */
	argv += optind;

	if (argc == 1)		/* Only a catalog name */
		;
	else if (argc == 2) {	/* catname set# */ 
		set = strtoul(argv[1], &cp, 10);
		if (cp == NULL || *cp != '\0')
			die(catgets(catderr, MS_DSPCAT, M_ILLEGALSET,
				"dspcat: Invalid set number.\n") );
	} else if (argc == 3) {	/* catname set# msg# */
		if (msgout)
			die( catgets(catderr, MS_DSPCAT, M_NO_ID, "No message id allowed with -g option.") );
		set = strtoul(argv[1], &cp, 10);
		if (cp == NULL || *cp != '\0')
			die(catgets(catderr, MS_DSPCAT, M_ILLEGALSET,
				"dspcat: Invalid set number.\n") );
		msg = strtoul(argv[2], &cp, 10);
		if (cp == NULL || *cp != '\0')
			die(catgets(catderr, MS_DSPCAT, M_ILLEGALMSG,
				"dspcat: Invalid msg number.\n") );
	}
	else {	/*---- Too many or too few arguments ----*/
		die( catgets(catderr, MS_DSPCAT, M_USAGE, "Usage:  dspcat [-g] catname [set#] [msg#]") );
	}

	/*--- Force message catalogue to be opened by getting any message ----*/

	catfile = argv[0];
	
	catd = catopen(catfile, NL_CAT_LOCALE);
	catgets(catd, 0, 0, "");
	if (catd == CATD_ERR || catd->_fd == FILE_UNUSED) {
                fprintf(stderr, catgets(catderr, MS_DSPCAT, M_CAT_NO_OPEN,
                        "Unable to open specified catalog (%s)\n") ,catfile);
                exit(1);
	}
	if (argc > 1) {
		if (set > catd->_setmax) {
			fprintf(stderr, catgets(catderr, MS_DSPCAT, M_BADSET,
	 "dspcat: Invalid set - catalog only has %d set(s).\n"), catd->_setmax);
			exit(1);
		}
		
		setptr = __cat_get_catset(catd, set);
		if (setptr == NULL) {
			fprintf(stderr,
	 catgets(catderr, MS_DSPCAT, M_MISSINGSET,
	 "dspcat: Invalid set - set %d not found.\n"), set);
			exit(1);
		}
	}

	if (msgout) {	/*---- ._msg style output format ----*/
		make_msg(catd,setptr);
		exit(0);
	}

/*______________________________________________________________________
	Standard dspcat style output
  ______________________________________________________________________*/

	if (argc == 3) {	/*---- both set and message specified ----*/
		cp = catgets(catd,set,msg,"");
		if (*cp == '\0') {
			fprintf(stderr,
			  catgets(catderr, MS_DSPCAT, M_MISSINGMSG,
			  "dspcat: Invalid message - message %d not found.\n"),
			  msg);
			exit(1);
		}
		fputs(cp, stdout);
	}
	else if (argc == 2) {	/*---- just the set ----*/
		for (i = FIRST_MSG(setptr) ; i <= setptr->_n_msgs ; i++) {
			if (setptr->_mp[i]._offset) {
				msg = setptr->_mp[i]._msgno;
				printf("%s\n",catgets(catd,set,msg,""));
			}
		}
	}
	else if (argc == 1) {	/*---- print the whole catalog ----*/
		for (j = FIRST_SET(catd) ; j <= catd->_n_sets ; j++) {
			setptr = &catd->_set[j];
			for (i = FIRST_MSG(setptr) ; i <= setptr->_n_msgs ; i++) {
				if (setptr->_mp[i]._offset) {
					msg = setptr->_mp[i]._msgno;
					printf("%d : %d %s\n",setptr->_setno,msg,
                                               catgets(catd,setptr->_setno,msg,""));
				}
			}
		}
	}
	exit(0);
}




/*
 * NAME: make_msg
 *
 * FUNCTION: 	Makes message and prints its output with a ._msg style format 
 *		(i.e. suitable for input to gencat).
 *
 * EXECUTION ENVIRONMENT:
 *    	User mode.
 *                                                                    
 * RETURNS: 	void
 */

void make_msg(catd, setptr) 
nl_catd catd;
struct _catset *setptr;

	/*---- catd: catalog descriptor ----*/
	/*---- setptr: ptr to set in set array (NULL for all sets) ----*/

{
	int 	i,	/*---- Misc counter(s) used for loops ----*/
		j,	/*---- Misc counter(s) used for loops ----*/
		setmin,	/*---- minimum set to be printed ----*/
		setmax, /*---- maximum set to be printed ----*/
		set,	/*---- current set number ----*/
		msg;	/*---- current message number ----*/

	char 	buffer[NL_TEXTMAX * 2];
  			/*---- buffer to store unpacked message in  ----*/

	if (setptr == NULL) {
		setmin = FIRST_SET(catd);
		setmax = catd->_n_sets;
	}
	else 
		setmin = setmax = setptr - catd->_set;

	for (j = setmin ; j <= setmax ; j++) {

		set = catd->_set[j]._setno;
		if (set == 0)
		    continue;	      /* skip unused elements in _set array */

		printf("\n$delset %d\n",set);	/* header info for each set */
		printf("$set %d\n",set);
		printf("$quote \"\n\n");

		for (i = FIRST_MSG(&catd->_set[j]); i <= catd->_set[j]._n_msgs; i++) {
			if (catd->_set[j]._mp[i]._offset) {
				msg = catd->_set[j]._mp[i]._msgno;
				unpack(buffer,catgets(catd,set,msg,""));
				printf("%d\t\"%s\"\n",msg,buffer);
			}
		}
	}
} 



/*
 * NAME: unpack
 *
 * FUNCTION: unpack a text string into a format suitable for gencat.   
 *
 * EXECUTION ENVIRONMENT:
 *    	User mode.
 *                                                                    
 * RETURNS: void
 */

void unpack (t, s) 
char *t; char *s;

	/*---- t: Target buffer ----*/
	/*---- s: Source buffer ----*/

{
	int	len;		/* # bytes in next character */
	int	mb_cur_max;	/* temp MB_CUR_MAX */
	wchar_t	wc;		/* process code of next character */

	mb_cur_max = MB_CUR_MAX;
	while (*s) {	/*---- Until end of string ----*/
		len = mbtowc(&wc, s, mb_cur_max);
		if (len < 1) {
			len = 1;
			wc = *s & 0xff;
		}
		switch(wc) {
		case '\b':
			*t++ = '\\';
			*t++ = 'b';
			break;
		case '\t':
			*t++ = '\\';
			*t++ = 't';
			break;
		case '\n':
			*t++ = '\\';
			*t++ = 'n';
			if (s[1] != '\0') {
				*t++ = '\\';
				*t++ = '\n';
			}
			break;
		case '\v':
			*t++ = '\\';
			*t++ = 'v';
			break;
		case '\f':
			*t++ = '\\';
			*t++ = 'f';
			break;
		case '\r':
			*t++ = '\\';
			*t++ = 'r';
			break;
		case '"':
			*t++ = '\\';
			*t++ = '"';
			break;
		case '\\':
			*t++ = '\\';
			*t++ = '\\';
			break;
		default:
			if (wc < 0x20)
				t += sprintf(t,"\\%3.3o", (int)wc);
			else
				do
					*t++ = *s++;
				while (--len > 0);
		}
		s += len;
	}
	*t = '\0';
}
