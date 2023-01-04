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
static char *rcsid = "@(#)$RCSfile: privileges.c,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/04/08 19:21:56 $";
#endif
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 */

#ident "@(#)privileges.c	6.3 14:29:16 2/28/91 SecureWare"
/*
 * Based on:
 *   "@(#)privileges.c	2.5.1.3 11:57:31 2/3/90 SecureWare"
 */

/*
 * This Module contains Proprietary Information of SecureWare, Inc. and
 * should be treated as Confidential.
 */

/*LINTLIBRARY*/

/*
 * This file contains routines to be used together in handling the
 * checking and manipulation of system authorizations and privileges.
 * For efficiency, the privilege vectors are cached locally.  All
 * functions in this module that modify the vectors maintain the
 * integrity of the cached copies.  However, direct manipulation of
 * the kernel vectors using the setpriv syscall can cause the local
 * copies to become stale.
 */

#include <sys/secdefines.h>

#include <sys/types.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include <errno.h>
#include <varargs.h>

extern int		errno;
extern struct namepair	sys_priv[];

extern char		*malloc(), *realloc();

#define	PRIVSTR_INCR	80

static privvec_t kernel_auths;
static privvec_t effective_privs;
static privvec_t potential_privs;
static privvec_t base_privs;
static char privs_inited = 0;
static char security_is_ON;
#if !SEC_PRIV
static char is_superuser;
#endif

/*
 * Initialize the local copies of the process privilege state.
 */
static void
privs_initialize()
{
	getpriv(SEC_MAXIMUM_PRIV, kernel_auths);
	getpriv(SEC_POTENTIAL_PRIV, potential_privs);
	getpriv(SEC_BASE_PRIV, base_privs);
	getpriv(SEC_EFFECTIVE_PRIV, effective_privs);
#if !SEC_PRIV
	/*
	 * On systems without file privilege sets, trusted programs are
	 * setuid to root or setgid to sec so that they acquire potential 
	 * privileges upon execution. 
	 * Revert the effective uid to the real uid, and the
	 * effective gid to the real gid so that
	 * privilege is controlled by the effective privilege vector.
	 */
	if (!privs_inited) {
		is_superuser = (starting_ruid() == 0);
		if (security_is_ON = (char)security_is_on()) {
			setuid(starting_ruid());
			setgid(starting_rgid());
		}
	}
#endif
	privs_inited = 1;
}


/*
 * Returns 1 if the process has the system authorization and 0 if not or the
 * system authorizations could not be retrieved.
 */
int
hassysauth(auth)
	int auth;
{
	if (!privs_inited)
		privs_initialize();

#if SEC_PRIV
	return ISBITSET(kernel_auths, auth);
#else
	return ISBITSET(kernel_auths, auth) || is_superuser;
#endif
}


/*
 * Initialize the privilege library.
 */
void
initprivs()
{
	if (!privs_inited)
		privs_initialize();

	/*
	 * If the program is self-auditing, turn on the
	 * privilege now.
	 */
	if (ISBITSET(potential_privs, SEC_SUSPEND_AUDIT))
		forcepriv(SEC_SUSPEND_AUDIT);
}


/*
 * Turn off a system authorization.  This action is irrevocable.
 * Since dropping an authorization can effect the base and effective
 * privilege vector, both vectors are updated.
 * Returns 1 if the operation succeeds, 0 otherwise.
 */
int
disablesysauth(priv)
	int priv;
{
	int	failed = 0;

	if (!privs_inited)
		privs_initialize();
	if (!security_is_ON)
		return 1;

again:
	if (ISBITSET(kernel_auths, priv))  {

		RMBIT(kernel_auths, priv);
		if (setpriv(SEC_MAXIMUM_PRIV, kernel_auths) < 0) {
			if (failed) {
				ADDBIT(kernel_auths, priv);
				return 0;
			}
			privs_initialize();
			failed = 1;
			goto again;
		}
		getpriv(SEC_BASE_PRIV, base_privs);
		getpriv(SEC_EFFECTIVE_PRIV, effective_privs);
	}

	return 1;
}


/*
 * Turn on a privilege if the process has the authorization for it.
 * The routine will fail if the privilege is not in either the base
 * privilege set or the program's potential set.
 */
int
enablepriv(priv)
	int priv;
{
	int	failed = 0;

	if (!privs_inited)
		privs_initialize();
	if (!security_is_ON)
		return 1;

	/* If privilege is already in the effective set then return 1 */
	if (ISBITSET(effective_privs, priv))
		return 1;
	
again:
#if SEC_PRIV
	if (!ISBITSET(kernel_auths, priv))
#else
	if (!ISBITSET(kernel_auths, priv) && !is_superuser)
#endif
		return 0;

	if (!ISBITSET(base_privs, priv) && !ISBITSET(potential_privs, priv))
		return 0;

	if (!ISBITSET(effective_privs, priv))  {

		ADDBIT(effective_privs, priv);
		if (setpriv(SEC_EFFECTIVE_PRIV, effective_privs) < 0) {
			if (failed) {
				RMBIT(effective_privs, priv);
				return 0;
			}
			privs_initialize();
			failed = 1;
			goto again;
		}
	}

	return 1;
}


/*
 * Turn on a privilege even if there is no authorization for it (if possible).
 * The routine will fail if the privilege is not in either the base
 * privilege set or the program's potential set.
 */
int
forcepriv(priv)
	int priv;
{
	int	failed = 0;

	if (!privs_inited)
		privs_initialize();
	if (!security_is_ON)
		return 1;

again:
	if (!ISBITSET(base_privs, priv) && !ISBITSET(potential_privs, priv))
		return 0;

	if (!ISBITSET(effective_privs, priv))  {

		ADDBIT(effective_privs, priv);
		if (setpriv(SEC_EFFECTIVE_PRIV, effective_privs) < 0) {
			if (failed) {
				RMBIT(effective_privs, priv);
				return 0;
			}
			privs_initialize();
			failed = 1;
			goto again;
		}
	}

	return 1;
}


/*
 * Turn off a privilege.
 */
int
disablepriv(priv)
	int priv;
{
	int failed = 0;

	if (!privs_inited)
		privs_initialize();
	if (!security_is_ON)
		return 1;

again:
	if (ISBITSET(effective_privs, priv))  {
		RMBIT(effective_privs, priv);
		if (setpriv(SEC_EFFECTIVE_PRIV, effective_privs) < 0) {
			if (failed) {
				ADDBIT(effective_privs, priv);
				return 0;
			}
			privs_initialize();
			failed = 1;
			goto again;
		}
	}

	return 1;
}


/*
 * Determine if the user has a set of kernel authorizations.  If all
 * requested authorizations are present, return a NULL pointer, else
 * return a pointer to a vector of missing authorizations.
 */
priv_t *
checksysauths(vec)
	privvec_t	vec;
{
	static privvec_t	missing;
	register int		i;
	register int		haveall = 1;

	if (!privs_inited)
		privs_initialize();

#if !SEC_PRIV
	if (is_superuser)
		return (priv_t *) 0;
#endif

	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
		if (missing[i] = (vec[i] & ~kernel_auths[i]))
			haveall = 0;
	return haveall ? (priv_t *) 0 : missing;
}


/*
 * Determine if the process can enable a set of privileges.  If all
 * can be enabled, return a NULL pointer, else return a pointer to a
 * vector of those that can't be enabled.
 */
priv_t *
checkprivs(vec)
	privvec_t	vec;
{
	static privvec_t	missing;
	register int		i;
	register int		haveall = 1;

	if (!privs_inited)
		privs_initialize();

	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
		missing[i] = vec[i] & ~(base_privs[i] | potential_privs[i]);

		if (missing[i])
			haveall = 0;
	}
	return haveall ? (priv_t *) 0 : missing;
}


/*
 * Add a set of privileges masked by the user's kernel authorizations
 * to the process's effective privilege set and optionally return the
 * previous effective privileges.
 */
int
enableprivs(add, prev)
	privvec_t	add;
	priv_t		*prev;
{
	privvec_t		old, new;
	register int		i;
	register int		dosyscall;
	register int		failed = 0;

	if (!privs_inited)
		privs_initialize();
	if (!security_is_ON)
		return 0;

again:
	dosyscall = 0;
	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
		old[i] = effective_privs[i];
#if !SEC_PRIV
		if (is_superuser)
			new[i] = effective_privs[i] | add[i];
		else
#endif
		new[i] = effective_privs[i] | add[i] & kernel_auths[i];
		if (new[i] & ~(base_privs[i] | potential_privs[i])) {
			errno = EPERM;
			return -1;
		}
		if (new[i] != old[i])
			dosyscall = 1;
	}
	if (dosyscall) {
		if (setpriv(SEC_EFFECTIVE_PRIV, new) == -1) {
			if (failed)
				return -1;
			/*
			 * Resynchronize local copies and retry.
			 */
			privs_initialize();
			failed = 1;
			goto again;
		}
		memcpy(effective_privs, new, sizeof(privvec_t));
	}
	if (prev)
		memcpy(prev, old, sizeof(privvec_t));
	return 0;
}


/*
 * Add a set of privileges to the process's effective privilege set
 * and optionally return the previous effective privileges.
 */
int
forceprivs(add, prev)
	privvec_t	add;
	priv_t		*prev;
{
	privvec_t		old, new;
	register int		i;
	register int		dosyscall;
	register int		failed = 0;

	if (!privs_inited)
		privs_initialize();
	if (!security_is_ON)
		return 0;
	
again:
	dosyscall = 0;
	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
		old[i] = effective_privs[i];
		new[i] = effective_privs[i] | add[i];
		if (new[i] & ~(base_privs[i] | potential_privs[i])) {
			errno = EPERM;
			return -1;
		}
		if (new[i] != old[i])
			dosyscall = 1;
	}
	if (dosyscall) {
		if (setpriv(SEC_EFFECTIVE_PRIV, new) == -1) {
			if (failed)
				return -1;
			/*
			 * Resynchronize local copies and retry.
			 */
			privs_initialize();
			failed = 1;
			goto again;
		}
		memcpy(effective_privs, new, sizeof(privvec_t));
	}
	if (prev)
		memcpy(prev, old, sizeof(privvec_t));
	return 0;
}


/*
 * Remove a set of privileges from the process's effective privilege set
 * and optionally return the previous effective privileges.
 */
int
disableprivs(sub, prev)
	privvec_t	sub;
	priv_t		*prev;
{
	privvec_t		old, new;
	register int		i;
	register int		dosyscall;
	register int		failed;

	if (!privs_inited)
		privs_initialize();
	if (!security_is_ON)
		return 0;
	
again:
	dosyscall = 0;
	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
		old[i] = effective_privs[i];
		new[i] = effective_privs[i] & ~sub[i];
		if (new[i] != old[i])
			dosyscall = 1;
	}
	if (dosyscall) {
		if (setpriv(SEC_EFFECTIVE_PRIV, new) == -1) {
			if (failed)
				return -1;
			/*
			 * Resynchronize local copies and retry.
			 */
			privs_initialize();
			failed = 1;
			goto again;
		}
		memcpy(effective_privs, new, sizeof(privvec_t));
	}
	if (prev)
		memcpy(prev, old, sizeof(privvec_t));
	return 0;
}


/*
 * Set the processes effective privileges and optionally return the
 * previous value.
 */
int
seteffprivs(new, prev)
	privvec_t	new;
	priv_t		*prev;
{
	privvec_t		old;
	register int		i;
	register int		dosyscall;
	register int		failed = 0;

	if (!privs_inited)
		privs_initialize();
	if (!security_is_ON)
		return 0;
	
again:
	dosyscall = 0;
	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
		old[i] = effective_privs[i];
		if (new[i] & ~(base_privs[i] | potential_privs[i])) {
			errno = EPERM;
			return -1;
		}
		if (new[i] != old[i])
			dosyscall = 1;
	}
	if (dosyscall) {
		if (setpriv(SEC_EFFECTIVE_PRIV, new) == -1) {
			if (failed)
				return -1;
			/*
			 * Resynchronize local copies and retry.
			 */
			privs_initialize();
			failed = 1;
			goto again;
		}
		memcpy(effective_privs, new, sizeof(privvec_t));
	}
	if (prev)
		memcpy(prev, old, sizeof(privvec_t));
	return 0;
}


/*
 * Set the process's base privileges.  Update effective.
 */
int
setbaseprivs(new)
	privvec_t	new;
{
	privvec_t		old;
	register int		i;
	register int		dosyscall;
	register int		failed = 0;

	if (!privs_inited)
		privs_initialize();
	if (!security_is_ON)
		return 0;
	
again:
	dosyscall = 0;
	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
		if (new[i] & ~((kernel_auths[i] & potential_privs[i]) |
					base_privs[i])) {
			errno = EPERM;
			return -1;
		}
		if (new[i] != base_privs[i])
			dosyscall = 1;
	}

	if (dosyscall) {
		if (setpriv(SEC_BASE_PRIV, new) == -1) {
			if (failed)
				return -1;
			/*
			 * Resynchronize local copies and retry.
			 */
			privs_initialize();
			failed = 1;
			goto again;
		}
		memcpy(base_privs, new, sizeof(privvec_t));
		getpriv(SEC_EFFECTIVE_PRIV, effective_privs);
	}
	return 0;
}


/*
 * Set the process's kernel authorizations. Update base and effective privs.
 */
int
setsysauths(new)
	privvec_t	new;
{
	privvec_t		old;
	register int		i;
	register int		dosyscall;
	register int		failed = 0;

	if (!privs_inited)
		privs_initialize();
	if (!security_is_ON)
		return 0;
	
again:
	dosyscall = 0;
	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
		if (new[i] & ~kernel_auths[i]) {
			errno = EPERM;
			return -1;
		}
		if (new[i] != kernel_auths[i])
			dosyscall = 1;
	}

	if (dosyscall) {
		if (setpriv(SEC_MAXIMUM_PRIV, new) == -1) {
			if (failed)
				return -1;
			/*
			 * Resynchronize local copies and retry.
			 */
			privs_initialize();
			failed = 1;
			goto again;
		}
		memcpy(kernel_auths, new, sizeof(privvec_t));
		getpriv(SEC_BASE_PRIV, base_privs);
		getpriv(SEC_EFFECTIVE_PRIV, effective_privs);
	}
	return 0;
}


/*
 * Construct a privilege vector from a variable number of privilege numbers.
 * Return a pointer to the constructed vector.
 */
priv_t *
privvec(va_alist)
	va_dcl
{
	static privvec_t	vec;
	register int		priv;
	va_list			ap;

	memset(vec, 0, sizeof vec);

	va_start(ap);
	while ((priv = va_arg(ap, int)) >= 0 && priv <= SEC_MAX_SPRIV)
		ADDBIT(vec, priv);
	va_end(ap);

	return vec;
}


/*
 * Construct a privilege vector from a variable number of privilege numbers.
 * Store the result in the supplied vector.
 */
void
setprivvec(vec, va_alist)
	privvec_t	vec;
	va_dcl
{
	register int	priv;
	va_list		ap;

	memset(vec, 0, sizeof vec);

	va_start(ap);
	while ((priv = va_arg(ap, int)) >= 0 && priv <= SEC_MAX_SPRIV)
		ADDBIT(vec, priv);
	va_end(ap);
}


/*
 * Convert a privilege number into a privilege name.
 * Return a pointer to the name (from static space)
 */

char *
privtoname(priv)
	int	priv;
{
	register int i;

	for (i = 0; sys_priv[i].name != (char *) 0; i++)
		if (sys_priv[i].value == priv)
			return sys_priv[i].name;
	return (char *) 0;
}

/*
 * Convert a privilege name into a number.
 */

int
nametopriv(privname)
	char	*privname;
{
	register int i;

	for (i = 0; sys_priv[i].name != (char *) 0; i++)
		if (strcmp(sys_priv[i].name, privname) == 0)
			return sys_priv[i].value;
	return -1;
}

/*
 * Convert a privilege vector into a printable string.
 */
char *
privstostr(vec, sep)
	privvec_t	vec;
	char		*sep;
{
	static char	*buf = (char *) 0;
	static int	buflen = 0;
	register int	priv, seplen, incr, i = 0;

	if (sep == (char *) 0 || *sep == '\0')
		sep = " ";
	seplen = strlen(sep);

	for (priv = 0; priv <= SEC_MAX_SPRIV; ++priv) {
		char *cp;

		if (!ISBITSET(vec, priv))
			continue;
		cp = privtoname(priv);
		incr = strlen(cp) + seplen;
		while (i + incr + 1 > buflen) {
			char	*newbuf;

			if (buf)
				newbuf = realloc(buf, buflen + PRIVSTR_INCR);
			else
				newbuf = malloc(PRIVSTR_INCR);
			if (newbuf == (char *) 0)
				return (char *) 0;
			buflen += PRIVSTR_INCR;
			buf = newbuf;
		}
		sprintf(&buf[i], "%s%s", cp, sep);
		i += incr;
	}

	/* Truncate final separator */

	if (i) {
		buf[i - seplen] = '\0';
		return buf;
	} else
		return "";
}


/*
 * Parse a string of privilege names into a privilege vector.
 */
int
strtoprivs(str, delim, vec)
	char		*str, *delim;
	privvec_t	vec;
{
	register char	*cp = str;
	register char	savec;
	register int	wordlen, priv;

	memset(vec, 0, sizeof vec);

	if (delim == (char *) 0 || *delim == '\0')
		delim = " ,\t\n";

	for (;;) {
		cp += strspn(cp, delim);
		if (*cp == '\0')
			break;

		wordlen = strcspn(cp, delim);
		savec = cp[wordlen];
		cp[wordlen] = '\0';

		for (priv = 0; priv <= SEC_MAX_SPRIV; ++priv)
			if (strcmp(cp, privtoname(priv)) == 0)
				break;

		cp[wordlen] = savec;
		if (priv > SEC_MAX_SPRIV)
			return cp - str;

		ADDBIT(vec, priv);
		cp += wordlen;
	}

	return -1;
}
