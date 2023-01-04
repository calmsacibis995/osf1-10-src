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
static char	*sccsid = "@(#)$RCSfile: run.c,v $ $Revision: 4.3.9.2 $ (DEC) $Date: 1993/10/01 01:25:13 $";
#endif 

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifndef lint

#endif
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <nl_types.h>
#include <fcntl.h>

#include "awk.def"
#include "math.h"
#include "awk.h"
#include "awk_msg.h"

#define MSGSTR(Num, Str) catgets(catd,NL_SETD, Num, Str)

extern nl_catd catd;


#define RECSIZE BUFSIZ

struct files_tag
{
	FILE *fp;
	int type;
	char *fname;
} *files;
int filenum;
FILE *popen();

extern obj execute(), nodetoobj(), fieldel(), dopa2(), gettemp();
#define PA2NUM	29
int pairstack[PA2NUM], paircnt;
node *winner = (node *)NULL;
#define MAXTMP 20
cell tmps[MAXTMP];
static cell nullval ={EMPTY,EMPTY,0.0,NUM,0,0};
obj	true	={ OBOOL, BTRUE, 0 };
obj	false	={ OBOOL, BFALSE, 0 };

run()
{
	register int i;

	/* Dynamically allocate the files structure array */
	filenum = getdtablesize();
	files = (struct files_tag *) calloc(filenum,sizeof(struct files_tag));
	if (files == (struct files_tag *) NULL) {
		fprintf(stderr,MSGSTR(NOMEM,"awk: memory allocation error\n"));
		errorflag = 1;
		return;
	}

	execute(winner);

	/* Wait for children to complete if output to a pipe. */
	for (i=0; i<filenum; i++)
		if (files[i].fp && files[i].type == '|')
			pclose(files[i].fp);

	free(files);
}

obj execute(node *u)
{
	register obj (*proc)();
	obj x;
	node *a;
	extern char *printname[];

	if (u==(node *)NULL)
		return(true);
	for (a = u; ; a = a->nnext) {
		if (cantexec(a))
			return(nodetoobj(a));
		if (a->ntype==NPA2)
			proc=dopa2;
		else {
			if (notlegal(a->nobj))
				error(FATAL,
                                MSGSTR(ILLSTAT,"illegal statement %o"), a);
			proc = proctab[a->nobj-FIRSTTOKEN];
		}
		x = (*proc)(a->narg,a->nobj);
		if (isfld(x)) fldbld();
		if (isexpr(a))
			return(x);
		/* a statement, goto next statement */
		if (isjump(x))
			return(x);
		if (a->nnext == (node *)NULL)
			return(x);
		tempfree(x);
	}
}

obj program(node **a, int n)
{
	obj x;

	if (a[0] != NULL) {
		x = execute(a[0]);
		if (isexit(x))
			return(true);
		if (isjump(x))
			error(FATAL,
                        MSGSTR(UXBREAK,"unexpected break, continue or next"));
		tempfree(x);
	}
	while (getrec()) {
		x = execute(a[1]);
		if (isexit(x)) break;
		tempfree(x);
	}
	tempfree(x);
	if (a[2] != NULL) {
		x = execute(a[2]);
		if (isbreak(x) || isnext(x) || iscont(x))
			error(FATAL,
                        MSGSTR(UXBREAK,"unexpected break, continue or next"));
		tempfree(x);
	}
	return(true);
}

obj getline()
{
	obj x;

	x = gettemp();
	setfval(x.optr, (awkfloat) getrec());
	return(x);
}

obj array(node **a, int n)
{
	obj x, y;
	extern obj arrayel();

	x = execute(a[1]);
	y = arrayel(a[0], x);
	tempfree(x);
	return(y);
}

obj arrayel(node *a, obj b)
{
	char *s;
	cell *x;
	int i;
	obj y;

	s = getsval(b.optr);
	x = (cell *) a;
	if (!(x->tval&ARR)) {
		strfree(x->sval);
		x->tval &= ~STR;
		x->tval |= ARR;
		x->sval = (char *) makesymtab();
	}
	y.optr = setsymtab(s, tostring(""), 0.0, STR|NUM, (cell **) x->sval);
	y.otype = OCELL;
	y.osub = CVAR;
	return(y);
}

obj matchop(node **a, int n)
{
	obj x;
	char *s;
	int i;

	x = execute(a[0]);
	if (isstr(x)) s = x.optr->sval;
	else	s = getsval(x.optr);
	tempfree(x);
	i = match(a[1], s);
	if (n==MATCH && i==1 || n==NOTMATCH && i==0)
		return(true);
	else
		return(false);
}

obj boolop(node **a, int n)
{
	obj x, y;
	int i;

	x = execute(a[0]);
	i = istrue(x);
	tempfree(x);
	switch (n) {
	default:
		error(FATAL,
                     MSGSTR(UKBOOL, "unknown boolean operator %d"), n);
	case BOR:
		if (i) return(true);
		y = execute(a[1]);
		i = istrue(y);
		tempfree(y);
		if (i) return(true);
		else return(false);
	case AND:
		if ( !i ) return(false);
		y = execute(a[1]);
		i = istrue(y);
		tempfree(y);
		if (i) return(true);
		else return(false);
	case NOT:
		if (i) return(false);
		else return(true);
	}
}

obj relop(node **a, int n)
{
	int i;
	obj x, y;
	awkfloat j;

	x = execute(a[0]);
	y = execute(a[1]);
	if (x.optr->tval&NUM && y.optr->tval&NUM) {
		j = x.optr->fval - y.optr->fval;
		i = j<0? -1: (j>0? 1: 0);
	} else {
		i = strcmp(getsval(x.optr), getsval(y.optr));
	}
	tempfree(x);
	tempfree(y);
	switch (n) {
	default:
		error(FATAL,
                     MSGSTR(UKRELOP, "unknown relational operator %d"), n);
	case LT:	if (i<0) return(true);
			else return(false);
	case LE:	if (i<=0) return(true);
			else return(false);
	case NE:	if (i!=0) return(true);
			else return(false);
	case EQ:	if (i==0) return(true);
			else return(false);
	case GE:	if (i>=0) return(true);
			else return(false);
	case GT:	if (i>0) return(true);
			else return(false);
	}
}

tempfree(obj a)
{
	if (!istemp(a)) return;
	strfree(a.optr->sval);
	a.optr->tval = 0;
}

obj gettemp()
{
	int i;
	obj x;

	for (i=0; i<MAXTMP; i++)
		if (tmps[i].tval==0)
			break;
	if (i==MAXTMP)
		error(FATAL,
                     MSGSTR(OUTTMP,"out of temporaries in gettemp"));
	x.optr = &tmps[i];
	tmps[i] = nullval;
	x.otype = OCELL;
	x.osub = CTEMP;
	return(x);
}

obj indirect(node **a, int n)
{
	obj x;
	int m;
	cell *fieldadr();

	x = execute(a[0]);
	m = getfval(x.optr);
	tempfree(x);
	x.optr = fieldadr(m);
	x.otype = OCELL;
	x.osub = CFLD;
	return(x);
}

obj substr(node **a, int nnn)
{
	char *s, temp;
	obj x;
	int k, m, n;

	x = execute(a[0]);
	s = getsval(x.optr);
	k = strlen(s) + 1;
	tempfree(x);
	x = execute(a[1]);
	m = getfval(x.optr);
	if (m <= 0)
		m = 1;
	else if (m > k)
		m = k;
	tempfree(x);
	if (a[2] != nullstat) {
		x = execute(a[2]);
		n = getfval(x.optr);
		tempfree(x);
	}
	else
		n = k - 1;
	if (n < 0)
		n = 0;
	else if (n > k - m)
		n = k - m;
	dprintf("substr: m=%d, n=%d, s=%s\n", m, n, s);
	x = gettemp();
	if (s) {
		temp = s[n+m-1];	/* with thanks to John Linderman */
		s[n+m-1] = '\0';
	}
	setsval(x.optr, s + m - 1);
	if (s) 
		s[n+m-1] = temp;
	return(x);
}

obj sindex(node **a, int nnn)
{
	obj x;
	char *s1, *s2, *p1, *p2, *q;

	x = execute(a[0]);
	s1 = getsval(x.optr);
	tempfree(x);
	x = execute(a[1]);
	s2 = getsval(x.optr);
	tempfree(x);

	x = gettemp();
	for (p1 = s1; *p1 != '\0'; p1++) {
		for (q=p1, p2=s2; *p2 != '\0' && *q == *p2; q++, p2++)
			;
		if (*p2 == '\0') {
			setfval(x.optr, (awkfloat) (p1 - s1 + 1));	/* origin 1 */
			return(x);
		}
	}
	setfval(x.optr, 0.0);
	return(x);
}

char *format(char *s, node *a)
{
#define TRUE 1
#define FALSE 0

	char *buf, *p, fmt[200], *t, *os;
	obj x;
	int flag = 0;
	awkfloat xf;

	int VarFieldWidth = FALSE;	/* TRUE if fieldwith seen */
	int VarPrecision = FALSE;	/* TRUE if precision seen */
	int FieldWidth, Precision;	/* hold variable values */

	int endloop;			/* loop variable */

/* macro for Precision and FieldWidth printf expansion */

#define Xsprintf(str,fmt,var)\
	if (!VarFieldWidth)\
		if (!VarPrecision) sprintf(str,fmt,(var));\
		else sprintf(str,fmt,Precision,(var));\
	else\
		if (!VarPrecision) sprintf(str,fmt,FieldWidth,(var));\
		else sprintf(str,fmt,FieldWidth,Precision,(var));


	os = s;
	p = buf = (char *)malloc(RECSIZE); /* buf,p hold complete line */

	while (*s) {			/* loop until d,f,o,x,g or e seen */

		if (*s != '%') {	/* handle all chars before % */
			*p++ = *s++;
			continue;
		}

		if (*(s+1) == '%') {	/* handle a double % */
			*p++ = '%';
			s += 2;
			continue;
		}

		/* to get to here, s must now point to % */

		t = fmt;		/* start buffer for sprintf call */
		*t++ = *s++;		/* store the '%' */
		endloop = TRUE;
		while (endloop)
			switch(*s){
				case ' ':
					s++;	/* throw away spaces */
					break;

				case '-':	/* optional left adjustment */
					*t++ = *s++;	/* store '-' */
					break;

				case '#':	/* optional alternate form */
					*t++ = *s++;	/* store '#' */
					break;

				case 'l':	/* long */
					*t++ = *s++;	/* store 'l' */
					break;

				case '*':	/* precision or field width */
					*t++ = *s++;	/* store '*' */

					if (!VarPrecision){
					    if (VarFieldWidth)
						error(FATAL,
       MSGSTR(FIELDW, "More than one fieldwidth definition in printf(%s)"),os);
					    VarFieldWidth = TRUE;
					    if (a == NULL)
						error(FATAL,
                     MSGSTR(NEARGS, "Not enough arguments in printf(%s)"),os);
					    x = execute(a);
					    FieldWidth = getfval(x.optr);
					    a = a->nnext;
					}
					else{
					    if (a == NULL)
						error(FATAL,
                     MSGSTR(NEARGS, "Not enough arguments in printf(%s)"),os);
					    x = execute(a);
					    Precision = getfval(x.optr);
					    a = a->nnext;
					}
					break;
		
				case '0': case '1': case '2':
				case '3': case '4': case '5':
				case '6': case '7':
				case '8': case '9':
					*t++ = *s++;
					if (VarPrecision)
						VarPrecision = FALSE;
					break;

				case '.':	/* precision */
					*t++ = *s++;	 /* store '.' */
					if (*s == '*') {
					    if (VarPrecision)
						error(FATAL,
       MSGSTR(FIELDW, "More than one fieldwidth definition in printf(%s)"),os);
					    VarPrecision = TRUE;
					}
					break;
				default:
					*t = '\0';
					if (t >= fmt + sizeof(fmt))
						printf(
       MSGSTR(FMTITEM, "format item %.20s... too long"),os);
					endloop = FALSE;
					break;
			}

/* the next characters are either conversion types or literal chars */

		switch (*s) {
		case 'f': case 'e': case 'g':
			flag = 1;
			*t++ = *s++;
			break;
		case 'd': case 'o': case 'x':
			flag = 2;
			*t++ = *s++;
			break;
		case 'c':
			flag = 3;
			*t++ = *s++;
			break;
		case 's':
			flag = 4;
			*t++ = *s++;
			break;
		default:
			flag = 0;
			for(t=fmt;((*s) && (*s != '%'));*t++ = *s++);
			break;
		}
		*t = '\0';

		if (flag == 0) sprintf(p,"%s",fmt);
		else{
			if (a == NULL)
				error(FATAL,
                     MSGSTR(NEARGS, "Not enough arguments in printf(%s)"),os);
			x = execute(a);
			a = a->nnext;
			if (flag != 4)
				xf = getfval(x.optr);
		}

		if (flag==1) Xsprintf(p, fmt, xf)
		else if (flag==2) Xsprintf(p, fmt, (long)xf)
		else if (flag==3) Xsprintf(p, fmt, (int)xf)
		else if (flag==4) Xsprintf(p, fmt, getsval(x.optr)) /* 002 - DNM */
	     /* else if (flag==4) Xsprintf(p, fmt, x.optr->sval==NULL ? "" : getsval(x.optr)) */

		tempfree(x);
		p += strlen(p);
	}
	*p = '\0';
	return(buf);
}

obj asprintf(node **a, int n)
{
	obj x;
	node *y;
	char *s;

	y = a[0]->nnext;
	x = execute(a[0]);
	s = format(getsval(x.optr), y);
	tempfree(x);
	x = gettemp();
	x.optr->sval = s;
	x.optr->tval = STR;
	return(x);
}

obj arith(node **a, int n)
{
	awkfloat i,j;
	obj x,y,z;

	x = execute(a[0]);
	i = getfval(x.optr);
	tempfree(x);
	if (n != UMINUS) {
		y = execute(a[1]);
		j = getfval(y.optr);
		tempfree(y);
	}
	z = gettemp();
	switch (n) {
	default:
		error(FATAL,
                      MSGSTR(ILLAROP,"illegal arithmetic operator %d"), n );
	case ADD:
		i += j;
		break;
	case MINUS:
		i -= j;
		break;
	case MULT:
		i *= j;
		break;
	case DIVIDE:
		if (j == 0)
			error(FATAL, MSGSTR(ZERODIV, "division by zero"));
		i /= j;
		break;
	case MOD:
		if (j == 0)
			error(FATAL, MSGSTR(ZERODIV, "division by zero"));
		i = i - j*(long)(i/j);
		break;
	case UMINUS:
		i = -i;
		break;
	}
	setfval(z.optr, i);
	return(z);
}

obj incrdecr(node **a, int n)
{
	obj x, z;
	int k;
	awkfloat xf;

	x = execute(a[0]);
	xf = getfval(x.optr);
	k = (n == PREINCR || n == POSTINCR) ? 1 : -1;
	if (n == PREINCR || n == PREDECR) {
		setfval(x.optr, xf + k);
		return(x);
	}
	z = gettemp();
	setfval(z.optr, xf);
	setfval(x.optr, xf + k);
	tempfree(x);
	return(z);
}


obj assign(node **a, int n)
{
	obj x, y;
	awkfloat xf, yf;

	x = execute(a[0]);
	y = execute(a[1]);
	if (n == ASSIGN) {	/* ordinary assignment */
		if ((y.optr->tval & (STR|NUM)) == (STR|NUM)) {
			setsval(x.optr, y.optr->sval);
			x.optr->fval = y.optr->fval;
			x.optr->tval |= NUM;
		}
		else if (y.optr->tval & STR)
			setsval(x.optr, y.optr->sval);
		else if (y.optr->tval & NUM)
			setfval(x.optr, y.optr->fval);
		tempfree(y);
		return(x);
	}
	xf = getfval(x.optr);
	yf = getfval(y.optr);
	switch (n) {
	case ADDEQ:
		xf += yf;
		break;
	case SUBEQ:
		xf -= yf;
		break;
	case MULTEQ:
		xf *= yf;
		break;
	case DIVEQ:
		if (yf == 0)
			error(FATAL, MSGSTR(ZERODIV, "division by zero"));
		xf /= yf;
		break;
	case MODEQ:
		if (yf == 0)
			error(FATAL, MSGSTR(ZERODIV, "division by zero"));
		xf = xf - yf*(long)(xf/yf);
		break;
	default:
		error(FATAL,
                      MSGSTR(ILLASSOP, "illegal assignment operator %d"), n );
		break;
	}
	tempfree(y);
	setfval(x.optr, xf);
	return(x);
}

obj cat(node **a, int q)
{
	obj x,y,z;
	int n1, n2;
	char *s;

	x = execute(a[0]);
	y = execute(a[1]);
	getsval(x.optr);
	getsval(y.optr);
	n1 = strlen(x.optr->sval);
	n2 = strlen(y.optr->sval);
	s = (char *) malloc(n1 + n2 + 1);
	strcpy(s, x.optr->sval);
	strcpy(s+n1, y.optr->sval);
	tempfree(y);
	z = gettemp();
	z.optr->sval = s;
	z.optr->tval = STR;
	tempfree(x);
	return(z);
}

obj pastat(node **a, int n)
{
	obj x;

	if (a[0]==nullstat)
		x = true;
	else
		x = execute(a[0]);
	if (istrue(x)) {
		tempfree(x);
		x = execute(a[1]);
	}
	return(x);
}

obj dopa2(node **a, int n)
{
	obj x;

	if (pairstack[n]==0) {
		x = execute(a[0]);
		if (istrue(x))
			pairstack[n] = 1;
		tempfree(x);
	}
	if (pairstack[n] == 1) {
		x = execute(a[1]);
		if (istrue(x))
			pairstack[n] = 0;
		tempfree(x);
		x = execute(a[2]);
		return(x);
	}
	return(false);
}

obj aprintf(node **a, int n)
{
	obj x;

	x = asprintf(a,n);
	if (a[1]==NULL) {
		printf("%s", x.optr->sval);
		tempfree(x);
		return(true);
	}
	redirprint(x.optr->sval, (int)a[1], a[2]);
	return(x);
}

obj split(node **a, int nnn)
{
	obj x;
	cell *ap;
	register char *s, *p;
	char *t, temp, num[5];
	register int sep;
	int n, flag;

	x = execute(a[0]);
	s = getsval(x.optr);
	tempfree(x);
	if (a[2] == nullstat)
		sep = **FS;
	else {
		x = execute(a[2]);
		sep = getsval(x.optr)[0];
		tempfree(x);
	}
	ap = (cell *) a[1];
	freesymtab(ap);
	dprintf("split: s=|%s|, a=%s, sep=|%c|\n", s, ap->nval, sep);
	ap->tval &= ~STR;
	ap->tval |= ARR;
	ap->sval = (char *) makesymtab();

	n = 0;
	if (sep == ' ')
		for (n = 0; ; ) {
			while (*s == ' ' || *s == '\t' || *s == '\n')
				s++;
			if (*s == 0)
				break;
			n++;
			t = s;
			do
				s++;
			while (*s!=' ' && *s!='\t' && *s!='\n' && *s!='\0');
			temp = *s;
			*s = '\0';
			(void)sprintf(num, "%d", n);
			if (isnumber(t))
				setsymtab(num, tostring(t), atof(t), STR|NUM, (cell **) ap->sval);
			else
				setsymtab(num, tostring(t), 0.0, STR, (cell **) ap->sval);
			*s = temp;
			if (*s != 0)
				s++;
		}
	else if (*s != 0)
		for (;;) {
			n++;
			t = s;
			while (*s != sep && *s != '\n' && *s != '\0')
				s++;
			temp = *s;
			*s = '\0';
			(void)sprintf(num, "%d", n);
			if (isnumber(t))
				setsymtab(num, tostring(t), atof(t), STR|NUM, (cell **) ap->sval);
			else
				setsymtab(num, tostring(t), 0.0, STR, (cell **) ap->sval);
			*s = temp;
			if (*s++ == 0)
				break;
		}
	x = gettemp();
	x.optr->tval = NUM;
	x.optr->fval = n;
	return(x);
}

obj ifstat(node **a, int n)
{
	obj x;

	x = execute(a[0]);
	if (istrue(x)) {
		tempfree(x);
		x = execute(a[1]);
	}
	else if (a[2] != nullstat) {
		tempfree(x);
		x = execute(a[2]);
	}
	return(x);
}

obj whilestat(node **a, int n)
{
	obj x;

	for (;;) {
		x = execute(a[0]);
		if (!istrue(x)) return(x);
		tempfree(x);
		x = execute(a[1]);
		if (isbreak(x)) {
			x = true;
			return(x);
		}
		if (isnext(x) || isexit(x))
			return(x);
		tempfree(x);
	}
}

obj forstat(node **a, int n)
{
	obj x;

	tempfree(execute(a[0]));
	for (;;) {
		if (a[1]!=nullstat) {
			x = execute(a[1]);
			if (!istrue(x)) return(x);
			else tempfree(x);
		}
		x = execute(a[3]);
		if (isbreak(x)) {	/* turn off break */
			x = true;
			return(x);
		}
		if (isnext(x) || isexit(x))
			return(x);
		tempfree(x);
		tempfree(execute(a[2]));
	}
}

obj instat(node **a, int n)
{
	cell *vp, *arrayp, *cp, **tp;
	obj x;
	int i;

	vp = (cell *) a[0];
	arrayp = (cell *) a[1];
	if (!(arrayp->tval & ARR))
		error(FATAL,
                      MSGSTR(NOTARRAY,"%s is not an array"), arrayp->nval );
	tp = (cell **) arrayp->sval;
	for (i = 0; i < MAXSYM; i++) {	/* this routine knows too much */
		for (cp = tp[i]; cp != NULL; cp = cp->nextval) {
			setsval(vp, cp->nval);
			x = execute(a[2]);
			if (isbreak(x)) {
				x = true;
				return(x);
			}
			if (isnext(x) || isexit(x))
				return(x);
			tempfree(x);
		}
	}
	return (true);
}

obj jump(node **a, int n)
{
	obj x, y;

	x.otype = OJUMP;
	switch (n) {
	default:
		error(FATAL,
                      MSGSTR(ILLJUMP,"illegal jump type %d"), n );
		break;
	case EXIT:
		if (a[0] != 0) {
			y = execute(a[0]);
			errorflag = getfval(y.optr);
		}
		x.osub = JEXIT;
		break;
	case NEXT:
		x.osub = JNEXT;
		break;
	case BREAK:
		x.osub = JBREAK;
		break;
	case CONTINUE:
		x.osub = JCONT;
		break;
	}
	return(x);
}

obj fncn(node **a, int n)
{
	obj x;
	awkfloat u;
	int t;

	t = (int) a[0];
	x = execute(a[1]);
	if (t == FLENGTH)
		u = (awkfloat) strlen(getsval(x.optr));
	else if (t == FLOG)
		u = log(getfval(x.optr));
	else if (t == FINT)
		u = (awkfloat) (long) getfval(x.optr);
	else if (t == FEXP)
		u = exp(getfval(x.optr));
	else if (t == FSQRT)
		u = sqrt(getfval(x.optr));
	else
		error(FATAL,
                      MSGSTR(ILLFUNC,"illegal function type %d"), t );
	tempfree(x);
	x = gettemp();
	setfval(x.optr, u);
	return(x);
}

obj print(node **a, int n)
{
	register node *x;
	obj y;
	char s[RECSIZE];

	s[0] = '\0';
	for (x=a[0]; x!=NULL; x=x->nnext) {
		y = execute(x);
		strcat(s, getsval(y.optr));
		tempfree(y);
		if (x->nnext==NULL)
			strcat(s, *ORS);
		else
			strcat(s, *OFS);
	}
	if (strlen(s) >= RECSIZE)
		error(FATAL,
                      MSGSTR(TOLONG2,"string %.20s ... too long to print"), s);
	if (a[1]==nullstat) {
		printf("%s", s);
		return(true);
	}
	redirprint(s, (int)a[1], a[2]);
	return(false);
}

obj nullproc() {}

obj nodetoobj(node *a)
{
	obj x;

	x.optr = (cell *) a->nobj;
	x.otype = OCELL;
	x.osub = a->subtype;
	if (isfld(x)) fldbld();
	return(x);
}

redirprint(char *s, int a, node *b)
{
	register int i;
	obj x;

	x = execute(b);
	getsval(x.optr);
	for (i=0; i<filenum; i++)
		if (files[i].fp && strcmp(x.optr->sval, files[i].fname) == 0)
			goto doit;
	for (i=0; i<filenum; i++)
		if (files[i].fp == 0)
			break;
	if (i >= filenum)
		error(FATAL, MSGSTR(TOMANYOF,"too many output files %d"), i );
	if (a == '|')	/* a pipe! */
		files[i].fp = popen(x.optr->sval, "w");
	else if (a == APPEND)
		files[i].fp = fopen(x.optr->sval, "a");
	else
		files[i].fp = fopen(x.optr->sval, "w");
	if (files[i].fp == NULL)
		error(FATAL,
                      MSGSTR(NOOPEN1, "can't open file %s"), x.optr->sval);

	if (fcntl(fileno(files[i].fp), F_SETFD, 1) < 0)
		error(FATAL,
                      MSGSTR(CLOSEXEC,"close on exec failure" ));
	files[i].fname = tostring(x.optr->sval);
	files[i].type = a;
doit:
	fprintf(files[i].fp, "%s", s);
#ifndef gcos
	fflush(files[i].fp);	/* in case someone is waiting for the output */
#endif
	tempfree(x);
}
