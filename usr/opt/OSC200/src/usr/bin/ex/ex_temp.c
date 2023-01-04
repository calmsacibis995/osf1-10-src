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
static char rcsid[] = "@(#)$RCSfile: ex_temp.c,v $ $Revision: 4.3.8.2 $ (DEC) $Date: 1993/06/10 20:28:51 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: (CMDEDIT) ex_temp.c
 *
 * FUNCTION: KILLreg, REGblk, TSYNC, YANKline, YANKreg, blkio, cleanup,
 * fileinit, getREG, getblock, getline, kshift, mapreg, notpart, partreg,
 * putline, putreg, rbflush, regbuf, regio, shread, synctmp, tflush, tlaste
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.15  com/cmd/edit/vi/ex_temp.c, cmdedit, bos320, 9130320 7/1/91 16:31:31
 * 
 */
/* Copyright (c) 1981 Regents of the University of California */

#include "ex.h"
#include "ex_temp.h"
#include "ex_vis.h"
#include "ex_tty.h"
#include <time.h>
#include <sys/file.h>

/*
 * Editor temporary file routines.
 * Very similar to those of ed, except uses 2 input buffers.
 */
#define READ	0
#define WRITE	1


static void blkio(short, wchar_t *, ssize_t(*)(int, void *, size_t));
static int shread(void);
static int getREG(void);
static int REGblk(void);
static void kshift(void);
static void rbflush(void);

static	wchar_t *getblock(line, int);

/* size of pathnames set to _POSIX_PATH_MAX */
static	char	tfname[256];
static	char	rfname[256];
static	int	havetmp;
short tfile = -1;


static	short	rfile = -1;
/*
 * Vi insists on putting one 'NULL' line in the buffer (see fixzero()).
 * This means that for the most common case (entering vi with a filename),
 * the tempfile gets created, the NULL line is added, and then the file
 * you're editing gets loaded.  Since the tmpfile is not 'empty', it gets
 * deleted and recreated.  If the /tmp directory is big, this may take 
 * some time, so we make a special check for this case.
 *	if (tline == INCRMT * (HBLKS+2))
 *		return;
 */

void fileinit(void)
{
	register char *p;
	register int i, j;
	struct stat stbuf;

	if (tline == INCRMT * (HBLKS+2))
		return;
	cleanup((short)0);
	close(tfile);
	tline = INCRMT * (HBLKS+2);
	blocks[0] = HBLKS;
	blocks[1] = HBLKS+1;
	blocks[2] = -1;
	dirtcnt = 0;
	iblock = -1;
	iblock2 = -1;
	oblock = -1;
	strcpy(tfname, svalue(DIRECTORY));
	if (stat(tfname, &stbuf)) {
dumbness:
		if (setexit() == 0)
			filioerr(tfname);
		else
			putNFL();
		cleanup((short)1);
		catclose(ex_catd);
		exit(1);
	}
	if ((stbuf.st_mode & S_IFMT) != S_IFDIR) {
		errno = ENOTDIR;
		goto dumbness;
	}
	ichanged = 0;
	ichang2 = 0;
	ignore(strcat(tfname, "/ExXXXXX"));
	for (p = strend(tfname), i = 5, j = getpid(); i > 0; i--, j /= 10)
		*--p = j % 10 | '0';
	tfile = creat(tfname, 0600);
	if (tfile < 0)
		goto dumbness;
	{
		extern stilinc; 	/* see below */
		stilinc = 0;
	}
	havetmp = 1;
	close(tfile);
	tfile = open(tfname, 2);
	if (tfile < 0)
		goto dumbness;
}

void cleanup(short all)
{
	extern char termtype[];
	if (all) {
		putpad(exit_ca_mode);
		flush();
		resetterm();
		normtty--;
	}
	if (havetmp)
		unlink(tfname);
	havetmp = 0;
	if (all && rfile >= 0) {
		unlink(rfname);
		close(rfile);
		rfile = -1;
	}
	if (all == 1) {
		/* close the message catalog */
		catclose(ex_catd);
		exit(0);
	}
}


void getline(line tl)
{
	register wchar_t *bp, *lp;
	register int nl;

	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl &= ~OFFMSK;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock(tl += INCRMT, READ);
			nl = nleft;
		}
}

line putline(void)
{
	register wchar_t *bp, *lp;
	register int nl;
	line tl;

	dirtcnt++;
	lp = linebuf;
	change();
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl &= ~OFFMSK;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl += INCRMT, WRITE);
			nl = nleft;
		}
	}
	tl = tline;
	tline += (((lp - linebuf) + BNDRY - 1) >> SHFT) & 077776;
	return (tl);
}

/*
 * atl is in terms of characters, not bytes.
 */
static wchar_t * getblock(line atl, int iof)
{
	register int bno, off; /* off is in terms of characters */
        register char *p1, *p2;
        register int n;

	bno = (atl >> OFFBTS) & BLKMSK;
	off = (atl << SHFT) & LBTMSK;
	if (bno >= NMBLKS)
		error(MSGSTR(M_194, " Tmp file too large"), DUMMY_INT);
	nleft = INCRMT - off;
	if (bno == iblock) {
		ichanged |= iof;
		hitin2 = 0;
		return (ibuff + off);
	}
	if (bno == iblock2) {
		ichang2 |= iof;
		hitin2 = 1;
		return (ibuff2 + off);
	}
	if (bno == oblock)
		return (obuff + off);
	if (iof == READ) {
		if (hitin2 == 0) {
			if (ichang2) {
#ifdef CRYPT
                                if(xtflag)
                                        crblock(tperm, ibuff2, CRSIZE, (long)0)
;
#endif
				blkio(iblock2, ibuff2, (ssize_t(*)(int, void *, size_t))write);
			}
			ichang2 = 0;
			iblock2 = bno;
			blkio((short)bno, ibuff2, read);
#ifdef CRYPT
                        if(xtflag)
                                crblock(tperm, ibuff2, CRSIZE, (long)0);
#endif
			hitin2 = 1;
			return (ibuff2 + off);
		}
		hitin2 = 0;
		if (ichanged) {
#ifdef CRYPT
                        if(xtflag)
                                crblock(tperm, ibuff, CRSIZE, (long)0);
#endif
			blkio(iblock, ibuff, (ssize_t(*)(int, void *, size_t))write);
		}
		ichanged = 0;
		iblock = bno;
		blkio((short)bno, ibuff, read);
#ifdef CRYPT
                if(xtflag)
                        crblock(tperm, ibuff, CRSIZE, (long)0);
#endif
		return (ibuff + off);
	}
	if (oblock >= 0) {
#ifdef CRYPT
                if(xtflag) {
                        /*
                         * Encrypt block before writing, so some devious
                         * person can't look at temp file while editing.
                         */
                        p1 = (char *)obuff;
                        p2 = crbuf;
                        n = CRSIZE;
                        while(n--)
                                *p2++ = *p1++;
                        crblock(tperm, crbuf, CRSIZE, (long)0);
                        blkio(oblock, (wchar_t *)crbuf, (ssize_t(*)(int, void *, size_t))write);
                } else
#endif
			blkio(oblock, obuff, (ssize_t(*)(int, void *, size_t))write);
	}
	oblock = bno;
	return (obuff + off);
}

#define INCORB	64
wchar_t incorb[INCORB+1][INCRMT];
#define pagrnd(a)	((void *)(((long)a)&~(BUFBYTES-1)))
int	stilinc = 0;	/* up to here not written yet */

static void blkio(short b, wchar_t *buf, ssize_t(*iofcn)(int, void *, size_t))
{

	if (b < INCORB) {
		if (iofcn == read) {
			copy(buf, pagrnd(incorb[b+1]), BUFBYTES);
			return;
		}
		copy(pagrnd(incorb[b+1]), buf, BUFBYTES);
		if (laste) {
			if (b >= stilinc)
				stilinc = b + 1;
			return;
		}
	} else if (stilinc)
		tflush();
	lseek(tfile, (long) (unsigned) b * BUFBYTES, 0);
	if ((*iofcn)(tfile, (char *) buf, BUFBYTES) != BUFBYTES)
		filioerr(tfname);
}

void tlaste(void)
{

	if (stilinc)
		dirtcnt = 0;
}


void tflush(void)
{
	int i = stilinc;

	stilinc = 0;
	lseek(tfile, (long) 0, 0);
	if (write(tfile, pagrnd(incorb[1]), i * BUFBYTES) != (i * BUFBYTES))
		filioerr(tfname);
}
/*
 * Synchronize the state of the temporary file in case
 * a crash occurs.
 */
void synctmp(void)
{
	register int cnt;
	register line *a;
	register short *bp;
	if (stilinc)
		return;
	if (dol == zero)
		return;
	/*
	 * In theory, we need to encrypt iblock and iblock2 before writing
	 * them out, as well as oblock, but in practice ichanged and ichang2
	 * can never be set, so this isn't really needed.  Likewise, the
	 * code in getblock above for iblock+iblock2 isn't needed.
	 */
	if (ichanged)
		blkio(iblock, ibuff, (ssize_t(*)(int, void *, size_t))write);
	ichanged = 0;
	if (ichang2)
		blkio(iblock2, ibuff2, (ssize_t(*)(int, void *, size_t))write);
	ichang2 = 0;
	if (oblock != -1)
		blkio(oblock, obuff, (ssize_t(*)(int, void *, size_t))write);
	time(&H.Time);
	uid = getuid();
	*zero = (line) H.Time;
	for (a = zero, bp = blocks; a <= dol; a += BUFBYTES / sizeof *a, bp++) {
		if (*bp < 0) {
			tline = (tline + OFFMSK) &~ OFFMSK;
			*bp = ((tline >> OFFBTS) & BLKMSK);
			if (*bp > NMBLKS)
				error(MSGSTR(M_194, " Tmp file too large"), DUMMY_INT);
			tline += INCRMT;
			oblock = *bp + 1;
			bp[1] = -1;
		}
		lseek(tfile, (long) (unsigned) *bp * BUFBYTES, 0);
		cnt = ((dol - a) + 2) * sizeof (line);
		if (cnt > BUFBYTES)
			cnt = BUFBYTES;
		if (write(tfile, (char *) a, cnt) != cnt) {
oops:
			*zero = 0;
			filioerr(tfname);
		}
		*zero = 0;
	}
	flines = lineDOL();
	lseek(tfile, 0l, 0);
	if (write(tfile, (char *) &H, sizeof H) != sizeof H)
		goto oops;
}

void TSYNC(void)
{

	if (dirtcnt > MAXDIRT) {	/* mjm: 12 --> MAXDIRT */
		if (stilinc)
			tflush();
		dirtcnt = 0;
		synctmp();
	}
}

/*
 * Named buffer routines.
 * These are implemented differently than the main buffer.
 * Each named buffer has a chain of blocks in the register file.
 * Each read and write operation is in BUFBYTES chunks.
 * Each block contains roughly
 *		    BUFBYTES - sizeof(short) - sizeof(short)
 * wchar_t of text
 *		[8190 wchar_t with BUFBYTES == 16384 and sizeof(short) == 2],
 * and a previous and next block number.  We also have information
 * about which blocks came from deletes of multiple partial lines,
 * e.g. deleting a sentence or a LISP object.
 *
 * We maintain a free map for the temp file.  To free the blocks
 * in a register we must read the blocks to find how they are chained
 * together.
 *
 * The register routines take an wchar_t as argument but support only
 * 'a'-'z' and '0'-'9' indices.  See cmdreg() in ex_cmds2.c which restricts
 * the range of a register name to ascii, alphabetic characters.
 *
 * BUG: 	The default savind of deleted lines in numbered
 *		buffers may be rather inefficient; it hasn't been profiled.
 */
struct	strreg {
	short	rg_flags;
	short	rg_nleft;
	short	rg_first;
	short	rg_last;
} strregs[('z'-'a'+1) + ('9'-'0'+1)], *strp;

/* sizeof(rbuf) == BUFBYTES, as used in regio() for read and write */
static struct	rbuf {
	short	rb_prev;
	short	rb_next;
	/* sizeof(rb_text) == sizeof(rbuf) - sizeof(first two fields) */
	wchar_t rb_text[INCRMT
		- ((sizeof (short) + sizeof(short)) / sizeof(wchar_t))];
} *rbuf, KILLrbuf, putrbuf, YANKrbuf, regrbuf;
short	rused[256];
static short	rnleft;
static short	rblock;
static short	rnext;
static wchar_t  *rbufcp;

void regio(short b, int (*iofcn)(int, char *, unsigned int))
{

	if (rfile == -1) {
		strcpy(rfname, tfname);
		*(strend(rfname) - 7) = 'R';
		rfile = creat(rfname, 0600);
		if (rfile < 0)
oops:
			filioerr(rfname);
		close(rfile);
		rfile = open(rfname, 2);
		if (rfile < 0)
			goto oops;
	}
	lseek(rfile, (long) b * BUFBYTES, 0);
	if ((*iofcn)(rfile, (char *)rbuf, BUFBYTES) != BUFBYTES)
		goto oops;
	rblock = b;
}


static int REGblk(void)
{
	register int i, j, m;

	for (i = 0; i < sizeof rused / sizeof rused[0]; i++) {
		m = (rused[i] ^ 0177777) & 0177777;
		if (i == 0)
			m &= ~1;
		if (m != 0) {
			j = 0;
			while ((m & 1) == 0)
				j++, m >>= 1;
			rused[i] |= (1 << j);
#ifdef RDEBUG
			printf("allocating block %d\n", i * 16 + j);
#endif
			return (i * 16 + j);
		}
	}
	error(MSGSTR(M_196, "Out of register space (ugh)"), DUMMY_INT);
	return(0);	/*NOTREACHED*/
}

static struct	strreg * mapreg(register wchar_t c)
{
	if (iswupper(c))
		c = towlower(c);
	return (iswdigit(c) ? &strregs[('z'-'a'+1)+(c-'0')] : &strregs[c-'a']);
}

static KILLreg(register wchar_t c)
{
	register struct strreg *sp;

	rbuf = &KILLrbuf;
	sp = mapreg(c);
	rblock = sp->rg_first;
	sp->rg_first = sp->rg_last = 0;
	sp->rg_flags = sp->rg_nleft = 0;
	while (rblock != 0) {
#ifdef RDEBUG
		printf("freeing block %d\n", rblock);
#endif
		rused[rblock / 16] &= ~(1 << (rblock % 16));
		regio(rblock, (int (*)(int, char *, unsigned int))shread);
		rblock = rbuf->rb_next;
	}
}

/*VARARGS*/
static int shread(void)
{
	struct front { short a; short b; };

	if (read(rfile, (char *) rbuf, sizeof (struct front)) == sizeof (struct front))
		return (sizeof (struct rbuf));
	return (0);
}


int	getREG(void);
void putreg(wchar_t c)
{
	register line *odot = dot;
	register line *odol = dol;
	register int cnt;

	deletenone();
	appendnone();
	rbuf = &putrbuf;
	rnleft = 0;
	rblock = 0;
	rnext = mapreg(c)->rg_first;
	if (rnext == 0) {
		if (inopen) {
			splitw++;
			vclean();
			vgoto(WECHO, 0);
		}
		vreg = -1;
		error(MSGSTR(M_197, "Nothing in register %c"), c);
	}
	if (inopen && partreg(c)) {
		if (!FIXUNDO) {
			splitw++; vclean(); vgoto(WECHO, 0); vreg = -1;
			error(MSGSTR(M_198, "Can't put partial line inside macro"), DUMMY_INT);
		}
		squish();
		addr1 = addr2 = dol;
	}
	cnt = append(getREG, addr2);
	if (inopen && partreg(c)) {
		unddol = dol;
		dol = odol;
		dot = odot;
		pragged((short)0);
	}
	killcnt(cnt);
	notecnt = cnt;
}

int partreg(wchar_t c)
{

	return (mapreg(c)->rg_flags);
}

void notpart(register wchar_t c)
{

	if (c)
		mapreg(c)->rg_flags = 0;
}

static int getREG(void)
{
	register wchar_t *lp = linebuf;
	register wchar_t c;

	for (;;) {
		if (rnleft == 0) {
			if (rnext == 0)
				return (EOF);
			regio(rnext, (int(*)(int,char *,unsigned int))read);
			rnext = rbuf->rb_next;
			rbufcp = rbuf->rb_text;
			rnleft = sizeof rbuf->rb_text / sizeof(wchar_t);
		}
		c = *rbufcp;
		if (c == 0)
			return (EOF);
		rbufcp++, --rnleft;
		if (c == '\n') {
			*lp++ = 0;
			return (0);
		}
		*lp++ = c;
	}
}

void YANKreg(register wchar_t c)
{
	register line *addr;
	register struct strreg *sp;
	wchar_t savelb[LBSIZE];

	if (iswdigit(c))
		kshift();
	if (iswlower(c))
		KILLreg(c);
	strp = sp = mapreg(c);
	sp->rg_flags = inopen && cursor && wcursor;
	rbuf = &YANKrbuf;
	if (sp->rg_last) {
		regio(sp->rg_last, (int(*)(int,char *,unsigned int))read);
		rnleft = sp->rg_nleft;
		rbufcp = &rbuf->rb_text[sizeof(rbuf->rb_text) / sizeof(wchar_t)
					 - rnleft];
	} else {
		rblock = 0;
		rnleft = 0;
	}
	CP(savelb,linebuf);
	for (addr = addr1; addr <= addr2; addr++) {
		getline(*addr);
		if (sp->rg_flags) {
			if (addr == addr2)
				*wcursor = 0;
			if (addr == addr1)
				wcscpy(linebuf, cursor);
		}
		YANKline();
	}
	rbflush();
	if (strcmp(Command, "delete") != 0)
		killed();
	CP(linebuf,savelb);
}

static void kshift(void)
{
	register int i;

	KILLreg('9');
	for (i = '8'; i >= '0'; i--)
		copy(mapreg(i+1), mapreg(i), sizeof (struct strreg));
}

void YANKline(void)
{
	register wchar_t *lp = linebuf;
	register struct rbuf *rp = rbuf;
	register int c;

	do {
		c = *lp++;
		if (c == 0)
			c = '\n';
		if (rnleft == 0) {
			rp->rb_next = REGblk();
			rbflush();
			rblock = rp->rb_next;
			rp->rb_next = 0;
			rp->rb_prev = rblock;
			rnleft = sizeof rp->rb_text / sizeof (wchar_t);
			rbufcp = rp->rb_text;
		}
		*rbufcp++ = c;
		--rnleft;
	} while (c != '\n');
	if (rnleft)
		*rbufcp = 0;
}

static void rbflush(void)
{
	register struct strreg *sp = strp;

	if (rblock == 0)
		return;
	regio(rblock, (int(*)(int,char *,unsigned int))write);
	if (sp->rg_first == 0)
		sp->rg_first = rblock;
	sp->rg_last = rblock;
	sp->rg_nleft = rnleft;
}

/* Register c to wchar_t buffer buf of size buflen wchar_t's */
void regbuf(wchar_t c, wchar_t  *buf, int buflen)
{
	register wchar_t *p, *lp;

	rbuf = &regrbuf;
	rnleft = 0;
	rblock = 0;
	rnext = mapreg(c)->rg_first;
	if (rnext==0) {
		*buf = 0;
		error(MSGSTR(M_199, "Nothing in register %c"),c);
	}
	p = buf;
	while (getREG()==0) {
		for (lp=linebuf; *lp;) {
			if (p >= &buf[buflen])
				error(MSGSTR(M_200, "The register is greater than %d characters."), LBSIZE);
			*p++ = *lp++;
		}
		*p++ = '\n';
	}
	if (partreg(c)) p--;
	*p = '\0';
	getDOT();
}

/*
 * Encryption routines.  These are essentially unmodified from ed.
 */

#ifdef CRYPT
/*
 * crblock: encrypt/decrypt a block of text.
 * buf is the buffer through which the text is both input and
 * output. nchar is the size of the buffer. permp is a work
 * buffer, and startn is the beginning of a sequence.
 */
crblock(permp, buf, nchar, startn)
char *permp;
char *buf;
int nchar;
long startn;
{
        register char *p1;
        int n1;
        int n2;
        register char *t1, *t2, *t3;

        t1 = permp;
        t2 = &permp[256];
        t3 = &permp[512];

        n1 = startn&0377;
        n2 = (startn>>8)&0377;
        p1 = buf;
        while(nchar--) {
                *p1 = t2[(t3[(t1[(*p1+n1)&0377]+n2)&0377]-n2)&0377]-n1;
                n1++;
                if(n1==256){
                        n1 = 0;
                        n2++;
                        if(n2==256) n2 = 0;
                }
                p1++;
        }
}

/*
 * makekey: initialize buffers based on user key a.
 */
makekey(a, b)
char *a, *b;
{
       register int i;
        time_t t;
        char temp[KSIZE + 1];

        for(i = 0; i < KSIZE; i++)
                temp[i] = *a++;
        time(&t);
        t += getpid();
        for(i = 0; i < 4; i++)
                temp[i] ^= (t>>(8*i))&0377;
        crinit(temp, b);
}

/*
 * crinit: besides initializing the encryption machine, this routine
 * returns 0 if the key is null, and 1 if it is non-null.
 */
crinit(keyp, permp)
char    *keyp, *permp;
{
       register char *t1, *t2, *t3;
        register i;
        int ic, k, temp;
        unsigned random;
        char buf[13];
        long seed;

        t1 = permp;
        t2 = &permp[256];
        t3 = &permp[512];
        if(*keyp == 0)
                return(0);
        strncpy(buf, keyp, 8);
        while (*keyp)
                *keyp++ = '\0';

        buf[8] = buf[0];
        buf[9] = buf[1];
        domakekey(buf);

        seed = 123;
        for (i=0; i<13; i++)
                seed = seed*buf[i] + i;
        for(i=0;i<256;i++){
                t1[i] = i;
                t3[i] = 0;
        }
        for(i=0; i<256; i++) {
                seed = 5*seed + buf[i%13];
                random = seed % 65521;
                k = 256-1 - i;
                ic = (random&0377) % (k+1);
                random >>= 8;
                temp = t1[k];
                t1[k] = t1[ic];
                t1[ic] = temp;
                if(t3[k]!=0) continue;
                ic = (random&0377) % k;
                while(t3[ic]!=0) ic = (ic+1) % k;
                t3[k] = ic;
                t3[ic] = k;
        }
        for(i=0; i<256; i++)
                t2[t1[i]&0377] = i;
        return(1);
}

/*
 * domakekey: the following is the major nonportable part of the encryption
 * mechanism. A 10 character key is supplied in buffer.
 * This string is fed to makekey (an external program) which
 * responds with a 13 character result. This result is placed
 * in buffer.
 */
domakekey(buffer)
char *buffer;
{
       int pf[2];

        if (pipe(pf)<0)
                pf[0] = pf[1] = -1;
        if (fork()==0) {
                close(0);
                close(1);
                dup(pf[0]);
                dup(pf[1]);
                execl("/usr/lbin/makekey", "-", 0);
                exit(1);
        }
        write(pf[1], buffer, 10);
        if (wait((int *)NULL)==-1 || read(pf[0], buffer, 13)!=13)
                error("crypt: cannot generate key", DUMMY_INT);
        close(pf[0]);
        close(pf[1]);
        /* end of nonportable part */
}
#endif

