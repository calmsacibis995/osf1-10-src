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
static char	*sccsid = "@(#)initcond.c	3.1	(ULTRIX/OSF)	2/26/91";
#endif 
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 *
 * This Module contains Proprietary Information of SecureWare, Inc.
 * and should be treated as Confidential.
 */


/*
 * Based on:

 */


#include <sys/secdefines.h>

#if SEC_BASE

/*
 * This library command is an auxiliary helper for init and getty.
 * The actual work with the authentication database is done here to
 * save space on init getty, which stay around in memory for a long time.
 * This command has a short execution time.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <pwd.h>

#include <sys/security.h>
#include <sys/audit.h>
#if SEC_MAC || SEC_ILB
#include <mandatory.h>
#endif
#if SEC_NCAV
#include <ncav.h>
#endif
#include <prot.h>
#include <grp.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "initcond_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_INITCOND,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#define	LOG			"/tcb/files/initcondlog"
#define	SECURE_GETTY_MODE	0600
#define	INSECURE_GETTY_MODE	0622
#define	DEV_HEADER		"/dev/"


static int init_authenticate();
static void init_update_tty();
static void getty_condition_line();
static void stopio_all_devs();
static void get_line_variant_names();
static struct pr_term *check_devasg();
#ifdef _OSF_SOURCE
static int init_shell();
#endif


extern char *strcpy(), *strcat(), *malloc(), *strrchr();
extern struct group *getgrnam();
extern struct passwd *getpwnam();


usage()
{
#ifdef _OSF_SOURCE
	fprintf(stderr, MSGSTR(USAGE1,
		"Usage: %s [init|getty|init_shell] args . . .\r\n"),
		command_name);
#else
	fprintf(stderr, MSGSTR(USAGE2,
		"Usage: %s [init|getty] args . . .\r\n", command_name);
#endif
}

main(argc, argv)
	int argc;
	char *argv[];
{
#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_INITCOND,NL_CAT_LOCALE);
#endif

	set_auth_parameters(argc, argv);

	if (argc < 2)  {
		usage();
		exit(1);
	}

	if (getluid() != -1)  {
		fprintf(stderr, MSGSTR(NORUN,
			"%s: Cannot run after a session is established.\r\n"),
			command_name);
		exit(1);
	}

	if (strcmp(argv[1], "init") == 0)
	    switch (argc)  {
		case 2:
			exit(init_authenticate(0));	/* multi-user boot */
			break;
		case 3:
			exit(init_authenticate(1));	/* single-user boot */
			break;
		case 4:
			init_update_tty(argv[2], argv[3]);
			break;
		default:
			usage();
			exit(1);
			break;
	    }
	else if (strcmp(argv[1], "getty") == 0)
	    switch (argc)  {
		case 3:
			getty_condition_line(argv[2]);
			break;
		default:
			usage();
			exit(1);
			break;
	    }
#ifdef _OSF_SOURCE
	else if (strcmp(argv[1], "init_shell") == 0 && argc <= 6)
		exit(init_shell(argc - 2, &argv[2]));
#endif
	else  {
		usage();
		exit(1);
	}

	exit(0);
}


/*
 * Loop until the user enters a login name and password for an account
 * that is authorized to boot the machine.  If booting to single user
 * mode, indicated by the is_single argument being non-zero, require
 * the "sysadmin" authorization.  Otherwise, require the "boot"
 * authorization.
 */
static int
init_authenticate(is_single)
	int	is_single;
{
	register struct pr_default *df;
	register struct pr_passwd *pr;
	register char *cleartext, *ciphertext;
	char username[9];
	int delay = 0;
	mask_t *cprivs;

	enter_quiet_zone();

	/*
	 * Read the system defaults database to find out if boot
	 * authentication is required.
	 */

	df = getprdfnam("default");
	if (df == (struct pr_default *) 0) {
/*		if(audgenl( AUTH_EVENT,
		     T_CHARP, MSGSTR(NOBOOT, "cannot get system defaults for boot authentication"), 0) == -1 )
			perror("audgenl");
*/
		fprintf(stderr, MSGSTR(NODATABASE,
			"init: can't read defaults database\r\n"));
		return 1;
	}
	if (!df->sflg.fg_boot_authenticate || !df->sfld.fd_boot_authenticate) {
/*
		if(audgenl(AUTH_EVENT, T_CHARP, MSGSTR(BOOTAUTH1 , "boot authentication"),T_CHARP, MSGSTR(NOTREQ, "not required"),0) == -1)
			perror("audgenl");
*/
		return 0;
	}

	if (df->tcg.fg_logdelay)
		delay = df->tcd.fd_logdelay;

	printf(MSGSTR(BOOTAUTH2,"\nBoot Authentication:\n\n"));

	/*
	 * The following loop prompts for user name and password until
	 * an account with appropriate authorization to boot the system
	 * is authenticated.
	 */

	for (;; sleep(delay)) {
		do {
			printf(MSGSTR(ENTERNAME,"Please enter your login name: "));
		} while (fgets(username, sizeof username, stdin) == NULL);

		if (*username)	/* drop newline */
			username[strlen(username) - 1] = '\0';

		pr = getprpwnam(username);
		if (pr == (struct pr_passwd *) 0) {
			/*
			 * Either the user name is invalid or its protected
			 * password entry is corrupted.  Prompt for a password
			 * anyway to avoid revealing validity of user name.
			 */
			(void) getpasswd(MSGSTR(DPASSWD,	"Password:"),
				AUTH_MAX_PASSWD_LENGTH);
			if(audgenl(AUTH_EVENT, T_CHARP, MSGSTR(UNRECOG,
			    "unrecognized user name for boot authentication"),0) == -1)
				perror("audgenl");
			fprintf(stderr, MSGSTR(AUTHFAILED,
				"Authentication incorrect\n\n"));
			continue;
		}

		/*
		 * A valid user name was entered.  Prompt for and check
		 * the password if there is one on the account.
		 */
		if (pr->uflg.fg_encrypt && *pr->ufld.fd_encrypt) {
			ciphertext = pr->ufld.fd_encrypt;
			cleartext = getpasswd(MSGSTR(DPASSWD, "Password:"),
						AUTH_MAX_PASSWD_LENGTH);
			if (cleartext && strcmp(bigcrypt(cleartext, ciphertext),ciphertext) != 0) {
				if(audgenl(AUTH_EVENT,T_LOGIN, pr->ufld.fd_name,T_CHARP,
					 MSGSTR(BOOTFAILED1, "boot authentication failed - bad password"),0) == -1)
					perror("audgenl");
				fprintf(stderr, MSGSTR(AUTHFAILED,
					"Authentication incorrect\n\n"));
				continue;
			}
			/* erase the cleartext buffer */
			getpasswd(NULL, AUTH_MAX_PASSWD_LENGTH);
		}

		/*
		 * The user is authenticated.  Now check his authorization
		 * to boot the machine.
		 */
		if (pr->uflg.fg_cprivs)
			cprivs = pr->ufld.fd_cprivs;
		else if (pr->sflg.fg_cprivs)
			cprivs = pr->sfld.fd_cprivs;
		else
			break;
		if (is_single) {
			if (hascmdauth("sysadmin", cprivs))
				break;
		} else {
			if (hascmdauth("boot", cprivs))
				break;
		}

		if(audgenl(AUTH_EVENT, T_LOGIN, pr->ufld.fd_name, T_UID, pr->ufld.fd_uid, T_CHARP, MSGSTR(BOOTFAILED2, "boot authentication failed - not authorized"),0)== -1)
			perror("audgenl");
		fprintf(stderr, MSGSTR(NOAUTH,
			"No authorization to boot the system\n\n"));
	}

	if(audgenl(AUTH_EVENT, T_LOGIN, pr->ufld.fd_name, T_UID, pr->ufld.fd_uid, T_CHARP, MSGSTR(AUTHSUCCEED, "boot authentication succeeded"),0) == -1 )
		perror("audgenl");
	exit_quiet_zone();
	return 0;
}


/*
 * Do 2 things.  First, upate the terminal control database with the
 * logout time of the session just ended on the line, and if obtainable
 * (which it should be for all user sessions, but not recycled inits and
 * gettys), note the user that logged into the line.
 * Second, always disable further I/O to the line with the stopio() call and
 * reset the line ownership.
 */
static void
init_update_tty(line_name, user)
	register char *line_name;
	register char *user;
{
	struct pr_term *prtc;
	struct pr_passwd *pr;
	struct group *g;
	register int term_group;
	struct pr_term save_data;
	char *full_line;
	char *line_tail;
	struct stat stat_buf;

	get_line_variant_names("init", line_name, &full_line, &line_tail);

	prtc = getprtcnam(line_tail);

	/* check device assignment database synonyms */
	if (prtc == (struct pr_term *) 0)
		prtc = check_devasg(full_line);

	if (prtc == (struct pr_term *) 0)  {
		/*
		 * No reason to fail here because network logins don't
		 * have an associated terminal control database entry.
		 * Still need to regrade the device below, however.
		 */
		;
	}
	else  {
		save_data = *prtc;
		prtc = &save_data;

		prtc->uflg.fg_louttime = 1;
		prtc->ufld.fd_louttime = time ((long *) 0);
		prtc->uflg.fg_loutuid = 0;

		/*
		 * If user is the null string, the last activity on the terminal
		 * was something other than a normal getty/login/shell sequence.
		 * In that case, we don't want to update the terminal control
		 * database, because it only wants to know of updates due to
		 * real logins.
		 *
		 * If the user name is LOGIN, that is really getty filling the
		 * user field before login does.  That also does not count as
		 * a true login.
		 */
		if ((user[0] != '\0') && (strcmp("LOGIN", user) != 0))  {
			pr = getprpwnam(user);
			if (pr == (struct pr_passwd *) 0)  {
/*				if(audgenl(AUTH_EVENT, T_CHARP, 
				     MSGSTR(NOTLOCATE,
				     "cannot locate Protected Password entry"),
				     0) == -1)
					perror("audgenl");
*/
				fprintf(stderr, MSGSTR (NOTPROTECT,
			"init: cannot find protected password entry for %s\r\n"),
					user);
			}
			else if (pr->uflg.fg_uid)  {
				prtc->uflg.fg_loutuid = 1;
				prtc->ufld.fd_loutuid = pr->ufld.fd_uid;
				if(audgenl(LOGOUT, T_LOGIN, pr->ufld.fd_name, T_UID, pr->ufld.fd_uid, T_CHARP, "logoff", T_DEVNAME, prtc->ufld.fd_devname,0) == -1)
					perror("audgenl");
			}

			if (!putprtcnam(line_tail, prtc))
				fprintf(stderr, MSGSTR(NOTERMINAL,
			"init: cannot update terminal control entry for %s\r\n"),
				line_tail);
		}
	}

	/*
	 * Reset the terminal so that after we stop I/O to it, we
	 * make sure no one can re-open it.
	 */
	g = getgrnam("terminal");
	if (g == (struct group *) 0)  {
		if (stat(full_line, &stat_buf) == 0)
			term_group = stat_buf.st_gid;
		else
			term_group = 0;
	}
	else
		term_group = g->gr_gid;

	(void) chmod(full_line, 0);
	(void) chown(full_line, 0, term_group);

#ifdef sledge
#if SEC_MAC || SEC_NCAV
	/*
	 * Raise the sensitivity level to the highest level when the line is not
	 * in use.  If it were set to Syslo or left alone, a small flow
	 * would exist and privacy a bit compromised by periodically
	 * querying the tty line.  This way, the only time the device is
	 * accessible is when it is meant to be.
	 */

	if (islabeledfs("/")) {
#if SEC_MAC
		mand_init();
#endif
#if SEC_NCAV
		ncav_init();
#endif

		terminal_regrade(prtc,
#if SEC_MAC
			mand_syshi,
#else
			0,
#endif
#if SEC_NCAV
			ncav_max_ir
#else
			0
#endif
			);
#if SEC_MAC
		mand_end();
#endif
#if SEC_NCAV
		ncav_end();
#endif
	}
#endif /* SEC_MAC || SEC_NCAV */
#endif /* sledge */

	endpwent();
	endgrent();
	endprpwent();
	endprdfent();
	endprtcent();
	endprfient();

	/*
	 * Even though this is done in getty before starting a new session,
	 * it should also be done at session close since it is possible that
	 * getty may not be the program called by init the next time this
	 * line is used.  Since this is called on a dead process when the
	 * previous process was of state USER_PROCESS (the typical case),
	 * LOGIN_PROCESS (if getty calls a program other than login), or
	 * INIT_PROCESS (if init calls an program other than getty).
	 *
	 * Note that daemons that write to /dev/console may die if a session
	 * ends on /dev/console and the daemon held /dev/console open.
	 * This requires that daemons open /dev/console only when they need
	 * to write, and close again right after to avoid the SIGHUP.
	 */

	stopio_all_devs (full_line);

	free(full_line);
	free(line_tail);
}


/*
 * Just prior to opening the terminal, set up the modes properly and
 * use stopio() to revoke all rights to the line by processes that already
 * have the line open.  Future opens (like the one about to be done in getty)
 * are not affected.
 */
static void
getty_condition_line(line)
	register char *line;
{
	register int uid_to_set;
	register int gid_to_set;
	register struct group *g;
	register struct passwd *p;
	struct stat line_data;
	char *full_line;
	char *line_tail;


	/*
	 * Open this non-character special file so that any library
	 * routines that write to stdout/stderr will have somewhere
	 * to go.  If we opened /dev/null, /dev/console or any other
	 * character special file at this point, the controlling terminal
	 * for the session would be fixed to that, so we can't do the
	 * thing we really want to do here.
	 */
	(void) freopen(LOG, "a", stdin);
	(void) freopen(LOG, "a", stdout);
	(void) freopen(LOG, "a", stderr);

	get_line_variant_names("getty", line, &full_line, &line_tail);

	if (stat(full_line, &line_data) != 0)  {
/*
		if(audgenl(AUTH_EVENT, T_CHARP,
			MSGSTR(TERMSTAT, "status of terminal"),
			T_CHARP, MSGSTR(FAILURE, "failure"), 0)== -1)
			perror("audgenl");
*/
		fprintf(stderr, MSGSTR(NOTGETSTATUS,
			"getty: cannot get status of %s\r\n"), line);
		exit(1);
	}
	if ((line_data.st_mode & S_IFMT) != S_IFCHR)  {
/*
		if(audgenl(AUTH_EVENT, T_CHARP,
			MSGSTR(TERNTYPE, "type of file for terminal"),
			T_CHARP,MSGSTR(NOTATERM, "not a terminal"), 0) ==-1)
			perror("audgenl");
*/
		fprintf(stderr, MSGSTR(GNOTERM,
			"getty: %s is not a terminal\r\n"), line);
		exit(1);
	}


	p = getpwnam("bin");
	if (p == (struct passwd *) 0) {
/*
		if(audgenl(AUTH_EVENT, T_CHARP, MSGSTR(TERMINAL, "terminal"), 
			T_CHARP, MSGSTR(NOTRETRIVE, "cannot retrieve"), 0) == -1)
			perror("audgenl");
*/
		uid_to_set = 0;
	}
	else
		uid_to_set = p->pw_uid;

	g = getgrnam("terminal");
	if (g == (struct group *) 0)  {
/*
		if(audgenl(AUTH_EVENT, T_CHARP,
			MSGSTR(TERMINAL, "terminal"), T_CHARP,
			MSGSTR(NOTRETRIVE, "cannot retrieve"), 0) == -1)
			perror("audgenl");
*/
		gid_to_set = line_data.st_gid;
	}
	else
		gid_to_set = g->gr_gid;

	endgrent();

	/*
	 * Until login, keep this terminal away from anyone's
	 * writing.  We must do this prior to the stopio() call
	 * to prevent a window of opportunity for a penetrator.
	 */
	if (chmod(full_line, SECURE_GETTY_MODE) != 0)  {
/*
		if(audgenl(AUTH_EVENT,T_CHARP, MSGSTR(CHMOD,
		  "change mode of terminal to prevent opening by users"),
		  T_CHARP, MSGSTR(FAILURE, "failure"), 0) == -1)
			perror("audgenl");
*/
		fprintf(stderr, MSGSTR(GNOMODE,
			"getty: cannot reset mode of %s\r\n"),
			line);
	}
	if (chown(full_line, uid_to_set, gid_to_set) != 0)  {
/*
		if(audgenl(AUTH_EVENT,T_CHARP, MSGSTR(GNOOWNER,
			"change owner of terminal to prevent opening by users"),
			T_CHARP,MSGSTR(FAILURE, "failure"), 0) == -1)
			perror("audgenl");
*/
		fprintf(stderr, MSGSTR(GNOOWNER,
			"getty: cannot reset ownership of %s\r\n"),
			line);
	}

	/*
	 * Disables all past I/O on this line.  This makes the
	 * line fresh for the new session.  Open file descriptors
	 * to this file remain, but all future attempts at I/O
	 * with them will cause SIGSYS to be sent to the process that
	 * is attempting the I/O.
	 */
	stopio_all_devs (full_line);

	sync();

	free(full_line);
	free(line_tail);

	endpwent();
	endprpwent();
	endprdfent();
	endprtcent();
	endprfient();

	(void) fclose(stdin);
	(void) fclose(stdout);
	(void) fclose(stderr);
}

#ifdef sledge
#if SEC_MAC || SEC_NCAV
/*
 * terminal_regrade()-this routine will perform the necessary functions to
 * regrade the sensitivity level and caveat set of a terminal device. To
 * do so, the existing device nodes must be removed and re-created to avoid
 * the link count enforcement in the setlabel(2) code. This check enforces
 * the mandatory increasing tree. By removing and re-creating the devices,
 * it can be assured that the reference count on the node will be one when
 * opened by getty or zero in the case of initcond. If the count is > 1,
 * ordinarily the setlabel(2) will fail. However, if the link count == 2
 * and the target is a character special device and the process is privileged
 * the call will succeed. The count will be two when he chslabel(2) call is
 * made since it does an namei() on the target pathname.
 */

terminal_regrade(prtc,mand_ir,ncav_ir)
struct pr_term *prtc;
#if SEC_MAC
mand_ir_t *mand_ir;
#else
int mand_ir;
#endif
#if SEC_NCAV
ncav_ir_t *ncav_ir;
#else
int ncav_ir;
#endif
{
	struct dev_asg *dev;
	int terminal_uid, terminal_gid;
	int i, devcount;
	struct stat statbuf;
	char newdev[20];
	char devnm[14];

	if(!prtc->uflg.fg_devname)
		return(0);

	/*
	 * Get the new owner and group values for the device.
	 */

	if((terminal_uid = pw_nametoid("bin")) == -1)
		terminal_uid = 0;

	if((terminal_gid = gr_nametoid("terminal")) == -1)
		terminal_gid = 0;

	/*
	 * Get the device assignment database entry.
	 */

	if((dev = getdvagnam(prtc->ufld.fd_devname)) == (struct dev_asg *) 0) {
		fprintf(stderr, MSGSTR(NOGETTERM,
			"Can not get terminal device assignment entry"));
		return(0);
	}

	strncpy(devnm,prtc->ufld.fd_devname,12);

	/*
	 * Stat(2) the device to get the major/minor for mknod(2)
	 */

	sprintf(newdev,"/dev/%s",prtc->ufld.fd_devname);

	if(stat(newdev,&statbuf) == -1) {
		fprintf(stderr, MSGSTR(NOTERMSTAT, "Unable to stat terminal device"));
		return(0);
	}

	/*
	 * Count the number of alias devices
	 */

	for(i=0; ; i++) {
		if(dev->ufld.fd_devs[i])
			devcount++;
		else break;
	}

	/*
	 * For the real device, create a new temporary node. Then unlink
	 * the real device name. Next, link the real device to the new
	 * node giving a link count of 2 for the new device inode. Then
	 * lastly, remove the temporary node leaving a new device inode
	 * for the real device with a link count of 1. The mode of the
	 * new node is left 000 to prevent discretionary access.
	 */

	sprintf(newdev,"/dev/%s-t",devnm);
	if(mknod(newdev,S_IFCHR,statbuf.st_rdev) == -1) {
		fprintf(stderr, MSGSTR(NOTCREATE, "Can not create temporary node"));
		return(0);
	}

	if(chmod(newdev,0) == -1) {
		fprintf(stderr, MSGSTR(NOCHMOD, "Unable to chmod new device node\n"));
		return(0);
	}

	if(chown(newdev,terminal_uid,terminal_gid) == -1) {
		fprintf(stderr, MSGSTR(NOCHOWN,
			"Unable to chown new device node\n"));
		return(0);
	}

	unlink(dev->ufld.fd_devs[0]);
	if(link(newdev,dev->ufld.fd_devs[0]) == -1) {
		fprintf(stderr, MSGSTR(NOLINK,
			"Unable to link new device node\n"));
		return(0);
	}
	unlink(newdev);

#if SEC_MAC
	/*
	 * Regrade the device according to the specified sensitivity level.
	 */

	if(chslabel(dev->ufld.fd_devs[0],mand_ir) == -1) {
		fprintf(stderr, MSGSTR(NOREGADE,
			"Unable to regade terminal device\n"));
		return(0);
	}
#endif

#if SEC_NCAV
	/*
	 * Modify the caveat set on the terminal device.
	 */

	if(chncav(dev->ufld.fd_devs[0],ncav_ir) == -1) {
		fprintf(stderr, MSGSTR(NORESET,
			"Unable to reset terminal caveat set\n");
		return(0);
	}
#endif

	/*
	 * If the device count is > 1, then alias devices exist. The
	 * aliases must be unlinked and relinked to the new real device
	 * node after it has been regraded.
	 */

	if(devcount > 1) {
		for(i=1;  i < devcount; i++) {
			if(!dev->ufld.fd_devs[i])
				break;

			unlink(dev->ufld.fd_devs[i]);
			link(dev->ufld.fd_devs[0],dev->ufld.fd_devs[i]);
		}
	}

	return(1);
}
#endif
#endif /* sledge */

/*
 * check the device assignment database for a synonym device matching the
 * terminal pathname specified.  If found, return the corresponding terminal
 * control database entry.
 */

static struct pr_term *
check_devasg(tty)
char *tty;
{
	int i;
	struct dev_asg *dvag;
	char *cp;

	setdvagent();
	while ((dvag = getdvagent()) != (struct dev_asg *) 0)
		if (dvag->uflg.fg_devs)
			for (i = 0; cp = dvag->ufld.fd_devs[i]; i++)
				if (strcmp(cp, tty) == 0)
					return getprtcnam(dvag->ufld.fd_name);
	return (struct pr_term *) 0;
}

/*
 * stop io on all devices which are synonymous with the terminal device.
 * The device names are stored in the device assignment database.
 */

static void
stopio_all_devs (line)
char *line;
{
	register struct dev_asg *dv;
	register char *cp;
	register int i;
	char found = 0;

	setdvagent();
	while (!found && (dv = getdvagent()) != (struct dev_asg *) 0) {
		if (!dv->uflg.fg_devs)
			continue;
		for (i = 0; cp = dv->ufld.fd_devs[i]; i++)
			if (strcmp (cp, line) == 0) {
				found = 1;
				break;
			}
	}
	/* if no entry contains the device, just stopio the device itself */
	if (!found) {
		if (stopio(line) != 0) {
/*
			if(audgenl(AUTH_EVENT, T_CHARP,
				MSGSTR(PREVENTTERM, "prevent use of terminal"),
				T_CHARP,MSGSTR(FAILURE, "failure"), 0) == -1)
				perror("audgenl");
*/
			fprintf(stderr, "init: could not stop I/O on %s.\r\n",
				line);
		}
	} else  /* otherwise, stopio on all device synonyms */
		for (i = 0; cp = dv->ufld.fd_devs[i]; i++)
			if (stopio (cp) != 0) {
/*
				if(audgenl(AUTH_EVENT, T_CHARP,
				  MSGSTR(PREVENTTERM, "prevent use of terminal"),
				  T_CHARP,MSGSTR(FAILURE, "failure"), 0) == -1)
					perror("audgenl");
*/
				fprintf(stderr,
					MSGSTR(INITNOSTOP, "init: could not stop I/O on %s.\r\n"),
					cp);
			}
	enddvagent();
}



/*
 * The input argument line_name refers to a device -- either of the
 * form /dev/ttyxx (or /dev/console, ...) or the form ttyx (or console, ...).
 * This routine determines which type and returns the full pathname
 * in the full_line variable and the relative pathname from /dev in
 * line_tail.  Each of those arguments points to malloc'd space.
 */
static void
get_line_variant_names(cmd, line_name, full_line, line_tail)
	char *cmd;
	register char *line_name;
	register char **full_line;
	register char **line_tail;
{
	register int header_size;
	register int line_name_size;

	header_size = strlen(DEV_HEADER);
	line_name_size = strlen(line_name);

	if (strncmp(line_name, DEV_HEADER, header_size) == 0)  {
		/*
		 * The argument was /dev/ttyxx.
		 */
		*full_line = malloc(line_name_size + 1);
		if (*full_line == (char *) 0)  {
			fprintf(stderr, MSGSTR(NOMEMTTYPATH,
				"Unable to get memory to save tty path\n"));
			exit(1);
		}
		(void) strcpy(*full_line, line_name);

		*line_tail = malloc(line_name_size - header_size + 1);
		if (*line_tail == (char *) 0)  {
			fprintf(stderr, MSGSTR(NOMEMTTYNAME,
				"Unable to get memory to build tty name\n"));
			exit(1);
		}
		(void) strcpy(*line_tail, &line_name[header_size]);
	}

	else  {
		/*
		 * The argument was ttyxx.
		 */
		*full_line = malloc(line_name_size + header_size + 1);
		if (*full_line == (char *) 0)  {
			fprintf(stderr, MSGSTR(NOMEMTTYNAME,
				"Unable to get memory to build tty name\n"));
			exit(1);
		}
		(void) strcpy(*full_line, DEV_HEADER);
		(void) strcat(*full_line, line_name);

		*line_tail = malloc(line_name_size + 1);
		if (*line_tail == (char *) 0)  {
			fprintf(stderr, MSGSTR(NOMEMTTYNAME,
				"Unable to get memory to build tty name\n"));
			exit(1);
		}
		(void) strcpy(*line_tail, line_name);
	}
}

#ifdef _OSF_SOURCE /*{*/

/*
 * Start a shell at the terminal with the passed uid and gid and current dir
 */

static int
init_shell(argc, argv)
	int	argc;
	char	*argv[];

{
	char			*shell	= argc > 0 ? argv[0] : "/sbin/sh";
	int			user	= argc > 1 ? atoi(argv[1]) : 0;
	int			group	= argc > 2 ? atoi(argv[2]) : 0;
	char			*dir	= argc > 3 ? argv[3] : NULL;
	struct passwd		*pwd;
	struct pr_passwd	*pr;
	struct pr_default	*prd;
#if SEC_MAC
	mand_ir_t		*mand_ptr;
#endif
	priv_t	*priv_ptr, *base_ptr;
	char	*cmd_ptr, *arg_ptr;

	pwd = getpwuid(user);
	if (pwd == (struct passwd *) 0)
		printf(MSGSTR(NOLOCIDPW,
			"Unable to locate user ID %d in /etc/passwd\n"), user);

	setluid(user);
	setgid(group);
#if SEC_GROUPS
	if (pwd)
		initgroups(pwd->pw_name, group);
#endif
	setuid(user);

	if (pwd)
		pr = getprpwnam(pwd->pw_name);
	else
		pr = getprpwuid(user);
	if (pr == (struct pr_passwd *) 0) {
		printf(MSGSTR(NOLOCPDB,
			"Unable to locate user %d in protected password DB\n"),
		  user);
#if SEC_MAC
		if (mand_init() == 0) {
			if (setclrnce(mand_syslo))
				printf(MSGSTR(NOSETCLR,
					"Unable to set clearance\n"));
			if (setslabel(mand_syslo))
				printf(MSGSTR(NOSETSENS,
					"Unable to set sensitivity label\n"));
#if SEC_ILB
			if (setilabel(mand_syslo))
				printf(MSGSTR(NOSETILB,
					"Unable to set information label\n"));
#endif
		} else
			printf(MSGSTR(NOSETPLBL,
				"Unable to set process labels\n"));
#endif /* SEC_MAC */
		/* Leave privileges in their initial state */
	} else {
#if SEC_MAC
		/*
		 * Don't rely on getprpwuid to have called mand_init.
		 * It is possible that neither the protected password
		 * file nor the default database contains a label.
		 */
		if (mand_init() == 0) {
			if (pr->uflg.fg_clearance)
				mand_ptr = &pr->ufld.fd_clearance;
			else if (pr->sflg.fg_clearance)
				mand_ptr = &pr->sfld.fd_clearance;
			else
				mand_ptr = mand_syslo;
			if (setclrnce(mand_ptr))
				printf(MSGSTR(NOSETCLR,
					"Unable to set clearance\n"));

			/*
			 * Get single user sensitivity label from default
			 * database.
			 */
			setprdfent();
			if ((prd = getprdfent()) != (struct pr_default *) 0 &&
					prd->sflg.fg_single_user_sl)
				mand_ptr = prd->sfld.fd_single_user_sl;
			else
				mand_ptr = mand_syslo;
			if (setslabel(mand_ptr))
				printf(MSGSTR(NOSETSENS,
					"Unable to set sensitivity label\n"));
#if SEC_ILB
			if (setilabel(mand_syslo))
				printf(MSGSTR(NOSETILB,
					"Unable to set information label\n"));
#endif
		} else
			printf(MSGSTR(NOSETPLBL,
				"Unable to set process labels\n"));
#endif /* SEC_MAC */
		if (pr->uflg.fg_sprivs)
			priv_ptr = pr->ufld.fd_sprivs;
		else if (pr->sflg.fg_sprivs)
			priv_ptr = pr->sfld.fd_sprivs;
		else
			priv_ptr = (priv_t *) 0;

		if (pr->uflg.fg_bprivs)
			base_ptr = pr->ufld.fd_bprivs;
		else if (pr->sflg.fg_bprivs)
			base_ptr = pr->sfld.fd_bprivs;
		else
			base_ptr = (priv_t *) 0;
		
		if (priv_ptr && setpriv(SEC_MAXIMUM_PRIV, priv_ptr))
			printf(MSGSTR(NOKERNAUTH,
				"Unable to set kernel authorizations\n"));
		if (base_ptr && setpriv(SEC_BASE_PRIV, base_ptr))
			printf(MSGSTR(NOSETBASEPRIV,
				"Unable to set base privileges\n"));
	}

	endprdfent();
	endprpwent();
	endpwent();

	if (dir == (char *) 0 && pwd)
		dir = pwd->pw_dir;
	if (dir && chdir(dir))
		printf(MSGSTR(NOCHDIR,"Unable to change directory to %s\n"),dir);

	cmd_ptr = strrchr(shell, '/');
	if (cmd_ptr == (char *) 0)
		cmd_ptr = shell;
	else
		cmd_ptr++;
	
	arg_ptr = (char *) malloc(strlen(cmd_ptr) + 2);
	if (!arg_ptr)
		arg_ptr = cmd_ptr;
	else {
		strcpy(arg_ptr, "-");
		strcat(arg_ptr, cmd_ptr);
	}

	execl(shell, arg_ptr, (char *) 0);
	perror(shell);
	return(-1);
}
#endif /*} _OSF_SOURCE */
#endif /*} SEC_BASE */
