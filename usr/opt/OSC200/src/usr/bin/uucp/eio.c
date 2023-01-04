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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: eio.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:04:57 $";
#endif
/* 
 * COMPONENT_NAME: UUCP eio.c
 * 
 * FUNCTIONS: MIN, ealarm, erdblk, erddata, erdmsg, eturnoff, 
 *            eturnon, ewrdata, ewrmsg 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
eio.c	1.5  com/cmd/uucp,3.1,9013 1/10/90 19:10:16";
*/
/*	/sccs/src/cmd/uucp/s.eio.c
	eio.c	1.4	7/29/85 16:32:54
*/

#include "uucp.h"
/*
VERSION( eio.c	1.1 - 87/06/30 - 19:02:30 );
*/

#ifdef ATTSV
#define     MIN(a,b) (((a)<(b))?(a):(b))
#endif

static jmp_buf Failbuf;

/*
 * error-free channel protocol
 */
static
ealarm() {
	longjmp(Failbuf, 1);
}
static void (*esig)();

/*
 * turn on protocol timer
 */
eturnon()
{
	esig=signal(SIGALRM, (void(*)(int))  ealarm);
	return(0);
}

eturnoff()
{
	signal(SIGALRM, esig);
	return(0);
}

/*
 * write message across link
 *	type	-> message type
 *	str	-> message body (ascii string)
 *	fn	-> link file descriptor
 * return
 *	FAIL	-> write failed
 *	0	-> write succeeded
 */
ewrmsg(type, str, fn)
register char *str;
int fn;
char type;
{
	register char *s;
	char bufr[BUFSIZ];
	int	s1, s2;

	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	s1 = strlen(bufr) + 1;
	if (setjmp(Failbuf)) {
		DEBUG(7, "ewrmsg write failed\n", "");
		return(FAIL);
	}
	alarm(60);
	s2 = write(fn, bufr, (unsigned) s1);
	alarm(0);
	if (s1 != s2)
		return(FAIL);
	return(0);
}

/*
 * read message from link
 *	str	-> message buffer
 *	fn	-> file descriptor
 * return
 *	FAIL	-> read timed out
 *	0	-> ok message in str
 */
erdmsg(str, fn)
register char *str;
{
	register int i;
	register int len;

	if(setjmp(Failbuf)) {
		DEBUG(7, "erdmsg read failed\n", "");
		return(FAIL);
	}

	i = BUFSIZ;
	for (;;) {
		alarm(60);
		if ((len = read(fn, str, i)) == 0)
			continue;	/* Perhaps should be FAIL, but the */
					/* timeout will get it (skip alarm(0) */
		alarm(0);
		if (len < 0) return(FAIL);
		str += len; i -= len;
		if (*(str - 1) == '\0')
			break;
	}
	return(0);
}

/*
 * read data from file fp1 and write
 * on link
 *	fp1	-> file descriptor
 *	fn	-> link descriptor
 * returns:
 *	FAIL	->failure in link
 *	0	-> ok
 */
ewrdata(fp1, fn)
int	fn;
register FILE *fp1;
{
	register int ret;
	int len;
	off_t bytes;
	char bufr[BUFSIZ];
	char text[BUFSIZ];
	time_t	ticks;
	int	mil;
	struct stat	statbuf;
	off_t	msglen;
	char	cmsglen[20];

	if (setjmp(Failbuf)) {
		DEBUG(7, "ewrdata failed\n", "");
		return(FAIL);
	}
	bytes = 0L;
	fstat(fileno(fp1), &statbuf);
	bytes = msglen = statbuf.st_size;
	(void) millitick();
	sprintf(cmsglen, "%ld", (int) msglen);
	alarm(60);
	ret = write(fn, cmsglen, sizeof(cmsglen));
	alarm(0);
	DEBUG(7, "ewrmsg ret %d\n", ret);
	if (ret != sizeof(cmsglen))
		return(FAIL);
	DEBUG(7, "ewrmsg write %d\n", statbuf.st_size);
	while ((len = fread(bufr, sizeof (char), BUFSIZ, fp1)) > 0) {
		alarm(60);
		ret = write(fn, bufr, (unsigned) len);
		alarm(0);
		DEBUG(7, "ewrmsg ret %d\n", ret);
		if (ret != len)
			return(FAIL);
		if ((msglen -= len) <= 0)
			break;
	}
	if (len < 0 || (len == 0 && msglen != 0)) return(FAIL);
	ticks = millitick();
	sprintf(text, "-> %ld / %ld.%.3d secs", bytes, ticks / 1000, ticks % 1000);
	DEBUG(4, "%s\n", text);
	syslog(text);
	return(0);
}

/*
 * read data from link and
 * write into file
 *	fp2	-> file descriptor
 *	fn	-> link descriptor
 * returns:
 *	0	-> ok
 *	FAIL	-> failure on link
 */
erddata(fn, fp2)
register FILE *fp2;
{
	register int len;
	int bytes;
	char text[BUFSIZ];
	char bufr[BUFSIZ];
	time_t	ticks;
	int	mil;
	int	msglen;
	char	cmsglen[20];

	bytes = 0L;
	(void) millitick();
	len = erdblk(cmsglen, sizeof(cmsglen), fn);
	if (len < 0) return(FAIL);
	sscanf(cmsglen, "%ld", &msglen);
	bytes = msglen;
	DEBUG(7, "erdblk msglen %d\n", msglen);
	for (;;) {
		len = erdblk(bufr, MIN(msglen, BUFSIZ), fn);
		DEBUG(7, "erdblk ret %d\n", len);
		if (len < 0) {
			DEBUG(7, "erdblk failed\n", "");
			return(FAIL);
		}
		if ((msglen -= len) < 0) {
			DEBUG(7, "erdblk read too much\n", "");
			return(FAIL);
		}
		fwrite(bufr, sizeof (char), len, fp2);
		if (msglen == 0)
			break;
	}
	ticks = millitick();
	sprintf(text, "<- %ld / %ld.%.3d secs", bytes, ticks / 1000, ticks % 1000);
	DEBUG(4, "%s\n", text);
	syslog(text);
	return(0);
}

/*
 * read block from link
 * reads are timed
 *	blk	-> address of buffer
 *	len	-> size to read
 *	fn	-> link descriptor
 * returns:
 *	FAIL	-> link error timeout on link
 *	i	-> # of bytes read
 */
erdblk(blk, len,  fn)
register char *blk;
{
	register int i, ret;

	if(setjmp(Failbuf)) {
		DEBUG(7, "erdblk timeout\n", "");
		return(FAIL);
	}

	for (i = 0; i < len; i += ret) {
		alarm(60);
		DEBUG(7, "ask %d ", len - i);
		if ((ret = read(fn, blk, (unsigned) len - i)) < 0) {
			alarm(0);
			DEBUG(7, "read failed\n", "");
			return(FAIL);
		}
		DEBUG(7, "got %d\n", ret);
		blk += ret;
		if (ret == 0)
			break;
	}
	alarm(0);
	return(i);
}
