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
static char     *sccsid = "@(#)$RCSfile: yppasswd.c,v $ $Revision: 4.2.3.5 $ (DEC) $Date: 93/01/26 13:52:48 $";
#endif
/*
 */

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <rpc/rpc.h>
#include <rpc/key_prot.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/yppasswd.h>
#include <rpc/netdb.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <errno.h>

#define PKMAP	"publickey.byname"

extern char *index();
extern time_t time();

struct yppasswd *getyppw();
char *oldpass;
char *newpass;

main(argc, argv)	
	char **argv;
{
	int ans, port, ok;
	struct yppasswd *yppasswd;
	char *domain;
	char *master;

	if (yp_get_default_domain(&domain) != 0) {
		(void)fprintf(stderr, "can't get domain\n");
		exit(1);
	}
	if (yp_master(domain, "passwd.byname", &master) != 0) {
		(void)fprintf(stderr, "can't get master for passwd file\n");
		exit(1);
	}
	port = getrpcport(master, YPPASSWDPROG, YPPASSWDPROC_UPDATE,
		IPPROTO_UDP);
	if (port == 0) {
		(void)fprintf(stderr, "%s is not running yppasswd daemon\n",
			      master);
		exit(1);
	}
	if (port >= IPPORT_RESERVED) {
		(void)fprintf(stderr,
		    "yppasswd daemon is not running on privileged port\n");
		exit(1);
	}
	yppasswd = getyppw(argc, argv);
	ans = callrpc(master, YPPASSWDPROG, YPPASSWDVERS,
	    YPPASSWDPROC_UPDATE, xdr_yppasswd, yppasswd, xdr_int, &ok);
	if (ans != 0) {
		clnt_perrno(ans);
		(void)fprintf(stderr, "\n");
		(void)fprintf(stderr, "couldn't change passwd\n");
		exit(1);
	}
	if (ok != 0) {
		(void)fprintf(stderr, "couldn't change passwd\n");
		exit(1);
	}
	(void)printf("NIS passwd changed on %s\n", master);
	/* reencrypt_secret(domain);*/
	free (master);
}

#ifdef SECRET
/*
 * If the user has a secret key, reencrypt it.
 * Otherwise, be quiet.
 */
reencrypt_secret(domain)
	char *domain;
{
	char who[MAXNETNAMELEN+1];
	char secret[HEXKEYBYTES+1];
	char public[HEXKEYBYTES+1];
	char crypt[HEXKEYBYTES + KEYCHECKSUMSIZE + 1];
	char pkent[sizeof(crypt) + sizeof(public) + 1];
	char *master;

	getnetname(who);
	if (!getsecretkey(who, secret, oldpass)) {
		/*
		 * Quiet: net is not running secure RPC
		 */
		return;
	}
	if (secret[0] == 0) {
		/*
		 * Quiet: user has no secret key
		 */
		return;
	}
	if (!getpublickey(who, public)) {
		(void)fprintf(stderr, 
			      "Warning: can't find public key for %s.\n", who);
		return;
	}
	bcopy(secret, crypt, HEXKEYBYTES); 
	bcopy(secret, crypt + HEXKEYBYTES, KEYCHECKSUMSIZE); 
	crypt[HEXKEYBYTES + KEYCHECKSUMSIZE] = 0; 
	xencrypt(crypt, newpass); 
	(void)sprintf(pkent, "%s:%s", public, crypt);
	if (yp_update(domain, PKMAP, YPOP_STORE,
	              who, strlen(who), pkent, strlen(pkent)) != 0) {

		(void)fprintf(stderr, 
			      "Warning: couldn't reencrypt secret key for %s\n",
			      who);
		return;
	}
	if (yp_master(domain, PKMAP, &master) != 0) {
		master = "NIS master";	/* should never happen */
	}
	(void)printf("secret key reencrypted for %s on %s\n", who, master);
}
#endif 

struct	passwd *pwd;
struct	passwd *getpwent();
void	endpwent();
char	*strcpy();
char	*crypt();
char	*getpass();
char	*getlogin();
char	*pw;
char	pwbuf[10];
char	pwbuf1[10];
char	hostname[256];
extern	int errno;

struct yppasswd *
getyppw(argc, argv)
	char *argv[];
{
	char *p;
	int i;
	char saltc[2];
	long salt;
	int u;
	int insist;
	int ok, flags;
	int c, pwlen;
	char *uname;
	static struct yppasswd yppasswd;

	insist = 0;
	uname = NULL;
	if (argc > 1)
		uname = argv[1];
	if (uname == NULL) {
		if ((uname = getlogin()) == NULL) {
			(void)fprintf(stderr, "Usage: yppasswd login_name\n");
			exit(1);
		}
		(void)gethostname(hostname, sizeof(hostname));
		(void)printf("Changing NIS password for %s\n", uname);
	}

	while (((pwd = getpwent()) != NULL) && strcmp(pwd->pw_name, uname))
		;
	u = getuid();
	if (pwd == NULL) {
		(void)printf("Not in passwd file.\n");
		exit(1);
	}
	if (u != 0 && u != pwd->pw_uid) {
		(void)printf("Permission denied.\n");
		exit(1);
	}
	endpwent();
	(void)strncpy(pwbuf1, getpass("Old NIS password:"), sizeof(pwbuf1) -1);
	pwbuf1 [sizeof(pwbuf1) - 1] = '\0'; 
	oldpass = pwbuf1;
tryagain:
	(void)strcpy(pwbuf, getpass("New password:"));
	newpass = pwbuf;
	pwlen = strlen(pwbuf);
	if (pwlen == 0) {
		(void)printf("Password unchanged.\n");
		exit(1);
	}
	/*
	 * Insure password is of reasonable length and
	 * composition.  If we really wanted to make things
	 * sticky, we could check the dictionary for common
	 * words, but then things would really be slow.
	 */
	ok = 0;
	flags = 0;
	p = pwbuf;
	while (c = *p++) {
		if (c >= 'a' && c <= 'z')
			flags |= 2;
		else if (c >= 'A' && c <= 'Z')
			flags |= 4;
		else if (c >= '0' && c <= '9')
			flags |= 1;
		else
			flags |= 8;
	}
	if (flags >= 7 && pwlen >= 4)
		ok = 1;
	if ((flags == 2 || flags == 4) && pwlen >= 6)
		ok = 1;
	if ((flags == 3 || flags == 5 || flags == 6) && pwlen >= 5)
		ok = 1;
	if (!ok && insist < 2) {
		(void)printf("Please use %s.\n", flags == 1 ?
			"at least one non-numeric character" :
			"a longer password");
		insist++;
		goto tryagain;
	}
	if (strcmp(pwbuf, getpass("Retype new password:")) != 0) {
		(void)printf("Mismatch - password unchanged.\n");
		exit(1);
	}
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
	pw = crypt(pwbuf, saltc);
	yppasswd.oldpass = pwbuf1;
	pwd->pw_passwd = pw;
	yppasswd.newpw = *pwd;
	return (&yppasswd);
}
