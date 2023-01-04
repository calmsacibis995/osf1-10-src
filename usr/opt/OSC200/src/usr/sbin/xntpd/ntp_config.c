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
static char     *sccsid = "@(#)$RCSfile: ntp_config.c,v $ $Revision: 4.3.3.5 $ (DEC) $Date: 1992/10/14 13:22:18 $";
#endif
/*
 */
 /*
 * ntp_config.c - read and apply configuration information
 */
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>

#include <ntp/ntp_syslog.h>
#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>
#include <ntp/ntp_refclock.h>

/*
 * These routines are used to read the configuration file at
 * startup time.  An entry in the file must fit on a single line.
 * Entries are processed as multiple tokens separated by white space
 * Lines are considered terminated when a '#' is encountered.  Blank
 * lines are ignored.
 */

/*
 * Configuration file name
 */
#ifndef	CONFIG_FILE
#define	CONFIG_FILE	"/etc/ntp.conf"
#endif	/* CONFIG_FILE */

/*
 * We understand the following configuration entries.
 *
 * peer 128.100.1.1 [ version 2 ] [ key 5 ] [ minpoll ]
 * server 128.100.2.2 [ version 1 ] [ key 6 ] [ minpoll ]
 * precision -7
 * broadcast 128.100.224.255 [ version 2 ] [ key 2 ] [ minpoll ]
 * broadcastclient yes|no
 * broadcastdelay 0.0102
 * authenticate yes|no
 * monitor yes|no
 * authdelay 0.00842
 * restrict 128.100.100.0 [ mask 255.255.255.0 ] ignore|noserve|notrust|noquery
 * driftfile file_name
 * keys file_name
 * resolver /path/progname
 *
 * And then some.  See the manual page.
 */

/*
 * Types of entries we understand.
 */
#define CONFIG_UNKNOWN		0

#define	CONFIG_PEER		1
#define	CONFIG_SERVER		2
#define	CONFIG_PRECISION	3
#define	CONFIG_DRIFTFILE	4
#define	CONFIG_BROADCAST	5
#define	CONFIG_BROADCASTCLIENT	6
#define	CONFIG_AUTHENTICATE	7
#define	CONFIG_KEYS		8
#define	CONFIG_MONITOR		9
#define	CONFIG_AUTHDELAY	10
#define	CONFIG_RESTRICT		11
#define	CONFIG_BDELAY		12
#define	CONFIG_TRUSTEDKEY	13
#define	CONFIG_REQUESTKEY	14
#define	CONFIG_CONTROLKEY	15
#define	CONFIG_TRAP		16
#define	CONFIG_FUDGE		17
#define	CONFIG_MAXSKEW		18
#define	CONFIG_RESOLVER		19
#define	CONFIG_SELECT		20

#define	CONF_MOD_VERSION	1
#define	CONF_MOD_KEY		2
#define	CONF_MOD_MINPOLL	3

#define	CONF_RES_MASK		1
#define	CONF_RES_IGNORE		2
#define	CONF_RES_NOSERVE	3
#define	CONF_RES_NOTRUST	4
#define	CONF_RES_NOQUERY	5
#define	CONF_RES_NOMODIFY	6
#define	CONF_RES_NOPEER		7
#define	CONF_RES_NOTRAP		8
#define	CONF_RES_LPTRAP		9
#define	CONF_RES_NTPPORT	10

#define	CONF_TRAP_PORT		1
#define	CONF_TRAP_INTERFACE	2

#define	CONF_FDG_TIME1		1
#define	CONF_FDG_TIME2		2
#define	CONF_FDG_VALUE1		3
#define	CONF_FDG_VALUE2		4
#define	CONF_FDG_FLAG1		5
#define	CONF_FDG_FLAG2		6
#define	CONF_FDG_FLAG3		7
#define	CONF_FDG_FLAG4		8

/*
 * Translation table - keywords to function index
 */
struct keyword {
	char *text;
	int keytype;
};

struct keyword keywords[] = {
	{ "peer",		CONFIG_PEER },
	{ "server",		CONFIG_SERVER },
	{ "precision",		CONFIG_PRECISION },
	{ "driftfile",		CONFIG_DRIFTFILE },
	{ "broadcast",		CONFIG_BROADCAST },
	{ "broadcastclient",	CONFIG_BROADCASTCLIENT },
	{ "authenticate",	CONFIG_AUTHENTICATE },
	{ "keys",		CONFIG_KEYS },
	{ "monitor",		CONFIG_MONITOR },
	{ "authdelay",		CONFIG_AUTHDELAY },
	{ "restrict",		CONFIG_RESTRICT },
	{ "broadcastdelay",	CONFIG_BDELAY },
	{ "trustedkey",		CONFIG_TRUSTEDKEY },
	{ "requestkey",		CONFIG_REQUESTKEY },
	{ "controlkey",		CONFIG_CONTROLKEY },
	{ "trap",		CONFIG_TRAP },
	{ "fudge",		CONFIG_FUDGE },
	{ "maxskew",		CONFIG_MAXSKEW },
	{ "resolver",		CONFIG_RESOLVER },
	{ "select",		CONFIG_SELECT },
	{ "",			CONFIG_UNKNOWN }
};

/*
 * Modifier keywords
 */
struct keyword mod_keywords[] = {
	{ "version",	CONF_MOD_VERSION },
	{ "key",	CONF_MOD_KEY },
	{ "minpoll",	CONF_MOD_MINPOLL },
	{ "",		CONFIG_UNKNOWN }
};


/*
 * Special restrict keywords
 */
struct keyword res_keywords[] = {
	{ "mask",	CONF_RES_MASK },
	{ "ignore",	CONF_RES_IGNORE },
	{ "noserve",	CONF_RES_NOSERVE },
	{ "notrust",	CONF_RES_NOTRUST },
	{ "noquery",	CONF_RES_NOQUERY },
	{ "nomodify",	CONF_RES_NOMODIFY },
	{ "nopeer",	CONF_RES_NOPEER },
	{ "notrap",	CONF_RES_NOTRAP },
	{ "lowpriotrap",	CONF_RES_LPTRAP },
	{ "ntpport",	CONF_RES_NTPPORT },
	{ "",		CONFIG_UNKNOWN }
};


/*
 * Keywords for the trap command
 */
struct keyword trap_keywords[] = {
	{ "port",	CONF_TRAP_PORT },
	{ "interface",	CONF_TRAP_INTERFACE },
	{ "",		CONFIG_UNKNOWN }
};


/*
 * Keywords for the fudge command
 */
struct keyword fudge_keywords[] = {
	{ "time1",	CONF_FDG_TIME1 },
	{ "time2",	CONF_FDG_TIME2 },
	{ "value1",	CONF_FDG_VALUE1 },
	{ "value2",	CONF_FDG_VALUE2 },
	{ "flag1",	CONF_FDG_FLAG1 },
	{ "flag2",	CONF_FDG_FLAG2 },
	{ "flag3",	CONF_FDG_FLAG3 },
	{ "flag4",	CONF_FDG_FLAG4 },
	{ "",		CONFIG_UNKNOWN }
};


/*
 * Limits on things
 */
#define	MAXTOKENS	20	/* 20 tokens on line */
#define	MAXLINE		1024	/* maximum length of line */
#define	MAXFILENAME	128	/* maximum length of a file name (alloca()??) */


/*
 * Miscellaneous macros
 */
#define	STRSAME(s1, s2)		(*(s1) == *(s2) && strcmp((s1), (s2)) == 0)
#define	ISEOL(c)		((c) == '#' || (c) == '\n' || (c) == '\0')
#define	ISSPACE(c)		((c) == ' ' || (c) == '\t')
#define	STREQ(a, b)		(*(a) == *(b) && strcmp((a), (b)) == 0)

/*
 * File descriptor used by the resolver save routines, and temporary file
 * name.
 */
static FILE *res_fp;
static char res_file[20];	/* enough for /tmp/xntpXXXXXX\0 */
#define	RES_TEMPFILE	"/tmp/xntpXXXXXX"

/*
 * Definitions of things either imported from or exported to outside
 */
#ifdef DEBUG
extern int debug;
#endif
char *progname;
char *xntp_options = "bc:df:glr:sx";


/*
 * getstartup - search through the options looking for a debugging flag
 */
void
getstartup(argc, argv)
	int argc;
	char *argv[];
{

	int errflg;
	int c;
	extern int optind;
	extern char *optarg;
	extern int getopt();
	extern int boottime;

	debug = 0;		/* no debugging by default */

	/*
	 * This is a big hack.  We don't really want to read command line
	 * configuration until everything else is initialized, since
	 * the ability to configure the system may depend on storage
	 * and the like having been initialized.  Except that we also
	 * don't want to initialize anything until after detaching from
	 * the terminal, but we won't know to do that until we've
	 * parsed the command line.  Do that now, crudely, and do it
	 * again later.  Our getopt() is explicitly reusable, by the
	 * way.  Your own mileage may vary.
	 */
	errflg = 0;
	progname = argv[0];

	/*
	 * Decode argument list
	 */
	while ((c = getopt(argc, argv, xntp_options)) != EOF)
		switch (c) {
		case 'd':
			++debug;
			break;
		case 'i':
			boottime=TRUE;
			break;
		case '?':
			++errflg;
			break;
		default:
			break;
		}
	
	if (errflg || optind != argc) {
		(void) fprintf(stderr,
		    "usage: %s [ -bdglsx ] [ -c config_file ] [ -f drift_file ] [ -r broadcast_delay ] \n", progname);
		exit(2);
	}
	optind = 0;		/* reset optind to restart getopt */

	if (debug)
		setlinebuf(stdout);

}

/*
 * getconfig - get command line options and read the configuration file
 */
void
getconfig(argc, argv)
	int argc;
	char *argv[];
{
	register int i;
	int c;
	int errflg;
	int peerversion;
	u_int peerkey;
	int peerflags;
	int hmode;
	struct sockaddr_in peeraddr;
	struct sockaddr_in maskaddr;
	FILE *fp;
	char line[MAXLINE];
	char *tokens[MAXTOKENS];
	int ntokens;
	int tok;
	struct interface *localaddr;
	char *config_file;
	struct refclockstat clock;
	int have_resolver;
	char resolver_name[MAXFILENAME];
	int have_keyfile;
	char keyfile[MAXFILENAME];
	extern int optind;
	extern char *optarg;
	int gettokens();
	int matchkey();
	int getnetnum();
	extern void proto_config();
	extern void authtrust();
	extern struct peer *peer_config();
	extern void stats_config();
	extern void mon_start(), mon_stop();
	extern int atolfp();
	extern int authreadkeys();
	extern struct interface *findinterface();
	extern char *ntoa();
	void save_resolve();
	void do_resolve();
	void abort_resolve();
	extern u_int info_auth_keyid;
	extern int count_peers;
	extern char *save_peeraddr[];
	extern int boottime;
	extern int dont_change_time;
	extern int allow_set_backward;
	extern int correct_any;
	extern int no_log;
	
	/*
	 * Initialize, initialize
	 */
	errflg = 0;
#ifdef DEBUG
	debug = 0;
#endif	/* DEBUG */
	config_file = CONFIG_FILE;
	progname = argv[0];
	res_fp = NULL;
	have_resolver = have_keyfile = 0;

	/*
	 * Decode argument list
	 */
	while ((c = getopt(argc, argv, xntp_options)) != EOF) {
		switch (c) {
		case 'a':
			proto_config(PROTO_AUTHENTICATE, 1);
			break;
		case 'b':
			proto_config(PROTO_BROADCLIENT, 1);
			break;
		case 'c':
			config_file = optarg;
			break;
		case 'd':
#ifdef DEBUG
			debug++;
#else
			errflg++;
#endif	/* DEBUG */
			break;

		case 'e':
			do {
				l_fp tmp;
				if (!atolfp(optarg, &tmp)) {
					syslog(LOG_ERR,
			"command line encryption delay value %s undecodable",
					    optarg);
					errflg++;
				} else if (tmp.l_ui != 0) {
					syslog(LOG_ERR,
			"command line encryption delay value %s is unlikely",
					    optarg);
					errflg++;
				} else {
					proto_config(PROTO_AUTHDELAY, tmp.l_f);
				}
			} while (0);
			break;

		case 'f':
			stats_config(STATS_FREQ_FILE, optarg);
			break;

	        case 'g':
			correct_any=TRUE;
			break;

	        case 'i':
			boottime=TRUE;
			break;
			
		case 'k':
#ifdef DES_OK
			getauthkeys(optarg);
#else /* DES_OK */
			syslog(LOG_ERR,
			       "command line -k ignored. Authentication not supported");
#endif /* DES_OK */
			break;
			
	        case 'l':
			no_log=TRUE;
			break;

		case 'r':
			do {
				l_fp tmp;
				u_fp utmp;

				if (!atolfp(optarg, &tmp)) {
					syslog(LOG_ERR,
			"command line broadcast delay value %s undecodable",
					    optarg);
				} else if (tmp.l_ui != 0) {
					syslog(LOG_ERR,
			 "command line broadcast delay value %s is unlikely",
					    optarg);
				} else {
					utmp = LFPTOFP(&tmp);
					proto_config(PROTO_BROADDELAY, utmp);
				}
			} while (0);
			break;

		case 's':
			dont_change_time = TRUE;
			break;
		case 't':
#ifdef DES_OK
			do {
				int tkey;

				tkey = atoi(optarg);
				if (tkey <= 0 || tkey > NTP_MAXKEY) {
					syslog(LOG_ERR,
				"command line trusted key %s is unlikely",
					    optarg);
				} else {
					authtrust(tkey, 1);
				}
			} while (0);
#else /* DES_OK */
			syslog(LOG_ERR,
			       "trusted key ignored.  Authentication is not supported");
#endif /* DES_OK */
			break;
	        case 'x':
			allow_set_backward=TRUE;
			break;

		default:
			errflg++;
			break;
		}
	}
	
	if (errflg || optind != argc) {
		(void) fprintf(stderr,
		    "usage: %s [ -bdglsx ] [ -c config_file ] [ -f drift_file ] [ -r broadcast_delay ] \n", progname);
		exit(2);
	}

	if ((fp = fopen(config_file, "r")) == NULL) {
		/*
		 * Broadcast clients can sometimes run without
		 * a configuration file.
		 */
		return;
	}

	while ((tok = gettokens(fp, line, tokens, &ntokens))
	    != CONFIG_UNKNOWN) {
		switch(tok) {
		case CONFIG_PEER:
		case CONFIG_SERVER:
		case CONFIG_BROADCAST:
			if (tok == CONFIG_PEER)
				hmode = MODE_ACTIVE;
			else if (tok == CONFIG_SERVER)
				hmode = MODE_CLIENT;
			else
				hmode = MODE_BROADCAST;

			if (ntokens < 2) {
				syslog(LOG_ERR,
				    "No address for %s, line ignored",
				    tokens[0]);
				break;
			}

			if (GetHostName(tokens[1], &peeraddr) == 0){
			    syslog(LOG_ERR, "%s: unknown host", tokens[1]);
			    errflg = -1;
			  }
			else
			  syslog(LOG_DEBUG,
				 "GetHostName returned %s\n",
				 ntoa(&peeraddr));
			
#ifdef NETNUM
			if (!getnetnum(tokens[1], &peeraddr, 0)) {
			  syslog(LOG_ERR,
				 "getnetnum failed for %s, %s\n", 
				 tokens[1], htaddr);
				errflg = -1;
			} else {
#endif /* NETNUM */
			  /* save_peeraddr is used by init_time */
			        count_peers++;

				save_peeraddr[count_peers] =
				  (char *) malloc(64);

				sprintf(save_peeraddr[count_peers],
					"%s", ntoa(&peeraddr));

				errflg = 0;

#ifdef REFCLOCK
				if (!ISREFCLOCKADR(&peeraddr)
				    && ISBADADR(&peeraddr)) {
#else
				if (ISBADADR(&peeraddr)) {
#endif
					syslog(LOG_ERR,
				    "attempt to configure invalid address %s",
					    ntoa(&peeraddr));
					break;
				}
#ifdef NETNUM
			}
#endif /* NETNUM */
			
			peerversion = NTP_VERSION;
			peerkey = 0;
			peerflags = 0;
			for (i = 2; i < ntokens; i++)
				switch (matchkey(tokens[i], mod_keywords)) {
				case CONF_MOD_VERSION:
					if (i >= ntokens-1) {
						syslog(LOG_ERR,
				"peer/server version requires an argument");
						errflg = 1;
						break;
					}
					peerversion = atoi(tokens[++i]);
					if (peerversion != NTP_VERSION
					    && peerversion != NTP_OLDVERSION) {
						syslog(LOG_ERR,
				"inappropriate version number %s, line ignored",
						    tokens[i]);
						errflg = 1;
					}
					break;

				case CONF_MOD_KEY:
					/*
					 * XXX
					 * This is bad because atoi
					 * returns 0 on errors.  Do
					 * something later.
					 */
					if (i >= ntokens-1) {
						syslog(LOG_ERR,
					"peer/server key requires an argument");
						errflg = 1;
						break;
					}
					peerkey = (u_int)atoi(tokens[++i]);
					peerflags |= FLAG_AUTHENABLE;
					break;

				case CONF_MOD_MINPOLL:
					peerflags |= FLAG_MINPOLL;
					break;

				case CONFIG_UNKNOWN:
					errflg = 1;
					break;
				}
			if (errflg == 0) {
				if (peer_config(&peeraddr,
				    (struct interface *)0,
				    hmode, peerversion, peerkey,
				    peerflags) == 0) {
					syslog(LOG_ERR,
					    "configuration of %s failed",
					    ntoa(&peeraddr));
				}
			} else if (errflg == -1) {
#ifdef DES_OK
				save_resolve(tokens[1], hmode, peerversion,
				    peerflags, peerkey);
#endif /* DES_OK */
			}
			break;

		case CONFIG_PRECISION:
			if (ntokens >= 2) {
				i = atoi(tokens[1]);
				if (i >= 0 || i < -25)
					syslog(LOG_ERR,
				"unlikely precision %s, line ignored",
					    tokens[1]);
				else
					proto_config(PROTO_PRECISION, i);
			}
			break;

		case CONFIG_DRIFTFILE:
			if (ntokens >= 2)
				stats_config(STATS_FREQ_FILE, tokens[1]);
			else
				stats_config(STATS_FREQ_FILE, (char *)0);
			break;
		
		case CONFIG_BROADCASTCLIENT:
			errflg = 0;
			if (ntokens >= 2) {
				if (STREQ(tokens[1], "yes"))
					proto_config(PROTO_BROADCLIENT, 1);
				else if (STREQ(tokens[1], "no"))
					proto_config(PROTO_BROADCLIENT, 0);
				else
					errflg++;
			} else {
				errflg++;
			}

			if (errflg)
				syslog(LOG_ERR,
				    "should be `broadcastclient yes|no'");
			break;
		
		case CONFIG_AUTHENTICATE:
			errflg = 0;
			if (ntokens >= 2) {
				if (STREQ(tokens[1], "yes"))
					proto_config(PROTO_AUTHENTICATE, 1);
				else if (STREQ(tokens[1], "no"))
					proto_config(PROTO_AUTHENTICATE, 0);
				else
					errflg++;
			} else {
				errflg++;
			}

			if (errflg)
				syslog(LOG_ERR,
				    "should be `authenticate yes|no'");
			break;

		case CONFIG_KEYS:
#ifdef DES_OK
				if (ntokens >= 2) {
				  getauthkeys(tokens[1]);
				  if (strlen(tokens[1]) >= MAXFILENAME) {
				    syslog(LOG_ERR,
					   "key file name too long (>%d, sigh), no name resolution possible",
					   MAXFILENAME);
				  } else {
				    have_keyfile = 1;
				    (void)strcpy(keyfile, tokens[1]);
				  }
				}
#else /* DES_OK */
				syslog(LOG_ERR,
				       "key file ignored.  Authentication unsupported.");
#endif /* DES_OK */
				  break;

		case CONFIG_MONITOR:
			errflg = 0;
			if (ntokens >= 2) {
				if (STREQ(tokens[1], "yes"))
					mon_start();
				else if (STREQ(tokens[1], "no"))
					mon_stop();
				else
					errflg++;
			} else {
				errflg++;
			}

			if (errflg)
				syslog(LOG_ERR,
				    "should be `monitor yes|no'");
			break;
		
		case CONFIG_AUTHDELAY:
			if (ntokens >= 2) {
				l_fp tmp;

				if (!atolfp(tokens[1], &tmp)) {
					syslog(LOG_ERR,
					    "authdelay value %s undecodable",
					    tokens[1]);
				} else if (tmp.l_ui != 0) {
					syslog(LOG_ERR,
					    "authdelay value %s is unlikely",
					    tokens[1]);
				} else {
					proto_config(PROTO_AUTHDELAY, tmp.l_f);
				}
			}
			break;
		
		case CONFIG_RESTRICT:
			if (ntokens < 2) {
				syslog(LOG_ERR, "restrict requires an address");
				break;
			}
			if (STREQ(tokens[1], "default"))
				peeraddr.sin_addr.s_addr = INADDR_ANY;
			else if (!getnetnum(tokens[1], &peeraddr, 1))
				break;
			
			/*
			 * Use peerversion as flags, peerkey as mflags.  Ick.
			 */
			peerversion = 0;
			peerkey = 0;
			errflg = 0;
			maskaddr.sin_addr.s_addr = ~0;
			for (i = 2; i < ntokens; i++) {
				switch (matchkey(tokens[i], res_keywords)) {
				case CONF_RES_MASK:
					if (i >= ntokens-1) {
						syslog(LOG_ERR,
						"mask keyword needs argument");
						errflg++;
						break;
					}
					i++;
					if (!getnetnum(tokens[i], &maskaddr, 1))
						errflg++;
					break;

				case CONF_RES_IGNORE:
					peerversion |= RES_IGNORE;
					break;
				
				case CONF_RES_NOSERVE:
					peerversion |= RES_DONTSERVE;
					break;
				
				case CONF_RES_NOTRUST:
					peerversion |= RES_DONTTRUST;
					break;
				
				case CONF_RES_NOQUERY:
					peerversion |= RES_NOQUERY;
					break;

				case CONF_RES_NOMODIFY:
					peerversion |= RES_NOMODIFY;
					break;

				case CONF_RES_NOPEER:
					peerversion |= RES_NOPEER;
					break;

				case CONF_RES_NOTRAP:
					peerversion |= RES_NOTRAP;
					break;

				case CONF_RES_LPTRAP:
					peerversion |= RES_LPTRAP;
					break;

				case CONF_RES_NTPPORT:
					peerkey |= RESM_NTPONLY;
					break;

				case CONFIG_UNKNOWN:
					errflg++;
					break;
				}
			}
			if (SRCADR(&peeraddr) == INADDR_ANY)
				maskaddr.sin_addr.s_addr = 0;
			if (!errflg)
				restrict(RESTRICT_FLAGS, &peeraddr, &maskaddr,
				    (int)peerkey, peerversion);
			break;

		case CONFIG_BDELAY:
			if (ntokens >= 2) {
				l_fp tmp;
				u_fp utmp;

				if (!atolfp(tokens[1], &tmp)) {
					syslog(LOG_ERR,
					 "broadcastdelay value %s undecodable",
					    tokens[1]);
				} else if (tmp.l_ui != 0) {
					syslog(LOG_ERR,
					  "broadcastdelay value %s is unlikely",
					    tokens[1]);
				} else {
					utmp = LFPTOFP(&tmp);
					proto_config(PROTO_BROADDELAY, utmp);
				}
			}
			break;
		
		case CONFIG_TRUSTEDKEY:
#ifdef DES_OK
			for (i = 1; i < ntokens; i++) {
				u_int tkey;

				tkey = (u_int) atoi(tokens[i]);
				if (tkey == 0) {
					syslog(LOG_ERR,
					    "trusted key %s unlikely",
					    tokens[i]);
				} else {
					authtrust(tkey, 1);
				}
			}
#else /* DES_OK */
			syslog(LOG_ERR,
			       "trusted key ignored.  Authentication is not supported");
#endif /* DES_OK */
				break;

		case CONFIG_REQUESTKEY:
			if (ntokens >= 2) {
				u_int rkey;

				if (!atouint(tokens[1], &rkey)) {
					syslog(LOG_ERR,
					    "%s is undecodeable as request key",
					    tokens[1]);
				} else if (rkey == 0) {
					syslog(LOG_ERR,
					    "%s makes a poor request keyid",
					    tokens[1]);
				} else {
#ifdef DEBUG
					if (debug > 3)
						printf(
					"set info_auth_key to %u\n", rkey);
#endif
					info_auth_keyid = rkey;
				}
			}
			break;
		
		case CONFIG_CONTROLKEY:
			if (ntokens >= 2) {
				u_int ckey;
				extern u_int ctl_auth_keyid;

				ckey = (u_int)atoi(tokens[1]);
				if (ckey == 0) {
					syslog(LOG_ERR,
					    "%s makes a poor control keyid",
					    tokens[1]);
				} else {
					ctl_auth_keyid = ckey;
				}
			}
			break;
		
		case CONFIG_TRAP:
			if (ntokens < 2) {
				syslog(LOG_ERR,
				  "no address for trap command, line ignored");
				  break;
			}
			if (!getnetnum(tokens[1], &peeraddr, 1))
				break;
			
			/*
			 * Use peerversion for port number.  Barf.
			 */
			errflg = 0;
			peerversion = 0;
			localaddr = 0;
			for (i = 2; i < ntokens-1; i++)
				switch (matchkey(tokens[i], trap_keywords)) {
				case CONF_TRAP_PORT:
					if (i >= ntokens-1) {
						syslog(LOG_ERR,
					    "trap port requires an argument");
						errflg = 1;
						break;
					}
					peerversion = atoi(tokens[++i]);
					if (peerversion <= 0
					    || peerversion > 32767) {
						syslog(LOG_ERR,
					"invalid port number %s, trap ignored",
						    tokens[i]);
						errflg = 1;
					}
					break;

				case CONF_TRAP_INTERFACE:
					if (i >= ntokens-1) {
						syslog(LOG_ERR,
					 "trap interface requires an argument");
						errflg = 1;
						break;
					}

					if (!getnetnum(tokens[++i],
					    &maskaddr, 1)) {
						errflg = 1;
						break;
					}

					localaddr = findinterface(&maskaddr);
					if (localaddr == NULL) {
						syslog(LOG_ERR,
					"can't find interface with address %s",
						    ntoa(&maskaddr));
						errflg = 1;
					}
					break;

				case CONFIG_UNKNOWN:
					errflg++;
					break;
				}

			if (!errflg) {
				extern struct interface *any_interface;

				if (peerversion != 0)
					peeraddr.sin_port = htons(peerversion);
				else
					peeraddr.sin_port = htons(TRAPPORT);
				if (localaddr == NULL)
					localaddr = any_interface;
				if (!ctlsettrap(&peeraddr, localaddr, 0,
				    NTP_VERSION))
					syslog(LOG_ERR,
					"can't set trap for %s, no resources",
					    ntoa(&peeraddr));
			}
			break;
		
		case CONFIG_FUDGE:
			if (ntokens < 2) {
				syslog(LOG_ERR,
				  "no address for fudge command, line ignored");
				  break;
			}
			if (!getnetnum(tokens[1], &peeraddr, 1))
				break;

			if (!ISREFCLOCKADR(&peeraddr)) {
				syslog(LOG_ERR,
	"%s is inappropriate address for the fudge command, line ignored",
				    ntoa(&peeraddr));
				break;
			}

			bzero((char *)&clock, sizeof clock);
			errflg = 0;
			for (i = 2; i < ntokens-1; i++) {
				switch (c = matchkey(tokens[i],
				    fudge_keywords)) {
				case CONF_FDG_TIME1:
					if (!atolfp(tokens[++i],
					    &clock.fudgetime1)) {
						syslog(LOG_ERR,
					"fudge %s time1 value in error",
						    ntoa(&peeraddr));
						errflg = i;
						break;
					}
					clock.haveflags |= CLK_HAVETIME1;
					break;

				case CONF_FDG_TIME2:
					if (!atolfp(tokens[++i],
					    &clock.fudgetime2)) {
						syslog(LOG_ERR,
					"fudge %s time2 value in error",
						    ntoa(&peeraddr));
						errflg = i;
						break;
					}
					clock.haveflags |= CLK_HAVETIME2;
					break;

				case CONF_FDG_VALUE1:
					if (!atoint(tokens[++i],
					    &clock.fudgeval1)) {
						syslog(LOG_ERR,
					"fudge %s value1 value in error",
						    ntoa(&peeraddr));
						errflg = i;
						break;
					}
					clock.haveflags |= CLK_HAVEVAL1;
					break;

				case CONF_FDG_VALUE2:
					if (!atoint(tokens[++i],
					    &clock.fudgeval2)) {
						syslog(LOG_ERR,
					"fudge %s value2 value in error",
						    ntoa(&peeraddr));
						errflg = i;
						break;
					}
					clock.haveflags |= CLK_HAVEVAL2;
					break;

				case CONF_FDG_FLAG1:
				case CONF_FDG_FLAG2:
				case CONF_FDG_FLAG3:
				case CONF_FDG_FLAG4:
					if (!atoint(tokens[++i], &peerkey)
					    || peerkey > 1) {
						syslog(LOG_ERR,
					"fudge %s flag value in error",
						    ntoa(&peeraddr));
						errflg = i;
						break;
					}
					switch(c) {
					case CONF_FDG_FLAG1:
						c = CLK_FLAG1;
						clock.haveflags|=CLK_HAVEFLAG1;
						break;
					case CONF_FDG_FLAG2:
						c = CLK_FLAG2;
						clock.haveflags|=CLK_HAVEFLAG2;
						break;
					case CONF_FDG_FLAG3:
						c = CLK_FLAG3;
						clock.haveflags|=CLK_HAVEFLAG3;
						break;
					case CONF_FDG_FLAG4:
						c = CLK_FLAG4;
						clock.haveflags|=CLK_HAVEFLAG4;
						break;
					}
					if (peerkey == 0)
						clock.flags &= ~c;
					else
						clock.flags |= c;
					break;

				case CONFIG_UNKNOWN:
					errflg = -1;
					break;
				}
			}

#ifdef REFCLOCK
			/*
			 * If reference clock support isn't defined the
			 * fudge line will still be accepted and syntax
			 * checked, but will essentially do nothing.
			 */
			if (!errflg) {
				extern void refclock_control();

				refclock_control(&peeraddr, &clock,
				    (struct refclockstat *)0);
			}
#endif
			break;

		case CONFIG_MAXSKEW:
			if (ntokens >= 2) {
				l_fp tmp;
				u_fp utmp;

				if (!atolfp(tokens[1], &tmp)) {
					syslog(LOG_ERR,
					 "maxskew value %s undecodable",
					    tokens[1]);
				} else if (tmp.l_ui != 0) {
					syslog(LOG_ERR,
					  "maxskew value %s is unlikely",
					    tokens[1]);
				} else {
					utmp = LFPTOFP(&tmp);
					proto_config(PROTO_MAXSKEW, utmp);
				}
			}
			break;
		
		case CONFIG_RESOLVER:
			if (ntokens >= 2) {
				if (strlen(tokens[1]) >= MAXFILENAME) {
					syslog(LOG_ERR,
	"resolver path name too long (>%d, sigh), no name resolution possible",
					    MAXFILENAME);
					break;
				}
				strcpy(resolver_name, tokens[1]);
				have_resolver = 1;
			}
			break;

		case CONFIG_SELECT:
			if (ntokens >= 2) {
				i = atoi(tokens[1]);
				if (i < SELECT_1 || i > SELECT_5)
					syslog(LOG_ERR,
				"invalid selection algorithm %s, line ignored",
					    tokens[1]);
				else
					proto_config(PROTO_SELECT, (int)i);
			}
			break;

		}
	}
	(void) fclose(fp);

	if (res_fp != NULL) {
		/*
		 * Need name resolution
		 */
		errflg = 0;
		if (info_auth_keyid == 0) {
			syslog(LOG_ERR,
		"no request key defined, peer name resolution not possible");
			errflg++;
		}
		if (!have_resolver) {
			syslog(LOG_ERR,
		"no resolver defined, peer name resolution not possible");
			errflg++;
		}
		if (!have_keyfile) {
			syslog(LOG_ERR,
		"no key file specified, peer name resolution not possible");
			errflg++;
		}

		if (!errflg)
			do_resolve(resolver_name, info_auth_keyid, keyfile);
		else
			abort_resolve();
	}
}


GetHostName(name, sin)
   char *name;
   struct sockaddr_in *sin;
{
	int HostAddr;
	struct hostent *hp;

	if ((HostAddr = inet_addr(name)) != -1) {
	        bzero((char *)sin, sizeof(struct sockaddr_in));
		sin->sin_addr.s_addr = (u_int) HostAddr;
		sin->sin_family = AF_INET;
		sin->sin_port = htons(NTP_PORT);
		return (1);
	}

	if (hp = gethostbyname(name)) {
		if (hp->h_addrtype != AF_INET)
			return (0);
		bzero((char *)sin, sizeof(struct sockaddr_in));
		bcopy((char *) hp->h_addr, (char *) &sin->sin_addr,
		      hp->h_length);
		sin->sin_port = htons(NTP_PORT);
		sin->sin_family = AF_INET;
		syslog(LOG_DEBUG,"ntoa(sin)=%s\n",ntoa(sin));
		return (1);
	}
	return (0);
}



/*
 * gettokens - read a line and return tokens
 */
int
gettokens(fp, line, tokenlist, ntokens)
	FILE *fp;
	char *line;
	char **tokenlist;
	int *ntokens;
{
	register char *cp;
	register int eol;
	register int ntok;
	int matchkey();

	/*
	 * Find start of first token
	 */
again:
	while ((cp = fgets(line, MAXLINE, fp)) != NULL) {
		cp = line;
		while (ISSPACE(*cp))
			cp++;
		if (!ISEOL(*cp))
			break;
	}
	if (cp == NULL) {
		*ntokens = 0;
		return CONFIG_UNKNOWN;	/* hack.  Is recognized as EOF */
	}

	/*
	 * Now separate out the tokens
	 */
	eol = 0;
	ntok = 0;
	while (!eol) {
		tokenlist[ntok++] = cp;
		while (!ISEOL(*cp) && !ISSPACE(*cp))
			cp++;
		if (ISEOL(*cp)) {
			*cp = '\0';
			eol = 1;
		} else {		/* must be space */
			*cp++ = '\0';
			while (ISSPACE(*cp))
				cp++;
			if (ISEOL(*cp))
				eol = 1;
		}
		if (ntok == MAXTOKENS)
			eol = 1;
	}

	/*
	 * Return the match
	 */
	*ntokens = ntok;
	ntok = matchkey(tokenlist[0], keywords);
	if (ntok == CONFIG_UNKNOWN)
		goto again;
	return ntok;
}



/*
 * matchkey - match a keyword to a list
 */
static int
matchkey(word, keys)
	register char *word;
	register struct keyword *keys;
{
	for (;;) {
		if (keys->keytype == CONFIG_UNKNOWN) {
			syslog(LOG_ERR,
			    "configure: keyword \"%s\" unknown, line ignored",
			    word);
			return CONFIG_UNKNOWN;
		}
		if (STRSAME(word, keys->text))
			return keys->keytype;
		keys++;
	}
}


/*
 * getnetnum - return a net number (this is crude, but careful)
 */
int
getnetnum(num, addr, complain)
	char *num;
	struct sockaddr_in *addr;
	int complain;
{
	register char *cp;
	register char *bp;
	register int i;
	register int temp;
	char buf[80];		/* will core dump on really stupid stuff */
	u_int netnum;
#ifdef DEBUG
	extern char *ntoa();
#endif

	cp = num;
	netnum = 0;
	for (i = 0; i < 4; i++) {
		bp = buf;
		while (isdigit(*cp))
			*bp++ = *cp++;
		if (bp == buf)
			break;

		if (i < 3) {
			if (*cp++ != '.')
				break;
		} else if (*cp != '\0')
			break;

		*bp = '\0';
		temp = atoi(buf);
		if (temp > 255)
			break;
		netnum <<= 8;
		netnum += temp;
#ifdef DEBUG
	if (debug > 3)
		printf("getnetnum %s step %d buf %s temp %d netnum %d\n",
		    num, i, buf, temp, netnum);
#endif
	}

	if (i < 4) {
		if (complain)
			syslog(LOG_ERR,
		    "configure: \"%s\" not valid host number, line ignored",
			    num);
		return 0;
	}

	/*
	 * make up socket address.  Clear it out for neatness.
	 */
	bzero((char *)addr, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(NTP_PORT);
	addr->sin_addr.s_addr = htonl(netnum);
#ifdef DEBUG
	if (debug > 1)
		printf("getnetnum given %s, got %s (%x)\n",
		    num, ntoa(addr), netnum);
#endif
	return 1;
}


/*
 * catchchild - receive the resolver's exit status
 */
static void
catchchild()
{
	/*
	 * We only start up one child, and if we're here
	 * it should have already exited.  Hence the following
	 * shouldn't hang.  If it does, please tell me.
	 */
	(void) wait(0);
}

/*
 * save_resolve - save configuration info into a file for later name resolution
 */
static void
save_resolve(name, mode, version, flags, keyid)
	char *name;
	int mode;
	int version;
	int flags;
	u_int keyid;
{
	extern char *mktemp();

	if (res_fp == NULL) {
		(void) strcpy(res_file, RES_TEMPFILE);
		(void) mktemp(res_file);
		res_fp = fopen(res_file, "w");
		if (res_fp == NULL) {
			syslog(LOG_ERR, "open failed for %s: %m", res_file);
			return;
		}
	}

	(void) fprintf(res_fp, "%s %d %d %d %u\n", name, mode,
	    version, flags, keyid);
}


/*
 * abort_resolve - terminate the resolver stuff and delete the file
 */
static void
abort_resolve()
{
	/*
	 * In an ideal world we would might reread the file and
	 * log the hosts which aren't getting configured.  Since
	 * this is too much work, however, just close and delete
	 * the temp file.
	 */
	if (res_fp != NULL)
		(void) fclose(res_fp);
	res_fp = NULL;

	(void) unlink(res_file);
}


/*
 * do_resolve - start up the resolver program
 */
static void
do_resolve(program, auth_keyid, keyfile)
	char *program;
	u_int auth_keyid;
	char *keyfile;
{
	register int i;
	register char **ap;
	/* 1 progname + 5 -d's + 1 -r + keyid + keyfile + tempfile + 1 */
	char *argv[15];
	char numbuf[15];
	/*
	 * Clean environment so the resolver is consistant
	 */
	static char *resenv[] = {
		"HOME=/",
		"SHELL=/bin/sh",
		"TERM=dumb",
		"USER=root",
		NULL
	};

	if (res_fp == NULL) {
		
		syslog(LOG_ERR, "internal error in do_resolve: res_fp == NULL");
		exit(1);
	}
	(void) fclose(res_fp);
	res_fp = NULL;

	ap = argv;
	*ap++ = program;

	/*
	 * xntpres [-d ...] -r key# keyfile tempfile
	 */
#ifdef DEBUG
	i = debug;
	if (i > 5)
		i = 5;
	while (i-- > 0)
		*ap++ = "-d";
#endif
	*ap++ = "-r";

	(void) sprintf(numbuf, "%u", auth_keyid);
	*ap++ = numbuf;
	*ap++ = keyfile;
	*ap++ = res_file;
	*ap = NULL;

	(void) signal(SIGCHLD, (void *)catchchild);

	i = fork();
	if (i == 0) {
		/*
		 * In child here, close up all descriptors and
		 * exec the resolver program.  Close the syslog()
		 * facility gracefully in case we must reopen it.
		 */
		(void) signal(SIGCHLD, SIG_DFL);
		closelog();
		i = getdtablesize();
#ifdef DEBUG
		while (i-- > 2)
#else
		while (i-- > 0)
#endif
			(void) close(i);
		(void) execve(program, argv, resenv);

		/*
		 * If we got here, the exec screwed up.  Open the log file
		 * and print something so we don't die without complaint
		 */
#ifndef	LOG_DAEMON
		openlog("xntpd", LOG_PID);
#else
#ifndef	LOG_NTP
#define	LOG_NTP	LOG_DAEMON
#endif
		openlog("xntpd", LOG_PID | LOG_NDELAY, LOG_NTP);
#endif	/* LOG_DAEMON */
		syslog(LOG_ERR, "exec of resolver %s failed!", program);
		abort_resolve();
		exit(1);
	}

	if (i == -1) {
		syslog(LOG_ERR, "fork() failed, can't start %s", program);
		(void) signal(SIGCHLD, SIG_DFL);
		abort_resolve();
	}
}
