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
static char	*sccsid = "@(#)$RCSfile: files.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:09:16 $";
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
/*
 * files.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 *
 *
 * files.c	4.17 (Berkeley) 87/12/21";
 */


/*
 * UNIX DEPENDENT PROCEDURES
 */

#include "defs.h"

#ifndef _BLD
#include "make_msg.h"
extern nl_catd  catd;
#define MSGSTR(Num, Str) catgets(catd, MS_MAKE, Num, Str)
#include <NLctype.h>
#else
#define MSGSTR(Num, Str) Str
#include <ctype.h>
#endif

#define MAXSRCDIRS	255

extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];
extern time_t time();

extern struct depblock *srchdir();

char vpath[INMAX];  /* XXX */
char machine[INMAX];  /* XXX */
FSTATIC char VPATH[] = "VPATH";
FSTATIC char *fixedv = 0;
FSTATIC char curdir[2];
FSTATIC char *newsd[MAXSRCDIRS+1];


setvpath()
{
	register struct varblock *cp;
	extern char *fixpath();

	if ((cp = srchvar(VPATH)) == 0)
		*vpath = 0;
	else {
		if (fixedv && cp->varval != fixedv)
			fixedv = fixpath(cp, newsd);
		(void) subst(cp->varval, vpath);
	}

	if (cp = srchvar("TARGET_MACHINE"))
		(void) subst(cp->varval, machine);
	if (*machine == 0)
		(void) strcpy(machine, "machine");
}


#define	absolute(f)	(*(f) == '/')
#define	dotslash(f)	(*(f) == '.' && *((f)+1) == '/')

time_t
exists(pname, ch, icd)
	struct nameblock *pname;
	register struct chain **ch;
	register int icd;
{
	register char *p, *filename;
	register struct stat *wbp;
	struct stat buf, buf1;
	char fname[MAXPATHLEN+1], temp[MAXPATHLEN+1];
	extern time_t lookarch();
	extern char *execat(), *canon();

	filename = pname->namep;

	if (pname->archp) {
		buf1.st_mtime = lookarch(pname);
		if (stat(filename, &buf) == -1 || buf1.st_mtime > buf.st_mtime)
			return buf1.st_mtime;
		return buf.st_mtime;
	}

	wbp = 0;

	if (icd && !dotslash(filename) && machdep) {
		mksubdir(filename, machine, temp);
		if (wbp == 0 && stat(temp, &buf) == 0) {
			pname->alias = copys(temp);
			if (!rcsdep || (buf.st_mode & S_IFMT) == S_IFDIR)
				/* IF_DIR => machine specific directory??? */
				return buf.st_mtime;
			*(wbp = &buf1) = buf;
		}
		if (coflag && ch && getrcs(temp, pname, &buf, ch, wbp))
			return buf.st_mtime;
	}
	if (!absolute(filename) && !dotslash(filename) && machdep) {
		p = vpath;
		while (p) {
			p = execat(p, filename, fname);
			if (!unequal(filename, canon(fname)))
				continue;
			mksubdir(fname, machine, temp);
			if (wbp == 0 && stat(temp, &buf) == 0) {
				if ((buf.st_mode & S_IFMT) == S_IFDIR) {
					/* machine specfic directory??? */
					mksubdir(filename, machine, fname);
					mkobjdir(fname, temp, &buf);
					pname->alias = copys(fname);
					return buf.st_mtime;
				}
				pname->alias = copys(temp);
				if (!rcsdep)
					return buf.st_mtime;
				*(wbp = &buf1) = buf;
			}
			if (coflag && ch && getrcs(temp, pname, &buf, ch, wbp))
				return buf.st_mtime;
		}
	}

	if (icd) {
		if (wbp == 0 && stat(filename, &buf) == 0) {
			if (!rcsdep || (buf.st_mode & S_IFMT) == S_IFDIR)
				return buf.st_mtime;
			*(wbp = &buf1) = buf;
		}
		if (coflag && ch && getrcs(filename, pname, &buf, ch, wbp))
			return buf.st_mtime;
	}
	if (!absolute(filename) && !dotslash(filename)) {
		p = vpath;
		while (p) {
			p = execat(p, filename, fname);
			if (!unequal(filename, canon(fname)))
				continue;
			if (wbp == 0 && stat(fname, &buf) == 0) {
				if ((buf.st_mode & S_IFMT) == S_IFDIR) {
					mkobjdir(filename, fname, &buf);
					return buf.st_mtime;
				}
				pname->alias = copys(fname);
				if (!rcsdep)
					return buf.st_mtime;
				*(wbp = &buf1) = buf;
			}
			if (coflag && ch && getrcs(fname, pname, &buf, ch, wbp))
				return buf.st_mtime;
		}
	}

	return wbp ? wbp->st_mtime : 0;
}


time_t
prestime()
{
	time_t tvec;

	(void) time(&tvec);
	return tvec;
}


/*
 * stolen from glob through find
 */
int
amatch(s, p)
	register char *s, *p;
{
	register int cc, scc, c;
	int k, lc;

again:
	scc = *s;
	lc = 077777;
	switch (c = *p) {
	case 0:
		return scc == 0;

	case '[':
		k = 0;
		while (cc = *++p) {
			switch (cc) {
			case ']':
				if (k == 0)
					return 0;
				++s, ++p;
				goto again;
			case '-':
				if (*(p+1) == 0 || *(p+1) == ']')
					break;
				p++;
				if ((lc <= scc) && (scc <= *p))
					k++;
				continue;
			case '\\':
				if (index("]-\\", *(p+1)))
					cc = *++p;
				break;
			}
			if (scc == (lc = cc))
				k++;
		}
		return 0;

	case '?':
		if (scc == 0)
			return 0;
		++s, ++p;
		goto again;

	case '*':
		if (*++p == 0)
			return 1;
		while (*s)
			if (amatch(s++, p))
				return 1;
		return 0;

	case '\\':
		if (index("[?*\\", *(p+1)))
			c = *++p;

	default:
		if (c != scc)
			return 0;
		++s, ++p;
		goto again;
	}
}


#ifdef METERFILE
#include <pwd.h>
int meteron	= 0;	/* default: metering off */
extern char *ctime();

meter(file)
	char *file;
{
	time_t tvec;
	char *p;
	FILE * mout;
	struct passwd *pwd;

	if (file == 0 || meteron == 0)
		return;

	pwd = getpwuid(getuid());

	tvec = prestime();

	if ((mout = fopen(file, "a")) != NULL) {
		p = ctime(&tvec);
		p[16] = 0;
		fprintf(mout, MSGSTR(USER,"User %s, %s\n"), pwd->pw_name, p+4);
		(void) fclose(mout);
	}
}
#endif


char *
execat(s1, s2, si)
	register char *s1, *s2;
	char *si;
{
	register char *s;

	s = si;
	while (*s1 && *s1 != ':')
		*s++ = *s1++;
	if (si != s)
		*s++ = '/';
	while (*s2)
		*s++ = *s2++;
	*s = 0;
	return *s1 ? ++s1 : 0;
}


/*
 * copy s to d, changing file names to file aliases
 */
fixname(s, d)
	char *s, *d;
{
	register char *r, *q;
	struct nameblock *pn;
	char name[MAXPATHLEN+1];

	while (*s) {
		if (isspace(*s) || (funny[*s] & META)) {
			*d++ = *s++;
			continue;
		}
		r = name;
		while (*s) {
			if (isspace(*s) || (funny[*s] & META))
				break;
			*r++ = *s++;
		}
		*r = 0;
		if ((pn = srchname(name)) && pn->alias && pn->aliasok)
			q = pn->alias;
		else
			q = name;
		while (*q)
			*d++ = *q++;
	}
	*d = 0;
}

extern struct chain *appendq();

/*
 * Try to find an RCS file corresponding to 'fn', and set the modified
 *	time of the RCS file in the stat buffer.
 * If ch is non-null, then append it on the chain for later check-out.
 */
int
getrcs(fn, p, sb, ch, wb)
	char *fn;
	register struct nameblock *p;
	struct stat *sb;
	struct chain **ch;
	struct stat *wb;
{
	char temp[MAXPATHLEN+1];
	register char *tail;
	register char *s;
	int headlen;

	if ((tail = rindex(fn, '/')) == 0) {
		headlen = 0;
		tail = fn;
	} else
		headlen = ++tail - fn;
	s = ncat(temp, fn, headlen);
	s = ncat(s, RCSdir, -1);
	*s++ = '/';
	s = ncat(s, tail, -1);
	(void) ncat(s, RCSsuf, -1);
	if (stat(temp, sb) == -1)
		if (stat(concat(fn, RCSsuf, temp), sb) == -1)
			return 0;
	rcstime(temp, sb);
	if (wb && sb->st_mtime <= wb->st_mtime)
		sb->st_mtime = wb->st_mtime;
	else if (wb && (wb->st_mode & 0222)) {
		if (!keepgoing)
			fatal(MSGSTR(OLDRCS1,"%s: old RCS working file"), fn);
		printf(MSGSTR(OLDRCS,"%s: old RCS working file\n"), fn);
	} else if (ch) {
		p->RCSnamep = copys(temp);
		*ch = appendq(*ch, p->namep);
		p->alias = 0;
	}
	return 1;
}


/*
 * Try to find the modification time of a particular version.
 */
rcstime(rcsf, sb)
	char *rcsf;
	struct stat *sb;
{
	register FILE *f;
	register char *p;
	int ign, nopr;
	char cmd[OUTMAX], buf[OUTMAX];
	char iobuf[BUFSIZ];
	long t;
	extern FILE *pfopen();
	extern long atol();

	if (rcstime_cmd == 0)
		return;
	setvar("<", rcsf, 1);
	(void) subst(rcstime_cmd->shp->shbp, buf);
	fixname(buf, cmd);
	ign = ignerr;
	nopr = NO;
	for (p = cmd; *p == '-' || *p == '@'; ++p)
		if (*p == '-')
			ign = YES;
		else
			nopr = YES;
	if (*p == 0)
		return;
	if (!silflag && !nopr)
		printf("%s%s\n", (noexflag ? "" : prompt), p);
	if (noexflag)
		return;
	if ((f = pfopen(p, ign)) == NULL)
		return;
	setbuf(f, iobuf);
	if (fgets(buf, sizeof(buf), f)
	&& (t = atol(buf)))
		sb->st_mtime = t;
	(void) pfclose(f, ign);
}


/*
 * really check-out a Make description file.
 */
struct nameblock *
rcsco(descfile)
	char *descfile;
{
	struct nameblock *omainname, *p;
	struct chain *ormchain;
	int onoexflag;
	struct chain *ch;

	omainname = mainname;
	p = makename(descfile);
	mainname = omainname;
	ch = 0;
	if ((p->modtime = exists(p, &ch, 1)) && ch) {
		ormchain = rmchain;
		rmchain = deschain;
		onoexflag = noexflag;
		noexflag = NO;
		co(ch);
		noexflag = onoexflag;;
		deschain = rmchain;
		rmchain = ormchain;
	}
	return p;
}


/*
 * Try to check-out the files specified in ch, if they do not
 *	already exist.  If rmflag is true, mark successful attempts
 *	for automatic deletion.
 * Try to make the modified time of the file the same as that of the
 *	RCS file.
 */
co(ch)
	register struct chain *ch;
{
	register struct nameblock *p;
	register char *file;
	char *RCSfile;
	struct stat sbuf;
	int i, onocmds;
	long tv[2];

	for (; ch; ch = ch->nextp) {
		if ((file = ch->datap) == 0
		|| (p = srchname(file)) == 0
		|| p->RCSnamep == 0)
			continue;
		if (p->alias)
			file = p->alias;
		if (!rcsdep && stat(file, &sbuf) == 0)
			continue;	/* don't do it again */
		RCSfile = p->RCSnamep;
		p->RCSnamep = 0;
		onocmds = nocmds;
		nocmds = NO;
		setvar("@", file, 1);
		setvar("<", RCSfile, 1);
		i = docom(co_cmd);
		setvar("@", (char *) 0, 1);
		nocmds = onocmds;
		if (i)
			continue;	/* docom() failed */
		/*
		 * since we succeeded, mark it for later deletion
		 */
		if (rmflag && !nocmds && !isprecious(p))
			rmchain = appendq(rmchain, file);
		if (noexflag || questflag)
			continue;	/* docom() did nothing */
		/*
		 * try to set modified time on file
		 */
		tv[0] = prestime();
		tv[1] = p->modtime;
		(void) utime(file, tv);
	}
}


/*
 * delete the files listed in rmchain and deschain
 */
rm()
{
	struct nameblock *p;
	register struct lineblock *lp;
	int onoexflag;
	static int once = 0;
	char *mkqlist();

	if (once)
		return;
	once = 1;
	if ((p = srchname(".CLEANUP")) == 0)
		return;
	for (lp = p->linep; lp; lp = lp->nxtlineblock)
		if (lp->shp)
			break;
	if (lp == 0)
		return;
	if (rmchain) {
		setvar("?", mkqlist(rmchain), 1);
		(void) docom(lp);
	}
	if (deschain) {
		setvar("?", mkqlist(deschain), 1);
		onoexflag = noexflag;
		noexflag = NO;
		(void) docom(lp);
		noexflag = onoexflag;
	}
}


mksubdir(path, subdir, temp)
	char *path;
	char *subdir;
	char *temp;
{
	register char *tail, *s;
	register int headlen;

	if ((tail = rindex(path, '/')) == 0) {
		s = ncat(temp, subdir, -1);
		*s++ = '/';
		(void) ncat(s, path, -1);
	} else {
		headlen = ++tail - path;
		s = ncat(temp, path, headlen);
		s = ncat(s, subdir, -1);
		*s++ = '/';
		(void) ncat(s, tail, -1);
	}
}


mkobjdir(ofn, sfn, sst)
	char *ofn, *sfn;
	struct stat *sst;
{
	register char *p, *q;
	struct stat sbuf;

	for (p = sfn; *p; p++)
		;
	for (q = ofn; *q; q++)
		;
	while (q != ofn)
		if (p == sfn || *--p != *--q)
			fatal(MSGSTR(NOTSUFFIX,"mkobjdir: not suffix"));
	for (;;) {
		while (*q && *q != '/')
			p++, q++;
		if (*q == 0)
			break;
		*p = 0;  *q = 0;
		if (stat(ofn, &sbuf) == -1) {
			if (stat(sfn, &sbuf) == -1
			|| (sbuf.st_mode & S_IFMT) != S_IFDIR)
				fatal(MSGSTR(BOTCH,"mkobjdir: dir botch: %s"),
				       sfn);
			if (!silflag)
				printf(MSGSTR(SMKDIR,"%smkdir %s\n"), 
				       (noexflag ? "" : prompt), ofn);
			if (mkdir(ofn, (int) sbuf.st_mode & 0777) == -1)
				fatal(MSGSTR(CANTMKDIR,
					     "Couldn't make directory: %s"), ofn);
		} else if ((sbuf.st_mode & S_IFMT) != S_IFDIR)
			fatal(MSGSTR(NOTADIR,"Not a directory: %s"), ofn);
		for (*q++ = '/', *p++ = '/'; *q == '/'; p++, q++)
			;
	}
	if (!silflag)
		printf(MSGSTR(SMKDIR,"%smkdir %s\n"), 
		       (noexflag ? "" : prompt), ofn);
	if (mkdir(ofn, (int) sst->st_mode & 0777) == -1)
		fatal(MSGSTR(CANTMKDIR,"Couldn't make directory: %s"), ofn);
}


/*
 * Do the srchdir for RCS files.  For
 * a pattern a/b, it searches a/RCS/b.
 */
srchRCS(pat)
	char *pat;
{
	char temp[MAXPATHLEN+1];

	mksubdir(pat, RCSdir, temp);
	(void) srchdir(temp, NO, (struct depblock *) 0);
}


/*
 * Do the srchdir for machine specific files.  For
 * a pattern a/b, it searches a/$(MACHINE)/b.
 */
srchmachine(pat)
	char *pat;
{
	char temp[MAXPATHLEN+1];

	if (machdep == NO || (*pat == '.' && *(pat+1) == '/'))
		return;
	mksubdir(pat, machine, temp);
	(void) srchdir(temp, NO, (struct depblock *) 0);
	if (coflag)
		srchRCS(temp);
}


/*
 * configuration file support
 */
#define getwd	AFSkludge_getwd
extern char *getenv(), *getwd();

FSTATIC char MAKECONF[]	= "MAKECONF";
FSTATIC char OBJECTS[]	= "OBJECTDIR";
FSTATIC char SOURCES[]	= "SOURCEDIR";
FSTATIC char MAKECWD[]	= "MAKECWD";
FSTATIC char MAKEPSD[]	= "MAKEPSD";
FSTATIC char MAKEIOD[]	= "MAKEIOD";
FSTATIC char MAKEDIR[]	= "MAKEDIR";
FSTATIC char MAKETOP[]	= "MAKETOP";
FSTATIC char MAKESUB[]	= "MAKESUB";

/*
 * Support for the CMU "super root" style remote file system.
 * Depends upon the characteristics of getwd(3) in super root.
 */
int hascmurfs		= 0;
FSTATIC char superpre[]	= "/../.LOCALROOT";
#define superroot(d)	(hascmurfs && strncmp(d, superpre, 4) == 0)


makemove(cf)
	char *cf;
{
	char *owd, *psd, cwd[MAXPATHLEN+1];
	struct stat sbuf;

	if (getwd(cwd) == 0 || *cwd != '/')
		fatal(MSGSTR(MKMOVE,"makemove: %s"), cwd);
	if (stat(superpre, &sbuf) != -1)
		hascmurfs = 1;
	if ((owd = getenv(MAKECWD)) && (psd = getenv(MAKEPSD)))
		movedmake(cwd, owd, psd);
	else
		plainmake(cwd, cf);
}


/*
 * find and read the configuration file and move
 * to the object tree - fix VPATH if required
 */
plainmake(cwd, cf)
	char *cwd, *cf;
{
	register char *d, *p, *q;
	struct nameblock *pname, *omainname;
	struct chain *ch;
	char *scf, path[MAXPATHLEN+1];

	setvpath();
	*vpath = 0;  /* should configuration files be found on VPATH? */
	omainname = mainname;
	pname = makename(cf);
	scf = pname->namep;
	mainname = omainname;
	ch = 0;
	if (exists(pname, &ch, 1)) {
		confmove(cwd, pname, (char *) 0, (char *) 0);
		return;
	}
	d = cwd;
	while (*++d)
		;
	while (*(d-1) == '/') {
		if (--d == cwd)
			goto noconf;
		*d = 0;
	}
	while (*(d-1) != '/')
		--d;
	q = strcpy(path + sizeof(path) - 1 - strlen(cf), cf);
	*--q = '/';  p = q;  *--q = '.';  *--q = '.';
	for (;;) {
		pname->namep = q;
		if (exists(pname, &ch, 1)) {
			pname->namep = copys(q);
			*p = 0;
			confmove(cwd, pname, q, d);
			return;
		}
		while (*(d-1) == '/')
			if (--d == cwd)
				goto noconf;
		while (*(d-1) != '/')
			--d;
		if (q < path + 3)
			fatal(MSGSTR(LONGPATH,"plainmake: path too long"));
		*--q = '/';  *--q = '.';  *--q = '.';
	}
noconf:
	pname->namep = scf;
	confmove(cwd, (struct nameblock *) 0, (char *) 0, (char *) 0);
}


/*
 * we may have changed directories, so we fix VPATH accordingly
 */
movedmake(cwd, owd, psd)
	char *cwd, *owd, *psd;
{
	register char *p, *q, *r, *s;
	register char **v;
	struct nameblock *pname, *omainname;
	struct chain *ch;
	char *ep, temp[INMAX];  /* XXX */
	extern char *rdircat(), *fixpath();
	extern char *canon(), *vtop();

	if (*owd != '/')
		fatal(MSGSTR(BADENV,"bad environment (movedmake)"));
	if (!superroot(owd) && superroot(cwd)) {
		for (p = owd; *p; p++)
			;
		do
			*(p + sizeof(superpre) - 1) = *p;
		while (p-- > owd);
		for (p = owd, q = superpre; *q; *p++ = *q++)
			;
	}
	for (p = r = owd, q = s = cwd; *r && *r == *s; r++, s++)
		if (*r == '/') {
			while (*(r+1) == '/')
				r++;
			while (*(s+1) == '/')
				s++;
			p = r + 1;
			q = s + 1;
		}
	if (*r == 0 && *s == 0) {
		p = r;
		q = s;
	} else if (*r == '/' && *s == 0) {
		while (*++r && *r == '/')
			;
		p = r;
		q = s;
	} else if (*r == 0 && *s == '/') {
		p = r;
		while (*++s && *s == '/')
			;
		q = s;
	}
	(void) ptov(psd, newsd);
	for (v = newsd; *v; v++) {
		r = temp;
		s = *v;
		if (*s != '/') {
			r = rdircat(q, p, r);
			if (r != temp && *(r-1) != '/')
				*r++ = '/';
		}
		while (*s)
			*r++ = *s++;
		if (r != temp && *(r-1) != '/' && (*p || *q))
			*r++ = '/';
		(void) rdircat(p, q, r);
		*v = copys(canon(temp));
	}
	if ((ep = getenv(MAKEDIR)) == 0)
		ep = "";
	else if (*ep == '/')
		ep++;
	r = temp;
	for (*r++ = '/'; *ep; *r++ = *ep++)
		;
	if (*(r-1) != '/' && (*p || *q))
		*r++ = '/';
	(void) rdircat(p, q, r);
	setenviron(MAKEDIR, canon(temp), 1);
	if (dbgflag)
		printf("%s=%s\n", MAKEDIR, temp);
	setpathvars(temp);
	if (ep = getenv(MAKECONF)) {
		setvpath();
		*vpath = 0;
		omainname = mainname;
		pname = makename(ep);
		mainname = omainname;
		ch = 0;
		if (exists(pname, &ch, 1))
			readconf((char *) 0, pname);
	}
	if (!didchdir())
		unsetvar(OBJECTS);
	setenviron(MAKECWD, cwd, 1);
	if (dbgflag)
		printf("%s=%s\n", MAKECWD, cwd);
	setenviron(MAKEPSD, vtop(newsd, temp), 1);
	if (dbgflag)
		printf("%s=%s\n", MAKEPSD, temp);
	fixedv = fixpath(varptr(VPATH), newsd);
}


setpathvars(dir)
	char *dir;
{
	register char *p, *q;
	char temp[INMAX];  /* XXX */

	setvar(MAKEDIR, copys(dir), 0);
	while (*dir == '/')
		dir++;
	p = temp;  q = dir;
	while (*q) {
		if (*q == '.' && strncmp(q, "../", 3) == 0)
			fatal("%s: bad MAKEDIR", dir);
		if (*q++ == '/') {
			while (*q == '/')
				q++;
		} else {
			*p++ = '.'; *p++ = '.'; *p++ = '/';
			while (*q && *q != '/')
				q++;
		}
	}
	*p = 0;
	setvar(MAKETOP, copys(temp), 0);
	p = temp;  q = dir;
	for (q = dir; *q; q++)
		*p++ = *q;
	if (p != temp && *(p-1) != '/')
		*p++ = '/';
	*p = 0;
	setvar(MAKESUB, copys(temp), 0);
}


confmove(cwd, p, pre, suf)
	char *cwd;
	struct nameblock *p;
	char *pre, *suf;
{
	register char *q, *r, *objb;
	register struct varblock *vp;
	char obj[MAXPATHLEN+1];
	char src[INMAX], temp[INMAX];  /* XXX */
	extern char *fixvar(), *vtop();

	q = src;
	*q++ = '/';
	if (r = suf)
		while (*r)
			*q++ = *r++;
	*q = 0;
	setenviron(MAKEDIR, src, 1);
	if (dbgflag)
		printf("%s=%s\n", MAKEDIR, src);
	setpathvars(src);
	if (p)
		readconf(cwd, p);
	*(objb = obj) = 0;
	if (vp = srchvar(OBJECTS)) {
		(void) subst(vp->varval, temp);
		if (dbgflag)
			printf("< %s=%s\n", vp->varname, temp);
		if (ptov(temp, newsd) > 1)
			fatal(MSGSTR(MAXCOMP,"%s: only one component allowed"),
			       OBJECTS);
		objb = fixvar(newsd, obj, pre, suf);
		if (dbgflag)
			printf("> %s=%s\n", vp->varname, vtop(newsd, temp));
	}
	newsd[0] = curdir;  /* implicit current dir */
	newsd[1] = 0;
	if (vp = srchvar(SOURCES)) {
		(void) subst(vp->varval, temp);
		if (dbgflag)
			printf("< %s=%s\n", vp->varname, temp);
		(void) ptov(temp, newsd + 1);
		(void) fixvar(newsd, src, pre, suf);
		if (dbgflag)
			printf("> %s=%s\n", vp->varname, vtop(newsd, temp));
	}
	if (*obj == 0)
		unsetvar(OBJECTS);
	if (*obj || newsd[1])
		reldir(cwd, obj, objb);
}


readconf(cwd, p)
	char *cwd;
	struct nameblock *p;
{
	register FILE *f;
	register char *cf;
	char buf[INMAX];  /* XXX */
	static char cmd[] = "$(CO) -q -p $<";
	extern char *canon();
	extern FILE *pfopen();

	cf = p->alias ? p->alias : p->namep;
	if (dbgflag)
		printf(MSGSTR(USING,"using %s\n"), cf);
	if (cwd) {
		if (*cf != '/')
			(void) strcat(strcat(strcpy(buf, cwd), "/"), cf);
		else
			(void) strcpy(buf, cf);
		setenviron(MAKECONF, canon(buf), 1);
		if (dbgflag)
			printf("%s=%s\n", MAKECONF, buf);
		setvar(MAKECONF, copys(buf), 0);
	}
	if (p->RCSnamep) {
		setvar("<", p->RCSnamep, 1);
		p->RCSnamep = 0;
		(void) subst(cmd, buf);
		if (!silflag)
			printf("%s%s\n", (noexflag ? "" : prompt), buf);
		if ((f = pfopen(buf, 0)) == NULL) {
			perror(buf);
			return;
		}
		rdd1(f, p->namep);
		(void) pfclose(f, 0);
	} else {
		if ((f = fopen(cf, "r")) == NULL) {
			perror(cf);
			return;
		}
		rdd1(f, cf);
		(void) fclose(f);
	}
}


char *
fixvar(pv, buf, pre, suf)
	char **pv, *buf, *pre, *suf;
{
	register char **v, *p, *q;
	register char *r, *s, *b;
	extern char *canon();

	b = buf;
	for (q = b, v = pv; *v; v++) {
		r = q;
		p = canon(*v);
		if ((s = pre) && *s && *p != '/') {
			do
				*q++ = *s++;
			while (*s);
			if (*p || (suf && *suf))
				*q++ = '/';
		}
		while (*p)
			*q++ = *p++;
		b = q;
		if ((s = suf) && *s) {
			if (q != r && *(q-1) != '/')
				*q++ = '/';
			do
				*q++ = *s++;
			while (*s);
		}
		*q++ = 0;
		*v = r;
	}
	return b;
}


/*
 * find names for the source directories after a
 * chdir(obj) and use them to adjust search path
 */
reldir(cwd, obj, objb)
	char *cwd, *obj, *objb;
{
	register char **v;
	register char *h, *t;
	register char *q, *r;
	char dir[MAXPATHLEN+1];
	char twd[MAXPATHLEN+1];
	extern char *canon();

	for (v = newsd; h = *v; v++) {
		if (*h == '/') {
			*v = copys(h);
			continue;
		}
		*(r = twd) = *(q = cwd);
		while (*++r = *++q)
			;
		while (--r != twd && *r == '/')
			;
		while (*h)
			h++;
		q = dir + sizeof(dir) - 1;
		for (*q = 0; h != *v; *--q = *--h)
			;
		if (*obj == '/' || superroot(cwd)) {
			if (*q)
				*--q = '/';
			for (r++; r != twd; *--q = *--r)
				;
			*v = copys(canon(q));
			continue;
		}
		for (h = t = obj; *h; h = t) {
			while (*t && *t != '/')
				t++;
			if (t != h + 1 || *h != '.') {
				if (*q)
					*--q = '/';
				if (t == h+2 && *h == '.' && *(h+1) == '.') {
					if (r == twd) {
						if (*q != '/')
							*--q = '/';
						break;
					}
					while (*r != '/')
						*--q = *r--;
					while (r != twd && *r == '/')
						--r;
				} else {
					for (*++r = '/'; h != t; *++r = *h++)
						;
					*--q = '.';  *--q = '.';
				}
			}
			while (*t == '/')
				t++;
		}
		*v = copys(canon(q));
	}
	makechdir(obj, objb, cwd);
}


makechdir(obj, objb, cwd)
	register char *obj;
	char *objb, *cwd;
{
	char temp[INMAX];  /* XXX */
	extern char *vtop(), *fixpath();

	if (*obj) {
		makepath(obj, *newsd, objb);
		if (!silflag)
			printf(MSGSTR(SCD,"%scd %s\n"), (noexflag ? "" : prompt), obj);
		if (chdir(obj) == -1)
			fatal("%s: %s", obj, (errno < sys_nerr) ?
			      sys_errlist[errno] : "chdir error");
		if (getwd(cwd = temp) == 0 || *cwd != '/')
			fatal(MSGSTR(MKCHDIR,"makechdir: %s"), cwd);
		setenviron(MAKEIOD, MAKEIOD, 1);
	}
	setenviron(MAKECWD, cwd, 1);
	if (dbgflag)
		printf("%s=%s\n", MAKECWD, cwd);
	setenviron(MAKEPSD, vtop(newsd, temp), 1);
	if (dbgflag)
		printf("%s=%s\n", MAKEPSD, temp);
	fixedv = fixpath(varptr(VPATH), newsd);
}


didchdir()
{
	register char *p;

	return (p = getenv(MAKEIOD)) != 0 && strcmp(p, MAKEIOD) == 0;
}


makepath(dir, src, dirb)
	char *dir, *src, *dirb;
{
	register char *p, *q, *r, *s;
	register char c, c1;
	char buf[MAXPATHLEN+1];
	struct stat sbuf;
	extern char *canon();

	p = buf;
	if (*src != '/') {
		for (q = dir; *q; *p++ = *q++)
			;
		*p++ = '/';
	}
	for (q = src; *p = *q; p++, q++)
		;
	src = canon(buf);
	for (p = dir; *p; p++)
		;
	for (q = src; *q; q++)
		;
	r = p;
	s = q;
	while (p > dir && q > src && *--p == *--q) {
		if (*p != '/')
			continue;
		r = p + 1;
		s = q + 1;
		while (p > dir && *(p - 1) == '/')
			--p;
		while (q > src && *(q - 1) == '/')
			--q;
	}
	for (q = dir; *q == '/'; q++)
		;
	while (*q) {
		while (*++q && *q != '/')
			;
		c = *q;
		*q = 0;
		if (q > r)
			while (*s && *s != '/')
				s++;
		if (stat(dir, &sbuf) == -1) {
			if (q <= dirb)
				fatal(MSGSTR(NODIR,"No such directory: %s"), dir);
			sbuf.st_mode = 0777;
			if (q > r) {
				c1 = *s;
				*s = 0;
				if (stat(src, &sbuf) != -1 && dbgflag)
					printf(MSGSTR(MODE, "using mode of %s\n"), src);
				*s = c1;
			}
			if (!silflag)
				printf(MSGSTR(SMKDIR,"%smkdir %s\n"),
				       (noexflag ? "" : prompt), dir);
			if (mkdir(dir, (int) sbuf.st_mode & 0777) == -1)
				fatal(MSGSTR(CANTMKDIR,
					 "Couldn't make directory: %s"), dir);
		} else if ((sbuf.st_mode & S_IFMT) != S_IFDIR)
			fatal(MSGSTR(NOTADIR,"Not a directory: %s"), dir);
		if (q > r)
			while (*s == '/')
				s++;
		*q = c;
		while (*q == '/')
			q++;
	}
}


/*
 * c = rev(a) | '/' | b
 */
char *
rdircat(a, b, c)
	char *a, *b;
	register char *c;
{
	register char *p;

	for (p = a; *p; *c++ = '.', *c++ = '.', *c++ = '/') {
		if (*p == '/'
		|| (*p == '.' && (*++p == 0 || *p == '/'
		|| (*p == '.' && (*++p == 0 || *p == '/')))))
			fatal(MSGSTR(BADDIR,"bad directory: %s"), a);
		while (*p && *p++ != '/')
			;
	}
	if (*a && *b == 0)
		*--c = 0;
	else
		for (p = b; *c = *p++; c++)
			;
	return c;
}


int
ptov(p, v)
	register char *p;
	register char **v;
{
	char **u = v;

	if (p && *p)
		for (;;) {
			*v++ = p;
			while (*p && *p != ':')
				p++;
			if (*p == 0)
				break;
			*p++ = 0;
		}
	*v = 0;
	return v - u;
}


char *
vtop(v, p)
	register char **v;
	register char *p;
{
	register char *q;
	char *r = p;

	if (q = *v)
		for (;;) {
			while (*q)
				*p++ = *q++;
			if ((q = *++v) == 0)
				break;
			*p++ = ':';
		}
	*p = 0;
	return r;
}


/*
 * modify each relative PATH component with the dirs
 */
char *
fixpath(cp, dirs)
	struct varblock *cp;
	char **dirs;
{
	register char *p, *q, *r;
	register char **u, **v, **w;
	char *cpv[MAXSRCDIRS+1], *fpv[MAXSRCDIRS+1];
	char temp[INMAX], path[INMAX];  /* XXX */
	extern char *canon();

	(void) subst(cp->varval, temp);
	if (dbgflag)
		printf("< %s=%s\n", cp->varname, temp);
	v = cpv;
	if (*temp != ':')
		*v++ = "";
	(void) ptov(temp, v);
	p = path;
	u = fpv;
	for (v = cpv; *v; v++) {
		if (v != cpv && **v == 0)
			continue;
		*u++ = p;
		for (q = *v; *q; *p++ = *q++)
			;
		*p++ = 0;
		if (**v == '/')
			continue;
		for (w = dirs; *w; w++) {
			if (**w == 0)
				continue;
			r = p;
			for (q = *w; *q; *p++ = *q++)
				;
			if (q != *w && **v)
				*p++ = '/';
			for (q = *v; *q; *p++ = *q++)
				;
			*p = 0;
			*u++ = p = canon(r);
			while (*p++)
				;
		}
	}
	*u = 0;
	cp->varval = copys(vtop(fpv + 1, temp));
	if (dbgflag)
		printf("> %s=%s\n", cp->varname, cp->varval);
	return cp->varval;
}


char *
canon(f)
	char *f;
{
	register char *p, *q, *r;
	register char **sp, **tp;
	char *stack[MAXPATHLEN >> 1];

	p = q = f;
	sp = tp = stack + (sizeof(stack)/sizeof(stack[0]));
	if (*q == '/') {
		p++;
		while (*++q == '/')
			;
	}
	while (*(r = q)) {
		while (*++q && *q != '/')
			;
		if (q != r+1 || *r != '.') {
			if (q != r+2 || *r != '.' || *(r+1) != '.')
				*--sp = r;
			else if (sp != tp)
				sp++;
			else {
/* How did this ever work? Fred Canter -- 4/1/91 */
/*				*p++ = '.', *p++ = '.', *q && (*p++ = '/');  */
				*p++ = '.', *p++ = '.', *p++ = '/';
			}
		}
		while (*q == '/')
			q++;
	}
	while (tp > sp)
		for (q = *--tp; *q; )
			if ((*p++ = *q++) == '/')
				break;
	if (p > f+1 && *(p-1) == '/')
		--p;
	*p = 0;
	return f;
}


#undef getwd
extern char *getwd();

char *
AFSkludge_getwd(p)
	char *p;
{
	register char *q;

	if ((q = getwd(p)) == 0 || strncmp(q, "/afs/.", 6))
		return q;
	for (q += 5; *q = *(q+1); q++)
		continue;
	return p;
}
