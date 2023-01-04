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
static char *rcsid = "@(#)$RCSfile: clnt_perror.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 17:59:18 $";
#endif
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)clnt_perror.c	1.4 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *  1.16 88/02/08
 */

/*
 * clnt_perror.c
 */
#include <rpc/rpc.h>

#ifndef KERNEL
extern char *sys_errlist[];
extern char *sprintf();
static char *auth_errmsg();
#endif

extern char *strcpy();

#ifndef KERNEL
static char *buf;

static char *
_buf()
{

	if (buf == NULL)
		buf = (char *)malloc(256);
	return (buf);
}

/*
 * Return string reply error info. For use after clnt_call()
 */
char *
clnt_sperror(rpch, s)
	CLIENT *rpch;
	char *s;
{
	struct rpc_err e;
	void clnt_perrno();
	char *err;
	char *str = _buf();
	char *strstart = str;

	if (str == NULL)
		return (NULL);
	CLNT_GETERR(rpch, &e);

	(void) sprintf(str, "%s: ", s);  
	str += strlen(str);

	(void) strcpy(str, clnt_sperrno(e.re_status));  
	str += strlen(str);

	switch (e.re_status) {
	case RPC_SUCCESS:
	case RPC_CANTENCODEARGS:
	case RPC_CANTDECODERES:
	case RPC_TIMEDOUT:     
	case RPC_PROGUNAVAIL:
	case RPC_PROCUNAVAIL:
	case RPC_CANTDECODEARGS:
	case RPC_SYSTEMERROR:
	case RPC_UNKNOWNHOST:
	case RPC_UNKNOWNPROTO:
	case RPC_PMAPFAILURE:
	case RPC_PROGNOTREGISTERED:
	case RPC_FAILED:
		break;

	case RPC_CANTSEND:
	case RPC_CANTRECV:
		(void) sprintf(str, "; errno = %s",
		    sys_errlist[e.re_errno]); 
		str += strlen(str);
		break;

	case RPC_VERSMISMATCH:
		(void) sprintf(str,
			"; low version = %lu, high version = %lu", 
			e.re_vers.low, e.re_vers.high);
		str += strlen(str);
		break;

	case RPC_AUTHERROR:
		err = auth_errmsg(e.re_why);
		(void) sprintf(str,"; why = ");
		str += strlen(str);
		if (err != NULL) {
			(void) sprintf(str, "%s",err);
		} else {
			(void) sprintf(str,
				"(unknown authentication error - %d)",
				(int) e.re_why);
		}
		str += strlen(str);
		break;

	case RPC_PROGVERSMISMATCH:
		(void) sprintf(str, 
			"; low version = %lu, high version = %lu", 
			e.re_vers.low, e.re_vers.high);
		str += strlen(str);
		break;

	default:	/* unknown */
		(void) sprintf(str, 
			"; s1 = %lu, s2 = %lu", 
			e.re_lb.s1, e.re_lb.s2);
		str += strlen(str);
		break;
	}
	return (strstart);
}

void
clnt_perror(rpch, s)
	CLIENT *rpch;
	char *s;
{
	(void) fprintf(stderr,"%s",clnt_sperror(rpch,s));
}

#endif /* ! KERNEL */

struct rpc_errtab {
	enum clnt_stat status;
	char *message;
};

static struct rpc_errtab  rpc_errlist[] = {
	{ RPC_SUCCESS, 
		"RPC: Success" }, 
	{ RPC_CANTENCODEARGS, 
		"RPC: Can't encode arguments" },
	{ RPC_CANTDECODERES, 
		"RPC: Can't decode result" },
	{ RPC_CANTSEND, 
		"RPC: Unable to send" },
	{ RPC_CANTRECV, 
		"RPC: Unable to receive" },
	{ RPC_TIMEDOUT, 
		"RPC: Timed out" },
	{ RPC_VERSMISMATCH, 
		"RPC: Incompatible versions of RPC" },
	{ RPC_AUTHERROR, 
		"RPC: Authentication error" },
	{ RPC_PROGUNAVAIL, 
		"RPC: Program unavailable" },
	{ RPC_PROGVERSMISMATCH, 
		"RPC: Program/version mismatch" },
	{ RPC_PROCUNAVAIL, 
		"RPC: Procedure unavailable" },
	{ RPC_CANTDECODEARGS, 
		"RPC: Server can't decode arguments" },
	{ RPC_SYSTEMERROR, 
		"RPC: Remote system error" },
	{ RPC_UNKNOWNHOST, 
		"RPC: Unknown host" },
	{ RPC_UNKNOWNPROTO,
		"RPC: Unknown protocol" },
	{ RPC_PMAPFAILURE, 
		"RPC: Port mapper failure" },
	{ RPC_PROGNOTREGISTERED, 
		"RPC: Program not registered"},
	{ RPC_FAILED, 
		"RPC: Failed (unspecified error)"}
};


/*
 * This interface for use by callrpc() and clnt_broadcast()
 */
char *
clnt_sperrno(stat)
	enum clnt_stat stat;
{
	int i;

	for (i = 0; i < sizeof(rpc_errlist)/sizeof(struct rpc_errtab); i++) {
		if (rpc_errlist[i].status == stat) {
			return (rpc_errlist[i].message);
		}
	}
	return ("RPC: (unknown error code)");
}

#ifndef KERNEL
void
clnt_perrno(num)
	enum clnt_stat num;
{
	(void) fprintf(stderr,"%s",clnt_sperrno(num));
}

/*
 * Return a string with the reason for
 * why a client handle could not be created
 */
char *
clnt_spcreateerror(s)
	char *s;
{
	extern int sys_nerr;
	extern char *sys_errlist[];
	char *str = _buf();

	if (str == NULL)
		return (NULL);
	(void) sprintf(str, "%s: ", s);
	(void) strcat(str, clnt_sperrno(rpc_createerr.cf_stat));
	switch (rpc_createerr.cf_stat) {
	case RPC_PMAPFAILURE:
		(void) strcat(str, " - ");
		(void) strcat(str,
		    clnt_sperrno(rpc_createerr.cf_error.re_status));
		break;

	case RPC_SYSTEMERROR:
		(void) strcat(str, " - ");
		if (rpc_createerr.cf_error.re_errno > 0
		    && rpc_createerr.cf_error.re_errno < sys_nerr)
			(void) strcat(str,
			    sys_errlist[rpc_createerr.cf_error.re_errno]);
		else
			(void) sprintf(&str[strlen(str)], "Error %d",
			    rpc_createerr.cf_error.re_errno);
		break;
	}
	return (str);
}

void
clnt_pcreateerror(s)
	char *s;
{
	(void) fprintf(stderr,"%s",clnt_spcreateerror(s));
}
#endif

#ifndef KERNEL
struct auth_errtab {
	enum auth_stat status;	
	char *message;
};

static struct auth_errtab auth_errlist[] = {
	{ AUTH_OK,
		"Authentication OK" },
	{ AUTH_BADCRED,
		"Invalid client credential" },
	{ AUTH_REJECTEDCRED,
		"Server rejected credential" },
	{ AUTH_BADVERF,
		"Invalid client verifier" },
	{ AUTH_REJECTEDVERF,
		"Server rejected verifier" },
	{ AUTH_TOOWEAK,
		"Client credential too weak" },
	{ AUTH_INVALIDRESP,
		"Invalid server verifier" },
	{ AUTH_FAILED,
		"Failed (unspecified error)" },
};

static char *
auth_errmsg(stat)
	enum auth_stat stat;
{
	int i;

	for (i = 0; i < sizeof(auth_errlist)/sizeof(struct auth_errtab); i++) {
		if (auth_errlist[i].status == stat) {
			return(auth_errlist[i].message);
		}
	}
	return(NULL);
}
#endif /* ! KERNEL */
