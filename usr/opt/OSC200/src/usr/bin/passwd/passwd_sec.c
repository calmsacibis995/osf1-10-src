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
static char	*sccsid = "@(#)$RCSfile: passwd_sec.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:45:07 $";
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


/*
 * Based on:

 */

#include <sys/secdefines.h>

#if SEC_BASE /*{*/

/*
 * Program that changes the password by enforcing the values in the
 * protected password database.  The program assumes that it is SGID to
 * a group that can read the protected passsword database files and
 * write into the protected password directories.  It is least privileged to
 * run with protected password files protected at System High.
 */

#include <sys/types.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <mandatory.h>
#include <sys/signal.h>
#include <prot.h>
#include <pwd.h>
#include <stdio.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "passwd_msg.h" 
nl_catd catd;
#define MSGSTR_SEC(n,s) catgets(catd,MS_PASSWD_SEC,n,s) 
#else
#define MSGSTR_SEC(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif


extern priv_t *privvec();
extern char *malloc();
extern char *ctime();
static void abort_change();
static char *passwd_crypt();
static void passwd_no_change();

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
int maxpasslen;		/* Maximum password length for the new password */
time_t lifetime;	/* password lifetime */
time_t log_delay;	/* Delay between login attempts (system-wide) */
time_t last_change;	/* Last successful change */
time_t last_bad;	/* Last unsuccessful change */
time_t min_time;	/* Minimum change time */
char allownullpw;	/* Are null passwords allowed on this account? */
char different_user;	/* Is the user changing another's account? */
char *old_cipher;	/* Ciphertext of old password */

/* Password generation options: */
int can_pick;
int can_do_random_word;
int can_do_random_chars;
int can_do_random_letters;
int choices;		/* Number of password generation choices */
int save_choice;	/* Of only one choice, the number allowed */

int login_uid;		/* Login user ID of user who invoked program */
char *login_name;	/* Name of user login_uid */
struct pr_passwd account_entry;	/* Protected password entry of account */
char *account_name;	/* User name of account whose password to be changed */
uid_t account_uid;	/* User ID of account whose password to be changed */
char *new_passwd;	/* New password chosen or generated */

#define	CANCEL		MSGSTR_SEC(S_QUIT, "quit")

#define	PASSWD_WORD_REQ	MSGSTR_SEC(S_ENT1, "Password: %-*s  Hyphenation: %s\nEnter password:")

#define	PASSWD_CHAR_REQ	MSGSTR_SEC(S_ENT2, "Password: %-*s   Enter password:")
#define	PASSWD_LETT_REQ	MSGSTR_SEC(S_ENT2, "Password: %-*s   Enter password:")
 
#define	PASSWD_PICK	1
#define	PASSWD_WORD	2
#define	PASSWD_CHARS	3
#define	PASSWD_LETTERS	4

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
		fprintf(stderr, MSGSTR_SEC(S_INSUFF_PRIV, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}

	/*
	 * luid is negative when it has not been set yet.
	 */
	login_uid = getluid();
	if (login_uid < 0)
		reason = MSGSTR_SEC(S_ID_NOT_SET, "Login user ID not set yet");
	else {
#if SEC_MAC
		/* Process must not have a wildcard sensitivity label */
		
		mand_ir_t *mand_ir;

		mand_ir = mand_alloc_ir();
		if (mand_ir == (mand_ir_t *) 0) {
			reason = MSGSTR_SEC(S_MEM_ALL_ERR, "Memory allocation error");
			goto bad;
		}
		if (getslabel(mand_ir) < 0) {
			reason = MSGSTR_SEC(S_LBL_NOT_SET, "Sensitivity label not set");
			goto bad;
		}
#endif
		RAISE_READ();
		p = getpwuid(login_uid);
		LOWER_READ();
		if (p == (struct passwd *) 0) {
			sprintf(buffer,
		MSGSTR_SEC(S_NO_ENTRY_PWD, "No /etc/passwd entry for user ID %d"),
			  login_uid);
			reason = buffer;
		} else {
			login_name = malloc(strlen(p->pw_name) + 1);
			if (login_name == NULL)
				reason = MSGSTR_SEC(S_MEM_ALL_ERR, "Memory allocation error");
			else
				strcpy(login_name, p->pw_name);
		}
	}

bad:
	if (reason != NULL)
		audit_security_failure(OT_PWD, reason,
		 MSGSTR_SEC(S_PWD_FAIL, "password change failure"), ET_SUBSYSTEM);
	return login_name;
}

/*
 * Check whether the user associated with uid is authorized to
 * modify the password (and shell and finger data, if supported)
 * of the user named by the /etc/passwd entry pwd.  Call passwd_no_change()
 * (never returns) if there is an authorization or database problem.
 */

void
passwd_auth(pwd, uid)
	struct passwd *pwd;
	uid_t uid;
{
	register struct pr_passwd *ppwd;

	account_name = malloc(strlen(pwd->pw_name) + 1);
	if (account_name == NULL)
		passwd_no_change(pwd->pw_name, (struct pr_passwd *) 0,
		  MSGSTR_SEC(S_MEM_ALL_ERR, "Memory allocation error"), ES_PW_CHANGE_FAILED);
	strcpy(account_name, pwd->pw_name);
	account_uid = pwd->pw_uid;

	RAISE_READ();
	ppwd = getprpwnam(pwd->pw_name);
	LOWER_READ();
	if (ppwd == (struct pr_passwd *) 0)
		passwd_no_change(pwd->pw_name, (struct pr_passwd *) 0,
		  MSGSTR_SEC(S_PROT_PWD, "cannot access protected password entry"),
		  ES_PW_CHANGE_FAILED);

	if (!ppwd->uflg.fg_uid || ppwd->ufld.fd_uid != account_uid)
		passwd_no_change(account_name, ppwd,
		  MSGSTR_SEC(S_MISMATCH, "ID mismatch"),
		  ES_PW_CHANGE_FAILED);

	if (ppwd->ufld.fd_uid != uid) {
		different_user = 1;
		if (!authorized_user("password"))
			passwd_no_change(ppwd->ufld.fd_name, ppwd,
			  MSGSTR_SEC(S_NOT_AUTH1, "not authorized to change another user's password"),
			  ES_PW_CHANGE_NOPRIV);
	}

	account_entry = *ppwd;

	return;
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

	/*
	 * Get the default login delay for terminals.
	 */

	df = getprdfnam(AUTH_DEFAULT);
	if (df == (struct pr_default *) 0)
		passwd_no_change(login_name, &account_entry,
		  MSGSTR_SEC(S_CANT_RETR, "cannot access system default database"),
		  ES_PW_CHANGE_FAILED);

	if (df->tcg.fg_logdelay)
		log_delay = df->tcd.fd_logdelay;
	else
		log_delay = (time_t) 1;
	/*
	 * Store the relevant fields from the account being changed.
	 * If the changer is different, retrieve that user's entry
	 * and store relevant fields from that account as well.
	 */

	ppwd = &account_entry;
	store_changee_values(ppwd);
	if (different_user) {
		RAISE_READ();
		ppwd = getprpwuid(login_uid);
		LOWER_READ();
		if (ppwd == (struct pr_passwd *) 0)
			passwd_no_change(login_name, &account_entry,
	MSGSTR_SEC(S_FAIL_RETR, "cannot access your protected password entry"),
			  ES_PW_CHANGE_FAILED);
	}
	store_changer_values(ppwd);

	/* Check that the user has at least one password changing option */

	if (choices == 0)
		passwd_no_change(login_name, &account_entry,
		  MSGSTR_SEC(S_NOOPT, "you do not have any password changing options"),
		  ES_PW_CHANGE_FAILED);

	if (different_user)
		ppwd = &account_entry;

	/* set up to audit user aborting password change */

	(void) signal(SIGINT, abort_change);
	(void) signal(SIGHUP, abort_change);
	(void) signal(SIGQUIT, abort_change);

	/* If necessary, validate that the changer knows the old password. */

	validate_old_password();

	/* Print the last successful and unsuccessful change times */

	(void) fflush(stdout);
	fprintf(stderr, MSGSTR_SEC(S_LAST_SUCC, "Last successful password change for %s: %s"), 
		account_entry.ufld.fd_name,
		(last_change == (time_t) 0) ? MSGSTR_SEC(S_NEVER, "NEVER\n"): ctime(&last_change));
	fprintf(stderr, MSGSTR_SEC(S_LAST_UNSUCC, "Last unsuccessful password change for %s: %s\n"),
		account_entry.ufld.fd_name,
		(last_bad == (time_t) 0) ? MSGSTR_SEC(S_NEVER, "NEVER\n"): ctime(&last_bad));
	(void) fflush(stderr);

	/* Check that the password falls between min change and lifetime */

	check_passwd_parameters();

	/* Choose the new password by whichever methods are allowed. */
		
	choose_password();

	/* Record the fact that the password was successfully changed */

	store_password();
}

/*
 * Store the relevant values from the changer's protected password entry
 */

store_changer_values(pr)
	struct pr_passwd *pr;
{
	if (pr->uflg.fg_restrict)
		restrict = pr->ufld.fd_restrict;
	else if (pr->sflg.fg_restrict)
		restrict = pr->sfld.fd_restrict;
	else
		restrict = 1;

	if (pr->uflg.fg_pick_pwd)
		can_pick = pr->ufld.fd_pick_pwd;
	else if (pr->sflg.fg_pick_pwd)
		can_pick = pr->sfld.fd_pick_pwd;
	if (can_pick) {
		choices++;
		save_choice = PASSWD_PICK;
	}

	if (pr->uflg.fg_gen_pwd)
		can_do_random_word = pr->ufld.fd_gen_pwd;
	else if (pr->sflg.fg_gen_pwd)
		can_do_random_word = pr->sfld.fd_gen_pwd;
	if (can_do_random_word) {
		choices++;
		save_choice = PASSWD_WORD;
	}

	if (pr->uflg.fg_gen_chars)
		can_do_random_chars = pr->ufld.fd_gen_chars;
	else if (pr->sflg.fg_gen_chars)
		can_do_random_chars = pr->sfld.fd_gen_chars;
	if (can_do_random_chars) {
		choices++;
		save_choice = PASSWD_CHARS;
	}

	if (pr->uflg.fg_gen_letters)
		can_do_random_letters = pr->ufld.fd_gen_letters;
	else if (pr->sflg.fg_gen_letters)
		can_do_random_letters = pr->sfld.fd_gen_letters;
	if (can_do_random_letters) {
		choices++;
		save_choice = PASSWD_LETTERS;
	}

}

/*
 * Store the relevant values from the changee's protected password entry
 */

store_changee_values(pr)
	struct pr_passwd *pr;
{

	if (pr->uflg.fg_maxlen)
		maxpasslen = pr->ufld.fd_maxlen;
	else if (pr->sflg.fg_maxlen)
		maxpasslen = pr->sfld.fd_maxlen;
	else
		maxpasslen = (time_t) 0;

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
		if (old_cipher == NULL)
			passwd_no_change(pr->ufld.fd_name, pr,
			  MSGSTR_SEC(S_MEM_ALL_ERR, "Memory allocation error"), ES_PW_CHANGE_FAILED);
		strcpy(old_cipher, pr->ufld.fd_encrypt);
	}
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
		return 0;

	/*
	 * Request the old password and encrypt and compare it
	 */

	cleartext = getpasswd(MSGSTR_SEC(S_OLD_PWD, "Old password:"), AUTH_MAX_PASSWD_LENGTH);

	ciphertext = passwd_crypt(cleartext, old_cipher);

	if (strcmp(ciphertext, old_cipher))  {
		passwd_no_change(login_name, &account_entry,
		  MSGSTR_SEC(S_UNK_OLD_PWD, "old password entered incorrectly"), ES_PW_CHANGE_FAILED);
	}
}

/*
 * Choose the new password according to the choices available to the
 * changer.  If there are multiple options, the changer is given a choice.
 */

choose_password()
{
	FILE *inp;
	char invalid_choice;
	char line[10];

	/*
	 * Open the user's controlling terminal.
	 */

	inp = fopen("/dev/tty", "r");
	if(inp == (FILE *) 0)  {
		audit_security_failure(OT_DEVICE,
			MSGSTR_SEC(S_OPN_CNTL_TERM, "opening control terminal"),
			MSGSTR_SEC(S_ABORT_PWD, "abort password change"), ET_RES_DENIAL);
		fprintf(stderr,
		  MSGSTR_SEC(S_CANT_OPN, "Cannot open your control terminal\n"));
		  exit(1);
	}
	else
		setbuf(inp, (char*) 0);

	if (choices > 1) {

	    invalid_choice = 1;
	    do {
		printf(MSGSTR_SEC(S_WANT, "Do you want (choose one letter only):\n"));
		if (can_do_random_word)
		    printf(MSGSTR_SEC(S_PWDGEN, "\tpronounceable passwords generated for you (g)\n"));
		if (can_do_random_chars)
		    printf(MSGSTR_SEC(S_STRING1, "\ta string of characters generated (c) ?\n"));
		if (can_do_random_letters)
		    printf(MSGSTR_SEC(S_STRING2, "\ta string of letters generated (l) ?\n"));
		if (can_pick)
		    printf(MSGSTR_SEC(S_PICK1, "\tto pick your passwords (p) ?\n\n"));
		printf(MSGSTR_SEC(S_ENTER, "Enter choice here (q to quit): "));
		fflush(stdout);
		if (fgets(line, sizeof(line), inp) != (char *) 0)
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
				  MSGSTR_SEC(S_ABORT, "user aborted program"),
				  ES_PW_CHANGE_FAILED);
			default:
				printf(MSGSTR_SEC(S_INVCH, "Invalid choice\n"));
				line[0] = '\0';
				break;
			}
		if (line[0] && invalid_choice)
			printf(MSGSTR_SEC(S_NOT_AUTH2, "You are not authorized for that method\n"));
	    } while (invalid_choice);
	}
	if (save_choice == PASSWD_PICK)
		passwd_pick();
	else
		passwd_random(save_choice);
	(void) fclose(inp);
}

/*
 * Check that the password's lifetime has not expired and that the
 * minimum change time has elapsed.
 */

check_passwd_parameters()
{
	time_t now;

	if (old_cipher == NULL)
		return;

	now = (time_t) time((long *) 0);

	/* Minimum change time */

	if (last_change != (time_t) 0 && last_change + min_time > now)
		passwd_no_change(login_name, &account_entry,
				MSGSTR_SEC(S_MINIMUM, "minimum time between changes has not elapsed"),
				ES_PW_CHANGE_FAILED);

	/*
	 * Check to make sure we are within password lifetime.  When the
	 * last change is 0 (or not set), the Account Administrator has
	 * cleared a locked condition due to password expiration, so we
	 * allow changes then.
	 */

	if (lifetime && last_change && last_change + lifetime < now)
		passwd_no_change(login_name, &account_entry,
				MSGSTR_SEC(S_LIFETIME, "password lifetime has passed"),
				 ES_PW_CHANGE_FAILED);
}

/*
 * Request that the user pick a password
 */

passwd_pick()
{
	char buffer[AUTH_MAX_PASSWD_LENGTH];
	int tries;
	char *retry_passwd;

	for (tries = 0; tries < MAXTRIES; tries++) {
		strcpy(buffer, getpasswd(MSGSTR_SEC(S_NEW_PWD, "New password:"), sizeof buffer));
		if (passwd_legal(buffer)) {
			retry_passwd = getpasswd(MSGSTR_SEC(S_RE_NEW_PWD, "Re-enter new password:"),
			  sizeof buffer);
			if (strcmp(retry_passwd, buffer) == 0) {
				bzero(buffer, sizeof buffer);
				new_passwd = retry_passwd;
				return;
			}
			bzero(buffer, sizeof buffer);
			fprintf(stderr, MSGSTR_SEC(S_DONT_MATCH1, "They don't match"), command_name);
		} else
			fprintf(stderr, MSGSTR_SEC(S_ILLEGAL, "Illegal password"), command_name);
		fprintf(stderr, MSGSTR_SEC(S_TRY_AGAIN, ", try again.\n"));
	}
	bzero(retry_passwd, strlen(retry_passwd));
	passwd_no_change(login_name, &account_entry,
	  MSGSTR_SEC(S_TOOMANY, "too many unsuccessful attempts"), ES_PW_CHANGE_FAILED);
	/* never returns */
}


/*
 * Generate a random password according to the choice requested (or required)
 */

passwd_random(choice)
	int choice;
{
	register int loop = 1;
	char *newpass;
	long seed;
	int alphabet_size;
	static char passwd [BUFSIZ];
	char hyphen [2*sizeof(passwd) - 1];
	char line [sizeof(passwd) + sizeof(hyphen) + sizeof(PASSWD_WORD_REQ)+9];
	int minpasslen;

	(void) fflush(stderr);
	printf(MSGSTR_SEC(S_GENERATING, "\nGenerating random "));
		switch (choice)  {
			case PASSWD_WORD:
				printf(MSGSTR_SEC(S_PWD_PRON, "pronounceable password"));
				alphabet_size = 26;
				break;
			case PASSWD_CHARS:
				printf(MSGSTR_SEC(S_CHAR_STR, "string of characters"));
				alphabet_size = (int) '~' - (int) '!' + 1;
				break;
			case PASSWD_LETTERS:
				printf(MSGSTR_SEC(S_LETT_STR, "string of letters"));
				alphabet_size = (int) 'z' - (int) 'a' + 1;
				break;
		}
	printf(MSGSTR_SEC(S_FOR, " for %s.\n"), account_entry.ufld.fd_name);
	printf(MSGSTR_SEC(S_SHOWN, "The password, along with a hyphenated version, is shown.\n"));
	if (restrict)
		printf(MSGSTR_SEC(S_SLOW, "\t(Password generation will be a bit slow.)\n"));
	printf(MSGSTR_SEC(S_CHOICE, "Hit <RETURN> or <ENTER> until you like the choice.\n"));
	printf(MSGSTR_SEC(S_TYPE, "When you have chosen the password you want, type it in.\n"));
	printf(MSGSTR_SEC(S_NOTE, "Note: type your interrupt character or `%s' to abort at any time.\n\n"), CANCEL);
	(void) fflush(stdout);

	/*
	 * A zero lifetime duration means to have an infinite lifetime.
	 * In that case, the password length reverts to the maximum
	 * password length, since the calculation for minimum password
	 * length given an infinite lifetime would result in an infinite
	 * length!  Note that an infinite lifetime is not recommended in
	 * a secure system, because it allows time to guess the password.
	 */
	if (lifetime == 0)
		minpasslen = maxpasslen;
	else
		minpasslen = passlen(max(lifetime, 1), log_delay,
				     alphabet_size);

	/*
	 * Quietly handle cases where the Account Administrator picked a maximum
	 * password length too small for the password lifetime, or where
	 * the maximum password size has not been set.  Consider it an
	 * error when both the minimum and maximum password size are
	 * negligible.
	 */
	if (minpasslen > maxpasslen)
		maxpasslen = minpasslen;
	else if (minpasslen == 0)
		minpasslen = maxpasslen;

	if ((minpasslen == 0) || (maxpasslen >= sizeof(passwd)))
		passwd_no_change(login_name, &account_entry,
		  MSGSTR_SEC(S_UNCOMP_PWD, "cannot compute password size"), ES_PW_CHANGE_FAILED);


	seed = get_seed(&account_entry);
	if (seed == -1L)
		passwd_no_change(login_name, &account_entry,
		  MSGSTR_SEC(S_BAD_SEED, "bad seed"), ES_PW_CHANGE_FAILED);

	do  {
		switch (choice)  {
			case PASSWD_WORD:
				(void) randomword(passwd, hyphen, minpasslen,
						  maxpasslen, restrict, seed);
				(void) sprintf(line, PASSWD_WORD_REQ,
						maxpasslen, passwd, hyphen);
				break;
			case PASSWD_CHARS:
				(void) randomchars(passwd, minpasslen,
						   maxpasslen, restrict, seed);
				(void) sprintf(line, PASSWD_CHAR_REQ,
						maxpasslen, passwd);
				break;
			case PASSWD_LETTERS:
				(void) randomletters(passwd, minpasslen,
						    maxpasslen, restrict, seed);
				(void) sprintf(line, PASSWD_LETT_REQ,
						maxpasslen, passwd);
				break;
		}

		/*
		 * Clear the hyphenated version of the password as soon
		 * as we no longer need it.
		 */
		bzero(hyphen, sizeof hyphen);

		newpass = getpasswd(line, maxpasslen);
		if (strcmp(newpass, CANCEL) == 0)  {
			(void) getpasswd((char *) 0, maxpasslen);
			passwd_no_change(login_name, &account_entry,
			  MSGSTR_SEC(S_PROG_STPD, "user stopped program"), ES_PW_CHANGE_FAILED);
		}
		else if (strcmp(newpass, passwd) == 0)  {
			newpass = getpasswd(MSGSTR_SEC(S_RE_NEW_PWD, "Re-enter new password:"),
					    maxpasslen);
			if (strcmp(newpass, passwd) == 0)
				loop = 0;
			else
				fprintf (stderr,
				   MSGSTR_SEC(S_DONT_MATCH2, "They don't match; try again.\n"));

		}
		else
			if (newpass[0] != '\0')
				fprintf (stderr,
				   MSGSTR_SEC(S_DONT_MATCH2, "They don't match; try again.\n"));

		(void) getpasswd((char *) 0, maxpasslen);

		if (loop)
			audit_security_failure(OT_PROCESS, 
			MSGSTR_SEC(S_PICK2, "pick of a random password was rejected or interrupted"),
			MSGSTR_SEC(S_USER_FORCED, "user forced to pick again"), ET_SUBSYSTEM);
	}
	while (loop);

	new_passwd = passwd;
}

/*
 * Encrypt the new password and store it in the protected password
 * database, updating the required fields.  Audit the successful change.
 */

store_password()
{
	time_t	salt;
	int	c, i;
	char	saltc[2];
	register struct pr_passwd *pr = &account_entry;

	(void)time(&salt);
	salt = 9 * getpid();
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

	strcpy(pr->ufld.fd_encrypt, bigcrypt(new_passwd, saltc));
	bzero(new_passwd, strlen(new_passwd));

	if (pr->ufld.fd_encrypt[0] == '\0')
		pr->uflg.fg_encrypt = 0;
	else
		pr->uflg.fg_encrypt = 1;


	pr->uflg.fg_schange = 1;
	pr->ufld.fd_schange = (time_t) time((long *) 0);
	if (pr->ufld.fd_uid != (ushort) login_uid) {
		pr->uflg.fg_pwchanger = 1;
		pr->ufld.fd_pwchanger = (ushort) login_uid;
	} else
		pr->uflg.fg_pwchanger = 0;
#ifdef TMAC
	pr->uflg.fg_pw_admin_num = 0;
#endif

	RAISE_WRITE();
	if (!putprpwnam(account_name, pr))  {
		passwd_no_change(login_name, &account_entry,
		  MSGSTR_SEC(S_FAILED, "failed to write protected password entry"),
		  ES_PW_CHANGE_FAILED);
	}

	audit_passwd(account_name, ES_PW_CHANGE, ET_SUBSYSTEM);

	exit(0);
}

/*
 * If a signal was received from the keyboard during a password change,
 * abort the program and audit the canceling.
 */

static void
abort_change()
{
	enter_quiet_zone();
	passwd_no_change(account_name, &account_entry, MSGSTR_SEC(S_PROG_STPD, "user stopped program"),
			 ES_PW_CHANGE_FAILED);
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
		char allownullpw = 0;

		if (!allownullpw) {
			fprintf(stderr,
			    MSGSTR_SEC(S_NOT_ALLOWED, "You are not allowed to have a null password.\n"));
			return 0;
		}

		if (old_cipher == NULL) {
			fprintf(stderr, MSGSTR_SEC(S_NULL_PWD, "You already have a null password.\n"));
			return 0;
		}

		return 1;
	}

	if (old_cipher &&
	    strcmp(old_cipher, bigcrypt(password, old_cipher)) == 0) {
		fprintf(stderr, MSGSTR_SEC(S_NOT_RE_USE, "You may not re-use the same password.\n"));
		return 0;
	}

	if (restrict && !acceptable_password(password, stderr))
		return 0;

	return 1;
}

/*
 * The password was not changed for some reason, whether it be because
 * the Protected Password database prevented it or because the user
 * aborted the program.
 */
static void
passwd_no_change(name, pr, reason, audit_code)
	register char *name;
	register struct pr_passwd *pr;
	register char *reason;
	int audit_code;
{
	time_t now;

	if (reason != (char *) 0)
#ifdef M_XENIX
		fprintf(stderr, MSGSTR_SEC(S_DENIED_REQ, "\nPassword request denied.\nReason: %s.\n"),
			reason);
#else
		fprintf(stderr, MSGSTR_SEC(S_PWD_CANT_CHG, "Password not changed: %s.\n"),
			reason);
#endif

	if ((name != (char *) 0) && (pr != (struct pr_passwd *) 0))  {
		now = (time_t) time((long *) 0);

		pr->uflg.fg_uchange = 1;
		pr->ufld.fd_uchange = now;

		RAISE_WRITE();
		if (!putprpwnam(name, pr))
			fprintf (stderr,
			   MSGSTR_SEC(S_ERR_AUTH, "Authentication error; see Account Administrator\n"));

		audit_passwd(name, audit_code, ET_SUBSYSTEM);
	}
	exit(1);
}

/*
 * Immediately after producing the new password, ensure that the cleartext
 * is erased.
 */

static char *
passwd_crypt(cleartext, salt)
	char *cleartext;
	char *salt;
{
	char *ciphertext;

	if (cleartext[0] == '\0')
		return "";

	ciphertext = bigcrypt(cleartext, salt);
	bzero(cleartext, strlen(cleartext));

	return ciphertext;
}


#endif /*} SEC_BASE */
