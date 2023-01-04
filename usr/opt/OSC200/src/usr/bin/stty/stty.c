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
static char	*sccsid = "@(#)$RCSfile: stty.c,v $ $Revision: 4.2.13.3 $ (DEC) $Date: 1993/10/18 20:33:17 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: CMDTTY terminal control commands
 *
 * FUNCTIONS: main (stty.c)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * stty-posix.c	1.12  com/cmd/tty/stty,3.1,9021 5/18/90 01:53:15
 */
 

 
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/termios.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <string.h>
 
#include        <nl_types.h>
#include        "stty_msg.h"
 
#include	<locale.h>	/* For setlocale() defines */

nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_STTY,num,str)  /*MSG*/
 
extern char *getenv();
 
#define eq(s) (!strcmp(arg, s))
 
struct
{
    char *string;
    int speed;
} speeds[] = {
    "0",     B0,
    "50",    B50,
    "75",    B75,
    "110",   B110,
    "134",   B134,
    "150",   B150,
    "200",   B200,
    "300",   B300,
    "600",   B600,
    "1200",  B1200,
    "1800",  B1800,
    "2400",  B2400,
    "4800",  B4800,
    "9600",  B9600,
    "19200", B19200,
    "38400", B38400,
    "134.5", B134,			/* alternates go down here */
    "19.2",  B19200,
    "38.4",  B38400,
    "exta",  B19200,
    "extb",  B38400,
    0, 0,
};
 
struct mds {
    char *string;
    int set;
    int reset;
};
 
struct mds cmodes[] = {
    "-parity",	CS8,			PARENB|CSIZE,
    "-evenp",	CS8,			PARENB|CSIZE,
    "-oddp",	CS8,			PARENB|PARODD|CSIZE,
    "pass8",	CS8, 			PARENB | CSIZE,
    "parity",	PARENB|CS7,		PARODD|CSIZE,
    "evenp",	PARENB|CS7,		PARODD|CSIZE,
    "oddp",	PARENB|PARODD|CS7,	CSIZE,
    "parenb",	PARENB,			0,
    "-parenb",	0,			PARENB,
    "parodd",	PARODD,			0,
    "-parodd",	0,			PARODD,
    "cs8",	CS8,			CSIZE,
    "cs7",	CS7,			CSIZE,
    "cs6",	CS6,			CSIZE,
    "cs5",	CS5,			CSIZE,
    "cstopb",	CSTOPB,			0,
    "-cstopb",	0,			CSTOPB,
    "hupcl",	HUPCL,			0,
    "hup",	HUPCL,			0,
    "-hupcl",	0,			HUPCL,
    "-hup",	0,			HUPCL,
    "clocal",	CLOCAL,			0,
    "-clocal",	0,			CLOCAL,
    "cread",	CREAD,			0,
    "-cread",	0,			CREAD,
    "raw",	CS8,			(CSIZE|PARENB),
    "-raw",	(CS8),			(CSIZE|PARENB),
    "cooked",	(CS8),			(CSIZE|PARENB),
    0
    };
 
struct mds imodes[] = {
    "ignbrk",	IGNBRK,			0,
    "-ignbrk",	0,			IGNBRK,
    "brkint",	BRKINT,			0,
    "-brkint",	0,			BRKINT,
    "ignpar",	IGNPAR,			0,
    "-ignpar",	0,			IGNPAR,
    "parmrk",	PARMRK,			0,
    "-parmrk",	0,			PARMRK,
    "inpck",	INPCK,			0,
    "-inpck",	0,			INPCK,		
    "istrip",	ISTRIP,			0,
    "-istrip",	0,			ISTRIP,
    "inlcr",	INLCR,			0,
    "-inlcr",	0,			INLCR,
    "igncr",	IGNCR,			0,
    "-igncr",	0,			IGNCR,
    "icrnl",	ICRNL,			0,
    "-icrnl",	0,			ICRNL,
    "nl",		ICRNL,			0,
    "-nl",		0,			(ICRNL|INLCR|IGNCR),
    "iuclc",	IUCLC,			0,
    "-iuclc",	0,			IUCLC,
    "lcase",	IUCLC,			0,
    "-lcase",	0,			IUCLC,
    "LCASE",	IUCLC,			0,
    "-LCASE",	0,			IUCLC,
    "flow",	IXON,			0,
    "-flow",	0,			IXON,
    "tandem",	IXOFF,			0,
    "-tandem",	0,			IXOFF,
    "decctlq",	0,			IXANY,
    "-decctlq",	IXANY,			0,
    "ixon",	IXON,			0,
    "-ixon",	0,			IXON,
    "ixany",	IXANY,			0,
    "-ixany",	0,			IXANY,
    "ixoff",	IXOFF,			0,
    "-ixoff",	0,			IXOFF,
    "dec",	0,			IXANY,
    "imaxbel",	IMAXBEL,		0,
    "-imaxbel",	0,			IMAXBEL,
    "raw",	0,			-1,
    "-raw",	(BRKINT|IGNPAR|ICRNL|IXON),	0,
    "cooked",	(BRKINT|IGNPAR|ICRNL|IXON),	0,
    "sane",	(BRKINT|IGNPAR|ICRNL|IXON|IXOFF|IMAXBEL),
			(IGNBRK|PARMRK|INPCK|INLCR|IGNCR|IUCLC),
    0
    };
 
struct mds lmodes[] = {
 "isig",	ISIG,	0,
 "-isig",	0,	ISIG,
 "icanon",	ICANON,	0,
 "-icanon",	0,	ICANON,
 "xcase",	XCASE,	0,
 "-xcase",	0,	XCASE,
 "lcase",	XCASE,	0,
 "-lcase",	0,	XCASE,
 "LCASE",	XCASE,	0,
 "-LCASE",	0,	XCASE,
 "echo",	ECHO,	0,
 "-echo",	0,	ECHO,
 "echoe",	ECHOE,	0,
 "-echoe",	0,	ECHOE,
 "crterase",	ECHOE,	0,
 "-crterase",	0,	ECHOE,
 "crtbs",	ECHOE,	0,   /* crtbs not supported, close enough */
 "-crtbs",	0,	ECHOE,
 "echok",	ECHOK,	0,
 "-echok",	0,	ECHOK,
 "lfkc",	ECHOK,	0,
 "-lfkc",	0,	ECHOK,
 "echonl",	ECHONL,	0,
 "-echonl",	0,	ECHONL,
 "noflsh",	NOFLSH,	0,
 "-noflsh",	0,	NOFLSH,
 "tostop",	TOSTOP,	0,
 "-tostop",	0,	TOSTOP,
 "echoctl",	ECHOCTL,	0,
 "-echoctl",	0,	ECHOCTL,
 "ctlecho",	ECHOCTL,	0,
 "-ctlecho",	0,	ECHOCTL,
 "echoprt",	ECHOPRT,	0,
 "-echoprt",	0,	ECHOPRT,
 "prterase",	ECHOPRT,	0,
 "-prterase",	0,	ECHOPRT,
 "echoke",	ECHOKE,	0,
 "-echoke",	0,	ECHOKE,
 "crtkill",	ECHOKE,	0,
 "-crtkill",	0,	ECHOKE,
 "altwerase",	ALTWERASE,	0,
 "-altwerase",	0,	ALTWERASE,
 "mdmbuf",	MDMBUF,	0,
 "-mdmbuf",	0,	MDMBUF,
 "nohang",	NOHANG,	0,
 "-nohang",	0,	NOHANG,
 "nokerninfo",  NOKERNINFO, 0,
 "-nokerninfo", 0,      NOKERNINFO,
 "iexten",	IEXTEN,	0,
 "-iexten",	0,	IEXTEN,
 "raw",		0,	(ISIG|ICANON|XCASE),
 "-raw",	(ISIG|ICANON),	0,
 "dec",		ECHOE|ECHOKE|ECHOCTL,	ECHOPRT,
 "cooked",	(ISIG|ICANON),	0,
 "sane",	(ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHOKE|ECHOCTL),
			(XCASE|ECHONL|ECHOPRT|NOFLSH),
 "crt",		ECHOE|ECHOKE|ECHOCTL,	ECHOK|ECHOPRT,
 "-crt",	ECHOK,	ECHOE|ECHOKE|ECHOCTL,
 "newcrt",	ECHOE|ECHOKE|ECHOCTL,	ECHOK|ECHOPRT,
 "-newcrt",	ECHOK,	ECHOE|ECHOKE|ECHOCTL, 
 0,
};
 
struct mds omodes[] = {
 "opost",	OPOST,	0,
 "-opost",	0,	OPOST,
 "-litout",	OPOST,	0,
 "litout",	0,	OPOST,
 "xtabs",	OXTABS,	0,
 "-xtabs",	0,	OXTABS,
 "oxtabs",	OXTABS,	0,
 "-oxtabs",	0,	OXTABS,
 "onoeot",	ONOEOT,	0,
 "-onoeot",	0,	ONOEOT,
 "olcuc",	OLCUC,	0,
 "-olcuc",	0,	OLCUC,
 "lcase",	OLCUC,	0,
 "-lcase",	0,	OLCUC,
 "LCASE",	OLCUC,	0,
 "-LCASE",	0,	OLCUC,
 "onlcr",	ONLCR,	0,
 "-onlcr",	0,	ONLCR,
 "nl",		ONLCR,	0,
 "-nl",		0,	(ONLCR|OCRNL|ONLRET),
 "ocrnl",	OCRNL,	0,
 "-ocrnl",	0,	OCRNL,
 "onocr",	ONOCR,	0,
 "-onocr",	0,	ONOCR,
 "onlret",	ONLRET,	0,
 "-onlret",	0,	ONLRET,
 "fill",	OFILL,	OFDEL,
 "-fill",	0,	OFILL|OFDEL,
 "nul-fill",	OFILL,	OFDEL,
 "del-fill",	OFILL|OFDEL,	0,
 "ofill",	OFILL,	0,
 "-ofill",	0,	OFILL,
 "ofdel",	OFDEL,	0,
 "-ofdel",	0,	OFDEL,
 "cr0",	CR0,	CRDLY,
 "cr1",	CR1,	CRDLY,
 "cr2",	CR2,	CRDLY,
 "cr3",	CR3,	CRDLY,
 "tab0",	TAB0,	TABDLY,
 "-tabs",	OXTABS,	0,
 "tab1",	TAB1,	TABDLY,
 "tab2",	TAB2,	TABDLY,
 "tab3",	TAB3,	TABDLY,
 "tabs",	0,	OXTABS,
 "nl0",	NL0,	NLDLY,
 "nl1",	NL1,	NLDLY,
 "nl2",	NL2,	NLDLY,
 "nl3",	NL3,	NLDLY,
 "ff0",	FF0,	FFDLY,
 "ff1",	FF1,	FFDLY,
 "vt0",	VT0,	VTDLY,
 "vt1",	VT1,	VTDLY,
 "bs0",	BS0,	BSDLY,
 "bs1",	BS1,	BSDLY,
 "raw",	0,	OPOST,
 "-raw",	OPOST,	0,
 "cooked",	OPOST,	0,
 "tty33",	CR1,	(CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
 "tn300",	CR1,	(CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
 "ti700",	CR2,	(CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
 "vt05",	NL1,	(CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
 "tek",	FF1,	(CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
 "tty37",	(FF1|VT1|CR2|TAB1|NL1),	(NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY),
 "sane",	(OPOST|ONLCR),	(OLCUC|OCRNL|ONOCR|ONLRET|OFILL|OFDEL|
				 NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY),
 0,
};
 
 
/*
 * Special control characters.
 *
 * Each entry has a list of names.  The first is the primary name
 * and is used when printing the control character in the "name = val;"
 * form.  The second is an abbreviation which is guaranteed to be less
 * than or equal to four characters in length and is primarily used
 * when printing the values in columunar form (guarantees all will
 * fit within 80 cols).  The rest are optional aliases.
 * All names are recognized on the command line.
 */
#define MAXNAMES 5	/* ??? */
struct {
	char	*names[MAXNAMES+1];	/* ??? */
	int	sub;
	u_char	def;
} cchars[] = {
	{ { "erase", "era" },		VERASE,	CERASE },
	{ { "werase", "wera" },		VWERASE, CWERASE },
	{ { "kill", "kill" },		VKILL,	CKILL },
	{ { "intr", "int" },		VINTR,	CINTR },
	{ { "quit", "quit" },		VQUIT,	CQUIT },
	{ { "susp", "susp" },		VSUSP,	CSUSP },
	{ { "dsusp", "dsus" },		VDSUSP,	CDSUSP },
	{ { "eof", "eof" },		VEOF,	CEOF },
	{ { "eol", "eol", "brk" },	VEOL,	CEOL },
	{ { "eol2", "eol2" },		VEOL2,	CEOL },
	{ { "stop", "stop", "xoff" },	VSTOP,	CSTOP },
	{ { "start", "star", "xon" },	VSTART,	CSTART },
	{ { "lnext", "lnxt" },		VLNEXT,	CLNEXT },
	{ { "discard", "disc", "flush", "flusho", "fls" },	VDISCARD, CDISCARD },
	{ { "reprint", "rpnt", "rprnt" },	VREPRINT, CREPRINT },
	{ { "status", "stat" },		VSTATUS, CSTATUS },
	{ { "time", "time" },		VTIME, CTIME },
	{ { "min", "min" },		VMIN, CMIN },
	0
};
 
char *arg;
char *STTY;
int ctl = 0;			/* File descriptor for setting modes */
int pitt = 0;
struct termios cb;
struct winsize win;
int ldisc;
int nldisc;
 
#define NORMAL	0	/* only print modes differing from defaults */
#define ALL	1	/* print all modes - POSIX standard format */
#define GFMT	2	/* print modes in a form that can be re-input to stty */
main(argc, argv)
char *argv[];
{
    register i;
    int fmt = NORMAL;
    extern char *optarg;
    extern int optind, opterr, optopt;
    int ch;
    int ok = 1;
    int old_optind = optind;
    
    
	/* Set locale information from user's environment */
	setlocale (LC_ALL, "");

	/* Open message catalog - use LC_MESSAGES if defined */
    catd = catopen(MF_STTY, NL_CAT_LOCALE);
 
    if (STTY = strrchr(argv[0], '/'))
	++STTY;
    else
	STTY = argv[0];
    
    opterr = 0;
    while (ok && (ch = getopt(argc, argv, "f:ga")) != EOF)
	    switch((char)ch) {
	    case 'f':
		    if ((ctl = open(optarg, O_RDONLY | O_NONBLOCK)) < 0){
			    fprintf(stderr, "%s:", STTY);
			    perror(optarg);
			    exit(2);
		    }
		    old_optind = optind;
		    break;
	    case 'a':
		    fmt = ALL;
		    old_optind = optind;
		    break;
	    case 'g':
		    fmt = GFMT;
		    old_optind = optind;
		    break;
	    case '?':
	    default:
		    ok = 0;
		    if (optopt == 'f') {
			printf("usage: stty [-a | -g] [-f special device] [arguments ...]\n");
		    	exit(2);
	 	    }
		    if (old_optind != optind)
			--optind;
		    optind;
		    break;
		    
	    }
    argc -= optind;
    argv += optind;
 
    gttyinfo();
    if (argc == 0) {
	    switch(fmt)
	    {
	    case NORMAL:
		    prmodes();
		    exit(0);
	    case ALL:
		    pramodes();
		    exit(0);
	    case GFMT:
		    prencode();
		    exit(0);
	    }
    }
    
 
 
    while(argc-- > 0) {
	int flag = 0;
 
	arg = *argv++;
	for (i=0; *cchars[i].names; i++) {
		char **cp = cchars[i].names;
		while (*cp) {
			if (eq(*cp) && argc-- > 0) {
				arg = *argv++;
				if (eq("undef") || 
				    eq("disable"))
					cb.c_cc[cchars[i].sub] = 
					   _POSIX_VDISABLE;
				else if (*arg == '^' && arg[1] != '\0' &&
						 arg[2] == '\0')
					cb.c_cc[cchars[i].sub] = 
					    ((arg)[1] == '?') ? 0177 :
					    ((arg)[1] == '-') ?
					     _POSIX_VDISABLE :
					     (arg)[1] & 037;
				else if ((cchars[i].sub == VTIME ||
						  cchars[i].sub == VMIN) &&
							  isdigit(*arg))
				{
					cb.c_cc[cchars[i].sub] = atoi(arg);
				}
				else if (arg[1] == '\0')
					cb.c_cc[cchars[i].sub] = *arg;
				else	
					break;  /* the input is ignored. */
				
				flag = 1;
				continue;
			}
			cp++;
		}
	}

	/* Force to go to top of while if a match with cchars[] was found */
	if (flag)
		continue;

	if (eq("rows")	&& argc-- > 0)
	{
		int i;
		arg = *argv++;
		if (sscanf(arg, "%d", &i) != 1)
		{
			continue;
		}
		win.ws_row = i;
		flag = 1;
		continue;
	}
	else if ((eq("columns") || eq("cols")) && argc-- > 0)
	{
		int i;
		arg = *argv++;
		if (sscanf(arg, "%d", &i) != 1)
		{
			continue;
		}
		win.ws_col = i;
		flag = 1;
		continue;
	}
	else if (eq("line") && argc-- > 0)
	{
		int i;
		arg = *argv++;
		if (sscanf(arg, "%d", &i) != 1)
			continue;
		nldisc = i;
		continue;
	}
	else if (eq("ispeed")) {
		if (argc-- <= 0) {
			fprintf(stderr, "missing ispeed\n");
			exit(2);
		}
		cfsetispeed(&cb, atoi(*argv++));
		continue;
	}
	else if (eq("ospeed")) {
		if (argc-- <= 0) {
			fprintf(stderr, "missing ospeed\n");
			exit(2);
		}
		cfsetospeed(&cb, atoi(*argv++));
		continue;
	}
	else if (eq("all") || eq("everything"))
	{
		pramodes();
		exit(0);
	}
	else if (eq("old") || eq("new")) {
		continue;
	}
	else if (eq("dec"))
	{
		cb.c_cc[VERASE] = 0177;
		cb.c_cc[VKILL] = CTRL('u');
		cb.c_cc[VINTR] = CTRL('c');
	}
	else if (eq("ek"))
	{
		cb.c_cc[VERASE] = CERASE;
		cb.c_cc[VKILL] = CKILL;
		flag = 1;
	}
	else if (eq("raw")) {
	    cb.c_cc[VMIN] = 1;
	    cb.c_cc[VTIME] = 1;
	} else if (eq("-raw") | eq("cooked")) {
	    cb.c_cc[VEOF] = CEOF;
	    cb.c_cc[VEOL] = CEOL;
	    cb.c_cc[VTIME] = 0;
	} else if(eq("sane")) {
		int i;
		for (i = 0; *cchars[i].names; i++)
		{
			cb.c_cc[cchars[i].sub]  = cchars[i].def;
		}
		
		
	}
	else if (isdigit(*arg)) {
	    char *o;
	    speed_t in = 0, out = 0;
 
	    if (o = strchr(arg, '/')) { 
		*o = '\0';
		for(i=0; speeds[i].string; i++)
		    if(eq(speeds[i].string)) {
			in = speeds[i].speed;
			break;
		    }
		arg = o+1;
	    }
	    for(i=0; speeds[i].string; i++)
		if(eq(speeds[i].string)) {
		    out = speeds[i].speed;
		    if (!o)	/* No input baud rate was specified. */
		    {
			    in = out;
		    }
		    cfsetospeed(&cb, out);
		    cfsetispeed(&cb, in);
		    break;
		}
	    if (speeds[i].string)
		continue;
	}
	else if (eq("speed"))
	{	
		prspeed();
		printf("\n");
		exit(0);
	}
	else if (eq("size"))
	{
		printf("%d %d\n", win.ws_row, win.ws_col);
		exit(0);
	}
	
	for(i=0; imodes[i].string; i++)
	    if(eq(imodes[i].string)) {
		cb.c_iflag &= ~imodes[i].reset;
		cb.c_iflag |= imodes[i].set;
		flag = 1;
	    }
	for(i=0; omodes[i].string; i++)
	    if(eq(omodes[i].string)) {
		cb.c_oflag &= ~omodes[i].reset;
		cb.c_oflag |= omodes[i].set;
		flag = 1;
	    }
	for(i=0; cmodes[i].string; i++)
	    if(eq(cmodes[i].string)) {
		cb.c_cflag &= ~cmodes[i].reset;
		cb.c_cflag |= cmodes[i].set;
		flag = 1;
	    }
	for(i=0; lmodes[i].string; i++)
	    if(eq(lmodes[i].string)) {
		cb.c_lflag &= ~lmodes[i].reset;
		cb.c_lflag |= lmodes[i].set;
		flag = 1;
	    }
	
	if(!flag && !encode(arg)) {
	    fprintf(stderr, MSGSTR(UNKNOWN, "unknown mode: %s\n"), arg);
	    exit(2);
	}
    }
    if (nldisc != ldisc && ioctl(ctl, TIOCSETD, &nldisc) < 0) {
	perror("ioctl(TIOCSETD)");
	exit(2);
    }
    if (tcsetattr(ctl, TCSADRAIN, &cb) == -1) {
	fprintf(stderr, "%s: ", STTY);
	perror("setattr");
	exit(2);
    }
    if (ioctl(ctl, TIOCSWINSZ, &win) < 0)
    {
	fprintf(stderr, "%s: ", STTY);
	perror("TIOCSWINSZ");
	exit(2);
     }	    
    exit(0);
}
 
gttyinfo()
{
    if (tcgetattr(ctl, &cb) == -1) {
	fprintf(stderr, "%s: ", STTY);
	perror("tcgetattr");
	exit(2);
    }
 
    if (ioctl(ctl, TIOCGETD, &ldisc) < 0)
    {
	    fprintf(stderr, "%s: ", STTY);
	    perror("TIOCGETD");
	    exit(2);
    }	    
 
    nldisc = ldisc;
    if (ioctl(ctl, TIOCGWINSZ, &win) < 0) {
	fprintf(stderr, "%s: ", STTY);
	perror("TIOCGWINSZ");
	exit(2);
    }
}
 
prmodes()
{
    register m;
    register i;
 
    m = cb.c_cflag;
 
    prldisc();
    prspeed();
    if (m&PARENB)
	if (m&PARODD)
	    printf("oddp ");
	else
	    printf("evenp ");
    else
	printf("-parity ");
    if(((m&PARENB) && !(m&CS7)) || (!(m&PARENB) && !(m&CS8)))
	printf("cs%c ",'5'+(m&CSIZE)/CS6);
    if (m&CSTOPB)
	printf("cstopb ");
    if (m&HUPCL)
	printf("hupcl ");
    if (!(m&CREAD))
	printf("cread ");
    if (m&CLOCAL)
	printf("clocal ");
    printf("\n");
    pitt = 0;
    /*
     * special control characters
     */
    for (i=0; *cchars[i].names; i++) {
	    if (cb.c_cc[cchars[i].sub] != cchars[i].def) {
		    pit(cb.c_cc[cchars[i].sub], *cchars[i].names);
	    }
    }
    if (pitt)
	printf("\n");
    m = cb.c_iflag;
    if (m&IGNBRK)
	printf("ignbrk ");
    else if (m&BRKINT)
	printf("brkint ");
    if (!(m&INPCK))
	printf("-inpck ");
    else if (m&IGNPAR)
	printf("ignpar ");
    if (m&PARMRK)
	printf("parmrk ");
    if (!(m&ISTRIP))
	printf("-istrip ");
    if (m&INLCR)
	printf("inlcr ");
    if (m&IGNCR)
	printf("igncr ");
    if (m&ICRNL)
	printf("icrnl ");
    if (m&IUCLC)
	printf("iuclc ");
    if (!(m&IXON))
	printf("-ixon ");
    else if (!(m&IXANY))
	printf("-ixany ");
    if (m&IXOFF)
	printf("ixoff ");
    m = cb.c_oflag;
    if (!(m&OPOST))
	printf("-opost ");
    else {
	if (m&OLCUC)
	    printf("olcuc ");
	if (m&ONLCR)
	    printf("onlcr ");
	if (m&OCRNL)
	    printf("ocrnl ");
	if (m&ONOCR)
	    printf("onocr ");
	if (m&ONLRET)
	    printf("onlret ");
	if (m&OXTABS)
	    printf("-tabs ");
 	if (m&ONOEOT)
	   printf("onoeot ");
	if (m&OFILL)
	    if (m&OFDEL)
		printf("del-fill ");
	    else
		printf("nul-fill ");
	delay((m&CRDLY)/CR1, "cr");
	delay((m&NLDLY)/NL1, "nl");
	delay((m&TABDLY)/TAB1, "tab");
	delay((m&BSDLY)/BS1, "bs");
	delay((m&VTDLY)/VT1, "vt");
	delay((m&FFDLY)/FF1, "ff");
    }
    printf("\n");
    m = cb.c_lflag;
    if (!(m&ISIG))
	printf("-isig ");
    if (!(m&ICANON))
	printf("-icanon ");
    if (m&XCASE)
	printf("xcase ");
    printf("-echo "+((m&ECHO)!=0));
    printf("-echoe "+((m&ECHOE)!=0));
    printf("-echok "+((m&ECHOK)!=0));
    if (m&ECHONL)
	printf("echonl ");
    if (m&NOFLSH)
	printf("noflsh ");
    printf("\n");
}
 
pramodes()
{
    register int m;
    register int i;
 
    prldisc();
    prspeed();
    prsize();
    printf("\n");
    pitt = 0;
    /*
     * special control characters
     */
    for (i=0; *cchars[i].names; i++) {
	    pit(cb.c_cc[cchars[i].sub], *cchars[i].names);
    }
    if (pitt)
	printf("\n");
 
    m = cb.c_cflag;
    printf("-parenb "+((m&PARENB)!=0));
    printf("-parodd "+((m&PARODD)!=0));
    printf("cs%c ",'5'+(m&CSIZE)/CS6);
    printf("-cstopb "+((m&CSTOPB)!=0));
    printf("-hupcl "+((m&HUPCL)!=0));
    printf("-cread "+((m&CREAD)!=0));
    printf("-clocal "+((m&CLOCAL)!=0));
    printf("\n");
    m = cb.c_iflag;
    printf("-ignbrk "+((m&IGNBRK)!=0));
    printf("-brkint "+((m&BRKINT)!=0));
    printf("-ignpar "+((m&IGNPAR)!=0));
    printf("-parmrk "+((m&PARMRK)!=0));
    printf("-inpck "+((m&INPCK)!=0));
    printf("-istrip "+((m&ISTRIP)!=0));
    printf("-inlcr "+((m&INLCR)!=0));
    printf("-igncr "+((m&IGNCR)!=0));
    printf("-icrnl "+((m&ICRNL)!=0));
    printf("-iuclc "+((m&IUCLC)!=0));
    printf("\n");
    printf("-ixon "+((m&IXON)!=0));
    printf("-ixany "+((m&IXANY)!=0));
    printf("-ixoff "+((m&IXOFF)!=0));
    printf("-imaxbel "+((m&IMAXBEL)!=0));
    printf("\n");
    m = cb.c_lflag;
    printf("-isig "+((m&ISIG)!=0));
    printf("-icanon "+((m&ICANON)!=0));
    printf("-xcase "+((m&XCASE)!=0));
    printf("-echo "+((m&ECHO)!=0));
    printf("-echoe "+((m&ECHOE)!=0));
    printf("-echok "+((m&ECHOK)!=0));
    printf("-echonl "+((m&ECHONL)!=0));
    printf("-noflsh "+((m&NOFLSH)!=0));
    printf("-mdmbuf "+((m&MDMBUF)!=0));    
    printf("-nohang "+((m&NOHANG)!=0));
    printf("\n");
    printf("-tostop "+((m&TOSTOP)!=0));
    printf("-echoctl "+((m&ECHOCTL)!=0));
    printf("-echoprt "+((m&ECHOPRT)!=0));
    printf("-echoke "+((m&ECHOKE)!=0));
    printf("-altwerase "+((m&ALTWERASE)!=0));
    printf("-iexten "+((m&IEXTEN)!=0));
    printf("-nokerninfo "+((m&NOKERNINFO)!=0));
    printf("\n");
    m = cb.c_oflag;
    printf("-opost "+((m&OPOST)!=0));
    printf("-olcuc "+((m&OLCUC)!=0));
    printf("-onlcr "+((m&ONLCR)!=0));
    printf("-ocrnl "+((m&OCRNL)!=0));
    printf("-onocr "+((m&ONOCR)!=0));
    printf("-onlret "+((m&ONLRET)!=0));
    printf("-ofill "+((m&OFILL)!=0));
    printf("-ofdel "+((m&OFDEL)!=0));
    printf("-tabs "+((m&OXTABS)==0));
    printf("-onoeot "+((m&ONOEOT)!=0));
    delay((m&CRDLY)/CR1, "cr");
    delay ((m & NLDLY) / NL1, "nl");
    delay((m&TABDLY)/TAB1, "tab");
    delay((m&BSDLY)/BS1, "bs");
    delay((m&VTDLY)/VT1, "vt");
    delay((m&FFDLY)/FF1, "ff");
    printf("\n");
}
 
gct(cp)
register char *cp;
{
    register c;
 
    c = *cp++;
    if (c == '^') {
	c = *cp;
	if (c == '?')
	    c = 0177;
	else if (c == '-')
	    c = 0377;
	else
	    c &= 037;
    }
    return(c);
}
 
pit(what, itsname)
unsigned char what;
char *itsname;
{
    if (pitt > 60) {
	printf("\n");
	pitt = 0;
    }
    if (pitt) {
	printf("; ");
	pitt += 2;
    }
 
    printf("%s", itsname);
    pitt += strlen(itsname);
    if (what == 0377) {
	printf(" <undef>");
	pitt += 8;
	return;
    }
    printf(" = ");
    pitt += 3;
    if (strcmp("time", itsname) &&  strcmp("min", itsname))
    {
	    if (iscntrl(what)) {
		    printf("^");
		    ++pitt;
		    what ^= '@';
	    }
	    printf("%c", what);
    }
    else
    {
	    printf("%d", what);
    }
    ++pitt;
}
 
delay(m, s)
char *s;
{
    if(m)
	printf("%s%d ", s, m);
}
 
prspeed()
{
    int i_index, o_index;
    speed_t is = cfgetispeed(&cb);
    speed_t os = cfgetospeed(&cb);
    
    if (is && is != os)
    {
	    for ( i_index = 0; speeds[i_index].string; i_index++)
	    {
		    if (is == speeds[i_index].speed)
		    {
			    break;
		    }
	    }

	    if (speeds[i_index].string == 0)
	    {
		    i_index = 0;
	    }
	    
	    for ( o_index = 0; speeds[o_index].string; o_index++)
	    {
		    if (os == speeds[o_index].speed)
		    {
			    break;
		    }
	    }

	    if (speeds[o_index].string == 0)
	    {
		    o_index = 0;
	    }
	    
	    printf(MSGSTR(SPLBAUD, "ispeed %s baud ; ospeed %s baud; "),
	       speeds[i_index].string, speeds[o_index].string);
    }
    else
    {
		    
	    
	    for ( o_index = 0; speeds[o_index].string; o_index++)
	    {
		    if (os == speeds[o_index].speed)
		    {
			    break;
		    }
	    }

	    if (speeds[o_index].string == 0)
	    {
		    o_index = 0;
	    }
	    
	    printf(MSGSTR(BAUD, "speed %s baud; "), speeds[o_index].string);
    }
    
}
 
prsize()
{
    printf(MSGSTR(ROWS, "%d rows; %d columns"), win.ws_row,
	    win.ws_col);
}
 
prldisc()
{
	char *ld;
	char unknown[32];
 
	if (ldisc != TTYDISC)
	{
		switch(ldisc)
		{
		case TABLDISC:
			ld =  "tablet";
			break;
		case SLIPDISC:
			ld = "slip(ed)";
			break;
		case DUDISC:
			ld = "dialup IP";
			break;
		default:	
			sprintf(unknown, "#%d", ldisc);
			ld = unknown;
		}
		printf(MSGSTR(LDISC, "%s disc;"), ld);
	}
}
 
prencode()
{
    int i;
 
    for (i = 0; i < NCCS; ++i)
	printf("%x:", cb.c_cc[i]);
    printf("%x:%x:%x:%x:\n", cb.c_iflag, cb.c_oflag, cb.c_cflag, cb.c_lflag);
}
 
encode(argptr)
char *argptr;
{
    int i, c;
 
    for (i = 0; i < NCCS; ++i, ++argptr) {
	if (sscanf(argptr, "%x", &c) != 1 ||
	    !(argptr = strchr(argptr, ':')))
	    return 0;
	cb.c_cc[i] = c;
    }
 
    if (sscanf(argptr, "%x:%x:%x:%x", &cb.c_iflag, &cb.c_oflag, &cb.c_cflag,
	       &cb.c_lflag) != 4)
	return 0;
    return(1);
}
