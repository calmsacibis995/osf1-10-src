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
static char	*sccsid = "@(#)$RCSfile: su_secure.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:55:27 $";
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

#if SEC_BASE
/* Copyright (c) 1988-1990, SecureWare, Inc.
 *   All rights reserved.
 */


/*
 * Based on:

 */

/*
 * This file is part of a library to make commands more secure.
 * This file contains those routines that are added to the
 * su command.
 *
 * Note that good/bad su attempts are not treated like login attempts
 * since the user who is attempting the su is already known to the system
 * and the SULOG keeps track of them.  Direct logins attempts can be
 * made by anyone, and those people are unaccountable to the system.
 * Also, with su, a known user could deny another user system access by
 * repeatedly trying the account if the su attempts were recorded in the
 * Protected Password database.  Another way of looking at it is that we
 * record in the Protected Password database cases involving the LUID, not just
 * the RUID.  Therefore, su does not have the same authentication update
 * requirements as login.  However, if the account is disabled for any
 * reason, we still need to obey that in su.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>

#include <sys/security.h>
#include <sys/audit.h>
#if SEC_MAC
#include <mandatory.h>
#endif
#include <prot.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "su_msg.h" 
nl_catd catd;
#define MSGSTR_SEC(n,s) catgets(catd,MS_SU_SEC,n,s) 
#else
#define MSGSTR_SEC(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#define	NO_WAY_TO_GET	"::"
#define OVERRIDE_USER   "root"
char *override_passwd = "OVERRIDE";
char auditbuf[80];

static struct pr_passwd desired_user;
static privvec_t no_sprivs;
#if SEC_MAC
static mand_ir_t *sec_level_ir = (mand_ir_t *) 0;
#endif

static void close_database();

extern long time();
extern char *ctime();
extern char *malloc();
extern priv_t *privvec();


/*
 * Ensure we should execute su as advertised.  If the user desired
 * is not one we can switch to as specified in the protected password
 * database, we swap the expected password with one that can never
 * be obtained.  (Ours has ':'s in it, which makes it impossible to
 * store in /etc/passwd!)   If we don't return with the password
 * being that of the protected password entry,
 * the security criteria have been met.
 *
 * Note that the current user is obtained from the LUID, not the RUID.
 * This ensures we deal with the true session owner.
 */
void
su_ensure_secure(pwd)
	register struct passwd *pwd;
{
	register struct pr_passwd *pr;
	register char *desired_name;
	register int uid;
	struct pr_passwd this_user;
	privvec_t saveprivs;

	if (checkprivs(privvec(SEC_SETPROCIDENT, SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
				-1))) {
		fprintf(stderr, MSGSTR_SEC(S_INSUFF_PRIV, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}

	/*
	 * Since getprpwuid() makes use of the static area referenced by
	 * pwd, we must re-establish the area referenced by pwd after all
	 * the getprpw* calls.  For now, we squirrel away the login name.
	 */
	desired_name = malloc(strlen(pwd->pw_name) + 1);
	if (desired_name == (char *) 0)  {
		fprintf(stderr, MSGSTR_SEC(S_NO_MEM, "No memory to save %s\n"), pwd->pw_name);
		exit(1);
	}
	strcpy(desired_name, pwd->pw_name);

	/*
	 * Get the information associated with the current user so we can
	 * see the rights of this user to become the requested user.
	 */
	forceprivs(privvec(SEC_ALLOWDACACCESS, -1), saveprivs);
	uid = getluid();
	if (uid >= 0)  {
		pr = getprpwuid(uid);
		if (pr == (struct pr_passwd *) 0) {
			audit_subsystem(
				MSGSTR_SEC(S_GET_PROT, "get protected password entry of current user"), 				MSGSTR_SEC(S_ABORT1, "user ID unknown, abort su"), ET_OBJECT_UNAV);
			fprintf(stderr, MSGSTR_SEC(S_UNKNOWN_UID, "su: Your own ID is unknown.\n"));
			exit(1);
		}
		this_user = *pr;
	}
	else {
		fprintf(stderr, MSGSTR_SEC(S_NO_FROM_RC, "su: may not be called from rc script\n"));
		exit(1);
	}

	/*
	 * Get the protected password entry for the requested user.
	 * Treat the absence of the name in EITHER database as an error,
	 * as well as a non-match between the two.
	 */
	pr = getprpwnam(desired_name);
	if (pr == (struct pr_passwd *) 0)  {
		audit_subsystem(
			MSGSTR_SEC(S_GET_PROT1, "get protected password entry for requested user"), 
			MSGSTR_SEC(S_ABORT2, "entry not found, abort su"), ET_OBJECT_UNAV);
		fprintf(stderr, MSGSTR_SEC(S_UNKNOWN1, "su: Unknown id: %s\n"), desired_name);
		exit(1);
	}
	desired_user = *pr;
	endprpwent();
	endprdfent();

	seteffprivs(saveprivs, (priv_t *) 0);

	pwd = getpwnam(desired_name);
	if (pwd == (struct passwd *) 0) {
		audit_subsystem(MSGSTR_SEC(S_GET_ENTRY, "get /etc/passwd entry for requested user"), 
			MSGSTR_SEC(S_ABORT2, "entry not found, abort su"), ET_OBJECT_UNAV);
		fprintf(stderr, MSGSTR_SEC(S_UNKNOWN1, "su: Unknown id: %s\n"), desired_name);
		exit(1);
	}


	if (!pr->uflg.fg_uid || !pr->uflg.fg_name ||
	    (pr->ufld.fd_uid != pwd->pw_uid) ||
	    (strcmp(desired_name, pwd->pw_name) != 0) ||
	    (strcmp(pr->ufld.fd_name, pwd->pw_name) != 0))  {
		audit_subsystem(
			MSGSTR_SEC(S_VERIFY_NAME, "verify name, id fields for requested user match in protected password and /etc/password"), MSGSTR_SEC(S_ABORT3, "abort su"), ET_SUBSYSTEM);
		fprintf(stderr, MSGSTR_SEC(S_UNKNOWN1, "su: Unknown id: %s\n"), desired_name);
		exit(1);
	}

	/*
	 * Only allow su to a user with either this login user
	 * as the owner, or to a user with the same owner as this
	 * login user.  This allows a user in a session to su to his
	 * heart's content, but only to users designating him
	 * as the owner.  This provides individual
	 * accountability to each user in the system.
	 *
	 * Denials to the account produce the same message
	 * as a bad login.  To do this, we simply set a bogus
	 * encrypted password that cannot be matched by any attempt.
	 */
	else if (pr->uflg.fg_owner)  {
		if ((this_user.uflg.fg_name &&
		     (strcmp(pr->ufld.fd_owner,
			    this_user.ufld.fd_name) == 0)) ||
		    (this_user.uflg.fg_owner &&
		     (strcmp(pr->ufld.fd_owner,
			     this_user.ufld.fd_owner) == 0)))

			/*
			 * Have the su program use the protected password, not
			 * the password in /etc/passwd.
			 */
			pwd->pw_passwd = pr->ufld.fd_encrypt;
		else
			pwd->pw_passwd = NO_WAY_TO_GET;
		
	}

	/*
	 * Also handle the trivial case where we su to the same user.
	 */
	else if (this_user.uflg.fg_uid &&
		 (this_user.ufld.fd_uid == pwd->pw_uid))
		pwd->pw_passwd = "";

	/*
	 * All other cases are no good.
	 */
	else
		pwd->pw_passwd = NO_WAY_TO_GET;


	free(desired_name);
}


/*
 * Obtain the cleartext password and use it to determine if the ciphertext
 * is the same as stored in the password file.  If it is valid, return 1,
 * otherwise return 0.  In all cases, once the ciphertext is determined,
 * immediately clear the cleartext.
 */
int
su_proper_password(pwd)
	register struct passwd *pwd;
{
	register char *cleartext;
	register int is_proper;

	cleartext = getpasswd(MSGSTR_SEC(S_ENTRY_PWD, "Password:"), AUTH_MAX_PASSWD_LENGTH);

	is_proper =
	    (strcmp(pwd->pw_passwd, bigcrypt(cleartext, pwd->pw_passwd)) == 0);

	getpasswd((char *) 0, AUTH_MAX_PASSWD_LENGTH);

	return is_proper;
}

#ifndef _OSF_SOURCE /*{*/
/*
 * Make sure the log file has the right mode.  If it does not already
 * exist, try to create it.
 */
void
su_check_sulog(log_name, root)
	register char *log_name;
	int root;
{
	char purpose[100];
	int cfs_status ;
	privvec_t	saveprivs;

#ifdef lint
	root = 0;
#endif
        strcpy(purpose,MSGSTR_SEC(S_REC_ATTEMPT, "record attempt at identity change"));

	if (forceprivs(privvec(SEC_OWNER, SEC_CHOWN, SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR_SEC(S_INSUFF_PRIV, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}

	if (access(log_name, 02) != 0)  {
		unlink(log_name);
		cfs_status = create_file_securely(log_name,
					AUTH_VERBOSE, purpose);
		if (cfs_status != CFS_GOOD_RETURN) {
			sprintf(auditbuf, MSGSTR_SEC(S_CREATE, "create_file_securely() returned: %d for %s"), cfs_status, log_name);
			audit_security_failure(OT_REGULAR, auditbuf,
				MSGSTR_SEC(S_ALLOWED, "su allowed to continue"), ET_OBJECT_UNAV);
			fprintf(stderr,
			  MSGSTR_SEC(S_CREATE1, "su: create_file_securely() returned: %d\n"),
			  cfs_status) ;
		}
	}

	seteffprivs(saveprivs, (priv_t *) 0);
}
#endif /*} _OSF_SOURCE */


/*
 * Make sure that the account is
 * not locked.  Note we do this after the password is obtained to
 * make sure that the real user of the account gets the locked/no locked
 * status.  Also, let the user know when he last logged in successfully
 * and his last unsuccessful attempt. The command "su -" is not a real
 * login, and the last login attempts do not reflect any su attempts.
 *
 * Locking is done by the Account Administrator unconditionally, or by
 * exceeding the number of bad consecutive attempts or by having the
 * password expire.
 */
void
su_ensure_admission(fake_login, log_name)
	int fake_login;
	char *log_name;
{
	register struct pr_passwd *pr;
	register time_t expiration;
	register time_t last_change;
	register int expired;
	register time_t now;
	time_t last_login;
	time_t last_bad;
	char *ptime;
#if defined(M_XENIX) && defined(INTL)
	struct tm *localtime(), *ptm;
	char buffer[128];
#endif

	pr = &desired_user;


	now = time ((long *) 0);
	
	/*
	 * Check for account expiration.
	 * If there is no field, there is no set end of life/expire
	 * for the account.  Also, if the password is null, no
	 * expiration check is performed.
	 */
	if (pr->uflg.fg_encrypt && pr->ufld.fd_encrypt[0] != '\0') {

		if (pr->uflg.fg_schange)
			last_change = pr->ufld.fd_schange;
		else if (pr->sflg.fg_schange)
			last_change = pr->sfld.fd_schange;
		else
			last_change = (time_t) 0;

		if (pr->uflg.fg_expire)
			expiration = pr->ufld.fd_expire;
		else if (pr->sflg.fg_expire)
			expiration = pr->sfld.fd_expire;
		else
			expiration = (time_t) 0;

		expired = ((expiration != (time_t) 0) &&
			   (last_change + expiration < now));

		if (expired)  {
#ifndef _OSF_SOURCE
			if (log_name != (char *) 0)
				log(log_name, pr->ufld.fd_name, 0);
#endif
			audit_subsystem(
			    MSGSTR_SEC(S_CHECK_COND, "check if any condition will prevent su"), 
			    MSGSTR_SEC(S_PWD_EXP, "password expired, abort su"),
			    ET_ACCESS_DENIAL);
			fflush(stdout);
			fprintf(stderr, MSGSTR_SEC(S_HAS_EXP, "Password has expired.\n"));
			fprintf(stderr, MSGSTR_SEC(S_SET_NEW_PWD, "Setting a new password for it cannot be done from here.\n"));
			exit(2);
		}
	}

	/*
	 * Now, see if any other condition will prevent an su to the account.
	 */

	if (locked_out(pr))  {
#ifndef _OSF_SOURCE
		if (log_name != (char *) 0)
			log(log_name, pr->ufld.fd_name, 0);
#endif
		audit_subsystem(MSGSTR_SEC(S_CHECK_ACC, "check for disabled account"), 
                  MSGSTR_SEC(S_ABORT4, "account disabled, abort su"), ET_ACCESS_DENIAL);
		fprintf(stderr,
		  MSGSTR_SEC(S_SEE_ADM1, "Account is disabled -- see Account Administrator.\n"));
		exit(2);
	}

	/* check for retired account */

	if (pr->uflg.fg_retired && pr->ufld.fd_retired) {
		audit_subsystem(MSGSTR_SEC(S_CHECK1, "check for retired account"), 
			MSGSTR_SEC(S_ABORT5, "account retired, abort su"), ET_ACCESS_DENIAL);
		fprintf(stderr,
		 MSGSTR_SEC(S_NO_LOG, "Account has been retired -- logins are no longer allowed.\n"));
		exit(2);
	}

	if (time_lock(pr))  {
#ifndef _OSF_SOURCE
		if (log_name != (char *) 0)
			log(log_name, pr->ufld.fd_name, 0);
#endif
		fprintf(stderr,
		  MSGSTR_SEC(S_WRONG_TIME, "Wrong time period to use this account.\n"));
		exit(2);
	}

	/*
	 * By here, the su will occur, so if we are doing a "su -", print
	 * out the login time information.
	 */
#ifndef _OSF_SOURCE
	/*
	 *  First, log the success if there is a log file.
	 */
	if (log_name != (char *) 0)
		log(log_name, pr->ufld.fd_name, 1);
#endif
	if (fake_login)  {
		if (pr->uflg.fg_ulogin)
			last_bad = pr->ufld.fd_ulogin;
		else
			last_bad = (time_t) 0;
	
		if (pr->uflg.fg_slogin)
			last_login = pr->ufld.fd_slogin;
		else
			last_login = (time_t) 0;
	
		fflush(stdout);
		if (last_login == 0)
			ptime = MSGSTR_SEC(S_NEVER, "NEVER");
		else {
#if defined(M_XENIX) && defined(INTL)
			ptm = localtime(&last_login);
			strftime(buffer, sizeof(buffer), "%c", ptm);
			ptime = buffer;
#else
			ptime = ctime(&last_login);
			/* strip newline */
			ptime[strlen(ptime)-1] = '\0';
#endif
		}
		fprintf(stderr, MSGSTR_SEC(S_LAST_SUCC, "Last   successful real login for %s: %s"),
		  pr->ufld.fd_name, ptime);
		if (pr->uflg.fg_suctty)
			fprintf(stderr, MSGSTR_SEC(S_ON, " on %s"), pr->ufld.fd_suctty);
		putc ('\n', stderr);

		if (last_bad == 0)
			ptime = MSGSTR_SEC(S_NEVER, "NEVER");
		else {
#if defined(M_XENIX) && defined(INTL)
			ptm = localtime(&last_bad);
			strftime(buffer, sizeof(buffer), "%c", ptm);
			ptime = buffer;
#else
			ptime = ctime(&last_bad);
			/* strip newline */
			ptime[strlen(ptime)-1] = '\0';
#endif
		}
		fprintf(stderr, MSGSTR_SEC(S_LAST_UNSUCC, "Last unsuccessful real login for %s: %s"),
		pr->ufld.fd_name, ptime);
		if (pr->uflg.fg_unsuctty)
			fprintf(stderr, MSGSTR_SEC(S_ON, " on %s"), pr->ufld.fd_unsuctty);
		putc ('\n', stderr);
		fflush(stderr);
	}
}


/*
 * Close all the database files before we exec so they don't get inherited
 * by the child process.
 */
static void
close_database()
{
	endprpwent();
	endprfient();
	endprdfent();
	endpwent();
	endgrent();
}


/*
 * Since setuid() and setgid() identity changes are not allowed until the
 * login user is set, we try to set the login user here so that su can be
 * used in /etc/rc and daemons before identity changes occur.  If we have
 * already set the user, we ignore the inevitable error.  We also set
 * the privileges that belong to both the current and new user.
 *
 * Note: this must be called after su_ensure_secure(), which sets
 *	 desired_user.
 */
void
su_set_login(luid)
	int luid;
{
	register mask_t *new_sysprivs;
	register int scan_privs;
	register struct pr_default *prdf;
	register struct pr_term *prtc;
	register mask_t *new_baseprivs;
	privvec_t curr_baseprivs, curr_sysprivs;
	privvec_t saveprivs;
#if SEC_MAC
	int relation;
	mand_ir_t *clearance_ir, *clearance, *sensitivity;
#endif

	/*
	 * The setluid() call requires the SEC_SETPROCIDENT privilege.
	 */
	forceprivs(privvec(SEC_SETPROCIDENT, -1), saveprivs);

	/*
	 * This call will only work from a daemon.  If su is invoked
	 * from an existing session, this call will not succeed.
	 * If we are from a daemon, set the audit mask for the user.
	 */
	if (setluid((ushort) luid) == 0)
		audit_adjust_mask(&desired_user);

	seteffprivs(saveprivs, (priv_t *) 0);

#if SEC_MAC
	/*
 	 * Set the clearance whenever transitioning to another user.
	 * Do this after setluid() so audit records will reflect the
	 * user.  The clearance can only be reset when the new one is
	 * dominated by the current one.  Failure of setclrnce() is not
	 * necessarily bad.  If the new clearance dominates the current one,
	 * the clearance is not changed;  there is no error.  Other
	 * reasons for failure are all grouped as errors.
	 *
	 * The call to mand_init is needed here because we are about to
	 * reference globals from mandlib.  Normally, it would have already
	 * been called implicitly as a result of parsing the protected
	 * password file, but we can't rely on this since it is possible
	 * for both the system defaults database and the protected password
	 * file to not contain any labels (as in the system initialization
	 * case).
	 */
	mand_init();
#if SEC_SHW
	clearance = mand_syshi;
#else
	if (desired_user.uflg.fg_clearance)
		clearance = &desired_user.ufld.fd_clearance;
	else if (desired_user.sflg.fg_clearance)
		clearance = &desired_user.sfld.fd_clearance;
	else
#if SEC_CMW
		clearance = mand_minclrnce;
#else
		clearance = mand_syslo;
#endif
#endif
	if (setclrnce(clearance) == -1)  {
#if !SEC_SHW
		clearance_ir = mand_alloc_ir();
		if ((clearance_ir == (mand_ir_t *) 0) ||
		    (getclrnce(clearance_ir) != 0))  {
			audit_subsystem(MSGSTR_SEC(S_GET_CLEAR, "get clearance for current user"), 
				MSGSTR_SEC(S_ABORT6, "cannot get clearance, abort su"),ET_SUBSYSTEM);
			fprintf(stderr, MSGSTR_SEC(S_CANT_GET_CL, "Cannot get your clearance.\n"));
			exit(1);
		}

		/*
		 * This examines the relation between the current clearance
		 * and the one that could not be set, to determine if an error
		 * occurred or the clearance is alright as is.
		 */
		forceprivs(privvec(SEC_ALLOWMACACCESS, -1), saveprivs);
		relation = mand_ir_relationship(clearance, clearance_ir);
		seteffprivs(saveprivs, (priv_t *) 0);
		mand_free_ir(clearance_ir);

		if ((relation & (MAND_EQUAL | MAND_SDOM)) == 0) {
			audit_subsystem(
				MSGSTR_SEC(S_ALTER_CL, "alter clearance to that of requested user"), 
				MSGSTR_SEC(S_CANT_GET_USRCL, "cannot set user's clearance"), ET_SEC_LEVEL);
			fprintf(stderr, MSGSTR_SEC(S_CANT_SET1, "Cannot set user's clearance.\n"));

			fprintf(stderr,
				MSGSTR_SEC(S_FAIL_REASON, "Reason for failure to set clearance:\n\t"));
			if (relation & MAND_ODOM)
				fprintf(stderr,
  				   MSGSTR_SEC(S_CURRENT, "Current one dominates the new user's.\n"));
			else if (relation & MAND_INCOMP)
				fprintf(stderr,
  			MSGSTR_SEC(S_CURRENT1, "Current one and the new user's are incomparable.\n"));
			else
				fprintf(stderr, MSGSTR_SEC(S_UNKNOWN2, "Unknown.\n"));

			exit(1);
		}
#endif
	}

#endif

	/*
	 * Set the system privilege mask for this process.  Do this after
	 * setting all other identity parameters for the user because
	 * this will turn off self-audit and the audit subsystem will now
	 * expect the login process to be established COMPLETELY for the user.
	 *
	 * The privilege mask is the anding of the current user mask and
	 * the user we su to.  This is to always ensure that someone does
	 * not try to bypass privilege settings for a user by su'ing rather
	 * than logging in.
	 */
	if (desired_user.uflg.fg_sprivs)
		new_sysprivs = desired_user.ufld.fd_sprivs;
	else if (desired_user.sflg.fg_sprivs)
		new_sysprivs = desired_user.sfld.fd_sprivs;
	else
		new_sysprivs = no_sprivs;

	if (getpriv(SEC_MAXIMUM_PRIV, curr_sysprivs) < 0)  {
		audit_subsystem(MSGSTR_SEC(S_RETR_KERN, "retrieve kernel authorizations"), 
			MSGSTR_SEC(S_CANT_RETR, "cannot retrieve"), ET_SUBSYSTEM);
		fprintf(stderr, MSGSTR_SEC(S_CANT_RETR_KERN, "Cannot retrieve kernel authorizations.\n"));
		exit(1);
	}

	/*
	 * Get the base privileges for the new user.
	 *
	 * The base privilege mask is the anding of the current user mask and
	 * the user we su to.  This is to combine the base security of both
	 * the old and new users.
	 */
	if (desired_user.uflg.fg_bprivs)
		new_baseprivs = desired_user.ufld.fd_bprivs;
	else if (desired_user.sflg.fg_bprivs)
		new_baseprivs = desired_user.sfld.fd_bprivs;
	else
		new_baseprivs = no_sprivs;

	if (getpriv(SEC_BASE_PRIV, curr_baseprivs) < 0)  {
		audit_subsystem(MSGSTR_SEC(S_RETR_BASE, "retrieve base privileges"), 
			MSGSTR_SEC(S_CANT_RETR, "cannot retrieve"), ET_SUBSYSTEM);
		fprintf(stderr, MSGSTR_SEC(S_CANT_RETR_BASE, "Cannot retrieve base privileges.\n"));
		exit(1);
	}

	for (scan_privs = 0;  scan_privs <= SEC_SPRIVVEC_SIZE; scan_privs++) {
		new_baseprivs[scan_privs] &= curr_baseprivs[scan_privs];
		new_sysprivs[scan_privs] &= curr_sysprivs[scan_privs];
	}

	close_database();

	if (setsysauths(new_sysprivs) < 0)  {
		audit_subsystem(MSGSTR_SEC(S_SET_RESTR, "set restricted kernel authorizations in su"),
                        MSGSTR_SEC(S_CANT_RESTR_KERN, "could not restrict kernel authorizations"),
                        ET_SUBSYSTEM);
		fprintf(stderr,
			MSGSTR_SEC(S_UNABLE, "Unable to set account's kernel authorizations.\n"));
		exit(1);
	}

	if (setbaseprivs(new_baseprivs) < 0)  {
		audit_subsystem(MSGSTR_SEC(S_SET_RESTR1, "set restricted base privileges in su"),
                        MSGSTR_SEC(S_NO_RESTR_BASE, "could not restrict base privileges"),
                        ET_SUBSYSTEM);
		fprintf(stderr, MSGSTR_SEC(S_UNABLE2, "Unable to set account's base privileges.\n"));
		exit(1);
	}

	/*
	 * This is to perform the actual identity change coming up next.
	 */
	forcepriv(SEC_SETPROCIDENT);
}

#endif
