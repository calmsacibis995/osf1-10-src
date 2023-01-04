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
static char	*sccsid = "@(#)login_sec.c	3.1	(ULTRIX/OSF)	2/26/91";
#endif 
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

#include <sys/secdefines.h>

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <sys/user.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ttyent.h>
#if SEC_NET_TTY
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <sys/security.h>
#include <sys/audit.h>
#ifndef	T_ERRNO
#define	T_ERRNO	T_ERROR
#endif
#include <sys/sec_objects.h>
#include <prot.h>
#include <protcmd.h>
#ifndef	LOGIN_PROGRAM
#define	LOGIN_PROGRAM	"/usr/bin/login"
#endif
#ifndef	PASSWD_PROGRAM
#define	PASSWD_PROGRAM	"/usr/bin/passwd"
#endif
#ifndef TPLOGIN
#if SEC_MAC || SEC_NCAV
#include <sys/secpolicy.h>
#include <fcntl.h>
#if SEC_MAC
#include <mandatory.h>
#endif
#if SEC_NCAV
#include <ncav.h>
#endif
#endif /* SEC_MAC || SEC_NCAV */
#endif /* !TPLOGIN */

#ifndef	TTY_PERMISSION
#define	TTY_PERMISSION 0620	/* Allow writes from group 'tty' */
#endif


#ifdef NLS
#include <locale.h>
#endif

#include "sia_mech.h"
#define MSGSTR_SEC(n,s) GETMSG(MS_SIA_LOGIN,n,s)

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif



#define	ATTEMPTS	4	/* login attempts allowed this connection */
#define	TYPEIN_TIME	10	/* in seconds, this time is liberal */
#define	NO_ENTRY	"::"
#define	SYSTEM_CONSOLE	"console"
#define OVERRIDE_USER	"root"

static char auditbuf[80];
extern int (*_c2_collect)();
extern int _c2_collinput;

static struct pr_passwd *current_prpwd = (struct pr_passwd *) 0;
static struct pr_term *current_prtc = (struct pr_term *) 0;
static struct pr_passwd noprot;
static struct pr_default dflt;
#ifdef BSD_TTY
static struct sgttyb initial_settings;
#else
static struct termio initial_settings;
#endif
static char *save_tty_path = (char *) 0;
static int save_tty_isatty = 1;
static int term_group = -1;
static int delay_between_attempts = 0;
static mask_t nosprivs[SEC_SPRIVVEC_SIZE];
static struct passwd **pwd_ref;
static struct passwd *pwd;
static char bad_username[9];
static int	timeout_occurred = 0 ;
#ifndef TPLOGIN
#if SEC_MAC && !SEC_SHW
static mand_ir_t *sec_level_ir = (mand_ir_t *) 0;
#endif
#if SEC_NCAV
static ncav_ir_t *ncav_ir = (ncav_ir_t *) 0;
#endif
#endif /* !TPLOGIN */
#if SEC_NET_TTY
static int is_network = 0;
#endif

extern struct passwd nouser;
#ifndef TPLOGIN
#if SEC_MAC
extern char *er_buffer;	/* declared and used in authaudit.c in libprot */
#endif
#if SEC_NCAV
extern char *ncav_buffer; /* declared and used in authaudit.c in libprot */
#endif
#else /* TPLOGIN */
#if SEC_CHOTS
extern int twoperson_check;	/* declared in tplogin.c */
#endif
#endif /* TPLOGIN */

struct pr_passwd *login_bad_user();
void login_delay();
struct pr_term *login_bad_tty();
char *login_crypt();

static void timeout();
static int execute();
int check_valid_tty();
static void impossible_user();
static void cancel_process();
static void reset_tty_discr_and_bye();
static void end_authentication();
static void failure_override();
static struct pr_term *check_devasg();
#ifndef TPLOGIN
#if SEC_MAC
static int get_sec_level();
#endif
#if SEC_NCAV
static int get_caveat_set();
#endif
#endif /* !TPLOGIN */
#if SEC_NET_TTY
static struct pr_term *check_devasg_net();
#endif


static void
audit_event(code, message)
int code;
caddr_t message;
{
/* Broken...
	audgenl ( LOGIN,
        	T_LOGIN,		current_prpwd->ufld.fd_name,
		T_HOMEDIR, 		pwd->pw_dir,
		T_SHELL,		pwd->pw_shell,
		T_DEVNAME,		current_prtc->ufld.fd_devname,
		T_CHARP,		message,
		code?T_ERRNO:T_RESULT,	code,
		NULL);
*/
		
}

/*
 * Exit the program now.  Used instead of exit(2) alone to update the
 * Authentication database with the failures.
 */
void
login_die(code)
	int code;
{
	cancel_process(code, current_prpwd, current_prtc);
}

/*
 * This routine is called when it expected that a user has a password.
 * If there is no password, this routine forces the user to get one now.
 * Return success indicator of 1 if successful else 0.
 */
int
login_need_passwd(pr, prtc, pnopassword)
	register struct pr_passwd *pr;
	register struct pr_term *prtc;
	register int *pnopassword;
{
	register int passwd_status;
	struct pr_passwd save_data;
	char nullpwok = 0;

	/* If no password required, return a 1 else continue */

	if((pnopassword == (int *) 0) || (*pnopassword == 0))
		return(1);

	if (pr->uflg.fg_nullpw)
		nullpwok = pr->ufld.fd_nullpw;
	else if (pr->sflg.fg_nullpw)
		nullpwok = pr->sfld.fd_nullpw;

	if (nullpwok || pr->uflg.fg_encrypt && pr->ufld.fd_encrypt[0])
		return 1;

	show_mesg(MSGSTR_SEC(LOGIN_SEC_1, "You don't have a password.\n"));
	if(!prtc) return 0;	/* If no tty password cannot be set -DAL */


	/*
	 * We must close and re-open the Protected Password database
	 * here because the passwd program should have updated
	 * the user's entry.  Since pr now points to a static area, we
	 * need to use save_data to hold the data while written.
	 */
	save_data = *pr;
	endprpwent();
	passwd_status = execute(PASSWD_PROGRAM, pr->ufld.fd_uid,
				pr->ufld.fd_name);
	setprpwent();
	pr = getprpwnam(save_data.ufld.fd_name);
	current_prpwd = pr;
	if (pr == (struct pr_passwd *) 0)  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_2,
			"protected password entry vanished after write"));
		show_error(MSGSTR_SEC(LOGIN_SEC_3,
			"Protected Password information suddenly vanished\n"));
		failure_override(&save_data, prtc);
		return 0; /* Don't exit(1) - DAL */
	}

	if (passwd_status < 0) {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_4,
				"cannot execute passwd program"));
		show_error(MSGSTR_SEC(LOGIN_SEC_5,
			"Cannot execute passwd program\n"));
		failure_override(&save_data, prtc);
		cancel_process(1, pr, prtc);
	}
	else if (passwd_status > 0) {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_6,
				"unsuccessful password change"));
		show_error(MSGSTR_SEC(LOGIN_SEC_7,
			"Login aborted due to no password.\n"));
		failure_override(&save_data, prtc);
		cancel_process(1, pr, prtc);
	}

	return(1);
}


/*
 * Check for an expired password.  Invoke the passwd program if the
 * passwd is indeed expired.  Returns the possible changed protected
 * password pointer.
 */
struct pr_passwd *
login_check_expired(pr, prtc)
	register struct pr_passwd *pr;
	register struct pr_term *prtc;
{
	register time_t expiration;
	register time_t last_change;
	time_t expiration_time;
	register time_t now;
	register int passwd_status;
	struct pr_passwd save_data;
	struct pr_default *df;

	now = time((time_t *) 0);

	if (pr->uflg.fg_expdate)
		expiration = pr->ufld.fd_expdate;
	else if (pr->sflg.fg_expdate)
		expiration = pr->sfld.fd_expdate;
	else
		expiration = (time_t) 0;

	df = getprdfnam(AUTH_DEFAULT);

	if (df && df->sflg.fg_pw_expire_warning && expiration &&
	    expiration - now <= df->sfld.fd_pw_expire_warning) {
		expiration_time = expiration;
		show_mesg(MSGSTR_SEC(LOGIN_SEC_128,
			"\nYour account will expire at %s\n"),
			ctime(&expiration_time));
	}

	/*
	 * Null passwords do not expire
	 */
	
	if (!pr->uflg.fg_encrypt || pr->ufld.fd_encrypt[0] == '\0')
		return pr;

	if (pr->uflg.fg_schange)
		last_change = pr->ufld.fd_schange;
	else
		last_change = (time_t) 0;

	if (pr->uflg.fg_expire)
		expiration = pr->ufld.fd_expire;
	else if (pr->sflg.fg_expire)
		expiration = pr->sfld.fd_expire;
	else
		expiration = (time_t) 0;

	/*
	 * A 0 or missing expiration field means there is no
	 * expiration.
	 */
	expiration_time = expiration ? last_change + expiration : 0;

	if (expiration_time && now > expiration_time)  {
		audit_auth_entry(pr->ufld.fd_name, OT_PRPWD,
                        MSGSTR_SEC(LOGIN_SEC_8, "password expired"), 2 /* ET_LOGIN */);
		show_mesg(MSGSTR_SEC(LOGIN_SEC_9, "Your password has expired.\n"));

		if(!prtc) return (struct pr_passwd *) 0;	/* Fail if no tty -DAL */

		/*
		 * We must close and re-open the Protected Passwd database here
		 * because the passwd program should have updated
		 * the user's entry.  Since pr now points to a static area, we
		 * need to use save_data to hold the data while written.
		 */
		save_data = *pr;
		endprpwent();
		passwd_status = execute(PASSWD_PROGRAM, pr->ufld.fd_uid,
					pr->ufld.fd_name);
		setprpwent();
		pr = getprpwnam(save_data.ufld.fd_name);
		current_prpwd = pr;
		if (pr == (struct pr_passwd *) 0)  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_2,
			"protected password entry vanished after write"));
			show_error(MSGSTR_SEC(LOGIN_SEC_3,
			"Protected Password information suddenly vanished\n"));
			failure_override(&save_data, prtc);
			return (struct pr_passwd *) 0; /* Don't exit(1); -DAL */
		}

		if (passwd_status < 0) {
			audit_event(1, MSGSTR_SEC(LOGIN_SEC_4,
				"cannot execute passwd program"));
			show_error(MSGSTR_SEC(LOGIN_SEC_5,
				"Cannot execute passwd program\n"));
			failure_override(&save_data, prtc);
			cancel_process(1, pr, prtc);
		}
		else if (passwd_status > 0) {
			audit_event(1, MSGSTR_SEC(LOGIN_SEC_6,
				"unsuccessful password change"));
			show_error(MSGSTR_SEC(LOGIN_SEC_10,
				"Login aborted due to no new password.\n"));
			failure_override(&save_data, prtc);
			cancel_process(1, pr, prtc);
		}
	} else if (df && df->sflg.fg_pw_expire_warning && expiration_time &&
		   expiration_time - now <= df->sfld.fd_pw_expire_warning) {
		show_mesg(MSGSTR_SEC(LOGIN_SEC_11, "\nYour password will expire on %s\n"),
			ctime(&expiration_time));
	}

	return pr;
}


/*
 * This is the point of no return for detection of a bad login and the
 * leaving of the login program.  We need to delay in case someone tries
 * to relog into a dedicated line, where there is no hardware hangup to
 * cause an inconvenience delay.
 */
static void
cancel_process(exit_code, pr, prtc)
	int exit_code;
	struct pr_passwd *pr;
	register struct pr_term *prtc;
{

/*
 *
 *	Turn off the alarm(2) so that we don't end up calling timeout()
 *	while we are cleaning up.  This isn't a problem if we were
 *	called from timeout().
 *
 */

	alarm(0) ;

#if defined(TPLOGIN) && SEC_CHOTS
	if(twoperson_check) {
		pr   = (struct pr_passwd *) 0;
		prtc = (struct pr_term *) 0;
	}
#endif
	if (pr != (struct pr_passwd *) 0)
		(void) login_bad_user(pr, prtc);
	if (prtc != (struct pr_term *) 0)
		(void) login_bad_tty(prtc, pr);
	else if (pr != (struct pr_passwd *) 0)
		impossible_user(&pr, pwd_ref);
	if ((pr != (struct pr_passwd *) 0) || (prtc != (struct pr_term *) 0))
		login_delay("exit");
	reset_tty_discr_and_bye(exit_code);
}


/*
 * Perform actions upon recognition of a bad login attempt.  For bad
 * accounts, do nothing.  For good accounts, update the database
 * entry for the account with the time of attempt and that this is
 * another bad attempt.
 */
struct pr_passwd *
login_bad_user(pr, prtc)
	struct pr_passwd *pr;
	struct pr_term *prtc;
{
	struct pr_passwd save_data;
	int	save_alarm ;

#if defined(TPLOGIN) && SEC_CHOTS
	if(twoperson_check) {
		current_prpwd = (struct pr_passwd *) 0;
		return(pr);
	}
#endif

	/*
	 * If this login name really doesn't exist, don't even try
	 * to update the Protected Password database.  The less time taken
	 * here than for good accounts that really are updated is
	 * obscured by sleeping login_delay().
	 * Also, reset static current_prpwd in case we get an
	 * alarm signal. For user names that have been set to NO_ENTRY,
	 * restore the original name used at login for auditing the
	 * failed attempt. Then restore NO_ENTRY upon return.
	 */
	if (strcmp(pr->ufld.fd_name, NO_ENTRY) == 0)  {
		strncpy(pr->ufld.fd_name,bad_username,8);

		/*
		 * Audit an unsuccessful login attempt.
		 */
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_13,
			 "login name doesn't exist"));
		strcpy(pr->ufld.fd_name,NO_ENTRY);
		current_prpwd = (struct pr_passwd *) 0;
		return pr;
	}


	/*
	 * We need the save area to avoid clashing with the static area when
	 * putprpwnam updates the file.  Then, we need to re-establish a new
	 * pr pointer based on what the new entry looks like, later on in this
	 * routine.
	 */
	save_data = *pr;
	pr = &save_data;
	pr->uflg.fg_ulogin = 1;
	pr->ufld.fd_ulogin = time((time_t *) 0);
	if (pr->uflg.fg_nlogins)
		pr->ufld.fd_nlogins++;
	else  {
		pr->uflg.fg_nlogins = 1;
		pr->ufld.fd_nlogins = 1;
	}
	pr->uflg.fg_unsuctty = 1;
	strncpy(pr->ufld.fd_unsuctty, prtc->ufld.fd_devname,
	  sizeof(prtc->ufld.fd_devname));

	/*
	 * Need to erase this in case we get an alarm signal for
	 * too much time.
	 */
	current_prpwd = (struct pr_passwd *) 0;

/*
 *
 *	Disable the alarm(2) so that it does not go off while we are
 *	updating the base.  We will reset the alarm(2) if the update
 *	succeeds.  Ignore the amount of time we took doing the update.
 *
 */

	save_alarm = alarm(0) ;

	if (!putprpwnam(pr->ufld.fd_name, pr))  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_14,
 				"can't rewrite protected password entry"));
		show_error(MSGSTR_SEC(LOGIN_SEC_15,
		"Can't rewrite protected password entry for user %s\n"),
			 pr->ufld.fd_name);
		login_delay("exit");
		return (struct pr_passwd *) 0; /* Don't exit(1) -DAL */
	}

/*
 *
 *	Reset possible alarm(2)
 *
 */

	alarm(save_alarm) ;

	/*
	 * We must re-open the Protected Password database here
	 * because the file has been updated and putprpwent()
	 * has closed the file.
	 */
	setprpwent();
	pr = getprpwnam(save_data.ufld.fd_name);
	current_prpwd = pr;
	if (pr == (struct pr_passwd *) 0)  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_2,
			"protected password entry vanished after write"));
		show_error(MSGSTR_SEC(LOGIN_SEC_16,
			"Protected Password database problem\n"));
		login_delay("exit");
		return (struct pr_passwd *) 0; /* Don't exit(1) -DAL */
	}

	/*
	 * Audit an unsuccessful login attempt.
	 */
	audit_event(1, MSGSTR_SEC(LOGIN_SEC_43,
		"Last unsuccessful login for %s: %s"));
	return pr;
}


/*
 * Set the login UID.  Before that, make sure that the account is
 * not locked.  Note we do this after the password is obtained to
 * make sure that the real user of the account gets the locked/nolocked
 * status.  Also, let the user know when he last logged in successfully
 * and his last unsuccessful attempt.
 *
 * Locking is done by the Account Administrator unconditionally, by
 * exceeding the number of bad consecutive attempts or by having the
 * password expire.
 *
 * To prevent denial of use of the system by a malicious person who locks
 * the superuser account by repeated attempting logins, we unconditionally
 * allow the superuser account access on the system console.
 */
int
login_set_user(pr, prtc, p)
	register struct pr_passwd *pr;
	register struct pr_term *prtc;
	register struct passwd *p;
{
	register int users_group;
	register int su_on_console;
	time_t last_login;
	time_t last_bad;
	time_t exp_date;
	char *ptime;
	char succ_mesg[512]; /* Buffer for login message */
#if SEC_NET_TTY
	struct hostent *hp;
	struct dev_asg *dvag;
	struct in_addr addr;
#endif

	/*
	 * Allow the superuser to log into the console.  This prevents
	 * someone from locking the superuser account or /dev/console
	 * with repeated failures.  We still audit the lock so the
	 * SSO can detect the problem.
	 */
	su_on_console = ((pr != (struct pr_passwd *) 0) &&
			 pr->uflg.fg_uid &&
			 (pr->ufld.fd_uid == 0) &&
			 (prtc != (struct pr_term *) 0) &&
			 prtc->uflg.fg_devname &&
			 (strcmp(SYSTEM_CONSOLE, prtc->ufld.fd_devname) == 0));

	/*
	 * See if any condition will prevent a superuser login.
	 * Locked_out() will still audit the lock even if the superuser
	 * access is given.
	 */
	if (locked_out(pr))  {
		if (su_on_console)
			show_error(MSGSTR_SEC(LOGIN_SEC_17,
	"Account is disabled but console login is allowed.\n"));
		else  {
			show_error(MSGSTR_SEC(LOGIN_SEC_18,
	"Account is disabled -- see Account Administrator.\n"));
			/* cancel_process(1, pr, prtc); -DAL */
			return 0;	/* Don't cancel_process -DAL */
		}
	}

	/*
	 * Check to see that the user logging in is in the authorized user
	 * list for the terminal. If there is no list for the terminal, let
	 * the login proceed. If there is a list, only allow login if the
	 * user name is in the list.
	 */
	if(prtc)	/* Check only if using tty -DAL */
	if (!auth_for_terminal(pr,prtc))  {
		if (su_on_console)
			show_error(MSGSTR_SEC(LOGIN_SEC_19,
	"Not authorized for terminal access but console login is allowed.\n"));
		else  {
			audit_event(1, MSGSTR_SEC(LOGIN_SEC_22,
				"user not authorized for terminal/host"));
#if SEC_NET_TTY
                        if (is_network)
                                show_error(MSGSTR_SEC(LOGIN_SEC_20,
"Not authorized for access from your host -- see System Administrator.\n"));
			else  
#endif
			show_error(MSGSTR_SEC(LOGIN_SEC_21,
	"Not authorized for terminal access -- see System Administrator.\n"));
			/* cancel_process(1, pr, prtc); -DAL */
			return 0;	/* Don't cancel process -DAL */
		}
	}

#if !SEC_PRIV /*{*/
	/* if root, check for secure terminal */

	if (prtc && !(pr->uflg.fg_uid && pr->ufld.fd_uid)) {
		struct ttyent *ttep;
		struct stat statbuf;
		char s[PATH_MAX];

		setttyent();
		if(prtc->ufld.fd_devname[0] != '/')
			strcpy(s, "/dev/");
		else
			strcpy(s, "");
		strcat(s, prtc->ufld.fd_devname);
		if(strlen(s) == 10 && !strncmp(s, "/dev/tty", 8) &&
		   ((s[8] >= 'a' && s [8] <= 'z' && s[8] != 'd') ||
		   (s[8] >= 'A' && s[8] <= 'Z')))
			ttep = getttynam("ptys");
		else
			if(stat(s, &statbuf) < 0)
				ttep = getttynam(prtc->ufld.fd_devname);
			else
				ttep = getttynam(s);
		endttyent();
		if (!ttep || !(ttep->ty_status & TTY_SECURE)) {
			audit_event(1, MSGSTR_SEC(LOGIN_SEC_22,
				"user not authorized for terminal/host"));
			syslog(LOG_WARNING|LOG_AUTH,
			    MSGSTR_SEC(ROOT_REFUSED, "ROOT LOGIN REFUSED %s"),
				s);
			show_error(MSGSTR_SEC(LOGIN_SEC_21,
	"Not authorized for terminal access -- see System Administrator.\n"));
			return 0;
		}
	}
#endif /*} !SEC_PRIV */

	/* check for template account */

	if (pr->uflg.fg_istemplate && pr->ufld.fd_istemplate) {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_125,
				"template account login attempt"));
		show_error(MSGSTR_SEC(LOGIN_SEC_124,
	"Template account cannot log in\n"));
		return 0;
	}

	/* check for auto-retired account */

	if (pr->uflg.fg_expdate)
		exp_date = pr->ufld.fd_expdate;
	else if (pr->sflg.fg_expdate)
		exp_date = pr->sfld.fd_expdate;
	else
		exp_date = (time_t) 0;
	
	if (exp_date && exp_date <= time((time_t *)0)) {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_126,
				"account has expired"));
		show_error(MSGSTR_SEC(LOGIN_SEC_127,
	"Account has expired -- please see system administrator.\n"));
		return 0;
	}

	/* check for retired account */

	if (pr->uflg.fg_retired && pr->ufld.fd_retired) {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_23,
				 "account has been retired"));
		show_error(MSGSTR_SEC(LOGIN_SEC_24,
	"Account has been retired -- logins are no longer allowed.\n"));
		/* cancel_process(1, pr, prtc); -DAL */
		return 0;	/* Don't cancel process -DAL */
	}

	/*
	 * See if this is a valid time period for the user to log in.
	 * Time_lock() will still audit the lock even if the superuser
	 * access is given.
	 */
	if (time_lock(pr))  {
		if (su_on_console)
			show_error(MSGSTR_SEC(LOGIN_SEC_25,
			"Wrong time period but console login is allowed.\n"));
		else  {
			show_error(MSGSTR_SEC(LOGIN_SEC_26,
			"Wrong time period to log into this account.\n"));
			/* cancel_process(1, pr, prtc); -DAL */
			return 0;	/* Don't cancel process -DAL */
		}
	}

#ifndef TPLOGIN /*{*/
	/*
	 * Set the ownership of the terminal.  Under most conditions, we
	 * set the group to the special terminal group.  If that group
	 * does not exist, we set the group to the user's login group.
	 */
	if (term_group == -1)
		users_group = p->pw_gid;
	else
		users_group = term_group;
	if(prtc && save_tty_isatty)	/* Check for no tty -DAL */
	if (save_tty_path == (char *) 0 ||
	    chown(save_tty_path, pr->ufld.fd_uid, users_group) == -1 ||
	    chmod(save_tty_path, TTY_PERMISSION) == -1)  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_27,
				"cannot set terminal mode"));
		show_error(MSGSTR_SEC(LOGIN_SEC_28,
			"Cannot set terminal mode.\n"));
		/* cancel_process(1, pr, prtc); -DAL */
		return 0; 	/* Don't cancel process -DAL */
	}
#endif /*} !TPLOGIN */

	if (!pr->uflg.fg_uid ||
	    (setluid(pr->ufld.fd_uid) != 0))  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_29,
				"bad login user id"));
		show_error(MSGSTR_SEC(LOGIN_SEC_30, "Bad login user id.\n"));
		/* cancel_process(1, pr, prtc); -DAL */
		return 0;	 /* Don't cancel process -DAL */
	}

#ifndef TPLOGIN /*{*/
#if SEC_MAC
	if (!get_sec_level(pr, prtc))  {
		/*
		 * Don't distinguish between a level not dominated by the user's
		 * clearance and an invalid level, so that the user cannot
		 * determine names of security levels that he may not be
		 * allowed to know of.
		 */
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_31,
				"bad session security level"));
		show_error(MSGSTR_SEC(LOGIN_SEC_32,
			"Bad clearance or session security level.\n"));

#if SEC_ENCODINGS
		if (mand_syslo == (mand_ir_t *) 0)
#else
		if ((mand_syslo == (mand_ir_t *) 0) || (!mand_syndb_valid()))
#endif
			failure_override(pr, prtc);
		cancel_process(1, pr, prtc);
	}
#endif /* SEC_MAC */

	/*
	 * Regrade the terminal device for MAC and caveat sets.
	 */
	if(prtc && save_tty_isatty)	/* Check if tty in use -DAL */
	if (!terminal_regrade(prtc,
#if SEC_MAC
		sec_level_ir,
#else
		0,
#endif
		0
		))
	{
#if SEC_NET_TTY
		if (is_network)
                        show_error(MSGSTR_SEC(LOGIN_SEC_35,
				"Can't set host security level.\n"));
                else
#endif
		show_error(MSGSTR_SEC(LOGIN_SEC_36,
			"Can't set terminal security level.\n"));
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_37,
				"can't set terminal/host security level"));
		failure_override(pr, prtc);
		cancel_process(1, pr, prtc);
	}
#endif /*} !TPLOGIN */

#if SEC_WIS
	/*
	 * Make this process a session leader
	 */
	setsession(getpid());
#endif

	/*
	 * Print the last times logins were attempted to this account.
	 */
	if(prtc) { /* -DAL */
	if (pr->uflg.fg_ulogin)
		last_bad = pr->ufld.fd_ulogin;
	else
		last_bad = (time_t) 0;

	if (pr->uflg.fg_slogin)
		last_login = pr->ufld.fd_slogin;
	else
		last_login = (time_t) 0;

	if (last_login == 0)
		ptime = MSGSTR_SEC(LOGIN_SEC_38, "NEVER");
	else {
		ptime = ctime(&last_login);
		ptime[strlen(ptime)-1] = '\0'; /* strip newline */
	}
	sprintf(succ_mesg, MSGSTR_SEC(LOGIN_SEC_39, "Last   successful login for %s: %s"),
			pr->ufld.fd_name, ptime);
	if (pr->uflg.fg_suctty) {
#if SEC_NET_TTY
            dvag = getdvagnam(pr->ufld.fd_suctty);
            if (dvag != (struct dev_asg *) 0 &&
		    ISBITSET (dvag->ufld.fd_type, AUTH_DEV_REMOTE)) {
		int	byte1, byte2, byte3, byte4;
		char	host_address[32];

		sscanf(pr->ufld.fd_suctty, "%2x%2x%2x%2x", &byte1, &byte2,
							   &byte3, &byte4);
		sprintf(host_address, "%d.%d.%d.%d", byte1, byte2,
						     byte3, byte4);
		addr.s_addr = inet_addr(host_address);
                hp = gethostbyaddr(&addr.s_addr, 4, AF_INET);
		if (hp == (struct hostent *) 0)
			strcat(succ_mesg, MSGSTR_SEC(LOGIN_SEC_40,
				" from (unknown)"));
		else
			sprintf(&succ_mesg[strlen(succ_mesg)], MSGSTR_SEC(LOGIN_SEC_41,
				" from %s"), hp->h_name);
		endhostent();
            }
            else
#endif /* SEC_NET_TTY */
		sprintf(&succ_mesg[strlen(succ_mesg)], MSGSTR_SEC(LOGIN_SEC_42,
			" on %s"), pr->ufld.fd_suctty);
        }
	strcat(succ_mesg, "\n");
/*
	show_mesg(succ_mesg);
*/
	if (last_bad == 0)
		ptime = MSGSTR_SEC(LOGIN_SEC_38, "NEVER");
	else {
		ptime = ctime(&last_bad);
		ptime[strlen(ptime)-1] = '\0'; /* strip newline */
	}
	sprintf(&succ_mesg[strlen(succ_mesg)], MSGSTR_SEC(LOGIN_SEC_43,
		"Last unsuccessful login for %s: %s"),
		pr->ufld.fd_name, ptime);
	if (pr->uflg.fg_unsuctty) {
#if SEC_NET_TTY
            dvag = getdvagnam(pr->ufld.fd_unsuctty);
            if (dvag != (struct dev_asg *) 0 &&
		    ISBITSET (dvag->ufld.fd_type, AUTH_DEV_REMOTE)) {
		int	byte1, byte2, byte3, byte4;
		char	host_address[32];

		sscanf(pr->ufld.fd_unsuctty, "%2x%2x%2x%2x", &byte1, &byte2,
							     &byte3, &byte4);
		sprintf(host_address, "%d.%d.%d.%d", byte1, byte2,
						     byte3, byte4);
		addr.s_addr = inet_addr(host_address);
                hp = gethostbyaddr(&addr.s_addr, 4, AF_INET);
		if (hp == (struct hostent *) 0)
			strcat(succ_mesg, MSGSTR_SEC(LOGIN_SEC_40,
				" from (unknown)"));
		else
			sprintf(&succ_mesg[strlen(succ_mesg)], MSGSTR_SEC(LOGIN_SEC_41,
				" from %s"), hp->h_name);
		endhostent();
            }
            else
#endif /* SEC_NET_TTY */
		sprintf(&succ_mesg[strlen(succ_mesg)], MSGSTR_SEC(LOGIN_SEC_42,
			" on %s"), pr->ufld.fd_unsuctty);
        }
	strcat(succ_mesg, "\n");
	show_mesg(succ_mesg);
	} /* -DAL */
	return 1;
}

/*
 * Delay for the number of seconds specified in the terminal control
 * entry for this terminal.  So the user still knows that the system
 * is responsive, print out a wait message (using reason string
 * supplied) and print a dot for each second waited.
 */
void
login_delay(reason)
	char *reason;
{
	int seconds;

	if (delay_between_attempts == 0)
		return;
	show_mesg(MSGSTR_SEC(LOGIN_SEC_44, "\nWait for login %s ...\n"), reason);
	for (seconds = 0; seconds < delay_between_attempts; seconds++)  {
		sleep(1);
/* DAL
		show_mesg(".");
*/
	}
/* DAL
	show_mesg("\n");
*/
}


/*
 * Find the protected password entry for the user and fill in the argument
 * ppr with it.  If the user cannot be found fill in a bogus entry to
 * use that will 1) allow login to still prompt for a password in the
 * usual way, and 2) not match ANY possible user name.  Fill in the ppwd
 * with the passwd equivalent of "no user" for the same reason.
 * We consider it the case that the user cannot be found if the passwd
 * entry and pr_passwd do not fully reflect the same user.
 */
int
login_fillin_user(user, ppr, ppwd)
	char *user;
	struct pr_passwd **ppr;
	struct passwd **ppwd;
{
	/*
	 * Save this so that later in login_bad_user(), the impossible_user()
	 * routine can be called with the proper pwd argument.
	 */
	pwd_ref = ppwd;

	*ppr = getprpwnam(user);

	/*
	 * Restore the passwd pointer that might have been destroyed
	 * by getprpwnam.
	 */

	*ppwd = getpwnam(user);
	if (*ppr == (struct pr_passwd *) 0) {
		/*
		 * if the root account has disappeared from the protected
		 * password database, allow the login anyway.  The check
		 * in login_validate() will continue the process.
		 */
		if (strcmp(user, OVERRIDE_USER) == 0)
			return 1; /* return positive value -DAL */
		strncpy(bad_username,user,8);	/* save for auditing */
		impossible_user(ppr, ppwd);
	}
	else if (!(*ppr)->uflg.fg_name || !(*ppr)->uflg.fg_uid ||
		 (*ppwd && ((*ppr)->ufld.fd_uid != (*ppwd)->pw_uid) ||
		 (strcmp((*ppr)->ufld.fd_name, (*ppwd)->pw_name) != 0)))  {
		audit_auth_entry((*ppwd)->pw_name, OT_PRPWD,
			MSGSTR_SEC(LOGIN_SEC_47,
"user appears in /etc/passwd but not in Protected Password database"),
			2 /* ET_LOGIN */);
		impossible_user(ppr, ppwd);
	}
	else current_prpwd = *ppr;
	return 1; /* return positive value -DAL */
}


int
login_validate(pprpwd, pprtc, pnopassword, password)
	register struct pr_passwd **pprpwd;
	register struct pr_term **pprtc;
	int *pnopassword;
	char *password;
/* Changed to pass in clear text password. -DAL */
{
	register int is_valid_password = 1;
	register int crypt_index = AUTH_CRYPT_BIGCRYPT;

#ifdef _OSF_SOURCE
	ioctl(0, TIOCSCTTY, 0);
#endif
	/*
	 * if there is a problem with the protected password database,
	 * allow a root account login at the console.  If there is a valid
	 * protected password entry, force the password check on the root
	 * account.  Otherwise, an invalid terminal control entry means
	 * that the system should reject the login.
	 */
	if (*pprpwd == (struct pr_passwd *) 0) {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_48,
			"no entry in protected password database"));
		failure_override(*pprpwd, *pprtc);
	}
	if (*pprtc == (struct pr_term *) 0 &&
	    strcmp((*pprpwd)->ufld.fd_name, OVERRIDE_USER)) {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_49,
			"no entry for terminal in terminal control database"));
		return 0; /* Don't exit(1) - DAL */
	}

	if ((*pprpwd)->uflg.fg_pwchanger) {
		char *pwchanger, buf[16];

		pwchanger = pw_idtoname((*pprpwd)->ufld.fd_pwchanger);
		if (pwchanger == NULL) {
			sprintf(buf, MSGSTR_SEC(LOGIN_SEC_50,
				"uid #%u"), (*pprpwd)->ufld.fd_pwchanger);
			pwchanger = buf;
		}
		show_mesg(MSGSTR_SEC(LOGIN_SEC_51,
			"Your password was changed by %s on %s"), pwchanger,
			ctime(&(*pprpwd)->ufld.fd_schange));
	}

	if ((*pprpwd)->uflg.fg_oldcrypt)
		crypt_index = (*pprpwd)->ufld.fd_oldcrypt;
	else if ((*pprpwd)->sflg.fg_oldcrypt)
		crypt_index = (*pprpwd)->sfld.fd_oldcrypt;

	if ((*pprpwd)->ufld.fd_encrypt[0] == '\0')
		check_valid_tty(*pprtc, *pprpwd);
	else  {
		if(pnopassword != (int *) 0)
			*pnopassword = 0;

		if(!password && strcmp(login_crypt(MSGSTR_SEC(LOGIN_SEC_52, "Password:"),
				(*pprpwd)->ufld.fd_encrypt, crypt_index),
				(*pprpwd)->ufld.fd_encrypt) == 0)
			check_valid_tty(*pprtc, *pprpwd);
		else if(strcmp(dispcrypt(password, (*pprpwd)->ufld.fd_encrypt,
					 crypt_index),
			       (*pprpwd)->ufld.fd_encrypt) == 0)
			check_valid_tty(*pprtc, *pprpwd);
		else  {
			if (*pprpwd && *pprtc) {
				*pprpwd = login_bad_user(*pprpwd, *pprtc);
				*pprtc = login_bad_tty(*pprtc, *pprpwd);
			}
			show_mesg(MSGSTR_SEC(LOGIN_SEC_53, "Login incorrect\n"));
			audit_event(1, MSGSTR_SEC(LOGIN_SEC_54,
				"login incorrect"));
			login_delay(MSGSTR_SEC(LOGIN_SEC_55, "retry"));
			is_valid_password = 0;
		}
	}

	return is_valid_password;
}


/*
 * Set the process file creation mask to be restrictive.  If the user
 * wants it more lax, he has to open it up explicitly.  Also catch
 * login timeouts as failed login attempts.
 */
int
login_set_sys()
{
	/*
	 * Make sure the luid is not already set. Since it is conceivable
	 * that -1 could be used as a user id, we must rely on errno to
	 * detect failure of getluid.
	 */
	errno = 0;
	(void) getluid();
	if ((errno == 0) && security_is_on()) {
		show_mesg("You are already logged in.\nPlease log out first.\n");
		return 0; /* Don't exit(1) -DAL */
	}
	umask(~SEC_DEFAULT_MODE);
	signal(SIGALRM, timeout);
	return 1;
}


/*
 * Find the terminal in the terminal control database.  Then, use the
 * information in the entry to set the alarm clock for login timeouts.
 * We make sure that stdin, stdout, and stderr are character special
 * files and that they point to the same device.
 * Also save the terminal settings in case we timeout later (so we can
 * restore any funny modes that we may be in due to getpasswd(3) settings).
 */
struct pr_term *
login_term_params(tty_path, tty_name)
	char *tty_path;
	char *tty_name;
{
	register struct group *g;
	register struct pr_term *pr;
	struct stat in_buf;
	struct stat out_buf;
	struct stat err_buf;
	char * envstr;

	save_tty_path = tty_path;
	save_tty_isatty = 1;

	pr = getprtcnam(tty_name);

	/*
	 * if terminal control database yields a failure, check the device
	 * assignment database for a synonym device.
	 */
	if (pr == (struct pr_term *) 0)
		pr = check_devasg(tty_path);

	if (pr == (struct pr_term *) 0)  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_58,
			"current terminal not in terminal control database"));
		show_error(MSGSTR_SEC(LOGIN_SEC_59,
		"Cannot obtain database information on this terminal\n"));
		/*
		 * if there is no entry for the console, the database is
		 * corrupted and a root login should be allowed
		 */
		if (strcmp(tty_name, SYSTEM_CONSOLE) == 0)
			return((struct pr_term *) 0);
		return (struct pr_term *) 0; /* Don't exit(1) -DAL */
	}

	g = getgrnam("terminal");
	if (!g)
		g = getgrnam("tty");
	if (g != (struct group *) 0)
		term_group = g->gr_gid;
	endgrent();

	if (pr->uflg.fg_logdelay)
		delay_between_attempts = pr->ufld.fd_logdelay;
	else if (pr->sflg.fg_logdelay)
		delay_between_attempts = pr->sfld.fd_logdelay;
	else
		delay_between_attempts = 0;

	/*
	 * Make sure stdin, stdout, and stderr are terminals and they
	 * are one and the same.
	 */
	if (pr->uflg.fg_xdisp && pr->ufld.fd_xdisp)
		save_tty_isatty = 0;
	else
	if ((fstat(fileno(stdin), &in_buf) != 0) ||
	    (fstat(fileno(stdout), &out_buf) != 0) ||
	    (fstat(fileno(stderr), &err_buf) != 0) ||
	    ((in_buf.st_mode & S_IFMT) != S_IFCHR) ||
	    ((out_buf.st_mode & S_IFMT) != S_IFCHR) ||
	    ((err_buf.st_mode & S_IFMT) != S_IFCHR) ||
	    (in_buf.st_rdev != out_buf.st_rdev) ||
	    (out_buf.st_rdev != err_buf.st_rdev))  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_61,
			 "stdin/out/err are not the same"));
		show_error(MSGSTR_SEC(LOGIN_SEC_62,
			"Error in terminal setup.\n"));
		return (struct pr_term *) 0; /* Don't exit(1) -DAL */
	}

	/*
	 * Get the tty settings in case we need to restore them later.
	 * If the ioctl fails, we don't have a terminal to play with where
	 * it should be.  We do this before the alarm is activated so we
	 * are sure to read the information here before the alarm clock
	 * will cause it to be used in timeout().
	 */
	if (save_tty_isatty)
#ifdef BSD_TTY
	if (ioctl(fileno(stdin), TIOCGETP, &initial_settings) != 0)
#else
	if (ioctl(fileno(stdin), TCGETA, &initial_settings) != 0)
#endif
	{
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_63,
				"could not get terminal settings"));
		show_error(MSGSTR_SEC(LOGIN_SEC_64,
			"Cannot obtain settings for this terminal\n"));
		return (struct pr_term *) 0; /* Don't exit(1) -DAL */
	}

	/*
	 * Use the login timeout value from the terminal control dbase
	 * entry if there is one defined. Otherwise, use the following.
	 * Base alarm on rate of login attempts.  Add TYPEIN_TIME seconds
	 * on each attempt to account for some of the overhead in typing in
	 * password.
	 */

	if(pr->uflg.fg_login_timeout)
	    (void) alarm(pr->ufld.fd_login_timeout);
	else if(pr->sflg.fg_login_timeout)
	    (void) alarm(pr->sfld.fd_login_timeout);
	else
	    (void) alarm(ATTEMPTS * (TYPEIN_TIME + delay_between_attempts));

	current_prtc = pr;
	return pr;
}

#if SEC_NET_TTY /*{*/
struct pr_term *
login_net_term_params(tty_path, hostname)
        char *tty_path;
        char *hostname;
{
        register struct group *g;
        register struct pr_term *pr;
        struct stat in_buf;
        struct stat out_buf;
        struct stat err_buf;

        struct hostent *hp = gethostbyname(hostname);
        char host_entry[14];
        int i;
        char *addr;


        if (hp == NULL) {
		audit_auth_entry(hostname, 0, MSGSTR_SEC(LOGIN_SEC_66,
			"current host is not in \'/etc/hosts\'"), 2 /* ET_LOGIN */);
                show_error(MSGSTR_SEC(LOGIN_SEC_67,
			"No entry for this hosts in \'/etc/hosts\'.\n"));
		return (struct pr_term *) 0; /* Don't exit(1) -DAL */
        }
        addr = hp->h_addr_list[0];
        for (i = 0; i < hp->h_length; i++)
                sprintf (&host_entry[i*2], "%2.2x", addr[i] & 0xff);
        strcat(host_entry, "0000");

        save_tty_path = tty_path;
	save_tty_isatty = 1;

        pr = getprtcnam(host_entry);

        /*
         * if terminal control database yields a failure, check the device
         * assignment database for a synonym device.
         */
        if (pr == (struct pr_term *) 0)
                pr = check_devasg_net(host_entry);

        if (pr == (struct pr_term *) 0)  { 
		audit_auth_entry(host_entry, OT_TERM_CNTL,
		MSGSTR_SEC(LOGIN_SEC_68,
		"current host is not in Terminal Control Database"), 2 /* ET_LOGIN */);
		show_error(MSGSTR_SEC(LOGIN_SEC_69,
			"Cannot obtain database information on this host\n"));
		return (struct pr_term *) 0; /* Don't exit(1) -DAL */
        }

        g = getgrnam("terminal");
        if (g != (struct group *) 0)
                term_group = g->gr_gid;
        endgrent();

        if (pr->uflg.fg_logdelay)
                delay_between_attempts = pr->ufld.fd_logdelay;
        else if (pr->sflg.fg_logdelay)
                delay_between_attempts = pr->sfld.fd_logdelay;
        else
                delay_between_attempts = 0;

	/*
         * Make sure stdin, stdout, and stderr are terminals and they
         * are one and the same.
         */
        if ((fstat(fileno(stdin), &in_buf) != 0) ||
            (fstat(fileno(stdout), &out_buf) != 0) ||
            (fstat(fileno(stderr), &err_buf) != 0) ||
            ((in_buf.st_mode & S_IFMT) != S_IFCHR) ||
            ((out_buf.st_mode & S_IFMT) != S_IFCHR) ||
            ((err_buf.st_mode & S_IFMT) != S_IFCHR) ||
            (in_buf.st_rdev != out_buf.st_rdev) ||
            (out_buf.st_rdev != err_buf.st_rdev))  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_61,
				 "stdin/out/err are not the same"));
		show_error(MSGSTR_SEC(LOGIN_SEC_62,
			"Error in terminal setup.\n"));
		return (struct pr_term *) 0; /* Don't exit(1) -DAL */
        }
	
        /*
         * Get the tty settings in case we need to restore them later.
         * If the ioctl fails, we don't have a terminal to play with where
         * it should be.  We do this before the alarm is activated so we
         * are sure to read the information here before the alarm clock
         * will cause it to be used in timeout().
         */
#ifdef BSD_TTY
        if (ioctl(fileno(stdin), TIOCGETP, &initial_settings) != 0)
#else
        if (ioctl(fileno(stdin), TCGETA, &initial_settings) != 0)
#endif
	{
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_63,
				"could not get terminal settings"));
		show_error(MSGSTR_SEC(LOGIN_SEC_64,
			"Cannot obtain settings for this terminal\n"));
		return (struct pr_term *) 0; /* Don't exit(1) -DAL */
        }

        /*
         * Use the login timeout value from the terminal control dbase
         * entry if there is one defined. Otherwise, use the following.
         * Base alarm on rate of login attempts.  Add TYPEIN_TIME seconds
         * on each attempt to account for some of the overhead in typing in
         * password.
         */

        if(pr->uflg.fg_login_timeout)
            (void) alarm(pr->ufld.fd_login_timeout);
        else if(pr->sflg.fg_login_timeout)
            (void) alarm(pr->sfld.fd_login_timeout);
        else
            (void) alarm(ATTEMPTS * (TYPEIN_TIME + delay_between_attempts));

        current_prtc = pr;
        is_network = 1;
        return pr;
}
#endif /*} SEC_NET_TTY */

#ifndef DEC
/*
 * For a sublogin, re-execute the login command with a special
 * environment indicating a sublogin.
 */
void
login_do_sublogin(envinit)
	char *envinit[];
{
	execle(LOGIN_PROGRAM, "login", (char *) 0, &envinit[0]);
	show_mesg(MSGSTR_SEC(LOGIN_SEC_70, "No login program on root\n"));
}
#endif /* !DEC */


/*
 * This routine is called on each bad login attempt.  The number of consecutive
 * bad attempts is set or incremented, the user (if he exists in the system) is
 * noted along with the time, and this information is written as the new
 * entry for this terminal in the terminal control database.
 */
struct pr_term *
login_bad_tty(pr, prpw)
	struct pr_term *pr;
	struct pr_passwd *prpw;
{
	struct pr_term save_data;
	int	save_alarm ;

#if defined(TPLOGIN) && SEC_CHOTS
	if(twoperson_check) {
		current_prtc = (struct pr_term *) 0;
		return(pr);
	}
#endif

	if (timeout_occurred) {
		if (prpw != (struct pr_passwd *) 0)
			impossible_user(&prpw, pwd_ref) ;
		return(pr) ;
	}

	/*
	 * pr until now has pointed to a static area that will get
	 * munged upon the file rewrite.  We have it point to a local
	 * save area until the write is finished.
	 */
	save_data = *pr;
	pr = &save_data;

	pr->uflg.fg_ulogin = 1;
	pr->ufld.fd_ulogin = time((time_t *) 0);

	if (pr->uflg.fg_nlogins)
		pr->ufld.fd_nlogins++;
	else  {
		pr->uflg.fg_nlogins = 1;
		pr->ufld.fd_nlogins = 1;
	}

	if ((prpw == (struct pr_passwd *) 0) || !prpw->uflg.fg_uid)
		pr->uflg.fg_uuid = 0;
	else  {
		pr->uflg.fg_uuid = 1;
		pr->ufld.fd_uuid = prpw->ufld.fd_uid;
	}

	/*
	 * Need to erase this in case we get an alarm signal for
	 * too much time.
	 */
	current_prtc = (struct pr_term *) 0;


/*
 *
 *	Disable the alarm(2) so that it does not go off while we are
 *	updating the base.  We will reset the alarm(2) if the update
 *	succeeds.  Ignore the amount of time we took doing the update.
 *
 */

	save_alarm = alarm(0) ;

	if (!putprtcnam(pr->ufld.fd_devname, pr))  {
		show_error(MSGSTR_SEC(LOGIN_SEC_71,
			"Can't rewrite terminal control entry for %s\n"),
			 pr->ufld.fd_devname);
		login_delay("exit");
		return (struct pr_term *) 0; /* Don't exit(1) -DAL */
	}


/*
 *
 *	Reset possible alarm(2)
 *
 */

	alarm(save_alarm) ;

	/*
	 * We must re-open the Terminal Control database here like above
	 * because the file has been updated and putprtcent()
	 * has closed the file.
	 */
	setprtcent();
	pr = getprtcnam(save_data.ufld.fd_devname);
	current_prtc = pr;
	if (pr == (struct pr_term *) 0)  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_72,
			"terminal control information vanished after write"));
		show_error(
MSGSTR_SEC(LOGIN_SEC_73, "Terminal Control information suddenly vanished\n"));
		login_delay("exit");
		return (struct pr_term *) 0; /* Don't exit(1) -DAL */
	}

	/*
	 * After a login attempt, reset the system's idea of the user
	 * so the previous user will not get nailed for future actions
	 * like timeout or forced login exit.
	 */
	if (prpw != (struct pr_passwd *) 0)
		impossible_user(&prpw, pwd_ref);

	return pr;
}


/*
 * This routine is called when the user has been authenticated and will
 * login this time.  The terminal in the terminal control database
 * reflects that a valid user logged in and any unsuccessful attempt count
 * is eliminated.
 */
struct pr_term *
login_good_tty(pr, prpw)
	struct pr_term *pr;
	struct pr_passwd *prpw;
{
	struct pr_term save_data;
	int	save_alarm ;

	/*
	 * pr until now has pointed to a static area that will get
	 * munged upon the file rewrite.  We have it point to a local
	 * save area until the write is finished.
	 */
	save_data = *pr;
	pr = &save_data;

	pr->uflg.fg_slogin = 1;
	pr->ufld.fd_slogin = time((time_t *) 0);

	pr->uflg.fg_nlogins = 0;

	if ((prpw == (struct pr_passwd *) 0) || !prpw->uflg.fg_uid)
		pr->uflg.fg_uid = 0;
	else  {
		pr->uflg.fg_uid = 1;
		pr->ufld.fd_uid = prpw->ufld.fd_uid;
	}

	/*
	 * Need to erase this in case we get an alarm signal for
	 * too much time.
	 */
	current_prtc = (struct pr_term *) 0;


/*
 *
 *	Disable the alarm(2) so that it does not go off while we are
 *	updating the base.  We will reset the alarm(2) if the update
 *	succeeds.  Ignore the amount of time we took doing the update.
 *
 */

	save_alarm = alarm(0) ;

	if (!putprtcnam(pr->ufld.fd_devname, pr))  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_74,
			"can't rewrite updated terminal control entry"));
		show_error(MSGSTR_SEC(LOGIN_SEC_71,
			"Can't rewrite terminal control entry for %s\n"),
			 pr->ufld.fd_devname);
		failure_override(prpw, pr);
		login_delay("exit");
		return (struct pr_term *) 0; /* Don't exit(1) -DAL */
	}


/*
 *
 *	Reset possible alarm(2)
 *
 */

	alarm(save_alarm) ;

	/*
	 * We must re-open the Terminal Control database here like
	 * above because the file has been updated and putprtcent()
	 * has closed the file.
	 */
	setprtcent();
	pr = getprtcnam(save_data.ufld.fd_devname);
	current_prtc = pr;
	if (pr == (struct pr_term *) 0)  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_75,
			"terminal control entry vanished after write"));
		show_error(
MSGSTR_SEC(LOGIN_SEC_73, "Terminal Control information suddenly vanished\n"));
		failure_override(prpw, &save_data);
		login_delay("exit");
		return (struct pr_term *) 0; /* Don't exit(1) -DAL */
	}

	return pr;
}


/*
 * This routine is called when the user has been authenticated and will
 * login this time.  The user in the protected password database
 * reflects that a valid user logged in and any unsuccessful attempt count
 * is eliminated.
 */
int
login_good_user(pprpwd, pprtc, pwd)
	register struct pr_passwd **pprpwd;
	register struct pr_term **pprtc;
	register struct passwd *pwd;
{
	register struct pr_passwd *prpwd;
	register struct pr_term *prtc;
	register int new_nice;
	struct pr_passwd save_data;
	int	save_alarm ;
#ifndef TPLOGIN
	register priv_t *sysprivs, *baseprivs;
#endif

	prpwd = *pprpwd;
	prtc = *pprtc;

	/*
	 * We need the save area to avoid clashing with the static area when
	 * putprpwnam updates the file.  Then, we need to re-establish a new
	 * pprpwd pointer based on what the new entry looks like, later on in
	 * this routine.
	 */
	if(prtc) {
	save_data = *prpwd;
	prpwd = &save_data;
	prpwd->uflg.fg_slogin = 1;
	prpwd->ufld.fd_slogin = time((time_t *) 0);
	prpwd->uflg.fg_nlogins = 0;
	prpwd->uflg.fg_suctty = 1;
	prpwd->uflg.fg_pwchanger = 0;
	strncpy(prpwd->ufld.fd_suctty, prtc->ufld.fd_devname,
	  sizeof(prtc->ufld.fd_devname));

	/*
	 * Need to erase this in case we get an alarm signal for
	 * too much time.
	 */
	current_prpwd = (struct pr_passwd *) 0;


/*
 *
 *	Disable the alarm(2) so that it does not go off while we are
 *	updating the base.  We will reset the alarm(2) if the update
 *	succeeds.  Ignore the amount of time we took doing the update.
 *
 */

	save_alarm = alarm(0) ;

	if (!putprpwnam(prpwd->ufld.fd_name, prpwd))  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_76,
			"can't rewrite updated protected password entry"));
		show_error(MSGSTR_SEC(LOGIN_SEC_15,
		"Can't rewrite protected password entry for user %s\n"),
			 prpwd->ufld.fd_name);
		failure_override(prpwd, prtc);
		login_delay("exit");
		return 0; /* Don't exit(1) -DAL */
	}


/*
 *
 *	Reset possible alarm(2)
 *
 */

	alarm(save_alarm) ;

	/*
	 * We must re-open the Protected Password database here like above
	 * because the file has been updated and putprtcent()
	 * has closed the file.
	 */
	setprpwent();
	prpwd = getprpwnam(save_data.ufld.fd_name);
	current_prpwd = prpwd;
	if (prpwd == (struct pr_passwd *) 0)  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_2,
			"protected password entry vanished after write"));
		show_error(
MSGSTR_SEC(LOGIN_SEC_3, "Protected Password information suddenly vanished\n"));
		failure_override(&save_data, prtc);
		login_delay("exit");
		return 0; /* Don't exit(1) -DAL */
	}
	}

	/*
	 * Set the user's special audit parameters.
	 */
#ifdef _OSF_AUDIT_
	audit_adjust_mask(prpwd);
#endif


	/*
	 * Set the priority if necessary.  Since the return value
	 * of nice(2) can normally be -1 from the documentation, and
	 * -1 is the error condition, we key off of errno, not the
	 * return value to find if the change were successful.
	 * Note we must do this before the setuid(2) below.
	 */
	errno = 0;
	if (prpwd->uflg.fg_nice)
		new_nice = prpwd->ufld.fd_nice;
	else if (prpwd->sflg.fg_nice)
		new_nice = prpwd->sfld.fd_nice;

	if (prpwd->uflg.fg_nice || prpwd->sflg.fg_nice)  {
		(void) nice(new_nice);
		if (errno != 0)  {
			audit_event(1, MSGSTR_SEC(LOGIN_SEC_77,
 				"bad priority setting"));
                        show_error(MSGSTR_SEC(LOGIN_SEC_78,
				"Bad priority setting.\n"));
			login_delay("exit");
			return 0; /* Don't exit(1) -DAL */
		}
	}

	/*
	 * We must do the setgid before the setuid because once the setuid
	 * is done, we are no longer the superuser and a setgid to any
	 * group other than login's group would fail.
	 */
	if (setgid(pwd->pw_gid) != 0) {
			audit_event(1, MSGSTR_SEC(LOGIN_SEC_79,
 				"bad group id"));
		show_error(MSGSTR_SEC(LOGIN_SEC_80, "Bad group id.\n"));
		login_delay("exit");
		return 0; /* Don't exit(1) -DAL */
	}
#if SEC_GROUPS
	(void) initgroups(prpwd->ufld.fd_name, pwd->pw_gid);
#endif
/* Set only the effective uid, the application must do a setuid(geteuid()). -DAL */
	if (seteuid(pwd->pw_uid) != 0) {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_81,
 				"bad user id"));
		show_error(MSGSTR_SEC(LOGIN_SEC_82, "Bad user id.\n"));
		login_delay("exit");
		return 0; /* Don't exit(1) -DAL */
	}

#ifndef TPLOGIN /*{*/
	/*
	 * Set the system privilege mask for this process.  Do this after
	 * setting all other identity parameters for the user because
	 * this will turn off self-audit and the audit subsystem will now
	 * expect the login process to be established COMPLETELY for the user.
	 */
	if (prpwd->uflg.fg_sprivs)
		sysprivs = prpwd->ufld.fd_sprivs;
	else if (prpwd->sflg.fg_sprivs)
		sysprivs = prpwd->sfld.fd_sprivs;
	else
		sysprivs = nosprivs;

	/*
	 * Set the base privilege mask for this process.  This mask provides
	 * for those privileges that are always on.
	 */
	if (prpwd->uflg.fg_bprivs)
		baseprivs = prpwd->ufld.fd_bprivs;
	else if (prpwd->sflg.fg_bprivs)
		baseprivs = prpwd->sfld.fd_bprivs;
	else
		baseprivs = nosprivs;

	if (setsysauths(sysprivs) != 0) {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_83,
			"unable to set kernel authorizations"));
		show_error(MSGSTR_SEC(LOGIN_SEC_84,
			"Unable to set kernel authorizations.\n"));
		login_delay("exit");
		return 0; /* Don't exit(1) -DAL */
	}
	if (setbaseprivs(baseprivs) != 0)  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_85,
 				"unable to set base privileges"));
		show_error(MSGSTR_SEC(LOGIN_SEC_86,
			"Unable to set base privileges.\n"));
		login_delay("exit");
		return 0; /* Don't exit(1) -DAL */
	}
#endif /*} !TPLOGIN */
	/*
         * Audit a successful login attempt.
         */
		audit_event(0, MSGSTR_SEC(LOGIN_SEC_123,
 				"successful login"));
	/*
	 * Shutdown completely the use of the Authentication database.
	 */
	end_authentication(pprpwd, pprtc);
	return 1;
}


/*
 * Not only get the encrypted password, but also clear out the cleartext
 * as soon as possible afterwards to prevent a penetration attempt to
 * read the value (from /dev/mem possibly) or from a failing of memory
 * clearing code for new processes.
 */
char *
login_crypt(prompt, seed, alg_index)
	char *prompt;
	char *seed;
	int alg_index;
{
	char *ciphertext;
	char *pass;

	pass = getpasswd(prompt, AUTH_MAX_PASSWD_LENGTH);
	ciphertext = dispcrypt(pass, seed, alg_index);

	/*
	 * This special entry into getpasswd() will clear the cleartext
	 * string without doing anything else.
	 */
	(void) getpasswd((char *) 0, AUTH_MAX_PASSWD_LENGTH);

	return ciphertext;
}


/*
 * This routine is called when SIGALRM goes off due to an expired alarm(2).
 * The alarm is to stop logins after a reasonable time if no correct login
 * is entered.  Here, we let the user know why his connection is about to
 * break and then update the Authentication database.  We reset the terminal
 * mode before we leave so we can leave the terminal in the state we found
 * it, because timeouts in routines like getpasswd() cause annoyances.
 * (This last bit is really only helpful in debugging, but it does not
 * harm normal operation.)
 */
static void
timeout()
{
#ifdef BSD_TTY
	ioctl(fileno(stdin), TIOCSETP, &initial_settings);
#else
	ioctl(fileno(stdin), TCSETAF, &initial_settings);
#endif

	audit_event(1, MSGSTR_SEC(LOGIN_SEC_87,
			"login timed out"));
	show_error(MSGSTR_SEC(LOGIN_SEC_88, "\nLogin timed out\n"));
	timeout_occurred = 1 ;
	cancel_process(1, current_prpwd, current_prtc);
}


/*
 * Check with this entry in the terminal control database to make sure that
 * the terminal is not unconditionally locked or locked because of too many
 * consecutive bad login attempts.  Ignore the lock on the console if the
 * superuser is logging in there.
 */
int
check_valid_tty(pr, prpw)
	register struct pr_term *pr;
	register struct pr_passwd *prpw;
{
	register int locked;
	register int max_tries;
	register int attempts;
	register int su_on_console;
	register time_t re_open, now;

	now = time((time_t *)0);

	/*
	 * in a failure situation, allow root on console to login.
	 * This check allows root password to be checked even if
	 * terminal control database is invalid
	 */
	if (pr == (struct pr_term *) 0 || prpw == (struct pr_passwd *) 0)
/*		failure_override(prpw, pr); -DAL */
		return 0;

	/*
	 * Check for unconditional lock on the terminal.
	 */
	if (pr->uflg.fg_lock)
		locked = pr->ufld.fd_lock;
	else if (pr->sflg.fg_lock)
		locked = pr->sfld.fd_lock;
	else
		locked = 0;

	/*
	 * Check for too many tries on the terminal.
	 */
	if (pr->uflg.fg_max_tries)
		max_tries = pr->ufld.fd_max_tries;
	else if (pr->sflg.fg_max_tries)
		max_tries = pr->sfld.fd_max_tries;
	else
		max_tries = 0;

	if (pr->uflg.fg_unlockint && pr->uflg.fg_ulogin)
		re_open = pr->ufld.fd_unlockint + pr->ufld.fd_ulogin;
	else if (pr->sflg.fg_unlockint && pr->uflg.fg_ulogin)
		re_open = pr->sfld.fd_unlockint + pr->ufld.fd_ulogin;
	else
		re_open = (time_t) 0;

	if (re_open && re_open < now)
		attempts = 0;
	else if (pr->uflg.fg_nlogins)
		attempts = pr->ufld.fd_nlogins;
	else if (pr->sflg.fg_nlogins)
		attempts = pr->sfld.fd_nlogins;
	else
		attempts = 0;

	/*
	 * Allow the superuser to log into the console.  This prevents
	 * someone from locking the superuser account or /dev/console
	 * with repeated failures.  We still audit the lock so the
	 * SSO can detect the problem.
	 */
	su_on_console = ((prpw != (struct pr_passwd *) 0) &&
			 prpw->uflg.fg_uid &&
			 (prpw->ufld.fd_uid == 0) &&
			 pr->uflg.fg_devname &&
			 (strcmp(SYSTEM_CONSOLE, pr->ufld.fd_devname) == 0));

	if (locked || ((max_tries != 0) && (attempts >= max_tries)))  {
		audit_event (1, MSGSTR_SEC(S_TOOMANY, "too many unsuccessful attempts"));
		if (su_on_console)
			show_error(MSGSTR_SEC(LOGIN_SEC_89,
			"Terminal is disabled but root login is allowed.\n"));
		else  {
#if SEC_NET_TTY
			if (is_network)
                                show_error(MSGSTR_SEC(LOGIN_SEC_90,
		"Host is disabled -- see Account Administrator. \n"));
                        else 
#endif
			show_error(MSGSTR_SEC(LOGIN_SEC_91,
		"Terminal is disabled -- see Account Administrator.\n"));
			/* cancel_process(1, prpw, pr); -DAL */
			return 0;
		}
	}
	return 1; /* -DAL */
}


/*
 * This routine cuts off all references to the Authentication database.
 * It loses the in core references we have as well as closing any open
 * files.
 */
static void
end_authentication(prpwd, prtc)
	struct pr_passwd **prpwd;
	struct pr_term **prtc;
{
	*prpwd = (struct pr_passwd *) 0;
	*prtc = (struct pr_term *) 0;
	current_prpwd = (struct pr_passwd *) 0;
	current_prtc = (struct pr_term *) 0;

	endprpwent();
	endprfient();
	endprtcent();
	endprdfent();
	endgrent();
	endpwent();
}


/*
 * This routine executes a command as the given uid and uses name as
 * the argument.  We do this as a subprocess so that we do not change
 * the ownership of the current process.  Return codes:
 * < 0	pgm_path could not be executed
 * = 0	pgm_path ran successfully
 * > 0	pgm_path ran unsuccessfully
 */
static int
execute(pgm_path, uid, user)
	char *pgm_path;
	int uid;
	char *user;
{
	register int child_pid;
	register int return_code;
	register uid_t savuid;
	int wait_stat;
	char *pargv[3];

	if (!strcmp(pgm_path, PASSWD_PROGRAM)) {
		savuid = getuid();
		if (_c2_collinput && (setruid(uid)==0)) {
			pargv[0] = pgm_path;
			pargv[1] = user;
			pargv[2] = (char *) 0;
			return_code = sia_chg_password(_c2_collect,
							user, 3, pargv);
			/* check against SIASUCCESS */
			return_code = !(return_code&1);
		}
		else
			return_code = -1;

		if (setruid(savuid) != 0) {
			show_error("restore uid");
			return_code = -1;
		}
		return return_code;
	}

	child_pid = fork();

	switch (child_pid)
	{
		case -1:
			/*
			 * Could not fork for some reason.
			 */
			return_code = -1;
			break;

		case 0:
			/*
			 * Child -- change user and execute program.
			 */
			    if(setuid(uid) == 0)
			    (void) execl(pgm_path,
				 strrchr(pgm_path,'/')+1,
				 user, (char *) 0);

			/*
			 * This makes a negative return code from execute().
			 */
			_exit(255);
			break;

		default:
			/*
			 * Parent -- just wait for child without
			 *	      interruptions.
			 */
			enter_quiet_zone();
			do  {
				return_code = wait(&wait_stat);
			}
			while ((return_code != child_pid) &&
			       (return_code != -1));
			exit_quiet_zone();

			/*
			 * Force sign extension so we'll make an
			 * exit() from the child, with bit 0x80 set
			 * returned here forced to be a short and then
			 * int, a negative number.  E.g., exit(255)
			 * from the child returns 0xFF00 here.  As a
			 * signed short, that number is negative.
			 */
			if (return_code == child_pid)
				return_code = (int) (short) wait_stat;
			break;
	}

	return return_code;
}


/*
 * Fill in both the passwd and pr_passwd entries provided with names
 * and passwords that cannot ever be matched.  This allows the authentication
 * procedure to continue to a point where the authentication can be done
 * unconditrionally on both good and bad entries.  Such a scheme, expanded
 * from the existing UNIX method, reduces a covert channel whereby an
 * intruder could determine valid and invalid login names based on either
 * the messages produced from login program or on the program delay differences
 * in processing good and bad entries.
 */
static void
impossible_user(ppr, ppwd)
	struct pr_passwd **ppr;
	struct passwd **ppwd;
{
	struct pr_default *df;

	noprot.uflg.fg_name = 1;
	noprot.uflg.fg_encrypt = 1;

	/*
	 * Since ':' is the delimiter of the passwd and protected passwd
	 * files, it cannot ever be part of a name.  Nothing will ever
	 * match this!
	 */
	(void) strcpy(noprot.ufld.fd_name, NO_ENTRY);
	(void) strcpy(noprot.ufld.fd_encrypt, NO_ENTRY);

	/*
	 * For the non-user, use system default values.
	 */
	df = getprdfnam(AUTH_DEFAULT);
	if (df != (struct pr_default *) 0)
		dflt = *df;
	noprot.sfld = dflt.prd;
	noprot.sflg = dflt.prg;

	*ppr = &noprot;
	current_prpwd = *ppr;
	*ppwd = &nouser;
}


/*
 * If possible, reset the terminal mode to no access and give back the
 * terminal so that no one can open it when not in use.  Then, leave the
 * program.
 */
static void
reset_tty_discr_and_bye(code)
	int code;
{
	if ((save_tty_path != (char *) 0) && save_tty_isatty)  {
		(void) chmod(save_tty_path, 0);
		if (term_group == -1)
			(void) chown(save_tty_path, 0, 0);
		else
			(void) chown(save_tty_path, 0, term_group);

#if SEC_MAC && !SEC_SHW && !defined(TPLOGIN)
		/*
		 * For a failure, if possible, reset the terminal security
		 * level to Syshi.  This locks out attempts at lower levels
		 * to detect terminal use by afuture session.  By setting to
		 * Syshi here, another user can only stat the line when a
		 * user is logged in that is dominated by him.
		 */
		if (mand_init() == 0)
			(void) chslabel(save_tty_path, mand_syshi);
#endif
	}

	exit(code);
}

/*
 * routine returns true if the account specified should allow logins on
 * the terminal regardless of the condition of the databases.  This is
 * an "out" to allow the system to be returned to a normal state.
 *
 * This routine starts up a shell with all privileges.
 *
 * Eventually, there should be a B1 definition does not assume a root account.
 */

static char *failure_env[] = {
	"HOME=/",
	(char *) 0,
	"SHELL=/sbin/sh",
	(char *) 0
};

static void
failure_override(pr, prtc)
struct pr_passwd *pr;
struct pr_term *prtc;
{
	mask_t privs[SEC_SPRIVVEC_SIZE];
	char logname_env[64];
	int i;

	/*
	 * If there is a protected password or a terminal control entry and
	 * the user is not root or the terminal is not the console, return.
	 */
	if ((pr != (struct pr_passwd *) 0 &&
	     strcmp(pr->ufld.fd_name, OVERRIDE_USER)) ||
	    (prtc != (struct pr_term *) 0 &&
	     strcmp(prtc->ufld.fd_devname, SYSTEM_CONSOLE)))
		return;

	/* Notify that the override is taking place. */
	show_error(MSGSTR_SEC(LOGIN_SEC_93,
		"\nThe security databases are corrupt.\nHowever, %s login at terminal %s is allowed\n\n"),
	  OVERRIDE_USER, SYSTEM_CONSOLE);

	/* do everything necessary to log into an account at the console */
	setgid(0);
	setuid(0);
#if SEC_GROUPS
	initgroups(OVERRIDE_USER, 0);
#endif
	/* set up a privilege mask that allows all defined privileges */
	memset(privs, '\0', sizeof(privs));
	for (i = 0; i <= SEC_MAX_SPRIV; i++)
		ADDBIT(privs, i);
	RMBIT(privs, SEC_SUCOMPAT);
	RMBIT(privs, SEC_SUPROPAGATE);

	setpriv(SEC_MAXIMUM_PRIV, privs);
	setpriv(SEC_BASE_PRIV, privs);
	chdir("/");
	for (i = 0; i < NSIG; i++)
		signal(i, SIG_DFL);

	/* put user name in environment */
	sprintf(logname_env, "LOGNAME=%s", OVERRIDE_USER);
	failure_env[1] = logname_env;
	execle("/sbin/sh", "-sh", (char *) 0, failure_env);
	show_error(MSGSTR_SEC(LOGIN_SEC_94,
		"Impossible to execute /sbin/sh!\n"));
	exit(1);
}

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

#if SEC_NET_TTY /*{*/
static struct pr_term *
check_devasg_net(host_entry)
char *host_entry;
{
        int i;
        struct dev_asg *dvag;
        char *cp;

        setdvagent();
        while ((dvag = getdvagent()) != (struct dev_asg *) 0)
                if (dvag->uflg.fg_devs)
                        for (i = 0; cp = dvag->ufld.fd_devs[i]; i++)
                                if (strcmp(cp, host_entry) == 0)
                                        return getprtcnam(dvag->ufld.fd_name);
        return (struct pr_term *) 0;
}
#endif /*} SEC_NET_TTY */

#ifndef TPLOGIN /*{*/
#if SEC_MAC /*{*/
/*
 * This routine attempts to set the user's clearance and the security
 * level for the session.  If it does so, and also reports back the
 * security level to the user, the routine returns 1.  In all other
 * cases, it is considered a failure and 0 is returned.
 *
 * The routine starts by getting the security level (or synonym) from the
 * user.  (Note that password authentication has already been successful
 * at this point.)  If the security level given is known, the clearance and
 * the security level are set (in that order, so that the system can determine
 * if the clearance dominates the security level).  Lastly, login echoes
 * the security level chosen, this time fully expanded so the user is reminded
 * of the actual security level chosen in all its glory.
 */
#if !SEC_SHW /*{*/
static int
get_sec_level(prpwd, prtc)
	register struct pr_passwd *prpwd;
	register struct pr_term *prtc;
{
	register int posn;
	register int error;
	register int more_chars;
	register int decision;
	register struct dev_asg *prdevasg;
	register FILE *tty;
	char *ttynm;
	char *sec_level_er;
	int valid_sec_level = 0;
	int sec_level_len;
	mand_ir_t *clearance;

	tty = fopen("/dev/tty", "r+");
	if ((tty == (FILE *) 0) || ((er_buffer = malloc(BUFSIZ)) == (char *) 0)){
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_95,
			"cannot request sensitivity level"));
		show_error(MSGSTR_SEC(LOGIN_SEC_96,
			"Cannot request sensitivity level.\n"));
		cancel_process(1, prpwd, prtc);
	}

	setbuf(tty, (char *) 0);

	fprintf(tty, MSGSTR_SEC(LOGIN_SEC_97, "Sensitivity level: "));

	error = !feof(tty) && !ferror(tty);
	more_chars = error;
	posn = 0;
	while (more_chars)  {
		er_buffer[posn] = getc(tty);
		error = feof(tty) || ferror(tty) ||
			(posn >= BUFSIZ - 1);
		if (error || (er_buffer[posn] == '\n'))  {
			/*
			 * Do not want \n in external representation.
			 */
			er_buffer[posn] = '\0';
			more_chars = 0;
		}
		else if (posn ||
			 er_buffer[posn] != ' ' && er_buffer[posn] != '\t')
			posn++;
	}

	(void) fclose(tty);

	if (error)
		cancel_process(1, prpwd, prtc);

	if (posn)
		sec_level_ir = mand_er_to_ir(er_buffer);
	else
#if SEC_CMW
		sec_level_ir = mand_minsl;
#else
		sec_level_ir = mand_syslo;
#endif

	if (prpwd->uflg.fg_clearance)
		clearance = &prpwd->ufld.fd_clearance;
	else if (prpwd->sflg.fg_clearance)
		clearance = &prpwd->sfld.fd_clearance;
	else
#if SEC_CMW
		clearance = mand_minclrnce;
#else
		clearance = mand_syslo;
#endif

	if (sec_level_ir != (mand_ir_t *) 0)  {

		/*
		 * Set the clearance of the user.  The system makes
		 * sure it falls within Syshi and Syslo.
		 */

		if(setclrnce(clearance) == -1)
			valid_sec_level = 0;
		else  {
#if SEC_CMW
			int rel;

			/*
			 * Ensure that the requested sensitivity level
			 * dominates the system's minimum useful SL.
			 * If not, use the system minimum.
			 */
			
			rel = mand_ir_relationship(sec_level_ir, mand_minsl);
			if ((rel & (MAND_EQUAL | MAND_SDOM)) == 0)
				sec_level_ir = mand_minsl;
#endif /* SEC_CMW */

			/*
			 * Now set the security level for the session.  The
			 * system checks to see if it is dominated by the
			 * previously set clearance.  It is an error if not.
			 */

			if(setslabel(sec_level_ir) == -1)
				valid_sec_level = 0;
			else
				valid_sec_level = 1;
		}
	}

	if (valid_sec_level)  {
		/*
		 * Reprint security level to both serve as a visual validation
		 * to the user and also to expand any synonyms he may have used.
		 */
		sec_level_er = mand_ir_to_er (sec_level_ir);
		if (sec_level_er != (char *) 0) {
			sec_level_len = strlen(sec_level_er) + 1;
			if (sec_level_len >= BUFSIZ)
				er_buffer = realloc(er_buffer, sec_level_len);
			if (er_buffer) {
				strcpy(er_buffer, sec_level_er);
				show_mesg(MSGSTR_SEC(LOGIN_SEC_98,
					"Sensitivity level for process: "));
				printbuf(sec_level_er,32,",/ ");
/* DAL
				show_mesg("\n");
*/
			} else {
				show_error(MSGSTR_SEC(LOGIN_SEC_99,
					"Sensitivity label is too long\n"));
				valid_sec_level = 0;
			}
		}
		else {
			show_error(MSGSTR_SEC(LOGIN_SEC_100,
			"Mandatory database error; see administrator\n"));
			valid_sec_level = 0;
		}
	}

	if(!valid_sec_level)
		return valid_sec_level;

	/*
	 * Get the device assignment database entry for the terminal. If
	 * there is no entry, get the system default database values.
	 */

	enddvagent();
#if SEC_NET_TTY
	if (is_network) {
                ttynm = prtc->ufld.fd_devname;
                if (ttynm == NULL) {
                        show_error(MSGSTR_SEC(LOGIN_SEC_101,
				"Can not get host name\n"));
                        return(0);
                }
        }
	else 
#endif /* SEC_NET_TTY */
	{
		ttynm = (char *) ttyname(0);
		if((ttynm == NULL) || ((ttynm = strrchr(ttynm,'/')) == NULL)) {
			show_error(MSGSTR_SEC(LOGIN_SEC_102,
				"Can not get terminal name\n"));
			return(0);
		}
		else ttynm++;		/* bump past the '/' */
	}
	if((prdevasg = getdvagnam(ttynm)) == (struct dev_asg *) 0) {
		show_error(MSGSTR_SEC(LOGIN_SEC_103,
			"Unable to get device sensitivity level\n"));
		return(0);
	}

	/*
	 * Check the current sensitivity level against the terminal min
	 * and max sensitivity level. If none, use the system default
	 * values for these.
	 */

	decision = -1;

	if((prdevasg) && ((prdevasg->uflg.fg_min_sl) ||
			  (prdevasg->sflg.fg_min_sl))) {

	   if(prdevasg->uflg.fg_min_sl)
		decision = mand_ir_relationship(sec_level_ir,
						prdevasg->ufld.fd_min_sl);
	   else if(prdevasg->sflg.fg_min_sl)
		decision = mand_ir_relationship(sec_level_ir,
						prdevasg->sfld.fd_min_sl);
	}
	else {
		show_error(MSGSTR_SEC(LOGIN_SEC_103,
			"Unable to get device sensitivity level\n"));
		return(0);
	}

	/*
	 * A decision value of -1 means no minimum level was found
	 * anywhere to compare against. This case falls through. A
	 * decision value of 0 means an error occurred. Otherwise,
	 * there is a decision which is checked to insure that the
	 * sensitivity level dominates the minimum level.
	 */

	if((decision == 0) || (decision == -1))
		return(0);

	if((decision > 0) &&
	   (!(decision & MAND_EQUAL) && !(decision & MAND_SDOM)))
		return(0);

	/*
	 * Do the same operations for the maximum level except that
	 * the maximum level must dominate the sensitivity level.
	 */

	decision = -1;

	if(prdevasg) {
	   if(prdevasg->uflg.fg_max_sl)
		decision = mand_ir_relationship(prdevasg->ufld.fd_max_sl,
						sec_level_ir);
	   else if(prdevasg->sflg.fg_max_sl)
		decision = mand_ir_relationship(prdevasg->sfld.fd_max_sl,
						sec_level_ir);
	}
	else {
		show_error(MSGSTR_SEC(LOGIN_SEC_103,
			"Unable to get device sensitivity level\n"));
		return(0);
	}

	/*
	 * A decision value of -1 means no maximum level was found
	 * anywhere to compare against. This case falls through. A
	 * decision value of 0 means an error occurred. Otherwise,
	 * there is a decision which is checked to insure that the
	 * sensitivity level dominates the maximum level.
	 */

	if((decision == 0) || (decision == -1))
		return(0);

	if((decision > 0) &&
	   (!(decision & MAND_EQUAL) && !(decision & MAND_SDOM)))
		return(0);

	/*
	 * The sensitivity level is valid and the ir can be used.
	 */

#if SEC_ILB
	setilabel(mand_syslo);
#endif

	return(1);

}

#else /* SEC_SHW */

static int
get_sec_level(prpwd, prtc)
	register struct pr_passwd *prpwd;
	register struct pr_term *prtc;
{
	if (setclrnce(mand_syshi) == -1)
		return 0;

	if (setslabel(mand_syshi) == -1)
		return 0;

#if SEC_ILB
	if (setilabel(mand_syslo) == -1)
		return 0;
#endif

	return(1);

}
#endif /*} SEC_SHW */

#endif /*} SEC_MAC */

#if SEC_NCAV /*{*/
/*
 * This routine requests the session caveat set from the user. This is
 * then enforced by checking the user citizenship field and the user's
 * maximum caveat set for the proper relationship. The caveat set is
 * set for the process and the caveat set range checking for the terminal
 * device is also enforced using the device assignment database.
 */

static int
get_caveat_set(prpwd, prtc)
register struct pr_passwd *prpwd;
register struct pr_term *prtc;
{
	register int posn;
	register int error;
	register int more_chars;
	register int decision;
	register struct dev_asg *prdevasg;
	register FILE *tty;
	char *ttynm;
	char *ncav_er;
	int ncav_er_len;

	tty = fopen("/dev/tty", "r+");
	if (tty == (FILE *) 0)  {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_104,
			"error in getting security level from /dev/tty"));
		show_error(MSGSTR_SEC(LOGIN_SEC_105,
			"Cannot request caveat set.\n"));
		cancel_process(1, prpwd, prtc);
	}

	if ((ncav_buffer = malloc(BUFSIZ)) == (char *) 0) {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_106,
				 "cannot request caveat set"));
		show_error(MSGSTR_SEC(LOGIN_SEC_105,
			"Cannot request caveat set.\n"));
		cancel_process(1, prpwd, prtc);
	}

	setbuf(tty, (char *) 0);

	fprintf(tty, MSGSTR_SEC(LOGIN_SEC_107, "Nationality caveat set: "));

	error = !feof(tty) && !ferror(tty);
	more_chars = error;
	posn = 0;
	while (more_chars)  {
		ncav_buffer[posn] = getc(tty);
		error = feof(tty) || ferror(tty) ||
			(posn >= BUFSIZ - 1);
		if (error || (ncav_buffer[posn] == '\n'))  {
			/*
			 * Do not want \n in external representation.
			 */
			ncav_buffer[posn] = '\0';
			more_chars = 0;
		}
		else
			posn++;
	}

	(void) fclose(tty);

	if (error) {
		audit_event(1, MSGSTR_SEC(LOGIN_SEC_108, 
				"error in getting caveat set from /dev/tty"));
		cancel_process(1, prpwd, prtc);
	}
	/*
	 * If the caveat set was specified as CR (null set), then set the
	 * caveat set internally to all caveats.
	 */

	if(ncav_buffer[0] == '\0') {
		if((ncav_ir = ncav_alloc_ir()) == (ncav_ir_t *) 0)
			return(0);

		*ncav_ir = *ncav_max_ir;
	}
	else {
		ncav_ir = ncav_er_to_ir(ncav_buffer);

		if (ncav_ir == (ncav_ir_t *) 0)
			return(0);
	}

	/*
	 * The specified process caveat set must dominate the user maximum
	 * caveat set and the caveat set must contain one or more of the
	 * user citizenship fields.
	 */

	if (!prpwd->uflg.fg_nat_caveats) return(0);

	decision = ncav_ir_relationship(ncav_ir, prpwd->ufld.fd_nat_caveats);

	if (decision == 0) return(0);
	
	if(!(decision & NCAV_EQUAL) && !(decision & NCAV_SDOM))
		return(0);

	/*
	 * Make sure that there is at least one of the user citizenship
	 * fields in the specified caveat set. No citizenship is required
	 * if the process caveat set is being set to the Null or Universe
	 * caveat set. This is equivalent to on caveats and as such
	 * should not require a citizenship. (Null citizenship support.)
	 */

	if(*ncav_max_ir != *ncav_ir) {
		if((!prpwd->uflg.fg_nat_citizen) ||
		   ((*ncav_ir & *prpwd->ufld.fd_nat_citizen) == 0))
			return(0);
	}

	/* Set the caveat set for the process */

	if(setncav(ncav_ir) == -1)
		return(0);

	/*
	 * Reprint caveat set to both serve as a visual validation
	 * to the user and also to expand any synonyms used.
	 */

	ncav_er = ncav_ir_to_er (ncav_ir);
	if (ncav_er != (char *) 0) {
		ncav_er_len = strlen(ncav_er_len) + 1;
		if (ncav_er_len >= BUFSIZ)
			ncav_buffer = realloc(ncav_buffer, ncav_er_len);
		if (ncav_buffer) {
			strcpy(ncav_buffer, ncav_er);
			show_mesg(MSGSTR_SEC(LOGIN_SEC_109,
				"Caveat set for process: "));
			printbuf(ncav_er,24,",");
/* DAL
			show_mesg("\n");
*/
		} else {
			show_error(MSGSTR_SEC(LOGIN_SEC_110,
				"Caveat set is too long\n"));
			return(0);
		}
	}
	else {
		show_error(MSGSTR_SEC(LOGIN_SEC_111,
			"Caveat database error; see administrator\n"));
		return(0);
	}

	/*
	 * Get the device assignment database entry for the terminal. If
	 * there is no entry, get the system default database values.
	 */

	enddvagent();
#if SEC_NET_TTY
	if (is_network) {
                ttynm = prtc->ufld.fd_devname;
                if (ttynm == NULL) {
                        show_error(MSGSTR_SEC(LOGIN_SEC_101,
				"Can not get host name\n"));
                        return(0);
                }
        }
        else 
#endif /* SEC_NET_TTY */
	{
                ttynm = (char *) ttyname(0);
                if((ttynm == NULL) || ((ttynm = strrchr(ttynm,'/')) == NULL)) {
                        show_error(MSGSTR_SEC(LOGIN_SEC_102,
				"Can not get terminal name\n"));
                        return(0);
                }
         	else ttynm++;
	}
	if((prdevasg = getdvagnam(ttynm)) == (struct dev_asg *) 0) {
		show_error(MSGSTR_SEC(LOGIN_SEC_112,
			"Unable to get device caveat set\n"));
		return(0);
	}

	/*
	 * Check the caveat set against the terminal min and
	 * max caveat sets. If none, use the system default
	 * values for these.
	 */

	decision = -1;

	if((prdevasg) && ((prdevasg->uflg.fg_min_nat_caveats) ||
			  (prdevasg->sflg.fg_min_nat_caveats))) {

	   if(prdevasg->uflg.fg_min_nat_caveats)
		decision =
		  ncav_ir_relationship(ncav_ir,prdevasg->ufld.fd_min_nat_caveats);
	   else if(prdevasg->sflg.fg_min_nat_caveats)
		decision =
		  ncav_ir_relationship(ncav_ir,prdevasg->sfld.fd_min_nat_caveats);
	}
	else return(0);

	/*
	 * A decision value of -1 means no minimum set was found
	 * anywhere to compare against. This case falls through. A
	 * decision value of 0 means an error occurred. Otherwise,
	 * there is a decision which is checked to insure that the
	 * new caveat set dominates the minimum caveat set.
	 */

	if((decision == 0) || (decision == -1))
		return(0);

	if((decision > 0) &&
	   (!(decision & NCAV_EQUAL) && !(decision & NCAV_SDOM)))
		return(0);

	/*
	 * Do the same operations for the maximum set except that
	 * the maximum set must dominate the new caveat set.
	 */

	decision = -1;

	if((prdevasg) && ((prdevasg->uflg.fg_max_nat_caveats) ||
			  (prdevasg->sflg.fg_max_nat_caveats))) {

	   if(prdevasg->uflg.fg_max_nat_caveats)
		decision =
		  ncav_ir_relationship(prdevasg->ufld.fd_max_nat_caveats,ncav_ir);
	   else if(prdevasg->sflg.fg_max_nat_caveats)
		decision =
		  ncav_ir_relationship(prdevasg->sfld.fd_max_nat_caveats,ncav_ir);
	}
	else return(0);

	/*
	 * A decision value of -1 means no maximum set was found
	 * anywhere to compare against. This case falls through. A
	 * decision value of 0 means an error occurred. Otherwise,
	 * there is a decision which is checked to insure that the
	 * caveat set dominates the maximum set.
	 */

	if((decision == 0) || (decision == -1))
		return(0);

	if((decision > 0) &&
	   (!(decision & NCAV_EQUAL) && !(decision & NCAV_SDOM)))
		return(0);

	/*
	 * The caveat set is valid and the ir can be used.
	 */

	return(1);

}
#endif /*} SEC_NCAV */

#if !SEC_SHW /*{*/
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
	char remdev[20];
	char newdev[20];
	char devnm[14];
	char *tty;
#if SEC_MAC
	int fd, i, devcount = 0;
	struct stat statbuf;
#endif

	if(!prtc->uflg.fg_devname)
		return(0);

	/*
	 * Don't try to regrade if the device is not on a
	 * labeled filesystem.
	 */
/* The following ifdef was added by DAL */
#ifdef	SEC_ARCH
	if (!islabeledfs("/dev"))
		return 1;
#endif	/* SEC_ARCH */

	/*
	 * Get the device assignment database entry.
	 */

	if((dev = getdvagnam(prtc->ufld.fd_devname)) == (struct dev_asg *) 0) {
		show_error(MSGSTR_SEC(LOGIN_SEC_113,
			"Can not get terminal device assignment entry"));
		return(0);
	}

#if SEC_NET_TTY
	if (is_network) {
                tty = strrchr(save_tty_path, '/');
                tty++;
                strcpy(devnm,tty);
                sprintf(newdev, "/dev/%s", tty);
                sprintf(remdev, "/dev/%s", tty);
        }
        else
#endif /* SEC_NET_TTY */
	{
		strncpy(devnm, prtc->ufld.fd_devname,
			sizeof(prtc->ufld.fd_devname));
		sprintf(newdev, "/dev/%s", prtc->ufld.fd_devname);
	}

#if SEC_MAC /*{*/
	/*
	 * Stat(2) the device to get the major/minor for mknod(2). Also,
	 * since the user and group have already been set, use the stat
	 * information to set these on the new device.
	 */

	if(stat(newdev,&statbuf) == -1) {
		show_error(MSGSTR_SEC(LOGIN_SEC_114,
			"Unable to stat terminal device"));
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
		show_error(MSGSTR_SEC(LOGIN_SEC_115,
			"Can not create temporary node"));
		return(0);
	}

	if(chmod(newdev,statbuf.st_mode & 0777) == -1) {
		show_error(MSGSTR_SEC(LOGIN_SEC_116,
			"Unable to chmod new device node\n"));
		return(0);
	}

	if(chown(newdev,statbuf.st_uid,statbuf.st_gid) == -1) {
		show_error(MSGSTR_SEC(LOGIN_SEC_117,
			"Unable to chown new device node\n"));
		return(0);
	}

#if SEC_NET_TTY
	if (is_network) {
                unlink(remdev);
                if(link(newdev,remdev) == -1) {
                        show_error(MSGSTR_SEC(LOGIN_SEC_118,
				"Unable to link new device node\n"));
                        return(0);
                }
        }
        else 
#endif /* SEC_NET_TTY */
	{
		unlink(dev->ufld.fd_devs[0]);
		if(link(newdev,dev->ufld.fd_devs[0]) == -1) {
			show_error(MSGSTR_SEC(LOGIN_SEC_118,
				"Unable to link new device node\n"));
			return(0);
		}
	}
	unlink(newdev);

	/*
	 * Regrade the device according to the specified sensitivity level.
	 */

#if SEC_NET_TTY
	if (is_network) {
                if(chslabel(remdev,mand_ir) == -1) {
                        show_error(MSGSTR_SEC(LOGIN_SEC_119,
				"Unable to regade terminal device\n"));
                        return(0);
                }
        }
        else 
#endif /* SEC_NET_TTY */
	{
		if(chslabel(dev->ufld.fd_devs[0],mand_ir) == -1) {
			show_error(MSGSTR_SEC(LOGIN_SEC_119,
				"Unable to regade terminal device\n"));
			return(0);
		}
	}
#endif /*} SEC_MAC */

#if SEC_NCAV /*{*/
	/*
	 * Modify the caveat set on the terminal device.
	 */

#if SEC_NET_TTY
	if (is_network) {
                if(chncav(remdev,ncav_ir) == -1) {
                        show_error(MSGSTR_SEC(LOGIN_SEC_120,
				"Unable to reset terminal caveat set\n"));
                        return(0);
                }
        }
        else 
#endif /* SEC_NET_TTY */
	{
		if(chncav(dev->ufld.fd_devs[0],ncav_ir) == -1) {
			show_error(MSGSTR_SEC(LOGIN_SEC_120,
				"Unable to reset terminal caveat set\n"));
			return(0);
		}
	}
#endif /*} SEC_NCAV */

#if SEC_MAC /*{*/
	/*
	 * If the device count is > 1, then alias devices exist. The
	 * aliases must be unlinked and relinked to the new real device
	 * node after it has been regraded.
	 */

#if SEC_NET_TTY
	if (!is_network) 
#endif
	{
		if(devcount > 1) {
			for(i=1;  i < devcount; i++) {
				if(!dev->ufld.fd_devs[i])
					break;

				unlink(dev->ufld.fd_devs[i]);
				link(dev->ufld.fd_devs[0],dev->ufld.fd_devs[i]);
			}
		}
	}
	/*
	 * The stdin, stdout, and stderr streams still point to the old
	 * device node that was unlinked. Open the new node, and then
	 * close the old streams dup'ing the new node for use by the
	 * login user.
	 */

#if SEC_NET_TTY
	if (is_network) {
                if((fd = open(remdev,O_RDWR)) == -1) {
                        show_error(MSGSTR_SEC(LOGIN_SEC_121,
			"Unable to open newly regraded terminal device\n"));
                        return(0);
                }
        } 
	else 
#endif
	{
		if((fd = open(dev->ufld.fd_devs[0],O_RDWR)) == -1) {
	    		show_error(MSGSTR_SEC(LOGIN_SEC_121,
			"Unable to open newly regraded terminal device\n"));
	    		return(0);
		}
	}
	close(0);
	close(1);
	close(2);
	dup(fd);
	dup(fd);
	dup(fd);
	close(fd);
#ifdef _OSF_SOURCE
	/* Must reset the controlling tty explicitly */
	if (ioctl(0, TIOCSCTTY, 0) < 0)
		show_error(MSGSTR_SEC(LOGIN_SEC_122,
			"Can't reset controlling tty after regrade\n"));
#endif /* _OSF_SOURCE */
#endif /*} SEC_MAC */

	return(1);
}
#endif /*} !SEC_SHW */
#endif /*} !TPLOGIN */
/* #endif */ /*} SEC_BASE */
