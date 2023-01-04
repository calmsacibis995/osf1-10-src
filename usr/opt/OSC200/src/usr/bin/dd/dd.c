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
static char	*sccsid = "@(#)$RCSfile: dd.c,v $ $Revision: 4.3.12.9 $ (DEC) $Date: 1993/11/09 23:44:26 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/file.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <ctype.h>
#include <locale.h>
#include <nl_types.h>
#include <errno.h>

#ifdef   THREADS 
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <pthread.h>
#endif   /* THREADS */

#include "dd_msg.h"

#define	LOWCASE	01
#define	UPPCASE	02
#define	SWAB	04
#define NERR	010
#define SYNC	020
#define SPARSE  040
#define TRUE  	1  
#define FALSE  	0 

#define TTYNAME "/dev/tty" 

#ifdef   THREADS 
#define INBUFS   8
#define ONBUFS   8
#endif   /* THREADS */

#define MSGSTR(num, str)	catgets(catd, MS_DD, num, str)
nl_catd catd;

unsigned int	ibs	= 512;
unsigned int	obs	= 512;
unsigned int	bs;
unsigned int	cbs;
int	ibc;    /* changed from unsigned int */
unsigned int	obc;
unsigned int	cbc;

int	fflag;
int	cflag;
int	nifr;
int	nipr;
int	nofr;
int	nopr;
int	ntrunc;
int	ibf;
int	obf;
int	skip;
int	seekn;
int	count;
int	nspace;
int	files	 = 1;
int	pending;

FILE    *Rtty, 
	*Wtty;

char	*string;
char	*ifile;
char	*ofile;
char	*ibuf;
char	*obuf;
char	*op;
char    *eommsg; 
char 	str[BUFSIZ];   


#ifdef   THREADS 
/*** 
 ***   The following variables are used for the thread implementation of dd. 
 ***/

int     inumbufs;         /* number of input buffers used           */ 
int     onumbufs;         /* number of output buffers used          */ 
int     cur_write;        /* index to current output buffer         */
int     cur_read;         /* index to current input buffer          */ 
int     first_round;      /* have writes not yet wrapped around?    */
int     async_nipr = 0;   /* actual number of partial records read  */ 
int     async_nifr = 0;   /* actual number of full records read     */

struct shio {
        char     *buf;    /* buffer to be read into/written from    */
        int      size;    /* block size to be read/written          */
        int      res;     /* result of read/write operation         */
        int      err;     /* error of read/write operation          */
} *ishared, *oshared;


pthread_cond_t     *cond_issue_read;
pthread_cond_t     *cond_done_read;
pthread_cond_t     *cond_issue_write;
pthread_cond_t     *cond_done_write;
pthread_mutex_t    *mutex_issue_read;
pthread_mutex_t    *mutex_done_read;
pthread_mutex_t    *mutex_issue_write;
pthread_mutex_t    *mutex_done_write;
int     *issue_read;      /* mutex locks to enable reading          */ 
int     *done_read;       /* mutex locks to inform read completion  */
int     *issue_write;     /* mutex locks to enable writing          */
int     *done_write;      /* mutex locks to inform write completion */
pthread_t        r_slave; /* read slave - does continuous reads     */ 
pthread_t        w_slave; /* write slave - does continuous writes   */
#endif   /* THREADS */


unsigned char etoa[] =	/* EBCDIC to ASCII conversion table */
{
	0000,0001,0002,0003,0234,0011,0206,0177,
	0227,0215,0216,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0235,0205,0010,0207,
	0030,0031,0222,0217,0034,0035,0036,0037,
	0200,0201,0202,0203,0204,0012,0027,0033,
	0210,0211,0212,0213,0214,0005,0006,0007,
	0220,0221,0026,0223,0224,0225,0226,0004,
	0230,0231,0232,0233,0024,0025,0236,0032,
	0040,0240,0241,0242,0243,0244,0245,0246,
	0247,0250,0325,0056,0074,0050,0053,0174,
	0046,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0041,0044,0052,0051,0073,0176,
	0055,0057,0262,0263,0264,0265,0266,0267,
	0270,0271,0313,0054,0045,0137,0076,0077,
	0272,0273,0274,0275,0276,0277,0300,0301,
	0302,0140,0072,0043,0100,0047,0075,0042,
	0303,0141,0142,0143,0144,0145,0146,0147,
	0150,0151,0304,0305,0306,0307,0310,0311,
	0312,0152,0153,0154,0155,0156,0157,0160,
	0161,0162,0136,0314,0315,0316,0317,0320,
	0321,0345,0163,0164,0165,0166,0167,0170,
	0171,0172,0322,0323,0324,0133,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0135,0346,0347,
	0173,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0350,0351,0352,0353,0354,0355,
	0175,0112,0113,0114,0115,0116,0117,0120,
	0121,0122,0356,0357,0360,0361,0362,0363,
	0134,0237,0123,0124,0125,0126,0127,0130,
	0131,0132,0364,0365,0366,0367,0370,0371,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0372,0373,0374,0375,0376,0377,
};
unsigned char atoe[] =	/* ASCII to EBCDIC conversion table */
{
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0132,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0255,0340,0275,0232,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0117,0320,0137,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0152,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0112,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0241,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};
unsigned char atoibm[] =	/* slightly different ASCII to EBCDIC table */
{
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0132,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0255,0340,0275,0137,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0117,0320,0241,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0232,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};


main(argc, argv)
int	argc;
char	**argv;
{
	char  *seek_buf;
	int (*conv)();
	register char *ip;
	register c;
	int ebcdic(), ibm(), ascii(), null(), cnull(), block(), unblock();
	void term();
#ifdef   THREADS 
	void async_initialize(), async_error(), async_term();
	int  async_read();
#endif   /* THREADS */
	int end_of_medium_while_reading();
	int a;
	struct stat stbuf;

	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_DD, NL_CAT_LOCALE);

	conv = null;
	for(c=1; c<argc; c++) {
		string = argv[c];
		if(match("ibs=")) {
			ibs = number(0, 0);
			continue;
		}
		if(match("obs=")) {
			obs = number(0, 0);
			continue;
		}
		if(match("cbs=")) {
			cbs = number(0, 0);
			continue;
		}
		if (match("bs=")) {
			bs = number(0, 0);
			continue;
		}
		if(match("if=")) {
			ifile = string;
			continue;
		}
		if(match("of=")) {
			ofile = string;
			continue;
		}
		if(match("skip=")) {
			skip = number(0, 0);
			continue;
		}
		if(match("seek=")) {
			seekn = number(0, 0);
			continue;
		}
		if(match("count=")) {
			count = number(0, 0);
			continue;
		}
		if(match("files=")) {
			files = number(0, 0);
			continue;
		}
		if(match("conv=")) {
		cloop:
			if(match(","))
				goto cloop;
			if(*string == '\0')
				continue;
			if(match("ebcdic")) {
				conv = ebcdic;
				goto cloop;
			}
			if(match("ibm")) {
				conv = ibm;
				goto cloop;
			}
			if(match("ascii")) {
				conv = ascii;
				goto cloop;
			}
			if(match("block")) {
				conv = block;
				goto cloop;
			}
			if(match("unblock")) {
				conv = unblock;
				goto cloop;
			}
			if(match("lcase")) {
				cflag |= LOWCASE;
				goto cloop;
			}
			if(match("ucase")) {
				cflag |= UPPCASE;
				goto cloop;
			}
			if(match("swab")) {
				cflag |= SWAB;
				goto cloop;
			}
			if(match("noerror")) {
				cflag |= NERR;
				goto cloop;
			}
			if(match("sync")) {
				cflag |= SYNC;
				goto cloop;
			}
			if(match("sparse")) {
				cflag |= SPARSE;
				goto cloop;
			}
		}
		fprintf(stderr, MSGSTR(EARG, "dd: bad arg: %s\n"), string);
		exit(1);
	}
	if(conv == null && cflag&(LOWCASE|UPPCASE))
		conv = cnull;

   	eommsg = MSGSTR(NEXTVOL,  
                        "Insert next volume and press RETURN key. [q] ");

	ibf = ifile ? open(ifile, O_RDONLY) : dup(0);
	if(ibf < 0) {
		perror(ifile);
		exit(1);
	}

	if (ifile && ofile) {
		struct stat sbuf_in, sbuf_out;

		if (stat(ifile, &sbuf_in) == -1 ) {
			perror(ifile);
			exit(1);
		}
		/*
		   If output file already exists AND it's not a special file AND
		   it's inode/device numbers are the same - don't stomp on it.
		*/
		if (stat(ofile, &sbuf_out) == 0 &&
		     !(sbuf_in.st_mode & (S_IFCHR | S_IFBLK)) &&
		     sbuf_in.st_dev == sbuf_out.st_dev &&
		     sbuf_in.st_ino == sbuf_out.st_ino ) {
			fprintf(stderr,
			  MSGSTR(ESTOMP, "dd: can't copy file onto itself\n"));
			exit(1);
		}
	}

	obf = ofile ? open(ofile, O_WRONLY|O_CREAT|O_TRUNC, 0666) : dup(1);

	if(obf < 0) {
		perror(ofile);
		exit(1);
	}

        if(cflag&SPARSE) { 
		if (!fstat (obf, &stbuf) && !(stbuf.st_mode & S_IFREG))  
			cflag &= ~SPARSE;
	}

	if (bs)
		ibs = obs = bs;
	if (ibs == obs && conv == null)
		fflag++;

	if (!ifile)
		fflag = 0;	/* reading a pipe requires buffering to */
				/* guarantee requested blocksize */

	if(ibs == 0 || obs == 0) {
		fprintf(stderr,
			MSGSTR(EBLKSIZ, "dd: block sizes cannot be zero\n"));
		exit(1);
	}

#ifdef   THREADS 
	async_initialize();
#else
	ibuf = (char *)malloc(ibs);
	if (fflag)
		obuf = ibuf;
	else
		obuf = (char *)malloc(obs);
	if(ibuf == (char *)NULL || obuf == (char *)NULL) {
		fprintf(stderr, MSGSTR(ENOMRY, "dd: not enough memory\n"));
		exit(1);
	}
#endif   /* THREADS */


	ibc = 0;
	obc = 0;
	cbc = 0;
	op = obuf;

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, term);
	while(skip) {
#ifdef   THREADS 
		if (async_read() == -1) {
			async_error(MSGSTR(ESKIP, "dd skip error"));

#else
		if (read(ibf, ibuf, ibs) == -1) {
			perror(MSGSTR(ESKIP, "dd skip error"));
#endif   /* THREADS */
			stats();
			if((cflag&NERR) == 0)
				exit(2);
		}
		skip--;
	}

	if (seekn) {
		if (lseek(obf, ((off_t)obs * seekn), 1) == -1) {
			perror(MSGSTR(ESEEK, "dd: unable to perform seek\n"));
			if((cflag&NERR) == 0)
				exit(1);
		}
	}

loop:
	if(ibc-- == 0) {
		ibc = 0;
		if(count==0 || nifr+nipr!=count) {
#ifdef   THREADS 
			ibc = async_read();
#else
			if(cflag&(NERR|SYNC))
			for(ip=ibuf+ibs; ip>ibuf;)
				*--ip = 0;
			ibc = read(ibf, ibuf, ibs);
#endif   /* THREADS */
		}
		if(ibc == -1) {
#ifdef   THREADS 
			async_error(MSGSTR(EREAD, "dd read error"));
#else
			perror(MSGSTR(EREAD, "dd read error"));
#endif   /* THREADS */
			if((cflag&NERR) == 0) {
				flsh();
#ifdef   THREADS 
				async_term(2);
#else
				term(2);
#endif   /* THREADS */
			}
			ibc = 0;
			for(c=0; c<ibs; c++)
				if(ibuf[c] != 0)
					ibc = c;
			stats();
		}
		if(ibc == 0 && --files<=0) {
			flsh();
#ifdef   THREADS 
			async_term(0);
#else
			term(0);
#endif   /* THREADS */
		}
		if(ibc != ibs) {
			nipr++;
			if(cflag&SYNC)
				ibc = ibs;
		} else
			nifr++;
		ip = ibuf;
		c = ibc >> 1;
		if(cflag&SWAB && c)
		do {
			a = *ip++;
			ip[-1] = *ip;
			*ip++ = a;
		} while(--c);
		ip = ibuf;
		if (fflag) {
			obc = ibc;
			flsh();
			ibc = 0;
		}
		goto loop;
	}
	c = 0;
	c |= *ip++;
	c &= 0377;
	(*conv)(c);
	goto loop;
} /* End of while(skip) */

flsh()
{
#ifdef   THREADS 
	void    async_write();
	if(obc) {
		async_write (obc);
		obc = 0;
	}
#else
	register c;

	if(obc) {
		do_write (obf, obuf, obc);
		obc = 0;
	}
#endif   /* THREADS */
}

match(s)
char *s;
{
	register char *cs;

	cs = string;
	while(*cs++ == *s)
		if(*s++ == '\0')
			goto true;
	if(*s != '\0')
		return(0);

true:
	cs--;
	string = cs;
	return(1);
}

/* Pass 0 as arguments for calls outside the function (non-recursive calls) */
number(err, inv_char)
int  err, inv_char;
{
	register char *cs;
	register n;

	cs = string;
	n = 0;
	while(*cs >= '0' && *cs <= '9')
		n = n*10 + *cs++ - '0';
	for(;;)
	switch(*cs++) {

	case 'K':
	case 'k':
		n *= 1024;
		continue;

	case 'W':
	case 'w':
		n *= 2;
		continue;

	case 'B':
	case 'b':
		n *= 512;
		continue;

	case '*':
	case 'X':
	case 'x':
		string = cs;
		n *= number(err, inv_char);

	case '\0':
		if (err)
			fprintf(stderr, MSGSTR(EINVCHAR, "dd: invalid character: %c\n"),inv_char);
		return(n);

	default:
		inv_char = *(cs-1); 
		err = 1;
	}
	/* never gets here */
}

cnull(cc)
{
	register c;

	c = cc;
	if(cflag&UPPCASE && islower(c))
		c = toupper(c);
	if(cflag&LOWCASE && isupper(c))
		c = tolower(c);
	null(c);
}

null(c)
{

	*op = c;
	op++;
	if(++obc >= obs) {
		flsh();
		op = obuf;
	}
}

ascii(cc)
{
	register c;

	c = etoa[cc] & 0377;
	if(cbs == 0) {
		cnull(c);
		return;
	}
	if(c == ' ')
		nspace++;
	else {
		while(nspace > 0) {
			null(' ');
			nspace--;
		}
		cnull(c);
	}

	if(++cbc >= cbs) {
		null('\n');
		cbc = 0;
		nspace = 0;
	}
}

unblock(cc)
{
	register c;

	c = cc & 0377;
	if(cbs == 0) {
		cnull(c);
		return;
	}
	if(c == ' ')
		nspace++;
	else {
		while(nspace > 0) {
			null(' ');
			nspace--;
		}
		cnull(c);
	}

	if(++cbc >= cbs) {
		null('\n');
		cbc = 0;
		nspace = 0;
	}
}

ebcdic(cc)
{
	register c;

	c = cc;
	if(cflag&UPPCASE && islower(c))
		c = toupper(c);
	if(cflag&LOWCASE && isupper(c))
		c = tolower(c);
	c = atoe[c] & 0377;
	if(cbs == 0) {
		null(c);
		return;
	}
	if(cc == '\n') {
		while(cbc < cbs) {
			null(atoe[' ']);
			cbc++;
		}
		cbc = 0;
		return;
	}
	if(cbc == cbs)
		ntrunc++;
	cbc++;
	if(cbc <= cbs)
		null(c);
}

ibm(cc)
{
	register c;

	c = cc;
	if(cflag&UPPCASE && islower(c))
		c = toupper(c);
	if(cflag&LOWCASE && isupper(c))
		c = tolower(c);
	c = atoibm[c] & 0377;
	if(cbs == 0) {
		null(c);
		return;
	}
	if(cc == '\n') {
		while(cbc < cbs) {
			null(atoibm[' ']);
			cbc++;
		}
		cbc = 0;
		return;
	}
	if(cbc == cbs)
		ntrunc++;
	cbc++;
	if(cbc <= cbs)
		null(c);
}

block(cc)
{
	register c;

	c = cc;
	if(cflag&UPPCASE && islower(c))
		c = toupper(c);
	if(cflag&LOWCASE && isupper(c))
		c = tolower(c);
	c &= 0377;
	if(cbs == 0) {
		null(c);
		return;
	}
	if(cc == '\n') {
		while(cbc < cbs) {
			null(' ');
			cbc++;
		}
		cbc = 0;
		return;
	}
	if(cbc == cbs)
		ntrunc++;
	cbc++;
	if(cbc <= cbs)
		null(c);
}

void
term(c)
{
        if(cflag&SPARSE) 		/* in case of a pending seek */ 
		write_sparse (1, obf, (char *)0, 0);
	stats();
        if (close(obf) == -1) {
                perror (MSGSTR(ECLOSE, "dd close error"));
                exit(2);
        }
	exit(c);
}

stats()
{

#ifdef   THREADS 
        if (fflag)
	   fprintf(stderr, MSGSTR(RECSIN, "%u+%u records in\n"), nifr, nipr);
        else
	   fprintf(stderr, MSGSTR(RECSIN, "%u+%u records in\n"), 
	      async_nifr, async_nipr);
#else
	fprintf(stderr, MSGSTR(RECSIN, "%u+%u records in\n"), nifr, nipr);
#endif   /* THREADS */

	fprintf(stderr, MSGSTR(RECSOUT, "%u+%u records out\n"), nofr, nopr);
	if(ntrunc)
		fprintf(stderr,
			MSGSTR(TRUNRECS, "%u truncated records\n"), ntrunc);
}


/*** 
 ***   Seeks and/or writes when the sparse option is specified. 
 *** 
 ***   When this function is called with a sparse buffer, it accumulates
 ***   the amount pending to seek and the statistics.  When it is called 
 ***   with a non-sparse buffer, it seeks by the accumulated amount, 
 ***   writes the buffer and updates the statistics now that the seek is
 ***   committed.
 ***   This function needs to be called after the last buffer is processed
 ***   to handle the case of a pending seek.
 ***/
int
write_sparse (int done, int filedes, char *buffer, int nbytes)
{
	int		sparse;		/* Is buffer sparse? */
	char		dummy = 0;	/* Byte written if last buf sparse */
	static int	inc_nofr = 0;	/* Accumulate stats */
	static int	inc_nopr = 0;
	int		i, c;

	if (done && pending) {
		if (lseek (filedes, (off_t)(pending-1), SEEK_CUR) == -1) {
			perror(MSGSTR(ESPARSE, "dd seek error"));
			stats();
			exit(2);
		}
		if ((c = write (filedes, &dummy, 1)) != 1) {
		    if (c == -1)
			if (!end_of_medium_while_writing())
			    perror(MSGSTR(EWRITE, "dd write error"));
			else { 
		            c = write (filedes, &dummy, 1);
			    goto normal_done;
			}
		    else
			fprintf(stderr, MSGSTR(EPART, "dd: unable to write complete record\n"));
		    stats();
		    exit(2);
		}
		normal_done:
		    nofr += inc_nofr;	/* Write committed seek: update */ 
		    nopr += inc_nopr;	/* stats */
		    pending = 0;
		    return (0);
	}

	sparse = 1;			/* Is buffer sparse ? */
	for (i=0; i < nbytes; i++)
		if (buffer[i] != 0) {
			sparse = 0;
			break;
		}

	if (sparse) {
		pending += nbytes;
		if(nbytes == obs)	/* Do not update stats until */
			inc_nofr++;	/* write commits seek */
		else
			inc_nopr++;
		return (0);

	}
	
	if (pending) { 
		if (lseek (filedes, pending, SEEK_CUR) == -1) {
			perror(MSGSTR(ESPARSE, "dd seek error"));
			stats();
			exit(2);
		}
		pending = 0;
	}

	c = write (filedes, buffer, nbytes);
        if(c > 0) {
		nofr += inc_nofr;	/* Write committed seek: update */ 
		nopr += inc_nopr;	/* stats */
		inc_nofr = 0;
		inc_nopr = 0;
	}
	return (c);
} 
  /*** 
   ***   Writes buffer and updates statistics. 
   ***/ 
do_write (int filedes, char *buffer, int nbytes) 
{
	int    c;
	if(cflag&SPARSE) {
		c = write_sparse (0, obf, buffer, nbytes);
		if (pending)		/* No write occurred */
			return;
	}
	else
		c = write (obf, buffer, nbytes);

	if (c < 0) {   /* end of medium upon writing */
		if ( errno == ENOSPC || errno == ENXIO ) {  
			fprintf(stderr, MSGSTR(EOM,"\007Reached end of medium; Please wait for file closing.\n"));

			if (close(filedes) == -1) {  /* close output file */
      				perror (MSGSTR(ECLOSE, "dd close error"));
      				exit(2);
			}
			if (!Rtty && !(Rtty = fopen(TTYNAME, "r"))) {
                		fprintf(stderr, MSGSTR(NOPROMPT,
                        		"Cannot prompt (can't open %s): "), TTYNAME);
                		perror("");
                		exit(2);
        		}
 			fprintf(stderr, eommsg);
                	fgets(str, sizeof(str), Rtty);
                		switch (*str) {
               			case '\n':
                       			break;
               			case 'q':
                       			exit(2);
               			default:
                       			break;
               			}

			obf = ofile ? open(ofile, O_WRONLY|O_CREAT|O_TRUNC, 0666) : dup(1);
			c = write (obf, buffer, nbytes);
		}
		else {
			perror (MSGSTR(EWRITE, "dd write error"));
			stats();
			exit(2);
		}
	}

	if ((c >= 0) && (c != nbytes)) {
		nopr++;
		fprintf(stderr, MSGSTR(EPART, "dd: unable to write complete record\n"));
		stats();
		exit(2);
	}

	if(nbytes == obs)
		nofr++;

	else 
		nopr++;
}

int
end_of_medium_while_reading()
{
		if ( files-- > 1 ) {

			fprintf(stderr, MSGSTR(EOM,"\007Reached end of medium; Please wait for file closing\n"));

			if (close(ibf) == -1) {  /* close input file */
      				perror (MSGSTR(ECLOSE, "dd close error"));
      				exit(2);
			}
			if (!Rtty && !(Rtty = fopen(TTYNAME, "r"))) {
                		fprintf(stderr, MSGSTR(NOPROMPT,
                        		"Cannot prompt (can't open %s): "), TTYNAME);
                		perror("");
                		exit(2);
        		}
 			fprintf(stderr, eommsg); /* tell user to insert new medium */
                	fgets(str, sizeof(str), Rtty);
                		switch (*str) {
               			case '\n':
                       			break;
               			case 'q':
                       			exit(2);
               			default:
                       			break;
               			}

			ibf = ifile ? open(ifile, O_RDONLY) : dup(0);
			return(TRUE);

		}
		else return(FALSE);
}  /* End of end_of_medium_while_reading function */

int
end_of_medium_while_writing()
{
		if ( errno == ENOSPC || errno == ENXIO ) {
			fprintf(stderr, MSGSTR(EOM,"\007Reached end of medium; Please wait for file closing.\n"));

			if (close(obf) == -1) {  /* close output file */
      				perror (MSGSTR(ECLOSE, "dd close error"));
      				exit(2);
			}
			if (!Rtty && !(Rtty = fopen(TTYNAME, "r"))) {
                		fprintf(stderr, MSGSTR(NOPROMPT,
                        		"Cannot prompt (can't open %s): "), TTYNAME);
                		perror("");
                		exit(2);
        		}
 			fprintf(stderr, eommsg);
                	fgets(str, sizeof(str), Rtty);
                		switch (*str) {
               			case '\n':
                       			break;
               			case 'q':
                       			exit(2);
               			default:
                       			break;
               			}

			obf = ofile ? open(ofile, O_WRONLY|O_CREAT|O_TRUNC, 0666) : dup(1);
			return(TRUE);
		}
		return(FALSE);
}
#ifdef   THREADS 
/***
 ***   Following is the code that uses threads to increase the performance
 ***   of dd.  
 ***
 ***   If ibs is equal to obs and no conversions are needed then 2 threads 
 ***   are used: the main thread does reads, and a "write_slave" thread 
 ***   does the writes.  Otherwise, 3 threads are used: the main thread 
 ***   does the processing, a "write_slave" thread does the writes and a 
 ***   "read_slave" thread does the reads.  These strategies guarantee the 
 ***   sequential ordering of both reads and writes needed for sequential 
 ***   devices such as tapes. 
 ***/

/*** 
 ***   Creates a synchronization object "lock" that is initially unlocked.
 ***/
void async_init(mutex, cond, lock)
   pthread_mutex_t  *mutex;
   pthread_cond_t  *cond;
   int  *lock;
{
   if (pthread_mutex_init(mutex, pthread_mutexattr_default) == -1) {
      perror(MSGSTR(ESYNCINIT, "dd: unable to initialize synchronization object\n"));
      exit(1);
   }
   if (pthread_cond_init(cond, pthread_condattr_default) == -1) {
      perror(MSGSTR(ESYNCINIT, "dd: unable to initialize synchronization object\n"));
      exit(1);
   }

   *lock = 0;
}


/*** 
 ***   Locks synchronization object "lock." 
 ***/
void async_lock(mutex, cond, lock)
   pthread_mutex_t  *mutex;
   pthread_cond_t  *cond;
   int  *lock;
{
   if (pthread_mutex_lock (mutex) == -1) {
      perror(MSGSTR(ESYNCLOCK, "dd: unable to lock synchronization object\n"));
      exit(1);
   }

   while (*lock) {
      if (pthread_cond_wait(cond, mutex) == -1) {
         perror(MSGSTR(ESYNCLOCK, "dd: unable to lock synchronization object\n"));
         exit(1);
      }
   }

   *lock = 1;

   if (pthread_mutex_unlock (mutex) == -1) {
      perror(MSGSTR(ESYNCLOCK, "dd: unable to lock synchronization object\n"));
      exit(1);
   }
}
 

/*** 
 ***   Attempts to lock synchronization object "lock."  If successful, 
 ***   it returns 1.  Otherwise, it returns 0. 
 ***/
int async_trylock(mutex, cond, lock)
   pthread_mutex_t  *mutex;
   pthread_cond_t  *cond;
   int  *lock;
{
   int  ret;

   ret = 0;
   if (pthread_mutex_lock (mutex) == -1) {
      perror(MSGSTR(ESYNCTRYLOCK, "dd: unable to trylock synchronization object\n"));
      exit(1);
   }

   if (*lock == 0) {
      *lock = 1;
      ret = 1;
   }

   if (pthread_mutex_unlock (mutex) == -1) {
      perror(MSGSTR(ESYNCTRYLOCK, "dd: unable to trylock synchronization object\n"));
      exit(1);
   }
   return (ret);
}


/*** 
 ***   Unlocks synchronization object "lock." 
 ***/
void async_unlock(mutex, cond, lock)
   pthread_mutex_t  *mutex;
   pthread_cond_t  *cond;
   int  *lock;
{
   if (pthread_mutex_lock (mutex) == -1) {
      perror(MSGSTR(ESYNCUNLOCK, "dd: unable to unlock synchronization object\n"));
      exit(1);
   }

   *lock = 0;

   if (pthread_cond_signal(cond) == -1) {
      perror(MSGSTR(ESYNCUNLOCK, "dd: unable to unlock synchronization object\n"));
      exit(1);
   }

   if (pthread_mutex_unlock (mutex) == -1) {
      perror(MSGSTR(ESYNCUNLOCK, "dd: unable to unlock synchronization object\n"));
      exit(1);
   }
}


/*** 
 ***   This routine prints an error message for read errors ONLY. It prints
 ***   "string" followed by a colon, followed by the appropriate perror
 ***   message.
 ***/
void async_error(string)
   char  *string;
{
   if (fflag)
      errno = oshared[cur_write].err;
   else
      errno = ishared[cur_read].err;

   perror (string);
}




/***
 ***   This thread never terminates.  For each write buffer, it waits until 
 ***   the main thread informs it that the buffer has data on it to be 
 ***   written.  The thread then performs a write, processes it, and 
 ***   informs the main thread that the buffer is free to be reused. 
 ***
 ***   The parameter "dummy" is not used.  It is needed since the
 ***   pthread_create call expects 4 parameters.
 ***/
void write_slave(dummy)
   int  *dummy;
{
   int  next_write;

   next_write = 0;
   while (1) {
      async_lock (&mutex_issue_write[next_write], &cond_issue_write[next_write], &issue_write[next_write]);
      signal(SIGPIPE,SIG_DFL); /* Terminate for broken pipe */
      
      do_write (obf, oshared[next_write].buf, oshared[next_write].size);

      async_unlock (&mutex_done_write[next_write], &cond_done_write[next_write], &done_write[next_write]);
      next_write = (next_write+1) % onumbufs;
   }
}


/***
 ***   This thread never terminates.  For each read buffer, it waits until 
 ***   the main thread informs it that the buffer is free to be reused.  
 ***   The thread then performs a read, gathers  statistics on the read 
 ***   results, and informs the main thread that the read has completed. 
 ***
 ***   The parameter "dummy" is not used.  It is needed since the
 ***   pthread_create call expects 4 parameters.
 ***/
void read_slave(dummy)
   int  *dummy;
{
   int  next_read;
   int  result, c;

   next_read = 0;
   while (1) {
      async_lock (&mutex_issue_read[next_read], &cond_issue_read[next_read], &issue_read[next_read]);
      if (cflag&(NERR|SYNC))
         bzero (ishared[next_read].buf, ibs);
      ishared[next_read].res = read(ibf, ishared[next_read].buf, ibs); 
      ishared[next_read].err = errno; 

      result = ishared[next_read].res; /* Actual read info is collected  */
      if (result == -1)                /* to insure correctness of stats */
         if(cflag&NERR)                /* printed out when interrupts    */
            for(c=0; c<ibs; c++)       /* and write errors occur         */ 
               if(ibuf[c] != 0)
                  result = c;
      if (result == 0) {      /* End of tape file - maybe  */
	if (end_of_medium_while_reading()) continue; 
      }
      if (result != -1) {              /* Counting EOFs as records       */  
           if (result != ibs ) async_nipr++;
           else async_nifr++; 
      }

      async_unlock (&mutex_done_read[next_read], &cond_done_read[next_read], &done_read[next_read]);
      next_read = (next_read+1) % inumbufs;
   }
}


/***
 ***   This function initializes the data structures and variables necessary 
 ***   to use threads when there are "onumbufs" buffers that are used both 
 ***   for input and output.  It then creates the write_slave thread.   
 ***/
void async_opt_initialize()
{
   int  i, j;

   onumbufs = ONBUFS;

   oshared = (struct shio *) malloc(sizeof (struct shio) * onumbufs);
   issue_write = (int *) malloc (sizeof (int) * onumbufs);
   done_write = (int *) malloc (sizeof (int) * onumbufs);
   cond_issue_write = (pthread_cond_t *) malloc (sizeof (pthread_cond_t) * onumbufs);
   cond_done_write = (pthread_cond_t *) malloc (sizeof (pthread_cond_t) * onumbufs);
   mutex_issue_write = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t) * onumbufs);
   mutex_done_write = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t) * onumbufs);
   if(oshared == (struct shio *)NULL || issue_write == (int *)NULL ||
         done_write == (int *)NULL) { 
      fprintf(stderr, MSGSTR(ENOMRY, "dd: not enough memory\n"));
      exit(1);
   }

   for (i=0; i < onumbufs; i++) {
      oshared[i].buf = (char *) malloc (obs); 
      if(oshared[i].buf == (char *)NULL) {
         if (i > 0) {                    /* attempt to recover */
            onumbufs = 1;
            for (j=1; j<i; j++)
               free (oshared[j].buf);
         }
         else {
            fprintf(stderr, MSGSTR(EBUFNOMRY, "dd: not enough memory for buffers\n"));
            exit(1);
         }
      }
      async_init(&mutex_issue_write[i], &cond_issue_write[i], &issue_write[i]);
      async_init(&mutex_done_write[i], &cond_done_write[i], &done_write[i]);
      async_lock(&mutex_issue_write[i], &cond_issue_write[i], &issue_write[i]);
   }

   first_round = 1;
   cur_write = 0; 
   cur_read = 1;
   ibuf = oshared[cur_write].buf;
   obuf = ibuf; 

   if (pthread_create(&w_slave, pthread_attr_default, 
       (void *) &write_slave, (pthread_addr_t) NULL) == -1) {
      perror(MSGSTR(ETHREAD, "dd: unable to create thread\n"));
      exit(1);
   }
}


/***
 ***   This function waits for the "write" to complete on the appropriate 
 ***   buffer.  It then performs a "read" and returns its result.
 ***/
int async_opt_read() 
{
   int  res;
   if (cur_read)
      async_lock(&mutex_done_write[cur_write], &cond_done_write[cur_write], &done_write[cur_write]);
   cur_read = 0;
   if(cflag&(NERR|SYNC))
      bzero (oshared[cur_write].buf, ibs);
   res = read(ibf, oshared[cur_write].buf, ibs);
   oshared[cur_write].err = errno;
   return (res);
}
      

/***
 ***   This function informs the write_slave thread that the current write 
 ***   buffer has data on it to be written.
 ***/
void async_opt_write(nbytes) 
   int  nbytes;
{
   oshared[cur_write].size = nbytes;
   async_unlock (&mutex_issue_write[cur_write], &cond_issue_write[cur_write], &issue_write[cur_write]);
   cur_write = (cur_write+1) % onumbufs;
   if (cur_write == 0) first_round = 0;
   obuf = oshared[cur_write].buf;
   ibuf = obuf; 
   cur_read = 1;
}


/***
 ***   This function initializes the data structures and variables necessary 
 ***   to use threads when there are "inumbufs" input buffers and "onumbufs" 
 ***   output buffers.  It then creates the read_slave, and write_slave
 ***   threads.   
 ***/
void async_initialize()
{
   int  i, j;

   if (fflag) {
      async_opt_initialize();
      return;
   }
   
   inumbufs = INBUFS;
   onumbufs = ONBUFS;

   ishared = (struct shio *) malloc(sizeof (struct shio) * inumbufs);
   oshared = (struct shio *) malloc(sizeof (struct shio) * onumbufs);
   issue_read = (int *) malloc (sizeof (int) * inumbufs);
   done_read = (int *) malloc (sizeof (int) * inumbufs);
   issue_write = (int *) malloc (sizeof (int) * onumbufs);
   done_write = (int *) malloc (sizeof (int) * onumbufs);
   cond_issue_read = (pthread_cond_t *) malloc (sizeof (pthread_cond_t) * inumbufs);
   cond_done_read = (pthread_cond_t *) malloc (sizeof (pthread_cond_t) * inumbufs);
   cond_issue_write = (pthread_cond_t *) malloc (sizeof (pthread_cond_t) * onumbufs);
   cond_done_write = (pthread_cond_t *) malloc (sizeof (pthread_cond_t) * onumbufs);
   mutex_issue_read = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t) * inumbufs);
   mutex_done_read = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t) * inumbufs);
   mutex_issue_write = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t) * onumbufs);
   mutex_done_write = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t) * onumbufs);
   if (ishared == (struct shio *)NULL || oshared == (struct shio *)NULL ||
         issue_read == (int *)NULL || done_read == (int *)NULL ||
         issue_write == (int *)NULL || done_write == (int *)NULL) { 
      fprintf(stderr, MSGSTR(ENOMRY, "dd: not enough memory\n"));
      exit(1);
   }
        
   for (i=0; i < inumbufs; i++) {
      ishared[i].buf = (char *) malloc(ibs);
      if(ishared[i].buf == (char *)NULL) { 
         if (i > 0) {                    /* attempt to recover */
            inumbufs = 1;
            onumbufs = 1;
            for (j=1; j<i; j++)
               free (ishared[j].buf);
         }
         else { 
            fprintf(stderr, MSGSTR(EBUFNOMRY, "dd: not enough memory for buffers\n"));
            exit(1);
         }
      }
      async_init(&mutex_done_read[i], &cond_done_read[i], &done_read[i]);
      async_init(&mutex_issue_read[i], &cond_issue_read[i], &issue_read[i]);
      async_lock(&mutex_done_read[i], &cond_done_read[i], &done_read[i]);
   }

   for (i=0; i < onumbufs; i++) {
      oshared[i].buf = (char *) malloc(obs);
      if(oshared[i].buf == (char *)NULL) { 
         if (i > 0) {                    /* attempt to recover */
            inumbufs = 1;
            onumbufs = 1;
            for (j=1; j<i; j++)
               free (oshared[j].buf);
            for (j=1; j<inumbufs; j++)
               free (ishared[j].buf);
         }
         else {
            fprintf(stderr, MSGSTR(EBUFNOMRY, "dd: not enough memory for buffers\n"));
            exit(1);
         }
      }
      async_init(&mutex_done_write[i], &cond_done_write[i], &done_write[i]);
      async_init(&mutex_issue_write[i], &cond_issue_write[i], &issue_write[i]);
      async_lock(&mutex_issue_write[i], &cond_issue_write[i], &issue_write[i]);
   }

   first_round = 1;
   cur_read = inumbufs-1;  /* First async_read will advance read buffer */ 
   cur_write = 0;          /* (inumbufs-1) and wait on buffer 0         */ 

   async_lock(&mutex_done_write[0], &cond_done_write[0], &done_write[0]);
   async_lock(&mutex_issue_read[inumbufs-1], &cond_issue_read[inumbufs-1], &issue_read[inumbufs-1]);

   ibuf = ishared[cur_read].buf;
   obuf = oshared[cur_write].buf;

   if (pthread_create(&r_slave, pthread_attr_default, (void *) &read_slave,
            (pthread_addr_t) NULL) == -1) { 
      perror(MSGSTR(ETHREAD, "dd: unable to create thread\n"));
      exit(1);
   }

   if (pthread_create(&w_slave, pthread_attr_default, (void *) &write_slave,
            (pthread_addr_t) NULL) == -1) {
      perror(MSGSTR(ETHREAD, "dd: unable to create thread\n"));
      exit(1);
   }
}


/***
 ***   This function backspaces the tape device head (if a tape device).
 ***   It figures out how much was overread (from buffer after "cur_read"
 ***   until and including buffer "overread").  It then backspaces the 
 ***   appropriate number of file marks followed by the appropriate number
 ***   of records.
 ***/
void async_backspace (overread) 
   int  overread;
{
   struct mtop  tape_rewind; 
   int  tape;
   int  err, done;
   int  num_eofs, num_records;
   int  result,  c, i;

   tape_rewind.mt_op = MTNOP;
   tape = ioctl (ibf, MTIOCTOP, &tape_rewind); 
   if (tape == 0) {
      done = 0; num_eofs = 0; num_records = 0;
      for (i=(cur_read+1)%inumbufs; !done; i=(i+1)%inumbufs) {
         result = ishared[i].res;
         if (result == 0)
            num_eofs++;
         else if (num_eofs == 0) {
            if (result == -1)
               if(cflag&NERR)              /* Was record read even though  */
                  for(c=0; c<ibs; c++)     /* read returned error?      */
                     if(ibuf[c] != 0)
                        result = c;
            if (result != -1) num_records++;
         }
         if (i == overread) done = 1;
      }

      tape_rewind.mt_op = MTCSE;           /* Clear exception if necessary */
      ioctl (ibf, MTIOCTOP, &tape_rewind); /* disregard error ! */

      tape_rewind.mt_op = MTBSF;           /* Backtrack overread eofs      */
      tape_rewind.mt_count = num_eofs;
      if (tape_rewind.mt_count) {
         if (ioctl (ibf, MTIOCTOP, &tape_rewind) == -1)
            perror(MSGSTR(ETAPE, "dd: error in repositioning the tape backward\n"));
      }

      tape_rewind.mt_op = MTCSE;           /* Clear exception if necessary */
         ioctl (ibf, MTIOCTOP, &tape_rewind);  /* disregard error ! */

      tape_rewind.mt_op = MTBSR;           /* Backtrack overread records   */
      tape_rewind.mt_count = num_records; 
      if (tape_rewind.mt_count) {
         if (ioctl (ibf, MTIOCTOP, &tape_rewind) == -1)
            perror(MSGSTR(ETAPE, "dd: error in repositioning the tape backward\n"));
      }
   }
}


/***
 ***   This function cancels the advance reads (if any), waits for the 
 ***   advance reads that weren't canceled to complete, backspaces the 
 ***   tape device head (when a tape device) for these extraneous reads, 
 ***   and waits for all of the writes to complete. 
 ***/
void async_term(err)
   int  err;
{
   int  i;
   int  last_overread;
     
   if (!fflag) {     

      last_overread = -1;
      for (i=(cur_read+1)%inumbufs; i != cur_read; i=(i+1)%inumbufs)
         if (!async_trylock (&mutex_issue_read[i], &cond_issue_read[i], &issue_read[i]))
            last_overread = i;
       
      if (last_overread != -1) {     
         async_lock (&mutex_done_read[last_overread], &cond_done_read[last_overread], &done_read[last_overread]);
         async_backspace (last_overread);
      }
   }

   /* Wait for all async writes issued to complete */
   /* Assumes that flsh() was called right before this procedure */
   if (first_round)
      for (i= 0; i < cur_write; i++)
         async_lock(&mutex_done_write[i], &cond_done_write[i], &done_write[i]);
   else
      for (i= (cur_write+1) %onumbufs; i != cur_write; i= (i+1)%onumbufs)
         async_lock(&mutex_done_write[i], &cond_done_write[i], &done_write[i]);

   if(cflag&SPARSE) 		/* in case of a pending seek */ 
	write_sparse (1, obf, (char *)0, 0);

   if (!fflag) {
      if (pthread_cancel (r_slave) == -1)
         perror(MSGSTR(ECANCEL, "dd: unable to cancel thread\n"));
      if (pthread_detach(&r_slave) == -1)
         perror(MSGSTR(ECANCEL, "dd: unable to cancel thread\n"));
   }
   if (pthread_cancel (w_slave) == -1)
      perror(MSGSTR(ECANCEL, "dd: unable to cancel thread\n"));
   if (pthread_detach(&w_slave) == -1)
      perror(MSGSTR(ECANCEL, "dd: unable to cancel thread\n"));
   
   async_nifr = nifr; 
   async_nipr = nipr;

   stats();
   if (close(obf) == -1) {
      perror (MSGSTR(ECLOSE, "dd close error"));
      exit(2);
   }
   exit(err);
}


/***
 ***   This function informs the read_slave thread that the current read 
 ***   buffer is ready to be reused. It then waits for the next read to 
 ***   complete and returns its result.  
 ***/
int async_read() 
{
int number_of_bytes_read;
   while(fflag) {
	number_of_bytes_read=async_opt_read();
	if (number_of_bytes_read <= 0) {
		if (end_of_medium_while_reading()) {
				number_of_bytes_read = 0;	
				continue;
		}
	}
        return (number_of_bytes_read);
   } 
   async_unlock (&mutex_issue_read[cur_read], &cond_issue_read[cur_read], &issue_read[cur_read]);
   cur_read = (cur_read+1) % inumbufs;
   async_lock(&mutex_done_read[cur_read], &cond_done_read[cur_read], &done_read[cur_read]);

   ibuf = ishared[cur_read].buf; 
   return (ishared[cur_read].res);
}


/***
 ***   This function informs the write_slave thread that the current write 
 ***   buffer has data on it to be written. It then waits until the next 
 ***   buffer is free for reuse.
 ***/
void async_write(nbytes) 
   int  nbytes;
{
   if (fflag) {
      async_opt_write(nbytes);
      return;
   }

   oshared[cur_write].size = nbytes;

   async_unlock (&mutex_issue_write[cur_write], &cond_issue_write[cur_write], &issue_write[cur_write]);
   cur_write = (cur_write+1) % onumbufs;
   if (cur_write == 0) first_round = 0;
   async_lock(&mutex_done_write[cur_write], &cond_done_write[cur_write], &done_write[cur_write]);
   obuf = oshared[cur_write].buf; 
}
#endif   /* THREADS */
