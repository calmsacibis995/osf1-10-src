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

#include "dec_audit.h"
#include <sys/user.h>
#include <sys/audit.h>
#include <sys/errno.h>
#ifdef SEC_PRIV
#include <sys/security.h>
#endif SEC_PRIV

/* global data */
u_int audswitch = 0;   /* audit on/off */
u_int audstyle  = 0;   /* style flags  */


/* audit configured */
#if DEC_AUDIT > 0

int audsize = DEC_AUDIT;    /* buffer size  */

/* set N_SITE_EVENTS to be the number of audit site events required
   do not set the value of N_SITE_EVENTS to be less than 1
*/
#define N_SITE_EVENTS 64

u_int n_sitevents = N_SITE_EVENTS;
u_int siteauditmask[AUDINTMASK_LEN(N_SITE_EVENTS)];


/* callable version of AUDIT_CALL macro; provided to decrease text space
   growth seen with inline expansion of AUDIT_CALL
*/
audit_call ( event, error, args, retval, flag, param )
u_int event;
int error;
char **args;
long retval;
u_int flag;
char *param;
{
    if ( DO_AUDIT(event,error) )
        audit_rec_build ( event, args, error, retval, (int *)0, flag, param );
}


#else /* DEC_AUDIT */ /* audit not configured */

struct aud_log { int x; } aud_log;
void *a_d_ptr;
void *audit_data;
u_int syscallauditmask[1];
u_int trustedauditmask[1];

#ifdef SEC_PRIV
#define ALLOWED(priv) (privileged ( (priv), 0 ))
#else
#define ALLOWED(priv) (u.u_uid == 0)
#endif /* SEC_PRIV */


audlock_init() {}

audcntl ( p, args, retval )
struct proc *p;
void *args;
long *retval;
{ 
    int error;

    if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
    else error = ENOSYS;
    return(error);
}

audgen ( p, args, retval )
struct proc *p;
void *args;
long *retval;
{
    int error;

    if ( !ALLOWED(SEC_WRITE_AUDIT) ) error = EACCES;
    else error = ENOSYS;
    return(error);
}

audit_rec_build() { return(0); }
audit_call() {}

#endif /* DEC_AUDIT */
