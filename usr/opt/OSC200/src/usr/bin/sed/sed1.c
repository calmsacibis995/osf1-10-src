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
static char rcsid[] = "@(#)$RCSfile: sed1.c,v $ $Revision: 4.3.9.2 $ (DEC) $Date: 1993/06/10 15:04:49 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDEDIT) sed1.c
 *
 * FUNCTIONS: execute, match, substitute, dosub, place, command, gline, 
 * arout and growbuff.
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1984, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.14  com/cmd/edit/sed1.c, cmdedit, bos320, 9130320 7/15/91 16:58:02";
 *
 */

#include "sed.h"
#include <fcntl.h>
#include <unistd.h>
#include <wchar.h>
#include <ctype.h>

/* Maximum no. of subexpressions allowed in an RE */
regmatch_t	pmatch[_REG_SUBEXP_MAX + 1];
char	*loc1;
union reptr     *abuf[ABUFSIZE+1];
union reptr **aptr;
char    ibuf[512];
char    *cbp;
char    *ebp;
int     dolflag;
int     sflag;
int     jflag;
int     delflag;
long    lnum;
char    *spend;
int     nflag;
long    tlno[NLINES];
int     f;
union reptr     *pending;

static char *cur_fname;
int exit_value;

static char *gline(char *);
static int match(regex_t *, int);
static void command(union reptr *);
static void arout(void);
static void dosub(char *);
static char *place(char *, char *, char *);
static int substitute(union reptr *);

/*
 *	For each line in the input file, carry out the sequence of
 *	sed commands as  stored in the ptrspace structure.
 */
void execute(char *file)
{
	register struct addr	*p1, *p2;
	register union reptr    *ipc;
	register char	*p3;
	int     c;
	char    *execp;

	cur_fname = "standard input" ;
	if (file) {
		if ((f = open(file, 0)) < 0) {
			fprintf(stderr, MSGSTR(OPENERR, "sed: Cannot find or open file %s.\n"), file);  /* MSG */
			exit_value = 2;
		}
		cur_fname = file;
	} else
		f = 0;

	ebp = ibuf;
	cbp = ibuf;

	if(pending) {
		ipc = pending;
		pending = 0;
		goto yes;
	}

	for(;;) {
		if((execp = gline(linebuf)) == badp) {
			close(f);
			return;
		}
		spend = execp;

		for(ipc = ptrspace; ipc->r1.command; ) {

			p1 = ipc->r1.ad1;
			p2 = ipc->r1.ad2;

			loc1 = linebuf;
			if(p1) {

				if(ipc->r1.inar) {
					if(p2->afl == STRA && *p2->ad.str == CEND) {
						p1 = 0;
					} else if(p2->afl == STRA && *p2->ad.str == CLNUM) {
						p1 = 0;
						c = p2->ad.str[1];
						if(lnum > tlno[c]) {
							ipc->r1.inar = 0;
							if(ipc->r1.negfl)
								goto yes;
							ipc++;
							continue;
						}
						if(lnum == tlno[c]) {
							ipc->r1.inar = 0;
						}
					} else if(p2->afl == REGA && match(p2->ad.re, 0) == 0) {
						ipc->r1.inar = 0;
					}
				} else if(p1->afl == STRA && *p1->ad.str == CEND) {
					if(!dolflag) {
						if(ipc->r1.negfl)
							goto yes;
						ipc++;
						continue;
					}

				} else if(p1->afl == STRA && *p1->ad.str == CLNUM) {
					c = p1->ad.str[1];
					if(lnum != tlno[c]) {
						if(ipc->r1.negfl)
							goto yes;
						ipc++;
						continue;
					}
					if(p2)
						ipc->r1.inar = 1;
				} else if(p1->afl == REGA && match(p1->ad.re, 0) == 0) {
					if(p2)
						ipc->r1.inar = 1;
				} else {
					if(ipc->r1.negfl)
						goto yes;
					ipc++;
					continue;
				}
				/*
				 * if range is invalid (ie 5,2) or 
				 * simple (ie 5,5),  we aren't in a range.
				 */
				if (p2 && (p2->afl == STRA) && (*p2->ad.str == CLNUM) && 
				    (lnum >= tlno[p2->ad.str[1]]))
					ipc->r1.inar = 0;
			}

			if(ipc->r1.negfl) {
				ipc++;
				continue;
			}
	yes:
			command(ipc);

			if(delflag)
				break;

			if(jflag) {
				jflag = 0;
				if((ipc = ipc->r2.lb1) == 0) {
					ipc = ptrspace;
					break;
				}
			} else
				ipc++;

		}
		if(!nflag && !delflag) {
			for(p3 = linebuf; p3 < spend; p3++)
				(void) putc(*p3, stdout);
			(void) putc('\n', stdout);
		}

		if(aptr > abuf) {
			arout();
		}

		delflag = 0;

	}
}

/*
 *	Match text in linebuf according to reg. expr. in exp
 *	returning offsets to the start and end of all matched
 *	substrings in the pmatch structure.
 */ 
static int match(regex_t *exp, int gf)
{	
	register char	*p;
	int	eflags;

	eflags = 0;
	if (gf) {	/* If not searching from beginning of line */
			/* If previously matched null string, increment loc1 */
		if (pmatch[0].rm_so == pmatch[0].rm_eo)
			loc1++;
		p = loc1;
		eflags |= REG_NOTBOL;
	} else
		p = linebuf; 

	return(regexec(exp, p, (exp->re_nsub + 1), pmatch, eflags));
}

/*
 *	Perform substitution in linebuf according to reg. expr. and
 *	replacement text stored in ipc structure.
 */
static int substitute(union reptr *ipc)
{
	int	scount, sdone;
	scount = sdone = 0;

	/*
 	 *	If re1 is in fact a line number address and not a r.e.
	 *	the replacement text should be inserted at the start of 
	 *	the current line. Hence set loc1 = loc2 = 0 and return(1).
	 */
	loc1 = linebuf;
	if(ipc->r1.re1 && ipc->r1.re1->afl == STRA)  {
		pmatch[0].rm_so = pmatch[0].rm_eo = 0;		
		sflag = 1;
		dosub(ipc->r1.rhs);
		return(1);
	}

	/*
	 *	If gfl is a positive integer n substitute for
	 *	the nth match in linebuf. If gfl is GLOBAL_SUB
	 *	substitute for every match on the line.
	 */
	while(match(ipc->r1.re1->ad.re, scount++) == 0) {
		if(ipc->r1.gfl == GLOBAL_SUB || ipc->r1.gfl == scount) {
			sdone++;
			dosub(ipc->r1.rhs);
			if (ipc->r1.gfl != GLOBAL_SUB && scount >= ipc->r1.gfl)
				break;
		} else {
			loc1 += pmatch[0].rm_eo;
		}
		if (!*loc1)
			break;
	}
	if (sdone)
		sflag = 1;
	return(sdone);
}

/*	
 *	Perform substitution of replacement text
 *	for the recognized (and matched) regular expresson.
 */
static void dosub(char *rhsbuf)
{

	register char *lp, *rp, *sp;
	char *p;  /* p is needed to store temporary value of sp for 
			 calls to growspace */
	char  c;
	int	len;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < (loc1 + pmatch[0].rm_so))
		*sp++ = *lp++;
	while(c = *rp) {
		while (sp >= gend) {
			p = sp;
			growbuff(&gsize, &genbuf, &gend, &p);
			sp = p;
		}
		if (c == '&') {
			rp++;
			sp = place(sp, (loc1 + pmatch[0].rm_so), (loc1 + pmatch[0].rm_eo));
			continue;
		} else if (c == '\\') {
			c = *++rp;		/* discard '\\'	*/
			if (c >= '1' && c < _REG_SUBEXP_MAX +'1') {
				rp++;
				sp = place(sp, (loc1 + pmatch[c-'0'].rm_so), (loc1 + pmatch[c-'0'].rm_eo) );
				continue;
			}
		}
		if ((len = mblen(rp, MB_CUR_MAX)) < 1) {
			fprintf(stderr, MSGSTR(CGMSG, CGMES), linebuf);	/* MSG */
			exiting(2);
		}
		while ((sp+len) >= gend) {
			p = sp;
			growbuff(&gsize, &genbuf, &gend, &p);
			sp = p;
		}
		while (len--) 
			*sp++ = *rp++;
	}
	lp = loc1 + pmatch[0].rm_eo;
	/* Set loc1 to end of replacement text */
	loc1 = sp - genbuf + linebuf;
	/* Copy the rest of the linebuf to genbuf */
	while (*sp++ = *lp++)
		while (sp >= gend) {
			p = sp;
			growbuff(&gsize, &genbuf, &gend, &p);
			sp = p;
		}
	/* Copy genbuf back into linebuf */
	while (lsize < gsize)
		growbuff(&lsize, &linebuf, &lbend, &loc1);
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
	spend = lp-1;
}

/*
 *	Copy the text between al1 and al2 into asp.
 *	Used to copy replacement text into buffer during 
 *	substitute commands.
 *	N.B. The growbuff here is safe since the new asp is
 *	returned as a result in every usage of this function
 *	and hence if the buffer moves, everything will move with it.
 */
static char    *place(char *asp, char *al1, char *al2)
{
	register char *l1, *l2, *sp;
	char *p;

	sp = asp;
	l1 = al1;
	l2 = al2;
	while (l1 < l2) {
		*sp++ = *l1++;
		while (sp >= gend) {
			p = sp;
			growbuff(&gsize, &genbuf, &gend, &p);
			sp = p;
		}
	}
	return(sp);
}

/*
 *	Process commands stored in ipc structure.
 */
static void command(union reptr *ipc)
{
	register int    i;
	register char   *p1, *p2;
	char    *execp;

	int col;

	switch(ipc->r1.command) {

		case ACOM:
			*aptr++ = ipc;
			if(aptr >= &abuf[ABUFSIZE]) {
				fprintf(stderr, MSGSTR( APPNDCNT, "sed: The file was not appended after line %ld.\n"), lnum);  /* MSG */
				aptr--;
			}
			*aptr = 0;
			break;

		case CCOM:
			delflag = 1;
			if(!ipc->r1.inar || dolflag) {
				for(p1 = ipc->r1.rhs; *p1; )
					(void) putc(*p1++, stdout);
				(void) putc('\n', stdout);
			}
			break;
		case DCOM:
			delflag++;
			break;
		case CDCOM:
			p1 = p2 = linebuf;

			while(*p1 != '\n') {
				if(*p1++ == '\0') {
					delflag++;
					return;
				}
			}

			p1++;
			while(*p2++ = *p1++);
			spend = p2-1;
			jflag++;
			break;

		case EQCOM:
			fprintf(stdout, "%ld\n", lnum);
			break;

		case GCOM:
			/* Copy the hold space into the pattern space */
			while (lsize < hsize)
				growbuff(&lsize, &linebuf, &lbend, (char **)0);
			p1 = linebuf;
			p2 = holdsp;
			while(*p1++ = *p2++);
			spend = p1-1;
			break;

		case CGCOM:
			/* Append the hold space into the pattern space */
			if (spend > linebuf) {
				*spend++ = '\n';
			}
			if (spend + (hspend - holdsp) > lbend) 
				growbuff(&lsize, &linebuf, &lbend, (char **)0);
			p1 = spend;
			p2 = holdsp;
			while(*p1++ = *p2++);
			spend = p1-1;
			break;

		case HCOM:
			/* Copy the pattern space into the hold space */
			while (hsize < lsize)
				growbuff(&hsize, &holdsp, &hend, (char **)0);
			p1 = holdsp;
			p2 = linebuf;
			while(*p1++ = *p2++);
			hspend = p1-1;
			break;

		case CHCOM:
			/* Append the pattern space into the hold space */
			if (hspend > holdsp) {
				*hspend++ = '\n';
			}
			while (hspend + (spend - linebuf) > hend) 
				growbuff(&hsize, &holdsp, &hend, &hspend);
			p1 = hspend;
			p2 = linebuf;
			while(*p1++ = *p2++);
			hspend = p1-1;
			break;

		case ICOM:
			for(p1 = ipc->r1.rhs; *p1; )
				(void) putc(*p1++, stdout);
			(void) putc('\n', stdout);
			break;

		case BCOM:
			jflag = 1;
			break;


		case LCOM:
			{
			int	chrwid, len;
			wchar_t c1;
			unsigned char	c2;

			p2 = genbuf;
			col = 0;
			p1 = linebuf;
			while (*p1) {
				if ((len = mbtowc(&c1, p1, MB_CUR_MAX)) == -1 || len == 0) {
					len = 1;
				} 
				if (len == 1) {
					/* display single byte characters */
					/* or illegal characters */
					c2 = *(unsigned char *)p1++;
					if( 1 ) { /* XPG/4 Compliance - Omega*/
						chrwid = (!isprint(c2) ? 4 : wcwidth(c1));
						if (col+chrwid >= 72) {
							*p2 = '\0';
							fprintf(stdout, "%s\\\n", genbuf);
							p2 = genbuf;
							col = 0;
						}
						col += chrwid;
						if (c2 == '\a') {
							*p2++ = '\\';
							*p2++ = 'a';
							col -= 2;
						} else if (c2 == '\b') {
							*p2++ = '\\';
							*p2++ = 'b';
							col -= 2;
						} else if (c2 == '\f') {
							*p2++ = '\\';
							*p2++ = 'f';
							col -= 2;
						} else if (c2 == '\r') {
							*p2++ = '\\';
							*p2++ = 'r';
							col -= 2;
						} else if (c2 == '\t') {
							*p2++ = '\\';
							*p2++ = 't';
							col -= 2;
						} else if (c2 == '\v') {
							*p2++ = '\\';
							*p2++ = 'v';
							col -= 2;
						} else if (c2 == '\n') {
							*p2++ = '\\';
							*p2++ = 'n';
							col -= 2;
						} else if (c2 == '\\') {
							*p2++ = '\\';
							*p2++ = '\\';
							col -= 2;
						} else if (!isprint(c2)) {
							sprintf(p2, "\\%03o", c2);
							p2 += 4;
						} else
							*p2++ = c2;
					}
				} else {
					/* display multi-byte characters */
					chrwid = (!iswprint(c1) ? 4*len : wcwidth(c1));
					if (col+chrwid >= 72) {
						col = 0;
						*p2++ = '\\';
						*p2++ = '\n';
					}
					col += chrwid;
					if (!iswprint(c1)) {
						while (len--) {
							sprintf(p2, "\\%03o", *(unsigned char *)p1++);
							p2 += 4;
						}
					} else {
						p1 += len;
						p2 += wctomb(p2, c1);
					}
				}
			} /* end while */
			} /* end case LCOM */
			*p2 = '\0';
			fprintf(stdout, "%s$\n", genbuf);
			break;

		case NCOM:
			if(!nflag) {
				for(p1 = linebuf; p1 < spend; p1++)
					(void) putc(*p1, stdout);
				(void) putc('\n', stdout);
			}

			if(aptr > abuf)
				arout();
			if((execp = gline(linebuf)) == badp) {
				pending = ipc;
				delflag = 1;
				break;
			}
			spend = execp;

			break;
		case CNCOM:
			if(aptr > abuf)
				arout();
			*spend++ = '\n';
			if((execp = gline(spend)) == badp) {
				pending = ipc;
				delflag = 1;
				break;
			}
			spend = execp;
			break;

		case PCOM:
			for(p1 = linebuf; p1 < spend; p1++)
				(void) putc(*p1, stdout);
			(void) putc('\n', stdout);
			break;
		case CPCOM:
	cpcom:
			for(p1 = linebuf; *p1 != '\n' && *p1 != '\0'; )
				(void) putc(*p1++, stdout);
			(void) putc('\n', stdout);
			break;

		case QCOM:
			if(!nflag) {
				for(p1 = linebuf; p1 < spend; p1++)
					(void) putc(*p1, stdout);
				(void) putc('\n', stdout);
			}
			if(aptr > abuf) arout();
			fclose(stdout);
			exiting(0);
		case RCOM:

			*aptr++ = ipc;
			if(aptr >= &abuf[ABUFSIZE]) {
				fprintf(stderr, MSGSTR( READCNT, "sed: There are too many reads after line %ld.\n"),  /* MSG */
					lnum);
				aptr--;
			}
			*aptr = 0;
			break;

		case SCOM:
			i = substitute(ipc);
			if(ipc->r1.pfl && i) {
				if(ipc->r1.pfl == 1) {
					for(p1 = linebuf; p1 < spend; p1++)
						(void) putc(*p1, stdout);
					(void) putc('\n', stdout);
				} else
					goto cpcom;
			}
			if(i && ipc->r1.fcode)
				goto wcom;
			break;

		case TCOM:
			if(sflag == 0)  break;
			sflag = 0;
			jflag = 1;
			break;

		wcom:
		case WCOM:
			fprintf(ipc->r1.fcode, "%s\n", linebuf);
			break;
		case XCOM:
			/* Exchange contents of the pattern and hold spaces */
			while (gsize < lsize)
				growbuff(&gsize, &genbuf, &gend, (char **)0);
			p1 = linebuf;
			p2 = genbuf;
			while(*p2++ = *p1++);
			while (lsize < hsize)
				growbuff(&lsize, &linebuf, &lbend, (char **)0);
			p1 = holdsp;
			p2 = linebuf;
			while(*p2++ = *p1++);
			spend = p2 - 1;
			while (hsize < gsize)
				growbuff(&hsize, &holdsp, &hend, (char **)0);
			p1 = genbuf;
			p2 = holdsp;
			while(*p2++ = *p1++);
			hspend = p2 - 1;
			break;

		case YCOM:
			p1 = linebuf;

			{
			register wchar_t *yp;
			register char	*p3;
			char *p;
			wchar_t c, tc;

			p3 = genbuf;
			do {
				p1 += mbtowc(&c, p1, MB_CUR_MAX);
				yp = ipc->r1.ytxt;
				/* Find replacement in yp for character c */
				tc = 0;
				while (*yp) {
					if (c == *yp) {
						tc = *++yp;
						break;
					} 
					yp += 2;
				}
				if (*yp == '\0')	/* replacement not found */
					tc = c;
				p3 += wctomb(p3, tc);
				while (p3 >= gend) {
					p = p3;
					growbuff(&gsize, &genbuf, &gend, &p);
					p3 = p;
				}
			} while (*p1);

			while (lsize < gsize)
				growbuff(&lsize, &linebuf, &lbend, (char **)0);
			*p3 = *p1;	/* Copy the NULL */
			p3=genbuf;
			p1=linebuf;
			while(*p1++ = *p3++);
			spend = p1-1;
			}
			break;
	}

}

/*
 *	Read a line from input file.
 */
static char    *gline(char *addr)
{
	register char   *p1, *p2;
	char	*p;
	register        c;

	p1 = addr;
	p2 = cbp;
	for (;;) {
		if (p2 >= ebp) {
			if ((c = read(f, ibuf, 512)) <= 0) {
			/* Don't drop lines w/ missing newline */
				if( p2 == cbp )
					return(badp);
				else
				{
					fprintf(stderr, MSGSTR(NONL, "sed: Missing newline at end of file %s.\n"), cur_fname);  /* MSG */
					exit_value = 2;
					close(f);
					if(eargc == 0)
						dolflag = 1;
					p2 = ibuf;
					ebp = ibuf + c;
					break;
				}
			}
			p2 = ibuf;
			ebp = ibuf+c;
		}
		if ((c = *p2++) == '\n') {
			if(p2 >=  ebp) {
				if((c = read(f, ibuf, 512)) <= 0) {
					close(f);
					if(eargc == 0)
							dolflag = 1;
				}

				p2 = ibuf;
				ebp = ibuf + c;
			}
			break;
		}
		if(c) {
			while (p1 >= lbend) {
				p = p1;
				growbuff(&lsize, &linebuf, &lbend, &p);
				p1 = p;
			}
			*p1++ = c;
		}
	}
	lnum++;
	sflag = 0;
	while (p1 >= lbend) {
		p = p1;
		growbuff(&lsize, &linebuf, &lbend, &p);
		p1 = p;
	}
	*p1 = 0;
	cbp = p2;

	return(p1);
}

static void arout(void)
{
	register char   *p1;
	FILE    *fi;
	char    c;
	int     t;

	aptr = abuf - 1;
	while(*++aptr) {
		if((*aptr)->r1.command == ACOM) {
			for(p1 = (*aptr)->r1.rhs; *p1; )
				(void) putc(*p1++, stdout);
			(void) putc('\n', stdout);
		} else {
			if((fi = fopen((*aptr)->r1.rhs, "r")) == NULL)
				continue;
			while((t = getc(fi)) != EOF) {
				c = t;
				(void) putc(c, stdout);
			}
			fclose(fi);
		}
	}
	aptr = abuf;
	*aptr = 0;
}

/*
 *	Dynamic memory allocation for buffer storage.
 *	lenp	- current and new length 
 *	startp - current and new start of buffer 
 *	endp - current and new end of buffer 
 *	intop - pointer into current => new 
 */
void growbuff(int *lenp, char **startp, char **endp, char **intop)
{
	char *new;

	if (!*lenp)
		*lenp = LBSIZE;
	else
		*lenp <<= 1;
	if (!(new = *startp ? realloc(*startp, *lenp) : malloc(*lenp))) {
		fprintf(stderr, MSGSTR(MALLOCMSG, MALLOCMES));
		exiting(2);
	}
	if (intop)
		*intop = new + (*intop - *startp);
	*startp = new;
	*endp = new + *lenp;	
}


