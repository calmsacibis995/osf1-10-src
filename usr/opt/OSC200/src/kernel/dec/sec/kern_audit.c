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
static char	*rcsid = "@(#)$RCSfile: kern_audit.c,v $ $Revision: 1.1.2.10 $ (DEC) $Date: 1993/12/09 20:24:04 $";
#endif 

#include <sys/audit.h>
#include <sys/param.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <kern/kalloc.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <ufs/inode.h>
#include <sys/mode.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/mount.h>
#include <rpc/types.h>
#include <nfs/nfs.h>
#include <sys/syscall.h>
#include <sys/sysv_syscall.h>
#include <sys/habitat.h>
#include <kern/parallel.h>
#include <kern/lock.h>
#include <cpus.h>


char *audit_data[NCPUS+1];    /* audit data buffer                           */
int  a_d_len[NCPUS+1];        /* # bytes data; incremented on each token     */
int  a_d_ptr[NCPUS+1];        /* # bytes data; incremented on each record    */
int  audbuf_mask[NCPUS+1];    /* to ensure exclusive use -- must be locked   */
int  initdone[NCPUS+1];       /* per-cpu values initialized                  */

#ifdef AUD_DEBUG
int audgen_overflow = 0;
int audrec_overflow = 0;
#endif /* AUD_DEBUG */

/* one audit data buffer per cpu; process gets buffer associated with
   its cpu at time of entry into audit subsystem
*/
udecl_simple_lock_data(,audbuf_lock)
#define	AUDBUF_LOCK()       usimple_lock(&audbuf_lock)
#define	AUDBUF_UNLOCK()     usimple_unlock(&audbuf_lock)


/* build an audit record
   return ptr to offset into audit buffer, or # bytes flushed
*/
audit_rec_build ( code, u_ap_a, error, result, buf_indx, op, param_fld )
u_int code;                     /* syscall code                 */
long *u_ap_a;                   /* ptr to parameters            */
int error;                      /* syscall op error             */
long result;                    /* syscall res; ptr update      */
int *buf_indx;                  /* for use by audgen only       */
u_int op;                       /* operations:
                                    AUD_VNO: store vnode info into u area
                                    AUD_HDR: build header
                                    AUD_LCL: local record for user only
                                    AUD_PRM: add parameters
                                    AUD_PTR: update buffer ptr (audgen)
                                    AUD_RES: add lengths; unlock buffer
                                    AUD_LCK: unlock buffer
                                    AUD_FLU: explicit flush
                                    AUD_GEN: audgen() usage     */
char *param_fld;                /* replace sysent.aud_param with this field */
{
    static int sleeping = 0;    /* must be locked               */
    static u_int hostaddr = 0;

    struct sysent *sysentp;     /* ptr to sysent structure      */
    u_int adb_siz;              /* kernel audit buffer size     */
    int vno_indx_l = 0;         /* current u_vno* index         */
    int narg;                   /* # args to syscall event      */
    u_int *param_p;             /* ptr to arg encodings         */
    u_int aud_param_lcl[64];    /* encoding override (64 max)   */

    register struct nameidata *ndp = &u.u_nd;

    register struct ifnet *ifp;     /* used to get host address */
    register struct ifaddr *ifa;    /* used to get host address */
    struct sockaddr_in *sinp;       /* used to get host address */
    register struct vnode *vp;      /* used to get vnode info   */
    struct file *fp;                /* used to get vnode info   */
    struct stat sb;                 /* used to get vnode info   */
    struct timeval tp;              /* timestamp                */

    struct semid_ds *semid;
    struct shmid_ds *shmid;

#ifdef __alpha
    u_int aud_version = AUD_VERSION | AUD_VERS_LONG;
#else
    u_int aud_version = AUD_VERSION;
#endif /* __alpha */

    int s;
    int ab;         /* audit_data buffer used for this record      */
    int len;        /* for use in INSERT_AUD macros                */
    int cpu_no = cpu_number();

    struct  ipc_perm *ipc_perm;
    struct  msghdr *msghdr;
    u_long  retval = 0;
    dev_t   dev;
    mode_t  mode;
    uid_t   uid;
    gid_t   gid;
    pid_t   pid;

    struct  exportfsdata *exptr;    /* used for exportfs        */
    u_int   naddrs;
    int     i, j, k, l;


/* Y_ZONE is upper limit (bytes) for current AUD0 and AUD5 macro insertions */
#define Y_ZONE 128


/* ----- macros to insert data into audit record buffer: start ----- */
#define SIZ_TOKEN 1

/* insert element type (I_what1) value (I_what2) */
/* no boundary check; guaranteed to have sufficient room */
#define INSERT_AUD0(I_what1,I_what2) \
    { \
        audit_data[ab][a_d_len[ab]] = I_what1;\
        a_d_len[ab] += SIZ_TOKEN;\
        bcopy ( (I_what2), &audit_data[ab][a_d_len[ab]], sizeof *(I_what2) );\
        a_d_len[ab] += sizeof *(I_what2);\
    }

/* insert element type (I_what1) value (I_what2) - with boundary check */
#define INSERT_AUD0b(I_what1,I_what2) \
    { \
        if ( a_d_len[ab] + sizeof *(I_what2) + SIZ_TOKEN*2 + sizeof(int) >= adb_siz )\
            aud_overflow_hndlr(ab,0);\
        if ( a_d_len[ab] + sizeof *(I_what2) + SIZ_TOKEN*2 + sizeof(int) < adb_siz ) {\
            audit_data[ab][a_d_len[ab]] = I_what1;\
            a_d_len[ab] += SIZ_TOKEN;\
            bcopy ( (I_what2), &audit_data[ab][a_d_len[ab]], sizeof *(I_what2) );\
            a_d_len[ab] += sizeof *(I_what2);\
        }\
    }

/* insert null-term string, type (I_what1) value (I_what2) */
#define INSERT_AUD1(I_what1,I_what2) \
    { \
        for ( len = 0; *((char *)(I_what2)+len); len++ ); \
        if ( a_d_len[ab] + sizeof len + len + SIZ_TOKEN*2 + sizeof(int) >= adb_siz )\
            aud_overflow_hndlr(ab,0);\
        if ( a_d_len[ab] + sizeof len + len + SIZ_TOKEN*2 + sizeof(int) < adb_siz ) {\
            audit_data[ab][a_d_len[ab]] = I_what1;\
            a_d_len[ab] += SIZ_TOKEN;\
            bcopy ( &len, &audit_data[ab][a_d_len[ab]], sizeof len );\
            a_d_len[ab] += sizeof len;\
            bcopy ( (char *)(I_what2), &audit_data[ab][a_d_len[ab]], len );\
            a_d_len[ab] += len;\
        }\
    }

/* insert null-term user space string, type (I_what1) value (I_what2) */
#define INSERT_AUD2(I_what1,I_what2) \
    { \
        if ( copyinstr ( (I_what2), &audit_data[ab][a_d_len[ab]+sizeof len+SIZ_TOKEN],\
        ( adb_siz-(a_d_len[ab]+sizeof len+SIZ_TOKEN*2+sizeof(int)) ), &len ) == 0 ) {\
            if ( len > 0 ) {\
                audit_data[ab][a_d_len[ab]] = I_what1;\
                a_d_len[ab] += SIZ_TOKEN;\
                bcopy ( &len, &audit_data[ab][a_d_len[ab]], sizeof len );\
                a_d_len[ab] += (len + sizeof len);\
            }\
        }\
    }

/* insert (I_len2) size element and size value, type (I_what1) value (I_what2) */
#define INSERT_AUD3(I_what1,I_what2,I_len2) \
    { \
        if ( a_d_len[ab] + sizeof (I_len2) + (I_len2) + SIZ_TOKEN*2 + sizeof(int) >= adb_siz )\
            aud_overflow_hndlr(ab,0);\
        if ( a_d_len[ab] + sizeof (I_len2) + (I_len2) + SIZ_TOKEN*2 + sizeof(int) < adb_siz ) {\
            audit_data[ab][a_d_len[ab]] = I_what1;\
            a_d_len[ab] += SIZ_TOKEN;\
            bcopy ( &(I_len2), &audit_data[ab][a_d_len[ab]], sizeof (I_len2) );\
            a_d_len[ab] += sizeof (I_len2);\
            bcopy ( (I_what2), &audit_data[ab][a_d_len[ab]], (I_len2) );\
            a_d_len[ab] += (I_len2);\
        }\
    }

/* insert (I_len2) size user-space element and size value, type (I_what1) value (I_what2)
   zero I_len2 on error
*/
#define INSERT_AUD4(I_what1,I_what2,I_len2) \
    { \
        if ( a_d_len[ab] + sizeof (I_len2) + (I_len2) + SIZ_TOKEN*2 + sizeof(int) >= adb_siz )\
            aud_overflow_hndlr(ab,0);\
        if ( a_d_len[ab] + sizeof (I_len2) + (I_len2) + SIZ_TOKEN*2 + sizeof(int) < adb_siz ) {\
            audit_data[ab][a_d_len[ab]] = I_what1;\
            a_d_len[ab] += SIZ_TOKEN;\
            bcopy ( &(I_len2), &audit_data[ab][a_d_len[ab]], sizeof (I_len2) );\
            a_d_len[ab] += sizeof (I_len2);\
            if ( (I_len2) > 0 ) {\
                if ( copyin ( (I_what2), &audit_data[ab][a_d_len[ab]], (I_len2) ) ) {\
                    a_d_len[ab] -= (sizeof (I_len2) + SIZ_TOKEN);\
                    (I_len2) = 0;\
                }\
                else a_d_len[ab] += (I_len2);\
            }\
        }\
	else (I_len2) = 0;\
    }

/* insert (I_len2) size user-space element, type (I_what1) value (I_what2) */
/* no boundary check; guaranteed to have sufficient room */
#define INSERT_AUD5(I_what1,I_what2,I_len2) \
    { \
        audit_data[ab][a_d_len[ab]] = I_what1;\
        if ( copyin ( (I_what2), &audit_data[ab][a_d_len[ab]+SIZ_TOKEN], (I_len2) ) == 0 )\
            a_d_len[ab] += (I_len2) + SIZ_TOKEN;\
    }

/* insert element type (I_what1) value (I_what2) at (I_where) */
#define INSERT_AUD6(I_where,I_what1,I_what2) \
    { \
        if ( (I_where) + sizeof *(I_what2) + SIZ_TOKEN*2 + sizeof(int) >= adb_siz )\
            aud_overflow_hndlr(ab,0);\
        if ( (I_where) + sizeof *(I_what2) + SIZ_TOKEN*2 + sizeof(int) < adb_siz ) {\
            audit_data[ab][(I_where)] = I_what1;\
            bcopy ( (I_what2), &audit_data[ab][(I_where)+SIZ_TOKEN], sizeof *(I_what2) );\
        }\
    }


#ifdef SEC_MAC
#define INSERT_SECLVL(seclvl) { \
    /* xxx_LOCK(); */ INSERT_AUD0 ( TP_SLEVEL, seclvl ); /* xxx_UNLOCK(); */ \
}
#else
#define INSERT_SECLVL(seclvl)
#endif /* SEC_MAC */

#ifdef SEC_ILB
#define INSERT_ILBLVL(seclvl) { \
    /* xxx_LOCK(); */ INSERT_AUD0 ( TP_ILEVEL, seclvl ); /* xxx_UNLOCK(); */ \
}
#else
#define INSERT_ILBLVL(seclvl)
#endif /* SEC_MAC */
/* ----- macros to insert data into audit record buffer: end ----- */


    /* set sysent pointer; 'code' is validated in syscall_trap() or audgen() */
    if ( code < nsysent ) sysentp = &sysent[code];
#if BIN_COMPAT
    else if ( i = hbval(code) )
        sysentp = (* habitats[i]->cm_syscall)(code&~(u_long)HABITAT_MASK);
#endif /* BIN_COMPAT */
    else sysentp = &sysent[0]; /* nosys */


    /* set ptr to param field encodings, set narg; use sysent struct as default
       these encodings control how the data on the stack is parsed and placed
       into the audit record
    */
    if ( param_fld == (char *)0 ) {
        param_p = (u_int *)sysentp->aud_param;
        narg = sysentp->sy_narg;
    }
    else {
        /* expand param_fld to int array */
        for ( narg = 0; narg <= sizeof(aud_param_lcl) && param_fld[narg]; narg++ )
            aud_param_lcl[narg] = param_fld[narg];
        param_p = aud_param_lcl;
    }


    /* load vnode information for descriptors into u_area.
       if the event referenced an object by descriptor, it is necessary
       to get more information on that object for the audit record
    */
    if ( op & AUD_VNO )
        for ( i = 0; (i < narg) && (u.u_vno_indx < AUD_VNOMAX); i++ )
            if ( param_p[i] == 'C' || param_p[i] == 'c' )
                if ( getf(&fp, u_ap_a[i], FILE_FLAGS_NULL, &u.u_file_state) == 0 ) {
                    if ( (vp = (struct vnode *)fp->f_data) && fp->f_type == DTYPE_VNODE ) {
                        if ( vn_stat ( vp, &sb ) ) break;
                        u.u_vno_dev[u.u_vno_indx]  = S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode) ?
                            sb.st_rdev : sb.st_dev;
                        u.u_vno_num[u.u_vno_indx]  = sb.st_ino;
                        u.u_vno_mode[u.u_vno_indx] = sb.st_mode;
#ifdef notyet
                        /* if not modifying, check for preselection */
                        if ( (fp->f_flag&FWRITE) == 0 )
                            u.u_vno_aud[u.u_vno_indx] = sb.st_flags&INOAUDIT ? '1' : '\0';
#endif /* notyet */
                        u.u_vno_indx++;
                    }
                    FP_UNREF(fp);
                }


    /* --- syscall specifics section: start --- */
#ifdef notyet
    switch ( code ) {

    /* special case of preselection in open(2) for r/w modes */
    case SYS_sysv_open|SVID2_HAB_NO:
    case SYS_old_open:
    case SYS_open:  if ( u.u_vno_aud[0] ) u.u_vno_aud[0] = (u_ap_a[1]&O_ACCMODE)+'1';
                    break;

    }
#endif /* notyet */
    /* --- syscall specifics section: end --- */


#ifdef notyet
    /* preselect out event based on <syscall,object audmode> (use all object refs) */
    for ( i = j = 0; i < u.u_vno_indx; i++ )
        if ( u.u_vno_aud[i] && ( (param_p[NUMSYSCALLARGS]&u.u_vno_aud[i]) == u.u_vno_aud[i] ) )
            j++;
    if ( i && j==i ) return(0);
#endif /* notyet */


    /* get timestamp before any sleeping could occur */
    s = splhigh();
    TIME_READ_LOCK(); tp = time; TIME_READ_UNLOCK();
    splx(s);


    /* use audit_data buffer according to cpu_no on which event occurred.
       audbuf_mask used to indicate which audit_data buffers are in use.
       SMP: use lk_audbuf to lock audbuf_mask and sleeping flag.

       if AUD_GEN flag set, buffer to use will be specified.
    */
    if ( (op & AUD_GEN) == 0 || *buf_indx == -1 ) ab = cpu_no & 0xff;
    else ab = *buf_indx;

    /* set size of audit data buffer; this is used for boundary checking
       in the INSERT_AUD* macros and the logic to implicitly flush data.
       only audgen() can set ab to NCPUS; this is a dedicated buffer for
       use by auditd; necessary to prevent deadlock.  assumption: auditd
       audit record required less than AUDITD_RECSZ bytes.
    */
    if ( ab != NCPUS ) adb_siz = (audsize*1024) < (AUD_BUF_SIZ<<2) ? (AUD_BUF_SIZ<<2) : (audsize*1024);
    else adb_siz = AUDITD_RECSZ;


    /* get audit buffer */
    if ( op & AUD_HDR ) {
        AUDBUF_LOCK();
        for ( ;; ) {
            if ( audbuf_mask[ab] == 0x0 ) {
                audbuf_mask[ab] = 0x1;
                AUDBUF_UNLOCK();
                break;
            }
            else {
                sleeping = 1;
                /* no timeout, no signals, no error */
                mpsleep ( (caddr_t)&sleeping, PZERO, "audit", 0,
                    simple_lock_addr(audbuf_lock), MS_LOCK_SIMPLE );
            }
        }
        if ( buf_indx ) *buf_indx = ab;
    }


    /* initialize cpu-specific audit buffer; get hostaddr */
    if ( initdone[ab] == 0 ) {
        initdone[ab]++;
        audit_data[ab] = kalloc ( adb_siz );
        if ( audit_data[ab] == NULL ) panic ( "kern_audit: no mem" );
        if ( hostaddr == 0 ) {
            /* smp: may need to lock here */
            for ( ifp = ifnet; ifp; ifp = ifp->if_next ) {
                if ( ifp->if_flags&IFF_LOOPBACK || (ifp->if_flags&IFF_UP == 0) )
                    continue;
                for ( ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next )
                    if ( ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET ) {
                        sinp = (struct sockaddr_in *)ifa->ifa_addr;
                        hostaddr = sinp->sin_addr.s_addr;
                    }
            }
        }
    }


    /* build header - length, auid, hostaddr, event, uid, pid, ....   
       guaranteed to have sufficient room for header in current buffer
       guaranteed to have AUD_BUF_SIZ bytes
    */
    if ( op & AUD_HDR ) {
	struct session *sess;
	dev_t ldev;
        a_d_len[ab] += (sizeof(int) + SIZ_TOKEN); /* leave space for TP_LENGTH and length */
        INSERT_AUD0 ( TP_VERSION, &aud_version );
        INSERT_AUD0 ( TP_AUID, &u.u_procp->p_auid );
        INSERT_AUD0 ( TP_RUID, &u.u_ruid );
        INSERT_AUD0 ( TP_HOSTADDR, &hostaddr );
#if BIN_COMPAT
        if ( i = hbval(code) ) {
            INSERT_AUD1 ( TP_EVENTP, habitats[i]->call_name[code&~(u_long)HABITAT_MASK] );
            INSERT_AUD1 ( TP_HABITAT, habitats[i]->cm_name );
        }
        else
#endif /* BIN_COMPAT */
            INSERT_AUD0 ( TP_EVENT, &code );
        INSERT_AUD0 ( TP_UID, &u.u_uid );
        INSERT_AUD0 ( TP_PID, &u.u_procp->p_pid );
        INSERT_AUD0 ( TP_PPID, &u.u_procp->p_ppid );
	sess = u.u_procp->p_session;
	SESS_LOCK(sess);
        if (sess->s_ttyvp ) {
	    ldev = sess->s_ttyvp->v_rdev;
	    SESS_UNLOCK(sess);
            INSERT_AUD0 ( TP_DEV, &ldev );
	} else SESS_UNLOCK(sess);
        INSERT_AUD0 ( TP_NCPU, &cpu_no );
        INSERT_AUD0 ( TP_TV_SEC, &tp.tv_sec );
        INSERT_AUD0 ( TP_TV_USEC, &tp.tv_usec );
        if ( i = u.u_set_uids - u.u_set_uids_snap )
            INSERT_AUD0 ( TP_SET_UIDS, &i );
        INSERT_SECLVL ( &SIP->si_pslevel );
        INSERT_ILBLVL ( &SIP->si_pilevel );
        retval = (u_long)&audit_data[ab][a_d_len[ab]];
    }


    /* copy event-dependent parameters into record */
    for ( i = 0; (op & AUD_PRM) && i < narg; i++ ) {

        /* sanity check here instead of AUD0 and AUD5 macros - optimization.
           Y_ZONE exceeds max size for current AUD0 or AUD5 macro insertions.
        */
        if ( a_d_len[ab] + Y_ZONE >= adb_siz ) aud_overflow_hndlr(ab,0);

        switch ( param_p[i] ) {


        /* nop */
        case '0':   break;


        /* mode_t parameter masked with umask */
        case '1':   mode = u_ap_a[i] & ~u.u_cmask;
                    INSERT_AUD0 ( T_MODE, &mode );
                    break;


        /* mode_t parameter */
        case '2':   mode = (mode_t)u_ap_a[i]; /* to force into a mode_t */
                    INSERT_AUD0 ( T_MODE, &mode );
                    break;


        /* address (long) */
        case '3':   INSERT_AUD0 ( TP_LONG, &u_ap_a[i] );
                    break;


        /* ptr to int */
        case '4':   INSERT_AUD5 ( T_INT, u_ap_a[i], sizeof(int) );
                    break;


        /* ptr to integer array */
        case '5':   k = u_ap_a[i-1] * sizeof(int);
                    INSERT_AUD4 ( TP_INTP, u_ap_a[i], k );
                    break;


        /* integer parameter */
        case 'A':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'a':   k = (int)u_ap_a[i]; /* to force into an INT */
                    INSERT_AUD0 ( T_INT, &k );
                    break;


        /* ptr to char array (user space) */
        case 'B':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'b':   INSERT_AUD2 ( T_CHARP, u_ap_a[i] );
                    if ( u.u_vno_indx && vno_indx_l < u.u_vno_indx ) {
                        INSERT_AUD0 ( TP_VNODE_DEV, &u.u_vno_dev[vno_indx_l] );
                        INSERT_AUD0 ( TP_VNODE_ID, &u.u_vno_num[vno_indx_l] );
                        INSERT_AUD0 ( TP_VNODE_MODE, &u.u_vno_mode[vno_indx_l] );
                        vno_indx_l++;
                    }
                    break;


        /* descriptors */
        case 'C':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'c':   k = (int)u_ap_a[i]; /* to force into an INT */
                    INSERT_AUD0 ( T_DESCRIP, &k );
                    if ( u.u_vno_indx && vno_indx_l < u.u_vno_indx ) {
                        INSERT_AUD0 ( TP_VNODE_DEV, &u.u_vno_dev[vno_indx_l] );
                        INSERT_AUD0 ( TP_VNODE_ID, &u.u_vno_num[vno_indx_l] );
                        INSERT_AUD0 ( TP_VNODE_MODE, &u.u_vno_mode[vno_indx_l] );
                        vno_indx_l++;
                    }
                    break;


        /* ptr to string of size specified in next param */
        case 'G':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'g':   k = u_ap_a[i+1];
                    INSERT_AUD4 ( T_CHARP, u_ap_a[i], k );
                    break;


        /* pointer to socket */
        case 'H':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'h':   k = u_ap_a[i+1];
                    INSERT_AUD4 ( T_SOCK, u_ap_a[i], k );
                    break;


        /* pointer to msghdr */
        case 'I':
        case 'T':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'i':
        case 't':   if ( !(msghdr = (struct msghdr *)u_ap_a[i]) ) break;
                    if ( copyin ( &msghdr->msg_namelen, &k, sizeof(int) ) == 0 && k )
                        INSERT_AUD4 ( TP_MSGHDR, msghdr->msg_name, k );
                    /* in BSD4.4, offset past cmshdr info */
                    k = param_p[i] == 'T' || param_p[i] == 't' ? sizeof(struct cmsghdr) : 0;
                    l = 0;
                    if ( copyin ( &msghdr->msg_controllen, &l, sizeof(int) ) == 0 && l ) {
                        l -= k;
                        INSERT_AUD4 ( TP_ACCRGHT, msghdr->msg_control+k, l );
                        l += k;

                        /* pick up access rights */
                        for ( ; k < l; k += sizeof(int) ) {
                            if ( copyin ( msghdr->msg_control+k, &j, sizeof(int) ) )
                                continue;
                            if ( getf ( &fp, j, FILE_FLAGS_NULL, &u.u_file_state) == 0 ) {
                                if ( (vp = (struct vnode *)fp->f_data) && fp->f_type == DTYPE_VNODE ) {
                                    if ( vn_stat ( vp, &sb ) ) break;
                                    /* insert data; perform boundary checks */
                                    INSERT_AUD0b ( TP_VNODE_DEV, S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode) ?
                                        &sb.st_rdev : &sb.st_dev );
                                    INSERT_AUD0b ( TP_VNODE_ID, &sb.st_ino );
                                    INSERT_AUD0b ( TP_VNODE_MODE, &sb.st_mode );
                                }
                                FP_UNREF(fp);
                            }
                        }
                    }
                    break;


        /* pointer to msqid_ds */
        case 'K':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'k':   if ( u_ap_a[i-1] != IPC_SET ) break;
                    INSERT_AUD5 ( TP_IPC_UID, &((struct msqid_ds *)u_ap_a[i])->msg_perm.uid, sizeof(ipc_perm->uid) );
                    INSERT_AUD5 ( TP_IPC_GID, &((struct msqid_ds *)u_ap_a[i])->msg_perm.gid, sizeof(ipc_perm->gid) );
                    INSERT_AUD5 ( TP_IPC_MODE, &((struct msqid_ds *)u_ap_a[i])->msg_perm.mode, sizeof(ipc_perm->mode) );
                    break;


        /* uid_t parameter */
        case 'L':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'l':   uid = (uid_t)u_ap_a[i]; /* to force into a uid_t */
                    INSERT_AUD0 ( T_UID, &uid );
                    break;


        /* gid_t parameter */
        case 'M':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'm':   gid = (gid_t)u_ap_a[i]; /* to force into a gid_t */
                    INSERT_AUD0 ( T_GID, &gid );
                    break;


        /* pointer to semid_ds */
        case 'N':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'n':   if ( u_ap_a[i-1] != IPC_SET ) break;
                    if ( !(semid = (struct semid_ds *)u_ap_a[i]) ) break;
                    INSERT_AUD5 ( TP_IPC_UID, &semid->sem_perm.uid, sizeof(ipc_perm->uid) );
                    INSERT_AUD5 ( TP_IPC_GID, &semid->sem_perm.gid, sizeof(ipc_perm->gid) );
                    INSERT_AUD5 ( TP_IPC_MODE, &semid->sem_perm.mode, sizeof(ipc_perm->mode) );
                    break;


        /* pointer to shmid_ds */
        case 'O':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'o':   if ( u_ap_a[i-1] != IPC_SET ) break;
                    if ( !(shmid = (struct shmid_ds *)u_ap_a[i]) ) break;
                    INSERT_AUD5 ( TP_IPC_UID, &shmid->shm_perm.uid, sizeof(ipc_perm->uid) );
                    INSERT_AUD5 ( TP_IPC_GID, &shmid->shm_perm.gid, sizeof(ipc_perm->gid) );
                    INSERT_AUD5 ( TP_IPC_MODE, &shmid->shm_perm.mode, sizeof(ipc_perm->mode) );
                    break;


        /* device */
        case 'P':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'p':   dev = (dev_t)u_ap_a[i]; /* to force into a dev_t */
                    INSERT_AUD0 ( T_DEV, &dev );
                    break;


        /* pid_t parameter */
        case 'Q':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'q':   pid = (pid_t)u_ap_a[i]; /* to force into a pid_t */
                    INSERT_AUD0 ( T_PID, &pid );
                    break;


        /* ptr to char array (kernel space) -- used for execv[e] */
        case 'R':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'r':   if ( u_ap_a[i] == NULL ) break;
                    INSERT_AUD1 ( T_CHARP, u_ap_a[i] );
                    if ( u.u_vno_indx && vno_indx_l < u.u_vno_indx ) {
                        INSERT_AUD0 ( TP_VNODE_DEV, &u.u_vno_dev[vno_indx_l] );
                        INSERT_AUD0 ( TP_VNODE_ID, &u.u_vno_num[vno_indx_l] );
                        INSERT_AUD0 ( TP_VNODE_MODE, &u.u_vno_mode[vno_indx_l] );
                        vno_indx_l++;
                    }
                    break;


        /* array of 2 descriptors (socketpair) */
        case 'S':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 's':   for ( k = 0; k < 2; k++ )
                        INSERT_AUD5 ( T_DESCRIP, &((int *)u_ap_a[i])[k], sizeof(int) );
                    break;


        /* exportfs operation */
        case 'V':   INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                    INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
        case 'v':   if ( u_ap_a[0] == EXPORTFS_READ ) break;
                    if ( !(exptr = (struct exportfsdata *)u_ap_a[i]) ) break;

                    INSERT_AUD2 ( T_CHARP, exptr->e_path );
                    if ( u_ap_a[0] == EXPORTFS_REMOVE ) break;

                    /* audit flags, rootmap, anon */
                    INSERT_AUD5 ( T_INT, &exptr->e_flags, sizeof(int) );
                    INSERT_AUD5 ( T_UID, &exptr->e_rootmap, sizeof(uid_t) );
                    INSERT_AUD5 ( T_UID, &exptr->e_anon, sizeof(uid_t) );

                    /* audit address vectors */
                    if ( copyin ( &exptr->e_writeaddrs.naddrs, &naddrs, sizeof naddrs ) == 0 && naddrs ) {
                        k = naddrs * sizeof(struct sockaddr);
                        INSERT_AUD4 ( TP_ADDRVEC, exptr->e_writeaddrs.addrvec, k );
                    }
                    if ( copyin ( &exptr->e_rootaddrs.naddrs, &naddrs, sizeof naddrs ) == 0 && naddrs ) {
                        k = naddrs * sizeof(struct sockaddr);
                        INSERT_AUD4 ( TP_ADDRVEC, exptr->e_rootaddrs.addrvec, k );
                    }
                    break;


        /* ptr to filename (user space) + fetch vnode info for returned fd
             use this only for object creations, where namei can't provide
             necessary vnode information
        */
        case 'W':
        case 'w':   INSERT_AUD2 ( T_CHARP, u_ap_a[i] );
                    if ( u.u_vno_indx && vno_indx_l < u.u_vno_indx ) {
                        INSERT_AUD0 ( TP_VNODE_DEV, &u.u_vno_dev[vno_indx_l] );
                        INSERT_AUD0 ( TP_VNODE_ID, &u.u_vno_num[vno_indx_l] );
                        INSERT_AUD0 ( TP_VNODE_MODE, &u.u_vno_mode[vno_indx_l] );
                        vno_indx_l++;
                    }
                    else if ( error == 0 &&
                    getf(&fp, result, FILE_FLAGS_NULL, &u.u_file_state) == 0 ) {
                        if ( (vp = (struct vnode *)fp->f_data) && fp->f_type == DTYPE_VNODE ) {
                            if ( vn_stat ( vp, &sb ) == 0 ) {
                                j = S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode) ?
                                    sb.st_rdev : sb.st_dev;
                                INSERT_AUD0 ( TP_VNODE_DEV, &j );
                                INSERT_AUD0 ( TP_VNODE_ID, &sb.st_ino );
                                INSERT_AUD0 ( TP_VNODE_MODE, &sb.st_mode );
                            }
                        }
                        FP_UNREF(fp);
                    }
                    if ( param_p[i] == 'W' ) {
                        INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                        INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
                    }
                    break;


        /* ptr to filename (user space) + fetch vnode info via namei
             use this only for object creations, where namei from syscall
             can't provide necessary vnode information
        */
        case 'X':
        case 'x':   INSERT_AUD2 ( T_CHARP, u_ap_a[i] );
                    if ( error == 0 && u.u_vno_indx == 0 ) {
                        ndp->ni_nameiop = LOOKUP;
                        ndp->ni_segflg = UIO_USERSPACE;
                        ndp->ni_dirp = (char *)u_ap_a[i];
                        if ( namei(ndp) == 0 ) /* namei stores vnode data in uthread */
                            vrele(ndp->ni_vp);
                    }
                    if ( param_p[i] == 'X' ) {
                        INSERT_SECLVL ( &u.u_obj_slevel[vno_indx_l] );
                        INSERT_ILBLVL ( &u.u_obj_ilevel[vno_indx_l] );
                    }
                    if ( u.u_vno_indx && vno_indx_l < u.u_vno_indx ) {
                        INSERT_AUD0 ( TP_VNODE_DEV, &u.u_vno_dev[vno_indx_l] );
                        INSERT_AUD0 ( TP_VNODE_ID, &u.u_vno_num[vno_indx_l] );
                        INSERT_AUD0 ( TP_VNODE_MODE, &u.u_vno_mode[vno_indx_l] );
                        vno_indx_l++;
                    }
                    break;

        }
    }


    /* update buffer ptr; used from audgen() */
    if ( op & AUD_PTR ) a_d_len[ab] += (u_int)result;


    /* insert lengths into audit record */
    if ( op & (AUD_RES|AUD_LCL) ) {
        INSERT_AUD0 ( T_ERRNO, &error );
        INSERT_AUD0 ( T_RESULT, &result );
        a_d_len[ab] += (sizeof(int)+SIZ_TOKEN); /* initial TP_LENGTH token and value */
        i = a_d_ptr[ab];
        j = a_d_len[ab] - a_d_ptr[ab];
        INSERT_AUD6 ( i, TP_LENGTH, &j );
        i = a_d_len[ab]-(sizeof(int)+SIZ_TOKEN); /* final TP_LENGTH token and value */
        INSERT_AUD6 ( i, TP_LENGTH, &j );
        /* advance a_d_ptr (unless destined for userspace) */
        if ( (op & AUD_LCL) == 0 ) a_d_ptr[ab] = a_d_len[ab];
        retval = (u_long)&audit_data[ab][a_d_ptr[ab]];
    }


    /* implicit flush audit buffer to /dev/audit
       bypass if AUD_GEN, unless also AUD_RES set
    */
    if ( a_d_len[ab] >= adb_siz-AUD_BUF_SIZ && ((op & AUD_RES) || ((op & AUD_GEN) == 0)) ) {
        kernaudwrite ( audit_data[ab], a_d_len[ab], 0 );
        retval = a_d_len[ab];
        a_d_len[ab] = a_d_ptr[ab] = 0;
    }


    /* unlock the updated audit buffer */
    if ( op & (AUD_RES|AUD_LCK) ) {
        /* if destinged for userspace, reset a_d_len */
        if ( op & AUD_LCK ) a_d_len[ab] = a_d_ptr[ab];
        AUDBUF_LOCK();
        audbuf_mask[ab] = 0x0;
        if ( sleeping ) {
            sleeping = 0;
            wakeup ( (caddr_t)&sleeping );
        }
        AUDBUF_UNLOCK();
    }


    /* explicit flush audit buffer to /dev/audit */
    if ( op & AUD_FLU ) {
        for ( i = j = 0; i < NCPUS; i++ ) {
            if ( initdone[i] ) {
                AUDBUF_LOCK();
                if ( audbuf_mask[i] == 0x0 ) {
                    audbuf_mask[i] = 0x1;
                    AUDBUF_UNLOCK();
                    kernaudwrite ( audit_data[i], a_d_len[i], 1 );
                    j += a_d_len[i];
                    a_d_len[i] = a_d_ptr[i] = 0;
                    audbuf_mask[i] = 0;
                }
                else AUDBUF_UNLOCK();
            }
        }
        retval = j;
        AUDBUF_LOCK();
        if ( sleeping ) {
            sleeping = 0;
            wakeup ( (caddr_t)&sleeping );
        }
        AUDBUF_UNLOCK();
    }


    return ( retval );
}


/* handle overflow situation where audit record won't fit in current buffer.
   flush audit buffer and reposition current record in the buffer.
   NOTE: this situation occurs relatively infrequently.
*/
aud_overflow_hndlr ( indx, top )
int indx;                       /* index of audit data buffer, pointers */
int top;            /* top of buffer to be repositioned; used in audgen */
{
    kernaudwrite ( audit_data[indx], a_d_ptr[indx], 0 );
    ovbcopy ( &audit_data[indx][a_d_ptr[indx]], audit_data[indx],
    top ? (top-a_d_ptr[indx]) : (a_d_len[indx]-a_d_ptr[indx]) );

#ifdef AUD_DEBUG
    if ( top ) audgen_overflow++;
    else audrec_overflow++;
#endif /* AUD_DEBUG */

    a_d_len[indx] = a_d_len[indx] - a_d_ptr[indx];
    a_d_ptr[indx] = 0;
}
