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
static char rcsid[] = "@(#)$RCSfile: tio.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:08:16 $";
#endif
/* 
 * COMPONENT_NAME: UUCP tio.c
 * 
 * FUNCTIONS: min, trdblk, trddata, trdmsg, twrblk, twrdata, twrmsg 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
tio.c	1.6  com/cmd/uucp,3.1,9013 12/1/89 09:53:25";
*/

/*
"tio.c	4.6 (Berkeley) 1/24/86";
*/

#include <signal.h>
#include "uucp.h"
#include <setjmp.h>
#include <sys/stat.h>
#include <netinet/in.h>

extern int pkfail();
extern int maxmsg;		/* vary the wait time between packets */

#define TPACKSIZE	512
#define TBUFSIZE	1024
#define min(a,b)	(((a)<(b))?(a):(b))

/*
 *	htonl is a function that converts a long from host
 *		order to network order
 *	ntohl is a function that converts a long from network
 *		order to host order
 *
 *	network order is 		0 1 2 3 (bytes in a long)
 *	host order on a vax is		3 2 1 0
 *	host order on a pdp11 is	1 0 3 2
 *	host order on a 68000 is	0 1 2 3
 *	most other machines are		0 1 2 3
 */

struct tbuf {
	int t_nbytes;
	char t_data[TBUFSIZE];
};

static jmp_buf Failbuf;

twrmsg(type, str, fn)
char type;
register char *str;
{
	char bufr[TBUFSIZE];
	register char *s;
	int len, i;
	int send_len;

	if(setjmp(Failbuf))
		return FAIL;
	signal(SIGALRM, (void(*)(int)) pkfail);
	alarm(maxmsg);
	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	len = strlen(bufr) + 1;
	/* Next, len is forced to be an integer multiple of TPACKSIZE */
	if ((i = len % TPACKSIZE)) {
		send_len = len + TPACKSIZE - i;	/* Round up. */
		while (len < send_len)		/* Pad out the unused */
			bufr[len++] = '\0';	/* buffer with nulls. */
	}
	twrblk(bufr, len, fn);
	alarm(0);
	return SUCCESS;
}

trdmsg(str, fn)
register char *str;
{
	int len, cnt = 0;

	if(setjmp(Failbuf))
		return FAIL;
	signal(SIGALRM, (void(*)(int)) pkfail);
	alarm(maxmsg);
	for (;;) {
		len = read(fn, str, TPACKSIZE);
		if (len == 0)
			continue;
		if (len < 0) {
			alarm(0);
			return FAIL;
		}
		str += len;
		cnt += len;
		if (*(str - 1) == '\0' && (cnt % TPACKSIZE) == 0)
			break;
	}
	alarm(0);
	return SUCCESS;
}

twrdata(fp1, fn)
FILE *fp1;
{
	struct tbuf bufr;
	register int len;
	int ret, mil;
	time_t ticks;
	int bytes;
	char text[TBUFSIZE];

	if(setjmp(Failbuf))
		return FAIL;
	signal(SIGALRM, (void(*)(int)) pkfail);
	bytes = 0L;
	(void) millitick();
	while ((len = read(fileno(fp1), bufr.t_data, TBUFSIZE)) > 0) {
		bytes += len;
#if defined(ns32000)
		bufr.t_nbytes = htonl((long)len);
#else
		bufr.t_nbytes = len;
#endif
		DEBUG(8,"twrdata sending %d bytes\n",len);
		len += sizeof(int);
		alarm(maxmsg);
		ret = twrblk((char *)&bufr, len, fn);
		alarm(0);
		if (ret != len)
			return FAIL;
		if (len != TBUFSIZE+sizeof(int))
			break;
	}
	bufr.t_nbytes = 0;
	len = sizeof(int);
	alarm(maxmsg);
	ret = twrblk((char *)&bufr, len, fn);
	alarm(0);
	if (ret != len)
		return FAIL;
	ticks = millitick();
	sprintf(text, "-> %ld / %ld.%.3d secs", bytes, ticks / 1000, ticks % 1000);
	return SUCCESS;
}

trddata(fn, fp2)
FILE *fp2;
{
	register int len, nread;
	char bufr[TBUFSIZE];
	time_t ticks;
	int mil;
	int bytes, Nbytes;

	if(setjmp(Failbuf))
		return FAIL;
	signal(SIGALRM, (void(*)(int)) pkfail);
	(void) millitick();
	bytes = 0L;
	for (;;) {
		alarm(maxmsg);
		len = trdblk((char *)&Nbytes,sizeof Nbytes,fn);
		alarm(0);
		if (len != sizeof Nbytes)
			return FAIL;
#if defined(ns32000)
		Nbytes = ntohl(Nbytes);
#endif /*ns32000*/
		DEBUG(8,"trddata expecting %ld bytes\n", Nbytes);
		nread = Nbytes;
		if (nread == 0)
			break;
		alarm(maxmsg);
		len = trdblk(bufr, nread, fn);
		alarm(0);
		if (len < 0) {
			return FAIL;
		}
		bytes += len;
		DEBUG(11, "trddata got %ld\n",bytes);
		if (write(fileno(fp2), bufr, len) != len) {
			alarm(0);
			return FAIL;
		}
	}
	ticks = millitick();
	sprintf(bufr, MSGSTR(MSG_TIO5,
	  "received data %ld bytes %ld.%02d secs"), bytes, 
	  ticks / 1000, ticks % 1000);
	DEBUG(4, "%s\n", bufr);
	return SUCCESS;
}

#if !defined(BSD4_2) && !defined(USG)
#define	TC	1024
static	int tc = TC;
#endif

trdblk(blk, len,  fn)
register int len;
char *blk;
{
	register int i, ret;

#if !defined(BSD4_2) && !defined(USG)
	/* call ultouch occasionally */
	if (--tc < 0) {
		tc = TC;
		ultouch();
	}
#endif
	for (i = 0; i < len; i += ret) {
		ret = read(fn, blk, len - i);
		if (ret < 0)
			return FAIL;
		blk += ret;
		if (ret == 0)
			return i;
	}
	return i;
}


twrblk(blk, len, fn)
register char *blk;
{
#if !defined(BSD4_2) && !defined(USG)
	/* call ultouch occasionally */
	if (--tc < 0) {
		tc = TC;
		ultouch();
	}
#endif
	return write(fn, blk, len);
}
