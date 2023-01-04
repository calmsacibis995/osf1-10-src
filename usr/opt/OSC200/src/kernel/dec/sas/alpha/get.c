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
#ifdef lint
static	char	*sccsid = "@(#)get.c	9.2	(ULTRIX)	11/14/90";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: /sys/sas/alpha/get.c
 *
 * 28-Sep-90 -- Tim Burke
 *	Created this file for Alpha boot support.
 */

#include <sys/types.h>
#include <machine/rpb.h>
#include <machine/entrypt.h>

/*
 * Since the console callbacks do not have a putc routine there is a "library"
 * routine for printf which is called to do a putchar.
 */
#define putchar(Character)	printf("%c", Character)
#define CHAR_MASK	0177	/* Strip chars to 7 bits - gack		*/
#define INBUFSZ 	256	/* Assume the input buffer is no bigger */
#define CONTROL_S 	19	/* tty stop character			*/
#define CONTROL_Q 	17	/* tty start character			*/

extern struct rpb_crb *crb;	/* Pointer to console routine block	*/
extern struct rpb_ctb *ctb;	/* Pointer to console terminal block	*/
extern struct rpb *hwrpb_addr;  /* Pointer to HWRPB                     */
/*
 * The variable "delflg" will be set to 1 to indicate that a delete character
 * has been encountered.  This is used when echoing characters because things
 * are handled differently for deletes.
 */
static int delflg = 0;


/*
 * Fix ups remaining to do:
 *
 * 1)  This version does not assume that the console is a graphics device;
 *     rather it assumes a hard copy console and echoes deletes and so forth
 *     accordingly.  This may need some work to make it pretty.
 *
 * 2)  This version strips the character to be 7-bits.  While this is not the
 *     right thing to do it will get us by for now.  Not 8-bit clean.
 */

/*
 * Pull in a character from the console.
 */
getchar()
{
	register u_char c;

	c = prom_getc();
	c &= CHAR_MASK;
	/*
	 * If a stop character has been received throw away any additional
	 * input until a start character is seen.
	 */
	if (c == CONTROL_S) {
		while (c != CONTROL_Q) {
			c = prom_getc();
			c &= CHAR_MASK;
		}
		c = prom_getc();
		c &= CHAR_MASK;
	}
	if (c=='\r')
		c = '\n';
	/*
	 * Echo the character back out onto the console if it is not a delete.
	 */
	if (!delflg && c != '') {
		putchar(c);
	}
	return(c);
}

/*
 * Read in a string from the console.  This consists of repeated calls to
 * getchar with some tty like mappings.
 */
gets(buf)
	char *buf;
{
	register char *lp;
	register u_char c;
	register cc;			/* character count */

	cc=0;
	lp = buf;
	for (;;) {
		c = getchar();
		switch(c) {
		case '\n':			/* String terminators */
		case '\r':
			if (delflg) {
				putchar('/');
				putchar(c);
				delflg = 0;
			}
			c = '\n';
			*lp++ = '\0';
			return;
		case '':			/* Delete key */
			if (lp != buf) {
				if (!delflg) {
					delflg++;
					putchar('\\');
				}
			}
			if (lp > buf)
				putchar(*--lp);
			if (lp < buf)
				lp = buf;
			continue;
		/*
		 * Backspace and pound sign are left for backward
		 * compatability.  Strange results are guaranteed if
		 * either of these are mixed with <del> on the same
		 * input line.
		 */
		case '\b':
		case '#':
			delflg = 0;
			lp--;
			if (lp < buf)
				lp = buf;
			continue;
		case '@':
		case '':			/* Control U */
			delflg = 0;
			lp = buf;
			printf("^U\n");
			continue;
		case '':			/* Control R */
			*lp = '\0';
			if (delflg) {
				putchar('/');
			}
			delflg = 0;
			printf("^R\n%s", buf);
			continue;
		default:
			if (delflg) {
				putchar('/');
				putchar(c);
				delflg = 0;
			}
			*lp++ = c;
			cc++;
			if (cc == INBUFSZ) {
				printf("\nInput line too long\n");
				lp = buf;
				*lp = '\0';
				return;
			}
		}
	}
}
