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
static char *rcsid = "@(#)$RCSfile: pcnfsd_v1.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1992/11/13 17:20:56 $";
#endif
/* RE_SID: @(%)/tmp_mnt/vol/dosnfs/shades_SCCS/unix/pcnfsd/v2/src/SCCS/s.pcnfsd_v1.c 1.3 92/08/18 12:54:02 SMI */
/*
**=====================================================================
** Copyright (c) 1986,1987,1988,1989,1990,1991 by Sun Microsystems, Inc.
**	@(#)pcnfsd_v1.c	1.3	8/18/92
**=====================================================================
*/
#include "common.h"

/*
**=====================================================================
**             I N C L U D E   F I L E   S E C T I O N                *
**                                                                    *
** If your port requires different include files, add a suitable      *
** #define in the customization section, and make the inclusion or    *
** exclusion of the files conditional on this.                        *
**=====================================================================
*/
#include "pcnfsd.h"

#include <stdio.h>
#include <pwd.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <string.h>

#ifndef SYSV
#include <sys/wait.h>
#endif

#ifdef ISC_2_0
#include <sys/fcntl.h>
#endif

#ifdef SHADOW_SUPPORT
#include <shadow.h>
#endif

#ifdef SIA
#include <sia.h>
#endif

/*
**---------------------------------------------------------------------
** Other #define's 
**---------------------------------------------------------------------
*/

extern void     scramble();
extern char    *crypt();

#ifdef WTMP
extern void wlogin();
#endif

extern struct passwd  *get_password();

/*
**---------------------------------------------------------------------
**                       Misc. variable definitions
**---------------------------------------------------------------------
*/

int             buggit = 0;

/*
**=====================================================================
**                      C O D E   S E C T I O N                       *
**=====================================================================
*/

#ifdef SIA
static SIAENTITY *entity=NULL;
static int oargc = 1;
static char *oargv[1] = { "rpc.pcnfsd" };
static int (*sia_collect)()=sia_collect_trm;
#endif

/*ARGSUSED*/
void *pcnfsd_null_1(arg)
void *arg;
{
static char dummy;
return((void *)&dummy);
}

auth_results *pcnfsd_auth_1(arg)
auth_args *arg;
{
static auth_results r;

char            uname[32];
char            pw[64];
int             c1, c2;
char		*msgp;

#ifdef SIA
struct passwd	p_instance;
struct passwd	*p = (struct passwd *) &p_instance;
#else
struct passwd  *p;
#endif /* SIA */

	r.stat = AUTH_RES_FAIL;	/* assume failure */
	r.uid = (int)-2;
	r.gid = (int)-2;

	scramble(arg->id, uname);
	scramble(arg->pw, pw);

#ifdef USER_CACHE
	if(check_cache(uname, pw, &r.uid, &r.gid)) {
		 r.stat = AUTH_RES_OK;
#ifdef WTMP
		wlogin(uname);
#endif
		 return (&r);
   }
#endif

#ifdef SIA
        if (sia_ses_init(&entity,oargc,oargv,NULL,uname,NULL,FALSE,NULL) == SIASUCCESS) {
		if ((sia_ses_authent(sia_collect,pw,entity)) == SIASUCCESS) {
			if ((sia_ses_estab(sia_collect,entity)) == SIASUCCESS) {
				r.stat = AUTH_RES_OK;
				p->pw_uid = entity->pwd->pw_uid;
				p->pw_gid = entity->pwd->pw_gid;
				p->pw_passwd = entity->pwd->pw_passwd;
				p->pw_name = entity->pwd->pw_name;
				r.uid = p->pw_uid;
				r.gid = p->pw_gid;
				sia_ses_release(&entity);
			}
			else {
			    return(&r);
		    }
		}
		else {
		    return(&r);
	    }
	}
	else {
		return(&r);
	}
#else /* not SIA */
        p = get_password(uname, &msgp);
	if (p == (struct passwd *)NULL)
	   return (&r);

	c1 = strlen(pw);
	c2 = strlen(p->pw_passwd);
	if ((c1 && !c2) || (c2 && !c1) ||
	   (strcmp(p->pw_passwd, crypt(pw, p->pw_passwd)))) 
           {
	   return (&r);
	   }
	r.stat = AUTH_RES_OK;
	r.uid = p->pw_uid;
	r.gid = p->pw_gid;
#endif /* SIA */

#ifdef WTMP
		wlogin(uname);
#endif

#ifdef USER_CACHE
	add_cache_entry(p);
#endif

return(&r);
}

pr_init_results *pcnfsd_pr_init_1(pi_arg)
pr_init_args *pi_arg;
{
static pr_init_results pi_res;

	pi_res.stat =
	  (pirstat) pr_init(pi_arg->system, pi_arg->pn, &pi_res.dir);

return(&pi_res);
}

pr_start_results *pcnfsd_pr_start_1(ps_arg)
pr_start_args *ps_arg;
{
static pr_start_results ps_res;
char *dummyptr;

	ps_res.stat =
	  (psrstat) pr_start2(ps_arg->system, ps_arg->pn, ps_arg->user,
	  ps_arg ->file, ps_arg->opts, &dummyptr);

return(&ps_res);
}

