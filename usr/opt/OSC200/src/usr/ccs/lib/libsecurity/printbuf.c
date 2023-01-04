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
static char *rcsid = "@(#)$RCSfile: printbuf.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:24:48 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	printbuf.c,v $
 * Revision 1.1.1.2  92/06/23  01:44:00  devrcs
 *  *** OSF1_1B30 version ***
 * 
 * Revision 1.6.2.2  1992/04/08  20:30:31  marquard
 * 	Added POSIX ACL support.
 * 	[1992/04/05  14:05:24  marquard]
 *
 * Revision 1.6  1991/03/04  17:44:58  devrcs
 * 	Comment out ident directives
 * 	[91/01/31  08:57:38  lehotsky]
 * 
 * Revision 1.5  91/01/07  15:59:22  devrcs
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:30:08  dwm]
 * 
 * Revision 1.4  90/10/07  20:08:18  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:16:35  gm]
 * 
 * Revision 1.3  90/07/17  12:20:53  devrcs
 * 	Internationalized
 * 	[90/07/05  07:28:33  staffan]
 * 
 * Revision 1.2  90/06/22  21:47:47  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:38:46  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved.
 *
 * routines that deal with column counts for printouts
 */

/* #ident "@(#)printbuf.c	2.1 16:18:09 4/20/90 SecureWare" */
/*
 * Based on:
 *   "@(#)printbuf.c	2.2.1.3 23:48:06 1/8/90 SecureWare"
 */

#include <sys/types.h>
#include <stdio.h>

static int num_cols;

extern char *getenv();
extern int strcspn();
extern int strspn();
#ifndef _OSF_SOURCE
extern int columns;	/* from curses */
#endif

static void
set_num_cols()
{
	char *cptr;
#ifndef _OSF_SOURCE
	int curses_ret;
#endif

	/* figure out how many columns */
	if ((cptr = getenv ("COLUMNS")) != (char *) 0)
		num_cols = atoi (cptr);
	else {
#ifndef _OSF_SOURCE
		setupterm ((char *) 0, 1, &curses_ret);
		if (curses_ret == 1) {
			num_cols = columns;
			resetterm ();
		}
		else
#endif
			num_cols = 80;
	}
}

/* print a character string on stdout, making sure we don't overflow the
 * column count.
 */

void
printbuf (buf, col, separators)
char *buf;
int  col;
char *separators;
{
	int	cur_col;	/* column counter */
	int	count;		/* number of characters fitting condition */
	char	*pbuf = buf;	/* first character to print next time */
	int	column ;

	if (num_cols == 0)
		set_num_cols();

	column = col ;
	count  = strcspn(buf, separators) ;
	if ((column + count + 1) >= num_cols && (num_cols / 3) < column - 10) {

		/*
		 * If the first "word" doesn't fit on the line given
		 * the specified indent and the specified indent is
		 * 10 or more characters greater than 1/3 of the total
		 * line length, we start on a new line with a smaller
		 * indent.
		 */

		putchar('\n') ;
		column = num_cols / 3 ;
		printf("%*s", column, "") ;
	}

	for (cur_col = column + 1; *buf != '\0'; ) {
		count = strcspn (buf, separators);	
		/* if the next field causes us to overflow, spit out what
		 * we've got so far.
		 */
		if (count + cur_col >= num_cols) {
			/* If we have a field that doesn't fit in the
			 * available space, just print what we can.
			 * Otherwise we would loop forever.
			 */
			if (pbuf == buf) {
				printf("%*s\\", num_cols - cur_col, pbuf);
				pbuf += num_cols - cur_col;
				buf = pbuf;
			} else {
				for (; pbuf < buf; pbuf++)
					putchar (*pbuf);
			}
			putchar ('\n');
			printf ("%*s", column, "");
			cur_col = column + 1;
			/* skip over leading whitespace on the next line */
			count = strspn (buf, " ");
			pbuf += count;
			buf += count;
		} else {
			cur_col += count;
			buf += count;
			if (*buf) {
				cur_col++;
				buf++;
			}
		}
	}
	if (*pbuf != '\0')
		printf ("%s\n", pbuf);
}

#if SEC_ACL_POSIX
int
pacl_printbuf (file, acl_er, uid, gid)
char    *file;
char    *acl_er;
uid_t   uid;
gid_t   gid;
{

        char            *owner, *group;

        (void) fprintf (stdout, "# file:%s\n",file);

        if ((owner = (char *)pw_idtoname(uid)) == (char *)NULL)
                (void) fprintf (stdout, "# owner:%d\n",uid);
        else
                (void) fprintf (stdout, "# owner:%s\n",owner);

        if ((group = (char *)gr_idtoname(gid)) == (char *)NULL)
                (void) fprintf (stdout, "# group:%d\n",gid);
        else
                (void) fprintf (stdout, "# group:%s\n",group);

       (void) fprintf (stdout, "%s", acl_er);

        return 0;

}
#endif				/* SEC_ACL_POSIX */
