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
static char rcsid[] = "@(#)$RCSfile: gio.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:05:49 $";
#endif
/* 
 * COMPONENT_NAME: UUCP gio.c
 * 
 * FUNCTIONS: grdblk, grddata, grdmsg, gturnoff, gturnon, gwrblk, 
 *            gwrdata, gwrmsg, pkfail 
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
gio.c	1.3  com/cmd/uucp,3.1,9013 10/10/89 13:40:38";
*/
/*	/sccs/src/cmd/uucp/s.gio.c
	gio.c	1.2	7/29/85 16:33:03
*/
#include "uucp.h"
/* VERSION( gio.c	5.2 -  -  ); */

#include "pk.h"

jmp_buf Gfailbuf;
struct pack *Pk;

pkfail()
{
	longjmp(Gfailbuf, 1);
}

gturnon()
{
	struct pack *pkopen();
	if (setjmp(Gfailbuf))
		return(FAIL);
	if (Debug > 4)
		pkdebug = 1;
	Pk = pkopen(Ifn, Ofn);
	if ( Pk == NULL)
		return(FAIL);
	return(0);
}


gturnoff()
{
	if(setjmp(Gfailbuf))
		return(FAIL);
	pkclose(Pk);
	return(0);
}


gwrmsg(type, str, fn)
char type, *str;
{
	char bufr[BUFSIZ], *s;
	int len, i;

	if(setjmp(Gfailbuf))
		return(FAIL);
	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	len = strlen(bufr) + 1;
	if ((i = len % PACKSIZE)) {
		len = len + PACKSIZE - i;
		bufr[len - 1] = '\0';
	}
	gwrblk(bufr, len, fn);
	return(0);
}


/*ARGSUSED*/
grdmsg(str, fn)
char *str;
{
	unsigned len;

	if(setjmp(Gfailbuf))
		return(FAIL);
	for (;;) {
		len = pkread(Pk, str, PACKSIZE);
		if (len == 0)
			continue;
		str += len;
		if (*(str - 1) == '\0')
			break;
	}
	return(0);
}


gwrdata(fp1, fn)
FILE *fp1;
{
	char bufr[BUFSIZ];
	int len;
	int ret;
	time_t ticks;
	long bytes;
	char text[BUFSIZ];

	if(setjmp(Gfailbuf))
		return(FAIL);
	bytes = 0L;
	(void) millitick();	/* set msec timer */
	while ((len = fread(bufr, sizeof (char), BUFSIZ, fp1)) > 0) {
		bytes += len;
		ret = gwrblk(bufr, len, fn);
		if (ret != len) {
			return(FAIL);
		}
		if (len != BUFSIZ)
			break;
	}
	ret = gwrblk(bufr, 0, fn);
	ticks = millitick();
	(void) sprintf(text, "-> %ld / %ld.%.3d secs", bytes, ticks / 1000,
		ticks % 1000);
	DEBUG(4, "%s\n", text);
	syslog(text);
	return(0);
}


grddata(fn, fp2)
FILE *fp2;
{
	int len;
	char bufr[BUFSIZ];
	time_t ticks;
	long bytes;
	char text[BUFSIZ];

	if(setjmp(Gfailbuf))
		return(FAIL);
	bytes = 0L;
	(void) millitick();	/* set msec timer */
	for (;;) {
		len = grdblk(bufr, BUFSIZ, fn);
		if (len < 0) {
			return(FAIL);
		}
		bytes += len;
		if (fwrite(bufr, sizeof (char), len, fp2) != len)
			return(FAIL);
		if (len < BUFSIZ)
			break;
	}
	ticks = millitick();
	(void) sprintf(text, "<- %ld / %ld.%.3d secs",
		bytes, ticks / 1000, ticks % 1000);
	DEBUG(4, "%s\n", text);
	syslog(text);
	return(0);
}


/*ARGSUSED*/
grdblk(blk, len,  fn)
char *blk;
{
	int i, ret;

	for (i = 0; i < len; i += ret) {
		ret = pkread(Pk, blk, len - i);
		if (ret < 0)
			return(FAIL);
		blk += ret;
		if (ret == 0)
			return(i);
	}
	return(i);
}


/*ARGSUSED*/
gwrblk(blk, len, fn)
char *blk;
{
	int ret;

	ret = pkwrite(Pk, blk, len);
	return(ret);
}
