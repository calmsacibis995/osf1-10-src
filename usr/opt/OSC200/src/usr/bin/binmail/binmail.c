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
static char	*sccsid = "@(#)$RCSfile: binmail.c,v $ $Revision: 4.3.8.4 $ (DEC) $Date: 1993/12/03 15:23:22 $";
#endif 
/*
 * "binmail"   /usr/binmail
 *
 *	"@(#)mail.c	4.18 (Berkeley) 9/9/83";
 *
 *	EDIT HISTORY:
 *
 *	10-Oct-91  01 Todd Kaehler
 *
 *		Mail would leave world readable /tmp/maXXXXX files behind
 *		which contained a copy of there /var/spool/mail/<user> file.
 *		This was caused by opening the temp file as root, then
 *		entering printmail() and doing a setuid(getuid()).  When
 *		exiting the process could not remove the root owned file.
 *		The temp file is now opened after the setuid(getuid()).
 *
 *	10-Oct-91  00 Todd Kaehler
 *
 *		Corrected a race condition in sendmail().  Changed from
 *		"open for append then lock" to "lock then open for append".
 *		This prevents 2 processes having the same spool file open
 *		for append at the same time, resulting in the second process
 *		overwritting the first.
 *
 *	26-Mar-91  Alf Vetvik
 *
 *		Nuke'd the old locking functions; lock(), unlock(), lock1()
 *		and getargs(). Replaced the whole shabang with a new lock()
 *		function. This affects printmail(), copybak() and sendmail().
 *		The old locking scheme, using system load figures, was 
 *		fairly unusable in multiprocessor environments. It was also
 *		unreliable across NFS. The new lock() uses the lockf(3) 
 *		function. 'lockf' should work reliably on multiprocessors. 
 *	        In theory lockf should work across NFS; however, it doesn't 
 *		appear to work over Digital OSF/1 ADK.  NFS should therefore 
 *		not be supported. This may eventually change if lockf is fixed.
 *		To use the old locking scheme use the compile option LOCKFILE.
 *		Note: this locking scheme is not backwards compatible; therefore
 *		it will not work with current 4.3 ucb/mail, MH etc.
 *
 *      24-Sep-90  Alf Vetvik
 *
 *		Merged in some of the BSD4.3 version of /bin/mail. Removed 
 *		the 1000 pointer limit of newargv in bulkmail, by dynamic
 *		allocation. Increased the size of truename from 1024 to 1025,
 *		to acount for the check bit. Change name of delete() to delex().
 *		Removed the parameter 'fromaddr' from sendmail().  Removed some
 *		redundant code in sendrmt(). New error message in copylet(). 
 *		A lot of de-linting and various cosmetic changes.
 *
 *	19-Jan-89  John Haxby
 *		Having recoded safefile, the code that ensures that the file
 *		created with the right owner and mode turned out to be
 *		somewhat dodgy.  Changed MAILMODE to be the mode that the file
 *		is created with rather than the umask and removed the
 *		associated (and somewhat redundant) calls to umask().  Removed
 *		a misleading comment about calling chown() (it wasn't) and
 *		moved the setreuid() call from immediately before fdopen()
 *		(which doesn't open the file) to immediately before the
 *		safefile() which does.  Note that this UID swapping is
 *		primarily for the sake of NFS since root doesn't have
 *		sufficient privilege across NFS -- the call to safefile() to
 *		create the dead.letter file is not changed since we don't have
 *		a specific UID to set ourselves to.  Note that the code, at
 *		present, requires the spool directory to be world writeable,
 *		we should really have fallback code to try create the maildrop
 *		as root when we can't create it as the user.
 *
 *      20-Dec-89  John Haxby/Paul Sharpe
 *              Recoded safefile() to return a file-descriptor, but only when
 *              the file is (hopefully) definitely 'safe': else race
 *              conditions may allow unauthorised mailbox access.
 *
 *	15-Jun-88  John Haxby
 *		Increased size of 'truename' to prevent SIGSEGV
 *		(which, incidentally, causes endless looping through
 *	         the signal)
 *
 *	08-Jun-88  Mark Parenti
 *		Changed signal handlers to void.
 *
 *	22-Jan-88  John Haxby
 *		Added -e flag for X/OPEN.
 *
 *	27-Feb-1987  Ray Glaser
 *		Added logic to extend the wait time on stale lock
 *		files to be a function of the system load ave.
 *
 *	12-Feb-1987  Ray Glaser 
 *		Massive revision to the file locking logic for NFS.
 *
 *	15-Dec-1986  Marc Teitelbaum  - 0001
 *		Only chown spool mailfile if we created it.
 *		Security reasons.  Also, bump timeout on
 *		waiting for lock to 60 seconds.  30 seems
 *		too low.  Flock would be preferable, but no
 *		time right now and much gastric distress.
 *
 *	aps00 10/26/83	-- added check for UID of uucp otherwise, would
 *				fail if mail is comming from off system,
 *				via uucp.
 *	02-Apr-84	mah.  Fix for gethostname for queued file.  This
 *				is to reflect 4.21 (Berkeley).
 */

#include <ctype.h>
#include <stdio.h>
#include <locale.h> /*GAG*/
#include <errno.h>
#include <pwd.h>
#include <utmp.h>
#include <signal.h>
#include <nlist.h>
#include <setjmp.h>
#include <syslog.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h> 
#include <string.h>
#include <unistd.h>
#include <userpw.h>

#include "binmail_msg.h" /*GAG*/
nl_catd scmc_catd;
#define MSGSTR(Num,Str) catgets(scmc_catd,MS_binmail,Num,Str)

#define	PGMNAME		"binmail"

#define SENDMAIL	"/usr/lib/sendmail"
/*copylet flags */
	/*remote mail, add rmtmsg */
#define REMOTE	1
	/* zap header and trailing empty line */
#define ZAP	3
#define ORDINARY 2
#define	FORWARD	4
#define	LSIZE	256
#define	MAXLET	1024	/* maximum number of letters */
#define	MAILMODE 0600	/* mode of created mail */

#define	DOUBLE_LK	/* Enable user-tunable locking style */

extern char *calloc();
extern char *mktemp();

/* Old locking scheme. */

int OVERRIDE = 0;	/* Flag net if lock file overridden */
int FIRSTSLEEP = 1;
int LOCKSLEEP = 4;	/* Basic # seconds to sleep between checks for
			 * name.lock file existance and to ck the
			 * peak load ave.
			 */
#define OLOCKSLEEPS 19 
int LOCKSLEEPS = OLOCKSLEEPS;	/* Basic # of times to sleep & wait for 
			 	 * name.lock  file to disappear of its' 
	 			 * own accord before we blow it away.
		 		 */ 
#define	LF_TMOUT	60	/* timeout for lockf'ing the mailfile */
				/* This should be larger than the ASE vals */

unsigned  char	line[LSIZE];

struct let {
	long	adr;
	char	change;
} let[MAXLET];
int	nlet	= 0;
char BELL[] = PGMNAME; /*GAG*/
extern char	*getenv();
char	lettmp[PATH_MAX+1], *gettmpdir(); /*2-92 TK*/
char	maildir[] = "/usr/spool/mail/";
char	mailfile[PATH_MAX+1];
char	dead[] = "dead.letter";
char	forwmsg[] = " forwarded\n";
FILE	*tmpf;
FILE	*malf;
char	*my_name;
extern char	*getlogin();
extern struct	passwd	*getpwuid();
int	error;
int	changed;
int	forward;
char	from[] = "From ";
extern long	ftell();
long    iop;
void	delex(); 
extern char	*ctime();
int	flgf;
int	flgp;
int	delflg = 1;
int	hseqno;
jmp_buf	sjbuf;
int	rmail;

main(argc, argv)
char **argv;
{
	register i;
	char sobuf[BUFSIZ];
	struct passwd *pwent;

	(void) setlocale(LC_ALL,""); /*GAG*/
	scmc_catd = catopen(MF_BINMAIL,NL_CAT_LOCALE);

	setbuf(stdout, sobuf);
	sprintf(lettmp, "%s/maXXXXXX", gettmpdir()); /*2-92 TK*/
	(void) mktemp(lettmp);
	(void) unlink(lettmp);
	my_name = getlogin();
	if (my_name == NULL || ((int) strlen(my_name)) == 0) {
		pwent = getpwuid(getuid());
		if (pwent==NULL)
			my_name = "???";
		else
			my_name = pwent->pw_name;
	}
	else {
                pwent = getpwnam(my_name);
		if (!pwent || getuid() != pwent->pw_uid) {
			pwent = getpwuid(getuid());
			my_name = pwent->pw_name;
		}
	}
	if(setjmp(sjbuf)) done();
	for (i=SIGHUP; i<=SIGTERM; i++)
		setsig(i, delex);
	if (argv[0][0] == 'r')
		rmail++;
	if (argv[0][0] != 'r' &&	/* no favors for rmail*/
	   (argc == 1 || argv[1][0] == '-' && !any(argv[1][1], "rhd")))
		printmail(argc, argv);
	else
		bulkmail(argc, argv);
	done();
}

setsig(i, f)
int i;
void (*f)();
{
	if( ((void (*)()) signal(i, SIG_IGN))!=((void (*)()) SIG_IGN))
		(void)signal(i, f);
}

any(c, str)
	register int c;
	register char *str;
{
	while (*str)
		if (c == *str++)
			return(1);
	return(0);
}

printmail(argc, argv)
char **argv;
{
	int flg, i, j, print, check = 0;
	int locked;
	char *p, *getarg();
	extern char *optarg;
	extern int opterr;
	extern int optind;
	int c;
	char	resp[LSIZE];
	char	lfil[LSIZE];
	struct stat statb;

	(void) setuid(getuid());
	tmpf = fopen(lettmp, "w");
	/*
	 * 01 TK open lettmp after setuid(getuid()) so process can unlink().
	 */
	if (tmpf == NULL) {
		fprintf(stderr,  MSGSTR(M_MSG_0, /*GAG*/
			"%s: can't open %s for writing\n"),BELL,lettmp);
		error = EX_CANTCREAT;
		done();
	}
	chmod(lettmp, S_IRUSR | S_IWUSR);
	cat(mailfile, maildir, my_name);
	if (stat(mailfile, &statb) >= 0 && (statb.st_mode & S_IFMT) == S_IFDIR) {
		strcat(mailfile, "/");
		strcat(mailfile, my_name);
	}
	for (; argc>1; argv++, argc--) {
                if (argv[1][0] != '-')
                        break;
                switch (argv[1][1]) {

                case 'p':
                        flgp++;
                        /* fall thru... */
                case 'q':
                        delflg = 0;
                        break;
                case 'f':
			flgf++;
			opterr = 0;
			c = getopt(argc, argv, "f:epqrbh");
			if (c == 'f') {
				strcpy(mailfile, optarg);
				if (optind == 3) {
					argv++; argc--;
				}
			} else {
				/* Bad -f option */
				fprintf(stderr,  MSGSTR(M_MSG_1,
				"usage:  %s [-epqbhr] [-f file]\n    or  %s [-d] [-r name] [-h N] user ...\n"),
				BELL,BELL);
				error = EX_USAGE;
				done();
			}
                        break;

                case 'e':
			check = 1;
			break;
                case 'b': 
                case 'r':
                case 'h':
                        forward = 1;
                        break;

                default:
			fprintf(stderr,  MSGSTR(M_MSG_1,
				"usage:  %s [-epqbhr] [-f file]\n    or  %s [-d] [-r name] [-h N] user ...\n"),
				BELL,BELL);
			error = EX_USAGE;
			done ();
		}
	}

/* New locking scheme */
	do
	{
		malf = fopen(mailfile, "r+");
		if (malf == NULL) 
			if (check) {
				error = 1;
				done();
			} else {
				/* if -f was specified, then we have an error, 
				 * otherwise, just say no mail
				 */
				if (flgf) {
					fprintf(stderr,MSGSTR(M_MSG_5,"%s: cannot open %s: "),BELL,mailfile);
					perror("");
				} else
					printf(MSGSTR(M_MSG_6,"No mail.\n") );
				return;
			}
		locked = lock (fileno (malf));
		if (locked == -1){
		    /* Error is set within lock() */
		    done();
		}
		if (!locked)
			fclose (malf); 
	} while (!locked);
	lock_file(mailfile);
	copymt(malf, tmpf);
	(void) fclose(tmpf);
	unlock_file();
	(void) fclose(malf);


	if (check) {
		error = nlet == 0;
		done();
	}
	tmpf = fopen(lettmp, "r");

	changed = 0;
	print = 1;
	for (i = 0; i < nlet; ) {
		j = forward ? i : nlet - i - 1;
		if(setjmp(sjbuf)) {
			print=0;
		} else {
			if (print)
				copylet(j, stdout, ORDINARY);
			print = 1;
		}
		if (flgp) {
			i++;
			continue;
		}
		(void) setjmp(sjbuf);
		(void) fprintf(stdout, "? ");
		(void) fflush(stdout);
		if (fgets(resp, LSIZE, stdin) == NULL)
			break;
		switch (resp[0]) {

		default:
			printf(MSGSTR(M_MSG_12,"usage\n") ); /*GAG*/
		case '?':
			print = 0;
			printf(MSGSTR(M_MSG_56,"q\t\tquit\n"));
			printf(MSGSTR(M_MSG_57,
				"x\t\texit without changing mail\n"));
			printf(MSGSTR(M_MSG_58,"p\t\tprint\n"));
			printf(MSGSTR(M_MSG_59,
				"s [file]\tsave (default mbox)\n"));
			printf(MSGSTR(M_MSG_60,
				"w [file]\tsame without header\n"));
			printf(MSGSTR(M_MSG_61,"-\t\tprint previous\n"));
			printf(MSGSTR(M_MSG_63,"+\t\tnext (no delete)\n"));
			printf(MSGSTR(M_MSG_62,"d\t\tdelete\n"));
			printf(MSGSTR(M_MSG_64,"m [user]\tmail to user\n"));
			printf(MSGSTR(M_MSG_65,"! cmd\t\texecute cmd\n"));
			break;

		case '+':
		case 'n':
		case '\n':
			i++;
			break;
		case 'x':
			changed = 0;
		case 'q':
			goto donep;
		case 'p':
			break;
		case '^':
		case '-':
			if (--i < 0)
				i = 0;
			break;
		case 'y':
		case 'w':
		case 's':
			flg = 0;
			if (resp[1] != '\n' && resp[1] != ' ') {
				printf(MSGSTR(M_MSG_14,"invalid command\n")); /*GAG*/
				flg++;
				print = 0;
				continue;
			}
			if (resp[1] == '\n' || resp[1] == '\0') {
				p = getenv("HOME");
				if(p != 0)
					cat(resp+1, p, "/mbox");
				else
					cat(resp+1, "", "mbox");
			}
			for (p = resp+1; (p = getarg(lfil, p)) != NULL; ) {
				malf = fopen(lfil, "a");
				if (malf == NULL) {
					fprintf(stderr, MSGSTR(M_MSG_15, /*GAG*/
						"%s: cannot append to %s\n")
						,BELL,lfil);
					flg++;
					continue;
				}
				copylet(j, malf, resp[0]=='w'? ZAP: ORDINARY);
				(void) fclose(malf);
			}
			if (flg)
				print = 0;
			else {
				let[j].change = 'd';
				changed++;
				i++;
			}
			break;
		case 'm':
			flg = 0;
			if (resp[1] == '\n' || resp[1] == '\0') {
				i++;
				continue;
			}
			if (resp[1] != ' ') {
				printf(MSGSTR(M_MSG_17,"invalid command\n")); /*GAG*/
				flg++;
				print = 0;
				continue;
			}
			for (p = resp+1; (p = getarg(lfil, p)) != NULL; )
				if (!sendrmt(j, lfil, "/bin/mail"))	/* couldn't send it */
					flg++;
			if (flg)
				print = 0;
			else {
				let[j].change = 'd';
				changed++;
				i++;
			}
			break;
		case '!':
			(void) system(resp+1);
			(void) printf("!\n");
			print = 0;
			break;
		case 'd':
			let[j].change = 'd';
			changed++;
			i++;
			if (resp[1] == 'q')
				goto donep;
			break;
		}
	}
   donep:
	if (changed)
		copyback();
	if (nlet==0)
		printf(MSGSTR(M_MSG_6,"No mail.\n") ); /*GAG*/
}

/* New locking scheme */

copyback ()
/* copy temp or whatever back to /usr/spool/mail */
{
	register i, c;
	int fd, new = 0;
	struct stat stbuf;
	int locked;


	(void) signal((int) SIGINT, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	do
	{
		fd = open (mailfile, O_RDWR);
		locked = lock (fd);
		if (locked == -1){
		    /*  Error is set within lock() */
		    done();
		}
		if (!locked)
			close(fd);
	} while (!locked);
	lock_file (mailfile);
	malf = fdopen (fd, "r+");

	(void) stat(mailfile, &stbuf);
	if (stbuf.st_size != let[nlet].adr) {	/* new mail has arrived */
		if (malf == NULL) {
			fprintf(stderr,MSGSTR(M_MSG_19, /*GAG*/
				"%s: can't re-read %s\n"),BELL,mailfile);
			error = EX_CANTCREAT;
			done();
		}
		(void) fseek(malf, let[nlet].adr, 0);
		(void) fclose(tmpf);
		tmpf = fopen(lettmp, "a");
		(void) fseek(tmpf, let[nlet].adr, 0);
		while ((c = fgetc(malf)) != EOF)
			(void) fputc(c, tmpf);
		(void) fclose(tmpf);
		tmpf = fopen(lettmp, "r");
		let[++nlet].adr = stbuf.st_size;
		new = 1;
	}
	(void) fseek (malf, 0L, SEEK_SET);
	(void) ftruncate (fd, 0);
	if (malf == NULL) {
		fprintf(stderr,MSGSTR(M_MSG_20, /*GAG*/
			"%s: can't re-write %s\n"),BELL,mailfile);
		error = EX_CANTCREAT;
		done();
	}
	for (i = 0; i < nlet; i++)
		if (let[i].change != 'd') {
			copylet(i, malf, ORDINARY);
		}
	(void) fclose(malf);
	unlock_file();
	if (new)
		printf(MSGSTR(M_MSG_25,"new mail arrived\n") ); /*GAG*/
}


copymt(f1, f2)	/* copy mail (f1) to temp (f2) */
FILE *f1, *f2;
{
	long nextadr;
	nlet = nextadr = 0;
	let[0].adr = 0;
	while (fgets((char *)line, LSIZE, f1) != NULL) {
		if (isfrom(line)) {
			if (nlet > MAXLET) {
				fprintf(stderr,MSGSTR(M_MSG_78, /*TK*/
					"%s: mail file %s exceeds message number maximum of %d\n"),BELL,mailfile,MAXLET);
				error = EX_SOFTWARE;
				done();
			}
			let[nlet++].adr = nextadr;
		}
		nextadr += (int)  strlen((char *) line);
		(void) fputs((char *)line, f2);
	}
	let[nlet].adr = nextadr;	/* last plus 1 */
}

copylet(n, f, type)
int n, type;
FILE *f;
{
	int ch;
	long k;
	char hostname[32];

	(void) fseek(tmpf, let[n].adr, 0);
	k = let[n+1].adr - let[n].adr;
	while(k-- > 1 && (ch=fgetc(tmpf))!='\n')
		if(type!=ZAP) (void) fputc(ch,f);
	switch (type) {
        case REMOTE:
                (void) gethostname(hostname, sizeof (hostname));
                (void) fprintf(f, " remote from %s\n", hostname);
                break;

        case FORWARD:
                (void) fprintf(f, forwmsg);
                break;

        case ORDINARY:
		(void) fputc(ch,f);
                break;

        case ZAP:
                break;

        default:
		printf(MSGSTR(M_MSG_70, /*GAG*/
			"%s: Bad letter type %d to copylet.\n"), BELL, type);
	}
	while(k-->1)
		(void) fputc(ch=fgetc(tmpf), f);
	if(type!=ZAP || ch!= '\n')
		(void) fputc(fgetc(tmpf), f);
}

isfrom(lp)
unsigned char *lp;
{
	register char *p;

	for (p = from; *p; )
		if (*lp++ != *p++)
			return(0);
	return(1);
}

bulkmail(argc, argv)
int argc; 
char **argv;
{
	char truename[1025];
	/* truename = maximum permitted by sendmail */
	int first;
	register char *cp;
	int gaver = 0;
	char **newargv;
	register char **ap;
	register char **vp;
	int dflag;
	int	mald;		/* 'safe' file desc returned for mail spool */
	int	i;

	dflag = 0;
	/*
	 * 01 TK open lettmp after start of bulkmail() (after any possible
	 *       setuid(getuid()) call).
	 */
	tmpf = fopen(lettmp, "w");
	if (tmpf == NULL) {
		fprintf(stderr,  MSGSTR(M_MSG_0, /*GAG*/
			"%s: can't open %s for writing\n"),BELL,lettmp);
		error = EX_CANTCREAT;
		done();
	}
	chmod(lettmp, S_IRUSR | S_IWUSR);
	newargv = (char **) calloc ((char *) (argc + 1), sizeof (char *)); 
	for (vp = argv, ap = newargv + 1; (*ap = *vp++) != 0; ap++)
	{
		if (ap[0][0] == '-' && ap[0][1] == 'd')
			dflag++;
	}
	if (!dflag)
	{
		/* give it to sendmail, rah rah! */
		(void) unlink(lettmp);
		ap = newargv+1;
		if (rmail)
			*ap-- = "-s";
		*ap = "-sendmail";
		(void) setuid(getuid());
		(void) execv(SENDMAIL, ap);
		perror(SENDMAIL);
		exit(EX_UNAVAILABLE);
	}
	(void) free ((char *) newargv); 

	truename[0] = 0;
	line[0] = '\0';

	/*
	 * When we fall out of this, argv[1] should be first name,
	 * argc should be number of names + 1.
	 */

	while (argc > 1 && *argv[1] == '-') {
		cp = *++argv;
		argc--;
		switch (cp[1]) {
		case 'r':
			if (argc <= 0) {
				printf(MSGSTR(M_MSG_12,"usage\n") ); /*GAG*/
				error=EX_USAGE;
				done();
			}
			gaver++;
			(void) strcpy(truename, argv[1]);
			(void) fgets((char *)line, LSIZE, stdin);
			if (strncmp("From", (char *)line, 4) == 0)
				line[0] = '\0';
			argv++;
			argc--;
			break;

		case 'h':
			if (argc <= 0) {
				printf(MSGSTR(M_MSG_12,"usage\n") ); /*GAG*/
				error=EX_USAGE;
				done();
			}
			hseqno = atoi(argv[1]);
			argv++;
			argc--;
			break;

		case 'd':
			break;
		
		default:
			printf(MSGSTR(M_MSG_12,"usage\n") ); /*GAG*/
			error=EX_USAGE;
			done();
		}
	}
	if (argc <= 1) {
		fprintf(stderr,  MSGSTR(M_MSG_1,
			"usage:  %s [-epqbhr] [-f file]\n    or  %s [-d] [-r name] [-h N] user ...\n"),
				BELL,BELL);
		error=EX_USAGE;
		done();
	}
	if (gaver == 0)
		(void) strcpy(truename, my_name);
	/*
	if (argc > 4 && strcmp(argv[1], "-r") == 0) {
		(void) strcpy(truename, argv[2]);
		argc -= 2;
		argv += 2;
		(void) fgets(line, LSIZE, stdin);
		if (strncmp("From", line, 4) == 0)
			line[0] = '\0';
	} else
		(void) strcpy(truename, my_name);
	*/
	(void) time((time_t *) &iop);
	(void) fprintf(tmpf, "%s%s %s", from, truename, ctime((time_t *) &iop));
	iop = ftell(tmpf);
	flgf = 1;
	for (first = 1;; first = 0) {
		if (first && line[0] == '\0' && fgets((char *)line, LSIZE, stdin) == NULL)
			break;
		if (!first && fgets((char *)line, LSIZE, stdin) == NULL)
			break;
		if (line[0] == '.' && line[1] == '\n' && isatty(fileno(stdin)))
			break;
		if (isfrom(line))
			(void) fputs(">", tmpf);
		(void) fputs((char *)line, tmpf);
		flgf = 0;
	}
	(void) fputs("\n", tmpf);
	nlet = 1;
	let[0].adr = 0;
	let[1].adr = ftell(tmpf);
	(void) fclose(tmpf);
	if (flgf)
		return;
	tmpf = fopen(lettmp, "r");
	if (tmpf == NULL) {
		fprintf(stderr,MSGSTR(M_MSG_32, /*GAG*/
			"%s: cannot reopen %s for reading\n"),BELL,lettmp);
		return;
	}

	/*  Some intelligence on delivery.  Try to deliver to all.
	**  If only a single user, we can give much better errors.
	**  This allows things like EX_TEMPFAIL to be propagated up,
	**  and leave the mail queued on our local machine.
	**  If multiple users, we default to EX_UNAVAILABLE, and
	**  bounce the message back.  - RW
	*/
	for (i = argc; --i > 0; ){
	    if (!sendmail(0, *++argv )){
		if (!error)
		    error = EX_UNAVAILABLE;
	    }
	}
	if (error && argc > 2)
	    error = EX_UNAVAILABLE;

	(void) unlink(lettmp);

	if (error && (mald = safefile(dead,-1)) >= 0) {
		(void) setuid(getuid());
		malf = fdopen(mald, "w");
		if (malf == NULL) {
			fprintf(stderr,MSGSTR(M_MSG_34, /*GAG*/
				"%s: cannot create %s\n"),BELL,&dead[0]);
			(void) fclose(tmpf);
			return;
		}
		copylet(0, malf, ZAP);
		(void) fclose(malf);
		printf(MSGSTR(M_MSG_36,"Mail saved in %s\n"),&dead[0]); /*GAG*/
	}
	(void) fclose(tmpf);
}

int sendrmt(n, name, rcmd)
int n;
char *name;
char *rcmd;
{
	FILE *rmf, *popen();
	register char *p;
	char rsys[64], cmd[64];
	register local, pid;
	int sts;
	pid_t wait(int*);

	local = 0;
	if (*name=='!')
		name++;
	for(p=rsys; *name!='!'; *p++ = *name++) {
                if (p - rsys > sizeof(rsys)) {
			 printf(MSGSTR(M_MSG_71, /*GAG*/
				"remote system name too long\n"));
                        return(0);
                }
		if (*name=='\0') {
			local++;
			break;
		}
	}
	*p = '\0';
	if ((!local && *name=='\0') || (local && *rsys=='\0')) {
		fprintf(stderr,MSGSTR(M_MSG_38,"null name\n") ); /*GAG*/
		return(0);
	}
	if ((pid = fork()) == -1) {
		printf(MSGSTR(M_MSG_72, /*GAG*/
			"%s: can't create proc for remote\n"),BELL);
		return(0);
	}
	if (pid) {
		while ((pid_t) wait(&sts) != pid) {
			if ((pid_t) wait(&sts)==-1)
				return(0);
		}
		return(!sts);
	}
	(void) setuid(getuid());
	if (local)
		(void) sprintf(cmd, "%s %s", rcmd, rsys);
	else {
		if ((int)index(name+1, '!'))
			(void) sprintf(cmd, "uux - %s!rmail \\(%s\\)", rsys, name+1);
		else
			(void) sprintf(cmd, "uux - %s!rmail %s", rsys, name+1);
	}
	if ((rmf=popen(cmd, "w")) == NULL) {
		(void) unlink(lettmp);
		exit(1);
	}
	copylet(n, rmf, local ? !strcmp(rcmd, "/bin/mail") ? FORWARD : ORDINARY : REMOTE);
	(void) unlink(lettmp);
	exit(pclose(rmf) != 0);
/* NOTREACHED */
}

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
struct sockaddr_in biffaddr;

sendmail(n, name) 
int n;
char *name;
{
	/* char file[100];  - replaced with the global var mailfile */
	register char *p;
	struct passwd *pw, *getpwnam();
	struct stat statb;
	char buf[128];
	int realuser;
	int f;
	struct hostent *hp = NULL;
	struct servent *sp = NULL;
	int mald;	/* 'Safe' file descriptor for mail spool file. */
	int locked;

	for(p=name; *p!='!'&&*p!='^' &&*p!='\0'; p++)
		;
	if (*p == '!'|| *p=='^')
		return(sendrmt(n, name, (char *) 0));
	if ((pw = getpwnam(name)) == NULL) {
		fprintf(stderr,MSGSTR(M_MSG_44, /*GAG*/
			"%s: can't send to %s\n"),BELL,name);
		return(0);
	}
	cat(mailfile, maildir, name);
	if (stat(mailfile, &statb) >= 0 && (statb.st_mode & S_IFMT) == S_IFDIR) {
		strcat(mailfile, "/");
		strcat(mailfile, name);
	}
	realuser = getuid();
	(void) setreuid(0,pw->pw_uid);

/* New locking scheme */
	for(;;)
	{
		if ((mald = safefile(mailfile, pw->pw_uid)) < 0) {
			(void) setuid (0);
			(void) setreuid (realuser, 0);
			return(0);
		}
		if ((locked = lock (mald)) == 1)
		    break;
		close(mald);
		if (locked == -1){
		    (void) setuid (0);
		    (void) setreuid (realuser, 0);
		    return(0);
		}
		
	}
	lock_file ( mailfile );

	/* changed "A" to "a" below */
	/*
	 * 00 TK open for append *after* the lock has been made.
	 */
	malf = fdopen(mald, "a");
	if (malf == NULL) {
		perror("fdopen");
		fprintf(stderr,MSGSTR(M_MSG_46, /*GAG*/
			"%s: cannot append to %s\n"), BELL,mailfile);
		(void) setuid(0);
		(void) setreuid(realuser,0);
		return(0);
	}

	/* Notify interersted parties via biff */
	{
		char hostbuf[256];
		(void) gethostname(hostbuf, sizeof (hostbuf));
		hp = gethostbyname(hostbuf);
		sp = getservbyname("biff", "udp");
		if (hp && sp) {
			f = socket(AF_INET, SOCK_DGRAM, 0);
			(void) sprintf(buf, "%s@%d\n", name, ftell(malf)); 
		}
	}
	copylet(n, malf, ORDINARY);
	(void) setreuid(0,pw->pw_uid);
	if (hp && sp) {
		biffaddr.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr,(char *)  &biffaddr.sin_addr, hp->h_length);
		biffaddr.sin_port = sp->s_port;
		(void) sendto(f, buf, (int) strlen(buf)+1, 0, (struct sockaddr *) &biffaddr, sizeof (biffaddr));
		(void) close(f);
	}
	/* 
	 * 2-92 TK check for fclose success.  
	 */
	unlock_file();
	if (fclose(malf) == EOF) {
		perror("fclose");
		fprintf(stderr,MSGSTR(M_MSG_46,
			"%s: cannot append to %s\n"), BELL,mailfile);
		(void) setuid(0);
		(void) setreuid(realuser,0);
		return(0);
	}

	(void) setuid(0);
	(void) setreuid(realuser,0);
	return(1);
}

void delex(i)
int i;
{
        if (i != SIGINT) {
                setsig(i, SIG_DFL);
                (void) sigsetmask(sigblock(0) &~ sigmask(i));
        }
	(void) fprintf(stderr, "\n");
        if (delflg)
                longjmp(sjbuf, 1);
        if (error == 0)
                error = i;
        done();

}

cat(to, from1, from2)
char *to, *from1, *from2;
{
	int i, j;

	j = 0;
	for (i=0; from1[i]; i++)
		to[j++] = from1[i];
	for (i=0; from2[i]; i++)
		to[j++] = from2[i];
	to[j] = 0;
}

char *getarg(s, p)	/* copy p... into s, update p */
register char *s, *p;
{
	while (*p == ' ' || *p == '\t')
		p++;
	if (*p == '\n' || *p == '\0')
		return(NULL);
	while (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0')
		*s++ = *p++;
	*s = '\0';
	return(p);
}

/* JCH/PJS:
 * safefile() now returns a file descriptor, so as to try to avoid race
 * conditions: the file may acquire links (h/s) between checking for these
 * and actually creating the file. 
 */
int
safefile(f, uid)
char	*f;		/* File name to be opened. */
int	uid;		/* To check ownership of the file. */
{
int		fd;
struct stat	statb;

/* We now try to create the file for writing to, but ensuring that this
 * will only succeed if the filename does not exist at the time.
 */
    if ((fd = open(f, O_WRONLY | O_CREAT | O_EXCL, MAILMODE)) >= 0)
	return(fd);
    if (errno != EEXIST) {	/* Something 'fatal' happened! */
	fprintf(stderr,MSGSTR(M_MSG_73,"%s: creating %s"),BELL,f); /*GAG*/
	perror(" -");
	return(-1);
    }

/* The file name must already exist, as the 'open' with O_EXCL failed.
 * So, we can try to open it simply for O_WRONLY.
 * The file is supposed to exist at this stage, so if the open fails
 * then we may only return a FAILed status.
 */
    if ((fd = open(f, O_WRONLY, MAILMODE)) < 0) {
	fprintf(stderr,MSGSTR(M_MSG_74,"%s: opening %s"),BELL,f); /*GAG*/
	perror(" -");
	return(-1);
    }

/* As a last precaution, we may check the filename for certain states:
 *  - the link count must not be more than one as it is a (potential)
 *    security hole.
 *  - the file has not become a symbolic link.
 *  - the file is owned by the 'uid' argument.
 * It is possible for the file to have been removed between the open() 
 * and the lstat(). In this case, we don't need to test the above states:
 * closing the file will recreate the file.
 */
    if (lstat(f, &statb) < 0) {
	if (errno != ENOENT) {
	    fprintf(stderr,MSGSTR(M_MSG_75,"%s: stating %s"),BELL,f); /*GAG*/
	    perror(" -");
	    return(-1);
	}
    }
    else {
	if (statb.st_nlink != 1 || (statb.st_mode & S_IFMT) == S_IFLNK) {
	    fprintf(stderr,MSGSTR(M_MSG_76, /*GAG*/
		"%s: %s has more than one link or is a symbolic link\n"),BELL,f);
	    return(-1);
	}
	if (uid > 0 && statb.st_uid != uid) {
	    fprintf(stderr,MSGSTR(M_MSG_77, /*GAG*/
	    	"%s: %s is not owned by you\n"),BELL,f);
	    return(-1);
	} 
    }

/* We have an open file that does not appear to:
 *  - have multiple hard links,
 *  - be a symbolic link,
 *  - be owned by someone else other than the expected owner.
 * Therefore, we may safely return the file descriptor.
 */
    return (fd);
}

/************************************
**  User-tunable locking style code.
**
**  The code block directly below between "#ifdef DOUBLE_LK" is replicated
**  in the mailx/quit.c and the MH directories/lock.c
**  R.W.
************************************/
#if defined DOUBLE_LK
#if ! defined _MTS_H_	/* Then this is not MH */
/* The following are for the locking style, and follow the naming
** conventions from MH  - R.W.
*/
#define LOK_UNIX	0	/* Defaults to LOK_KERNEL */
#define	LOK_BELL	1	/* ".lock" lock file */
#define	LOK_MMDF	2	/* mmdf-style lock file; not supported */
#define	LOK_KERNEL	4	/* Explicitly request lockf */

#define	LOK_VALID(lok)	(!((lok) & ~(LOK_BELL|LOK_KERNEL)))
#define	LOK_INVAL	-1

#define	LOK_DFLT	(LOK_KERNEL|LOK_BELL)	/* Don't make this LOK_UNIX */
int lockstyle = LOK_DFLT;
#endif	/* _MTS_H_ */

typedef struct {
    char *fname;
    char *pattern;
} Lkcfg_tbl;

static Lkcfg_tbl lkcfg_tbl[] = {
    { "/etc/rc.config", "MAILLOCKING=" },
#ifndef	_MTS_H_
    { "/usr/lib/mh/mtstailor", "lockstyle:" },
#endif
    { NULL, NULL }
};

static int
lock_style_init()
{
    /*	Get the style of locking from the configuration file(s).
    **  After the first invocation, it simply returns the cached return
    **  status.
    **  Returns: the found, legal lockvalue
    **          0 if the value doesn't exist (or file not found)
    **          LOK_INVAL (-1) on an illegal value from the file
    **	LOK_UNIX is never returned (since it is 0).  LOK_KERNEL is used instead.
    */

    register Lkcfg_tbl *tp;
    register char *cp;

    static int initd = 0;
    FILE *fp;
    register int  len;
    int val;
    char buf[128];
    char *ep;

    if (initd)
	return(initd - 2);
    else
	initd = 2;

    for (tp = lkcfg_tbl; tp->fname; tp++){
	if (!(fp = fopen(tp->fname, "r")))
	    continue;

	cp = buf + (len = strlen(tp->pattern));
	while (fgets(buf, sizeof(buf), fp)){
	    /*  Search for pattern.  Assume that the full pattern is:
	    **	{pattern}{some terminating char}{optional '"'}
	    */
	    if (strncmp(buf, tp->pattern, len))
		continue;
	    cp[-1] = '\0';	/* wipe out the terminator */
	    if (*cp == '"')
		cp++;

	    errno=0;
	    val = strtol(cp, &ep, 0);
	    if (!errno && cp != ep && LOK_VALID(val)){
		if (val == LOK_UNIX)
		    val = LOK_KERNEL;
		lockstyle = val;
	    }
	    else {
#ifdef	_MTS_H_		/* Then MH */
                advise(NULLCP, "illegal %s value in %s", buf, tp->fname);
#else
		fprintf(stderr, MSGSTR(ILL_LKSTYLE, "%s: illegal %s value in %s\n"), PGMNAME, buf, tp->fname);
#endif

#if defined OSF1 || defined OSF	/* Log in syslog too */
		(void) openlog(PGMNAME, 1, LOG_MAIL);
                syslog(LOG_ERR, "Illegal %s value in %s", buf, tp->fname);
		closelog();
#endif
		val = -1;
	    }
	    fclose(fp);
	    initd += val;
	    return(val);
	}
	fclose(fp);
    }
    return(0);
}
#endif	/* DOUBLE_LK */


/* the lockfile mechanism for compatibility with ULTRIX locks */

#define MAXLOADAPPROX 10  /* in place of getla(); */

char	*maillock	= ".lock";		/* Lock suffix for mailname */
char	curlock[PATH_MAX +1];			/* Last used name of lock */
int	locked;					/* To note that we locked it */

lock_file(file)
char *file;
{
	register int f;
	struct stat sbuf;		/* Stat of ".lock" file */
	struct stat original;		/* Orig. stat of ".lock" file */
	int statfailed;

	register int n;
	off_t osize;
	off_t nsize;
	struct stat mbox;

	if (locked || flgf)
		return(0);

#ifdef	DOUBLE_LK
	if (!(lockstyle & LOK_BELL))
	    return(0);
#endif

        n = strlen(file);
        if (n > sizeof(curlock)-2){
	    fprintf(stderr, MSGSTR(M_MSG_34, "%s: cannot create %s\n"), BELL, "lockfile");
	    error = EX_CANTCREAT;
	    done();
        }
        (void) strncpy(curlock, file, sizeof(curlock)-1);
        n = sizeof(curlock)-1 - n;
        (void) strncat(curlock, maillock, n);

	statfailed = 0;

top:
	/* Get the original size of the users' mail box
	 * and save it to check for changes to the mail box whilst
	 * we are sleeping on a lock file (if any).
	 */
	if (stat(file,&mbox) < 0)
		osize = 0;
	else
		osize = mbox.st_size;

	/* Get original mod time of possible lock file to test
	 * for creation of new lock file while we were sleeping.
	 */
	if (stat(curlock, &original) < 0) {
		original.st_ctime = 0;
	}

	/* Make number of sleep cycles.
	 */
	LOCKSLEEPS = OLOCKSLEEPS + MAXLOADAPPROX;

	for (n=0; n < LOCKSLEEPS; n++) {

		if ((f = lock1(curlock)) == -1){
		    /*	Some serious error; error value set in lock1() */
		    done();
		}
		if (f == 0) {
			if (OVERRIDE) {
				/*
	 			 * At this point, we would have waited 
				 * a long time for the lock file to go
				 * away. If it didn't, log a complaint.
				 */
				 (void) openlog(PGMNAME, 1, LOG_MAIL);
				 syslog(LOG_ERR,"Overriding mail lock file for  %s",file);
				 closelog();
			}
			/* We have locked the file, return to caller.
			 */
			locked = 1;
			OVERRIDE = 0;
			return(0);
		}

		/*  Lock file exists.  Stat it so we know when we can timeout
		*/
		if (stat(curlock, &sbuf) < 0) {
			if (statfailed++ > 5){
				/*  In some twisted race cycle (NFS timeouts?).
				**  Give up.
				*/
				(void) openlog(PGMNAME, 1, LOG_MAIL);
				syslog(LOG_ERR,"Cannot stat mail lock file\n");
				closelog();
				fprintf(stderr, MSGSTR(BL_ELOCK, "%s: cannot create lock file %s\n"), PGMNAME, curlock);
				error = EX_TEMPFAIL;
				done();
			}

			(void) sleep((unsigned) /* AV */ LOCKSLEEP);

			/* Take a new reading on the load.
			(void) getla(); */
			continue;
		}
		statfailed = 0;

		/* A lock file exists. Sleep for awhile and look again.
		 */
		if (FIRSTSLEEP) {
			FIRSTSLEEP = 0;
			(void) openlog(PGMNAME, 1, LOG_MAIL);
			syslog(LOG_ERR,"Waiting on mail lock file %s",curlock);
			closelog(); 
		}
		(void) sleep((unsigned) /* AV */ (LOCKSLEEP /* + peak */));

		/* Take a new reading on the load.
		(void) getla(); */

		/* While we were sleeping, the mail box may have grown,
		 * shrunk, -or- disappeared....
		 * Get a new size to compare to the original.
		 */
		if (stat(file,&mbox) < 0) {
			osize = nsize = 0;
		}
		else
			nsize = mbox.st_size;

		if ((nsize != osize) ||
			(original.st_ctime != sbuf.st_ctime)) {

			/* If the users' mail box changed size, reset
			 * to new size and restart the entire wait
			 * cycle over. ie. We have to see the mail box
			 * not change size for the required amount of
			 * time if there was a lock file present
			 * in the first place before we think about
			 * removing the existing lock file.
			 */
			original.st_ctime = sbuf.st_ctime;
			n = 0;
			osize = nsize;
			LOCKSLEEPS = OLOCKSLEEPS; /*  + peak; */
		}
		continue;
	}
	/* If we get here, the mail lock file (name.lock) has existed
	 * for the required amount of time &  we didn't see the
	 * users' mail box change size. -or- If we saw it change size,
	 * we reset our counters and rewound the clock for another
	 * time and then waited the respectable interval before
	 * resorting to removing the lock file by force.
	 *
	 * After our last sleep, make one final attempt to gracefully
	 * create a lock file.
	 */
	if ((f = lock1(curlock)) == -1){
	    /*  Some serious error; error value set in lock1() */
	    done();
	}
	if (f == 0) {
		/*
		 * We got lucky and were able to create the lock file.
		 */
		locked = 1;
		return(0);
	}	
	/* Make one last ck to see if a new lock file has
	 * been made whilst we were asleep.
	 */
	(void) stat(curlock, &sbuf);
	if (original.st_ctime != sbuf.st_ctime) {
		OVERRIDE = 0;
		goto top;
	}
	/* We have to remove the lock file by force.
	 */
	f = unlink(curlock);

	if (f < 0) {
		/* If we can't remove the lock file, send the mail
		 * back and record our complaint.
		 */
		if (errno != ENOENT) {
			(void) openlog(PGMNAME, 1, LOG_MAIL);
			syslog(LOG_ERR,"Cannot override mail lock file  %s",curlock);
			closelog();
			error = EX_UNAVAILABLE;
			done();
		}
	}
	OVERRIDE = 1;
	goto top;	/* Rewind */
}

/*
 * Remove the mail lock, and note that we no longer
 * have it locked.
 */
unlock_file()
{
	(void) unlink(curlock);
	locked = 0;
}

/*
 * Attempt to create the lock file.
 * Returns: 0 on success
 *	    1 if the lock already exists
 *	   -1 if a problem exists in creating the lock.
 * If a problem occurs, it displays an error message (stderr & syslog).
 *
 * N.B. This version takes advantage of the O_EXCL flag to atomically
 * create the lock file, and the good errno's back from it.  Other methods
 * need (e.g. link), need a more prolonged create, test, retry scheme.
 */
lock1(name)
	char name[];
{
	register int fd;

	if ((fd = open(name, O_EXCL|O_CREAT|O_RDONLY, 0)) >= 0){
	    (void) close(fd);
	    return(0);
	}

	if (errno == EEXIST)
	    return(1);

	(void) openlog(PGMNAME, 1, LOG_MAIL);
	syslog(LOG_ERR, "Cannot create mail lock file %s\n", name);
	closelog();
	fprintf(stderr, MSGSTR(BL_ELOCK, "%s: cannot create lock file %s\n"), PGMNAME, name);

	/*  Assume: ETIMEDOUT ==> NFS file system busy, otherwise probably
	**  directory permissions.  Either way, we can't do it now.
	*/
	error = (errno == ETIMEDOUT) ? EX_TEMPFAIL : EX_CANTCREAT;
	return(-1);
}


static int tmout;
void alrmser()
{
	tmout++;
}

/* New locking function. */
/* Alf Vetvik  26-Mar-91 */
int lock (fd)
int fd;
{
	struct stat status;
	void (*f)();
#if defined DOUBLE_LK
	if (lock_style_init() == LOK_INVAL){
	    error = EX_OSFILE;
	    done();
	}

	/*  Cheat.  Tell the code that we successfully lockf'd the file */
	if (!(lockstyle & LOK_KERNEL))
	    return(1);
#endif

	f=signal(SIGALRM, alrmser);
	alarm(LF_TMOUT);
	if (lockf (fd, F_LOCK, 0L) == -1){
	    alarm(0);
	    (void) openlog(PGMNAME, 1, LOG_MAIL);
	    syslog(LOG_ERR,"cannot lockf %s", mailfile);
	    fprintf(stderr, MSGSTR(LF_ELOCK, "%s: cannot lockf %s\n"), PGMNAME, mailfile);
	    error = EX_TEMPFAIL;
	    return(-1);
	}
	alarm(0);
	signal(SIGALRM, f);

	/* File locked, now do fstat to check link count. */
	if (fstat (fd, &status) == -1) {
		(void) unlink(lettmp);
		perror ("fstat failed");
		exit (1);
	}
	return (status.st_nlink == 0) ? 0 : 1; 
}

done()
{      
	if(locked)
		unlock_file();
	(void) unlink(lettmp);
	exit(error);
}


/*
 * 2-92 TK * get $TMPDIR if it exists, otherwise "/tmp"
 */
char *
gettmpdir()
{
	char *tmpdir, *getenv();

	return((tmpdir = getenv("TMPDIR")) ? tmpdir : "/tmp");
}
