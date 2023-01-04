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
static char	*sccsid = "@(#)$RCSfile: passwd_sec.c,v $ $Revision: 1.1.7.7 $ (DEC) $Date: 1993/12/16 23:55:50 $";
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
 * Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 */

#include <sys/secdefines.h>

/* #if SEC_BASE */ /*{*/


/*
 * Program that changes the password by enforcing the values in the
 * protected password database.  The program assumes that it is SGID to
 * a group that can read the protected passsword database files and
 * write into the protected password directories.  It is least privileged to
 * run with protected password files protected at System High.
 */

#include <stdlib.h>
#include <sia.h>
#include <siad.h>
#include <sia_mech.h>
#include <time.h>

#include <sys/types.h>
#include <sys/security.h>
#include <sys/audit.h>
#ifndef	T_ERRNO
#define	T_ERRNO	T_ERROR
#endif
#include <sys/sec_objects.h>
#include <mandatory.h>
#include <sys/signal.h>
#include <prot.h>
#include <pwd.h>
#include <ndbm.h>
#include <stdio.h>
#include <sys/wait.h>
#ifndef _PATH_DEVNULL
#include <paths.h>
#endif

#define PASSWD_BUFSIZ 100

extern priv_t *privvec();
extern int (*_c2_collect)();
char *putGetLine();
static void abort_change();
static volatile int interrupted;
static char *passwd_crypt();
static char *c2_getpasswd();
static int passwd_random(), passwd_pick();

static int ask_pwpolicy();

/*
 * Macros that raise and lower the ability to read and write the protected
 * password database:
 */

privvec_t		save_privs;	/* utility use vector */
#if SEC_ENCODINGS
#define RAISE_READ()	forceprivs(privvec(SEC_ALLOWMACACCESS, -1), save_privs)
#define LOWER_READ()	seteffprivs(save_privs, (priv_t *) 0)
#else
#define RAISE_READ()
#define LOWER_READ()	
#endif
privvec_t		write_privs;	/* privileges to raise for writing */
#define RAISE_WRITE()	forceprivs(write_privs, save_privs)
#define LOWER_WRITE	seteffprivs(save_privs, (priv_t *) 0)

#define MAXTRIES 3
#define max(a,b)	((a) > (b) ? (a) : (b))

/* Parameters from the protected password database that govern password
 * change procedures.
 */

int restrict;		/* Whether the new password must be checked */
int prestrict;		/* Ditto, but by external policy call */
int maxpasslen;		/* Maximum password length for the new password */
int minpasslen;		/* Minimum  "	     "	    "   "   "   " */
time_t lifetime;	/* password lifetime */
time_t log_delay;	/* Delay between login attempts (system-wide) */
time_t last_change;	/* Last successful change */
time_t last_bad;	/* Last unsuccessful change */
time_t min_time;	/* Minimum change time */
char allownullpw;	/* Are null passwords allowed on this account? */
char different_user;	/* Is the user changing another's account? */
char *old_cipher;	/* Ciphertext of old password */
char **cipher_hist;	/* Ciphertext list of password history */
int cipher_depth;	/* How much history to keep */
char pwpolicy[MAXPATHLEN]; /* Pathname of site password policy program */

/* Password generation options: */
int can_pick;
int can_do_random_word;
int can_do_random_chars;
int can_do_random_letters;
int choices;		/* Number of password generation choices */
int save_choice;	/* Of only one choice, the number allowed */
int old_alg, new_alg;	/* Current and new encryption algorithms */

int login_uid;		/* Login user ID of user who invoked program */
char *login_name;	/* Name of user login_uid */
struct pr_passwd account_entry;	/* Protected password entry of account */
char *account_name;	/* User name of account whose password to be changed */
uid_t account_uid;	/* User ID of account whose password to be changed */
char new_passwd[AUTH_MAX_PASSWD_LENGTH+1];	/* New password chosen or generated */

#define	PASSWD_WORD_REQ	GETMSG(MS_SIA_PWD, S_ENT1, "Password: %-*s  Hyphenation: %s\nEnter password:")

#define	PASSWD_CHAR_REQ	GETMSG(MS_SIA_PWD, S_ENT2, "Password: %-*s\nEnter password:")
#define	PASSWD_LETT_REQ	GETMSG(MS_SIA_PWD, S_ENT2, "Password: %-*s\nEnter password:")
 
#define	PASSWD_PICK	1
#define	PASSWD_WORD	2
#define	PASSWD_CHARS	3
#define	PASSWD_LETTERS	4

#define	PASSWD		"/etc/passwd"
#define	PTEMP		"/etc/ptmp"

/*
 * Generate an audit record.
 */
static void audevent(status, code)
char *status;
int code;
{
/*  Verify where account_name comes from... 
	audgenl ( AUTH_EVENT,
		T_CHARP, 			status,
		(code ? T_ERRNO : T_RESULT ),	(char *) code,
		T_CHARP, 			account_name,
		T_UID,				account_uid,
		NULL);
*/
}

/*
 * retrieve the login user ID associated with the current process.
 * If it is not set, password change is not allowed.
 * Return the string name of the user.
 */

char *
passwd_getlname()
{
	register struct passwd *p;
	char *reason = NULL;
	char buffer[128];

	setprivvec(write_privs, SEC_CHOWN, SEC_LIMIT,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1);

	if (checkprivs(write_privs)) {
	        show_error(GETMSG(MS_SIA_PWD, PWD_INSUFF_PRIV, "insufficient privileges"));
		return(SIADFAIL);
	}

	/*
	 * luid is negative when it has not been set yet.
	 */
	login_uid = getluid();
	if (login_uid < 0)
		reason = GETMSG(MS_SIA_PWD, S_ID_NOT_SET, "Login user ID not set yet");
	else {
#if SEC_MAC
		/* Process must not have a wildcard sensitivity label */
		
		mand_ir_t *mand_ir;

		mand_ir = mand_alloc_ir();
		if (mand_ir == (mand_ir_t *) 0) {
			reason = GETMSG(MS_SIA_PWD, S_MEM_ALL_ERR, "Memory allocation error");
			goto bad;
		}
		if (getslabel(mand_ir) < 0) {
			reason = GETMSG(MS_SIA_PWD, S_LBL_NOT_SET, "Sensitivity label not set");
			goto bad;
		}
#endif
		RAISE_READ();
		p = getpwuid(login_uid);
		LOWER_READ();
		if (p == (struct passwd *) 0) {
			sprintf(buffer,
		GETMSG(MS_SIA_PWD, S_NO_ENTRY_PWD, "No /etc/passwd entry for user ID %d"),
			  login_uid);
			reason = buffer;
		} else {
			login_name = malloc(strlen(p->pw_name) + 1);
			if (login_name == NULL)
				reason = GETMSG(MS_SIA_PWD, S_MEM_ALL_ERR, "Memory allocation error");
			else
				strcpy(login_name, p->pw_name);
		}
	}

bad:
	if (reason != NULL)
		audevent(GETMSG(MS_SIA_PWD, S_PWD_FAIL, "password change failure"),1);
	return login_name;
}

/*
 * Check whether the user associated with uid is authorized to
 * modify the password (and shell and finger data, if supported)
 * of the user named by the /etc/passwd entry pwd.  Call passwd_no_change()
 * if there is an authorization or database problem.
 */

int
passwd_auth(pwd, uid)
	struct passwd *pwd;
	uid_t uid;
{
	register struct pr_passwd *ppwd;

	account_name = malloc(strlen(pwd->pw_name) + 1);
	if (account_name == NULL) {
		passwd_no_change(pwd->pw_name, (struct pr_passwd *) 0,
		  GETMSG(MS_SIA_PWD, S_MEM_ALL_ERR, "Memory allocation error"));
		return(SIADFAIL);
	}
	strcpy(account_name, pwd->pw_name);
	account_uid = pwd->pw_uid;

	RAISE_READ();
	ppwd = getprpwnam(pwd->pw_name);
	LOWER_READ();
	if (ppwd == (struct pr_passwd *) 0) {
		passwd_no_change(pwd->pw_name, (struct pr_passwd *) 0,
		  GETMSG(MS_SIA_PWD, S_PROT_PWD, "cannot access protected password entry"));
		return(SIADFAIL);
	}

	if (!ppwd->uflg.fg_uid || ppwd->ufld.fd_uid != account_uid) {
		passwd_no_change(account_name, ppwd,
		  GETMSG(MS_SIA_PWD, S_MISMATCH, "ID mismatch"));
		return(SIADFAIL);
	}

	if (ppwd->ufld.fd_uid != uid) {
		different_user = 1;
		if (!authorized_user("password")) {
			passwd_no_change(ppwd->ufld.fd_name, ppwd,
			  GETMSG(MS_SIA_PWD, S_NOT_AUTH1, "not authorized to change another user's password"));
			return(SIADFAIL);
		}
	}

	account_entry = *ppwd;

	return(SIADSUCCESS);
}

/*
 * Change the password associated with the user whose /etc/passwd entry is
 * in pwd.  The LUID of the user making the change is in uid.
 * This function never returns.
 */

passwd_change()
{
	struct pr_default *df;
	struct pr_passwd *ppwd;
	char *pC, buf[256];
	
	/*
	 * Get the default login delay for terminals.
	 */

	df = getprdfnam(AUTH_DEFAULT);
	if (df == (struct pr_default *) 0)
		passwd_no_change(login_name, &account_entry,
		  GETMSG(MS_SIA_PWD, S_CANT_RETR, "cannot access system default database"));

	if (df->tcg.fg_logdelay)
		log_delay = df->tcd.fd_logdelay;
	else
		log_delay = (time_t) 1;

	if (df->sflg.fg_pw_site_callout && df->sfld.fd_pw_site_callout[0]) {
		(void) strncpy(pwpolicy, df->sfld.fd_pw_site_callout,
				sizeof pwpolicy - 1);
	}
	else
		*pwpolicy = '\0';

	/*
	 * Store the relevant fields from the account being changed.
	 * If the changer is different, retrieve that user's entry
	 * and store relevant fields from that account as well.
	 */

	ppwd = &account_entry;
	
	if (store_changee_values(ppwd) == SIADFAIL)
	  return(SIADFAIL);

	if (different_user) {
		RAISE_READ();
		ppwd = getprpwuid(login_uid);
		LOWER_READ();
		if (ppwd == (struct pr_passwd *) 0)
		{
			passwd_no_change(login_name, &account_entry,
	GETMSG(MS_SIA_PWD, S_FAIL_RETR, "cannot access your protected password entry"));
			return(SIADFAIL);
		}
	}

	if (store_changer_values(ppwd) == SIADFAIL)
		return(SIADFAIL);
	
	/* Check that the user has at least one password changing option */

	if (choices == 0) {
		passwd_no_change(login_name, &account_entry,
		  GETMSG(MS_SIA_PWD, S_NOOPT, "you do not have any password changing options"));
		return(SIADFAIL);
	}

	if (different_user)
		ppwd = &account_entry;

	/* Check out the site password policy selection */

	if (prestrict) {
		struct stat stb;
		if (!pwpolicy[0]) {
			/* audgenl(?) */
			prestrict = 0;
		}
		else if (pwpolicy[0] != '/') {
			/* audgenl(?) */
			prestrict = 0;
		}
		else if (stat(pwpolicy, &stb) < 0) {
			/* audgenl(?) */
			prestrict = 0;
		}
		else if (!S_ISREG(stb.st_mode)) {
			/* audgenl(?) */
			prestrict = 0;
		}
		if (!prestrict) {
			restrict = 1; /* enforce some restriction */
		}
	}

	/* set up to audit user aborting password change */

	interrupted = 0;

	(void) signal(SIGINT, abort_change);
	(void) signal(SIGHUP, abort_change);
	(void) signal(SIGQUIT, abort_change);

	/* If necessary, validate that the changer knows the old password. */

	if (validate_old_password() == SIADFAIL)
	  return(SIADFAIL);
	
	/* the Print last successful and unsuccessful change times */

	sprintf(buf, GETMSG(MS_SIA_PWD, S_LAST_SUCC_CH, "Last successful password change for %s: %s"), 
		account_entry.ufld.fd_name,
		(last_change == (time_t) 0) ? GETMSG(MS_SIA_PWD, S_NEVER, "NEVER\n"): ctime(&last_change));
	
	sprintf(&buf[strlen(buf)], GETMSG(MS_SIA_PWD, S_LAST_UNSUCC_CH, "Last unsuccessful password change for %s: %s"),
		account_entry.ufld.fd_name,
		(last_bad == (time_t) 0) ? GETMSG(MS_SIA_PWD, S_NEVER, "NEVER\n"): ctime(&last_bad));

	buf[strlen(buf)] = '\0'; 	/* chop off the newline char */
	show_mesg(buf);

	  /* Check that the password falls between min change and lifetime */
	
	if (check_passwd_parameters() == SIADFAIL)
	  return(SIADFAIL);
	
	/* Choose the new password by whichever methods are allowed. */

	if (choose_password() == SIADFAIL)
	  return(SIADFAIL);
	
	/* Record the fact that the password was successfully changed */

	if (store_password() == SIADFAIL)
	  return(SIADFAIL);

	return(SIADSUCCESS);
}

/*
 * Store the relevant values from the changer's protected password entry
 */

store_changer_values(pr)
	struct pr_passwd *pr;
{
	if (pr->uflg.fg_policy)
		prestrict = pr->ufld.fd_policy;
	else if (pr->sflg.fg_policy)
		prestrict = pr->sfld.fd_policy;

	if (pr->uflg.fg_restrict)
		restrict = pr->ufld.fd_restrict;
	else if (pr->sflg.fg_restrict)
		restrict = pr->sfld.fd_restrict;

	if (pr->uflg.fg_pick_pwd)
		can_pick &= pr->ufld.fd_pick_pwd;
	else if (pr->sflg.fg_pick_pwd)
		can_pick &= pr->sfld.fd_pick_pwd;
	if (can_pick) {
		choices++;
		save_choice = PASSWD_PICK;
	}

	if (pr->uflg.fg_gen_pwd)
		can_do_random_word |= pr->ufld.fd_gen_pwd;
	else if (pr->sflg.fg_gen_pwd)
		can_do_random_word |= pr->sfld.fd_gen_pwd;
	if (can_do_random_word) {
		choices++;
		save_choice = PASSWD_WORD;
	}

	if (pr->uflg.fg_gen_chars)
		can_do_random_chars |= pr->ufld.fd_gen_chars;
	else if (pr->sflg.fg_gen_chars)
		can_do_random_chars |= pr->sfld.fd_gen_chars;
	if (can_do_random_chars) {
		choices++;
		save_choice = PASSWD_CHARS;
	}

	if (pr->uflg.fg_gen_letters)
		can_do_random_letters |= pr->ufld.fd_gen_letters;
	else if (pr->sflg.fg_gen_letters)
		can_do_random_letters |= pr->sfld.fd_gen_letters;
	if (can_do_random_letters) {
		choices++;
		save_choice = PASSWD_LETTERS;
	}

	return(SIADSUCCESS);
}

/*
 * Store the relevant values from the changee's protected password entry
 */

store_changee_values(pr)
	struct pr_passwd *pr;
{

	if (pr->uflg.fg_policy)
		prestrict = pr->ufld.fd_policy;
	else if (pr->sflg.fg_policy)
		prestrict = pr->sfld.fd_policy;
	else
		prestrict = 0;

	if (pr->uflg.fg_restrict)
		restrict = pr->ufld.fd_restrict;
	else if (pr->sflg.fg_restrict)
		restrict = pr->sfld.fd_restrict;
	else
		restrict = 1;

	if (pr->uflg.fg_oldcrypt)
		old_alg = pr->ufld.fd_oldcrypt;
	else if (pr->sflg.fg_oldcrypt)
		old_alg = pr->sfld.fd_oldcrypt;
	else
		old_alg = AUTH_CRYPT_BIGCRYPT;
	if (pr->uflg.fg_newcrypt)
		new_alg = pr->ufld.fd_newcrypt;
	else if (pr->sflg.fg_newcrypt)
		new_alg = pr->sfld.fd_newcrypt;
	else
		new_alg = AUTH_CRYPT_BIGCRYPT;

	if (pr->uflg.fg_pick_pwd)
		can_pick = pr->ufld.fd_pick_pwd;
	else if (pr->sflg.fg_pick_pwd)
		can_pick = pr->sfld.fd_pick_pwd;
	else
		can_pick = 0;

	if (pr->uflg.fg_gen_pwd)
		can_do_random_word = pr->ufld.fd_gen_pwd;
	else if (pr->sflg.fg_gen_pwd)
		can_do_random_word = pr->sfld.fd_gen_pwd;
	else
		can_do_random_word = 0;

	if (pr->uflg.fg_gen_chars)
		can_do_random_chars = pr->ufld.fd_gen_chars;
	else if (pr->sflg.fg_gen_chars)
		can_do_random_chars = pr->sfld.fd_gen_chars;
	else
		can_do_random_chars = 0;

	if (pr->uflg.fg_gen_letters)
		can_do_random_letters = pr->ufld.fd_gen_letters;
	else if (pr->sflg.fg_gen_letters)
		can_do_random_letters = pr->sfld.fd_gen_letters;
	else
		can_do_random_letters = 0;

	if (pr->uflg.fg_maxlen)
		maxpasslen = pr->ufld.fd_maxlen;
	else if (pr->sflg.fg_maxlen)
		maxpasslen = pr->sfld.fd_maxlen;
	else
		maxpasslen = 0;

	if (pr->uflg.fg_minlen)
		minpasslen = pr->ufld.fd_minlen;
	else if (pr->sflg.fg_minlen)
		minpasslen = pr->sfld.fd_minlen;
	else
		minpasslen = 0;

	if (pr->uflg.fg_lifetime)
		lifetime = pr->ufld.fd_lifetime;
	else if (pr->sflg.fg_lifetime)
		lifetime = pr->sfld.fd_lifetime;
	else
		lifetime = (time_t) 0;

	if (pr->uflg.fg_uchange)
		last_bad = pr->ufld.fd_uchange;
	else if (pr->sflg.fg_uchange)
		last_bad = pr->sfld.fd_uchange;
	else
		last_bad = (time_t) 0;

	if (pr->uflg.fg_schange)
		last_change = pr->ufld.fd_schange;
	else if (pr->sflg.fg_schange)
		last_change = pr->sfld.fd_schange;
	else
		last_change = (time_t) 0;

	if (pr->uflg.fg_min)
		min_time = pr->ufld.fd_min;
	else if (pr->sflg.fg_min)
		min_time = pr->sfld.fd_min;
	else
		min_time = (time_t) 0;

	if (pr->uflg.fg_nullpw)
		allownullpw = pr->ufld.fd_nullpw;
	else if (pr->sflg.fg_nullpw) 
		allownullpw = pr->sfld.fd_nullpw;

	if (!pr->uflg.fg_encrypt || pr->ufld.fd_encrypt[0] == '\0')
		old_cipher = NULL;
	else {
		old_cipher = malloc(strlen(pr->ufld.fd_encrypt) + 1);
		if (old_cipher == NULL) {
			passwd_no_change(pr->ufld.fd_name, pr,
			  GETMSG(MS_SIA_PWD, S_MEM_ALL_ERR, "Memory allocation error"));
			return(SIADFAIL);
		}
		strcpy(old_cipher, pr->ufld.fd_encrypt);
	}
	if (pr->uflg.fg_pwdepth)
		cipher_depth = pr->ufld.fd_pwdepth;
	else if (pr->sflg.fg_pwdepth)
		cipher_depth = pr->sfld.fd_pwdepth;
	else
		cipher_depth = 0;
	if (cipher_depth > AUTH_MAX_PASSWD_DICT_DEPTH)
		cipher_depth = AUTH_MAX_PASSWD_DICT_DEPTH;
	else if (cipher_depth < 0)
		cipher_depth = 0;
	if (cipher_depth) {
		register char *sp, **cp;
		register int i;
		cp = (char **) malloc(sizeof(char **)*(cipher_depth+1) +
			sizeof(pr->ufld.fd_pwdict));
		if (!cp) {
			passwd_no_change(pr->ufld.fd_name, pr,
			  GETMSG(MS_SIA_PWD, S_MEM_ALL_ERR,
			    "Memory allocation error"));
			free(old_cipher);
			return(SIADFAIL);
		}
		sp = (char *)(cp + cipher_depth + 1);
		cipher_hist = cp;		/* save the address */
		memmove(sp, pr->ufld.fd_pwdict, sizeof(pr->ufld.fd_pwdict));
		i = cipher_depth;
		if (!*sp) sp = (char *)0;	/* account for null history */
		while (i) {
			*cp = sp;
			i--;
			cp++;
			if (sp) sp = strchr(sp, ',');
			if (sp) *sp++ = '\0';
		}
		*cp = (char *)0;		/* NULL-terminated list */
	}

	return(SIADSUCCESS);
}

/*
 * Choose the new password according to the choices available to the
 * changer.  If there are multiple options, the changer is given a choice.
 */

choose_password()
{
	FILE *inp;
	char invalid_choice;
	char *line;
	char buf[1024];
	
	/*
	 * Open the user's controlling terminal.
	 */

	if (choices > 1) {

	    strcpy(buf, GETMSG(MS_SIA_PWD, S_WANT, "Do you want (choose one letter only):\n\n"));
	    if (can_do_random_word)
	      strcat(buf, GETMSG(MS_SIA_PWD, S_MENU_PWDGEN, "\tPronounceable passwords generated for you (g)\n"));
	
	    if (can_do_random_chars)
	      strcat(buf, GETMSG(MS_SIA_PWD, S_MENU_STRING1, "\tA string of characters generated for you (c)\n"));
	
	    if (can_do_random_letters)
	      strcat(buf, GETMSG(MS_SIA_PWD, S_MENU_STRING2, "\tA string of letters generated for you (l) ?\n"));
	
	    if (can_pick)
	      strcat(buf, GETMSG(MS_SIA_PWD, S_MENU_PICK1, "\tTo pick your password (p) ?\n\n"));
	
	    strcat(buf, GETMSG(MS_SIA_PWD, S_ENTER, "\nEnter choice here (q to quit): "));
	    invalid_choice = 1;
	    do {

		if ((line = putGetLine(buf, 256)) != (char *) 0
		    && !interrupted)
		    switch (line[0])  {
			case 'p':
			case 'P':
				if (can_pick) {
					save_choice = PASSWD_PICK;
					invalid_choice = 0;
				}
				break;
			case 'g':
			case 'G':
				if (can_do_random_word) {
					save_choice = PASSWD_WORD;
					invalid_choice = 0;
				}
				break;
			case 'c':
			case 'C':
				if (can_do_random_chars) {
					save_choice = PASSWD_CHARS;
					invalid_choice = 0;
				}
				break;
			case 'l':
			case 'L':
				if (can_do_random_letters) {
					save_choice = PASSWD_LETTERS;
					invalid_choice = 0;
				}
				break;
			case 'q':
				passwd_no_change(login_name, &account_entry,
				  GETMSG(MS_SIA_PWD, S_ABORT, "user aborted program"));
				exit(1);
				
			default:
				printf(GETMSG(MS_SIA_PWD, S_INVCH, "Invalid choice\n"));
				line[0] = '\0';
				break;
			}
		if (line && line[0] && invalid_choice && !interrupted)
			printf(GETMSG(MS_SIA_PWD, S_NOT_AUTH2, "You are not authorized for that method\n"));
	    } while (invalid_choice && !interrupted);
	}

	if (interrupted)
	  return SIADFAIL;

	if (save_choice == PASSWD_PICK)
	  return(passwd_pick());
	else
	  return(passwd_random(save_choice));
}

/*
 * Check that the password's lifetime has not expired and that the
 * minimum change time has elapsed.
 */

check_passwd_parameters()
{
	time_t now;

	if (old_cipher == NULL)
		return(SIASUCCESS);

	now = (time_t) time((time_t *) 0);

	/* Minimum change time */

	if (last_change != (time_t) 0 && last_change + min_time > now) {
		passwd_no_change(login_name, &account_entry,
				GETMSG(MS_SIA_PWD, S_MINIMUM, "minimum time between changes has not elapsed"));
		return(SIADFAIL);
	}

	/*
	 * Check to make sure we are within password lifetime.  When the
	 * last change is 0 (or not set), the Account Administrator has
	 * cleared a locked condition due to password expiration, so we
	 * allow changes then.
	 */

	if (lifetime && last_change && last_change + lifetime < now) {
		passwd_no_change(login_name, &account_entry,
				GETMSG(MS_SIA_PWD, S_LIFETIME, "password lifetime has passed"));
		return(SIADFAIL);
	}

	return(SIADSUCCESS);
}

/*
 * Validate that the user knows the old password, if required.
 */

validate_old_password()
{
	char *cleartext;
	char *ciphertext;

	/*
	 * You must know the old password if you are changing your own
	 * password and there was a password on the account.
	 */

	if (different_user || old_cipher == NULL)
		return (SIADSUCCESS);

	if (!security_is_on() && !login_uid)
		return (SIADSUCCESS);

	/*
	 * Request the old password and encrypt and compare it
	 */

	cleartext = c2_getpasswd(GETMSG(MS_SIA_PWD, S_OLD_PWD, "Old password:"), AUTH_MAX_PASSWD_LENGTH);

	if (interrupted)
		return(SIADFAIL);

	ciphertext = passwd_crypt(cleartext, old_cipher, old_alg);

	if (strcmp(ciphertext, old_cipher))  {
		passwd_no_change(login_name, &account_entry,
		  GETMSG(MS_SIA_PWD, S_UNK_OLD_PWD, "old password entered incorrectly"));
		c2_getpasswd(NULL, AUTH_MAX_PASSWD_LENGTH);
		return(SIADFAIL);
	}

	c2_getpasswd(NULL, AUTH_MAX_PASSWD_LENGTH);
	return(SIADSUCCESS);
}

/*
 * Request that the user pick a password
 */

static int
passwd_pick()
{
	char buffer[AUTH_MAX_PASSWD_LENGTH];
	int tries;
	char *retry_passwd = (char *) 0;
	char buf[256];

	for (tries = 0; tries < MAXTRIES; tries++) {
		strcpy(buffer, c2_getpasswd(GETMSG(MS_SIA_PWD, S_NEW_PWD, "New password:"), sizeof buffer));
		if (interrupted) {
			bzero(buffer, sizeof buffer);
			return SIADFAIL;
		}
		if (passwd_legal(buffer)) {
			retry_passwd = c2_getpasswd(GETMSG(MS_SIA_PWD, S_RE_NEW_PWD, "Re-enter new password:"),
			  sizeof buffer);
			
			if (interrupted) {
				bzero(buffer, sizeof buffer);
				if (retry_passwd)
					bzero(retry_passwd, strlen(retry_passwd));
				return SIADFAIL;
			}
			if (strcmp(retry_passwd, buffer) == 0) {
				bzero(buffer, sizeof buffer);
				strcpy(new_passwd, retry_passwd);
				c2_getpasswd(NULL, AUTH_MAX_PASSWD_LENGTH);
				return(SIADSUCCESS);
			}
			bzero(buffer, sizeof buffer);
			strcpy(buf, GETMSG(MS_SIA_PWD, S_DONT_MATCH1, "They don't match"));
		} else
			strcpy(buf, GETMSG(MS_SIA_PWD, S_ILLEGAL, "Illegal password"));
		if (tries < MAXTRIES - 1) {
		    	strcat(buf, GETMSG(MS_SIA_PWD, S_TRY_AGAIN, ", try again."));
		}
		show_error(buf);
	}
	if (retry_passwd)
	  bzero(retry_passwd, strlen(retry_passwd));
	
	c2_getpasswd(NULL, AUTH_MAX_PASSWD_LENGTH);

	passwd_no_change(login_name, &account_entry,
	  GETMSG(MS_SIA_PWD, S_TOOMANY, "too many unsuccessful attempts") );

	return(SIADFAIL);
}

/*
 * Generate a random password according to the choice requested (or required)
 */

static int
passwd_random(choice)
     int choice;
{
	register int loop = 1;
	char *newpass, buf[1024], *pC;
	long seed;
	int status;
	int alphabet_size;
	static char passwd [PASSWD_BUFSIZ];
	char hyphen [2*sizeof(passwd) - 1];
	char line [sizeof(passwd) + sizeof(hyphen) + sizeof(PASSWD_WORD_REQ)+9];

	strcpy(buf, GETMSG(MS_SIA_PWD, S_GENERATING, "\nGenerating random "));
		switch (choice)  {
			case PASSWD_WORD:
				strcat(buf, GETMSG(MS_SIA_PWD, S_PWD_PRON, "pronounceable password"));
				alphabet_size = 26;
				break;
			case PASSWD_CHARS:
				strcat(buf, GETMSG(MS_SIA_PWD, S_CHAR_STR, "string of characters"));
				alphabet_size = (int) '~' - (int) '!' + 1;
				break;
			case PASSWD_LETTERS:
				strcat(buf, GETMSG(MS_SIA_PWD, S_LETT_STR, "string of letters"));
				alphabet_size = (int) 'z' - (int) 'a' + 1;
				break;
		}
	pC = buf + strlen(buf);
	
	sprintf(pC, GETMSG(MS_SIA_PWD, S_FOR, " for %s."), account_entry.ufld.fd_name);
	show_mesg(buf);

	if (choice == PASSWD_WORD)
	show_mesg(GETMSG(MS_SIA_PWD, S_SHOWN, "The password, along with a hyphenated version, is shown."));
	if (restrict || prestrict)
		show_mesg(GETMSG(MS_SIA_PWD, S_SLOW, "\t(Password generation will be a bit slow.)"));
	show_mesg(GETMSG(MS_SIA_PWD, S_CHOICE, "Hit <RETURN> or <ENTER> until you like the choice."));
	show_mesg(GETMSG(MS_SIA_PWD, S_TYPE, "When you have chosen the password you want, type it in."));
	show_mesg(GETMSG(MS_SIA_PWD, S_NOTE, "Note: type your interrupt character or `%s' to abort at any time.\n"), GETMSG(MS_SIA_PWD, S_QUIT, "quit"));

	/*
	 * A zero lifetime duration means to have an infinite lifetime.
	 * In that case, the password length reverts to the maximum
	 * password length, since the calculation for minimum password
	 * length given an infinite lifetime would result in an infinite
	 * length!  Note that an infinite lifetime is not recommended in
	 * a secure system, because it allows time to guess the password.
	 */
#ifdef BROKEN_CODE	/* the passlen call returns -1 */
	if (lifetime == 0)
		minpasslen = maxpasslen;
	else
#endif /* BROKEN_CODE */
		minpasslen = max(minpasslen,
				 passlen(max(lifetime, 1), log_delay,
					 alphabet_size));

	/*
	 * Quietly handle cases where the Account Administrator picked a maximum
	 * password length too small for the password lifetime, or where
	 * the maximum password size has not been set.  Consider it an
	 * error when both the minimum and maximum password size are
	 * negligible.
	 */
	
	if (minpasslen > maxpasslen)
		maxpasslen = minpasslen;

	if ((maxpasslen == 0) || (maxpasslen >= sizeof(passwd))) {
		passwd_no_change(login_name, &account_entry,
		  GETMSG(MS_SIA_PWD, S_UNCOMP_PWD, "cannot compute password size") );
		return(SIADFAIL);
	}

	seed = get_seed(&account_entry);
	if (seed == -1L) {
		passwd_no_change(login_name, &account_entry,
		  GETMSG(MS_SIA_PWD, S_BAD_SEED, "bad seed"));
		return(SIADFAIL);
	}

	do  {
		switch (choice)  {
			case PASSWD_WORD:
				status = randomword(passwd, hyphen, minpasslen,
						    maxpasslen, restrict, seed);
				if (status < 0) {
					passwd_no_change(login_name,
							 &account_entry,
					  GETMSG(MS_SIA_PWD, S_UNCOMP_PWD,
					  "cannot compute password size") );
					return(SIADFAIL);
				}
				(void) sprintf(line, PASSWD_WORD_REQ,
						maxpasslen, passwd, hyphen);
				break;
			case PASSWD_CHARS:
				status = randomchars(passwd, minpasslen,
						     maxpasslen,
						     restrict, seed);
				if (status < 0) {
					passwd_no_change(login_name,
							 &account_entry,
					  GETMSG(MS_SIA_PWD, S_UNCOMP_PWD,
					  "cannot compute password size") );
					return(SIADFAIL);
				}
				(void) sprintf(line, PASSWD_CHAR_REQ,
						maxpasslen, passwd);
				break;
			case PASSWD_LETTERS:
				status = randomletters(passwd, minpasslen,
						       maxpasslen,
						       restrict, seed);
				if (status < 0) {
					passwd_no_change(login_name,
							 &account_entry,
					  GETMSG(MS_SIA_PWD, S_UNCOMP_PWD,
					  "cannot compute password size") );
					return(SIADFAIL);
				}
				(void) sprintf(line, PASSWD_LETT_REQ,
						maxpasslen, passwd);
				break;
		}

		/*
		 * Clear the hyphenated version of the password as soon
		 * as we no longer need it.
		 */
		bzero(hyphen, sizeof hyphen);

		if (prestrict && ask_pwpolicy(passwd))
			continue;	/* bad generation, try again */
		newpass = c2_getpasswd(line, maxpasslen);
		if (interrupted) {
			(void) c2_getpasswd((char *) 0, maxpasslen);
			return SIADFAIL;
		}
		if (strcmp(newpass, GETMSG(MS_SIA_PWD, S_QUIT, "quit")) == 0) {
			(void) c2_getpasswd((char *) 0, maxpasslen);
			passwd_no_change(login_name, &account_entry,
			  GETMSG(MS_SIA_PWD, S_PROG_STPD, "user stopped program") );
			return(SIAFAIL);
		}
		else if (strcmp(newpass, passwd) == 0)  {
			newpass = c2_getpasswd(GETMSG(MS_SIA_PWD, S_RE_NEW_PWD, "Re-enter new password:"),
					    maxpasslen);
			if (interrupted) {
				c2_getpasswd((char *) 0, maxpasslen);
				return SIADFAIL;
			}
			if (strcmp(newpass, passwd) == 0)
				loop = 0;
			else
				show_error(
				   GETMSG(MS_SIA_PWD, S_DONT_MATCH2, "They don't match; try again.\n"));

		}
		else
			if (newpass[0] != '\0')
				show_error(
				   GETMSG(MS_SIA_PWD, S_DONT_MATCH2, "They don't match; try again.\n"));

		(void) c2_getpasswd((char *) 0, maxpasslen);

		if (loop)
			audevent(GETMSG(MS_SIA_PWD, S_PICK2_USER_FORCED, 
		"pick of a random password was rejected or interrupted\nuser forced to pick again"), 1);
	}
	while (loop);

	strcpy(new_passwd, passwd);
	return(SIADSUCCESS);	
}

/*
 * Encrypt the new password and store it in the protected password
 * database, updating the required fields.  Audit the successful change.
 */

store_password()
{
	int	salt;
	int	c, i;
	char	saltc[2];
	register struct pr_passwd *pr = &account_entry;

	set_seed(get_seed(pr));
	salt = (int)(drand48() * (1<<12));
	saltc[0] = salt & 077;
	saltc[1] = (salt>>6) & 077;
	for (i = 0; i < 2; i++) {
		c = saltc[i] + '.';
		if (c > '9')
			c += 7;
		if (c > 'Z')
			c += 6;
		saltc[i] = c;
	}

	strcpy(pr->ufld.fd_encrypt, passwd_crypt(new_passwd, saltc, new_alg));
	pr->ufld.fd_oldcrypt = new_alg;
	pr->uflg.fg_oldcrypt = 1;

	if (pr->ufld.fd_encrypt[0] == '\0')
		pr->uflg.fg_encrypt = 0;
	else
		pr->uflg.fg_encrypt = 1;


	pr->uflg.fg_schange = 1;
	pr->ufld.fd_schange = (time_t) time((time_t *) 0);
	if (pr->ufld.fd_uid != (uid_t) login_uid) {
		pr->uflg.fg_pwchanger = 1;
		pr->ufld.fd_pwchanger = (uid_t) login_uid;
	} else
		pr->uflg.fg_pwchanger = 0;
#ifdef TMAC
	pr->uflg.fg_pw_admin_num = 0;
#endif

	if (cipher_depth) {
		register int i;
		register char **cp, *sp;
		cp = cipher_hist;
		if (old_cipher) {
			(void) strcpy(pr->ufld.fd_pwdict, old_cipher);
			i = cipher_depth - 1;
			sp = ",";
		}
		else {
			i = cipher_depth;
			sp = "";
		}
		while (*cp) {
			(void) strcat(pr->ufld.fd_pwdict, sp);
			(void) strcat(pr->ufld.fd_pwdict, *cp);
			cp++;
		}
		if (pr->ufld.fd_pwdict[0])
			pr->uflg.fg_pwdict = 1;
		else
			pr->uflg.fg_pwdict = 0;
	}

	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	(void) signal(SIGTSTP, SIG_IGN);

	RAISE_WRITE();
	if (!putprpwnam(account_name, pr))  {
		passwd_no_change(login_name, &account_entry,
		  GETMSG(MS_SIA_PWD, S_FAILED, "failed to write protected password entry"));
		return(SIADFAIL);
	}

	audevent("password successfully changed.",0);

	return(SIADSUCCESS);
}

/*
 * If a signal was received from the keyboard during a password change,
 * abort the program and audit the canceling.
 */

static void
abort_change()
{
	enter_quiet_zone();
	passwd_no_change(account_name, &account_entry, GETMSG(MS_SIA_PWD, S_PROG_STPD, "user stopped program"));
	interrupted = 1;
	exit_quiet_zone();
}

/*
 * Returns 1 if the password is acceptable and 0 if not.
 * When passwords are restricted, our own acceptability
 * tests are used.
 */
int
passwd_legal(password)
	char *password;
{
	if (password[0] == '\0') {	/* null password */
		if (!allownullpw) {
			show_error(
			    GETMSG(MS_SIA_PWD, S_NOT_ALLOWED, "You are not allowed to have a null password.\n"));
			return 0;
		}

		if (old_cipher == NULL) {
			show_error(GETMSG(MS_SIA_PWD, S_NULL_PWD, "You already have a null password.\n"));
			return 0;
		}

		return 1;
	}

	if (old_cipher &&
	    strcmp(old_cipher,
		   dispcrypt(password, old_cipher, old_alg)) == 0) {
		show_error(GETMSG(MS_SIA_PWD, S_NOT_RE_USE, "You may not re-use the same password.\n"));
		return 0;
	}

	if (cipher_depth) {
		register int i;
		register char **cp, *sp;
		for (cp = cipher_hist, i=0;  i < cipher_depth;  i++,cp++) {
			if (!(sp = *cp))
				break;
			if (strcmp(sp, bigcrypt(password, sp, old_alg)) == 0) {
				show_error(GETMSG(MS_SIA_PWD, S_NOT_RE_USE,
				"You may not re-use the same password.\n"));
				return 0;
			}
		}
	}

	if (prestrict && ask_pwpolicy(password))
		return 0;

	if (restrict && !acceptable_password(password, stderr))
		return 0;

	return 1;
}

/*
 * The password was not changed for some reason, whether it be because
 * the Protected Password database prevented it or because the user
 * aborted the program.
 */

passwd_no_change(name, pr, reason )
	register char *name;
	register struct pr_passwd *pr;
	register char *reason;
{
	time_t now;

	if (reason != (char *) 0)
		show_error(GETMSG(MS_SIA_PWD, S_PWD_CANT_CHG, "Password not changed: %s.\n"),
			reason);

	if ((name != (char *) 0) && (pr != (struct pr_passwd *) 0))  {
		now = (time_t) time((time_t *) 0);

		pr->uflg.fg_uchange = 1;
		pr->ufld.fd_uchange = now;

		RAISE_WRITE();
		if (!putprpwnam(pr->ufld.fd_name, pr))
			show_error(
			   GETMSG(MS_SIA_PWD, S_ERR_AUTH, "Authentication error; see Account Administrator\n"));

		audevent(name, 1);
	}
	return(SIADSUCCESS);
}

/*
 * Immediately after producing the new password, ensure that the cleartext
 * is erased.
 */

static char *
passwd_crypt(cleartext, salt, alg_idx)
	char *cleartext;
	char *salt;
	int alg_idx;
{
	char *ciphertext;

	if (cleartext[0] == '\0')
		return "";

	ciphertext = dispcrypt(cleartext, salt, alg_idx);
	bzero(cleartext, strlen(cleartext));

	return ciphertext;
}

int
updatePasswordFile(pwd)
     struct passwd *pwd;
{
	char buf[256], pwstr[1024];
	FILE	*tf;
	FILE 	*passfp;
	DBM	*dp;
	char	*str, *getpwline();
	int	i, acctlen, ch, fd, dochfn, dochsh;
	
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	(void) signal(SIGTSTP, SIG_IGN);
	(void) umask(0);

	if (forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_CHOWN, SEC_LIMIT,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0)) {
		show_error("insufficient privileges\n");
		return(SIADFAIL);
	}

	if ((i=create_file_securely(PTEMP, AUTH_VERBOSE, 0)) != CFS_GOOD_RETURN) {
	       show_error(GETMSG(MS_SIA_PWD, PWDFILEBUSY, "password file busy - try again.\n"));
		return(SIADFAIL);
	}

	if ((fd = open(PTEMP, O_WRONLY)) < 0) {
		sprintf(buf, GETMSG(MS_SIA_PWD, OPENFAIL, "open failed: %s "), PTEMP);
		show_perror(buf);
		return(SIADFAIL);
	}

	if ((tf = fdopen(fd, "w")) == NULL) {
		show_error(GETMSG(MS_SIA_PWD, FDOPNFAIL, "fdopen failed.\n"));
		return(SIADFAIL);
	}

	if ((dp = dbm_open(PASSWD, O_RDWR, 0644)) == NULL) {
		if(errno == ENOENT) {
			sprintf(buf, GETMSG(MS_SIA_PWD, NODB, "Hashed database not in use, only %s text file updated.\n"), PASSWD);
			show_error(buf);
		}
		else {
			sprintf(buf, GETMSG(MS_SIA_PWD, WARN1, "Warning: dbm_open failed: %s: "), PASSWD);
			show_perror(buf);
		}
	}
	else if (flock(dp->dbm_dirf, LOCK_EX) < 0) {
		show_perror(GETMSG(MS_SIA_PWD, WARN2, "Warning: lock failed"));
		dbm_close(dp);
		dp = NULL;
	}	

	unlimit(RLIMIT_CPU);
	unlimit(RLIMIT_FSIZE);
	/*****************Ultrix 4.2 /etc/passwd update***************
	 *
   	 * Fix from V4.2 Ultrix to maintain the (+,-,+:) YP syntax in the
	 * original password file. The code from OSF assumed that getpwents
	 * would provide all passwd file strings. Not true for YP syntax.
	 *
	 * Copy the original PASSWD "/etc/passwd" file substituting only
	 * the password to be modified
	 *************************************************************/
	 if ((passfp = fopen("/etc/passwd", "r")) == NULL) {
		show_perror(GETMSG(MS_SIA_PWD, WARN1, "Warning: can not open /etc/passwd\n"));
	}

	acctlen = strlen(pwd->pw_name);
	while(str=getpwline(pwstr, sizeof pwstr, passfp)) {
		i = strcspn(pwstr, ":\n");
		if(i == acctlen && !strncmp(pwd->pw_name, pwstr, i)) {
			if (pwd->pw_gecos[0] == '*')	/* ??? */
				pwd->pw_gecos++;
			replace(dp, pwd);
			fprintf(tf, "%s:%s:%d:%d:%s:%s:%s\n",
			pwd->pw_name,
			pwd->pw_passwd,
			pwd->pw_uid,
			pwd->pw_gid,
			pwd->pw_gecos,
			pwd->pw_dir,
			pwd->pw_shell);
		}
		else
			fputs(pwstr, tf);
	}
	endpwent();
	(void)fclose(passfp);
	if (dp && dbm_error(dp))
		show_error(GETMSG(MS_SIA_PWD, WARN3, "Warning: dbm_store failed.\n"));
	(void) fflush(tf);
	if (ferror(tf)) {
		sprintf(buf, GETMSG(MS_SIA_PWD, WARN4, "Warning: %s write error, %s not updated.\n"), PTEMP, PASSWD);
		show_error(buf);
		goto out;
	}
	(void)fclose(tf);
	if (dp != NULL)
		dbm_close(dp);
	if (rename(PTEMP, PASSWD) < 0) {
		show_perror("");
	out:
		(void)unlink(PTEMP);
		return(SIADFAIL);
	}
	return(SIADSUCCESS);
}

unlimit(lim)
	int	lim;
{
	struct rlimit rlim;

	rlim.rlim_cur = rlim.rlim_max = RLIM_INFINITY;
	(void)setrlimit(lim, &rlim);
}

/*
 * Replace the password entry in the dbm data base with pwd.
 */

replace(dp, pwd)
	DBM *dp;
	struct passwd *pwd;
{
	datum key, content;
	register char *cp, *tp;
	char buf[256];

	if (dp == NULL)
		return;

	cp = buf;
#define	COMPACT(e)	tp = pwd->e; while (*cp++ = *tp++);
	COMPACT(pw_name);
	COMPACT(pw_passwd);
	bcopy((char *)&pwd->pw_uid, cp, sizeof (int));
	cp += sizeof (int);
	bcopy((char *)&pwd->pw_gid, cp, sizeof (int));
	cp += sizeof (int);
	bcopy((char *)&pwd->pw_quota, cp, sizeof (int));
	cp += sizeof (int);
	COMPACT(pw_comment);
	COMPACT(pw_gecos);
	COMPACT(pw_dir);
	COMPACT(pw_shell);
	content.dptr = buf;
	content.dsize = cp - buf;
	key.dptr = pwd->pw_name;
	key.dsize = strlen(pwd->pw_name);
	dbm_store(dp, key, content, DBM_REPLACE);
	key.dptr = (char *)&pwd->pw_uid;
	key.dsize = sizeof (int);
	dbm_store(dp, key, content, DBM_REPLACE);
}

/*
 * Internal function to safely get a line from the named stream.  If
 * the line is too long to fit into the buffer it is thrown away and a
 * new line is fetched until successful or end-of-file.
 */
char *
getpwline(strng, len, file)
char *strng;
int len;
FILE *file;
{
	register char *s;
	int c;

	while(s=fgets(strng, len, file))
		if(strchr(strng, '\n'))
			break;
		else
			while((c=getc(file)) != EOF && c != '\n') ;
	return s;
}

/*
 * this routine gets a line from the SIA input source.  It expects
 * that the string returned does NOT have a '\n' stuck on the end, and
 * so adds a '\n' character itself (the default sia collect routine
 * uses gets(3) to get input, which swallows '\n's.
 */

char *
putGetLine(promptString, max_size)
     char *promptString;
     int max_size;
{
	static int once = 0;
	static prompt_t prompt;
	char *pC;
	
	check_auth_parameters();

	prompt.prompt = (u_char *) promptString;
	prompt.control_flags = SIAPRINTABLE;	
	prompt.min_result_length = 0;
	prompt.max_result_length = max_size;

	if (once++)
	  free(prompt.result);
	
	prompt.result = (u_char *) malloc(max_size);
	bzero((char *)prompt.result, max_size);
	  
	if ((*_c2_collect)(0, SIAONELINER, "", 1, &prompt) != SIASUCCESS)
	  interrupted++;

	if ((pC = strrchr((char *)prompt.result, '\0')))
	  strcpy(pC, "\n");	     /* add a null terminated newline */

	return((char *)prompt.result);
}

static char *
c2_getpasswd(promptString, max_size)
     char *promptString;
     int max_size;
{
	static prompt_t prompt;
	static int once = 0;

	if (!once)
	  once = 1;
	
	check_auth_parameters();

	if (!promptString)
	  {
	    if (once && prompt.result)
	      {
		/* erase the previous answer to the prompt string */
		(void) memset(prompt.result, '\0', strlen((char *)prompt.result));
		free(prompt.result);
	      }
	    return((char *)SIASUCCESS);
	  }
	
	prompt.prompt = (u_char *) promptString;
	prompt.control_flags = SIARESINVIS;	
	prompt.min_result_length = 0;
	prompt.max_result_length = max_size;
	prompt.result = (u_char *) malloc(max_size);
	
	if ((*_c2_collect)(0, SIAONELINER, "", 1, &prompt) != SIASUCCESS)
	  interrupted++;

	if (interrupted)
	  bzero((char *)prompt.result, max_size);

	return((char *)prompt.result);
}

static int
ask_pwpolicy(pw)
char *pw;
{
	int status, wstatus;
	int pfd[2];	/* for pipe() */
	int cpid;	/* for fork() */
	FILE *fp;	/* for ease of writing to the pipe */

	status = pipe(pfd);
	if (0 > status) {
		return(status);
	}
	cpid = fork();
	switch (cpid) {
	  case -1:
		/* report failure, dummy */
		(void) close(pfd[1]);
		(void) close(pfd[0]);
		return -1;

	  case 0:	/* in the child */
		(void) close(pfd[1]);	/* close writer's end */
		status = dup2(pfd[0], 0); /* try to make this be stdin */
		if (status < 0)
			_exit(-1);
		(void) close(1);
		(void) close(2);
		status = open(_PATH_DEVNULL, O_RDWR, 0);
		if (status != 1)
			_exit(-1);
		if (dup(1) != 2)
			_exit(-1);
		status = execl(pwpolicy, pwpolicy, NULL);
		_exit(status);
		break;
	  default:	/* parent, after successful fork */
		(void) close(pfd[0]);	/* close reader's end */
		fp = fdopen(pfd[1], "w");	/* try for stdin writes */
		if (!fp) {
			return -1;	/* resource error */
		}
		fprintf(fp, "%d\n%s\n%s\n", AUTH_PW_OKPASSWORD,
			account_name, pw);
		(void) fclose(fp);
		do {
			status = waitpid(cpid, &wstatus, 0);
		} while (status == -1 && errno == EINTR);
		if (WIFEXITED(wstatus))
			status = (int)(char)(WEXITSTATUS(wstatus));
		else
			status = -1;
		break;
	}
	return(status);
}

/* #endif */ /*} SEC_BASE */
