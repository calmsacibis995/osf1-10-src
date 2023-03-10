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
static char rcsid[] = "@(#)$RCSfile: tip.c,v $ $Revision: 4.2.12.5 $ (DEC) $Date: 1993/10/18 20:08:46 $";
#endif
/*
tip.c	1.5  com/cmd/tip,3.1,9013 12/21/89 16:42:26";
 */
/* 
 * COMPONENT_NAME: UUCP tip.c
 * 
 * FUNCTIONS: MSGSTR, Mtip, any, cleanup, ctrl, escape, help, 
 *            interp, intprompt, load_etable, load_sep, prompt, 
 *            pwrite, raw, setparity, size, sname, speed, tipin, 
 *            ttysetup, unraw 
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/* static char sccsid[] = "tip.c	5.4 (Berkeley) 4/3/86"; */

/*
 * tip - UNIX link to other systems
 *  tip [-v] [-speed] system-name
 * or
 *  cu phone-number [-s speed] [-l line] [-a acu]
 */
#include <locale.h>

#include "tip.h"
#include "pathnames.h"

extern char *sep[];
extern esctable_t etable[];/*
 * Baud rate mapping table
 */
int bauds[] = {
	0, 50, 75, 110, 134, 150, 200, 300, 600,
	1200, 1800, 2400, 4800, 9600, 19200, 38400, -1
};

int	disc = OTTYDISC;		/* tip normally runs this way */
void	intprompt();
void	timeout();
void	cleanup();
char	*sname();
char	PNbuf[256];			/* This limits the size of a number */

main(argc, argv)
	char *argv[];
{
	char *system = NOSTR;
	register int i;
	register char *p;
	char sbuf[12];
	int sysflag=0;			/* system arg given? */

	setlocale(LC_ALL, "");
	catd = catopen(MF_TIP, NL_CAT_LOCALE);
	load_sep();
	load_etable();
	gid = getgid();
        egid = getegid();
        uid = getuid();
        euid = geteuid();
	if (equal(sname(argv[0]), "cu")) {
		cumode = 1;
		cumain(argc, argv);
		goto cucommon;
	}

	if (argc > 4) {
		fprintf(stderr, MSGSTR(USAGE4, "usage: tip [-v] [-speed] system-name | number\n"));
		exit(1);
	}
	if (!isatty(0)) {
		fprintf(stderr, MSGSTR(INTERACTIVE, "tip: must be interactive\n"));
		exit(1);
	}

	for (; argc > 1; argv++, argc--) {
		if (argv[1][0] != '-') {
			if (sysflag) {	/* already have one */
				fprintf(stderr, MSGSTR(USAGE4, "usage: tip [-v] [-speed] system-name | number\n"));
				exit(1);
			}
			sysflag++;
			system = argv[1];
		}
		else switch (argv[1][1]) {

		case 'v':
			vflag++;
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			BR = atoi(&argv[1][1]);
			break;

		default:
			fprintf(stderr, MSGSTR(UNKOPT, "tip: %s, unknown option\n"), argv[1]); /*MSG*/
			break;
		}
	}

	if (system == NOSTR)
		goto notnumber;
	if (isalpha(*system))
		goto notnumber;
	/*
	 * System name is really a phone number...
	 * Copy the number then stomp on the original (in case the number
	 *	is private, we don't want 'ps' or 'w' to find it).
	 */
	if (strlen(system) > sizeof PNbuf - 1) {
		fprintf(stderr, MSGSTR(NUMTOOLONG, "tip: phone number too long (max = %d bytes)\n"), /*MSG*/
			sizeof PNbuf - 1);
		exit(1);
	}
	strncpy( PNbuf, system, sizeof PNbuf - 1 );
	for (p = system; *p; p++)
		*p = '\0';
	PN = PNbuf;
	(void)sprintf(system = sbuf, "tip%d", BR); /* post-BSD4.3 semantics */

notnumber:
	signal(SIGINT, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGHUP, cleanup);
	signal(SIGTERM, cleanup);

	if ((i = hunt(system)) == 0) {
		printf(MSGSTR(ALLBUSY, "all ports busy\n")); /*MSG*/
		exit(3);
	}
	if (i == -1) {
		printf(MSGSTR(LINKDOWN, "link down\n")); /*MSG*/
		(void)uu_unlock(uucplock);
		exit(3);
	}
	setbuf(stdout, NULL);
	loginit();

	/*
	 * Kludge, their's no easy way to get the initialization
	 *   in the right order, so force it here
	 */
	if ((PH = getenv("PHONES")) == NOSTR)
		PH = _PATH_PHONES;
	vinit();				/* init variables */
	setparity("none");/* QAR 4038 */	/* set the parity table */

	if ((i = speed(number(value(BAUDRATE)))) == (int)NULL) {
		printf(MSGSTR(BADBAUD, "tip: bad baud rate %d\n"), number(value(BAUDRATE))); /*MSG*/
		(void)uu_unlock(uucplock);
		exit(3);
	}

	/*
	 * Now that we have the logfile and the ACU open
	 *  return to the real uid and gid.  These things will
	 *  be closed on exit.  Swap real and effective uid's
	 *  so we can get the original permissions back
	 *  for removing the uucp lock.
	 */
        /*
         * However, if the phone number is to be read from the phones
         *  file instead of an environmental variable, do not return to the
         *  real uid and gid as access to the file requires these ids.
         */
        if (strcmp(PH,_PATH_PHONES) != 0)               /* QAR 01686 - DJP */
                user_uid();
	/*
	 * Hardwired connections require the
	 *  line speed set before they make any transmissions
	 *  (this is particularly true of things like a DF03-AC)
	 */
	if (HW)
		ttysetup(i);
	if (p = connect()) {
		printf(MSGSTR(EOT2, "\07%s\n[EOT]\n"), p); /*MSG*/
		daemon_uid();
		(void)uu_unlock(uucplock);
		exit(1);
	}
        /*
         * We are done reading the phones file.  If the real and effective
         * uids and gids were not swapped above, swap them now.
         */
        if (strcmp(PH,_PATH_PHONES) == 0)               /* QAR 01686 - DJP */
                user_uid();                             /* QAR 01686 - DJP */
	if (!HW)
		ttysetup(i);
cucommon:
	/*
	 * From here down the code is shared with
	 * the "cu" version of tip.
	 */

	ioctl(0, TIOCGETP, (char *)&defarg);
	ioctl(0, TIOCGETC, (char *)&defchars);
	ioctl(0, TIOCGLTC, (char *)&deflchars);
	ioctl(0, TIOCGETD, (char *)&odisc);
	arg = defarg;
	arg.sg_flags = ANYP | CBREAK;
	tchars = defchars;
	tchars.t_intrc = tchars.t_quitc = -1;
	ltchars = deflchars;
	ltchars.t_suspc = ltchars.t_dsuspc = ltchars.t_flushc
		= ltchars.t_lnextc = -1;

	raw();

	pipe(fildes); pipe(repdes);
	signal(SIGALRM, (void(*)(int)) timeout);

	/*
	 * Everything's set up now:
	 *	connection established (hardwired or dialup)
	 *	line conditioned (baud rate, mode, etc.)
	 *	internal data structures (variables)
	 * so, fork one process for local side and one for remote.
	 */
	printf(cumode ? MSGSTR(CONNED, "Connected\r\n") : MSGSTR(CONNECT2, "\07connected\r\n")); /*MSG*/ /*MSG*/
	if (pid = fork())
		tipin();
	else
		tipout();
	/*NOTREACHED*/
}

void
cleanup()
{

	daemon_uid();
	(void)uu_unlock(uucplock);
	if (odisc)
		ioctl(0, TIOCSETD, (char *)&odisc);
	exit(0);
}

/*
 * Muck with user ID's.  We are setuid to the owner of the lock
 * directory when we start.  user_uid() reverses real and effective
 * ID's after startup, to run with the user's permissions.
 * daemon_uid() switches back to the privileged uid for unlocking.
 * Finally, to avoid running a shell with the wrong real uid,
 * shell_uid() sets real and effective uid's to the user's real ID.
 */
static int uidswapped;

user_uid()
{
        if (uidswapped == 0) {
                setregid(egid, gid);
		setreuid(euid, uid);
                uidswapped = 1;
        }
}

daemon_uid()
{

        if (uidswapped) {
                setreuid(uid, euid);
                setregid(gid, egid);
                uidswapped = 0;
	}
}

shell_uid()
{

        setreuid(uid, uid);
        setregid(gid, gid);
}

/*
 * put the controlling keyboard into raw mode
 */
raw()
{
	ioctl(0, TIOCSETP, &arg);
	ioctl(0, TIOCSETC, &tchars);
	ioctl(0, TIOCSLTC, &ltchars);
	ioctl(0, TIOCSETD, (char *)&disc);
}


/*
 * return keyboard to normal mode
 */
unraw()
{

	ioctl(0, TIOCSETD, (char *)&odisc);
	ioctl(0, TIOCSETP, (char *)&defarg);
	ioctl(0, TIOCSETC, (char *)&defchars);
	ioctl(0, TIOCSLTC, (char *)&deflchars);
}

static	jmp_buf promptbuf;

/*
 * Print string ``s'', then read a string
 *  in from the terminal.  Handles signals & allows use of
 *  normal erase and kill characters.
 */
prompt(s, p)
	char *s;
	register char *p;
{
	register char *b = p;
	sig_t oint, oquit;

	stoprompt = 0;
	oint = signal(SIGINT, intprompt); 
	oquit = signal(SIGQUIT, SIG_IGN);
	unraw();
	printf("%s", s);
	if (setjmp(promptbuf) == 0)
		while ((*p = getchar()) != (char) EOF && *p != '\n')
			p++;
	*p = '\0';

	raw();
	signal(SIGINT, oint);
	signal(SIGQUIT, oquit); 
	return (stoprompt || p == b);
}

/*
 * Interrupt service routine during prompting
 */
void
intprompt()
{

	signal(SIGINT, SIG_IGN);
	stoprompt = 1;
	printf("\r\n");
	longjmp(promptbuf, 1);
}

/*
 * ****TIPIN   TIPIN****
 */
tipin()
{
	char gch, bol = 1;
        char *parity;
        parity = value(PARITY);

	/*
	 * Kinda klugey here...
	 *   check for scripting being turned on from the .tiprc file,
	 *   but be careful about just using setscript(), as we may
	 *   send a SIGEMT before tipout has a chance to set up catching
	 *   it; so wait a second, then setscript()
	 */

	if (boolean(value(SCRIPT))) {
		sleep(1);
		setscript();
	}

	while (1) {
                if(strncmp(parity,"none",4) != 0)
                        gch = getchar()&0177;
                else
                        gch = getchar();
		if ((gch == character(value(ESCAPE))) && bol) {
			if (!(gch = escape()))
				continue;
		} else if (!cumode && gch == character(value(RAISECHAR))) {
			boolean(value(RAISE)) = !boolean(value(RAISE));
			continue;
		} else if (gch == '\r') {
			bol = 1;
			pwrite(FD, &gch, 1);
			if (boolean(value(HALFDUPLEX)))
				printf("\r\n");
			continue;
		} else if (!cumode && gch == character(value(FORCE)))
                       {
                                if(strncmp(parity,"none",4) != 0)
                                        gch = getchar()&0177;
                                else
                                        gch = getchar();
                        }
		bol = any(gch, value(EOL));
		if (boolean(value(RAISE)) && islower(gch))
			gch = toupper(gch);
		pwrite(FD, &gch, 1);
		if (boolean(value(HALFDUPLEX)))
			printf("%c", gch);
	}
}

/*
 * Escape handler --
 *  called on recognition of ``escapec'' at the beginning of a line
 */
escape()
{
	register char gch;
	register esctable_t *p;
	char c = character(value(ESCAPE));
        char *parity;
        parity = value(PARITY);


        if(strncmp(parity,"none",4) != 0)
                gch = (getchar()&0177);
        else
                gch = getchar();
	for (p = etable; p->e_char; p++)
		if (p->e_char == gch) {
			if ((p->e_flags&PRIV) && uid)
				continue;
			printf("%s", ctrl(c));
			(*p->e_func)(gch);
			return (0);
		}
	/* ESCAPE ESCAPE forces ESCAPE */
	if (c != gch)
		pwrite(FD, &c, 1);
	return (gch);
}

speed(n)
	int n;
{
	register int *p;

	for (p = bauds; *p != -1;  p++)
		if (*p == n)
			return (p - bauds);
	return ((int)NULL);
}

any(c, p)
	register char c, *p;
{
	while (p && *p)
		if (*p++ == c)
			return (1);
	return (0);
}

size(s)
	register char	*s;
{
	register int i = 0;

	while (s && *s++)
		i++;
	return (i);
}

char *
interp(s)
	register char *s;
{
	static char buf[512];
	register char *p = buf, c, *q;
        register int bufsiz = 512;


        while ( (c = *s++) && (--bufsiz > 0) ) {
		for (q = "\nn\rr\tt\ff\033E\bb"; *q; q++)
			if (*q++ == c) {
				*p++ = '\\'; *p++ = *q;
				goto next;
			}
		if (c < 040) {
			*p++ = '^'; *p++ = c + 'A'-1;
		} else if (c == 0177) {
			*p++ = '^'; *p++ = '?';
		} else
			*p++ = c;
	next:
		;
	}
	*p = '\0';
	return (buf);
}

char *
ctrl(c)
	char c;
{
	static char s[3];

	if (c < 040 || c == 0177) {
		s[0] = '^';
		s[1] = c == 0177 ? '?' : c+'A'-1;
		s[2] = '\0';
	} else {
		s[0] = c;
		s[1] = '\0';
	}
	return (s);
}

/*
 * Help command
 */
help(c)
	char c;
{
	register esctable_t *p;


	printf("%c\r\n", c);
	for (p = etable; p->e_char; p++) {
		if ((p->e_flags&PRIV) && uid)
			continue;
		printf("%2s", ctrl(character(value(ESCAPE))));
		printf("%-2s %c   %s\r\n", ctrl(p->e_char),
			p->e_flags&EXP ? '*': ' ', p->e_help);
	}
}

/*
 * Set up the "remote" tty's state
 */
ttysetup(speed)
	int speed;
{
	unsigned bits = LDECCTQ;

	arg.sg_ispeed = arg.sg_ospeed = speed;
	arg.sg_flags = RAW;
	if (boolean(value(TAND)))
		arg.sg_flags |= TANDEM;
	ioctl(FD, TIOCSETP, (char *)&arg);
	ioctl(FD, TIOCLBIS, (char *)&bits);
}

/*
 * Return "simple" name from a file name,
 * strip leading directories.
 */
char *
sname(s)
	register char *s;
{
	register char *p = s;

	while (*s)
		if (*s++ == '/')
			p = s;
	return (p);
}

static char partab[0200];
static int bits8;

/*
 * Do a write to the remote machine with the correct parity.
 * We are doing 8 bit wide output, so we just generate a character
 * with the right parity and output it.
 */
pwrite(fd, buf, n)
	int fd;
	char *buf;
	register int n;
{
	register int i;
	register char *bp;
	extern int errno;
        char *parity;
        parity = value(PARITY);


	bp = buf;
	if (bits8 == 0)
	    for (i = 0; i < n; i++) {
		                if(strncmp(parity,"none",4) != 0)
                        *bp = partab[(*bp) & 0177];
                else
                        *bp = partab[(*bp)];

		bp++;
	    }
	if (write(fd, buf, n) < 0) {
		if (errno == EIO)
			abort(MSGSTR(LOSTCARRIER, "Lost carrier.")); /*MSG*/
		/* this is questionable */
		perror(MSGSTR(WRITEIT, "write")); /*MSG*/
	}
}

/*
 * Build a parity table with appropriate high-order bit.
 */
setparity(defparity)
	char *defparity;
{
	register int i;
	char *parity;
	extern char evenpartab[];

	if (value(PARITY) == NOSTR)
		value(PARITY) = defparity;
	parity = value(PARITY);
	if (equal(parity, "none")) {
                bits8 = 1;
                return;
        } else
                bits8 = 0;
	for (i = 0; i < 0200; i++)
		partab[i] = evenpartab[i];
	if (equal(parity, "even"))
		return;
	if (equal(parity, "odd")) {
		for (i = 0; i < 0200; i++)
			partab[i] ^= 0200;	/* reverse bit 7 */
		return;
	}
	if (equal(parity, "zero")) {
		for (i = 0; i < 0200; i++)
			partab[i] &= ~0200;	/* turn off bit 7 */
		return;
	}
	if (equal(parity, "one")) {
		for (i = 0; i < 0200; i++)
			partab[i] |= 0200;	/* turn on bit 7 */
		return;
	}
	fprintf(stderr, MSGSTR(UNKPARITY, "%s: unknown parity value\n"), PA); /*MSG*/
	fflush(stderr);
}

load_sep() {
	char *mes;
	int i;
	char *defaults[] = {"second", "minute", "hour" };

	for (i=0; i<3; i++) {
		mes = MSGSTR(LITSECOND + i, defaults[i]);
		sep[i] = (char *)malloc(strlen(mes) + 1);
		if (sep[i] != NULL)
			strcpy(sep[i], mes);
		else {
			fprintf(stderr, MSGSTR(NOMEM, "tip: Out of memory\n"));
			exit(1);
		}
	}
}

load_etable() {

	int i = 0;
	char *mes;

	extern char 	  *etable_default[];

	for (i=0; etable_default[i] != NULL; i++) {
		mes = MSGSTR(HELP01 + i, etable_default[i]);
		etable[i].e_help = (char *)malloc(strlen(mes) + 1);
		if (etable[i].e_help != NULL) 
			strcpy(etable[i].e_help, mes);
		else {
			fprintf(stderr, MSGSTR(NOMEM, "tip: Out of memory\n"));
			exit(1);
		}
	}
}
