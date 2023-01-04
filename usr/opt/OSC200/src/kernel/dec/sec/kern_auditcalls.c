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
static char *rcsid = "@(#)$RCSfile: kern_auditcalls.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/12/20 21:23:48 $";
#endif


#include <sys/types.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <kern/kalloc.h>
#include <sys/socket.h>
#include <sys/proc.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <ufs/inode.h>
#include <cpus.h>

u_int syscallauditmask[N_SYSCALLS];
u_int trustedauditmask[TRUSTED_INTMASK_LEN];

udecl_simple_lock_data(,audmask_lock)
#define	AUDMASK_LOCK()       usimple_lock(&audmask_lock)
#define	AUDMASK_UNLOCK()     usimple_unlock(&audmask_lock)

/* audcntl system call */
audcntl ( p, args, retval )
struct proc *p;
void *args;
long *retval;
{
    struct args {
        long req;
        char *argp;     /* auditmask            */
        long len;       /* length of argp       */
        long cntl;      /* really u_int         */
        long audit_id;  /* really type uid_t    */
        long pid;       /* really type pid_t    */
    } *uap = (struct args *) args;

#ifdef notyet
    register struct nameidata *ndp = &u.u_nd;
    register struct vnode *vp;
    struct vattr vattr;
#endif /* notyet */

    u_char local[AUDIT_MASK_LEN];
    char *sitelocal;        /* site auditmask                       */
    int len;
    int before, after;      /* state of auditing audcntl syscall    */
    u_long code;            /* habitat code                         */
    char buf[MAXHABSYSCALL_LEN];
    extern lock_data_t cm_lock;

    int indx;
    int error = 0;
    int i, j;

    before = DO_AUDIT(SYS_audcntl,error);


#ifdef SEC_PRIV
#define ALLOWED(priv) (privileged ( (priv), 0 ))
#else
#define ALLOWED(priv) (u.u_uid == 0)
#endif

    switch ( uap->req ) {

    /* get system auditmask */
    case GET_SYS_AMASK:
        len = uap->len < SYSCALL_MASK_LEN ? uap->len : SYSCALL_MASK_LEN;
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else if ( len < 0 ) error = EINVAL;
        else {
            bzero ( local, AUDIT_MASK_LEN );
            AUDMASK_LOCK();
            /* syscallauditmask stored internally one syscall per int (for
               performance reasons; represented externally as 4 syscalls per
               byte.
            */
            for ( i = 0; i < len<<2; i++ )
                local[i>>2] |= syscallauditmask[i] << ( (~i & 0x3) << 1 );
            AUDMASK_UNLOCK();
            error = copyout ( local, uap->argp, len );
        }
        *retval = len;
        break;


    /* set system auditmask */
    case SET_SYS_AMASK:
        len = uap->len < SYSCALL_MASK_LEN ? uap->len : SYSCALL_MASK_LEN;
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else if ( len < 0 ) error = EINVAL;
        else {
            if ( error = copyin ( uap->argp, local, len ) ) break;
            AUDMASK_LOCK();
            /* syscallauditmask stored internally one syscall per int (for
               performance reasons; represented externally as 4 syscalls per
               byte.
            */
            for ( i = 0; i < len<<2; i++ )
                syscallauditmask[i] = local[i>>2] >> ( (~i & 0x3) << 1 ) & 0x3;
            AUDMASK_UNLOCK();
        }
        *retval = len;
        break;


    /* get system trusted event mask */
    case GET_TRUSTED_AMASK:
        len = uap->len < TRUSTED_MASK_LEN ? uap->len : TRUSTED_MASK_LEN;
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else if ( len < 0 ) error = EINVAL;
        else {
            AUDMASK_LOCK();
            bcopy ( (char *)trustedauditmask, local, len );
            AUDMASK_UNLOCK();
            error = copyout ( local, uap->argp, len );
        }
        *retval = len;
        break;


    /* set system trusted event mask */
    case SET_TRUSTED_AMASK:
        len = uap->len < TRUSTED_MASK_LEN ? uap->len : TRUSTED_MASK_LEN;
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else if ( len < 0 ) error = EINVAL;
        else {
            if ( error = copyin ( uap->argp, local, len ) ) break;
            AUDMASK_LOCK();
            bcopy ( local, (char *)trustedauditmask, len );
            AUDMASK_UNLOCK();
        }
        *retval = len;
        break;


    /* get process auditmask */
    case GET_PROC_AMASK:
        len = uap->len < AUDIT_MASK_LEN ? uap->len : AUDIT_MASK_LEN;
        if ( !ALLOWED(SEC_SUSPEND_AUDIT) ) error = EACCES;
        else if ( len < 0 ) error = EINVAL;
        else {
            AUDMASK_LOCK();
            bcopy ( (char *)u.u_auditmask, local, len );
            AUDMASK_UNLOCK();
            error = copyout ( local, uap->argp, len );
        }
        *retval = len;
        break;


    /* set process auditmask */
    case SET_PROC_AMASK:
        len = uap->len < AUDIT_MASK_LEN ? uap->len : AUDIT_MASK_LEN;
        if ( !ALLOWED(SEC_SUSPEND_AUDIT) ) error = EACCES;
        else if ( len < 0 ) error = EINVAL;
        else {
            if ( error = copyin ( uap->argp, local, len ) ) break;
            AUDMASK_LOCK();
            bcopy ( local, (char *)u.u_auditmask, len );
            AUDMASK_UNLOCK();
        }
        *retval = len;
        break;


    /* get process audcntl flag */
    case GET_PROC_ACNTL:
        if ( !ALLOWED(SEC_SUSPEND_AUDIT) ) error = EACCES;
        else *retval = u.u_audit_cntl;
        break;


    /* set process audcntl flag */
    case SET_PROC_ACNTL:
        if ( !ALLOWED(SEC_SUSPEND_AUDIT) ) error = EACCES;
        else {
            u.u_audit_cntl = uap->cntl;
            *retval = u.u_audit_cntl;
        }
        break;


    /* get system audit switch */
    case GET_AUDSWITCH:
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else *retval = audswitch;
        break;


    /* set system audit switch */
    case SET_AUDSWITCH:
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else {
            *retval = audswitch;
            audswitch = uap->cntl ? 1 : 0;
        }
        break;


#ifdef notdef
    /* get process audit_id */
    case GETPAID:
        if ( u.u_procp->p_auid == AUID_INVAL ) error = EPERM;
        else *retval = u.u_procp->p_auid;
        break;

    /* set process audit_id */
    case SETPAID:
        if ( u.u_uid ) error = EPERM;
        else if ( uap->audit_id == AUID_INVAL ) error = EINVAL;
        else u.u_procp->p_auid = uap->audit_id;
        break;
#endif /* notdef */


    /* get system audstyle flag */
    case GET_AUDSTYLE:
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else *retval = audstyle;
        break;


    /* set system audstyle flag */
    case SET_AUDSTYLE:
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else {
            *retval = audstyle;
            audstyle = uap->cntl;
        }
        break;


    /* get system site-defined mask */
    case GET_SITEMASK:
        len = uap->len < AUDMASK_LEN(n_sitevents) ? uap->len : AUDMASK_LEN(n_sitevents);
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else if ( len < 0 ) error = EINVAL;
        else {
            if ( (sitelocal = kalloc ( len )) == NULL ) return ( ENOMEM );
            AUDMASK_LOCK();
            bcopy ( (char *)siteauditmask, sitelocal, len );
            AUDMASK_UNLOCK();
            error = copyout ( sitelocal, uap->argp, len );
            kfree ( sitelocal, len );
        }
        *retval = len;
        break;


    /* set system site-defined mask */
    case SET_SITEMASK:
        len = uap->len < AUDMASK_LEN(n_sitevents) ? uap->len : AUDMASK_LEN(n_sitevents);
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else if ( len < 0 ) error = EINVAL;
        else {
            if ( (sitelocal = kalloc ( len )) == NULL ) return ( ENOMEM );
            if ( (error = copyin ( uap->argp, sitelocal, len )) == 0 ) {
                AUDMASK_LOCK();
                bcopy ( sitelocal, (char *)siteauditmask, len );
                AUDMASK_UNLOCK();
            }
            kfree ( sitelocal, len );
        }
        *retval = len;
        break;


    /* get object's audmode bit (used for deselection by object) */
    case GET_OBJAUDBIT:
        error = EINVAL;     /* until code gets enabled */
#ifdef notyet
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else {
            ndp->ni_nameiop = LOOKUP|FOLLOW;
            ndp->ni_segflg = UIO_SYSSPACE;
            if ( (ndp->ni_dirp = kalloc ( MAXPATHLEN )) == NULL ) {
                error = ENOMEM;
                break;
            }
            if ( error = copyinstr ( uap->argp, ndp->ni_dirp, MAXPATHLEN, &uap->len ) ) {
                kfree ( ndp->ni_dirp, MAXPATHLEN );
                break;
            }
            if ( (error = namei(ndp)) == 0 ) {
                vp = ndp->ni_vp;
                vattr_null(&vattr);
                VOP_GETATTR(vp, &vattr, ndp->ni_cred, error);
                *retval = vattr.va_flags & INOAUDIT ? 1 : 0;
                vrele(vp);
            }
            kfree ( ndp->ni_dirp, MAXPATHLEN );
        }
#endif /* notyet */
        break;


    /* set object's audmode bit (used for deselection by object) */
    case SET_OBJAUDBIT:
        error = EINVAL;     /* until code gets enabled */
#ifdef notyet
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else {
            ndp->ni_nameiop = LOOKUP|FOLLOW;
            ndp->ni_segflg = UIO_SYSSPACE;
            if ( (ndp->ni_dirp = kalloc ( MAXPATHLEN )) == NULL ) {
                error = ENOMEM;
                break;
            }
            if ( error = copyinstr ( uap->argp, ndp->ni_dirp, MAXPATHLEN, &uap->len ) ) {
                kfree ( ndp->ni_dirp, MAXPATHLEN );
                break;
            }
            if ( (error = namei(ndp)) == 0 ) {
                vp = ndp->ni_vp;
                if ( rofs(vp->v_mount) == 0 ) {
                    vattr_null(&vattr);
                    vattr.va_flags = uap->cntl ? INOAUDIT : 0;
                    VOP_SETATTR(vp, &vattr, ndp->ni_cred, error);
                }
                vrele(vp);
            }
            kfree ( ndp->ni_dirp, MAXPATHLEN );
        }
#endif /* notyet */
        break;


    /* update target process' auditmask */
    case UPDEVENTS:
        len = uap->len < AUDIT_MASK_LEN ? uap->len : AUDIT_MASK_LEN;
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else if ( len < 0 ) error = EINVAL;
        else if ( error = copyin ( uap->argp, local, len ) ) error = EIO;
        else error = aud_upd ( local, len, uap->cntl, uap->audit_id, uap->pid );
        break;


    /* flush system audit buffers */
    case FLUSH_AUD_BUF:
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else {
            *retval = audit_rec_build ( 0, (char *)0, 0, 0L, 0, AUD_FLU, (char *)0 );
            /* make sure all the buffers are clean if audit disabled */
            for ( j = 0; audswitch == 0 && j; ) {
                j = audit_rec_build ( 0, (char *)0, 0, 0L, 0, AUD_FLU, (char *)0 );
                *retval += j;
            }
        }
        break;


    /* get habitat/syscall name and auditmask bits for specified event # */
    case GET_HABITAT_EVENT:
#if BIN_COMPAT
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else {
            /* get habitat number and habitat offset */
            i = hbval(uap->cntl);
            code = uap->cntl & ~(u_long)HABITAT_MASK;
            lock_read ( &cm_lock );
            if ( i <= 0 || i >= MAXHABITATS || habitats[i] == NULL
            || code >= habitats[i]->cm_nsysent || habitats[i]->call_name == NULL ) {
                lock_done ( &cm_lock );
                error = EINVAL;
                break;
            }

            /* get habitat/syscall names */
            error = copyoutstr ( habitats[i]->cm_name, uap->argp, uap->len, &len );
            if ( error == 0 )
                error = copyout ( "/", &uap->argp[len-1], 1 );
            if ( error == 0 )
                error = copyoutstr ( habitats[i]->call_name[code], &uap->argp[len], uap->len-len, &len );

            /* get auditmask bits */
            if ( habitats[i]->cm_auditmask )            
                *retval = EVENT_BITS(habitats[i]->cm_auditmask,code);

            lock_done ( &cm_lock );
            break;
        }
#else
        error = ENOSYS;
#endif /* BIN_COMPAT */
        break;


    /* set auditmask bits for specified habitat/syscall name */
    case SET_HABITAT_EVENT:
#if BIN_COMPAT
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else if ( error = copyinstr ( uap->argp, buf, sizeof(buf), &uap->len ) ) error = EIO;
        else {

            /* parse cm_name, syscallname */
            for ( indx = uap->len-1; indx && buf[indx] != '/'; indx-- );
            if ( buf[indx] == '/' ) buf[indx] = '\0';
            else {
                error = EINVAL;
                break;
            }

            /* find habitat */
            lock_write ( &cm_lock );
            for ( i = 1; i < MAXHABITATS; i++ )
                if ( habitats[i] && bcmp ( habitats[i]->cm_name, buf, indx+1 ) == 0 ) {
                    lock_done ( &cm_lock );
                    break;
                }

            /* note: habitat must be loaded before habitat auditmask gets set */
            if ( i == MAXHABITATS || habitats[i]->cm_auditmask == NULL ) {
                lock_done ( &cm_lock );
                error = EINVAL;
                break;
            }

            /* scan habitats[]->call_name for syscallname */
            for ( j = 0; j < habitats[i]->cm_nsysent; j++ ) {
                if ( bcmp ( habitats[i]->call_name[j], &buf[indx+1], uap->len-indx-1 ) == 0 ) {
                    /* set habitats[]->auditmask bits */
                    habitats[i]->cm_auditmask[WORD_IN_MASK(j)] &= ~BITS_IN_WORD(0x3,j);
                    habitats[i]->cm_auditmask[WORD_IN_MASK(j)] |= BITS_IN_WORD(uap->cntl,j);
                    break;
                }
            }
            if ( j == habitats[i]->cm_nsysent ) error = EINVAL;
            lock_done ( &cm_lock );
        }
#else
        error = ENOSYS;
#endif /* BIN_COMPAT */
        break;


    /* get defined number of site-events */
    case GET_NSITEVENTS:
        *retval = n_sitevents;
        break;


    /* get size of audit device buffer */
    case GET_AUDSIZE:
        if ( !ALLOWED(SEC_CONFIG_AUDIT) ) error = EACCES;
        else *retval = AUDSIZE(audsize);
        break;


    default:    /* not supported */
        error = EINVAL;
        break;

    }

    /* must do own auditing here to catch transitions of turning audit on/off */
    after = DO_AUDIT(SYS_audcntl,error);
    if ( before | after )
        audit_rec_build ( SYS_audcntl, args, error, *retval, (int *)0, AUD_HPR, (char *)0 );

    return ( error );
}


/* audgen system call */
audgen ( p, args, retval )
struct proc *p;
void *args;
int *retval;
{
    struct args {
        u_long code;
        char *tokenmask;
        char **params;
        char *usrbuf;
        long *siz;
    } *uap = (struct args *) args;

    unsigned int adb_siz;           /* kernel audit buffer size     */
    char tokenmask[N_AUDTUPLES];    /* mask of token types          */
    char *ad_buf;                   /* ptr to audit record data     */
    long result = -1L;              /* operation result             */
    int error_l = 0;                /* operation error              */
    int narg;                       /* # of arguments               */
    u_int len = 0;                  /* length of audit record data  */
    int buf_indx;                   /* buf # returned from build    */
    int failed = 0;                 /* set to 1 if T_ERRNO used     */
    int top;                        /* top of buffer to be moved    */
    int usrbuf_siz;                 /* in/out: user buffer size     */

    extern char *audit_data[];      /* audit data buffer            */
    extern int a_d_ptr[];           /* # bytes data (record)        */
    extern int a_d_len[];           /* # bytes data (token)         */
    extern pid_t auditd_pid;        /* pid of audit daemon          */

    char *ptr;
    int error = 0;
    int copyrtrn;
    int i, j;

/* ----- macros to insert data into audit record buffer: start ----- */
#define AUD_OFFSET (u_long)(ad_buf - audit_data[buf_indx])
#define SIZ_TOKEN 1

/* insert null-term user space string, type (I_what1) value (I_what2) */
#define INSERT_AUD2(I_what1,I_what2,I_len2) \
    { \
        copyrtrn = copyinstr ( (I_what2), &ad_buf[len+sizeof(I_len2)+SIZ_TOKEN],\
        ( adb_siz-(len+AUD_OFFSET+sizeof(I_len2)+SIZ_TOKEN) ), &(I_len2) );\
        if ( copyrtrn ) {\
            top = len + AUD_OFFSET;\
            ad_buf -= a_d_ptr[buf_indx];\
            aud_overflow_hndlr(buf_indx,top);\
            copyrtrn = copyinstr ( (I_what2), &ad_buf[len+sizeof(I_len2)+SIZ_TOKEN],\
            ( adb_siz-(len+AUD_OFFSET+sizeof(I_len2)+SIZ_TOKEN) ), &(I_len2) );\
        }\
        if ( copyrtrn == 0 && (I_len2) > 0 ) {\
            ad_buf[len] = I_what1;\
            len += SIZ_TOKEN;\
            bcopy ( &(I_len2), &ad_buf[len], sizeof (I_len2) );\
            len += ((I_len2) + sizeof (I_len2));\
        }\
        else error = E2BIG;\
    }

/* insert (I_len2) size user-space element and size value, type (I_what1) value (I_what2) */
#define INSERT_AUD4(I_what1,I_what2,I_len2) \
    { \
        if ( len + AUD_OFFSET + sizeof (I_len2) + (I_len2) + SIZ_TOKEN*2 + sizeof(int) >= adb_siz ) {\
            top = len + AUD_OFFSET;\
            ad_buf -= a_d_ptr[buf_indx];\
            aud_overflow_hndlr(buf_indx,top);\
        }\
        if ( len + AUD_OFFSET + sizeof (I_len2) + (I_len2) + SIZ_TOKEN*2 + sizeof(int) < adb_siz ) {\
            ad_buf[len] = I_what1;\
            len += SIZ_TOKEN;\
            bcopy ( &(I_len2), &ad_buf[len], sizeof (I_len2) );\
            len += sizeof (I_len2);\
            if ( (I_len2) > 0 ) {\
                if ( copyin ( (I_what2), &ad_buf[len], (I_len2) ) )\
                    len -= (sizeof (I_len2) + SIZ_TOKEN);\
                else len += (I_len2);\
            }\
        }\
        else error = E2BIG;\
    }

/* insert (I_len2) size user-space element, type (I_what1) value (I_what2) */
#define INSERT_AUD5(I_what1,I_what2,I_len2) \
    { \
        if ( len + (I_len2) + AUD_OFFSET + SIZ_TOKEN*2 + sizeof(int) >= adb_siz ) {\
            top = len + AUD_OFFSET;\
            ad_buf -= a_d_ptr[buf_indx];\
            aud_overflow_hndlr(buf_indx,top);\
        }\
        if ( len + (I_len2) + AUD_OFFSET + SIZ_TOKEN*2 + sizeof(int) < adb_siz ) {\
            ad_buf[len] = I_what1;\
            if ( copyin ( (I_what2), &ad_buf[len+SIZ_TOKEN], (I_len2) ) == 0 )\
                len += (I_len2) + SIZ_TOKEN;\
        }\
        else error = E2BIG;\
    }
/* ----- macros to insert data into audit record buffer: end ----- */


    /* access check */
#ifdef SEC_PRIV
    if ( !privileged ( SEC_WRITE_AUDIT, 0 ) ) return ( EACCES );
#else
    if ( u.u_uid ) return ( EACCES );
#endif

    /* sanity-check code number */
    if ( (uap->code < MIN_TRUSTED_EVENT) || (uap->code >= MIN_SITE_EVENT+n_sitevents)
    || (uap->code >= MIN_TRUSTED_EVENT+N_TRUSTED_EVENTS && uap->code < MIN_SITE_EVENT) )
        return ( EINVAL );

    /* check tokenmask */
    if ( copyinstr ( uap->tokenmask, tokenmask, sizeof(tokenmask), &narg ) )
        return ( EIO );
    for ( i = 0; i < narg; i++ ) {
        if ( A_TOKEN_PRIV(tokenmask[i]) ) return ( EINVAL );
        if ( (tokenmask[i]&0xff) == T_ERRNO ) failed = 1;
    }


    /* fetch/modify uap->siz here in case event doesn't get audited.
       this is used for applications building records without having
       those records implicitly placed in the audit trail (auditd).
    */
    if ( uap->siz ) {
        usrbuf_siz = fuword(uap->siz);
        suword ( uap->siz, 0 );
    }


    /* check auditability of event; bypass check for user-space records */
    if ( uap->siz == 0 ) {
        if ( audswitch == 0 ) return(0);
        if ( uap->code >= MIN_TRUSTED_EVENT && uap->code < MIN_TRUSTED_EVENT+N_TRUSTED_EVENTS )
            if ( (i = DO_TRUSTED_AUD(uap->code)) == 0 ) return(0);
        if ( uap->code >= MIN_SITE_EVENT && uap->code < MIN_SITE_EVENT+n_sitevents )
            if ( (i = AUDIT_EVENT_S ( uap->code-MIN_SITE_EVENT )) == 0 ) return(0);
        if ( (failed == 1) && ((i&0x01) == 0) ) return(0);
        if ( (failed == 0) && ((i&0x02) == 0) ) return(0);
        if ( u.u_audit_cntl & AUDIT_OFF ) return(0);
    }


    /* build header of audit record; get ptr to data.
       calculate kernel audit buffer size (set in audit_rec_build).
       auditd uses a dedicated resource to prevent deadlock.
       assumption: auditd audit record fits in AUDITD_RECSZ.
    */
    if ( u.u_procp->p_pid == auditd_pid ) {
        buf_indx = NCPUS;
        if ( (ad_buf = (char *)audit_rec_build ( uap->code, (char *)0,
        0, 0L, &buf_indx, AUD_HDR|AUD_GEN, (char *)0 )) == (char *)0 ) return(0);
        adb_siz = AUDITD_RECSZ;
    }
    else {
        buf_indx = -1;
        if ( (ad_buf = (char *)audit_rec_build ( uap->code, (char *)0,
        0, 0L, &buf_indx, AUD_HDR|AUD_GEN, (char *)0 )) == (char *)0 ) return(0);
        adb_siz = (audsize*1024) < (AUD_BUF_SIZ<<2) ? (AUD_BUF_SIZ<<2) : (audsize*1024);
    }


    /* add user-entered parameters to audit record */
    for ( i = 0; i < narg && tokenmask[i]; i++ ) {

        /* special case result and error */
#ifdef __alpha
        if ( (tokenmask[i]&0xff) == T_RESULT ) result = fuqword(uap->params++);
#else
        if ( (tokenmask[i]&0xff) == T_RESULT ) result = fuword(uap->params++);
#endif
        else if ( (tokenmask[i]&0xff) == T_ERRNO ) error_l = fuword(uap->params++);

        /* socket structure (from user space) */
        else if ( (tokenmask[i]&0xff) == T_SOCK ) {
#ifdef __alpha
            ptr = (char *)fuqword(uap->params++);
#else
            ptr = (char *)fuword(uap->params++);
#endif
            if ( ptr == (char *)-1 ) continue;
            j = sizeof(struct sockaddr);
            INSERT_AUD4 ( tokenmask[i], ptr, j );
        }

        /* user null-terminated strings */
        else if ( A_TOKEN_PTR(tokenmask[i]) ) {
#ifdef __alpha
            ptr = (char *)fuqword(uap->params++);
#else
            ptr = (char *)fuword(uap->params++);
#endif
            if ( ptr == (char *)-1 ) continue;
            INSERT_AUD2 ( tokenmask[i], ptr, j );
        }

        /* all other user (int-sized) objects */
        else INSERT_AUD5 ( tokenmask[i], uap->params++, sizeof(int) );
    }


    /* update ptr's to record in audit_rec_build for just added params */
    audit_rec_build ( 0, (char *)0, 0, len, &buf_indx, AUD_PTR|AUD_GEN, (char *)0 );


    /* if uap->siz given, this record is to be copied out to
       user space, and not entered into auditlog from here */
    if ( uap->siz ) {
        /* finish building the audit record */
        ad_buf = (char *)audit_rec_build ( 0, (char *)0, error_l, result, &buf_indx, AUD_LCL|AUD_GEN, (char *)0 );

        /* get/set buffer size */
        i = a_d_len[buf_indx]-a_d_ptr[buf_indx];
        if ( suword(uap->siz,i) == -1 ) error = EIO;
        else if ( usrbuf_siz < i ) error = E2BIG;

        /* copy the data, backing it out of audit_rec_build buffer, unlock */
        if ( error == 0 )
            if ( copyout ( ad_buf, uap->usrbuf, i ) ) error = EIO;
        audit_rec_build ( 0, (char *)0, 0, 0L, &buf_indx, AUD_LCK|AUD_GEN, (char *)0 );
        return ( error );
    }


    /* finish audit record; pass to /dev/audit */
    audit_rec_build ( 0, (char *)0, error_l, result, &buf_indx, AUD_RES|AUD_GEN, (char *)0 );

    return ( error );
}


/* update auditmask and cntl flag for specified pid or auid */
aud_upd ( mask, len, cntl, auid, pid )
char *mask;
int len;
long cntl;
long auid;
long pid;  /* really type pid_t */
{
    register struct proc *p;
    char *ptr;
    int s;
    int i;

    /* update auditmask/audcntl flag for specified pid */
    if ( pid ) {
        /* SMP: need a proc_get equivalent */
        if ( (p = pfind(pid)) == 0 )
            return ( ESRCH );
        PROC_LOCK(p);

        /* set cntl flag if (cntl != -1); set mask if (len > 0) */
        if ( cntl != -1 ) p->u_audit_cntl = (u_int)cntl&0xff;
        if ( len > 0 ) {
            AUDMASK_LOCK();
            bcopy ( mask, p->u_auditmask, AUDIT_MASK_LEN );
            AUDMASK_UNLOCK();
        }

        /* SMP: need a proc_rele equivalent */
        PROC_UNLOCK(p);
    }


    /* update auditmask/audcntl flag for all processes with specified auid */
    if ( auid != AUID_INVAL ) for ( p = allproc; p != NULL; p = p->p_nxt ) {
        if ( auid != p->p_auid ) continue;

        /* SMP: need a proc_get equivalent */
        PROC_LOCK(p);

        /* set cntl flag if (cntl != -1); set mask if (len > 0) */
        if ( cntl != -1 ) p->u_audit_cntl = (u_int)cntl&0xff;
        if ( len > 0 ) {
            AUDMASK_LOCK();
            bcopy ( mask, p->u_auditmask, AUDIT_MASK_LEN );
            AUDMASK_UNLOCK();
        }

        /* SMP: when find proc_rele equivalent */
        PROC_UNLOCK(p);
    }
    return(0);
}
