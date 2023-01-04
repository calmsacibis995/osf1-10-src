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
static char *sccsid = "@(#)$RCSfile: fdtype.c,v $ $Revision: 4.3.10.2 $ (DEC) $Date: 1993/12/14 18:47:29 $";
#endif

/*
 * OSF/1 Release 1.0
 */
/* Derived from the work
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * File:	fdtype.c
 * Author:	Adrian Thoms (thoms@wessex)
 * Description:
 *	This is derived directly from filetype.c of the file(1)
 *	utility.
 *	It offers a library interface to the magic number part of the
 *	file(1) file guessing algorithm
 *
 * Modification History:
 *
 * 28-Sep-90 - Adrian Thoms (thoms@wessex)
 *	Fixed unaligned access problem (Ref. #001)
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#ifdef vax
#include <a.out.h>
#endif vax
#include "magic_data.h"
#include "filetype.h"
#include <locale.h>
#include "guesser.h"


/*
**	Structure of magic file entry
*/

Entry	*mtab;

int	ifile = -1;
int maxexecfn = 2;

/*
 * Forward References:
 */
int binary_mkmtab();
int ckmtab(struct stat *mbuf, int pflg, char *buf);

int
fdtype(fd, pmbuf, pcount, buf, bufsiz, pflg)
	int fd;
	struct stat *pmbuf;
	int *pcount;
	char *buf;
	unsigned bufsiz;
	int pflg;
{
	reg int i;
	reg int in=0;
	static struct stat dummy_mbuf;
	int retnum=0;
	unsigned char c; 

	if (pmbuf == NULL) pmbuf = &dummy_mbuf;
	(void) bzero (buf, bufsiz);
	in = read(fd, buf, bufsiz);
	*pcount = in;

	if(in == 0){
		if (pflg) printf("empty\n");
		return(MASH(EMPTY, UNKNOWN));
	}
        if (retnum = ckmtab(pmbuf, pflg, buf)) {
		return(retnum);
        }

	if (buf[0] == '\037' && (unsigned char)buf[1] == '\235') {
		if (buf[2]&0x80 && pflg)
			printf("block ");
		if (pflg) printf("compressed %d bit code data\n", buf[2]&0x1f);
		return(MASH(COMPRESSED, UNKNOWN));
	}

	if (strcmp((char *) setlocale (LC_CTYPE, NULL), "C") != 0)
		/*
		 * A non-default LC_TYPE locale was specified,
		 * use the Asian version of the test which
		 * deals with multi-byte characters.
		 */
		for(i = 0; i < in;) {
			reg len;
			if ((len = mblen(&buf[i], MB_MAX)) < 0) {
				if (in - i < MB_MAX)
					break;
				if (pflg)
					printf("data\n");
				return(MASH(DATA, UNKNOWN));
			}
			i += len;
		}
	else
		/*
		 * The default LC_CTYPE locale was selected which
		 * only supports 7-bit characters, therefore test
		 * the valid range of allowed 8-bit characters.
		 */
		for(i=0; i < in; i++) {
			c = buf[i] ;
			/*
			 * Since most people are printing text files, test
			 * for valid data first.  This gives a litte more
			 * performance.
			 */
			if (c < 0177)
				continue;
			else
				/*
				 * Test all non-valid multinational characters.
				 * Other characters such as null, xon/xoff,
				 * could be added, however this could cause
				 * backward compatibility problems.
				 */
				if (((c >= 0177) && (c <= 0203)) ||
				    ((c >= 0230) && (c <= 0232))) {
					if (pflg)
						 printf("data\n"); 

					return(MASH(DATA, UNKNOWN));
				}
		}
	return(0);
}


#ifdef HAVE_MAGIC
extern Entry_init magic_tab[];

binary_mkmtab()
{
	mtab = (Entry *)magic_tab;
}
#else
binary_mkmtab()
{
	return(0);
}
#endif

char *
execmodes(mbuf, fbuf)
struct stat *mbuf;
char *fbuf;
{
	static char msg_buf[26];

	sprintf(msg_buf, "%s%s%s",
		(mbuf->st_mode & S_ISUID) ? "setuid " : "",
		(mbuf->st_mode & S_ISGID) ? "setgid " : "",
		(mbuf->st_mode & S_ISVTX) ? "sticky " : "");
	return(msg_buf);
}

#ifdef vax
char *
symtable(mbuf, fbuf)
struct stat *mbuf;
char *fbuf;
{
	struct exec ex;
	struct stat stb;

	ex = *(struct exec *)fbuf;
	if (fstat(ifile, &stb) < 0)
		return("");

	if (((int *)fbuf[4])!= 0)
 	if ((int)N_STROFF(ex)+sizeof(off_t) > stb.st_size)
		return ("version 7 style symbol table");
	return ("");
}
#else !vax
char *
symtable(mbuf, fbuf)
struct stat *mbuf;
char *fbuf;
{
	return("");
}
#endif vax
char *(*execfns[])() = { symtable, symtable, execmodes };

int
ckmtab(mbuf, pflg, buf)
struct stat *mbuf;
int pflg;
char *buf;
{
	reg	Entry	*ep;
	reg	char	*p;
	reg	int	lev1 = 0;
	int	fn;
	int	retcode;

	auto	union	{
		int	l;
		char	ch[4];
		}	val, revval;

	struct matcher *match;
	char pbuf[256]; /* these fixed length arrays are bad. fix then sometime */
	char sbuf[256];
	char *sptr;
	int slen = 0;
	int match_len;

	for(ep = mtab; ep->e_off > -2 * maxexecfn; ep++) {
		if(lev1) {
			if(ep->e_level != 1)
				break;
			if (pflg && (slen > 0)) putchar(' ');
			slen = 0;
		} else if(ep->e_level == 1)
			continue;

		if (pflg && ep->e_off < 0) {
			fn = -1 * ep->e_off;
			if (fn > maxexecfn)
				sprintf(sbuf, ep->e_str, " ???? ");
			else
				sprintf(sbuf, ep->e_str, (char *)(*execfns[fn])(mbuf, buf));
			slen = strlen(sbuf);
			printf(sbuf);
			continue;
			}
			
		p = &buf[ep->e_off];
		switch(ep->e_type) {
		case STR:
		{
			match = (struct matcher *)fre_exec(p, ep->e_value.str);
#if defined(TESTING)
			if (match == (struct matcher *) 0) {
			    continue;
			} else if ((match == (struct matcher *) -1) && pflg) {
				fprintf(stderr,
				  "Regular expression syntax error for entry: '%s'\n", ep->e_str);
				continue;
			}
#else /* !defined(TESTING) */
			if ( (match == (struct matcher *) 0) || (match == (struct matcher *) -1) )
				continue;	/* No match, or reg expr error. */
#endif /* defined(TESTING) */
			match_len = match->finish - match->start;
			strncpy(pbuf, match->start, match_len);
			pbuf[match_len] = '\0';
			break;
		}

		case BYTE:
			val.l = (int)(*(unsigned char *) p);
			break;

		case SHORT:
			val.l = (int)(*(unsigned short *) p);
			break;

		case LONG:
			val.l = (int)(*(unsigned int *) p);
			break;
		}

		if (ep->e_type != STR)
		switch(ep->e_opcode & ~SUB) {
		case EQ:
			if(val.l != ep->e_value.num)
				continue;
			break;
		case GT:
			if(val.l <= ep->e_value.num)
				continue;
			break;

		case LT:
			if(val.l >= ep->e_value.num)
				continue;
			break;
		}
		if (pflg) {
			if(ep->e_opcode & SUB) { /* we really need the SysV printf here to return */
					         /* the number of characters printed.		*/
				if (ep->e_type == STR) {
				    sprintf (sbuf, ep->e_str, pbuf);
				} else {
				    sprintf (sbuf, ep->e_str, val.l);
				}
				sptr = sbuf;
			}
			else
				sptr=ep->e_str;
			slen = strlen(sptr);
			printf(sptr);
		}
		retcode = ep->e_retcode;
		lev1 = 1;
	}
	if(lev1) {
		if (pflg) putchar('\n');
		return(retcode);
	}
	return(0);
}
