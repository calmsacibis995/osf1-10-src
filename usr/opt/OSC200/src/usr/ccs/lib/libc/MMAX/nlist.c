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
static char	*sccsid = "@(#)$RCSfile: nlist.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:04:42 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* @(#)nlist.c	3.1 */
/*  */
/* 2/26/91 */
/*	3.0 SID #	1.4	*/
/*LINTLIBRARY*/

/* The following file contains the definition for ISMAGIC */
#include <sgs.h>

#if u3b || vax || n16 || m68 || u3b5
#define BADMAG(x)	(!ISMAGIC(x))
#endif


/*
 *	When a UNIX aout header is to be built in the optional header,
 *	the following magic numbers can appear in that header:
 *
 *		AOUT1MAGIC : default : readonly sharable text segment
 *		AOUT2MAGIC :	     : writable text segment
 *		PAGEMAGIC  :	     : directly paged object file
 */

#define AOUT1MAGIC 0410
#define AOUT2MAGIC 0407
#define PAGEMAGIC  0413

#define	SGSNAME	""
#define SGS ""
#define RELEASE "Release 6.0 6/1/82"
#include <a.out.h>

#if vax || n16 || u3b || m68 || u3b5
#	ifndef FLEXNAMES
#		define FLEXNAMES 1
#	endif
#	undef n_name		/* this patch causes problems here */
#endif

#if !(u3b || vax || n16 || m68 || u3b5)
#define SPACE 300		/* number of symbols read at a time */
#endif

extern long lseek();
extern int open(), read(), close(), strncmp(), strcmp();

int
nlist(name, list)
	const char *name;
	struct nlist *list;
{
#if u3b || vax || n16 || m68 || u3b5
	extern char *malloc();
	extern void free();
	struct	filehdr	buf;
	struct	syment	sym;
	union	auxent	aux;
	long n;
	int bufsiz=FILHSZ;
#if FLEXNAMES
	char *strtab = (char *)0;
	long strtablen;
#endif
	register struct nlist *p;
	register struct syment *q;
#else
	struct nlist space[SPACE];
	struct exec buf;
	int	nlen=sizeof(space[0].n_name), bufsiz=(sizeof(buf));
	unsigned n, m; 
	register struct nlist *p, *q;
#endif
	long	sa;
	int	fd;

	for (p = list; p->n_name && p->n_name[0]; p++)	/* n_name can be ptr */
	{
		p->n_type = 0;
		p->n_value = 0;
#if u3b || vax || n16 || m68 || u3b5
		p->n_value = 0L;
		p->n_scnum = 0;
		p->n_sclass = 0;
#endif
	}
	
	if ((fd = open(name, 0)) < 0)
		return(-1);
	(void) read(fd, (char *)&buf, bufsiz);
#if u3b || vax || n16 || m68 || u3b5
	if (BADMAG(buf.f_magic))
#else
	if (BADMAG(buf))
#endif
	{
		(void) close(fd);
		return (-1);
	}
#if u3b || vax || n16 || m68 || u3b5
	sa = buf.f_symptr;	/* direct pointer to sym tab */
	lseek(fd, sa, 0);
	q = &sym;
	n = buf.f_nsyms;	/* num. of sym tab entries */
#else
	sa = buf.a_text;
	sa += buf.a_data;
#if u370
	sa += (long)(buf.a_trsize + buf.a_drsize);
#endif
#if pdp11
	if (buf.a_flag != 1)
		sa += sa;
	else if ( buf.a_magic == A_MAGIC5 )
		sa += (long)buf.a_hitext << 16; /* remainder of text size for system overlay a.out */
#endif
	sa += (long)sizeof(buf);
	(void) lseek(fd, sa, 0);
	n = buf.a_syms;
#endif

	while (n)
	{
#if u3b || vax || n16 || m68 || u3b5
		read(fd, (char *)&sym, SYMESZ);
		n -= (q->n_numaux + 1L);
		/* read past aux ent , if there is one */
		if (q->n_numaux != 0)
			read(fd, (char *) &aux, AUXESZ);
#else
		m = (n < sizeof(space))? n: sizeof(space);
		(void) read(fd, (char*)space, m);
		n -= m;
		for (q=space; (int)(m -= sizeof(space[0])) >= 0; ++q)
		{
#endif
			for (p = list; p->n_name && p->n_name[0]; ++p)
			{
#if u3b || vax || n16 || m68 || u3b5
				/*
				* For 6.0, the name in an object file is
				* either stored in the eight long character
				* array, or in a string table at the end
				* of the object file.  If the name is in the
				* string table, the eight characters are
				* thought of as a pair of longs, (n_zeroes
				* and n_offset) the first of which is zero
				* and the second is the offset of the name
				* in the string table.
				*/
#if FLEXNAMES
				if (q->n_zeroes == 0L)	/* in string table */
				{
					if (strtab == (char *)0) /* need it */
					{
						long home = lseek(fd, 0L, 1);

						if (lseek(fd, buf.f_symptr +
							buf.f_nsyms * SYMESZ,
							0) == -1 || read(fd,
							(char *)&strtablen,
							sizeof(long)) !=
							sizeof(long) ||
							(strtab = malloc(
							(unsigned)strtablen))
							== (char *)0 ||
							read(fd, strtab +
							sizeof(long),
							strtablen -
							sizeof(long)) !=
							strtablen -
							sizeof(long) ||
							strtab[strtablen - 1]
							!= '\0' ||
							lseek(fd, home, 0) ==
							-1)
						{
							(void) lseek(fd,home,0);
							(void) close(fd);
							if (strtab != (char *)0)
								free(strtab);
							return (-1);
						}
					}
					if (q->n_offset < sizeof(long) ||
						q->n_offset >= strtablen)
					{
						(void) close(fd);
						if (strtab != (char *)0)
							free(strtab);
						return (-1);
					}
					if (strcmp(&strtab[q->n_offset],
						p->n_name))
					{
						continue;
					}
				}
				else
#endif /*FLEXNAMES*/
				{
					if (strncmp(q->_n._n_name,
						p->n_name, SYMNMLEN))
					{
						continue;
					}
				}
#else
				if (strncmp(p->n_name, q->n_name, nlen))
					continue;
#endif
#if u3b || vax || n16 || m68 || u3b5
				if (p->n_value != 0L)	/* got one already */
					break;
#endif
#if	CMU
				/*
				 *  The -g switch causes the compiler
				 *  to put all kinds of junk in the
				 *  symbol table; ignore it.  Anything
				 *  that's interested in it won't use
				 *  this routine to read it.
				 */
				switch (q->n_sclass) {
				    case C_EXT:
				    case C_STAT:
				    case C_FILE:
#endif	CMU
					p->n_value = q->n_value;
					p->n_type = q->n_type;
#if u3b || vax || n16 || m68 || u3b5
					p->n_scnum = q->n_scnum;
					p->n_sclass = q->n_sclass;
#endif
#if	CMU
					/*
					 *  Make sure n_type is
					 *  non-zero -- 127 is
					 *  unlikely to be confused
					 *  with anything useful.
					 */
					if (p->n_type == T_NULL)
					    p->n_type = 127;
					break;
				    default:
					break;
				}
#endif	CMU
				break;
			}
#if !(u3b || vax || n16 || m68 || u3b5)
		}
#endif
	}
	(void) close(fd);
#if (vax || n16 || m68 || u3b || u3b5) && FLEXNAMES
	if (strtab != (char *)0)
		free(strtab);
#endif
	return (0);
}
