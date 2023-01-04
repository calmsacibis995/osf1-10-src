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
static char	*sccsid = "@(#)$RCSfile: m4macs.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/08/31 15:34:22 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: arg, def, dochcom, docq, dodecr, dodef, dodefn,
 *	      dodiv, dodivnum, dodlen, dodnl, dodump, doerrp,
 *	      doeval, doexit, doif, doifdef, doincl, doincr,
 *	      doindex, dolen, domake, dopopdef, dopushdef,
 *	      doshift, dosincl, dosubstr, dosyscmd, dosysval,
 *	      dotransl, dotroff, dotron, doundef, doundiv,
 *	      dowrap, dump, incl, leftmatch, ncgetbuf, undef
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 */

#include	<stdio.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<stdlib.h>
#include	<sys/stat.h>
#include	"m4.h"

#define arg(n)	(c<(n)? nullstr: ap[n])



wchar_t* 
ncgetbuf(src)
char *src;
{
	register int dlen;
	register wchar_t *dest;

	/* Alloc one wchar_t for each of the bytes in the multibyte
	** string: src.  Plus one more wchar_t for the null wchar_t.
	*/
	dlen = ( (strlen(src)+1) * sizeof(wchar_t));
	dest = (wchar_t*) malloc( dlen);
	if(dest == NULL){
		error(OST,nocore);
	}
	if (mbstowcs(dest, src, dlen/sizeof(wchar_t)) <0) {
		error2(MBWC,MBtoWCerr,src);
	}
	return (dest);
}


dochcom(ap,c)
char	**ap;
{
	register char	*l = arg(1);
	register char	*r = arg(2);

	if (strlen(l)>MAXSYM || strlen(r)>MAXSYM)
		error2(CMLB,"comment marker longer than %d bytes",MAXSYM);
	strcpy(lcom,l);
	strcpy(rcom,*r?r:"\n");
}

docq(ap,c)
register char 	**ap;
{
	register char	*l = arg(1);
	register char	*r = arg(2);

	if (strlen(l)>MAXSYM || strlen(r)>MAXSYM)
		error2(QLNB,"quote marker longer than %d bytes", MAXSYM);

	if (c<=1 && !*l) {
		l = "`";
		r = "'";
	} else if (c==1) {
		r = l;
	}

	strcpy(lquote,l);
	strcpy(rquote,r);
}

dodecr(ap,c)
char 	**ap;
{
	pbnum(ctol(arg(1))-1);
}

dodef(ap,c)
char	**ap;
{
	def(ap,c,NOPUSH);
}

def(ap,c,mode)
register char 	**ap;
{
	register char	*s;

	if (c<1)
		return;

	s = ap[1];
	if (isalpha(*s)||*s=='_')
		while (isalnum(*++s)||*s=='_')
			;
	if (*s || s==ap[1])
		error(BMN,"bad macro name");

	if ((strcmp(ap[1],ap[2])==0)||(strcmp(ap[2],"$0")==0))
		error(MD,"macro defined as itself");
	install(ap[1],arg(2),mode);
}

dodefn(ap,c)
register char	**ap;
register c;
{
	register char *d;

	while (c > 0)
		if ((d = lookup(ap[c--])->def) != NULL) {
			pbstr(rquote);
			while (*d)
				putbak(*d++);
			pbstr(lquote);
		}
}

dodiv(ap,c)
register char **ap;
{
	register int f;

	f = atoi(arg(1));
	if (f>=10 || f<0) {
		cf = NULL;
		ofx = f;
		return;
	}
	tempfile[7] = 'a'+f;
	if (ofile[f] || (ofile[f]=xfopen(tempfile,"w"))) {
		ofx = f;
		cf = ofile[f];
	}
}

/* ARGSUSED */
dodivnum(ap,c)
{
	pbnum((long) ofx);
}

/* ARGSUSED */
dodnl(ap,c)
char 	*ap;
{
	register t;

	while ((t=getchr())!='\n' && t!=EOF)
		;
}

dodump(ap,c)
char 	**ap;
{
	register struct nlist *np;
	register	i;

	if (c > 0)
		while (c--) {
			if ((np = lookup(*++ap))->name != NULL)
				dump(np->name,np->def);
		}
	else
		for (i=0; i<hshsize; i++)
			for (np=hshtab[i]; np!=NULL; np=np->next)
				dump(np->name,np->def);
}

dump(name,defnn)
register char	*name,
		*defnn;
{
	register char	*s = defnn;

	fprintf(stderr,"%s:\t",name);

	while (*s++)
		;
	--s;

	while (s>defnn)
		if (*defnn == 0x7f) {
			fprintf(stderr,"<%s>",barray[*(defnn + 1)&LOW7].bname);
			s = defnn;
		} else {
			fputc(*--s,stderr);
		}

	fputc('\n',stderr);
}

doerrp(ap,c)
char 	**ap;
{
	if (c > 0)
		fprintf(stderr,"%s",ap[1]);
}

long	evalval;	/* return value from yacc stuff */
char	*pe;	/* used by grammar */
doeval(ap,c)
char 	**ap;
{
	register	base = atoi(arg(2));
	register	pad = atoi(arg(3));

	evalval = 0;
	if (c > 0) {
		pe = ap[1];
		if (yyparse()!=0)
			error(IE,"invalid expression");
	}
	pbnbr(evalval, base>0?base:10, pad>0?pad:1);
}

doexit(ap,c)
char	**ap;
{
	delexit(atoi(arg(1)));
}

doif(ap,c)
register char **ap;
{
	if (c < 3)
		return;
	while (c >= 3) {
		if (strcmp(ap[1],ap[2])==0) {
			pbstr(ap[3]);
			return;
		}
		c -= 3;
		ap += 3;
	}
	if (c > 0)
		pbstr(ap[1]);
}

doifdef(ap,c)
char 	**ap;
{
	if (c < 2)
		return;

	while (c >= 2) {
		if (lookup(ap[1])->name != NULL) {
			pbstr(ap[2]);
			return;
		}
		c -= 2;
		ap += 2;
	}

	if (c > 0)
		pbstr(ap[1]);
}

doincl(ap,c)
char	**ap;
{
	incl(ap,c,1);
}

incl(ap,c,noisy)
register char 	**ap;
{
	if (c>0 && strlen(ap[1])>0) {
		if (ifx >= 9)
			error(IFLN,"input file nesting too deep (9)");
		if ((ifile[++ifx]=fopen(ap[1],"r"))==NULL){
			--ifx;
			if (noisy)
				error(OFL,badfile);
		} else {
			ipstk[ifx] = ipflr = ip;
			setfname(ap[1]);
		}
	}
}

doincr(ap,c)
char 	**ap;
{
	pbnum(ctol(arg(1))+1);
}

doindex(ap,c)
char	**ap;
{
	register char	*subj = arg(1);
	register char	*obj  = arg(2);
	register	i;

	for (i=0; *subj; ++i)
		if (leftmatch(subj++,obj)) {
			pbnum( (long) i );
			return;
		}

	pbnum( (long) -1 );
}

leftmatch(str,substr)
register char	*str;
register char	*substr;
{
	while (*substr)
		if (*str++ != *substr++)
			return (0);

	return (1);
}

/* returns byte length */
dolen(ap,c)
char 	**ap;
{
	pbnum((long) strlen(arg(1)));
}

/* new function - returns display length */
dodlen(ap,c)
char **ap;
{
	wchar_t *wcs=0;
	int mblen, dispcol;
	char 	*p;

	p = arg(1);

	mblen = (  strlen(p) + 1) * sizeof(wchar_t) ;
	wcs = (wchar_t *)malloc(mblen);
	if(wcs == NULL) {
		error(OST,nocore);
	}
	if(mbstowcs(wcs, p, mblen/sizeof(wchar_t)) <0) {
		error2(MBWC,MBtoWCerr,p);
	};
	dispcol = wcswidth(wcs,mblen/sizeof(wchar_t));
	if(dispcol<0){
		error(WCDISP,WCDISPerr);
	}
	pbnum((long)dispcol);

	if (wcs) cfree((char *)wcs);
}

domake(ap,c)
char 	**ap;
{
	if (c > 0)
		pbstr(mktemp(ap[1]));
}

dopopdef(ap,c)
char	**ap;
{
	register	i;

	for (i=1; i<=c; ++i)
		undef(ap[i]);
}

dopushdef(ap,c)
char	**ap;
{
	def(ap,c,PUSH);
}

doshift(ap,c)
register char	**ap;
register c;
{
	if (c <= 1)
		return;

	for (;;) {
		pbstr(rquote);
		pbstr(ap[c--]);
		pbstr(lquote);

		if (c <= 1)
			break;

		pbstr(",");
	}
}

dosincl(ap,c)
char	**ap;
{
	incl(ap,c,0);
}

dosubstr(ap,c)
register char 	**ap;
{
	char	*str;
	int	inlen, outlen;
	register	offset, ix;

	inlen = strlen(str=arg(1));
	offset = atoi(arg(2));

	if (offset<0 || offset>=inlen)
		return;

	outlen = c>=3? atoi(ap[3]): inlen;
	ix = min(offset+outlen,inlen);

	while (ix > offset)
		putbak(str[--ix]);
}

dosyscmd(ap,c)
char 	**ap;
{
	sysrval = 0;
	if (c > 0) {
		fflush(stdout);
		sysrval = system(ap[1]);
	}
}

/* ARGSUSED */
dosysval(ap,c)
char	**ap;
{
	pbnum((long) (sysrval < 0 ? sysrval :
		(sysrval >> 8) & ((1 << 8) - 1)) |
		((sysrval & ((1 << 8) - 1)) << 8));
}

dotransl(ap,c)
char 	**ap;
{

	if(mbcurmax== 1)
		dotransl_sb(ap,c);
	else
		dotransl_mb(ap,c);

}
		

dotransl_sb(ap,c)
char 	**ap;
{
	/* Transliteration takes place as dictated by the from and to strings.
	   The buffer on which this takes place is pointed to by ap[1] 
	   (source and sink) Note that the source buffer is also the 
	   sink buffer, ie, replacements are done in the buffer given by ap[1].
	*/

	char	*sink, *from, *sto;
	register char	*source, *to;
	register char	*p;

	if (c<1)
		return;
	sink 	= ap[1];
	from 	= arg(2);
	sto 	= arg(3);
	for (source = ap[1]; *source; source++) {
		to = sto;
		for (p = from; *p; ++p) {
			if (*source==*p)
				break;
			if (*to)
				++to;
		}
		if (*p) {
			if (*to)
				*sink++ = *to;
		} else
			*sink++ = *source;
	}
	*sink = EOS;
	pbstr(ap[1]);
}
dotransl_mb(ap,c)
char 	**ap;
{
	wchar_t  	*ap1, *sink, *fr, *sto;
	wchar_t		*f_ap1=0, *f_fr=0, *f_sto=0;

	char 	*dest=0;       /* used for final result instead of ap[1] */ 
	int 	dest_len;

	int 	dlen;
	register wchar_t 	*to, *source;
	register wchar_t	*i, *wc;


	if (c<1)
		return;


	f_ap1 = ap1  =	ncgetbuf(ap[1]);
	sink	=	ap1;

	f_fr = fr   =  	(c < 2 ?  (wchar_t*)nullstr : ncgetbuf(ap[2]));
	f_sto = sto =	(c < 3 ?  (wchar_t*)nullstr : ncgetbuf(ap[3]));

	/* how many bytes are need by the multibyte string after wctomb 
	   conversion? Assume each wchar_t can be expanded to MB_CUR_MAX 
	   number of bytes and add one for the terminating null byte 
	*/

	dest_len = (wcslen(sink) +1) * mbcurmax ;
	dest = (char *)malloc(dest_len);
	if(dest == NULL){
		error(OST,nocore);
	}

	for (source = ap1; *source; source++) {
		to = sto;
		for (wc = fr; *wc; ++wc) {
			if (*source == *i)
				break;
			if (*to)
				++to;
		}
		if (*wc) {
			if (*to)
				*sink++ = *to;
		} else
			*sink++ = *source;
	}
	*sink = EOS;

	if (f_fr) cfree((char *)f_fr);
	if (f_sto) cfree((char *)f_sto);

	if(wcstombs( dest, ap1, dest_len/mbcurmax) <0){
		error(WCMB,WCtoMBerr);
	}

	pbstr(dest);

	if (dest) cfree(dest);
	if (f_ap1) cfree((char *)f_ap1);
}

dotroff(ap,c)
register char	**ap;
{
	register struct nlist	*np;

	trace = 0;

	while (c > 0)
		if ((np=lookup(ap[c--]))->name)
			np->tflag = 0;
}

dotron(ap,c)
register char	**ap;
{
	register struct nlist	*np;

	trace = !*arg(1);

	while (c > 0)
		if ((np=lookup(ap[c--]))->name)
			np->tflag = 1;
}

doundef(ap,c)
char	**ap;
{
	register	i;

	for (i=1; i<=c; ++i)
		while (undef(ap[i]))
			;
}

undef(nam)
char	*nam;
{
	register struct	nlist *np, *tnp;

	if ((np=lookup(nam))->name==NULL)
		return 0;
	tnp = hshtab[hshval];	/* lookup sets hshval */
	if (tnp==np)	/* it's in first place */
		hshtab[hshval] = tnp->next;
	else {
		while (tnp->next != np)
			tnp = tnp->next;

		tnp->next = np->next;
	}
	cfree(np->name);
	cfree(np->def);
	cfree((char *) np);
	return 1;
}

doundiv(ap,c)
register char 	**ap;
{
	register int i;

	if (c<=0)
		for (i=1; i<10; i++)
			undiv(i,OK);
	else
		while (--c >= 0)
			undiv(atoi(*++ap),OK);
}

dowrap(ap,c)
char	**ap;
{
	register char	*a = arg(1);

	if (Wrapstr)
		cfree(Wrapstr);

	Wrapstr = xcalloc(strlen(a)+1,sizeof(char));
	strcpy(Wrapstr,a);
}

struct bs	barray[] = {
	dochcom,	"changecom",
	docq,		"changequote",
	dodecr,		"decr",
	dodef,		"define",
	dodefn,		"defn",
	dodiv,		"divert",
	dodivnum,	"divnum",
	dodlen,		"dlen",
	dodnl,		"dnl",
	dodump,		"dumpdef",
	doerrp,		"errprint",
	doeval,		"eval",
	doexit,		"m4exit",
	doif,		"ifelse",
	doifdef,	"ifdef",
	doincl,		"include",
	doincr,		"incr",
	doindex,	"index",
	dolen,		"len",
	domake,		"maketemp",
	dopopdef,	"popdef",
	dopushdef,	"pushdef",
	doshift,	"shift",
	dosincl,	"sinclude",
	dosubstr,	"substr",
	dosyscmd,	"syscmd",
	dosysval,	"sysval",
	dotransl,	"translit",
	dotroff,	"traceoff",
	dotron,		"traceon",
	doundef,	"undefine",
	doundiv,	"undivert",
	dowrap,		"m4wrap",
	0,		0
};
