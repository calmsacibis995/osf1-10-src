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
static char *rcsid = "@(#)$RCSfile: audit_tool.c,v $ $Revision: 1.1.4.22 $ (DEC) $Date: 1993/12/20 21:21:57 $";
#endif

/* Audit Reduction Tool
    ULTRIX V4.x:    -Dultrix -laud /sys/`machine`/BINARY/syscalls.o
    DEC OSF/1:      -D__osf__ -D_BSD -DCOMPAT_43 -laud

    debug - blk maps, proc lists, parsing:  -D__DEBUG1
*/

/*
   To handle new token in audit.h:
    1) update audit_fields struct
    2) update init_audit_fields()
    3) update appropriate output_*() routine
    4) update parse_rec()

   To add a new post-selection parameter:
    1) update selectn struct
    2) update main()'s command line processing
    3) update init_selectn()
    4) update interact()
    5) update match_rec()

   To handle new syscall:
    1) to get special (non-default) output processing, update output_param_fmt()
    2) if syscall affects process state, update state_maint()
*/

/*
    This is NOT a complete calling tree, but to give the general idea...

    main()
    {
        init_selectn -      establish selection parameters
        build_ruleset -     establish deselection parameters
        audit_reduce -      does it all
            fetch_matching_rec -    gets the next record of interest
                fetch_hdr -             gets the .hdr file when opening a new log
                fetch_rec -             gets the next record
                parse_rec -             parses the record
                state_maint -           updates state tables
                match_rec -             determines if record is requested
                deselect -              determines if record is requested
            output_rec_fmt -        output the record
                output_param_fmt -      outputs event-specific information
                    output_option -         outputs option string for given event
    }

*/


#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/audit.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/conf.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <strings.h>
#include <syscall.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <varargs.h>
#include <pwd.h>
#include <grp.h>
#include <utmp.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/systm.h>
#include <sys/ptrace.h>
#define ULT_BIN_COMPAT  1
#include <sys/sysinfo.h>
#ifdef __osf__
#include <sys/fs_types.h>
#include <sys/table.h>
#include <sys/kloadcall.h>
#include <sys/sysconfig.h>
#include <sys/security.h>
#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <netns/ns.h>
#include <sys/swap.h>
#include <sys/systeminfo.h>
#endif __osf__
#ifdef ultrix
#include <sys/limits.h>
#include <nlist.h>
#include <sys/cpudata.h>
#include <fs/nfs/nfs_clnt.h>
#include <fs/nfs/vfs.h>
#endif /* ultrix */

#ifdef __osf__
typedef pid_t pid_l;
typedef uid_t auid_l;
typedef int cpu_l;
#endif /* __osf__ */
#ifdef ultrix
typedef short pid_l;
typedef long  auid_l;
typedef short cpu_l;
#endif /* ultrix */
typedef u_int ipaddr_l;
typedef struct _long64 { u_int val[2]; } long64;

#define HOST_LEN        MAXHOSTNAMELEN
#define MAX_RULE_SIZ    4096
#define NO_FAIL         0x01
#define NO_SUCC         0x02
#define NO_CARE         0x04
#define NUM_FDP         ((OPEN_MAX_SYSTEM-1)/NOFILE+1)
#define NUM_FDS         OPEN_MAX_SYSTEM
#define NRULESETS       32
#define N_SELECT        ((NUM_SYSCALLS+N_TRUSTED_EVENTS)*2)
#define N_SELECT2       8
#ifndef OPEN_MAX_SYSTEM
#define OPEN_MAX_SYSTEM NOFILE
#endif  /* OPEN_MAX_SYSTEM */
#define RULE(i,x)       rules[i/NRULESETS]->x[i%NRULESETS]
#define RULES_IN_SET    32
#define STR_LEN         1024
#define STR_LEN2        AUD_MAXEVENT_LEN
#define TIME_LEN        13

#ifdef  __osf__
#define AUD_NPARAM      NUMSYSCALLARGS
#define AUDIT_SHUTDOWN  AUDIT_STOP
#define NUM_SYSCALLS    N_SYSCALLS
#define ATOL(x)         strtoul(x,(char **)NULL,10)
#endif  /* __osf__ */
#ifdef  ultrix
#define LAST            (NUM_SYSCALLS-1)
#define MAX_SPECIAL     263
#define MIN_SPECIAL     260
#define NUM_SYSCALLS    (257+1)
                    /* based on # entries in syscalls.c */
#define AUD_VERS_LONG   0x8000
#define AUID_INVAL      0
#define NUMSYSCALLARGS  AUD_NPARAM
#define MAX_TRUSTED_EVENT   (MIN_TRUSTED_EVENT+N_TRUSTED_EVENTS-1)
#define ATOL(x)         atol(x)
#endif  /* ultrix */


/* token aliases */
#ifdef ultrix
#define TP_NCPU         T_NCPU
#define TP_TV_SEC       T_TV_SEC
#define TP_TV_USEC      T_TV_USEC
#define TP_TZ_MIN       T_TZ_MIN
#define TP_TZ_DST       T_TZ_DST
#define TP_SHORT        T_SHORT
#define TP_VNODE_DEV    T_GNODE_DEV
#define TP_VNODE_ID     T_GNODE_ID
#define TP_IPC_UID      T_IPC_UID
#define TP_IPC_GID      T_IPC_GID
#define TP_IPC_MODE     T_IPC_MODE
#define TP_MSGHDR       T_MSGHDR
#define TP_ACCRGHT      T_ACCRGHT
#define TP_LENGTH       T_LENGTH
#define TP_INTP         T_INTP
#define T_ERRNO         T_ERROR
#endif  /* ultrix */


/* mem alloc/control */
#define MEM_ELMNT       32
#define MEM_NELMNT      1024
#define MEM_NBLKS       1000

/* run-time misc operation flags */
#define FLAG_BINARY     0x0001
#define FLAG_DISPLAY    0x0002
#define FLAG_FOLLOW     0x0004
#define FLAG_OVERRIDE   0x0008
#define FLAG_REPORT     0x0010
#define FLAG_BRIEF      0x0020
#define FLAG_SORT       0x0040
#define FLAG_LOCALID    0x0080
#define FLAG_STAT       0x0100
#define FLAG_XAUDIT     0x0200
#define FLAG_PARSE_DBG  0x0400
#define FLAG_RECOVER    0x0800
#define FLAG_INVERT     0x1000
#define FLAG_FASTMODE   0x2000

#if AUD_BUF_SIZ < 0x10000
#define AUDBUFSIZ_L     0x10000
#else
#define AUDBUFSIZ_L     AUD_BUF_SIZ
#endif /* AUD_BUF_SIZ < 0x10000 */

/* aud_mem_op operations */
#define AMO_FETCH       0
#define AMO_FREE        1
#define AMO_DEBUG       2

/* aud_mem_proc operations */
#define AMP_FREE        0
#define AMP_GETAPROC    1
#define AMP_GETADDR     2
#define AMP_GETNEXT     3
#define AMP_DEBUG       4
#define AMP_DUMP        5

/* state_maint operations */
#define SMA_USER        1
#define SMA_PROC        2
#define SMC_ALL         0
#define SMC_FD          1

/* miscellaneous operations */
#define RCVR_STORE      0
#define RCVR_FETCH      1
#define RCVR_HDRSTR     2
#define RCVR_HDRFETCH   3
#define SORT_CHECK      1
#define SORT_UPDATE     2


/* audit_fields flag values / ascii messages */
char *af_msg[] = {
"ok",
#define AF_BADERRVAL    1
"bad error value",
#define AF_BADLENTOKEN  2
"bad length token",
#define AF_BADRECLEN    3
"bad record length",
#define AF_BADRECLEN2   4
"bad record length (2)",
#define AF_BADRECOVER   5
"bad record recovery",
#define AF_CORRUPTED    6
"corrupted data",
#define AF_LOGBREAK     7
"audit data logging stopped/started",
#define AF_LOGCHNG      8
"audit record changed",
#define AF_RECOVER      9
"audit record recovered",
#define AF_PARTIAL     10
"partial record read"
};


/* miscellaneous functions */
char *aud_mem_op();
struct a_proc *aud_mem_proc();
char *fetch_matching_rec();
char *fetch_rec();
char *gethost_l();
char *itoa();
char *output_option();
char *output_username();
extern void *realloc();
int sig_int1();
u_char *output_64();


#ifdef ultrix
/* special events */
char *special_event[] = {
    "shmget",
    "shmdt",
    "shmctl",
    "shmat",
    "logout"
};
#define SYS_SHMGET MIN_SPECIAL
#define SYS_SHMDT  MIN_SPECIAL+1
#define SYS_SHMCTL MIN_SPECIAL+2
#define SYS_SHMAT  MIN_SPECIAL+3
#define _LOGOUT special_event[4]

struct nlist nlst[] = {
    { "_nsysent" },
#define X_NSYSENT      0
    { "_n_sitevents" },
#define X_N_SITEVENTS  1
    { 0 },
};
#endif /* ultrix */


/* audit record fields
    For compatibility between 32-bit and 64-bit architectures, do not use
    64-bit types; instead, use the long64 typedef.
*/
struct audit_fields {
    auid_l auid;
    dev_t device;
    int error;
    int event;
    short flag;
    int hostid;
    ipaddr_l ipaddr;
    cpu_l n_cpu;
    pid_l pid;
    pid_l ppid;
    long64 result;
    struct timeval timeval;
#ifdef ultrix
    struct timezone timezone;
#endif /* ultrix */
    uid_t uid;
    uid_t ruid;
    int subevent;
    short login_proc;
    u_int version;

    auid_l auid2[AUD_NPARAM];           int auid2_indx;
    ipaddr_l ipaddr2[AUD_NPARAM];       int ipaddr2_indx;
    dev_t device2[AUD_NPARAM];          int device2_indx;
    pid_l pid2[AUD_NPARAM];             int pid2_indx;
    pid_l ppid2[AUD_NPARAM];            int ppid2_indx;
    uid_t uid2[AUD_NPARAM];             int uid2_indx;
    uid_t ruid2[AUD_NPARAM];            int ruid2_indx;
    int event2[AUD_NPARAM];             int event2_indx;
    int subevent2[AUD_NPARAM];          int subevent2_indx;
    char *eventp[AUD_NPARAM];           int eventp_indx;     u_int eventp_len[AUD_NPARAM];
    char *habitat[AUD_NPARAM];          int habitat_indx;    u_int habitat_len[AUD_NPARAM];
    int set_uids[AUD_NPARAM];           int set_uids_indx;

    char *charparam[AUD_NPARAM];        int charp_indx;      u_int charlen[AUD_NPARAM];
    int intparam[AUD_NPARAM];           int intp_indx;
    short shortparam[AUD_NPARAM];       int shortp_indx;
    char *int_array[AUD_NPARAM];        int int_array_indx;  u_int int_array_len[AUD_NPARAM];   

    int descrip[AUD_NPARAM];            int descrip_indx;
    ino_t vnode_id[AUD_NPARAM];         int vp_id_indx;
    dev_t vnode_dev[AUD_NPARAM];        int vp_dev_indx;
    mode_t vnode_mode[AUD_NPARAM];      int vp_mode_indx;
    gid_t gid[AUD_NPARAM];              int gid_indx;
    mode_t mode[AUD_NPARAM];            int mode_indx;

    char *socketaddr[AUD_NPARAM];       int socket_indx;     u_int socketlen[AUD_NPARAM];
    struct sockaddr_in *addrvec[AUD_NPARAM]; int addrvec_indx; u_int addrvec_len[AUD_NPARAM];
    char *msgaddr[AUD_NPARAM];          int msg_indx;        u_int msglen[AUD_NPARAM];
    char *accessaddr[AUD_NPARAM];       int access_indx;     u_int accesslen[AUD_NPARAM];
    uid_t ipc_uid[AUD_NPARAM];          int ipc_uid_indx;
    gid_t ipc_gid[AUD_NPARAM];          int ipc_gid_indx;
    mode_t ipc_mode[AUD_NPARAM];        int ipc_mode_indx;

    char *login[AUD_NPARAM];            int login_indx;      u_int login_len[AUD_NPARAM];
    char *homedir[AUD_NPARAM];          int homedir_indx;    u_int homedir_len[AUD_NPARAM];
    char *shell[AUD_NPARAM];            int shell_indx;      u_int shell_len[AUD_NPARAM];
    char *service[AUD_NPARAM];          int service_indx;    u_int service_len[AUD_NPARAM];
    char *devname[AUD_NPARAM];          int devname_indx;    u_int devname_len[AUD_NPARAM];
    char *hostname[AUD_NPARAM];         int hostname_indx;   u_int hostname_len[AUD_NPARAM];

    long64 longparam[AUD_NPARAM];       int lngprm_indx;
#ifdef ultrix
    int hostid2[AUD_NPARAM];            int hostid2_indx;
    char *login2[AUD_NPARAM];           int login2_indx;     u_int login2_len[AUD_NPARAM];
    struct aud_client_info x_client[AUD_NPARAM];  int x_client_indx;
#endif /* ultrix */

    int atom_id[AUD_NPARAM];            int atom_id_indx;
    int client_id[AUD_NPARAM];          int client_id_indx;
    int property[AUD_NPARAM];           int property_indx;
    unsigned int res_class[AUD_NPARAM]; int res_class_indx;
    unsigned int res_type[AUD_NPARAM];  int res_type_indx;
    int res_id[AUD_NPARAM];             int res_id_indx;

#ifdef __DEBUG1
    char dbg_buf[AUDBUFSIZ_L];
    u_int dbg_cnt;
#endif /* __DEBUG1 */
};


/* audit record fields on which post-selection enabled */
struct selectn {
    auid_l auid[N_SELECT2];             int auid_indx;
    char charparam[N_SELECT2][STR_LEN]; int charparam_indx;
    dev_t dev[N_SELECT2];               int dev_indx;
    int error[N_SELECT2];               int error_indx;
    char event[N_SELECT][STR_LEN2];     int event_indx;
    char event_status[N_SELECT];        char subevent[N_SELECT][STR_LEN2];
    ino_t vnode[N_SELECT2];             int vnode_indx;
    dev_t vnode_dev[N_SELECT2];         int vnode_dev_indx;
    ipaddr_l ipaddr[N_SELECT2];         int ipaddr_indx;
    char logfile[MAXPATHLEN];
    cpu_l n_cpu;
    pid_l pid[N_SELECT2];               int pid_indx;
    pid_l ppid[N_SELECT2];              int ppid_indx;
    char procname[N_SELECT2][STR_LEN];  int procname_indx;
    uid_t ruid[N_SELECT2];              int ruid_indx;
    char rulesfil[STR_LEN];
    char time_end[TIME_LEN];
    char time_start[TIME_LEN];
    uid_t uid[N_SELECT2];               int uid_indx;
    char username[N_SELECT2][STR_LEN];  int username_indx;
} selectn;


#ifdef __DEBUG1
long debug_ptr = 0;         /* strictly for debug work                */
#endif /* __DEBUG1 */
int flag = 0;               /* see runtime operation FLAG def's above */
char close_buf[MAXPATHLEN]; /* last closed file                       */
char procnm_buf[MAXPATHLEN];/* last process name (for exit,execv)     */
ino_t last_vno_id;          /* last vnode id (for dup2)               */
dev_t last_vno_dev;         /* last vnode dev (for dup2)              */

int hdr_flag = 0;           /* sort status and version # of data file 
                               bit  31:  0-unsorted, 1-sorted         
                               bits 3-0: .hdr file version #          */
/* header flag (hdr_flag) values */
#define HDR_VRSN_MASK   0x000f
#define HDR_VRSN0       0x0000
#define HDR_VRSN1       0x0001
#define HDR_SORT        0x8000
#define HDR_DATASPLIT   0x4000


/* deselection ruleset, and ruleset ptrs */
struct ruleset {
    char *host[RULES_IN_SET];
    auid_l auid[RULES_IN_SET];
    uid_t ruid[RULES_IN_SET];
    char *event[RULES_IN_SET];
    char *subevent[RULES_IN_SET];
    char *param[RULES_IN_SET];
    int  oprtn[RULES_IN_SET];
} ruleset;
struct ruleset *rules[NRULESETS];
int ruleno = 0;             /* # rules in ruleset file  */


/* process state information */
struct a_proc {
    auid_l auid;
    ipaddr_l ipaddr;
    pid_l pid;
    uid_t ruid;
    short login_proc;

    char *cwd;
    char *root;
    char *username;
    char *procname;
    char *fd_nmd[NOFILE];
    char **fd_nm[NUM_FDP];
    struct a_proc *a_proc_next;
    struct a_proc *a_proc_prev;
};
#define A_PROC_HDR_SIZ (sizeof(auid_l) + sizeof(ipaddr_l) + sizeof(pid_l) + sizeof(uid_t) + sizeof(short))


/* align on longword boundary */
#define ALIGN(to,from,type) \
    to = (struct type *)((long)from + (sizeof(long) - \
    ((long)from & (sizeof(long) -1))) % sizeof(long))

/* round val up to next longword */
#define RND(val) (val)&(sizeof(long)-1) ? ((val)&~(sizeof(long)-1))+sizeof(long) : (val)


/* reduction tool */
main ( argc, argv )
int argc;
char *argv[];
{
    int interactive = 0;    /* interactive initialization mode        */
    struct hostent *hp;     /* hostentry pointer                      */
    extern char *sys_errlist[];
    extern int sys_nerr;
    char *reportnm = NULL;  /* auditlog report name ("-R")            */
    int i, j;


    if ( argc < 2 ) {
        printf ( "Audit reduction tool usage: [options] logfile\n" );
        printf ( "\nSelection options:\n" );
#ifdef __osf__
        printf ( "  -a audit_id                      -e event[.subevent][:succeed:fail]\n" );
        printf ( "  -E error# or error_string        -h hostname or ip address\n" );
        printf ( "  -p pid                           -P ppid\n" );
        printf ( "  -r real_uid                      -s string_parameter\n" );
        printf ( "  -t start_time                    -T end_time     format: yymmdd[hh[mm[ss]]]\n" );
        printf ( "  -u uid                           -U username\n" );
        printf ( "  -v vnode_id                      -V vnode's device-major#,minor#\n" );
        printf ( "  -x device-major#,minor#          -y procname\n" );
#endif /* __osf__ */
#ifdef ultrix
        printf ( "  -a audit_id                      -e event[.subevent][:succeed:fail]\n" );
        printf ( "  -E error# or error_string        -g gnode_id\n" );
        printf ( "  -G gnode's device-major#,minor#  -h hostname or ip address\n" );
        printf ( "  -p pid                           -P ppid\n" );
        printf ( "  -r real_uid                      -s string_parameter\n" );
        printf ( "  -t start_time                    -T end_time     format: yymmdd[hh[mm[ss]]]\n" );
        printf ( "  -u uid                           -U username\n" );
        printf ( "  -x device-major#,minor#          -y procname\n" );
#endif /* ultrix */
        printf ( "\nControl options:\n" );
        printf ( "  -b          Output in binary format\n" );
        printf ( "  -B          Output in abbreviated format\n" );
        printf ( "  -d file     Use specified deselection rules file (-D to print ruleset)\n" );
        printf ( "  -f          Keep reading auditlog (like tail -f)\n" );
        printf ( "  -F          Fast mode; no state data maintained\n" );
        printf ( "  -i          Interactive selection mode\n" );
        printf ( "  -o          Override switching logfile due to change_auditlog records\n" );
        printf ( "  -R [name]   Generate reports by audit_id\n" );
        printf ( "  -S          Sort audit records by time (for SMP only)\n" );
        printf ( "  -w          Map ruid, group #'s to names using passwd, group tables\n" );
        printf ( "  -Z          Display statistics for selected events\n" );
#ifdef __DEBUG1
        printf ( "  -?          Display parse debugging information\n" );
        printf ( "  -!          Invert selection match logic\n" );
#endif /* __DEBUG1 */
        exit(0);
    }


    /* sig hndlr to trigger interact() on ^C */
    signal ( SIGINT, sig_int1 );

    /* initializations for post-selection */
    init_selectn ( &selectn );


    /* process command line */
    for ( i = 1; i < argc; i++ ) {

        /* select on audit record fields */
        if ( argv[i][0] == '-' ) switch ( argv[i][1] ) {

            case 'a':
                if ( selectn.auid_indx < N_SELECT2 && ++i < argc )
                    selectn.auid[selectn.auid_indx++] = ATOL(argv[i]);
                break;

            case 'B':
                flag |= FLAG_BRIEF;
                break;

            case 'b':
                flag |= FLAG_BINARY;
                break;

            case 'D':
                flag |= FLAG_DISPLAY;       /* fall through to 'd' */
            case 'd':
                if ( ++i < argc ) strncpy (selectn.rulesfil, argv[i], STR_LEN);
                break;

            case 'e':
                if ( ++i < argc ) build_event_list ( argv[i], &selectn, NO_CARE );
                break;

            case 'E':
                if ( selectn.error_indx < N_SELECT2 && ++i < argc ) {
                    if ( isdigit(argv[i][0]) )
                        selectn.error[selectn.error_indx++] = atoi(argv[i]);
                    else {
                        for ( j = 0; j < sys_nerr; j++ )
                            if ( strcmp ( sys_errlist[j], argv[i] ) == 0 ) {
                                selectn.error[selectn.error_indx++] = j;
                                break;
                            }
                        if ( j == sys_nerr ) fprintf ( stderr, "No such error: %s\n", argv[i] );
                    }
                }
                break;

            case 'f':
                flag |= FLAG_FOLLOW;
                break;

            case 'F':
                flag |= FLAG_FASTMODE;
                break;

            case 'h':
                if ( selectn.ipaddr_indx < N_SELECT2 && ++i < argc ) {
                    if ( hp = gethostbyname(argv[i]) )
                        bcopy ( hp->h_addr, &selectn.ipaddr[selectn.ipaddr_indx++], hp->h_length );
                    else if ( (j = inet_addr(argv[i])) != -1 )
                        selectn.ipaddr[selectn.ipaddr_indx++] = j;
                    else write ( 1, "bad host/address\n", 17 );
                }
                break;

            case 'i':
                interactive = 1;
                break;

            case 'I':
                if ( selectn.ipaddr_indx < N_SELECT2 && ++i < argc )
                    selectn.ipaddr[selectn.ipaddr_indx] = inet_addr(argv[i]);
                break;

            case 'o':
                flag |= FLAG_OVERRIDE;
                break;

            case 'p':
                if ( selectn.pid_indx < N_SELECT2 && ++i < argc )
                    selectn.pid[selectn.pid_indx++] = atoi(argv[i]);
                break;

            case 'P':
                if ( selectn.ppid_indx < N_SELECT2 && ++i < argc )
                    selectn.ppid[selectn.ppid_indx++] = atoi(argv[i]);
                break;

            case 'r':
                if ( selectn.ruid_indx < N_SELECT2 && ++i < argc )
                    selectn.ruid[selectn.ruid_indx++] = atoi(argv[i]);
                break;

            case 'R':
                flag |= FLAG_REPORT;
                if ( i < argc-1 && argv[i+1][0] != '-' )
                    reportnm = argv[++i];
                break;

            case 's':
                if ( selectn.charparam_indx < N_SELECT2 && ++i < argc )
                    strncpy (selectn.charparam[selectn.charparam_indx++], argv[i], STR_LEN);
                break;

            case 'S':
                flag |= FLAG_SORT;
                break;

            case 't':
                if ( ++i < argc ) strncpy (selectn.time_start, argv[i], TIME_LEN);
                break;

            case 'T':
                if ( ++i < argc ) strncpy (selectn.time_end, argv[i], TIME_LEN);
                break;

            case 'u':
                if ( selectn.uid_indx < N_SELECT2 && ++i < argc )
                    selectn.uid[selectn.uid_indx++] = atoi(argv[i]);
                break;

            case 'U':
                if ( selectn.username_indx < N_SELECT2 && ++i < argc )
                    strncpy (selectn.username[selectn.username_indx++], argv[i], STR_LEN);
                 break;

#ifdef __osf__
            case 'v':
#endif /* __osf__ */
#ifdef ultrix
            case 'g':
#endif /* ultrix */
                if ( selectn.vnode_indx < N_SELECT2 && ++i < argc )
                    selectn.vnode[selectn.vnode_indx++] = atoi(argv[i]);
                break;

#ifdef __osf__
            case 'V':
#endif /* __osf__ */
#ifdef ultrix
            case 'G':
#endif /* ultrix */
                if ( selectn.vnode_dev_indx < N_SELECT2 && ++i < argc ) {
                    for ( j = 0; argv[i][j] && argv[i][j] != ','; j++ );
                    if ( argv[i][j] == ',' )
                        selectn.vnode_dev[selectn.vnode_dev_indx++] =
                        makedev ( atoi(argv[i]), atoi(argv[i]+j+1) );
                }
                break;

            case 'w':
                flag |= FLAG_LOCALID;
                break;

            case 'x':
                if ( selectn.dev_indx < N_SELECT2 && ++i < argc ) {
                    for ( j = 0; argv[i][j] && argv[i][j] != ','; j++ );
                    if ( argv[i][j] == ',' )
                        selectn.dev[selectn.dev_indx++] =
                        makedev ( atoi(argv[i]), atoi(argv[i]+j+1) );
                }
                break;

            case 'y':
                if ( selectn.procname_indx < N_SELECT2 && ++i < argc )
                    strncpy (selectn.procname[selectn.procname_indx++], argv[i], STR_LEN);
                 break;

            case 'Z':
                flag |= FLAG_STAT;
                break;

#ifdef __DEBUG1
            case '?':
                flag |= FLAG_PARSE_DBG;
                break;

            case '!':
                flag |= FLAG_INVERT;
                break;
#endif /* __DEBUG1 */

            case 030: /* ^X for XAudit_Tool */
                flag |= FLAG_XAUDIT;
                break;

            default:
                fprintf ( stderr, "audit_tool: unknown option: %c ignored\n", argv[i][1] );
                break;
        }

        /* set initial log file */
        else if ( argv[i][0] != '-' )
            strncpy ( selectn.logfile, argv[i], MAXPATHLEN );

    }


    /* check for log file on command line */
    if ( selectn.logfile[0] == '\0' ) {
        fprintf ( stderr, "Usage: %s [ option ... ] auditlog_file\n", argv[0] );
        exit(1);
    }

    /* interactive mode */
    if ( interactive ) interact ( &selectn, &flag );

    /* build deselection ruleset */
    if ( *(selectn.rulesfil) ) ruleno = build_ruleset ( selectn.rulesfil, flag&FLAG_DISPLAY );
    fflush ( stdout );

    /* process auditlog */
    audit_reduce ( &selectn, &flag, reportnm );
    exit(0);
}


/* fetch and output audit records */
audit_reduce ( selectn, flag_p, rprtnm )
struct selectn *selectn;    /* selection criteria       */
int *flag_p;                /* misc options             */
char *rprtnm;               /* report name              */
{
    struct audit_fields audit_fields;
    char *rec_ptr;          /* ptr to audit data        */
    int rec_len;            /* length of audit rec      */
    static char output_file[MAXPATHLEN];
    char buf_ptr[AUDBUFSIZ_L];
    struct stat sbuf;       /* stat struct for report   */

    u_long cnt_pr = 0;      /* # records processed      */
    u_long cnt = 0;         /* # records output         */
    int fd_i = 0;           /* input file descriptor    */
    int fd_o = 1;           /* output file descriptor   */
    int nmlen = 0;          /* length: output_file name */
    char *ptr;
    int i, j;

    /* set up report name */
    if ( rprtnm ) {
        for ( nmlen = 0; nmlen < MAXPATHLEN && (output_file[nmlen] = rprtnm[nmlen]); nmlen++ );

        /* directory specified; append "/report." */
        if ( stat ( output_file, &sbuf ) == 0 && (sbuf.st_mode&S_IFMT) == S_IFDIR )
            for ( i = 0; nmlen < MAXPATHLEN && (output_file[nmlen] = "/report."[i]); nmlen++, i++ );

        /* directory not specified */
        else {
            /* check that parent dir exists */
            for ( i = nmlen; i >= 0 && output_file[i] != '/'; i-- );
            if ( i >= 0 ) {
                output_file[i] = '\0';
                if ( stat ( output_file, &sbuf ) || (sbuf.st_mode&S_IFMT) != S_IFDIR ) {
                    fprintf ( stderr, "%s: no such directory\n", output_file );
                    return(-1);
                }
                output_file[i] = '/';
            }

            /* append "." */
            output_file[nmlen++] = '.';
        }
    }

    /* default report name - "report." */
    else for ( ; output_file[nmlen] = "report."[nmlen]; nmlen++ );


    /* fetch and display all the requested data */
    for ( ; (rec_ptr = fetch_matching_rec ( &audit_fields, selectn,
    &cnt_pr, *flag_p, &rec_len, &fd_i )) != (char *)-1; cnt++ ) {
        fflush ( stdout );  fflush ( stderr );

        /* set file descriptor to tty or output_file */
        if ( *flag_p & FLAG_REPORT ) {
            ptr = itoa(audit_fields.auid);
            for ( i = 0, j = nmlen; j < MAXPATHLEN && (output_file[j] = ptr[i]); i++, j++ );
            fd_o = open ( output_file, O_CREAT|O_RDWR|O_APPEND, 0600 );
            output_file[nmlen] = '\0';
            if ( fd_o == -1 ) fd_o = 1;
        }

        /* collect statistics */
        if ( *flag_p&FLAG_STAT ) statistic ( &audit_fields, 0 );

        /* XAudit_Tool protocol (for output) */
        if ( *flag_p&FLAG_XAUDIT ) {
            i = output_rec_fmt ( buf_ptr, audit_fields, *flag_p|FLAG_BRIEF );
            write ( fd_o, buf_ptr, i );
            write ( fd_o, "\000", 1 );
            i = output_rec_fmt ( buf_ptr, audit_fields, *flag_p&~FLAG_BRIEF );
            write ( fd_o, buf_ptr, i );
            write ( fd_o, "\000", 1 );
        }

        /* binary output */
        else if ( *flag_p&FLAG_BINARY ) {
            write ( fd_o, rec_ptr, rec_len );
        }

        /* readable output */
        else {
#ifdef __DEBUG1
            if ( *flag_p&FLAG_PARSE_DBG )
                write ( fd_o, audit_fields.dbg_buf, audit_fields.dbg_cnt );
#endif /* __DEBUG1 */
            i = output_rec_fmt ( buf_ptr, audit_fields, *flag_p );
            write ( fd_o, buf_ptr, i );
        }

        if ( *flag_p&FLAG_REPORT ) close ( fd_o );
        fflush ( stdout );  fflush ( stderr );
    }


    /* output stats */
    if ( *flag_p&FLAG_STAT ) statistic ( &audit_fields, 1 );
    if ( (*flag_p&(FLAG_BINARY|FLAG_BRIEF)) == 0 ) {
        fprintf ( stderr, "%ld records output\n", cnt );
        fprintf ( stderr, "%ld records processed\n", cnt_pr );
    }
}


/* sort audit records by time - for SMP
   this is an inter-cpu sort only; assumes each cpu's data is time-ordered
*/
audit_sort ( logfile )
char *logfile;
{
    struct audit_fields af;     /* fields of parsed record      */
    struct selectn selectn;     /* selection criteria           */
    struct sort {
        off_t pos;
        struct timeval tv;
    } *sortptr;	                /* per-cpu time, posn           */
    char *rec_ptr;              /* ptr to audit data            */
    int rec_len;                /* length of record             */

    char sortfile[MAXPATHLEN];  /* tmp file to hold sorted data */
    struct stat logstat;        /* stat struct for logfile      */
    struct stat sortstat;       /* stat struct for sortfile     */
    u_long cnt = 0;             /* # records processed          */
    off_t opos = 0;             /* posn in input file           */
    int fd = 0;                 /* input file descriptor        */
    int fd_o;                   /* output file descriptor       */
    int maxcpu = 64;            /* current maxcpu number found  */
    int i, j;
#define _ABS(x) (x > 0 ? x : -x)


    /* check if logfile previously sorted */
    if ( fetch_hdr ( logfile, selectn.time_start, SORT_CHECK ) == HDR_SORT ) {
        fprintf ( stderr, "\n%s already sorted.\n", logfile );
        return;
    }


    /* set up for sort */
    init_selectn ( &selectn );
    strncpy ( selectn.logfile, logfile, MAXPATHLEN );
    if ( (sortptr = (struct sort *)malloc ( sizeof(struct sort) * maxcpu )) == NULL ) {
        fprintf ( stderr, "sort malloc failed\n" );
        return;
    }
    for ( i = 0; i < maxcpu; i++ ) sortptr[i].pos = -1;

    /* pass 1 - find first record per cpu */
    fprintf ( stderr, "\nsorting %s... (pass 1)\n", logfile );
    for ( ;; ) {
        if ( fetch_matching_rec ( &af, &selectn, &cnt, FLAG_OVERRIDE|FLAG_FASTMODE,
        &rec_len, &fd ) == (char *)-1 ) break;
        /* realloc for more cpu's */
        if ( af.n_cpu >= maxcpu ) {
            i = af.n_cpu+1 > maxcpu*2 ? af.n_cpu+1 : maxcpu*2;
            if ( (sortptr = realloc ( sortptr, sizeof(struct sort) * i )) == NULL ) {
                fprintf ( stderr, "sort realloc failed\n" );
                return;
            }
            for ( ; maxcpu < i; maxcpu++ ) sortptr[maxcpu].pos = -1;
        }
        if ( sortptr[af.n_cpu].pos == -1 ) {
            opos = tell(fd);
            sortptr[af.n_cpu].pos = opos-rec_len;
            sortptr[af.n_cpu].tv = af.timeval;
        }
        if ( cnt%1000 == 0 ) fprintf ( stderr, "(pass 1: %ld records processed...)\n", cnt );
    }
    fprintf ( stderr, "pass 1 complete: %ld records sorted\n", cnt );


    /* quick check to bypass sort (single cpu data) */
    for ( i = j = 0; i < maxcpu; i++ ) if ( sortptr[i].pos != -1 ) j++;
    if ( j == 1 ) {
        fprintf ( stderr, "data from single cpu; pass 2 not needed\n\n" );
        fetch_hdr ( logfile, selectn.time_start, SORT_UPDATE );
        hdr_flag |= HDR_SORT;
        free ( sortptr );
        return;
    }


    /* open sortfile */
    for ( i = 0; i < MAXPATHLEN-5 && (sortfile[i] = logfile[i]); i++ );
    for ( j = 0; i < MAXPATHLEN && (sortfile[i] = ".sort"[j]); i++, j++ );
    if ( (fd_o = open ( sortfile, O_RDWR|O_CREAT|O_TRUNC, 0600 )) < 0 ) {
        fprintf ( stderr, "failed to open %s\n\n", sortfile );
        free ( sortptr );
        return;
    }

    /* pass 2 - build sorted logfile */
    fprintf ( stderr, "sorting... (pass 2)\n" );
    for ( cnt = 0;; cnt++ ) {
        /* find next (time-sequenced) record */
        for ( i = 0, j = -1; i < maxcpu; i++ ) {
            if ( sortptr[i].pos != -1 ) {
                if ( j == -1 ) j = i;
                else if ( _ABS(sortptr[i].tv.tv_sec) < _ABS(sortptr[j].tv.tv_sec) )
                    j = i;
                else if ( ( _ABS(sortptr[i].tv.tv_sec) == _ABS(sortptr[j].tv.tv_sec) )
                && ( _ABS(sortptr[i].tv.tv_usec) < _ABS(sortptr[j].tv.tv_usec) ) )
                    j = i;
            }
        }
        if ( j == -1 ) break;

        /* write next record into sortfile */
        if ( lseek ( fd, sortptr[j].pos, L_SET ) == -1 ) perror ( "lseek" );
        af.n_cpu = 0;
        if ( (rec_ptr = fetch_rec ( &fd, &rec_len, &af, 0, 0L, logfile )) == (char *)-1 ) break;
        write ( fd_o, rec_ptr, rec_len );

        /* find next audit record for this cpu */
        parse_rec ( rec_ptr, rec_len, &af, 0 );
        selectn.n_cpu = af.n_cpu;
        i = 0;
        if ( fetch_matching_rec ( &af, &selectn, &i, FLAG_OVERRIDE|FLAG_FASTMODE,
        &rec_len, &fd ) != (char *)-1 ) {
            opos = tell(fd);
            sortptr[af.n_cpu].pos = opos-rec_len;
            sortptr[af.n_cpu].tv = af.timeval;
        }
        else sortptr[selectn.n_cpu].pos = -1;

        if ( cnt && (cnt%1000 == 0) ) fprintf ( stderr, "(pass 2: %ld records processed...)\n", cnt );
    }
    fprintf ( stderr, "pass 2 complete: %ld records sorted\n\n", cnt );
    free ( sortptr );


    /* check filesizes, rename sortfile, update hdr file */
    stat ( logfile, &logstat );
    stat ( sortfile, &sortstat );
    if ( logstat.st_size != sortstat.st_size )
        fprintf ( stderr, "sort failed; %s and %s not same size\n\n", logfile, sortfile );
    else if ( rename ( sortfile, logfile ) == -1 )
        perror ( "rename from sortfile to logfile" );
    else {
        fetch_hdr ( logfile, selectn.time_start, SORT_UPDATE );
        hdr_flag |= HDR_SORT;
    }
}


/* get/free/provide memory for reduction state processing */
char *aud_mem_op ( oprtn, free_ptr, siz )
int oprtn;              /* AMO_FETCH:   alloc mem; return ptr
                           AMO_FREE:    free mem ref'ed by free_ptr
                           AMO_DEBUG:   output blk_map's  */
char *free_ptr;     /* ptr to mem to be free'd     */
int siz;            /* # bytes requested           */
{
    static struct block *blk_ptr[MEM_NBLKS];
    static int blk_ptr_used = -1;
    struct block {
        u_char blk_map[(MEM_NELMNT-1)/NBBY+1];
        char blk_mem[MEM_ELMNT*MEM_NELMNT];
    };
    char *cp;
    int fetch_siz;
    int i, j, k;


    /* fetch memory */
    if ( oprtn == AMO_FETCH && siz ) {
        fetch_siz = (siz-1)/MEM_ELMNT + 1;

        /* check each blk_ptr */
        for ( i = 0; i < MEM_NBLKS; i++ ) {
            if ( i > blk_ptr_used ) {
                if ( (cp = (char *)malloc ( RND(sizeof(struct block)) )) == NULL )
                    return((char *)0);
                bzero ( cp, RND(sizeof(struct block)) );
                ALIGN ( blk_ptr[i], cp, block );
                blk_ptr_used++;
                for ( j = 0; j < (MEM_NELMNT-1)/NBBY+1; j++ )
                    blk_ptr[i]->blk_map[j] = 0x0;
            }

            /* check blk_map for fetch_siz contiguous entries */
            for ( j = 0; j < (MEM_NELMNT-fetch_siz)/NBBY; j+=(k+1) ) {
                for ( k = 0; k < fetch_siz; k++ )
                    if ( blk_ptr[i]->blk_map[(j+k)/NBBY] & (1 << ((j+k)%NBBY)) ) break;
                if ( k < fetch_siz ) continue;
                for ( k = 0; k < fetch_siz; k++ )
                    blk_ptr[i]->blk_map[(j+k)/NBBY] |= (1 << ((j+k)%NBBY));
#ifdef __DEBUG1
                if ( &blk_ptr[i]->blk_mem[j*MEM_ELMNT] <= (char *)debug_ptr &&
                (char *)debug_ptr < &blk_ptr[i]->blk_mem[j*MEM_ELMNT]+siz ) {
                    fprintf ( stderr, "fetching 0x%x\n", debug_ptr );
                    fflush ( stderr );
                }
#endif /* __DEBUG1 */
                return ( &blk_ptr[i]->blk_mem[j*MEM_ELMNT] );
            }
        }

        return((char *)0);
    }


    /* free memory */
    if ( oprtn == AMO_FREE && siz && free_ptr ) {
#ifdef __DEBUG1
        if ( free_ptr <= (char *)debug_ptr &&
        (char *)debug_ptr < free_ptr+siz ) {
            fprintf ( stderr, "freeing 0x%x\n", debug_ptr );
            fflush ( stderr );
        }
#endif /* __DEBUG1 */
        for ( i = 0; i <= blk_ptr_used; i++ ) {
            if ( free_ptr >= blk_ptr[i]->blk_mem && 
            free_ptr+siz <= &blk_ptr[i]->blk_mem[MEM_ELMNT*MEM_NELMNT-1] ) {
                k = (int)((free_ptr - blk_ptr[i]->blk_mem) / MEM_ELMNT);
                for ( j = 0; j <= (siz-1)/MEM_ELMNT; j++ )
                    blk_ptr[i]->blk_map[(j+k)/NBBY] &= ~(1 << ((j+k)%NBBY));
                return((char *)0);
            }
        }
        return ((char *)0);
    }


    /* debug: draw blk_map's */
    if ( oprtn == AMO_DEBUG ) {
        for ( i = 0; i <= blk_ptr_used; i++ ) {
            for ( k = (MEM_NELMNT-1)/NBBY; k >= 0 && blk_ptr[i]->blk_map[k] == 0; k-- );
            if ( k == -1 ) continue;
            fprintf ( stderr, "block %.03d:  ", i );
            for ( j = 0; j <= k; j++ ) {
                fprintf ( stderr, "%02x", blk_ptr[i]->blk_map[j] );
                if ( (j+1)%4 == 0 ) fprintf ( stderr, "  " );
                if ( (j+1)%24 == 0 ) fprintf ( stderr, "\n            " );
            }
            if ( j ) fprintf ( stderr, "\n            " );
            if ( k < (MEM_NELMNT-1)/NBBY ) fprintf ( stderr, "***" );
            fprintf ( stderr, "\n" );
        }
        fprintf ( stderr, "\n" );
    }

    return ((char *)0);
}


/* get/free/provide a_proc struct for reduction state processing */
struct a_proc *aud_mem_proc ( oprtn, rel_ptr, pid, hostp, fd )
int oprtn;              /* AMP_FREE:     release a_proc struct
                           AMP_GETAPROC: get new a_proc struct
                           AMP_GETADDR:  get addr for <pid,hostp>
                           AMP_GETNEXT:  get next a_proc struct
                           AMP_DEBUG:    output linked lists of a_proc's
                           AMP_DUMP:     dump state information to fd   */
struct a_proc *rel_ptr; /* free referenced a_proc                       */
pid_l pid;
ipaddr_l hostp;
int fd;
{
    static struct a_proc *free_list = (struct a_proc *)0;
    static struct a_proc *used_list = (struct a_proc *)0;
    struct a_proc *ptr, *ptr2;
    char *cp;
    int fdp;
    int i, j, k;

#define DUMPIT(obj,indx) \
    for ( indx = 0; obj && obj[indx]; indx++ ); \
    write ( fd, &indx, sizeof(int) ); \
    write ( fd, obj, indx );


    /* return ptr to a_proc structure */
    if ( oprtn == AMP_GETAPROC ) {

        /* get a_proc structure; use free_list and dbly-linked used_list */
        if ( free_list ) {
            ptr = free_list;
            free_list = free_list->a_proc_next;
        }
        else {
            if ( (cp = (char *)malloc(RND(sizeof (struct a_proc)))) == NULL )
                return ((struct a_proc *)-1);
            bzero ( cp, RND(sizeof(struct a_proc)) );
            ALIGN ( ptr, cp, a_proc );
        }
        if ( used_list == (struct a_proc *)0 ) used_list = ptr;
        ptr->a_proc_next = used_list;
        ptr->a_proc_prev = ptr;
        ptr->a_proc_prev = ptr->a_proc_next->a_proc_prev;
        ptr->a_proc_next->a_proc_prev = ptr;
        ptr->a_proc_prev->a_proc_next = ptr;

        /* update proc_tbl and a_proc structure */
        ptr->auid = AUID_INVAL;
        ptr->pid = pid;
        ptr->ruid = -1;
        ptr->login_proc = 0;
        ptr->ipaddr = hostp;
        ptr->cwd = (char *)0;
        ptr->root = (char *)0;
        ptr->username = (char *)0;
        ptr->procname = (char *)0;
        ptr->fd_nm[0] = ptr->fd_nmd;
        for ( i = 1; i < NUM_FDP; i++ ) ptr->fd_nm[i] = (char **)0;
        return ( ptr );
    }


    /* find a_proc struct for this <pid,hostp> */
    if ( oprtn == AMP_GETADDR ) {
        if ( used_list == (struct a_proc *)0 ) return ( (struct a_proc *)-1 );
        ptr = used_list;
        do {
            ptr = ptr->a_proc_prev;
            if ( ptr->pid == pid && ptr->ipaddr == hostp ) return ( ptr );
        } while ( ptr != used_list );
        return ( (struct a_proc *)-1 );
    }


    /* get next a_proc struct */
    if ( oprtn == AMP_GETNEXT )
        return ( used_list ? used_list : (struct a_proc *)-1 );


    /* release a_proc struct */
    if ( oprtn == AMP_FREE && rel_ptr ) {
        if ( rel_ptr->pid == pid && rel_ptr->ipaddr == hostp ) {
            rel_ptr->a_proc_next->a_proc_prev = rel_ptr->a_proc_prev;
            rel_ptr->a_proc_prev->a_proc_next = rel_ptr->a_proc_next;
            used_list = rel_ptr->a_proc_next;
            if ( used_list == rel_ptr ) used_list = (struct a_proc *)0;
            rel_ptr->a_proc_next = free_list;
            free_list = rel_ptr;
            return ( (struct a_proc *)0 );
        }
        else return ( (struct a_proc *)-1 );
    }

    /* debug */
    if ( oprtn == AMP_DEBUG ) {
        fprintf ( stderr, "used_list:  " );
        if ( used_list )
            for ( i = 0, ptr = ptr2 = used_list; ptr != ptr2 || i == 0; ptr = ptr->a_proc_next, i++ ) {
                fprintf ( stderr, "0x%x  ", ptr );
                if ( (i+1)%5 == 0 ) fprintf ( stderr, "\n            " );
            }
        fprintf ( stderr, "\n" );
        fprintf ( stderr, "free_list:  " );
        for ( i = 0, ptr = free_list; ptr; ptr = ptr->a_proc_next, i++ ) {
            fprintf ( stderr, "0x%x  ", ptr );
            if ( (i+1)%5 == 0 ) fprintf ( stderr, "\n            " );
        }
        fprintf ( stderr, "\n" );
    }


    /* dump rvalues and ptrs in a_proc structures to fd */
    if ( oprtn == AMP_DUMP && used_list )
        for ( i = 0, ptr = ptr2 = used_list; ptr != ptr2 || i == 0; ptr = ptr->a_proc_next, i++ ) {
            write ( fd, ptr, A_PROC_HDR_SIZ );
            DUMPIT ( ptr->cwd, j );
            DUMPIT ( ptr->root, j );
            DUMPIT ( ptr->username, j );
            DUMPIT ( ptr->procname, j );
            /* find maximum descriptor value in **fd_nm */
            k = 0;
            for ( fdp = NUM_FDP-1; fdp >= 0; fdp-- ) {
                for ( ; fdp >= 0 && ptr->fd_nm[fdp] == '\0'; fdp-- );
                if ( fdp == 0 && ptr->fd_nm[fdp] == '\0' ) break;
                for ( k = NOFILE-1; ptr->fd_nm[fdp][k] == '\0' && k; k-- );
                if ( ptr->fd_nm[fdp][k] ) break;
            }
            k = fdp*NOFILE + k;
            /* output maximum descriptor number; if none, -1 */
            if ( k == 0 && ptr->fd_nm[fdp] == '\0' ) k = -1;
            write ( fd, &k, sizeof(int) );
            for ( ; k >= 0; k-- ) {
                if ( ptr->fd_nm[k/NOFILE] && ptr->fd_nm[k/NOFILE][k%NOFILE] ) {
                    DUMPIT ( ptr->fd_nm[k/NOFILE][k%NOFILE], j );
                }
                else {
                    DUMPIT ( "", j );
                }
            }
        }

    return ( (struct a_proc *)0 );
}


/* add event to selection event list */
build_event_list ( event, selectn, flag )
char *event;
struct selectn *selectn;
int flag;
{
    char aliasevent[AUD_MAXEVENT_LEN];
    int i, j, k;


    /* expand aliases into event list */
    for ( i = j = 0; j == 0; i++ ) {
        for ( k = 0; k < AUD_MAXEVENT_LEN-4 && event[k] && event[k] != ':'; k++ );
        if ( event[k] == ':' && flag == NO_CARE ) {
            flag = 0x0;
            if ( event[k+1] == '0' ) flag += NO_SUCC;
            if ( event[k+3] == '0' ) flag += NO_FAIL;
        }
        event[k] = '\0';
        if ( aud_alias_event ( event, i, aliasevent, sizeof aliasevent ) == -1 ) break;
        build_event_list ( aliasevent, selectn, flag );
    }
    if ( i ) return;


    /* add event to selectn struct */
    if ( selectn->event_indx < N_SELECT ) {
        for ( j = 0; j < STR_LEN2 && event[j] && event[j] != ':' && event[j] != '.'; j++ );
        bcopy ( event, selectn->event[selectn->event_indx], j );
        if ( event[j] != ':' )
            selectn->event_status[selectn->event_indx] = NO_CARE;
        else selectn->event_status[selectn->event_indx] = 0x0;
        if ( event[j] == ':' && event[j+1] == '0' )
            selectn->event_status[selectn->event_indx] |= NO_SUCC;
        if ( event[j] && event[j+2] == ':' && event[j+3] == '0' )
            selectn->event_status[selectn->event_indx] |= NO_FAIL;
        if ( flag != NO_CARE ) selectn->event_status[selectn->event_indx] = flag;
        for ( ; event[j] && event[j] != '.'; j++ );
        if ( event[j] == '.' )
            strncpy (selectn->subevent[selectn->event_indx], &event[j+1], STR_LEN2);
        if ( selectn->event_status[selectn->event_indx] != (NO_SUCC|NO_FAIL) )
            selectn->event_indx++;
    }
}


/* build deselection ruleset; return # rules built */
build_ruleset ( rulesfile, display )
char *rulesfile;    /* pathname of rulesfile */
int display;        /* display rulesets      */
{
    int ruleno = 0;
    int lineno;
    char line[1024];
    char *ptr[6];
    FILE *fp;
    char *cp;
    int i, j;

    /* open rulesfile */
    if ( (fp = fopen ( rulesfile, "r" )) == NULL ) {
        fprintf ( stderr, "build_ruleset: could not open %s\n", rulesfile );
        return(0);
    }


    for ( lineno = 0;; lineno++ ) {

        /* read line (NOTE: fscanf doesn't allow variable width specification) */
        bzero ( line, 1024 );
        i = fscanf ( fp, "%1024[^\n]%*[\n]", line );
        if ( i == EOF ) break;

        /* ignore empty lines and comment lines */
        if ( i == 0 ) {
            fscanf ( fp, "%*[\n]", line );
            continue;
        }
        if ( line[0] == '#' ) continue;


        /* parse line into 6 strings
           allow for "old event" style names
        */
        for ( i = j = 0; i < 1024 && line[i] && j < 6; ) {
            if ( *(ptr[j++] = &line[i]) == '"' ) {
                ptr[j-1]++;
                for ( i++; i < 1024 && line[i] && line[i] != '"'; i++ );
                if ( i == 1024 ) break;
                line[i++] = '\0';
            }
            for ( ; i < 1024 && line[i] && line[i] != ' ' && line[i] != '\t'; i++ );
            if ( i < 1024 && line[i] ) line[i++] = '\0';
            for ( ; i < 1024 && line[i] && (line[i] == ' ' || line[i] == '\t'); i++ );
        }
        if ( j < 6 ) {
            fprintf ( stderr, "bad rule at line #%d in %s\n", lineno+1, rulesfile );
            continue;
        }


        /* allocate rules struct */
        if ( ruleno%RULES_IN_SET == 0 ) {
            if ( (cp = (char *)malloc (RND(sizeof(struct ruleset)) )) == NULL ) {
                fprintf ( stderr, "alloc failed on ruleset %d\n", ruleno/RULES_IN_SET );
                return ( ruleno );
            }
            bzero ( cp, RND(sizeof(struct ruleset)) );
            ALIGN ( rules[ruleno/RULES_IN_SET], cp, ruleset );
        }


        /* load hostname rule */
        for ( i = 0; i < HOST_LEN && ptr[0][i]; i++ );
        if ( (RULE(ruleno,host) = (caddr_t)malloc(RND(i+1))) == NULL ) {
            fprintf ( stderr, "alloc failed on ruleset %d, rule %d\n",
                ruleno/RULES_IN_SET, ruleno%RULES_IN_SET );
            continue;
        }
        bcopy ( ptr[0], RULE(ruleno,host), i+1 );
        

        /* load audit_id rule */
        if ( ptr[1][0] == '*' ) RULE(ruleno,auid) = -1;
        else RULE(ruleno,auid) = ATOL(ptr[1]);


        /* load ruid rule */
        if ( ptr[2][0] == '*' ) RULE(ruleno,ruid) = -1;
        else RULE(ruleno,ruid) = ATOL(ptr[2]);


        /* load event rule */
        for ( i = 0; i < STR_LEN2 && ptr[3][i]; i++ );
        if ( (RULE(ruleno,event) = (caddr_t)malloc(RND(i+1))) == NULL ) {
            fprintf ( stderr, "alloc failed on ruleset %d, rule %d\n",
                ruleno/RULES_IN_SET, ruleno%RULES_IN_SET );
            continue;
        }
        bcopy ( ptr[3], RULE(ruleno,event), i+1 );
        for ( i = 0; i < sizeof(ptr[3]) && ptr[3][i] && ptr[3][i] != '.'; i++ );
        if ( ptr[3][i] == '.' ) {
            RULE(ruleno,event)[i] = '\0';
            RULE(ruleno,subevent) = &RULE(ruleno,event)[i+1];
        }
        else RULE(ruleno,subevent) = (char *)0;


        /* load param rule */
        for ( i = 0; i < MAX_RULE_SIZ && ptr[4][i]; i++ );
        if ( (RULE(ruleno,param) = (caddr_t)malloc(RND(i+1))) == NULL ) {
            fprintf ( stderr, "alloc failed on ruleset %d, rule %d\n",
                ruleno/RULES_IN_SET, ruleno%RULES_IN_SET );
            continue;
        }
        bcopy ( ptr[4], RULE(ruleno,param), i+1 );


        /* load read/write ptr[5] rule */
        RULE(ruleno,oprtn) = ( *ptr[5] == '*' ? -1 : 0 );
        for ( i = 0; i < sizeof(ptr[5]) && ptr[5][i]; i++ ) {
            if ( ptr[5][i] == 'r' ) RULE(ruleno,oprtn) += 1;
            else if ( ptr[5][i] == 'w' ) RULE(ruleno,oprtn) += 2;
        }
        if ( RULE(ruleno,oprtn) != -1 ) RULE(ruleno,oprtn)--;


        /* max # rules hit */
        if ( ++ruleno == RULES_IN_SET * NRULESETS ) {
            fprintf ( stderr, "Maximum # rules (%d) reached.\n", ruleno );
            break;
        }

    }

    fclose(fp);


    /* display rulesets */
    if ( display ) fprintf ( stderr, "    hostname audit_id ruid event string oprtn(r/w)\n" );
    for ( i = 0; (i < ruleno) && display; i++ ) {
        fprintf ( stderr, "r%d: ", i );
        fprintf ( stderr, "%s ", RULE(i,host) );
        if ( RULE(i,auid) == (unsigned)-1 )
            fprintf ( stderr, "* " );
        else fprintf ( stderr, "%-6d ", RULE(i,auid) );
        if ( RULE(i,ruid) == (unsigned)-1 )
            fprintf ( stderr, "* " );
        else fprintf ( stderr, "%-6d ", RULE(i,ruid) );
        fprintf ( stderr, "%s", RULE(i,event) );
        if ( RULE(i,subevent) ) fprintf ( stderr, ".%s", RULE(i,subevent) );
        fprintf ( stderr, " " );
        fprintf ( stderr, "%s ", RULE(i,param) );
        if ( RULE(i,oprtn) == -1 )
            fprintf ( stderr, "*\n" );
        else fprintf ( stderr, "%d\n", RULE(i,oprtn) );
    }
    if ( display ) fprintf ( stderr, "\n\n" );

    return ( ruleno );
}


/* process change_auditlog directive
   store_hdr functionality similar to fetch_hdr
*/
change_log ( fd_p, logfile, time_l, af, sort )
int *fd_p;                  /* ptr to auditlog descriptor       */
char *logfile;              /* current auditlog file            */
int time_l;                 /* seconds component of timestamp   */
struct audit_fields *af;    /* audit record fields              */
int sort;
{
    struct stat sbuf;
    char logfilehdr[MAXPATHLEN];
    int curlog;
    int remote = 0;
    int i, j;
    char ch;


    /* close fd; compress previously compressed files */
    close(*fd_p);
    *fd_p = -1;
    compress ( 1, (char *)0 );


    /* fetch next logname from current logfile hdr */
    for ( i = 0; logfile[i]; i++ );
    bcopy ( logfile, logfilehdr, i );
    if ( strncmp ( &logfilehdr[i-2], ".Z", 2 ) == 0 ) i -= 2;
    bcopy ( ".hdr\0", &logfilehdr[i], 5 );
    if ( (curlog = open ( logfilehdr, O_RDONLY, 0600 )) != -1 ) {
        i = 0;
        if ( lseek ( curlog, sizeof(hdr_flag) + sizeof(time_l) + sizeof(af->timeval.tv_sec), L_SET ) > 0 )

            /* might have a logfilehr without a next logname */
            if ( read ( curlog, &ch, 1 ) == 1 && ch != '\0' ) {
                logfile[i++] = ch;
                do {
                    if ( read ( curlog, &logfile[i], 1 ) == -1 ) break;
                } while ( logfile[i++] && i < MAXPATHLEN );
            }
        close ( curlog );
    }
    else i = 0;

    /* update current logfile hdr */
    if ( (curlog = open ( logfilehdr, O_CREAT|O_WRONLY, 0600 )) != -1 ) {
        write ( curlog, &hdr_flag, sizeof(hdr_flag) );
        write ( curlog, &time_l, sizeof(time_l) );
        write ( curlog, &af->timeval.tv_sec, sizeof(af->timeval.tv_sec) );
        /* no previous idea of next logfile */
        if ( i == 0 ) {
            write ( curlog, af->charparam[1], af->charlen[1] );
            bcopy ( af->charparam[1], logfile, af->charlen[1] );
        }
    }


    /* audit data transferred to another host */
    i = af->charlen[0]-5;
    if ( strncmp ( &af->charparam[0][i], "host", 4 ) == 0 ) {
        fprintf ( stderr, "** Audit log change: data sent to remote host %s **\n\n", logfile );
        remote++;
    }


    for ( ; *fd_p == -1; ) {

        /* open next logfile (or logfile.Z) */
        if ( remote == 0 ) {
            if ( sort ) audit_sort ( logfile );
            if ( stat ( logfile, &sbuf ) == 0 ) *fd_p = open ( logfile, 0 );
            else {
                for ( i = 0; i < MAXPATHLEN && logfile[i]; i++ );
                bcopy ( ".Z\0", &logfile[i], 4 );
                if ( stat ( logfile, &sbuf ) == 0 ) {
                    compress ( 0, logfile );
                    *fd_p = open ( logfile, 0 );
                }
                logfile[i] = '\0';
            }
        }

        /* allow for change to next logfile */
        if ( *fd_p == -1 ) {
            if ( remote == 0 )
                fprintf ( stderr, "** Audit log change to %s failed **\n\n", logfile );
            fprintf ( stderr, "   You may enter a new location for the next auditlog.\n" );
            fprintf ( stderr, "   This link will be maintained in %s.\n", logfilehdr );
            fprintf ( stderr, "   Pathname of next auditlog (<cr> to exit):  " );
            i = read ( 0, logfile, MAXPATHLEN );
            fprintf ( stderr, "\n" );
            logfile[i-1] = '\0';
            if ( i == 1 ) break;
            remote = 0;
        }
        else fprintf ( stderr, "** switching to auditlog %s **\n\n", logfile );

    }


    /* update current logfile hdr with next logfile */
    if ( *fd_p != -1 ) {
        if ( lseek ( curlog, sizeof(hdr_flag) + sizeof(time_l) + sizeof(af->timeval.tv_sec), L_SET ) > 0 )
            write ( curlog, logfile, i+1 );
    }
    close ( curlog );


    /* determine next logfile hdr */
    for ( i = 0; logfile[i]; i++ ); /* may already be null terminated */
    bcopy ( logfile, logfilehdr, i );
    if ( strncmp ( &logfilehdr[i-2], ".Z", 2 ) == 0 ) i -= 2;
    bcopy ( ".hdr\0", &logfilehdr[i], 5 );


    /* if next logfile hdr contains only sort status, append state info
       after sort status
    */
    j = -1;
    hdr_flag &= ~HDR_SORT;
    if ( stat ( logfilehdr, &sbuf ) == 0 ) {
        if ( sbuf.st_size == sizeof(hdr_flag) )
            if ( (j = open ( logfilehdr, O_RDWR, 0600 )) != -1 )
                read ( j, &hdr_flag, sizeof(hdr_flag) );
    }
    if ( j == -1 )
        j = open ( logfilehdr, O_CREAT|O_WRONLY|O_EXCL, 0600 );

    if ( j != -1 ) {
        if ( af->event == AUDIT_LOG_CHANGE ) hdr_flag |= HDR_DATASPLIT;
        hdr_flag = (hdr_flag&~HDR_VRSN_MASK) | HDR_VRSN1;
        lseek ( j, 0, L_SET );
        write ( j, &hdr_flag, sizeof(hdr_flag) );
        lseek ( j, (sizeof time_l)*2+(sizeof hdr_flag)+MAXPATHLEN, L_SET );
        if ( af->event == AUDIT_LOG_CHANGE )
            recover ( RCVR_HDRFETCH, (char *)0, (char *)0, &j, (char *)0, 0 );
        aud_mem_proc ( AMP_DUMP, (struct a_proc *)0, 0, NULL, j );
        close(j);
    }

}


/* compress/uncompress filnam */
compress ( op, filnam )
int op;         /* 1: compress; 0: uncompress */
char *filnam;   /* file to uncompress         */
{
    static char oldlog[MAXPATHLEN];
    static char cmd1[MAXPATHLEN+9] = "compress ";
    static char cmd2[MAXPATHLEN+11] = "uncompress ";
    static int compress = 0;
    int i;

    switch ( op ) {

    case 0: /* uncompress filnam, if ending in .Z */
        for ( i = 0; filnam[i]; i++ );
        if ( (filnam[i-2] == '.') && (filnam[i-1] == 'Z') ) {
            bcopy ( filnam, &cmd2[11], i+1 );
            if ( system ( cmd2 ) ) fprintf ( stderr, "failed on: %s\n", cmd2 );
            else {
                fprintf ( stderr, "** Uncompressed %s **\n\n", filnam );
                filnam[i-2] = '\0';
                bcopy ( filnam, oldlog, i-1 );
                compress = 1;
            }
        }
        break;

    case 1: /* compress filename if it had been previously compressed */
        if ( compress == 1 ) {
            for ( i = 0; oldlog[i]; i++ );
            bcopy ( oldlog, &cmd1[9], i+1 );
            if ( system ( cmd1 ) ) fprintf ( stderr, "failed on: %s\n", cmd1 );
            else fprintf ( stderr, "** Compressed %s **\n\n", oldlog );
            compress = 0;
        }
        break;

    }
}


/* check audit_fields against deselection rules; return match */
int deselect ( af, ruleno )
struct audit_fields *af;    /* audit fields struct */
int ruleno;                 /* # deselection rules */
{
    int match = 0;
    char *eventp;
    char *subeventp;
    char *hostp;
    int i, j, k;

    /* search for a rule which matches current audit record */
    for ( i = 0; i < ruleno; i++ ) {

        /* compare hostname against rules */
        if ( RULE(i,host)[0] == '*' || (hostp = gethost_l(af->ipaddr)) == (char *)0 )
             match = 1;
        else {
            for ( j = 0; hostp[j] && RULE(i,host)[j] == hostp[j]; j++ );
            if ( RULE(i,host)[j] == '\0' && hostp[j] == '\0' ) match = 1;
        }

        /* compare auid, ruid against rules */
        if ( match )
            match = ((af->auid == RULE(i,auid)) || (RULE(i,auid) == (unsigned)-1));
        if ( match )
            match = ((af->ruid == RULE(i,ruid)) || (RULE(i,ruid) == (unsigned)-1));

        /* compare event against rules */
        if ( match ) {
            if ( RULE(i,event)[0] == '*' ) match = 1;
            else {
                get_eventnm ( af, af->event, af->subevent, &eventp, &subeventp );
                match = match_event ( eventp, subeventp, RULE(i,event), RULE(i,subevent), 0, NO_CARE );
                for ( j = 0; match == 0 && af->event2_indx == 0 && j < af->subevent2_indx; j++ ) {
                    get_eventnm ( af, af->event, af->subevent2[j], &eventp, &subeventp );
                    match = match_event ( eventp, subeventp, RULE(i,event), RULE(i,subevent), 0, NO_CARE );
                }
            }
        }

        /* compare string params against rules; allow '*' wildcard */
        if ( match ) {
            if ( RULE(i,param)[0] == '*' ) match = 1;
            else for ( match = j = 0; j < af->charp_indx; j++ ) {
                for ( k = 0; k < af->charlen[j] &&
                (af->charparam[j][k] == RULE(i,param)[k]); k++ );
                if ( (k == af->charlen[j] && RULE(i,param)[k] == '\0')
                || RULE(i,param)[k] == '*' ) {
                    match = 1;
                    break;
                }
            }
        }

        /* compare operation against rules for open()'s (using 1st intparam) */
        if ( match && af->event == SYS_open )
            match = (af->intparam[0] == RULE(i,oprtn)) || (RULE(i,oprtn) == -1);

        if ( match ) return ( match );
    }
    return(0);
}


/* fetch header file and update a_proc structure and state; return sort status */
int fetch_hdr ( logfile, start_str, sort )
char *logfile;
char *start_str;                    /* user specified start time           */
int  sort;                          /* SORT_CHECK: check sort status only  */
                                    /* SORT_UPDATE: update sortflag in hdr */
{
    char logfilehdr[MAXPATHLEN];
    struct a_proc *a_ptr;
    time_t start_time;                /* start time from hdr file  */
    time_t end_time;                  /* end time from header file */
    char nextlog[MAXPATHLEN];         /* next auditlog filename    */
#ifdef ultrix
    char filler[512];
#endif /* ultrix */
    int retval = 0;
    int fd;
    int i, j;

#define FETCHIT(obj,indx) \
    read ( fd, &indx, sizeof(int) ); \
    if ( indx ) { \
        obj = aud_mem_op ( AMO_FETCH, (char *)0, indx ); \
        if ( obj ) read ( fd, obj, indx ); \
        else lseek ( fd, indx, L_INCR ); \
    }


    /* open current logfile's .hdr file */
    for ( i = 0; i < MAXPATHLEN-5 && logfile[i]; i++ );
    bcopy ( logfile, logfilehdr, i );
    if ( strncmp ( &logfilehdr[i-2], ".Z", 2 ) == 0 ) i -= 2;
    bcopy ( ".hdr\0", &logfilehdr[i], 5 );
    if ( (fd = open ( logfilehdr, 2 )) == -1 ) {
        hdr_flag = HDR_VRSN1;
        if ( sort == SORT_UPDATE ) {
            hdr_flag |= HDR_SORT;
            if ( (fd = open ( logfilehdr, O_CREAT|O_WRONLY, 0600 )) == -1 )
                return ( retval );
            write ( fd, &hdr_flag, sizeof(hdr_flag) );
            close(fd);
            return ( retval );
        }
        else return ( retval );
    }

    /* fetch hdr_flag */
    if ( read ( fd, &hdr_flag, sizeof(hdr_flag) ) == -1 ) {
        close(fd);
        return ( retval );
    }

    /* check/update sort status (return) */
    if ( sort == SORT_UPDATE ) {
        hdr_flag |= HDR_SORT;
        lseek ( fd, 0, L_SET );
        write ( fd, &hdr_flag, sizeof(hdr_flag) );
        close(fd);
        return ( retval );
    }
    if ( sort == SORT_CHECK ) {
        close(fd);
        return ( hdr_flag&HDR_SORT );
    }


    /* open file which corresponds to user specified time */
    if ( *start_str ) {
        j = 0;
        do {
            if ( lseek ( fd, sizeof hdr_flag, L_SET ) == -1 ) {
                close(fd);
                return ( retval );
            }
            if ( (read ( fd, &start_time, sizeof(start_time) ) == -1) ||
            (read ( fd, &end_time, sizeof(end_time) ) == -1) ) {
                close(fd);
                return ( retval );
            }
            i = 0;
            do {
                if ( read ( fd, &nextlog[i], 1 ) == -1 ) {
                    close(fd);
                    return ( retval );
                }
            } while ( nextlog[i++] && i < MAXPATHLEN );

            if ( start_time == 0 && end_time == 0 ) j = 1;
            else if ( (match_time ( start_str, start_time ) >= 0) &&
                (match_time ( start_str, end_time ) <= 0) ) j = 1;

            /* get next logfile hdr */
            else {
                bcopy ( nextlog, logfile, i );
                close(fd);
                bcopy ( logfile, logfilehdr, i );
                bcopy ( ".hdr\0", &logfilehdr[i-1], 5 );
                if ( (fd = open ( logfilehdr, 0 )) == -1 ) return ( retval );
            }
        } while ( j == 0 );
    }
    lseek ( fd, (sizeof start_time)*2+(sizeof hdr_flag)+MAXPATHLEN, L_SET );


    /* store start of split record into recover buffer (for later recovery) */
    if ( hdr_flag & HDR_DATASPLIT ) {
        if ( read ( fd, &i, sizeof(i) ) == -1 ) return ( retval );
        if ( recover ( RCVR_HDRSTR, (char *)0, &i, &fd, (char *)0, 0 ) > 0 )
            retval = 1;
    }


    /* read state information */
    for ( ;; ) {
        a_ptr = aud_mem_proc ( AMP_GETAPROC, (struct a_proc *)0, -1, (ipaddr_l)-1, 0 );
        if ( a_ptr == (struct a_proc *)-1 ) break;

#ifdef ultrix
        /* version 0: ignore first 388 bytes of each a_proc, then read 12 bytes */
        if ( (hdr_flag&HDR_VRSN_MASK) == HDR_VRSN0 ) {
            if ( read ( fd, filler, 388 ) != 388 ) break;
            if ( read ( fd, (char *)a_ptr, 12 ) != 12 ) break;
            FETCHIT ( a_ptr->cwd, i );
            FETCHIT ( a_ptr->root, i );
            FETCHIT ( a_ptr->username, i );
            for ( i = 0; i < NOFILE; i++ ) {
                if ( a_ptr->fd_nm[0] == '\0' ) {
                    a_ptr->fd_nm[0] = (char **)aud_mem_op ( NOFILE*sizeof(char *), (char *)0, 0, 0 );
                    if ( a_ptr->fd_nm[0] == '\0' ) {
                        fprintf ( stderr, "warning: fetch_hdr could not alloc space\n" );
                        break;
                    }
                    for ( j = 0; j < NOFILE; j++ ) a_ptr->fd_nm[0][j] = '\0';
                }
                FETCHIT ( a_ptr->fd_nm[0][i], j );
            }
        }
#endif /* ultrix */

        /* version 1: read A_PROC_HDR size */
        if ( (hdr_flag&HDR_VRSN_MASK) == HDR_VRSN1 ) {
            if ( read ( fd, a_ptr, A_PROC_HDR_SIZ ) != A_PROC_HDR_SIZ ) break;
            FETCHIT ( a_ptr->cwd, i );
            FETCHIT ( a_ptr->root, i );
            FETCHIT ( a_ptr->username, i );
            FETCHIT ( a_ptr->procname, i );
            if ( read ( fd, &i, sizeof(int) ) != sizeof(int) ) break;
            for ( ; i >= 0; i-- ) {
                if ( a_ptr->fd_nm[i/NOFILE] == '\0' ) {
                    a_ptr->fd_nm[i/NOFILE] = (char **)aud_mem_op ( AMO_FETCH, (char *)0, NOFILE*sizeof(char *) );
                    if ( a_ptr->fd_nm[i/NOFILE] == '\0' ) {
                        fprintf ( stderr, "warning: fetch_hdr could not alloc space\n" );
                        break;
                    }
                    for ( j = 0; j < NOFILE; j++ ) a_ptr->fd_nm[i/NOFILE][j] = '\0';
                }
                FETCHIT ( a_ptr->fd_nm[i/NOFILE][i%NOFILE], j );
            }
        }

        else {
            fprintf ( stderr, "failed to parse %s\n", logfilehdr );
            return ( retval );
        }
    }

    close ( fd );
    return ( retval );
}


/* fetch records matching stated conditions; return record ptr; update rec_len */
char *fetch_matching_rec ( audit_fields, selectn, cnt, flag, rec_len, fd_p )
struct audit_fields *audit_fields;  /* audit record fields    */
struct selectn *selectn;            /* selection criteria     */
u_long *cnt;                        /* # records processed    */
int flag;                           /* misc options           */
int *rec_len;                       /* length of record       */
int *fd_p;                          /* data file descriptor   */
{
    static char logfile[MAXPATHLEN];
    struct stat sbuf;
    char *rec_ptr;
    int match;
    static int get_time = 1;
    static int time_l;
    static u_long cnt_l = 0;        /* # records processed in current log */
    static int auditd_event = 0;    /* auditd auditlog event occurred     */
    off_t pos1, pos2;
    int i;

    /* open auditlog datafile and .hdr file; use timestamp if provided */
    if ( *fd_p == 0 ) {
        for ( i = 0; selectn->logfile[i]; i++ );
        bcopy ( selectn->logfile, logfile, i+1 );
        if ( fetch_hdr ( logfile, selectn->time_start, 0 ) )
            flag |= FLAG_RECOVER;
        if ( stat ( logfile, &sbuf ) == -1 ) {
            for ( i = 0; logfile[i]; i++ );
            if ( strncmp ( &logfile[i-2], ".Z", 2 ) )
                bcopy ( ".Z\0", &logfile[i], 3 );
        }
        if ( stat ( logfile, &sbuf ) == 0 ) compress ( 0, logfile );
        else logfile[i] = '\0';
        if ( flag & FLAG_SORT ) audit_sort ( logfile );
        if ( (*fd_p = open ( logfile, 0 )) == -1 ) {
            fprintf ( stderr, "failed to open %s\n", logfile );
            return((char *)-1);
        }
        cnt_l = 0;
    }
    if ( *fd_p == -1 ) return((char *)-1);


    /* fetch records until selection criteria satisfied */
    do {

        if ( *cnt && *cnt%1000 == 0 ) fprintf ( stderr, "(%ld records processed...)\n\n", *cnt );

        /* fetch a valid record */
        init_audit_fields ( audit_fields );
        rec_ptr = fetch_rec ( fd_p, rec_len, audit_fields, flag, cnt_l, logfile );
        if ( audit_fields->flag == AF_BADLENTOKEN && *cnt == 0 )
            audit_fields->flag = AF_PARTIAL;

        /* close file if no more audit records */
        if ( rec_ptr == (char *)-1 ) {
            compress ( 1, (char *)0 );
            return ( (char *)-1 );
        }
        (*cnt)++; cnt_l++;


        /* fetch_rec() might have found an abnormality in the data stream */
        if ( audit_fields->flag != AF_LOGBREAK && audit_fields->flag != AF_CORRUPTED 
        && audit_fields->flag != AF_LOGCHNG ) {
            parse_rec ( rec_ptr, *rec_len, audit_fields, flag );
            if ( !(flag & FLAG_FASTMODE) ) state_maint ( audit_fields );
        }


        /* parse_rec() might have found an abnormality in the data stream */
        /* allow AF_BADLENTOKEN if AF_LOGBREAK situation immediately precedes it */
        if ( audit_fields->flag == AF_LOGBREAK ) auditd_event++;
        else if ( audit_fields->flag == AF_BADLENTOKEN && auditd_event )
            audit_fields->flag = 0;
        else if ( audit_fields->flag == AF_CORRUPTED ) (*cnt)--;
        else auditd_event = 0;


        /* find records matching input params; deselect according to ruleset */
        match = match_rec ( audit_fields, selectn, flag );
#ifdef __DEBUG1
        if ( flag & FLAG_INVERT ) match ^= 1;
#endif /* __DEBUG1 */
        if ( ruleno && match ) match = !deselect ( audit_fields, ruleno );


        /* save first timestamp for logfile hdr */
        if ( get_time ) {
            time_l = audit_fields->timeval.tv_sec;
            get_time = 0;
        }


        /* check for auditd change log messages */
        if ( (audit_fields->event == AUDIT_LOG_CHANGE) && ((flag&FLAG_OVERRIDE) == 0) ) {
            pos1 = tell ( *fd_p );
            lseek ( *fd_p, 0, L_XTND );
            pos2 = tell ( *fd_p );
            if ( pos1 == pos2 ) {
                change_log ( fd_p, logfile, time_l, audit_fields, flag&FLAG_SORT );
                cnt_l = 0;
                get_time = 1;
            }
            else lseek ( *fd_p, pos1, L_SET );
        }

    } while ( match == 0 );

    return ( rec_ptr );
}


/* read audit record; return ptr; modify length */
char *fetch_rec ( fd_p, rec_len, af, flag, cnt, logfile )
int *fd_p;                  /* audit file descriptor             */
int *rec_len;               /* length of audit record - returned */
struct audit_fields *af;    /* audit record fields               */
int flag;                   /* misc options                      */
u_long cnt;                 /* # records processed from *fd_p    */
char *logfile;              /* log filename                      */
{
    static char buf[AUDBUFSIZ_L];
    static int recvr = 0;
    char rec_len_buf[sizeof *rec_len];
    char token;
    int ptr = 0;
    int i, j, k;

    /* 1) read record length */
    do {

        /* find first LENGTH token */
        do {
            j = read ( *fd_p, &token, sizeof token );
            if ( (token&0xff) != TP_LENGTH && j == sizeof token )
                af->flag = AF_BADLENTOKEN;

            /* attempt recovery of split record */
            /* assumes first record is remainder of split record */
            if ( cnt == 0 && (recvr || (flag&FLAG_RECOVER)) ) {
                recvr = 0;
                if ( recover ( RCVR_FETCH, buf, rec_len, fd_p, logfile, flag ) == 0 )
                    af->flag = AF_RECOVER;
                return ( buf );
            }

            if ( (j == 0) && flag&FLAG_FOLLOW ) {
                if ( fetch_rec_chk ( fd_p, logfile ) == -1 ) return ( (char *)-1 );
                sleep(4);
            }
        } while ( ((j == 0) && flag&FLAG_FOLLOW) || ((j > 0) && ((token&0xff) != TP_LENGTH)) );
        if ( j <= 0 ) return ( (char *)-1 );
        bcopy ( &token, buf, sizeof token );

        /* read length */
        i = 0;
        do {
            j = read ( *fd_p, &rec_len_buf[i], 1 );
            if ( j ) i++;
            if ( (j == 0) && flag&FLAG_FOLLOW ) {
                if ( fetch_rec_chk ( fd_p, logfile ) == -1 ) return ( (char *)-1 );
                sleep(4);
            }
        } while ( ((j == 0) && flag&FLAG_FOLLOW) || ((j > 0) && (i < sizeof *rec_len)) );
        bcopy ( rec_len_buf, rec_len, sizeof(int) );
        if ( j <= 0 ) return ( (char *)-1 );
        if ( (*rec_len < 0) | (*rec_len >= AUDBUFSIZ_L) ) af->flag = AF_BADRECLEN;

    } while ( (*rec_len < 0) || (*rec_len >= AUDBUFSIZ_L) );
    bcopy ( rec_len, &buf[sizeof token], sizeof *rec_len );


    /* 2) read next char into start of data area */
    do {
        j = read ( *fd_p, &buf[sizeof *rec_len + sizeof token], sizeof token );
        if ( (j == 0) && flag&FLAG_FOLLOW ) {
            if ( fetch_rec_chk ( fd_p, logfile ) == -1 ) return ( (char *)-1 );
            sleep(4);
        }
    } while ( (j == 0) && flag&FLAG_FOLLOW );
    if ( j <= 0 ) return ( (char *)-1 );


    /* 3) if next char was another LENGTH token, read length again */
    if ( (buf[sizeof *rec_len + sizeof token]&0xff) == TP_LENGTH ) {
        i = 0;
        do {
            j = read ( *fd_p, &rec_len_buf[i], 1 );
            if ( j ) i++;
            if ( (j == 0) && flag&FLAG_FOLLOW ) {
                if ( fetch_rec_chk ( fd_p, logfile ) == -1 ) return ( (char *)-1 );
                sleep(4);
            }
        } while ( ((j == 0) && flag&FLAG_FOLLOW) || ((j > 0) && (i < sizeof *rec_len)) );
        bcopy ( rec_len_buf, rec_len, sizeof(int) );
        if ( j <= 0 ) return ( (char *)-1 );
        bcopy ( rec_len, &buf[sizeof token], sizeof *rec_len );
    }
    else ptr++;
    if ( (*rec_len < 0) || (*rec_len >= AUDBUFSIZ_L) ) {
        af->flag = AF_BADRECLEN2;
        return ( buf );
    };


    /* 4) read data */
    i = *rec_len - (sizeof *rec_len + sizeof token + ptr);
    j = 0;
    do {
        k = read ( *fd_p, &buf[ptr + sizeof *rec_len + sizeof token + j], i-j );
        if ( (k == 0) && flag&FLAG_FOLLOW ) {
            if ( fetch_rec_chk ( fd_p, logfile ) == -1 ) return ( (char *)-1 );
            sleep(4);
        }
        j += k;
    } while ( (j < i) && flag&FLAG_FOLLOW );
    if ( j < i ) return ( (char *)-1 );


    /* 5) consistency check */
    bcopy ( &buf[*rec_len - (sizeof *rec_len)], &i, sizeof(int) );
    if ( i != *rec_len && af->flag != AF_RECOVER ) {
        switch ( recover ( RCVR_STORE, buf, rec_len, fd_p, logfile, flag ) ) {
        case AUDIT_LOG_CHANGE:
            af->flag = AF_LOGCHNG;
            recvr++;
            break;

        case AUDIT_SUSPEND:
        case AUDIT_LOG_CREAT:
        case AUDIT_LOG_OVERWRITE:
        case AUDIT_SHUTDOWN:
        case AUDIT_XMIT_FAIL:
            af->flag = AF_LOGBREAK;

        default:
            af->flag = AF_CORRUPTED;
        }

        *rec_len = 0;
     }


    /* 6) check for logfile getting overwritten */
    if ( cnt && cnt%1000 == 0 )
        if ( fetch_rec_chk ( fd_p, logfile ) == -1 ) return ( (char *)-1 );

    return ( buf );
}


/* check for logfile having been overwritten (by auditd)
   don't want to keep reference to an overwritten file
*/
int fetch_rec_chk ( fd_p, logfile )
int *fd_p;      /* audit file descriptor */
char *logfile;  /* auditlog filename     */
{
    struct stat statbuf;
    static off_t prevsiz = 0;

    /* use stat instead of fstat to check for existence of log in filesys */
    if ( stat ( logfile, &statbuf ) == -1 ) return(0);

    /* actual file has shrunk (overwritten)
       close file to release blocks
    */
    if ( statbuf.st_size < prevsiz ) {
        fprintf ( stderr, "\n** auditlog has been overwritten **\n\n" );
        fflush ( stderr );
        close ( *fd_p );
        if ( (*fd_p = open ( logfile, 0 )) == -1 ) {
            perror ( "open" );
            return(-1);
        }
    }
    prevsiz = statbuf.st_size;

    return(0);
}


/* translate event/subevent numbers into event/subevent names */
get_eventnm ( af, ev_num, subev_num, ev_nam, subev_nam )
struct audit_fields *af;
int ev_num, subev_num;
char **ev_nam, **subev_nam;
{
    extern char *syscallnames[];
    extern char *trustedevent[];
    static int n_sevents;
    static int init = 0;
    static char ev_naml[STR_LEN2];
    static char subev_naml[STR_LEN2];

    subev_naml[0] = '\0';
    *subev_nam = subev_naml;

    /* check for specified event name in audit data */
    if ( af->eventp_indx ) {
        bcopy ( af->eventp[0], ev_naml, af->eventp_len[0] );
        ev_naml[af->eventp_len[0]] = '\0';
        *ev_nam = ev_naml;
        return;
    }


    /* get number of site events */
    if ( init == 0 ) {
        init++;
#ifdef __osf__
        if ( (n_sevents = audcntl ( GET_NSITEVENTS, (char *)0, 0, 0, 0, 0 )) == -1 )
            n_sevents = 0;
#endif /* __osf__ */
#ifdef ultrix
        if ( (n_sevents = getkval ( X_N_SITEVENTS )) == -1 ) n_sevents = 0;
#endif /* ultrix */
    }


    /* set parameters to point at event,subevent names */
    if ( ev_num == SYS_exit && af->login_proc )
        *ev_nam = "logout";
    else if ( ev_num >= 0 && ev_num < NUM_SYSCALLS )
        *ev_nam = syscallnames[ev_num];
    else if ( ev_num >= MIN_TRUSTED_EVENT && ev_num < MIN_TRUSTED_EVENT + N_TRUSTED_EVENTS )
        *ev_nam = trustedevent[ev_num-MIN_TRUSTED_EVENT];
#ifdef ultrix
    else if ( ev_num >= MIN_SPECIAL && ev_num <= MAX_SPECIAL )
        *ev_nam = special_event[ev_num-MIN_SPECIAL];
#endif /* ultrix */
    else if ( ev_num >= MIN_SITE_EVENT && ev_num < MIN_SITE_EVENT+n_sevents ) {
        if ( aud_sitevent ( ev_num, subev_num, ev_naml, subev_naml ) == -1 ) {
            if ( subev_num >= 0 ) strcpy ( subev_naml, itoa(subev_num) );
            if ( aud_sitevent ( ev_num, -1, ev_naml, '\0' ) == -1 )
                strcpy ( ev_naml, itoa(ev_num) );
        }
        *ev_nam = ev_naml;
        *subev_nam = subev_naml;
    }

    else {
        strcpy ( ev_naml, "UNKNOWN EVENT" );
        *ev_nam = ev_naml;
    }
}


/* return hostname associated with ipaddr */
char *gethost_l ( ipaddr )
ipaddr_l ipaddr;
{
    struct hostent *hostp;
    static ipaddr_l ipaddr_prev = 0;
    static char h_name_prev[MAXHOSTNAMELEN];
    int i;

    if ( ipaddr_prev && ipaddr == ipaddr_prev ) return ( h_name_prev );
    if ( (hostp = gethostbyaddr ( &ipaddr, sizeof(ipaddr), AF_INET )) == (struct hostent *)0 )
        return ( (char *)0 );

    /* save hostname, ipaddr for next iteration */
    ipaddr_prev = ipaddr;
    for ( i = 0; i < MAXHOSTNAMELEN-1 && hostp->h_name[i]; i++ );
    bcopy ( hostp->h_name, h_name_prev, i+1 );

    return ( hostp->h_name );
}


#ifdef ultrix
/* get integer value from kernel */
int getkval ( var )
int var;
{
    static int vm_fd = -1;
    static int km_fd;
    int i = 0;

    if ( vm_fd == -1 ) {
        if ( (vm_fd = open ( "/vmunix", 0 )) == -1 ) return(-1);
        if ( (km_fd = open ( "/dev/kmem", 0 )) == -1 ) return(-1);
        nlist ( "/vmunix", nlst );
        if ( nlst[0].n_type == 0 ) return (-1);
    }

    if ( lseek ( km_fd, nlst[var].n_value, 0 ) == -1 ) return(-1);
    read ( km_fd, &i, sizeof(int) );

    return ( i );
}
#endif /* ultrix */


/* initialize audit_fields structure */
init_audit_fields ( af )
struct audit_fields *af;
{
    af->auid              =  AUID_INVAL;
    af->device            = -1;
    af->error             =  0;
    af->event             = -1;
    af->eventp_indx       =  0;
    af->flag              =  0;
    af->hostid            = -1;
    af->ipaddr            =  0;
    af->n_cpu             =  0;
    af->pid               = -1;
    af->ppid              = -1;
    af->result.val[0]     = -1;
    af->result.val[1]     = -1;
    af->timeval.tv_sec    =  0;
    af->uid               = -1;
    af->ruid              = -1;
    af->subevent          = -1;
    af->login_proc        =  0;
    af->version           =  0;

    af->auid2_indx        =  0;
    af->ipaddr2_indx      =  0;
    af->device2_indx      =  0;
    af->pid2_indx         =  0;
    af->ppid2_indx        =  0;
    af->uid2_indx         =  0;
    af->ruid2_indx        =  0;
    af->event2_indx       =  0;
    af->subevent2_indx    =  0;
    af->habitat_indx      =  0;
    af->set_uids_indx     =  0;

    af->charp_indx        =  0;
    af->intp_indx         =  0;
    af->shortp_indx       =  0;
    af->int_array_indx    =  0;

    af->descrip_indx      =  0;
    af->vp_id_indx        =  0;
    af->vp_dev_indx       =  0;
    af->vp_mode_indx      =  0;
    af->gid_indx          =  0;
    af->mode_indx         =  0;

    af->socket_indx       =  0;
    af->addrvec_indx      =  0;
    af->msg_indx          =  0;
    af->access_indx       =  0;
    af->ipc_uid_indx      =  0;
    af->ipc_gid_indx      =  0;
    af->ipc_mode_indx     =  0;

    af->login_indx        =  0;
    af->homedir_indx      =  0;
    af->shell_indx        =  0;
    af->service_indx      =  0;
    af->devname_indx      =  0;
    af->hostname_indx     =  0;

    af->lngprm_indx       =  0;
#ifdef ultrix
    af->hostid2_indx      =  0;
    af->login2_indx       =  0;
    af->x_client_indx     =  0;
#endif /* ultrix */

    af->atom_id_indx      =  0;
    af->client_id_indx    =  0;
    af->property_indx     =  0;
    af->res_class_indx    =  0;
    af->res_type_indx     =  0;
    af->res_id_indx       =  0;

    close_buf[0]          = '\0';
    procnm_buf[0]         = '\0';
    last_vno_id           =  0;
    last_vno_dev          =  0;
}


/* initialize selectn structure */
init_selectn ( selectn_p )
struct selectn *selectn_p;
{
    int i;

    selectn_p->auid_indx        = 0;
    selectn_p->charparam_indx   = 0;
    selectn_p->dev_indx         = 0;
    selectn_p->error_indx       = 0;
    selectn_p->event_indx       = 0;
    selectn_p->vnode_indx       = 0;
    selectn_p->vnode_dev_indx   = 0;
    selectn_p->ipaddr_indx      = 0;
    selectn_p->logfile[0]       = '\0';
    selectn_p->n_cpu            = -1;
    selectn_p->pid_indx         = 0;
    selectn_p->ppid_indx        = 0;
    selectn_p->procname_indx    = 0;
    selectn_p->ruid_indx        = 0;
    selectn_p->rulesfil[0]      = '\0';
    for ( i = 0; i < TIME_LEN; i++ ) {
        selectn_p->time_end[i]      = '\0';
        selectn_p->time_start[i]    = '\0';
    }
    selectn_p->uid_indx         = 0;
    selectn_p->username_indx    = 0;
}


/* interactive mode */
interact ( selectn, flag )
struct selectn *selectn;    /* selection criteria   */
int *flag;                  /* misc options         */
{
    char buf[MAXPATHLEN];
    int i;

#define INTER1(str,arg,len,buf,i) \
    fprintf ( stderr, str ); \
    if ( arg[0] != '\0' ) fprintf ( stderr, "(%s)  ", arg ); \
    i = read ( 0, buf, len ); \
    if ( i > 1 ) { \
        if ( buf[0] == '*' ) i = 1, arg[0] = '\0'; \
        else strncpy ( arg, buf, i-1 ); \
        arg[i-1] = '\0'; \
    }

#define INTER2(str,flag_mode,buf,i) \
    fprintf ( stderr, str ); \
    fprintf ( stderr, "(%s)  ", *flag & flag_mode ? "yes" : "no" ); \
    i = read ( 0, buf, STR_LEN ); \
    if ( i > 1 ) { \
        if ( buf[0] == '1' || buf[0] == 'y' || buf[0] == 'Y') *flag |= flag_mode; \
        else *flag &= ~flag_mode; \
    }


    fprintf ( stderr, "subject:\n" );
    interact_num ( "  audit_id:  \t", selectn->auid, sizeof(selectn->auid[0]), &selectn->auid_indx, 0 );
    interact_num ( "  ruid:  \t", selectn->ruid, sizeof(selectn->ruid[0]), &selectn->ruid_indx, 0 );
    interact_num ( "  uid:  \t", selectn->uid, sizeof(selectn->uid[0]), &selectn->uid_indx, 0 );
    interact_str ( "  username:  \t", selectn->username, &selectn->username_indx );
    interact_num ( "  pid:  \t", selectn->pid, sizeof(selectn->pid[0]), &selectn->pid_indx, 0 );
    interact_num ( "  ppid:  \t", selectn->ppid, sizeof(selectn->ppid[0]), &selectn->ppid_indx, 0 );
    interact_num ( "  dev:  \t", selectn->dev, sizeof(selectn->dev[0]), &selectn->dev_indx, 1 );
    interact_host ( selectn->ipaddr, &selectn->ipaddr_indx );


    fprintf ( stderr, "\nevent:\n" );
    interact_event ( selectn->event, &selectn->event_indx, selectn->event_status, selectn->subevent );
    interact_str ( "  procname:  \t", selectn->procname, &selectn->procname_indx );
    interact_num ( "  error:  \t", selectn->error, sizeof(selectn->error[0]), &selectn->error_indx, 0 );
    INTER1 ( "  time_start:  \t", selectn->time_start, STR_LEN, buf, i );
    INTER1 ( "  time_end:  \t", selectn->time_end, STR_LEN, buf, i );


    fprintf ( stderr, "\nobject:\n" );
    interact_str ( "  charparam:  \t", selectn->charparam, &selectn->charparam_indx );
#ifdef __osf__
    interact_num ( "  vnode:  \t", selectn->vnode, sizeof(selectn->vnode[0]), &selectn->vnode_indx, 0 );
    interact_num ( "  vnode_dev:  \t", selectn->vnode_dev, sizeof(selectn->vnode_dev[0]), &selectn->vnode_dev_indx, 1 );
#endif /* __osf__ */
#ifdef ultrix
    interact_num ( "  gnode:  \t", selectn->vnode, sizeof(selectn->vnode[0]), &selectn->vnode_indx, 0 );
    interact_num ( "  gnode_dev:  \t", selectn->vnode_dev, sizeof(selectn->vnode_dev[0]), &selectn->vnode_dev_indx, 1 );
#endif /* ultrix */


    INTER1 ( "\nrules file:  \t", selectn->rulesfil, STR_LEN, buf, i );
    if ( i > 1 ) ruleno = build_ruleset ( selectn->rulesfil, *flag&FLAG_DISPLAY );
    INTER2 ( "continuous operation:  \t", FLAG_FOLLOW, buf, i );
    INTER2 ( "report by audit_id:    \t", FLAG_REPORT, buf, i );
    INTER2 ( "brief output format:   \t", FLAG_BRIEF, buf, i );
    INTER2 ( "fast mode:             \t", FLAG_FASTMODE, buf, i );
    INTER2 ( "override auditlog changes:  \t", FLAG_OVERRIDE, buf, i );
    INTER2 ( "use local /etc/passwd and /etc/group:  \t", FLAG_LOCALID, buf, i );
    INTER2 ( "report statistics:     \t", FLAG_STAT, buf, i );
    fprintf ( stderr, "\n\n" );
}


/* event list = { event[:success[:fail]] } */
interact_event ( field, indx, status, subfield )
char field[][STR_LEN2];
int *indx;
char *status;
char subfield[][STR_LEN2];
{
    char buf[MAXPATHLEN];
    int i, j, k;

    /* output current events[:succeed:fail], subevents selected */
    if ( *indx == 0 ) fprintf ( stderr, "  (events: all selected)\n" );
    else fprintf ( stderr, "  (events: %d selected; enter new events or '*' to select all events\n", *indx );
    for ( j = 0; j < *indx; j++ ) {
        fprintf ( stderr, "    %s:\t", field[j] );
        if ( (status[j] & NO_SUCC) == 0 ) fprintf ( stderr, " succeed" );
        if ( (status[j] & NO_FAIL) == 0 ) fprintf ( stderr, " fail" );
        if ( *subfield[j] ) fprintf ( stderr, "  (subevent %s)", subfield[j] );
        fprintf ( stderr, "\n" );
    }
    if ( *indx ) fprintf ( stderr, "\n" );


    /* read new event selections */
    for ( ; *indx < N_SELECT; (*indx)++ ) {
        fprintf ( stderr, "  event:  \t" );
        /* note: this is legit because (STR_LEN2*2+5) < MAXPATHLEN */
        i = read ( 0, buf, STR_LEN2*2+5 );

        /* special cases: no input, or magic character */
        if ( i == 1 ) break;
        if ( buf[0] == '*' ) {
            *indx = 0;
            break;
        }

        /* event name */
        for ( j = 0; buf[j] != '\n' && buf[j] != ':' && buf[j] != '/'; j++);
        strncpy ( field[*indx], buf, j );
        field[*indx][j] = '\0';

        /* status bits for success, failure */
        status[*indx] = 0x0;
        if ( buf[j] == ':' ) {
            if ( buf[j+1] == '0' ) status[*indx] += NO_SUCC;
            if ( buf[j+2] == ':' && buf[j+3] == '0' ) status[*indx] += NO_FAIL;
            j += 4;
        }

        /* subevent name */
        subfield[*indx][0] = '\0';
        if ( buf[j] == '/' ) {
            strncpy ( subfield[*indx], &buf[j+1], i-(j+2) );
            subfield[*indx][i-(j+2)] = '\0';
        }


        /* check for previous occurrence in list */
        for ( k = 0; k < *indx; k++ ) {
            if ( (strcmp ( field[k], field[*indx] ) == 0)
            && (strcmp ( subfield[k], subfield[*indx] ) == 0) ) {
                status[k] = status[*indx];
                if ( status[*indx] == (NO_SUCC|NO_FAIL) ) {
                    (*indx)--;
                    strncpy ( field[k], field[*indx], STR_LEN2 );
                    strncpy ( subfield[k], subfield[*indx], STR_LEN2 );
                    status[k] = status[*indx];
                }
                (*indx)--;
                break;
            }
        }

        if ( *indx >= 0 && status[*indx] == (NO_SUCC|NO_FAIL) ) (*indx)--;
    }
}


/* interactive mode for hostnames and ip addresses */
interact_host ( ipaddr, indx )
ipaddr_l *ipaddr;
int *indx;
{
    char buf[MAXPATHLEN];
    struct hostent *hp;
    char *name;
    int i, j;

    /* output current hosts selected */
    fprintf ( stderr, "  hostname/addr:  " );
    if ( *indx == 1 ) {
        if ( name = gethost_l(ipaddr[0]) ) fprintf ( stderr, "(%s)  ", name );
        else fprintf ( stderr, "(%d)  ", inet_ntoa(ipaddr[0]) );
    }
    else if ( *indx ) {
        fprintf ( stderr, "( " );
        for ( i = 0; i < *indx; i++ ) {
            if ( name = gethost_l(ipaddr[i]) )
                fprintf ( stderr, "%s  ", name );
            else fprintf ( stderr, "%s  ", inet_ntoa(ipaddr[i]) );
        }
        fprintf ( stderr, ")  " );
    }
    if ( *indx ) fprintf ( stderr, "\n  hostname/addr ('*' to select all hosts):  " );


    /* read new host selections */
    for ( i = 0; i < N_SELECT2; i++ ) {
        j = read ( 0, buf, HOST_LEN );
        if ( j == 1 ) break;
        if ( buf[0] == '*' ) {
            *indx = 0;
            break;
        }
        buf[j-1] = '\0';

        if ( hp = gethostbyname(buf) ) bcopy ( hp->h_addr, &ipaddr[i], hp->h_length );
        else if ( (j = inet_addr(buf)) != -1 ) ipaddr[i] = j;
        else {
            fprintf ( stderr, "   -- bad host/address\n" );
            i--;
        }
        *indx = i+1;
        if ( *indx < N_SELECT2 ) fprintf ( stderr, "  hostname/addr:  " );
    }
}


/* interactive mode for short/integer/dev arrays */
interact_num ( string, field, len, indx, dev )
char *string;   /* prompting string                     */
char *field;    /* selection field ptr (note (char *)   */
int len;        /* length of selection field            */
int *indx;      /* index into selection field           */
int dev;        /* used to mark devices                 */
{
    int *intp = (int *)field;
    short *shortp = (short *)field;
    dev_t *devp = (dev_t *)field;
    char buf[MAXPATHLEN];
    int i = 0;
    int j;

    /* output a single existing selection value */
    fprintf ( stderr, "%s", string );
    if ( *indx == 1 ) {
        if ( dev ) fprintf ( stderr, "(%d,%d)  ", major(devp[0]), minor(devp[0]) );
        else if ( len == sizeof(short) ) fprintf ( stderr, "(%d)  ", shortp[0] );
        else if ( len == sizeof(int) ) fprintf ( stderr, "(%d)  ", intp[0] );
    }

    /* output multiple existing selection values */
    else if ( *indx ) {
        fprintf ( stderr, "( " );
        for ( i = j = 0; i < *indx; i++ ) {
            if ( dev ) fprintf ( stderr, "%d,%d ", major(devp[i]), minor(devp[i]) );
            else if ( len == sizeof(short) ) fprintf ( stderr, "%d ", shortp[i] );
            else if ( len == sizeof(int) ) fprintf ( stderr, "%d ", intp[i] );
        }
        fprintf ( stderr, ")  " );
    }


    /* read in new values */
    for ( i = 0; i < N_SELECT2; i++ ) {
        j = read ( 0, buf, STR_LEN2 );
        if ( j == 1 ) break;
        if ( buf[0] == '*' ) { /* magic wildcard */
            *indx = 0;
            break;
        }
        if ( dev ) { /* for device major,minor values */
            for ( j = 0; buf[j] && buf[j] != ','; j++ );
            if ( buf[j] == ',' ) devp[i] = makedev ( atoi(buf), atoi(&buf[j+1]) );
            else continue;
        }
        else if ( len == sizeof(short) ) shortp[i] = atoi(buf);
        else if ( len == sizeof(int) ) intp[i] = atoi(buf);
        *indx = i+1;
        if ( *indx < N_SELECT2 ) fprintf ( stderr, "%s", string );
    }
}


/* interactive mode for string arrays */
interact_str ( string, field, indx )
char *string;
char field[][STR_LEN];
int *indx;
{
    char buf[MAXPATHLEN];
    int i, j;

    /* prompt */
    fprintf ( stderr, "%s", string );


    /* output string values */
    if ( *indx == 1 ) fprintf ( stderr, "(%s)  ", field[0] );
    else if ( *indx ) {
        fprintf ( stderr, "( " );
        for ( i = 0; i < *indx; i++ ) fprintf ( stderr, "%s ", field[i] );
        fprintf ( stderr, ")  " );
    }


    /* read new string values */
    for ( i = 0; i < N_SELECT2; i++ ) {
        j = read ( 0, buf, STR_LEN );
        if ( j == 1 ) break;
        if ( buf[0] == '*' ) {
            *indx = 0;
            break;
        }
        strncpy ( field[i], buf, j-1 );
        field[i][j-1] = '\0';
        *indx = i+1;
        if ( *indx < N_SELECT2 ) fprintf ( stderr, "%s", string );
    }
}


/* convert integer to alphanumeric string; return ptr to string */
char *itoa ( num )
int num;
{
    static char num_rep[]= "            ";
    int i, j = 0;

    num = num < 0 ? j++, -num : num;

    for ( i = sizeof num_rep; i == sizeof num_rep || (num > 0 && i >= 0); num = num/10, i-- )
        num_rep[i] = num%10 + '0';

    if ( j ) num_rep[i--] = '-';
    return ( &num_rep[++i] );
}


/* return comparison between current event and selected event */
int match_event ( cur_event, cur_subevent, sel_event, sel_subevent, error, flag )
char *cur_event;
char *cur_subevent;
char *sel_event;
char *sel_subevent;
int error;              /* from af->error             */
char flag;              /* selectn->event_status flag */
{
    int len;
    int i, j;

    /* compare current event : selected event */
    for ( len = 0; sel_event[len]; len++ );
    if ( sel_event[len-1] == '+' ) len--;
    else len = STR_LEN2;
    j = !strncmp ( cur_event, sel_event, len );


    /* check subevent parameters */
    if ( j && sel_subevent && *sel_subevent ) {
        for ( len = 0; sel_subevent[len]; len++ );
        if ( sel_subevent[len-1] == '+' ) len--;
        else len = STR_LEN2;
        if ( *cur_subevent ) j &= !strncmp ( cur_subevent, sel_subevent, len );
        else j = 0;
    }


    /* check event_status selection */
    if ( flag == (NO_FAIL | NO_SUCC) ) j = 0;
    if ( flag == NO_FAIL && error ) j = 0;
    if ( flag == NO_SUCC && error == 0 ) j = 0;


    return(j);
}


/* return 1 for records matching selection criteria; 0 for no match */
int match_rec ( af, selectn, flag )
struct audit_fields *af;            /* audit record fields    */
struct selectn *selectn;            /* selection criteria     */
int flag;                           /* misc options           */
{
    struct a_proc *a_ptr;           /* a_proc ptr to get fd's */
    char *eventp, *subeventp;
    int retval = 1;                 /* 1: match; 0: no match  */
    int len;
    int strnglen;
    struct passwd *pw_ptr;
    char *ptr = NULL;
    int i, j, k, l;

#define MATCH(sel,af,indx,retval,i,j) \
    for ( i = j = 0; i < indx; i++ ) \
        if ( sel[i] == af ) j = 1; \
    if ( indx ) retval &= j;

    MATCH ( selectn->auid, af->auid, selectn->auid_indx, retval, i, j );
    MATCH ( selectn->uid, af->uid, selectn->uid_indx, retval, i, j );
    MATCH ( selectn->ruid, af->ruid, selectn->ruid_indx, retval, i, j );
    MATCH ( selectn->error, af->error, selectn->error_indx, retval, i, j );
    MATCH ( selectn->ipaddr, af->ipaddr, selectn->ipaddr_indx, retval, i, j );
    MATCH ( selectn->pid, af->pid, selectn->pid_indx, retval, i, j );
    MATCH ( selectn->ppid, af->ppid, selectn->ppid_indx, retval, i, j );
    MATCH ( selectn->dev, af->device, selectn->dev_indx, retval, i, j );
#undef MATCH


    /* selectn->n_cpu used internally by sort algorithm */
    if ( selectn->n_cpu != -1 )  retval &= (selectn->n_cpu == af->n_cpu);


    /* check for matching vnode */
    for ( i = j = 0; i < selectn->vnode_indx; i++ )
        for ( k = 0; k < af->vp_id_indx; k++ )
            if ( selectn->vnode[i] == af->vnode_id[k] ) j = 1;
    if ( selectn->vnode_indx ) retval &= j;
    for ( i = j = 0; i < selectn->vnode_dev_indx; i++ )
        for ( k = 0; k < af->vp_dev_indx; k++ )
            if ( selectn->vnode_dev[i] == af->vnode_dev[k] ) j = 1;
    if ( selectn->vnode_dev_indx ) retval &= j;


    /* check for matching event [/subevent] */
    get_eventnm ( af, af->event, af->subevent, &eventp, &subeventp );
    for ( i = j = 0; i < selectn->event_indx && j == 0; i++ )
        j = match_event ( eventp, subeventp, selectn->event[i],
        selectn->subevent[i], af->error, selectn->event_status[i] );
    for ( k = 0; j == 0 && af->event2_indx == 0 && k < af->subevent2_indx; k++ ) {
        get_eventnm ( af, af->event, af->subevent2[k], &eventp, &subeventp );
        for ( i = 0; i < selectn->event_indx && j == 0; i++ )
            j = match_event ( eventp, subeventp, selectn->event[i],
            selectn->subevent[i], af->error, selectn->event_status[i] );
    }
    if ( selectn->event_indx ) retval &= j;


    /* check for matching procname */
    for ( i = j = 0; i < selectn->procname_indx; i++ ) {
        if ( (a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af->pid, af->ipaddr, 0 ))!= (struct a_proc *)-1 ) {
            if ( a_ptr->procname ) {
                /* use previous procname for exec's; exec changes procname */
                if ( af->event == SYS_execv || af->event == SYS_execve )
                    ptr = procnm_buf;
#ifdef __osf__
                else if ( af->event == SYS_exec_with_loader ) ptr = procnm_buf;
#endif /* __osf__ */
                else ptr = a_ptr->procname;
                if ( strcmp ( selectn->procname[i], ptr ) == 0 ) j = 1;
                /* check only final pathname component if have relative procname */
                else if ( (index ( selectn->procname[i], '/' ) == (char *)0)
                && (ptr = rindex ( ptr, '/' )) )
                    if ( strcmp ( selectn->procname[i], ++ptr ) == 0 ) j = 1;
            }
        }
    }
    if ( selectn->procname_indx ) retval &= j;


    /* check for matching username */
    for ( i = j = 0; i < selectn->username_indx; i++ ) {
        if ( (a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af->pid, af->ipaddr, 0 ))!= (struct a_proc *)-1 ) {
            if ( a_ptr->username && strcmp ( selectn->username[i], a_ptr->username ) == 0 )
                j = 1;
        }
        if ( (j == 0) && (flag & FLAG_LOCALID) && ( (a_ptr != (struct a_proc *)-1)
        && (a_ptr->username == '\0') ) || (a_ptr == (struct a_proc *)-1) ) {
            if ( pw_ptr = getpwuid ( af->ruid ))
                if ( strcmp ( selectn->username[i], pw_ptr->pw_name ) == 0 )
                    j = 1;
        }
    }
    if ( selectn->username_indx ) retval &= j;


    /* check for matching strings in charparams and dereferenced fd's */
    for ( i = j = 0; j == 0 && i < selectn->charparam_indx; i++ ) {
        for ( len = 0; (len < STR_LEN) && selectn->charparam[i][len]; len++ );
        for ( k = 0; j == 0 && k < af->charp_indx; k++ )
            for ( l = 0; j == 0 && l <= (int)af->charlen[k]-len; l++ )
                if ( strncmp ( selectn->charparam[i], &af->charparam[k][l], len ) == 0 )
                    j = 1;
        if ( (j == 0) && ((a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, 
        af->pid, af->ipaddr, 0 )) != (struct a_proc *)-1) )
            for ( k = 0; j == 0 && k < af->descrip_indx; k++ ) {
                if ( (af->error == 0 && af->event == SYS_close)
                || (af->error == 0 && af->event == SYS_dup2 && k == 0 ) )
                    ptr = close_buf;
                else {
                    l = af->descrip[k];
                    if ( l >= 0 && l < NUM_FDS )
                        ptr = a_ptr->fd_nm[l/NOFILE] ? a_ptr->fd_nm[l/NOFILE][l%NOFILE] : '\0';
                }
                if ( ptr ) {
                    for ( strnglen = 0; ptr[strnglen]; strnglen++ );
                    for ( l = 0; j == 0 && l <= strnglen-len; l++ )
                        if ( strncmp ( selectn->charparam[i], &ptr[l], len ) == 0 )
                            j = 1;
                }
            }
    }
    if ( selectn->charparam_indx ) retval &= j;


    /* check for matching timestamp */
    if ( *selectn->time_start ) {
        if ( match_time ( selectn->time_start, af->timeval.tv_sec ) <= 0 ) retval &= 1;
        else retval &= 0;
    }
    if ( *selectn->time_end ) {
        if ( match_time ( selectn->time_end, af->timeval.tv_sec ) >= 0 ) retval &= 1;
        else retval &= 0;
    }


    return ( retval );
}


/* return comparison between user specified time string and numeric time_sec */
int match_time ( time_str, time_sec )
char *time_str;
time_t time_sec;
{
    struct tm *tma_p;               /* tm struct for time     */
    char tma_tm[TIME_LEN];          /* ascii rep for *tma_p   */
    int i, j;

    tma_p      =  localtime ( &time_sec );
    tma_tm[0]  =  tma_p->tm_year/10 + '0';
    tma_tm[1]  =  tma_p->tm_year%10 + '0';
    tma_tm[2]  =  (tma_p->tm_mon + 1)/10 + '0';
    tma_tm[3]  =  (tma_p->tm_mon + 1)%10 + '0';
    tma_tm[4]  =  tma_p->tm_mday/10 + '0';
    tma_tm[5]  =  tma_p->tm_mday%10 + '0';
    tma_tm[6]  =  tma_p->tm_hour/10 + '0';
    tma_tm[7]  =  tma_p->tm_hour%10 + '0';
    tma_tm[8]  =  tma_p->tm_min/10  + '0';
    tma_tm[9]  =  tma_p->tm_min%10  + '0';
    tma_tm[10] =  tma_p->tm_sec/10  + '0';
    tma_tm[11] =  tma_p->tm_sec%10  + '0';

    for ( i = 0; i < TIME_LEN && time_str[i]; i++ );
    j = strncmp ( time_str, tma_tm, i );

    return(j);
}


/* output address vector data (& label) */
output_addrvec ( addrvec, len, label, bp, ofs )
struct sockaddr_in *addrvec;
u_int len;
char *label;
char *bp;
u_int *ofs;
{
    struct sockaddr_in sin;
    char *ptr;
    int i, j;

    for ( j = 0; j < len/sizeof(struct sockaddr_in); j++ ) {
        bcopy ( &addrvec[j], &sin, sizeof(struct sockaddr_in) );

        if ( j == 0 ) {
            if ( label ) sprintf_l ( bp, ofs, "%.12s ", label );
            else sprintf_l ( bp, ofs, "addr vector: " );
            sprintf_l ( bp, ofs, "%s", inet_ntoa(sin.sin_addr) );
        }
        else sprintf_l ( bp, ofs, "             %s", inet_ntoa(sin.sin_addr) );

        if ( ptr = gethost_l(sin.sin_addr) ) sprintf_l ( bp, ofs, " (%s)", ptr );
        sprintf_l ( bp, ofs, "\n" );
    }
}


/* output specifics for audcntl() */
output_audcntl ( af, bp, ofs )
struct audit_fields *af;
char *bp;
u_int *ofs;
{
    int i;

    /* audcntl operation arguments */
    if ( af->intp_indx < 3 ) return;
    switch ( af->intparam[0] ) {

    case SET_SITEMASK:
        sprintf_l ( bp, ofs, "mask:        " );
        for ( i = 0; af->charp_indx && i < af->charlen[0]; i++ )
            sprintf_l ( bp, ofs, "%02X", af->charparam[0][i]&0xff );
        sprintf_l ( bp, ofs, "\n" );
        /* fall through */
    case SET_SYS_AMASK:  case SET_TRUSTED_AMASK:  case SET_PROC_AMASK:
        if ( af->charp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "mask:        " );
        output_audmask ( bp, ofs, af->intparam[0], af->charlen[0], af->charparam[0] );
        sprintf_l ( bp, ofs, "\n" );
        break;

    case SET_PROC_ACNTL:
        if ( af->intparam[1] == AUDIT_OR )
            sprintf_l ( bp, ofs, "cntl flag:   AUDIT_OR\n" );
        else if ( af->intparam[1] == AUDIT_AND )
            sprintf_l ( bp, ofs, "cntl flag:   AUDIT_AND\n" );
        else if ( af->intparam[1] == AUDIT_OFF )
            sprintf_l ( bp, ofs, "cntl flag:   AUDIT_OFF\n" );
        else if ( af->intparam[1] == AUDIT_USR )
            sprintf_l ( bp, ofs, "cntl flag:   AUDIT_USR\n" );
        break;

    case SET_AUDSTYLE:
        if ( af->intparam[1] == AUD_EXEC_ARGP )
            sprintf_l ( bp, ofs, "style flag:  AUD_EXEC_ARGP\n" );
        else if ( af->intparam[1] == AUD_EXEC_ENVP )
            sprintf_l ( bp, ofs, "style flag:  AUD_EXEC_ENVP\n" );
#ifdef __osf__
        else if ( af->intparam[1] == AUD_LOGIN_UNAME )
            sprintf_l ( bp, ofs, "style flag:  AUD_LOGIN_UNAME\n" );
#endif /* __osf__ */
        break;

    case SET_AUDSWITCH:  case GET_AUDSWITCH:
        sprintf_l ( bp, ofs, "audswitch:   %d\n", af->intparam[1] );
        break;

    case SETPAID:
        sprintf_l ( bp, ofs, "audit_id:    %d\n", af->intparam[2] );
        break;

    case UPDEVENTS:
        if ( af->intparam[2] != AUID_INVAL )
            sprintf_l ( bp, ofs, "audit_id:    %d\n", af->intparam[2] );
#ifdef ultrix
        if ( af->intp_indx > 3 && af->intparam[3] )
            sprintf_l ( bp, ofs, "process id:  %d\n", af->intparam[3] );
#endif /* ultrix */
        if ( af->intparam[1] == AUDIT_OR )
            sprintf_l ( bp, ofs, "cntl flag:   AUDIT_OR\n" );
        else if ( af->intparam[1] == AUDIT_AND )
            sprintf_l ( bp, ofs, "cntl flag:   AUDIT_AND\n" );
        else if ( af->intparam[1] == AUDIT_OFF )
            sprintf_l ( bp, ofs, "cntl flag:   AUDIT_OFF\n" );
        else if ( af->intparam[1] == AUDIT_USR )
            sprintf_l ( bp, ofs, "cntl flag:   AUDIT_USR\n" );
        if ( af->charp_indx ) {
            sprintf_l ( bp, ofs, "mask:        " );
            output_audmask ( bp, ofs, af->intparam[0], af->charlen[0], af->charparam[0] );
            sprintf_l ( bp, ofs, "\n" );
        }
        break;

#ifdef __osf__
    case SET_OBJAUDBIT:
        if ( af->charp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "object:      %s\n", af->charparam[0] );
        sprintf_l ( bp, ofs, "flag:        %d\n", af->intparam[1] );
        break;

    case SET_HABITAT_EVENT:
        if ( af->charp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "event:       %s\n", af->charparam[0] );
        sprintf_l ( bp, ofs, "flag:        %d\n", af->intparam[1] );
        break;
#endif __osf__

    }
}


/* format ascii display of auditmask; modify ofs to reflect # chars output */
output_audmask ( bp, ofs, audcntl_opt, len, mask )
char *bp;           /* pointer to output buffer   */
u_int *ofs;         /* ptr to offset into buffer  */
int audcntl_opt;    /* audcntl operation          */
int len;            /* auditmask length           */
char *mask;         /* auditmask                  */
{
    extern char *syscallnames[];
    extern char *trustedevent[];
    char eventname[STR_LEN2];
    int clear;
    int i, j, k;
    int adj = N_SYSCALLS%4;

    /* display syscall mask */
    if ( audcntl_opt == SET_SYS_AMASK || audcntl_opt == SET_PROC_AMASK ||
    audcntl_opt == UPDEVENTS ) {
        clear = 1;
        for ( j = 0; j < SYSCALL_MASK_LEN && j < len; j++ )
            for ( k = 0; k < 8; k+=2 ) {
                if ( (j<<2)+(k>>1) > LAST ) break;
                if ( mask[j] & (0x3 << (6-k)) ) {
                    sprintf_l ( bp, ofs, "%s", syscallnames[(j<<2)+(k>>1)] );
                    sprintf_l ( bp, ofs, ":%c:%c  ", 
                        (mask[j] & (0x2 << (6-k))) ? '1' : '0',
                        (mask[j] & (0x1 << (6-k))) ? '1' : '0' );
                    clear = 0;
                }
            }

#ifdef ultrix
        /* display special syscalll events */
        j = (N_SYSCALLS*2)/8 - 1;
        for ( k = 0; k < 8 && j < len; k+=2 )
            if ( mask[j] & (0x3 << (6-k)) ) {
                sprintf_l ( bp, ofs, "%s", special_event[k>>1] );
                sprintf_l ( bp, ofs, ":%c:%c  ",
                (mask[j] & (0x2 << (6-k))) ? '1' : '0',
                (mask[j] & (0x1 << (6-k))) ? '1' : '0' );
                clear = 0;
            }
#endif /* ultrix */

        if ( clear && audcntl_opt == SET_SYS_AMASK ) sprintf_l ( bp, ofs, "(clear)" );
    }


    /* display trusted event mask */
    if ( audcntl_opt == SET_TRUSTED_AMASK || audcntl_opt == SET_PROC_AMASK ||
    audcntl_opt == UPDEVENTS ) {
        clear = 1;

        /* an extra adjustment here, as the process' trustedmask
           appears to be contiguous with the process' syscallmask
        */
        if ( audcntl_opt == SET_PROC_AMASK || audcntl_opt == UPDEVENTS )
            i = adj ? SYSCALL_MASK_LEN-1 : SYSCALL_MASK_LEN;
        else i = 0;

        for ( j = 0; j < TRUSTED_MASK_LEN && i+j < len; j++ )
            for ( k = j == 0 ? adj<<1 : 0; k < 8; k+=2 ) {
                if ( mask[i+j] & (0x3 << (6-k)) ) {
                    sprintf_l ( bp, ofs, "%s", trustedevent[(j<<2)+(k>>1)-adj] );
                    sprintf_l ( bp, ofs, ":%c:%c  ",
                    (mask[i+j] & (0x2 << (6-k))) ? '1' : '0',
                    (mask[i+j] & (0x1 << (6-k))) ? '1' : '0' );
                    clear = 0;
                }
            }
        if ( clear ) sprintf_l ( bp, ofs, "(clear)" );
    }


    /* display site mask */
    if ( audcntl_opt == SET_SITEMASK ) {
        clear = 1;
        for ( j = 0; j < len; j++ )
            for ( k = 0; k < 8; k+=2 ) {
                if ( mask[j] & (0x3 << (6-k)) ) {
                    clear = 0;
                    if ( aud_sitevent ( MIN_SITE_EVENT+(j<<2)+(k>>1), -1, eventname, '\0' ) == -1 )
                        continue;
                    sprintf_l ( bp, ofs, "%s", eventname );
                    sprintf_l ( bp, ofs, ":%c:%c  ",
                    (mask[j] & (0x2 << (6-k))) ? '1' : '0',
                    (mask[j] & (0x1 << (6-k))) ? '1' : '0' );
                }
            }
        if ( clear ) sprintf_l ( bp, ofs, "(clear)" );
    }
}


/* format record output in abbreviated format into bp
   modify ofs to reflect # chars output
*/
output_brief_fmt ( af, bp, ofs, a_ptr, flag )
struct audit_fields *af;
char *bp;
u_int *ofs;
struct a_proc *a_ptr;
int flag;
{
    static int quick = 0;
    char *eventp, *subeventp;
    char *ptr;
    int i, j;


    /* output header and username */
    if ( flag & FLAG_LOCALID ) {
        if ( (quick == 0) && !(flag&FLAG_REPORT) ) {
            sprintf_l ( bp, ofs, "USERNAME    PID    RES/(ERR)          EVENT\n" );
            sprintf_l ( bp, ofs, "--------    ---    ---------          -----\n" );
            quick = 1;
        }
        ptr = output_username ( af, bp, ofs, a_ptr, flag );
        if ( ptr )
            j = sprintf_l ( bp, ofs, "%-10.8s", ptr );
        else if ( af->auid != (unsigned)AUID_INVAL )
            j = sprintf_l ( bp, ofs, "auid %d", af->auid );
        else if ( af->ruid != (unsigned)-1 )
            j = sprintf_l ( bp, ofs, "ruid %d", af->ruid ); 
        if ( j < 10 ) sprintf_l ( bp, ofs, "%*.*s", 10-j, 10-j, " " );
        /* mark events which have a modified uid */
        if ( af->uid != (unsigned)-1 && af->uid != af->ruid )
            sprintf_l ( bp, ofs, "* " );
        else sprintf_l ( bp, ofs, "  " );
    }

    /* output header and uids */
    if ( !(flag & FLAG_LOCALID) ) {
        if ( (quick == 0) && !(flag&FLAG_REPORT) ) {
            sprintf_l ( bp, ofs, "AUID:RUID:EUID    PID    RES/(ERR)          EVENT\n" );
            sprintf_l ( bp, ofs, "--------------    ---    ---------          -----\n" );
            quick = 1;
        }
        i = sprintf_l ( bp, ofs, "%d:%d:%d ", af->auid, af->ruid, af->uid );
        if ( i < 18 ) sprintf_l ( bp, ofs, "%*.*s", 18-i, 18-i, " " );
    }


    /* output result or error */
    if ( af->error ) {
        i = sprintf_l ( bp, ofs, "%-5d  (err %d) ", af->pid, af->error );
        if ( i < 26 ) sprintf_l ( bp, ofs, "%*.*s", 26-i, 26-i, " " );
    }
    else
        sprintf_l ( bp, ofs, "%-5d  %-18s ", af->pid, output_64(&af->result,af->version) );


    /* output event */
    get_eventnm ( af, af->event, af->subevent, &eventp, &subeventp );
    if ( af->event == SYS_exit && af->login_proc )
        sprintf_l ( bp, ofs, "exit (%s)\n", "logout" );
    else if ( strncmp ( eventp, "UNKNOWN EVENT", 13 ) == 0 )
        sprintf_l ( bp, ofs, "UNKNOWN EVENT (%d) ", af->event );
    else sprintf_l ( bp, ofs, "%s (", eventp );


    /* output various parameters */
    for ( i = 0; i < NUMSYSCALLARGS; i++ ) {
        if ( i < af->descrip_indx && af->error == 0 ) {
            j = af->descrip[i];
            if ( (a_ptr != (struct a_proc *)-1) && (j >= 0) && (j < NUM_FDS) ) {
                if ( a_ptr->fd_nm[j/NOFILE] && a_ptr->fd_nm[j/NOFILE][j%NOFILE] )
                    sprintf_l ( bp, ofs, " %s", a_ptr->fd_nm[j/NOFILE][j%NOFILE] );
                else sprintf_l ( bp, ofs, " %d", j );
            }
            else sprintf_l ( bp, ofs, " %d", j );
        }
        if ( i < af->descrip_indx && af->error )
            sprintf_l ( bp, ofs, " %d", af->descrip[i] );

        if ( i < af->login_indx )
            sprintf_l ( bp, ofs, " %.*s", af->login_len[i], af->login[i] );
        if ( i < af->charp_indx && af->event != SYS_audcntl )
            sprintf_l ( bp, ofs, " %.*s", af->charlen[i], af->charparam[i] );

        if ( i < af->intp_indx ) sprintf_l ( bp, ofs, " 0x%x", af->intparam[i] );
        if ( i < af->shortp_indx ) sprintf_l ( bp, ofs, " 0x%x", af->shortparam[i] );
        if ( i < af->lngprm_indx ) sprintf_l ( bp, ofs, " %s", output_64(&af->longparam[i],af->version) );
        if ( i < af->pid2_indx ) sprintf_l ( bp, ofs, " 0x%x", af->pid2[i] );
        if ( i < af->uid2_indx ) sprintf_l ( bp, ofs, " 0x%x", af->uid2[i] );
        if ( i < af->gid_indx ) sprintf_l ( bp, ofs, " 0x%x", af->gid[i] );
        if ( i < af->mode_indx ) sprintf_l ( bp, ofs, " %04o", af->mode[i] );
    }

    sprintf_l ( bp, ofs, " )\n" );
}


/* format event-specific char parameters output into bp; update ofs for # bytes output
   VERY DEPENDENT ON WHICH PARAMETERS PER SYSCALL ARE LOGGED
*/
int output_charparam_fmt ( af, bp, ofs )
struct audit_fields *af;
char *bp;
u_int *ofs;
{
    char buf[MAXPATHLEN];
    int i, j;

    switch ( af->event ) {

    case SYS_audcntl:
        break;


#ifdef __osf__
    case SYS_priocntlset:
        j = af->charlen[0] < MAXPATHLEN-1 ? af->charlen[0] : MAXPATHLEN-1;
        bcopy ( af->charparam[0], buf, j );
        buf[j] = '\0';
        sprintf_l ( bp, ofs, "class:       %s\n", buf );
        af->charp_indx = 0;
        break;
#endif /* __osf__ */


    default:
        for ( i = 0; i < af->charp_indx; i++ ) {
            if ( af->charlen[i] == 0 ) continue;
            j = af->charlen[i] < MAXPATHLEN-1 ? af->charlen[i] : MAXPATHLEN-1;
            bcopy ( af->charparam[i], buf, j );
            buf[j] = '\0';
            sprintf_l ( bp, ofs, "char param:  %s\n", buf );
        }

    }

}


/* format SYS_setgroups output; update ofs to reflect # chars output */
output_grp_fmt ( af, bp, ofs )
struct audit_fields *af;
char *bp;
u_int *ofs;
{
    struct group *grp;
    int i, j;

    if ( af->int_array_indx == 0 ) return;

    sprintf_l ( bp, ofs, "groups:      " );
    for ( i = 0; i < af->int_array_len[0]; i += sizeof(int) ) {
        bcopy ( af->int_array[0]+i, &j, sizeof(int) );
        if ( grp = getgrgid(j) )
            sprintf_l ( bp, ofs, "%s ", grp->gr_name );
        else sprintf_l ( bp, ofs, "%d ", j );
    }
    sprintf_l ( bp, ofs, "\n" );
}


/* format ipc output intp bp; update ofs to reflect # chars output */
output_ipc_fmt ( af, bp, ofs, a_ptr )
struct audit_fields *af;
char *bp;
u_int *ofs;
struct a_proc *a_ptr;
{
    struct sockaddr *sockptr;
    union {
        struct sockaddr sockbuf;
        char buf[MAXPATHLEN];
    } sock_un;
    char *ptr;
    u_int inaddr_any = INADDR_ANY;
    u_short family = 0;
    int i, j, k;


    /* output socket information */
    for ( i = 0; i < af->socket_indx; i++ ) {
        j = af->socketlen[i] < MAXPATHLEN-1 ? af->socketlen[i] : MAXPATHLEN-1;
        bcopy ( af->socketaddr[i], sock_un.buf, j );
        sock_un.buf[j] = '\0';
        sockptr = (struct sockaddr *)&sock_un.sockbuf;

#ifdef ultrix
        /* NOTE: this allows only one aud_client_info structure per record */
        /* aud_client_info structure keeps family separate */
        if ( af->x_client_indx ) {
            bcopy ( &af->x_client[af->x_client_indx-1].family, sock_un.buf, 2 );
            bcopy ( af->socketaddr[i], &sock_un.buf[2], j );
        }
#endif /* ultrix */

        /* sa_family here is 4.3 style (u_short); if family in
           audit_record appears to be > UCHAR_MAX, it must be
           4.4 style sockaddr of length (byte) then family (byte)
        */
        family = sockptr->sa_family;
        if ( family > UCHAR_MAX ) family >>= NBBY;
        switch ( family ) {

        case AF_UNIX:
            sprintf_l ( bp, ofs, "socket:      address (AF_UNIX) = %s\n", sockptr->sa_data );
            break;

        case AF_INET:
            sprintf_l ( bp, ofs, "socket:      port (AF_INET) = %d    ",
                ntohs(((struct sockaddr_in *)sockptr)->sin_port) );
            sprintf_l ( bp, ofs, "addr = %s",
                inet_ntoa(((struct sockaddr_in *)sockptr)->sin_addr) );
            if ( ptr = gethost_l(((struct sockaddr_in *)sockptr)->sin_addr) )
                sprintf_l ( bp, ofs, " (%s)", ptr );
            else if ( bcmp ( &((struct sockaddr_in *)sockptr)->sin_addr, &inaddr_any, 
            sizeof(inaddr_any) ) == 0 )
                sprintf_l ( bp, ofs, " (INADDR_ANY)" );
            sprintf_l ( bp, ofs, "\n" );
            break;

        case AF_UNSPEC:
            sprintf_l ( bp, ofs, "socket:      port (AF_UNSPEC) = %d\n",
                ntohs(((struct sockaddr_in *)sockptr)->sin_port) );
            break;

        default:
            sprintf_l ( bp, ofs, "socket:      address (unknown) = %s  address_family = %d\n",
            sockptr->sa_data, family );
            break;
        }
    }


    /* output message information */
    for ( i = 0; i < af->msg_indx; i++ ) {
        j = af->msglen[i] < MAXPATHLEN-1 ? af->msglen[i] : MAXPATHLEN-1;
        bcopy ( af->msgaddr[i], sock_un.buf, j );
        sock_un.buf[j] = '\0';
        sockptr = (struct sockaddr *)&sock_un.sockbuf;

        /* sa_family here is 4.3 style (u_short); if family in
           audit_record appears to be > UCHAR_MAX, it must be
           4.4 style sockaddr of length (byte) then family (byte)
        */
        family = sockptr->sa_family;
        if ( family > UCHAR_MAX ) family >>= NBBY;
        switch ( family ) {

        case AF_UNIX:
            sprintf_l ( bp, ofs, "socket:      address (AF_UNIX) = %s\n", sockptr->sa_data );
            break;

        case AF_INET:
            sprintf_l ( bp, ofs, "socket:      port (AF_INET) = %d\n",
                ntohs(((struct sockaddr_in *)sockptr)->sin_port) );
            break;

        default:
            sprintf_l ( bp, ofs, "socket:      address (unknown) = %s  address_family = %d\n",
            sockptr->sa_data, family );
            break;
        }
    }


    /* output access information */
    if ( family == AF_UNIX ) for ( i = 0; i < af->access_indx; i++ ) {
        for ( j = 0; j < af->accesslen[i]/sizeof(int); j++ ) {
            bcopy ( af->accessaddr[i]+j*sizeof(int), &k, sizeof(int) );
            if ( (a_ptr != (struct a_proc *)-1) && (k >= 0) && (k < NUM_FDS) ) {
                if ( a_ptr->fd_nm[k/NOFILE] && a_ptr->fd_nm[k/NOFILE][k%NOFILE] )
                    sprintf_l ( bp, ofs, "accrights:   %s (%d)\n", a_ptr->fd_nm[k/NOFILE][k%NOFILE], k );
                else sprintf_l ( bp, ofs, "accrights:   %d\n", k );
            }
            else sprintf_l ( bp, ofs, "accrights:   %d\n", k );
        }
    }


    /* output ipc information */
    for ( i = 0; i < af->ipc_uid_indx; i++ ) sprintf_l ( bp, ofs, "ipc_uid:     %d\n", af->ipc_uid[i] );
    for ( i = 0; i < af->ipc_gid_indx; i++ ) sprintf_l ( bp, ofs, "ipc_gid:     %d\n", af->ipc_gid[i] );
    for ( i = 0; i < af->ipc_mode_indx; i++ ) sprintf_l ( bp, ofs, "ipc_mode:    %04o\n", af->ipc_mode[i]&0x01ff );
}


/* output login specific information */
output_login_info ( af, bp, ofs, lvl )
struct audit_fields *af;
char *bp;
u_int *ofs;
int lvl;        /* 0: primary id only; 1: all secondary id's */
{
    char buf[MAXPATHLEN];
    int start = lvl ? 1 : 0;
    int stop = lvl ? AUD_NPARAM : 1;
    int i, j;


    for ( i = start; i < af->login_indx && i < stop; i++ ) {
        j = af->login_len[i] < MAXPATHLEN-1 ? af->login_len[i] : MAXPATHLEN-1;
        bcopy ( af->login[i], buf, j );
        buf[j] = '\0';
        sprintf_l ( bp, ofs, "login name:  %s\n", buf );
    }

    for ( i = start; i < af->homedir_indx && i < stop; i++ ) {
        j = af->homedir_len[i] < MAXPATHLEN-1 ? af->homedir_len[i] : MAXPATHLEN-1;
        bcopy ( af->homedir[i], buf, j );
        buf[j] = '\0';
        sprintf_l ( bp, ofs, "home dir:    %s\n", buf );
    }

    for ( i = start; i < af->shell_indx && i < stop; i++ ) {
        j = af->shell_len[i] < MAXPATHLEN-1 ? af->shell_len[i] : MAXPATHLEN-1;
        bcopy ( af->shell[i], buf, j );
        buf[j] = '\0';
        sprintf_l ( bp, ofs, "shell:       %s\n", buf );
    }

    for ( i = start; i < af->service_indx && i < stop; i++ ) {
        j = af->service_len[i] < MAXPATHLEN-1 ? af->service_len[i] : MAXPATHLEN-1;
        bcopy ( af->service[i], buf, j );
        buf[j] = '\0';
        sprintf_l ( bp, ofs, "service:     %s\n", buf );
    }

    for ( i = start; i < af->devname_indx && i < stop; i++ ) {
        j = af->devname_len[i] < MAXPATHLEN-1 ? af->devname_len[i] : MAXPATHLEN-1;
        bcopy ( af->devname[i], buf, j );
        buf[j] = '\0';
        sprintf_l ( bp, ofs, "devname:     %s\n", buf );
    }

    /* hostname (T_HOSTNAME) is always secondary id info */
    for ( i = start-1; i >= 0 && i < af->hostname_indx && i < stop; i++ ) {
        j = af->hostname_len[i] < MAXPATHLEN-1 ? af->hostname_len[i] : MAXPATHLEN-1;
        bcopy ( af->hostname[i], buf, j );
        buf[j] = '\0';
        sprintf_l ( bp, ofs, "hostname:    %s\n", buf );
    }
}


/* return string containing option name for given event and parameter */
char *output_option ( event, param1, param2 )
int event;
int param1;
{
    switch ( event ) {

    case SYS_audcntl:
        switch ( param1 ) {
        case GET_SYS_AMASK:             return ( "GET_SYS_AMASK" );
        case SET_SYS_AMASK:             return ( "SET_SYS_AMASK" );
        case GET_TRUSTED_AMASK:         return ( "GET_TRUSTED_AMASK" );
        case SET_TRUSTED_AMASK:         return ( "SET_TRUSTED_AMASK" );
        case GET_PROC_AMASK:            return ( "GET_PROC_AMASK" );
        case SET_PROC_AMASK:            return ( "SET_PROC_AMASK" );
        case GET_PROC_ACNTL:            return ( "GET_PROC_ACNTL" );
        case SET_PROC_ACNTL:            return ( "SET_PROC_ACNTL" );
        case GET_AUDSWITCH:             return ( "GET_AUDSWITCH" );
        case SET_AUDSWITCH:             return ( "SET_AUDSWITCH" );
        case GETPAID:                   return ( "GETPAID" );
        case SETPAID:                   return ( "SETPAID" );
        case GET_AUDSTYLE:              return ( "GET_AUDSTYLE" );
        case SET_AUDSTYLE:              return ( "SET_AUDSTYLE" );
        case GET_SITEMASK:              return ( "GET_SITEMASK" );
        case SET_SITEMASK:              return ( "SET_SITEMASK" );
#ifdef __osf__
        case GET_OBJAUDBIT:             return ( "GET_OBJAUDBIT" );
        case SET_OBJAUDBIT:             return ( "SET_OBJAUDBIT" );
        case GET_HABITAT_EVENT:         return ( "GET_HABITAT_EVENT" );
        case SET_HABITAT_EVENT:         return ( "SET_HABITAT_EVENT" );
        case GET_NSITEVENTS:            return ( "GET_NSITEVENTS" );
        case GET_AUDSIZE:               return ( "GET_AUDSIZE" );
#endif /* __osf__ */
        case UPDEVENTS:                 return ( "UPDEVENTS" );
        case FLUSH_AUD_BUF:             return ( "FLUSH_AUD_BUF" );
        default:                        return ( "(unknown)" );
        }


    case SYS_kill:
        switch ( param1 ) {
        case 0:                         return ( "null signal" );
        case SIGHUP:                    return ( "SIGHUP" );
        case SIGINT:                    return ( "SIGINT" );
        case SIGQUIT:                   return ( "SIGQUIT" );
        case SIGILL:                    return ( "SIGILL" );
        case SIGTRAP:                   return ( "SIGTRAP" );
        case SIGABRT:                   return ( "SIGABRT" );
        case SIGEMT:                    return ( "SIGEMT" );
        case SIGFPE:                    return ( "SIGFPE" );
        case SIGKILL:                   return ( "SIGKILL" );
        case SIGBUS:                    return ( "SIGBUS" );
        case SIGSEGV:                   return ( "SIGSEGV" );
        case SIGSYS:                    return ( "SIGSYS" );
        case SIGPIPE:                   return ( "SIGPIPE" );
        case SIGALRM:                   return ( "SIGALRM" );
        case SIGTERM:                   return ( "SIGTERM" );
        case SIGURG:                    return ( "SIGURG" );
        case SIGSTOP:                   return ( "SIGSTOP" );
        case SIGTSTP:                   return ( "SIGTSTP" );
        case SIGCONT:                   return ( "SIGCONT" );
        case SIGCHLD:                   return ( "SIGCHLD" );
        case SIGTTIN:                   return ( "SIGTTIN" );
        case SIGTTOU:                   return ( "SIGTTOU" );
        case SIGIO:                     return ( "SIGIO" );
        case SIGXCPU:                   return ( "SIGXCPU" );
        case SIGXFSZ:                   return ( "SIGXFSZ" );
        case SIGVTALRM:                 return ( "SIGVTALRM" );
        case SIGPROF:                   return ( "SIGPROF" );
        case SIGWINCH:                  return ( "SIGWINCH" );
#ifdef __osf__
        case SIGINFO:                   return ( "SIGINFO" );
#endif /* __osf__ */
#ifdef ultrix
        case SIGLOST:                   return ( "SIGLOST" );
#endif /* ultrix */
        case SIGUSR1:                   return ( "SIGUSR1" );
        case SIGUSR2:                   return ( "SIGUSR2" );
        default:                        return ( "(unknown)" );
        }


#ifdef __osf__
    case SYS_kloadcall:
        switch ( param1 ) {
        case KLC_VM_ALLOCATE:           return ( "KLC_VM_ALLOCATE" );
        case KLC_VM_DEALLOCATE:         return ( "KLC_VM_DEALLOCATE" );
        case KLC_VM_READ:               return ( "KLC_VM_READ" );
        case KLC_VM_WRITE:              return ( "KLC_VM_WRITE" );
        case KLC_VM_PROTECT:            return ( "KLC_VM_PROTECT" );
        case KLC_VM_ALLOCATE_WIRED:     return ( "KLC_VM_ALLOCATE_WIRED" );
        case KLC_CALL_FUNCTION:         return ( "KLC_CALL_FUNCTION" );
        default:                        return ( "(unknown)" );
        }


    case SYS_kmodcall:
        switch ( param1 ) {
        case SYSCONFIG_NOSPEC:          return ( "SYSCONFIG_NOSPEC" );
        case SYSCONFIG_CONFIGURE:       return ( "SYSCONFIG_CONFIGURE" );
        case SYSCONFIG_UNCONFIGURE:     return ( "SYSCONFIG_UNCONFIGURE" );
        case SYSCONFIG_QUERY:           return ( "SYSCONFIG_QUERY" );
#ifdef SYSCONFIG_RECONFIGURE
        case SYSCONFIG_RECONFIGURE:     return ( "SYSCONFIG_RECONFIGURE" );
#else
#ifdef SYSCONFIG_OPERATE
        case SYSCONFIG_OPERATE:         return ( "SYSCONFIG_OPERATE" );
#endif /* SYSCONFIG_OPERATE */
#endif /* SYSCONFIG_RECONFIGURE */

#ifdef SYSCONFIG_QUERYSIZE
        case SYSCONFIG_QUERYSIZE:       return ( "SYSCONFIG_QUERYSIZE" );
#endif /* SYSCONFIG_QUERYSIZE */
        default:                        return ( "(unknown)" );
        }
#endif /* __osf__ */


#ifdef __osf__
    case SYS_memcntl:
        switch ( param1 ) {
        case MC_SYNC:                   return ( "MC_SYNC" );
        case MC_LOCK:                   return ( "MC_LOCK" );
        case MC_UNLOCK:                 return ( "MC_UNLOCK" );
        case MC_LOCKAS:                 return ( "MC_LOCKAS" );
        case MC_UNLOCKAS:               return ( "MC_UNLOCKAS" );
        default:                        return ( "(unknown)" );
        }
#endif /* __osf__ */


    case SYS_ptrace:
        switch ( param1 ) {
        case PT_TRACE_ME:               return ( "PT_TRACE_ME" );
        case PT_READ_I:                 return ( "PT_READ_I" );
        case PT_READ_D:                 return ( "PT_READ_D" );
        case PT_READ_U:                 return ( "PT_READ_U" );
        case PT_WRITE_I:                return ( "PT_WRITE_I" );
        case PT_WRITE_D:                return ( "PT_WRITE_D" );
        case PT_WRITE_U:                return ( "PT_WRITE_U" );
        case PT_CONTINUE:               return ( "PT_CONTINUE" );
        case PT_KILL:                   return ( "PT_KILL" );
        case PT_STEP:                   return ( "PT_STEP" );
        default:                        return ( "(unknown)" );
        }


#ifdef SYS_security
    case SYS_security:
        switch ( param1 ) {
        case SEC_STOPIO:                return ( "SEC_STOPIO" );
        case SEC_GETLUID:               return ( "SEC_GETLUID" );
        case SEC_SETLUID:               return ( "SEC_SETLUID" );
        case SEC_STATPRIV:              return ( "SEC_STATPRIV" );
        case SEC_CHPRIVSYS:             return ( "SEC_CHPRIVSYS" );
#if SEC_ARCH
        case SEC_EACCESS:               return ( "SEC_EACCESS" );
#if SEC_MAC
        case SEC_MKMULTDIR:             return ( "SEC_MKMULTDIR" );
        case SEC_RMMULTDIR:             return ( "SEC_RMMULTDIR" );
        case SEC_ISMULTDIR:             return ( "SEC_ISMULTDIR" );
#endif /* SEC_MAC */
        case SEC_GETLABEL:              return ( "SEC_GETLABEL" );
        case SEC_SETLABEL:              return ( "SEC_SETLABEL" );
        case SEC_LMOUNT:                return ( "SEC_LMOUNT" );
        case SEC_ISLABELEDFS:           return ( "SEC_ISLABELEDFS" );
#endif /* SEC_ARCH */
        case SEC_SWITCH_CALL:
            switch ( param2 ) {
            case (int)SEC_SWITCH_STAT:  return ( "SEC_SWITCH_CALL - STAT" );
            case (int)SEC_SWITCH_ON:    return ( "SEC_SWITCH_CALL - ON" );
            case (int)SEC_SWITCH_OFF:   return ( "SEC_SWITCH_CALL - OFF" );
            case (int)SEC_SWITCH_CONF:  return ( "SEC_SWITCH_CALL - CONFIG" );
            default:                    return ( "SEC_SWITCH_CALL - (unknown)" );
            }
        default:                        return ( "(unknown)" );
        }
#endif /* SYS_security */


    case SYS_semctl:
        switch ( param1 ) {
        case GETNCNT:                   return ( "GETNCNT" );
        case GETPID:                    return ( "GETPID" );
        case GETVAL:                    return ( "GETVAL" );
        case GETALL:                    return ( "GETALL" );
        case GETZCNT:                   return ( "GETZCNT" );
        case SETVAL:                    return ( "SETVAL" );
        case SETALL:                    return ( "SETALL" );
        }
        /* drop into next "case SYS_msgctl" */
    case SYS_msgctl:
#ifdef __osf__
    case SYS_shmctl:
#endif /* __osf__ */
        switch ( param1 ) {
        case IPC_RMID:                  return ( "IPC_RMID" );
        case IPC_SET:                   return ( "IPC_SET" );
        case IPC_STAT:                  return ( "IPC_STAT" );
        default:                        return ( "(unknown)" );
        }


    case SYS_setsockopt:
        switch ( param1 ) {             /* socket/protocol */

        case SOL_SOCKET:
            switch ( param2 ) {         /* option */
            case SO_DEBUG:              return ( "SO_DEBUG" );
            case SO_ACCEPTCONN:         return ( "SO_ACCEPTCONN" );
            case SO_REUSEADDR:          return ( "SO_REUSEADDR" );
            case SO_KEEPALIVE:          return ( "SO_KEEPALIVE" );
            case SO_DONTROUTE:          return ( "SO_DONTROUTE" );
            case SO_BROADCAST:          return ( "SO_BROADCAST" );
            case SO_USELOOPBACK:        return ( "SO_USELOOPBACK" );
            case SO_LINGER:             return ( "SO_LINGER" );
            case SO_OOBINLINE:          return ( "SO_OOBINLINE" );
#ifdef SO_REUSEPORT
            case SO_REUSEPORT:          return ( "SO_REUSEPORT" );
#endif /* SO_REUSEPORT */
#if SEC_BASE
            case SO_EXPANDED_RIGHTS:    return ( "SO_EXPANDED_RIGHTS" );
#endif
            case SO_SNDBUF:             return ( "SO_SNDBUF" );
            case SO_RCVBUF:             return ( "SO_RCVBUF" );
            case SO_SNDLOWAT:           return ( "SO_SNDLOWAT" );
            case SO_RCVLOWAT:           return ( "SO_RCVLOWAT" );
            case SO_SNDTIMEO:           return ( "SO_SNDTIMEO" );
            case SO_RCVTIMEO:           return ( "SO_RCVTIMEO" );
            case SO_ERROR:              return ( "SO_ERROR" );
            case SO_TYPE:               return ( "SO_TYPE" );
            default:                    return ( "(unknown SOL_SOCKET option)" );
            }

        case IPPROTO_RAW:
        case IPPROTO_IP:
            switch ( param2 ) {
            case IP_OPTIONS:            return ( "IP_OPTIONS" );
#ifdef __osf__
            case IP_MULTICAST_IF:       return ( "IP_MULTICAST_IF" );
            case IP_MULTICAST_TTL:      return ( "IP_MULTICAST_TTL" );
            case IP_MULTICAST_LOOP:     return ( "IP_MULTICAST_LOOP" );
            case IP_ADD_MEMBERSHIP:     return ( "IP_ADD_MEMBERSHIP" );
            case IP_DROP_MEMBERSHIP:    return ( "IP_DROP_MEMBERSHIP" );
            case IP_HDRINCL:            return ( "IP_HDRINCL" );
            case IP_TOS:                return ( "IP_TOS" );
            case IP_TTL:                return ( "IP_TTL" );
            case IP_RECVOPTS:           return ( "IP_RECVOPTS" );
            case IP_RECVRETOPTS:        return ( "IP_RECVRETOPTS" );
            case IP_RECVDSTADDR:        return ( "IP_RECVDSTADDR" );
            case IP_RETOPTS:            return ( "IP_RETOPTS" );
#endif /* __osf__ */
            default:                    return ( "(unknown IPPROTO_IP/IPPROTO_RAW option)" );
            }

        case IPPROTO_TCP:
            switch ( param2 ) {
            case TCP_NODELAY:           return ( "TCP_NODELAY" );
            case TCP_MAXSEG:            return ( "TCP_MAXSEG" );
#ifdef __osf__
            case TCP_RPTR2RXT:          return ( "TCP_RPTR2RXT" );
#endif /* __osf__ */
            default:                    return ( "(unknown IPPROTO_TCP option)" );
            }

        case IPPROTO_IDP:
            switch ( param2 ) {
#ifdef __osf__
            case SO_HEADERS_ON_INPUT:   return ( "IDP/SO_HEADERS_ON_INPUT" );
            case SO_HEADERS_ON_OUTPUT:  return ( "IDP/SO_HEADERS_ON_OUTPUT" );
            case SO_DEFAULT_HEADERS:    return ( "IDP/SO_DEFAULT_HEADERS" );
            case SO_LAST_HEADER:        return ( "IDP/SO_LAST_HEADER" );
            case SO_NSIP_ROUTE:         return ( "IDP/SO_NSIP_ROUTE" );
            case SO_SEQNO:              return ( "IDP/SO_SEQNO" );
            case SO_ALL_PACKETS:        return ( "IDP/SO_ALL_PACKETS" );
            case SO_MTU:                return ( "IDP/SO_MTU" );
#endif /* __osf__ */
            default:                    return ( "(unknown IPPROTO_IDP option)" );
            }

        case IPPROTO_ICMP:
            switch ( param2 ) {
            default:                    return ( "(unknown IPPROTO_ICMP option)" );
            }

        case IPPROTO_GGP:
            switch ( param2 ) {
            default:                    return ( "(unknown IPPROTO_GGP option)" );
            }

        case IPPROTO_PUP:
            switch ( param2 ) {
            default:                    return ( "(unknown IPPROTO_PUP option)" );
            }

        case IPPROTO_UDP:
            switch ( param2 ) {
            default:                    return ( "(unknown IPPROTO_UDP option)" );
            }

        case IPPROTO_EGP:
            switch ( param2 ) {
            default:                    return ( "(unknown IPPROTO_EGP option)" );
            }

#ifdef __osf__
        case IPPROTO_IGMP:
            switch ( param2 ) {
            default:                    return ( "(unknown IPPROTO_IGMP option)" );
            }

        case IPPROTO_TP:
            switch ( param2 ) {
            default:                    return ( "(unknown IPPROTO_TP option)" );
            }

        case IPPROTO_EON:
            switch ( param2 ) {
            default:                    return ( "(unknown IPPROTO_EON option)" );
            }
#endif /* __osf__ */

        default:                        return ( "(unknown protocol/option)" );
        } /* end SYS_setsockopt */


    case SYS_setsysinfo:
        switch ( param1 ) {
        case SSI_NVPAIRS:               return ( "SSI_NVPAIRS" );
        case SSI_ZERO_STRUCT:           return ( "SSI_ZERO_STRUCT" );
        case SSI_SET_STRUCT:            return ( "SSI_SET_STRUCT" );
        case SSI_LMF:                   return ( "SSI_LMF" );
        case SSI_LOGIN:                 return ( "SSI_LOGIN" );
#ifdef __osf__
        case SSI_SLIMIT:                return ( "SSI_SLIMIT" );
        case SSI_ULIMIT:                return ( "SSI_ULIMIT" );
        case SSI_DUMPDEV:               return ( "SSI_DUMPDEV" );
        case SSI_SIA_PROC_CRED_VAL:     return ( "SSI_SIA_PROC_CRED_VAL" );
        case SSI_IPRSETUP:              return ( "SSI_IPRSETUP" );
#endif /* __osf__ */
        default:                        return ( "(unknown)" );
        }


#ifdef __osf__
    case SYS_swapctl:
        switch ( param1 ) {
        case SC_ADD:                    return ( "SC_ADD" );
        case SC_LIST:                   return ( "SC_LIST" );
        case SC_REMOVE:                 return ( "SC_REMOVE" );
        case SC_GETNSWP:                return ( "SC_GETNSWP" );
        default:                        return ( "(unknown)" );
        }
#endif /* __osf__ */


#ifdef __osf__
    case SYS_sysinfo:
        switch ( param1 ) {
        case SI_SYSNAME:                return ( "SI_SYSNAME" );
        case SI_HOSTNAME:               return ( "SI_HOSTNAME" );
        case SI_RELEASE:                return ( "SI_RELEASE" );
        case SI_VERSION:                return ( "SI_VERSION" );
        case SI_MACHINE:                return ( "SI_MACHINE" );
        case SI_ARCHITECTURE:           return ( "SI_ARCHITECTURE" );
        case SI_HW_SERIAL:              return ( "SI_HW_SERIAL" );
        case SI_HW_PROVIDER:            return ( "SI_HW_PROVIDER" );
        case SI_SRPC_DOMAIN:            return ( "SI_SRPC_DOMAIN" );
        case SI_SET_HOSTNAME:           return ( "SI_SET_HOSTNAME" );
        case SI_SET_SRPC_DOMAIN:        return ( "SI_SET_SRPC_DOMAIN" );
        case SI_SET_SYSNAME:            return ( "SI_SET_SYSNAME" );
        default:                        return ( "(unknown)" );
        }
#endif /* __osf__ */


#ifdef __osf__
    case SYS_table:
        switch ( param1 ) {
        case TBL_TTYLOC:                return ( "TBL_TTYLOC" );
        case TBL_U_TTYD:                return ( "TBL_U_TTYD" );
        case TBL_UAREA:                 return ( "TBL_UAREA" );
        case TBL_LOADAVG:               return ( "TBL_LOADAVG" );
        case TBL_INCLUDE_VERSION:       return ( "TBL_INCLUDE_VERSION" );
        case TBL_FSPARAM:               return ( "TBL_FSPARAM" );
        case TBL_ARGUMENTS:             return ( "TBL_ARGUMENTS" );
        case TBL_MAXUPRC:               return ( "TBL_MAXUPRC" );
        case TBL_AID:                   return ( "TBL_AID" );
        case TBL_MODES:                 return ( "TBL_MODES" );
        case TBL_PROCINFO:              return ( "TBL_PROCINFO" );
        case TBL_ENVIRONMENT:           return ( "TBL_ENVIRONMENT" );
        case TBL_SYSINFO:               return ( "TBL_SYSINFO" );
        case TBL_DKINFO:                return ( "TBL_DKINFO" );
        case TBL_TTYINFO:               return ( "TBL_TTYINFO" );
        case TBL_MSGDS:                 return ( "TBL_MSGDS" );
        case TBL_SEMDS:                 return ( "TBL_SEMDS" );
        case TBL_SHMDS:                 return ( "TBL_SHMDS" );
        case TBL_MSGINFO:               return ( "TBL_MSGINFO" );
        case TBL_SEMINFO:               return ( "TBL_SEMINFO" );
        case TBL_SHMINFO:               return ( "TBL_SHMINFO" );
        case TBL_INTR:                  return ( "TBL_INTR" );
        case TBL_SWAPINFO:              return ( "TBL_SWAPINFO" );
        case TBL_SCALLS:                return ( "TBL_SCALLS" );
        case TBL_FILEINFO:              return ( "TBL_FILEINFO" );
        case TBL_TBLSTATS:              return ( "TBL_TBLSTATS" );
        case TBL_RUNQ:                  return ( "TBL_RUNQ" );
        case TBL_BUFFER:                return ( "TBL_BUFFER" );
        case TBL_KMEM:                  return ( "TBL_KMEM" );
        case TBL_PAGING:                return ( "TBL_PAGING" );
        case TBL_MAPINFO:               return ( "TBL_MAPINFO" );
        default:                        return ( "(unknown)" );
        }
#endif /* __osf__ */


#ifdef __osf__
    case SYS_unmount:
        switch ( param1 ) {
        case MNT_FORCE:                 return ( "MNT_FORCE" );
        case MNT_NOFORCE:               return ( "MNT_NOFORCE" );
        case MNT_SKIPSYSTEM:            return ( "MNT_SKIPSYSTEM" );
        default:                        return ( "(unknown)" );
        }
#endif /* __osf__ */


    default:                            return ( "(unknown)" );
    }
}


/* format event-specific parameters output into bp; update ofs for # bytes output */
/* VERY DEPENDENT ON WHICH PARAMETERS PER SYSCALL ARE LOGGED */
#ifdef __osf__
output_param_fmt ( af, bp, ofs, flag )
struct audit_fields *af;
char *bp;
u_int *ofs;
int flag;               /* misc options         */
{
    struct passwd *pw_ptr;
    struct group *gr_ptr;
    int event;
    int i;


    /* deal with habitat events */
    if ( af->eventp_indx ) {

        /* open */
        if ( !(bcmp ( af->eventp[0], "open", af->eventp_len[0] )) )
            event = SYS_open;
    }
    else event = af->event;


    switch ( event ) {

    /* audcntl */
    case SYS_audcntl:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "operation:   %s (%d)\n",
            output_option ( SYS_audcntl, af->intparam[0], 0 ), af->intparam[0] );
        output_audcntl ( af, bp, ofs );
        break;


    /* dup2: get previous filename/vnode info */
    case SYS_dup2:
        if ( af->descrip_indx > 1 && ((last_vno_id && last_vno_dev) || *close_buf) )
            sprintf_l ( bp, ofs, "previous state of descriptor %d:\n", af->descrip[1] );
        if ( last_vno_id && last_vno_dev ) {
            sprintf_l ( bp, ofs, " vnode id:   %d  \t", last_vno_id );
            sprintf_l ( bp, ofs, "vnode dev:   (%d,%d)\n", major(last_vno_dev), minor(last_vno_dev) );
        }
        if ( *close_buf && af->descrip_indx > 1 )
            sprintf_l ( bp, ofs, " descriptor: %s (%d)\n", close_buf, af->descrip[1] );
        if ( af->vp_id_indx ) af->vp_id_indx = 1;
        if ( af->vp_dev_indx ) af->vp_dev_indx = 1;
        if ( af->vp_mode_indx ) af->vp_mode_indx = 1;
        break;


    /* exportfs */
    case SYS_exportfs:
        if ( af->intp_indx < 1 ) break;
        i = af->intp_indx-1;
        switch ( af->intparam[0] ) {
        case EXPORTFS_CREATE:
            sprintf_l ( bp, ofs, "option:      CREATE   " );
            if ( af->uid2_indx )
                sprintf_l ( bp, ofs, "rootmap: %d   ", af->uid2[0] );
            if ( af->uid2_indx > 1 )
                sprintf_l ( bp, ofs, "anon: %d   ", af->uid2[1] );
            af->uid2_indx = 0;
            sprintf_l ( bp, ofs, "\nflags:      " );
            if ( af->intparam[i] == M_EXPORTED ) sprintf_l ( bp, ofs, " exported" );
            if ( af->intparam[i] & M_EXRDONLY ) sprintf_l ( bp, ofs, " read-only" );
            if ( af->intparam[i] & M_EXRDMOSTLY ) sprintf_l ( bp, ofs, " read-mostly" );
#ifdef SEC_ARCH
            if ( af->intparam[i] & M_SECURE ) sprintf_l ( bp, ofs, " labelled" );
#endif /* SEC_ARCH */
            sprintf_l ( bp, ofs, " (0x%x)\n", af->intparam[i] );
            if ( af->addrvec_indx && af->addrvec_len[0] )
                output_addrvec ( af->addrvec[0], af->addrvec_len[0], "writeaddrs:  ", bp, ofs );
            if ( af->addrvec_indx > 1 && af->addrvec_len[1] )
                output_addrvec ( af->addrvec[1], af->addrvec_len[1], "rootaddrs:   ", bp, ofs );
            af->addrvec_indx = 0;
            break;
        case EXPORTFS_REMOVE:
            sprintf_l ( bp, ofs, "option:      REMOVE\n" );
            break;
        case EXPORTFS_READ:
            sprintf_l ( bp, ofs, "option:      READ     cookie: 0x%x\n",
                af->intparam[i] );
            break;
        }
        break;


    /* ioctl: TIOCSTI */
    case SYS_ioctl:
        if ( af->charp_indx && af->intp_indx &&
        strncmp ( af->charparam[0], "TIOCSTI", af->charlen[0] ) == 0 ) {
            if ( isalnum(af->intparam[0]) )
                sprintf_l ( bp, ofs, "arg:         %c\n", af->intparam[0] );
            else
                sprintf_l ( bp, ofs, "arg:         \\%03o\n", af->intparam[0] );
        }
        break;


    /* kill */
    case SYS_kill:
    case SYS_sigsendset:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "signal:      %s (%d)\n",
            output_option ( SYS_kill, af->intparam[0], 0 ), af->intparam[0] );
        break;
    case SYS_old_killpg:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "pgrp:        %d\n", af->intparam[0] );
        sprintf_l ( bp, ofs, "signal:      %s (%d)\n",
            output_option ( SYS_kill, af->intparam[1], 0 ), af->intparam[1] );
        break;


    /* kloadcall */
    case SYS_kloadcall:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "operation:   %s (%d)\n",
            output_option ( SYS_kloadcall, af->intparam[0], 0 ), af->intparam[0] );
        switch ( af->intparam[0] ) {
        case KLC_VM_ALLOCATE:   case KLC_VM_WRITE:
            if ( af->lngprm_indx > 3 ) af->lngprm_indx = 3;
            break;
        case KLC_VM_DEALLOCATE: case KLC_VM_READ:
            if ( af->lngprm_indx > 2 ) af->lngprm_indx = 2;
            break;
        }
        for ( i = 0; i < af->lngprm_indx; i++ ) 
            sprintf_l ( bp, ofs, "long param:  %s\n", output_64(&af->longparam[i],af->version) );
        break;


    /* kmodcall */
    case SYS_kmodcall:
        if ( af->intp_indx < 1 ) break;
        sprintf_l ( bp, ofs, "operation:   %s (%d)\n",
            output_option ( SYS_kmodcall, af->intparam[0], 0 ), af->intparam[0] );
        if ( af->lngprm_indx < 1 ) break;
        sprintf_l ( bp, ofs, "address:     %s\n", 
            output_64(&af->longparam[0],af->version) );
        break;


    /* memcntl */
    case SYS_memcntl:
        if ( af->intp_indx < 4 ) break;
        sprintf_l ( bp, ofs, "operation:   %s (%d)\n",
            output_option ( SYS_memcntl, af->intparam[0], 0 ), af->intparam[0] );
        sprintf_l ( bp, ofs, "arg:         %d\n", af->intparam[1] );
        sprintf_l ( bp, ofs, "attribute:   %d", af->intparam[2] );
        if ( af->intparam[2] & SHARED ) sprintf_l ( bp, ofs, " shared" );
        if ( af->intparam[2] & PRIVATE ) sprintf_l ( bp, ofs, " private" );
        if ( af->intparam[2] & PROT_READ ) sprintf_l ( bp, ofs, " read" );
        if ( af->intparam[2] & PROT_WRITE ) sprintf_l ( bp, ofs, " write" );
        if ( af->intparam[2] & PROT_EXEC ) sprintf_l ( bp, ofs, " exec" );
        sprintf_l ( bp, ofs, "\nmask:        %d\n", af->intparam[3] );
        if ( af->lngprm_indx < 2 ) break;
        sprintf_l ( bp, ofs, "address:     %s\n", output_64(&af->longparam[0],af->version) );
        sprintf_l ( bp, ofs, "length:      %s\n", output_64(&af->longparam[1],af->version) );
        break;


    /* mmap, munmap */
    case SYS_mmap:
        if ( af->lngprm_indx < 1 || af->intp_indx < 2 ) break;
        if ( af->error )
            sprintf_l ( bp, ofs, "address:     %s   ", output_64(&af->longparam[0],af->version) );
        else
            sprintf_l ( bp, ofs, "address:     %s   ", output_64(&af->result,af->version) );
        sprintf_l ( bp, ofs, "len: %s   prot: %04o   flag: ", 
            output_64(&af->longparam[1],af->version), af->intparam[0] );
        if ( af->intparam[1] & MAP_SHARED ) sprintf_l ( bp, ofs, " shared" );
        if ( af->intparam[1] & MAP_PRIVATE ) sprintf_l ( bp, ofs, " private" );
        if ( af->intparam[1] & MAP_INHERIT ) sprintf_l ( bp, ofs, " inherit" );
        sprintf_l ( bp, ofs, "\n" );
        break;
    case SYS_munmap:
        if ( af->lngprm_indx < 2 ) break;
        sprintf_l ( bp, ofs, "address:     %s   ", output_64(&af->longparam[0],af->version) );
        sprintf_l ( bp, ofs, "len: %s\n", output_64(&af->longparam[1],af->version) );
        break;


    /* mount */
    case SYS_mount:
        if ( af->intp_indx < 1 ) break;
        if ( af->intparam[0] >= 0 && af->intparam[0] <= MOUNT_MAXTYPE )
            sprintf_l ( bp, ofs, "fs type:     %s\n", mnt_names[af->intparam[0]] );
        else sprintf_l ( bp, ofs, "fs type:     %d\n", af->intparam[0] );
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "flags:       0x%x :", af->intparam[1] );
        if ( af->intparam[1] & M_RDONLY ) sprintf_l ( bp, ofs, " rdonly" );
        if ( af->intparam[1] & M_SYNCHRONOUS ) sprintf_l ( bp, ofs, " synchronous" );
        if ( af->intparam[1] & M_NOEXEC ) sprintf_l ( bp, ofs, " noexec" );
        if ( af->intparam[1] & M_NOSUID ) sprintf_l ( bp, ofs, " nosuid" );
        if ( af->intparam[1] & M_NODEV ) sprintf_l ( bp, ofs, " nodev" );
        if ( af->intparam[1] & M_UPDATE ) sprintf_l ( bp, ofs, " update" );
#ifdef M_SYNCING
        if ( af->intparam[1] & M_SYNCING ) sprintf_l ( bp, ofs, " syncing" );
#endif M_SYNCING
#ifdef M_MPBUSY
        if ( af->intparam[1] & M_MPBUSY ) sprintf_l ( bp, ofs, " mpbusy" );
#endif M_MPBUSY
        if ( af->intparam[1] & M_FMOUNT ) sprintf_l ( bp, ofs, " fmount" );
        sprintf_l ( bp, ofs, "\n" );
        break;


    /* mprotect */
    case SYS_mprotect:
        if ( af->lngprm_indx < 2 || af->intp_indx < 1 ) break;
        sprintf_l ( bp, ofs, "address:     %s   ", output_64(&af->longparam[0],af->version) );
        sprintf_l ( bp, ofs, "len: %s   prot: %04o\n",
            output_64(&af->longparam[1],af->version), af->intparam[0] );
        break;


    /* msgop syscalls */
    case SYS_msgctl:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "msqid:       %d\n", af->intparam[0] );
        sprintf_l ( bp, ofs, "cmd:         %s (0x%x)\n",
            output_option ( SYS_msgctl, af->intparam[1], 0 ), af->intparam[1] );
        break;
    case SYS_msgrcv:    case SYS_msgsnd:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "msqid:       %d\n", af->intparam[0] );
        break;


    /* SYSVipc-get syscalls */
    case SYS_msgget:
    case SYS_semget:
    case SYS_shmget:
        if ( af->intp_indx < 2 ) break;
        if ( af->intparam[0] == IPC_PRIVATE ) sprintf_l ( bp, ofs, "key:         IPC_PRIVATE\n" );
        else sprintf_l ( bp, ofs, "key:         %d\n", af->intparam[0] );
        sprintf_l ( bp, ofs, "flag:        0x%x :", af->intparam[1] );
        if ( af->intparam[1] & IPC_CREAT )
            sprintf_l ( bp, ofs, " creat" );
        if ( af->intparam[1] & IPC_EXCL )
            sprintf_l ( bp, ofs, " excl" );
        sprintf_l ( bp, ofs, "\n" );
        if ( af->result.val[0] != (unsigned)-1 && af->result.val[1] != (unsigned)-1 )
            sprintf_l ( bp, ofs, "id:          %s\n", output_64(&af->result,af->version) );
        break;


    /* open */
    case SYS_open:  case SYS_old_open:
#ifdef FDFS_MAJOR
        /* special case of FDFS open */
        if ( af->vp_dev_indx && major(af->vnode_dev[0]) == FDFS_MAJOR ) {
            sprintf_l ( bp, ofs, "             FDFS filesystem open\n" );
            af->mode_indx--;
            break;
        }
#endif /* FDFS_MAJOR */
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "flags:       %d :", af->intparam[0] );
        if ( af->intparam[0] == O_RDONLY ) sprintf_l ( bp, ofs, " read" );
        if ( af->intparam[0] & O_WRONLY ) sprintf_l ( bp, ofs, " write" );
        if ( af->intparam[0] & O_RDWR ) sprintf_l ( bp, ofs, " rdwr" );
        if ( af->intparam[0] & O_TRUNC ) sprintf_l ( bp, ofs, " trunc" );
        if ( af->intparam[0] & O_CREAT ) sprintf_l ( bp, ofs, " creat" );
        else af->mode_indx--;  /* only if O_CREAT */
        sprintf_l ( bp, ofs, "\n" );
        break;


    /* priocntlset */
    case SYS_priocntlset:
        if ( af->intp_indx > 0 ) sprintf_l ( bp, ofs, "priority:    %d\n", af->intparam[0] );
        if ( af->intp_indx > 1 ) sprintf_l ( bp, ofs, "quantum:     %d millisecs\n", af->intparam[1] );
        break;


    /* ptrace */
    case SYS_ptrace:
        if ( af->intp_indx < 1 ) break;
        sprintf_l ( bp, ofs, "request:     %s (%d)\n",
            output_option ( SYS_ptrace, af->intparam[0], 0 ), af->intparam[0] );
        break;


    /* recvmsg */
    case SYS_recvmsg:
    case SYS_nrecvmsg:
        if ( af->error )
            af->vp_id_indx = af->vp_dev_indx = af->vp_mode_indx = 0;
        break;

    /* security */
#ifdef SYS_security
    case SYS_security:
        if ( af->intp_indx < 1 ) break;
        sprintf_l ( bp, ofs, "request:     %s (%d)\n",
            output_option ( SYS_security, af->intparam[0], 
            af->intp_indx < 2 ? 0 : af->intparam[1] ), af->intparam[0] );
        break;
#endif /* SYS_security */


    /* sem syscalls */
    case SYS_semctl:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "semid:       %d\n", af->intparam[0] );
        sprintf_l ( bp, ofs, "cmd:         %s (0x%x)\n",
            output_option ( SYS_semctl, af->intparam[1], 0 ), af->intparam[1] );
        break;
    case SYS_semop:
        if ( af->intp_indx < 1 ) break;
        sprintf_l ( bp, ofs, "semid:       %d\n", af->intparam[0] );
        break;


    /* setpgrp */
    case SYS_setpgrp:
        if ( af->intp_indx < 1 ) break;
        sprintf_l ( bp, ofs, "pgrp:        %d\n", af->intparam[0] );
        break;


    /* setre{u,g}id */
    case SYS_setregid:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "rgid:        %d", af->intparam[0] );
        if ( (flag & FLAG_LOCALID) && (gr_ptr = getgrgid(af->intparam[0])) )
            sprintf_l ( bp, ofs, " (%s)", gr_ptr->gr_name );
        sprintf_l ( bp, ofs, "\negid:        %d", af->intparam[1] );
        if ( (flag & FLAG_LOCALID) && (gr_ptr = getgrgid(af->intparam[1])) )
            sprintf_l ( bp, ofs, " (%s)", gr_ptr->gr_name );
        sprintf_l ( bp, ofs, "\n" );
        break;
    case SYS_setreuid:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "ruid:        %d", af->intparam[0] );
        if ( (flag & FLAG_LOCALID) && (pw_ptr = getpwuid(af->intparam[0])) )
            sprintf_l ( bp, ofs, " (%s)", pw_ptr->pw_name );
        sprintf_l ( bp, ofs, "\neuid:        %d", af->intparam[1] );
        if ( (flag & FLAG_LOCALID) && (pw_ptr = getpwuid(af->intparam[1])) )
            sprintf_l ( bp, ofs, " (%s)", pw_ptr->pw_name );
        sprintf_l ( bp, ofs, "\n" );
        break;


    /* setpriority */
    case SYS_setpriority:
        if ( af->intp_indx < 2 ) break;
        if ( af->intparam[0] == PRIO_PROCESS ) sprintf_l ( bp, ofs, "process:     %d\n", af->intparam[1] );
        else if ( af->intparam[0] == PRIO_PGRP) sprintf_l ( bp, ofs, "proc group:  %d\n", af->intparam[1] );
        else if ( af->intparam[0] == PRIO_USER ) sprintf_l ( bp, ofs, "uid:         %d\n", af->intparam[1] );
        break;


    /* setsockopt */
    case SYS_setsockopt:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "socket opt:  %s (level %d, option %d)\n",
            output_option ( SYS_setsockopt, af->intparam[0], af->intparam[1] ),
            af->intparam[0], af->intparam[1] );
        break;


    /* setsysinfo */
    case SYS_setsysinfo:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "operation:   %s (%d)\n",
            output_option ( SYS_setsysinfo, af->intparam[0], 0 ), af->intparam[0] );
        break;


    /* shm syscalls */
    case SYS_shmdt:
        if ( af->lngprm_indx == 0 ) break;
        sprintf_l ( bp, ofs, "shmaddr:     %s\n", output_64(&af->longparam[0],af->version) );
        break;
    case SYS_shmctl:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "shmid:       %d\n", af->intparam[0] );
        sprintf_l ( bp, ofs, "cmd:         %s (0x%x)\n",
            output_option ( SYS_shmctl, af->intparam[1], 0 ), af->intparam[1] );
        break;
    case SYS_shmat:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "shmid:       %d  shmflg: 0x%x :",
            af->intparam[0], af->intparam[1] );
        if ( af->intparam[1] & SHM_RDONLY )
            sprintf_l ( bp, ofs, " rdonly" );
        if ( af->intparam[1] & SHM_RND )
            sprintf_l ( bp, ofs, " shm_rnd" );
        sprintf_l ( bp, ofs, "\n" );
        if ( af->result.val[0] != (unsigned)-1 && af->result.val[1] != (unsigned)-1 )
            sprintf_l ( bp, ofs, "shmaddr:     %s\n", output_64(&af->result,af->version) );
        break;


    /* swapctl */
    case SYS_swapctl:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "cmd:         %s (%d)\n",
            output_option ( SYS_swapctl, af->intparam[0], 0 ), af->intparam[0] );
        if ( af->lngprm_indx == 0 ) break;
        sprintf_l ( bp, ofs, "offset:      %s\n", output_64(&af->longparam[0],af->version) );
        sprintf_l ( bp, ofs, "length:      %s\n", output_64(&af->longparam[1],af->version) );
        af->lngprm_indx = 0;
        break;


    /* sysinfo */
    case SYS_sysinfo:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "cmd:         %s (%d)\n",
            output_option ( SYS_sysinfo, af->intparam[0], 0 ), af->intparam[0] );
        break;


    /* table */
    case SYS_table:
        if ( af->intp_indx < 3 ) break;
        sprintf_l ( bp, ofs, "tbl option:  %s (%d)\n",
            output_option ( SYS_table, af->intparam[0], 0 ), af->intparam[0] );
        sprintf_l ( bp, ofs, "index:       %d\n", af->intparam[1] );
        sprintf_l ( bp, ofs, "# elements:  %d (%s)\n", af->intparam[2],
            af->intparam[2] < 0 ? "set" : "fetched" );
        break;


    /* unmount */
    case SYS_unmount:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "option:      %s (%d)\n",
            output_option ( SYS_unmount, af->intparam[0], 0 ), af->intparam[0] );
        break;


    default:
        for ( i = 0; i < af->intp_indx; i++ )
            sprintf_l ( bp, ofs, "int param:   %d\n", af->intparam[i] );
        for ( i = 0; i < af->shortp_indx; i++ )
            sprintf_l ( bp, ofs, "short param: %d\n", af->shortparam[i] );
        for ( i = 0; i < af->lngprm_indx; i++ ) 
            sprintf_l ( bp, ofs, "long param:  %s\n", output_64(&af->longparam[i],af->version) );

    }
}
#endif /* __osf__ */


/* format event-specific parameters output into bp; update ofs for # bytes output */
/* VERY DEPENDENT ON WHICH PARAMETERS PER SYSCALL ARE LOGGED */
#ifdef ultrix
output_param_fmt ( af, bp, ofs, flag )
struct audit_fields *af;
char *bp;
u_int *ofs;
int flag;               /* misc options         */
{
    struct passwd *pw_ptr;
    struct group *gr_ptr;
    int i;


    switch ( af->event ) {

    /* audcntl */
    case SYS_audcntl:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "operation:   %s (%d)\n",
            output_option ( SYS_audcntl, af->intparam[0], 0 ), af->intparam[0] );
        output_audcntl ( af, bp, ofs );
        break;


    /* chmod, fchmod */
    case SYS_chmod:     case SYS_fchmod:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "mode:        %04o\n", af->intparam[0] );
        break;


    /* chown, fchown */
    case SYS_chown:     case SYS_fchown:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "owner:       %d", af->intparam[0] );
        if ( (flag & FLAG_LOCALID) && (pw_ptr = getpwuid(af->intparam[0])) )
            sprintf_l ( bp, ofs, " (%s)", pw_ptr->pw_name );
        sprintf_l ( bp, ofs, "\ngroup:       %d", af->intparam[1] );
        if ( (flag & FLAG_LOCALID) && (gr_ptr = getgrgid(af->intparam[1])) )
            sprintf_l ( bp, ofs, " (%s)", gr_ptr->gr_name );
        sprintf_l ( bp, ofs, "\n" );
        break;


    /* creat, mknod, mkdir */
    case SYS_creat:     case SYS_mkdir:     case SYS_mknod:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "mode:        %04o\n", af->intparam[0] );
        for ( i = 1; i < af->intp_indx; i++ )
            sprintf_l ( bp, ofs, "int param:   %d\n", af->intparam[i] );
        break;


    /* exportfs - NOTE different value for SYS_exportfs on mips & vax */
    case 168: case 169:
        if ( af->intp_indx < 2 ) break;
        switch ( af->intparam[0] ) {
        case EXPORTFS_CREATE:
            sprintf_l ( bp, ofs, "option:      CREATE   " );
            if ( af->shortp_indx )
                sprintf_l ( bp, ofs, "rootmap: %d   ", af->shortparam[0] );
            i = af->intp_indx-1;
            sprintf_l ( bp, ofs, "flags:" );
            if ( af->intparam[i] & M_NOFH ) sprintf_l ( bp, ofs, " no fhandle" );
            if ( af->intparam[i] & M_EXRONLY ) sprintf_l ( bp, ofs, " read-only" );
            sprintf_l ( bp, ofs, " (0x%x)\n", af->intparam[i] );
            break;
        case EXPORTFS_REMOVE:
            sprintf_l ( bp, ofs, "option:      REMOVE\n" );
            break;
        case EXPORTFS_READ:
            sprintf_l ( bp, ofs, "option:      READ     cookie: 0x%x\n",
                af->intparam[1] );
            break;
        }
        break;


    /* fcntl: only with F_DUPFD */
    case SYS_fcntl:
        sprintf_l ( bp, ofs, "operation:   F_DUPFD\n" );
        break;


    /* dup2: get previous filename/vnode info */
    case SYS_dup2:
        if ( af->descrip_indx > 1 && ((last_vno_id && last_vno_dev) || *close_buf) )
            sprintf_l ( bp, ofs, "previous state of descriptor %d:\n", af->descrip[1] );
        if ( last_vno_id && last_vno_dev ) {
            sprintf_l ( bp, ofs, " gnode id:   %d  \t", last_vno_id );
            sprintf_l ( bp, ofs, "gnode dev:   (%d,%d)\n", major(last_vno_dev), minor(last_vno_dev) );
        }
        if ( *close_buf && af->descrip_indx > 1 )
            sprintf_l ( bp, ofs, " descriptor: %s (%d)\n", close_buf, af->descrip[1] );
        break;


    /* ioctl: only with TIOCSTI */
    case SYS_ioctl:
        sprintf_l ( bp, ofs, "operation:   TIOCSTI\n" );
        break;


    /* kill, killpg */
    case SYS_kill:
    case SYS_killpg:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "pid:         %d\n", af->intparam[0] );
        break;


    /* mmap, munmap */
    case SYS_mmap:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "address:     %s   len: %d   prot: %04o\n",
            output_64(&af->result,af->version), af->intparam[0], af->intparam[1] );
        break;
    case SYS_munmap:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "address:     0x%x   len: %d\n",
            af->intparam[0], af->intparam[1] );
        break;


    /* mprotect */
    case SYS_mprotect:
        if ( af->intp_indx < 3 ) break;
        sprintf_l ( bp, ofs, "address:     0x%lx   len: %d   prot: %04o\n",
            af->intparam[0], af->intparam[1], af->intparam[2] );
        break;


    /* msg syscalls */
    case SYS_msgctl:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "msqid:       %d  cmd: 0x%x\n",
            af->intparam[0], af->intparam[1] );
        break;
    case SYS_msgget:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "msgflag:     0x%x\n", af->intparam[0] );
        if ( af->result.val[0] != -1L )
            sprintf_l ( bp, ofs, "msqid        %ld\n", af->result.val[0] );
        break;
    case SYS_msgrcv:    case SYS_msgsnd:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "msqid:       %d\n", af->intparam[0] );
        break;


    /* open */
    case SYS_open:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "flags:       %d :", af->intparam[0] );
        if ( af->intparam[0] == O_RDONLY ) sprintf_l ( bp, ofs, " read" );
        if ( af->intparam[0] & O_WRONLY ) sprintf_l ( bp, ofs, " write" );
        if ( af->intparam[0] & O_RDWR ) sprintf_l ( bp, ofs, " rdwr" );
        if ( af->intparam[0] & O_TRUNC ) sprintf_l ( bp, ofs, " trunc" );
        if ( af->intparam[0] & O_CREAT ) sprintf_l ( bp, ofs, " creat" );
        sprintf_l ( bp, ofs, "\n" );
        if ( (af->intparam[0] & O_CREAT) )
            sprintf_l ( bp, ofs, "mode:        %04o\n", af->intparam[1] );
        break;


    /* ptrace */
    case SYS_ptrace:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "request      %s (%d)  pid: %d\n",
            output_option ( SYS_ptrace, af->intparam[0], 0 ),
            af->intparam[0], af->intparam[1] );
        break;


    /* semaphore syscalls */
    case SYS_semctl:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "semid:       %d  cmd: 0x%x\n",
            af->intparam[0], af->intparam[1] );
        break;
    case SYS_semget:
        if ( af->intp_indx < 1 ) break;
        sprintf_l ( bp, ofs, "key:         %d\n", af->intparam[0] );
        break;
    case SYS_semop:
        if ( af->intp_indx < 1 ) break;
        sprintf_l ( bp, ofs, "semid:       %d\n", af->intparam[0] );
        break;


    /* setpgrp */
    case SYS_setpgrp:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "pid:         %d     pgrp: %d\n",
            af->intparam[0], af->intparam[1] );
        break;


    /* setregid */
    case SYS_setregid:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "rgid:        %d", af->intparam[0] );
        if ( (flag & FLAG_LOCALID) && (gr_ptr = getgrgid(af->intparam[0])) )
            sprintf_l ( bp, ofs, " (%s)", gr_ptr->gr_name );
        sprintf_l ( bp, ofs, "\negid:        %d", af->intparam[1] );
        if ( (flag & FLAG_LOCALID) && (gr_ptr = getgrgid(af->intparam[1])) )
            sprintf_l ( bp, ofs, " (%s)", gr_ptr->gr_name );
        sprintf_l ( bp, ofs, "\n" );
        break;

    /* setreuid */
    case SYS_setreuid:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "ruid:        %d", af->intparam[0] );
        if ( (flag & FLAG_LOCALID) && (pw_ptr = getpwuid(af->intparam[0])) )
            sprintf_l ( bp, ofs, " (%s)", pw_ptr->pw_name );
        sprintf_l ( bp, ofs, "\neuid:        %d", af->intparam[1] );
        if ( (flag & FLAG_LOCALID) && (pw_ptr = getpwuid(af->intparam[1])) )
            sprintf_l ( bp, ofs, " (%s)", pw_ptr->pw_name );
        sprintf_l ( bp, ofs, "\n" );
        break;


    /* shm syscalls */
    case SYS_SHMGET:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "shmflg:      0x%x\n", af->intparam[0] );
        if ( af->result.val[0] != -1 && af->result.val[1] != -1 )
            sprintf_l ( bp, ofs, "shmid        %s\n", output_64(&af->result,af->version) );
        break;
    case SYS_SHMDT:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "shmaddr:     0x%x\n", af->intparam[0] );
        break;
    case SYS_SHMCTL:
        if ( af->intp_indx < 2 ) break;
        sprintf_l ( bp, ofs, "shmid:       %d  cmd: %d\n",
            af->intparam[0], af->intparam[1] );
        break;
    case SYS_SHMAT:
        if ( af->intp_indx < 3 ) break;
        sprintf_l ( bp, ofs, "shmid:       %d  shmflg: 0x%x\n",
            af->intparam[0], af->intparam[2] );
        if ( af->result.val[0] != -1 && af->result.val[1] != -1 )
            sprintf_l ( bp, ofs, "shmaddr:     %s\n", output_64(&af->result,af->version) );
        break;


    /* umount */
    case SYS_umount:
        if ( af->intp_indx == 0 ) break;
        sprintf_l ( bp, ofs, "device:      (%d,%d)\n",
            major(af->intparam[0]), minor(af->intparam[0]) );
        break;


    /* setpriority */
    case SYS_setpriority:
        if ( af->intp_indx < 2 ) break;
        if ( af->intparam[0] == PRIO_PROCESS ) sprintf_l ( bp, ofs, "process:     %d\n", af->intparam[1] );
        else if ( af->intparam[0] == PRIO_PGRP) sprintf_l ( bp, ofs, "proc group:  %d\n", af->intparam[1] );
        else if ( af->intparam[0] == PRIO_USER ) sprintf_l ( bp, ofs, "uid:         %d\n", af->intparam[1] );
        break;


    default:
        for ( i = 0; i < af->intp_indx; i++ )
            sprintf_l ( bp, ofs, "int param:   %d\n", af->intparam[i] );
        for ( i = 0; i < af->shortp_indx; i++ )
            sprintf_l ( bp, ofs, "short param: %d\n", af->shortparam[i] );
        for ( i = 0; i < af->lngprm_indx; i++ )
            sprintf_l ( bp, ofs, "long param:  %s\n", output_64(&af->longparam[i],af->version) );

    }
}
#endif /* ultrix */


/* format audit record output into bp; return # chars output */
int output_rec_fmt ( bp, af, flag )
char *bp;
struct audit_fields af;
int flag;               /* misc options         */
{
    struct a_proc *a_ptr;
    char *eventp, *subeventp;
    u_int ofs = 0;
    extern int  sys_nerr;
    extern char *sys_errlist[];

    struct passwd *pw_ptr;
    struct group *gr_ptr;
    struct tm *tma_p;

    char *ptr;
    int i, j, k;


    /* flag represents length consistency check */
    if ( af.flag == AF_LOGBREAK ) return ( ofs );
    else if ( af.flag == AF_LOGCHNG ) {
        sprintf_l ( bp, &ofs, "\007\007NOTE -- partial audit record read\n" );
        return ( ofs );
    }
    else if ( af.flag == AF_RECOVER ) sprintf_l ( bp, &ofs, "\007\007NOTE -- partial audit record recovered\n" );
    else if ( af.flag ) sprintf_l ( bp, &ofs, "\007\007WARNING -- audit record corrupted (%s)\n\n", af_msg[af.flag] );

    /* get a_proc struct for current process */
    a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af.pid, af.ipaddr, 0 );


    /* quick output */
    if ( flag & FLAG_BRIEF ) {
        output_brief_fmt ( &af, bp, &ofs, a_ptr, flag );
        return ( ofs );
    }


    /* output audit record header (formatted for display) */
    if ( af.auid != (unsigned)AUID_INVAL ) 
        sprintf_l ( bp, &ofs, "audit_id:    %-13ld ", af.auid );
    if ( af.ruid != (unsigned)-1 && af.uid != (unsigned)-1 ) {
        i = sprintf_l ( bp, &ofs, "ruid/euid:   %d/%d ", af.ruid, af.uid );
        if ( i < 27 ) sprintf_l ( bp, &ofs, "%*.*s", 27-i, 27-i, " " );
    }
    else if ( af.ruid != (unsigned)-1 ) sprintf_l ( bp, &ofs, "ruid:        %-13d ", af.ruid );
    else if ( af.uid != (unsigned)-1 ) sprintf_l ( bp, &ofs, "uid:         %-13d ", af.uid );

    /* output username */
    ptr = output_username ( &af, bp, &ofs, a_ptr, flag );
    if ( ptr || af.auid != (unsigned)AUID_INVAL || af.ruid != (unsigned)-1 
    || af.uid != (unsigned)-1 )
        sprintf_l ( bp, &ofs, "\n" );

    if ( af.pid != -1 ) sprintf_l ( bp, &ofs, "pid:         %-13d ", af.pid );
    if ( af.ppid != -1 ) sprintf_l ( bp, &ofs, "ppid: %-20d ", af.ppid );
    if ( af.device != -1 ) sprintf_l ( bp, &ofs, "cttydev: (%d,%d)", major(af.device), minor(af.device) );
    if ( af.pid != -1 || af.ppid != -1 || af.device != -1 ) sprintf_l ( bp, &ofs, "\n" );


    /* output procname */
    if ( af.event == SYS_execv || af.event == SYS_execve
#ifdef __osf__
    || af.event == SYS_exec_with_loader
#endif /* __osf__ */
    || af.event == SYS_exit ) {
        if ( procnm_buf[0] )
            sprintf_l ( bp, &ofs, "procname:    %s\n", procnm_buf );
    }
    else if ( a_ptr != (struct a_proc *)-1 && a_ptr->procname )
        sprintf_l ( bp, &ofs, "procname:    %s\n", a_ptr->procname );


    /* output event */
    get_eventnm ( &af, af.event, af.subevent, &eventp, &subeventp );
    if ( af.event == SYS_exit && af.login_proc )
        sprintf_l ( bp, &ofs, "event:       exit (%s)", "logout" );
    else if ( strncmp ( eventp, "UNKNOWN EVENT", 13 ) == 0 )
        sprintf_l ( bp, &ofs, "event:       UNKNOWN EVENT (%d)", af.event );
    else sprintf_l ( bp, &ofs, "event:       %s", eventp );
    if ( af.habitat_indx ) 
        sprintf_l ( bp, &ofs, " (%.*s)", af.habitat_len[0], af.habitat[0] );
    sprintf_l ( bp, &ofs, "\n" );
    if ( *subeventp ) sprintf_l ( bp, &ofs, "subevent:    %s\n", subeventp );
    for ( i = 0; af.event2_indx == 0 && i < af.subevent2_indx; i++ ) {
        get_eventnm ( &af, af.event, af.subevent2[i], &eventp, &subeventp );
        if ( *subeventp ) sprintf_l ( bp, &ofs, "subevent:    %s\n", subeventp );
    }


    /* output login specific information */
    output_login_info ( &af, bp, &ofs, 0 );


    /* output secondary/remote identification data for trusted events */
    if ( af.event >= MIN_TRUSTED_EVENT && af.event <= MAX_TRUSTED_EVENT )
        output_secondary_id ( &af, bp, &ofs );


    /* output string parameters */
    if ( af.charp_indx ) output_charparam_fmt ( &af, bp, &ofs, flag );


    /* output event-specific parameters */
    output_param_fmt ( &af, bp, &ofs, flag );


    /* output vnode and descriptor information */
    output_vnode_info ( &af, bp, &ofs );
    for ( i = 0; i < af.descrip_indx; i++ ) {
        j = af.descrip[i];
        if ( (a_ptr != (struct a_proc *)-1) && (j >= 0) && (j < NUM_FDS) ) {
            if ( a_ptr->fd_nm[j/NOFILE] && a_ptr->fd_nm[j/NOFILE][j%NOFILE] )
                sprintf_l ( bp, &ofs, "descriptor:  %s (%d)\n", a_ptr->fd_nm[j/NOFILE][j%NOFILE], j );
            else if ( (af.event == SYS_close || af.event == SYS_dup2) && *close_buf )
                sprintf_l ( bp, &ofs, "descriptor:  %s (%d)\n", close_buf, j );
            else sprintf_l ( bp, &ofs, "descriptor:  %d\n", j );
        }
        else sprintf_l ( bp, &ofs, "descriptor:  %d\n", j );
    }


    /* output secondary uid/pid arguments */
    if ( af.event < MIN_TRUSTED_EVENT || af.event > MAX_TRUSTED_EVENT ) {
        for ( i = 0; i < af.auid2_indx; i++ )
            sprintf_l ( bp, &ofs, "audit_id:    %ld\n", af.auid2[i] );
        for ( i = 0; i < af.ruid2_indx; i++ )
            sprintf_l ( bp, &ofs, "ruid:        %d\n", af.ruid2[i] );
        for ( i = 0; i < af.uid2_indx; i++ ) {
            sprintf_l ( bp, &ofs, "uid:         %d", af.uid2[i] );
            if ( (flag & FLAG_LOCALID) && (pw_ptr = getpwuid(af.uid2[i])) )
                sprintf_l ( bp, &ofs, " (%s)\n", pw_ptr->pw_name );
            else sprintf_l ( bp, &ofs, "\n" );
        }
        for ( i = 0; i < af.pid2_indx; i++ )
            sprintf_l ( bp, &ofs, "pid:         %d\n", af.pid2[i] );
        for ( i = 0; i < af.ppid2_indx; i++ )
            sprintf_l ( bp, &ofs, "ppid:        %d\n", af.ppid2[i] );
        for ( i = 0; i < af.device2_indx; i++ )
            sprintf_l ( bp, &ofs, "dev:         (%d,%d)\n", major(af.device2[i]), minor(af.device2[i]) );
    }


    /* output gid parameters */
    for ( i = 0; i < af.gid_indx; i++ ) {
        sprintf_l ( bp, &ofs, "gid:         %d", af.gid[i] );
        if ( (flag & FLAG_LOCALID) && (gr_ptr = getgrgid(af.gid[i])) )
            sprintf_l ( bp, &ofs, " (%s)\n", gr_ptr->gr_name );
        else sprintf_l ( bp, &ofs, "\n" );
    }


    /* output mode parameters */
    for ( i = 0; i < af.mode_indx; i++ ) {
        if ( af.event >= 0 && af.event < NUM_SYSCALLS )
            sprintf_l ( bp, &ofs, "req mode:    %04o\n", af.mode[i] );
        else sprintf_l ( bp, &ofs, "prot mode:   %04o\n", af.mode[i] );
    }


    /* output current directory and root */
    if ( a_ptr != (struct a_proc *)-1 )
        if ( af.descrip_indx || af.charp_indx ) {
            if ( a_ptr->cwd )
                sprintf_l ( bp, &ofs, "directory:   %s\n", a_ptr->cwd );
            if ( a_ptr->root && bcmp ( a_ptr->root, "/", 2 ) )
                sprintf_l ( bp, &ofs, "root dir:    %s\n", a_ptr->root );
        }


    /* output integer array parameters; special case SYS_setgroups */
    if ( (af.event == SYS_setgroups) && (flag & FLAG_LOCALID) )
        output_grp_fmt ( &af, bp, &ofs );
    else for ( i = 0; i < af.int_array_indx; i++ ) {
        sprintf_l ( bp, &ofs, "int array:   " );
        for ( j = 0; j < af.int_array_len[i]; j += sizeof(int) ) {
            bcopy ( af.int_array[i]+j, &k, sizeof(int) );
            sprintf_l ( bp, &ofs, "%d ", k );
        }
        sprintf_l ( bp, &ofs, "\n" );
    }


    /* output ipc information */
    output_ipc_fmt ( &af, bp, &ofs, a_ptr );


    /* output addrvec information */
    for ( i = 0; i < af.addrvec_indx; i++ )
        if ( af.addrvec_len[i] )
            output_addrvec ( af.addrvec[i], af.addrvec_len[i], (char *)0, bp, &ofs );


    /* output X information */
    output_x_fmt ( &af, bp, &ofs );


    /* output note of uid change */
    if ( af.set_uids_indx )
        sprintf_l ( bp, &ofs, "note:        user id value changed during this event\n" );


    /* output result */
#ifdef __osf__
    if ( af.error && af.event < NUM_SYSCALLS ) {
#endif /* __osf__ */
#ifdef ultrix
    if ( af.error && (af.event < NUM_SYSCALLS || (af.event >= SYS_SHMGET && af.event < SYS_SHMAT)) ) {
#endif /* ultrix */
        if ( (af.error < 0) || (af.error >= sys_nerr) ) af.flag = AF_BADERRVAL;
        else sprintf_l ( bp, &ofs, "error:       %s (%d)\n", sys_errlist[af.error], af.error );
    }
    else if ( af.error ) sprintf_l ( bp, &ofs, "error:       %d\n", af.error );
#ifdef __osf__
    else if ( af.event == SYS_exit && WIFSIGNALED(af.result.val[0]) )
        sprintf_l ( bp, &ofs, "result:      %ld            [termination due to signal #%d]\n",
        af.result.val[0]>>8, WTERMSIG(af.result.val[0]) );
    else if ( af.event == SYS_exit )
        sprintf_l ( bp, &ofs, "result:      %ld\n", af.result.val[0]>>8 );
#endif /* __osf__ */
    else {
        if ( af.result.val[0] == 0 && af.result.val[1] == 0 )
            sprintf_l ( bp, &ofs, "result:      0\n" );
        else if ( af.result.val[1] == 0 )
            sprintf_l ( bp, &ofs, "result:      %d (%s)\n",
                af.result.val[0], output_64(&af.result,af.version) );
        else if ( af.result.val[0] != (unsigned)-1 && af.result.val[1] != (unsigned)-1 )
            sprintf_l ( bp, &ofs, "result:      %s\n", output_64(&af.result,af.version) );
    }


    /* output ipaddr */
    if ( af.hostid != -1 ) sprintf_l ( bp, &ofs, "hostid:      %-8x", af.hostid );
    if ( af.ipaddr ) {
        sprintf_l ( bp, &ofs, "ip address:  %s", inet_ntoa(af.ipaddr) );
        if ( ptr = gethost_l(af.ipaddr) ) sprintf_l ( bp, &ofs, " (%s)", ptr );
    }
    if ( af.hostid != -1 || af.ipaddr ) sprintf_l ( bp, &ofs, "\n" );


    /* output timestamp */
    if ( af.timeval.tv_sec ) {
#ifdef __osf__
        tma_p = localtime ( &af.timeval.tv_sec );
        i = daylight && tma_p->tm_isdst;
        sprintf_l ( bp, &ofs, "timestamp:   %.19s.%02d%.5s %s\n",
        ctime(&af.timeval), (int)(af.timeval.tv_usec/10000.0), ctime(&af.timeval)+19,
        tzname[i] );
#endif /* __osf__ */
#ifdef ultrix
        sprintf_l ( bp, &ofs, "timestamp:   %.19s.%02d%.5s %s\n",
        ctime(&af.timeval), (int)(af.timeval.tv_usec/10000.0), ctime(&af.timeval)+19,
        timezone(af.timezone.tz_minuteswest, af.timezone.tz_dsttime) );
#endif /* ultrix */
    }

#ifdef __DEBUG1
    sprintf_l ( bp, &ofs, "cpu # = 0x%x  version # = 0x%x\n", af.n_cpu, af.version );
#endif /* __DEBUG1 */
    sprintf_l ( bp, &ofs, "\n\n" );

    return ( ofs );
}


/* output secondary/remote identification data */
output_secondary_id ( af, bp, ofs )
struct audit_fields *af;
char *bp;
u_int *ofs;
{
    char *eventp, *subeventp;
    char *ptr;
    char buf[MAXPATHLEN];
    int i, j;

    /* output secondary/remote identification data */
    if ( af->auid2_indx || af->ruid2_indx || af->uid2_indx || af->ipaddr2_indx 
#ifdef __osf__
    || af->login_indx > 1 || af->homedir_indx > 1 || af->shell_indx > 1
    || af->service_indx > 1 || af->devname_indx > 1 || af->hostname_indx
#endif /* __osf__ */
#ifdef ultrix
    || af->login2_indx || af->hostid2_indx
#endif /* ultrix */
    || af->pid2_indx || af->ppid2_indx || af->device2_indx || af->event2_indx ) {
        sprintf_l ( bp, ofs, "........... \nremote/secondary identification data --\n" );

#ifdef __osf__
        output_login_info ( af, bp, ofs, 1 );
#endif /* __osf__ */
#ifdef ultrix
        for ( i = 0; i < af->login2_indx; i++ ) {
            j = af->login2_len[i] < MAXPATHLEN-1 ? af->login2_len[i] : MAXPATHLEN-1;
            bcopy ( af->login2[i], buf, j );
            buf[j] = '\0';
            sprintf_l ( bp, ofs, "login name:  %s\n", buf );
        }
        for ( i = 0; i < af->hostid2_indx; i++ )
            sprintf_l ( bp, ofs, "hostid: %-8x\n", af->hostid2[i] );
#endif /* ultrix */

        for ( i = 0; i < af->auid2_indx; i++ )
            sprintf_l ( bp, ofs, "audit_id: %-10ld\n", af->auid2[i] );
        for ( i = 0; i < af->ruid2_indx; i++ )
            sprintf_l ( bp, ofs, "ruid: %-14d\n", af->ruid2[i] );
        for ( i = 0; i < af->uid2_indx; i++ )
            sprintf_l ( bp, ofs, "uid: %-14d\n", af->uid2[i] );
        for ( i = 0; i < af->ipaddr2_indx; i++ ) {
            sprintf_l ( bp, ofs, "ip address:  %s", inet_ntoa(af->ipaddr2[i]) );
            if ( ptr = gethost_l(af->ipaddr2[i]) ) sprintf_l ( bp, ofs, " (%s)", ptr );
            sprintf_l ( bp, ofs, "\n" );
        }

        for ( i = 0; i < af->pid2_indx; i++ )
            sprintf_l ( bp, ofs, "pid: %-10d\n", af->pid2[i] );
        for ( i = 0; i < af->ppid2_indx; i++ )
            sprintf_l ( bp, ofs, "ppid: %-10d\n", af->ppid2[i] );
        for ( i = 0; i < af->device2_indx; i++ )
            sprintf_l ( bp, ofs, "dev: (%d,%d)\n", major(af->device2[i]), minor(af->device2[i]) );

        for ( i = 0; i < af->event2_indx; i++ ) {
            get_eventnm ( &af, af->event2[i], af->subevent2[i], &eventp, &subeventp );
            if ( af->event2[i] == SYS_exit && af->login_proc )
                sprintf_l ( bp, ofs, "event:       exit (%s)\n", "logout" );
            else if ( strncmp ( eventp, "UNKNOWN EVENT", 13 ) == 0 )
                sprintf_l ( bp, ofs, "event: UNKNOWN EVENT (%d)", af->event2[i] );
            else sprintf_l ( bp, ofs, "event:       %s\n", eventp );
        }
        sprintf_l ( bp, ofs, "...........\n" );
    }
}


/* output username */
char *output_username ( af, bp, ofs, a_ptr, flag )
struct audit_fields *af;
char *bp;
u_int *ofs;
struct a_proc *a_ptr;
int flag;
{
#define NAME_LEN sizeof(utmp.ut_name)
    struct a_proc *p_ptr;
    struct passwd *pw_ptr;
    struct utmp utmp;
    static char prev_name[NAME_LEN];
    static uid_t prev_ruid = -1;
    static char name[NAME_LEN];
    int i = -1;


    /* output username */
    if ( a_ptr != (struct a_proc *)-1 ) {
        if ( a_ptr->username ) {
            if ( flag & FLAG_BRIEF ) strncpy ( name, a_ptr->username, NAME_LEN );
            else sprintf_l ( bp, ofs, "username: %s", a_ptr->username );
            i = 0;
        }
    }
    else {  /* used to catch username for exit()'s */
        p_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af->ppid, af->ipaddr, 0 );
        if ( p_ptr != (struct a_proc *)-1 )
            if ( p_ptr->username && af->auid == p_ptr->auid ) {
                if ( flag & FLAG_BRIEF ) strncpy ( name, p_ptr->username, NAME_LEN );
                else sprintf_l ( bp, ofs, "username: %s", p_ptr->username );
                i = 0;
            }
    }


    /* no username found; try passwd file; cache last used ruid/name */
    if ( af->ruid != (unsigned)-1 && flag & FLAG_LOCALID && i == -1 ) {
        if ( af->ruid == prev_ruid ) {
            if ( flag & FLAG_BRIEF ) strncpy ( name, prev_name, NAME_LEN );
            else sprintf_l ( bp, ofs, "(username: %.*s)", NAME_LEN, prev_name );
            i = 0;
        }
        else if (pw_ptr = getpwuid ( af->ruid )) {
            if ( flag & FLAG_BRIEF ) strncpy ( name, pw_ptr->pw_name, NAME_LEN );
            else sprintf_l ( bp, ofs, "(username: %.*s)", NAME_LEN, pw_ptr->pw_name );
            strncpy ( prev_name, pw_ptr->pw_name, NAME_LEN );
            prev_ruid = af->ruid;
            i = 0;
        }
    }


    /* return cached name, if found */
    if ( i == 0 ) return(name);
    else return((char *)0);
}


/* output vnode_id, vnode_dev, vnode_mode information */
output_vnode_info ( af, bp, ofs )
struct audit_fields *af;
char *bp;
u_int *ofs;
{
#ifdef __osf__
    static char *label = "vnode";
#endif /* __osf__ */
#ifdef ultrix
    static char *label = "gnode";
#endif /* ultrix */
    int i;

    for ( i = 0; (i < af->vp_id_indx || i < af->vp_dev_indx || i < af->vp_mode_indx)
    && af->error != ENOENT && af->error != ENXIO; i++ ) {

        /* check for duplicate info */
        if ( i && i < af->vp_id_indx && i < af->vp_dev_indx )
            if ( af->vnode_id[i] == af->vnode_id[i-1] && af->vnode_dev[i] == af->vnode_dev[i-1] )
                continue;

        /* vnode id and dev info */
        if ( i < af->vp_id_indx && af->vnode_id[i] )
            sprintf_l ( bp, ofs, "%s id:    %d  \t", label, af->vnode_id[i] );
        if ( i < af->vp_dev_indx && af->vnode_id[i] )
            sprintf_l ( bp, ofs, "%s dev:   (%d,%d)  \t",
            label, major(af->vnode_dev[i]), minor(af->vnode_dev[i]) );

        /* vnode mode */
        if ( i < af->vp_mode_indx ) {
            if ( af->vnode_id[i] == 0 )
                sprintf_l ( bp, ofs, "%s type:  ", label );
            output_vnode_mode ( af->vnode_mode[i], bp, ofs );
        }
        else sprintf_l ( bp, ofs, "\n" );

    }
}


/* output vnode mode information */
output_vnode_mode ( mode, bp, ofs )
mode_t mode;
char *bp;
u_int *ofs;
{
    int i;

    switch ( mode & S_IFMT ) {

    case S_IFREG:  sprintf_l ( bp, ofs, "[regular file]\n" );
        break;
    case S_IFDIR:  sprintf_l ( bp, ofs, "[directory]\n" );
        break;
    case S_IFBLK:  sprintf_l ( bp, ofs, "[block device]\n" );
        break;
    case S_IFCHR:  sprintf_l ( bp, ofs, "[character device]\n" );
        break;
    case S_IFIFO:  sprintf_l ( bp, ofs, "[fifo]\n" );
        return;    /* no object mode */
    case S_IFLNK:  sprintf_l ( bp, ofs, "[link]\n" );
        break;
    case S_IFSOCK: sprintf_l ( bp, ofs, "[socket]\n" );
        break;
    default: sprintf_l ( bp, ofs, "[unknown device type]\n" );

    }

    sprintf_l ( bp, ofs, "object mode: 0%o\t", mode&07777 );
    if ( mode & 06000 ) {
        if ( mode & 04000 ) sprintf_l ( bp, ofs, "setuid " );
        if ( mode & 02000 ) sprintf_l ( bp, ofs, "setgid " );
        sprintf_l ( bp, ofs, "executable" );
    }

    sprintf_l ( bp, ofs, "\n" );
}


/* format X information output into bp; update ofs to reflect # chars output */
output_x_fmt ( af, bp, ofs )
struct audit_fields *af;
char *bp;
u_int *ofs;
{
    int i;

    for ( i = 0; i < af->atom_id_indx; i++ ) sprintf_l ( bp, ofs, "atom_id:     %d\n", af->atom_id[i] );
    for ( i = 0; i < af->client_id_indx; i++ ) sprintf_l ( bp, ofs, "client_id:   0x%x\n", af->client_id[i] );
    for ( i = 0; i < af->property_indx; i++ ) sprintf_l ( bp, ofs, "property:    %d\n", af->property[i] );

    for ( i = 0; i < af->res_id_indx; i++ ) sprintf_l ( bp, ofs, "res_id:   0x%x\n", af->res_id[i] );
    for ( i = 0; i < af->res_class_indx; i++ ) sprintf_l ( bp, ofs, "res_class:   %d\n", af->res_class[i] );
    for ( i = 0; i < af->res_type_indx; i++ ) {
        if ( af->res_class[i] == 0 && af->res_type[i] == 8 )
            sprintf_l ( bp, ofs, "res_type:   pixmap\n" );
        else if ( af->res_class[i] == 0 && af->res_type[i] == 16 )
            sprintf_l ( bp, ofs, "res_type:   window\n" );
        else sprintf_l ( bp, ofs, "res_type:   %d\n", af->res_type[i] );
    }


#ifdef ultrix
    /* output X aud_client_info structure */
    for ( i = 0; i < af->x_client_indx; i++ ) {
        sprintf_l ( bp, ofs, "client_host: 0x%x\n", af->x_client[i].hostid );
        sprintf_l ( bp, ofs, "client_audit_id: %ld   client_ruid: 0x%x   client_uid: %d\n",
        af->x_client[i].auid, af->x_client[i].ruid, af->x_client[i].uid );
        sprintf_l ( bp, ofs, "client_pid: %d   client_ppid: %d\n",
        af->x_client[i].pid, af->x_client[i].ppid );
    }
#endif /* ultrix */
}


/* return string representation of a 64-bit quantity (in hex notation).
   necessary for displaying 64-bit longs on a 32-bit machine.
   NOTE: as ptr to static space is returned, subsequent calls will
       overwrite previous data.  Do not use this routine multiple times
       in a sprintf_l arg list, for example.
*/
u_char *output_64 ( arg, version )
long64 *arg;
int version;
{
    static u_char buf[19];     /* 64 bits   (independent of 32 or 64-bit arch */

    bzero ( buf, sizeof(buf) );

    if ( (version & AUD_VERS_LONG) && arg->val[0] == (unsigned)-1 && arg->val[1] == (unsigned)-1 )
        sprintf ( buf, "-1" );

    else if ( (version & AUD_VERS_LONG) == 0 && arg->val[0] == (unsigned)-1 )
        sprintf ( buf, "-1" );

    else if ( arg->val[1] == 0 )
        sprintf ( buf, "0x%x", arg->val[0] );

    else sprintf ( buf, "0x%x%08x", arg->val[1], arg->val[0] );

    return ( buf );
}


/* parse audit record */
parse_rec ( rec_ptr, rec_len, af, flag )
char *rec_ptr;              /* ptr to audit record      */
int rec_len;                /* length of audit record   */
struct audit_fields *af;    /* audit fields struct      */
int flag;                   /* misc options             */
{
    char token;
    int *intp;
    int ch;                 /* is an int to compare against constants       */
    int badcnt = 6;         /* limit warnings against bad values per record */
    int i, j;
    static u_long cnt = 0;

#ifdef __DEBUG1

/* fixed length scalar value */
#define PARSE_DEF1(tokentype,field) \
    if ( flag&FLAG_PARSE_DBG ) { \
        sprintf_l ( af->dbg_buf, &af->dbg_cnt, "%s (%o):	", tokentype, ch ); \
        for ( j = 0; j < sizeof(field); j++ ) \
            sprintf_l ( af->dbg_buf, &af->dbg_cnt, "%03o ", 0xff&rec_ptr[i+sizeof token+j] ); \
        sprintf_l ( af->dbg_buf, &af->dbg_cnt, "\n" ); \
    } \
    bcopy ( &rec_ptr[i+sizeof token], &field, sizeof(field) ); \
    i += (sizeof token + sizeof(field)); \
    break;

/* fixed length field in array */
#define PARSE_DEF2(tokentype,field,indx) \
    if ( flag&FLAG_PARSE_DBG ) { \
        sprintf_l ( af->dbg_buf, &af->dbg_cnt, "%s (%o):	", tokentype, ch ); \
        for ( j = 0; j < sizeof(field[0]); j++ ) \
            sprintf_l ( af->dbg_buf, &af->dbg_cnt, "%03o ", 0xff&rec_ptr[i+sizeof token+j] ); \
        sprintf_l ( af->dbg_buf, &af->dbg_cnt, "\n" ); \
    } \
    if ( indx < AUD_NPARAM ) \
        bcopy ( &rec_ptr[i+sizeof token], &field[indx++], sizeof(field[0]) ); \
    i += (sizeof token + sizeof(field[0])); \
    break;

/* array of strings */
#define PARSE_DEF3(tokentype,len,field,indx) \
    bcopy ( &rec_ptr[i+sizeof token], &j, sizeof(int) ); \
    if ( j >= rec_len ) j = 0; \
    if ( indx < AUD_NPARAM ) { \
        len[indx] = j; \
        field[indx++] = (char *)&rec_ptr[i+(sizeof token)+(sizeof *intp)]; \
        if ( flag&FLAG_PARSE_DBG ) \
            sprintf_l ( af->dbg_buf, &af->dbg_cnt, "%s (%o):	%.*s\n", tokentype, ch, len[indx-1], field[indx-1] ); \
    } \
    i += (sizeof token + sizeof *intp + j); \
    break;

/* fixed length scalar value whose size is h/w dependent (32 or 64-bit) */
#define PARSE_DEF4(tokentype,field) \
    if ( flag&FLAG_PARSE_DBG ) { \
        sprintf_l ( af->dbg_buf, &af->dbg_cnt, "%s (%o):	", tokentype, ch ); \
        for ( j = 0; j < (af->version & AUD_VERS_LONG ? sizeof(int)*2 : sizeof(int)); j++ ) \
            sprintf_l ( af->dbg_buf, &af->dbg_cnt, "%03o ", 0xff&rec_ptr[i+sizeof token+j] ); \
        sprintf_l ( af->dbg_buf, &af->dbg_cnt, "\n" ); \
    } \
    j = af->version & AUD_VERS_LONG ? sizeof(int)*2 : sizeof(int); \
    bzero ( field.val, sizeof(field.val) ); \
    bcopy ( &rec_ptr[i+sizeof token], field.val, j ); \
    i += (sizeof token + j); \
    break;

/* fixed length field in array whose size is h/w dependent (32 or 64-bit) */
#define PARSE_DEF5(tokentype,field,indx) \
    if ( flag&FLAG_PARSE_DBG ) { \
        sprintf_l ( af->dbg_buf, &af->dbg_cnt, "%s (%o):	", tokentype, ch ); \
        for ( j = 0; j < (af->version & AUD_VERS_LONG ? sizeof(int)*2 : sizeof(int)); j++ ) \
            sprintf_l ( af->dbg_buf, &af->dbg_cnt, "%03o ", 0xff&rec_ptr[i+sizeof token+j] ); \
        sprintf_l ( af->dbg_buf, &af->dbg_cnt, "\n" ); \
    } \
    bzero ( field[indx].val, sizeof(field[indx].val) ); \
    j = af->version & AUD_VERS_LONG ? sizeof(int)*2 : sizeof(int); \
    if ( indx < AUD_NPARAM ) \
        bcopy ( &rec_ptr[i+sizeof token], field[indx++].val, j ); \
    i += (sizeof token + j); \
    break;

#else /* __DEBUG1 */

/* fixed length scalar value */
#define PARSE_DEF1(tokentype,field) \
    bcopy ( &rec_ptr[i+sizeof token], &field, sizeof(field) ); \
    i += (sizeof token + sizeof(field)); \
    break;

/* fixed length field in array */
#define PARSE_DEF2(tokentype,field,indx) \
    if ( indx < AUD_NPARAM ) \
        bcopy ( &rec_ptr[i+sizeof token], &field[indx++], sizeof(field[0]) ); \
    i += (sizeof token + sizeof(field[0])); \
    break;

/* array of strings */
#define PARSE_DEF3(tokentype,len,field,indx) \
    bcopy ( &rec_ptr[i+sizeof token], &j, sizeof(int) ); \
    if ( j >= rec_len ) j = 0; \
    if ( indx < AUD_NPARAM ) { \
        len[indx] = j; \
        field[indx++] = (char *)&rec_ptr[i+(sizeof token)+(sizeof *intp)]; \
    } \
    i += (sizeof token + sizeof *intp + j); \
    break;

/* fixed length scalar value whose size is h/w dependent (32 or 64-bit) */
#define PARSE_DEF4(tokentype,field) \
    bzero ( field.val, sizeof(field.val) ); \
    j = af->version & AUD_VERS_LONG ? sizeof(int)*2 : sizeof(int); \
    bcopy ( &rec_ptr[i+sizeof token], field.val, j ); \
    i += (sizeof token + j); \
    break;

/* fixed length field in array whose size is h/w dependent (32 or 64-bit) */
#define PARSE_DEF5(tokentype,field,indx) \
    bzero ( field[indx].val, sizeof(field[indx].val) ); \
    j = af->version & AUD_VERS_LONG ? sizeof(int)*2 : sizeof(int); \
    if ( indx < AUD_NPARAM ) \
        bcopy ( &rec_ptr[i+sizeof token], field[indx++].val, j ); \
    i += (sizeof token + j); \
    break;

#endif /* __DEBUG1 */


    /* parse audit record (int+token bytes read elsewhere for length)
       note that ULTRIX kernel stores pids as shorts (despite pid_t)
    */
#ifdef __DEBUG1
    af->dbg_cnt = 0;
    if ( flag&FLAG_PARSE_DBG ) sprintf_l ( af->dbg_buf, &af->dbg_cnt, "\n%ld)\n", ++cnt );
#endif /* __DEBUG1 */
    for ( i = sizeof rec_len + sizeof token; i < rec_len-(sizeof(int)+sizeof(token)); ) {
        ch = rec_ptr[i] & 0xff;

        switch ( ch ) {

        case T_ERRNO:       PARSE_DEF1 ( "T_ERRNO       ", af->error );
        case T_HOSTID:      PARSE_DEF1 ( "T_HOSTID      ", af->hostid );
        case TP_NCPU:       PARSE_DEF1 ( "TP_NCPU       ", af->n_cpu );

        case T_INT:         PARSE_DEF2 ( "T_INT         ", af->intparam, af->intp_indx );
        case T_DESCRIP:     PARSE_DEF2 ( "T_DESCRIP     ", af->descrip, af->descrip_indx );

        case T_X_ATOM:      PARSE_DEF2 ( "T_X_ATOM      ", af->atom_id, af->atom_id_indx );
        case T_X_CLIENT:    PARSE_DEF2 ( "T_X_CLIENT    ", af->client_id, af->client_id_indx );
        case T_X_PROPERTY:  PARSE_DEF2 ( "T_X_PROPERTY  ", af->property, af->property_indx );
        case T_X_RES_CLASS: PARSE_DEF2 ( "T_X_RES_CLASS ", af->res_class, af->res_class_indx );
        case T_X_RES_TYPE:  PARSE_DEF2 ( "T_X_RES_TYPE  ", af->res_type, af->res_type_indx );
        case T_X_RES_ID:    PARSE_DEF2 ( "T_X_RES_ID    ", af->res_id, af->res_id_indx );

        case TP_INTP:       PARSE_DEF3 ( "TP_INTP       ", af->int_array_len, af->int_array, af->int_array_indx );
        case T_CHARP:       PARSE_DEF3 ( "T_CHARP       ", af->charlen, af->charparam, af->charp_indx );
        case T_SOCK:        PARSE_DEF3 ( "T_SOCK        ", af->socketlen, af->socketaddr, af->socket_indx );
        case T_LOGIN:       PARSE_DEF3 ( "T_LOGIN       ", af->login_len, af->login, af->login_indx );
        case T_HOMEDIR:     PARSE_DEF3 ( "T_HOMEDIR     ", af->homedir_len, af->homedir, af->homedir_indx );
        case T_SHELL:       PARSE_DEF3 ( "T_SHELL       ", af->shell_len, af->shell, af->shell_indx );
        case T_SERVICE:     PARSE_DEF3 ( "T_SERVICE     ", af->service_len, af->service, af->service_indx );
        case T_DEVNAME:     PARSE_DEF3 ( "T_DEVNAME     ", af->devname_len, af->devname, af->devname_indx );

        case T_RESULT:      PARSE_DEF4 ( "T_RESULT      ", af->result );

        case TP_TV_SEC:     PARSE_DEF1 ( "TP_TV_SEC     ", af->timeval.tv_sec );
        case TP_TV_USEC:    PARSE_DEF1 ( "TP_TV_USEC    ", af->timeval.tv_usec );

        case TP_SHORT:      PARSE_DEF2 ( "TP_SHORT      ", af->shortparam, af->shortp_indx );
        case TP_VNODE_DEV:  PARSE_DEF2 ( "TP_VNODE_DEV  ", af->vnode_dev, af->vp_dev_indx );
        case TP_VNODE_ID:   PARSE_DEF2 ( "TP_VNODE_ID   ", af->vnode_id, af->vp_id_indx );
        case TP_IPC_UID:    PARSE_DEF2 ( "TP_IPC_UID    ", af->ipc_uid, af->ipc_uid_indx );
        case TP_IPC_GID:    PARSE_DEF2 ( "TP_IPC_GID    ", af->ipc_gid, af->ipc_gid_indx );
        case TP_IPC_MODE:   PARSE_DEF2 ( "TP_IPC_MODE   ", af->ipc_mode, af->ipc_mode_indx );
        case TP_MSGHDR:     PARSE_DEF3 ( "TP_MSGHDR     ", af->msglen, af->msgaddr, af->msg_indx );
        case TP_ACCRGHT:    PARSE_DEF3 ( "TP_ACCRGHT    ", af->accesslen, af->accessaddr, af->access_indx );

#ifdef __osf__
        case TP_EVENT:      PARSE_DEF1 ( "TP_EVENT      ", af->event );
        case TP_AUID:       PARSE_DEF1 ( "TP_AUID       ", af->auid );
        case TP_UID:        PARSE_DEF1 ( "TP_UID        ", af->uid );
        case TP_RUID:       PARSE_DEF1 ( "TP_RUID       ", af->ruid );
        case TP_HOSTADDR:   PARSE_DEF1 ( "TP_HOSTADDR   ", af->ipaddr );
        case TP_PID:        PARSE_DEF1 ( "TP_PID        ", af->pid );
        case TP_PPID:       PARSE_DEF1 ( "TP_PPID       ", af->ppid );
        case TP_DEV:        PARSE_DEF1 ( "TP_DEV        ", af->device );
        case TP_SUBEVENT:   PARSE_DEF1 ( "TP_SUBEVENT   ", af->subevent );
        case TP_VERSION:    PARSE_DEF1 ( "TP_VERSION    ", af->version );

        case T_EVENT:       PARSE_DEF2 ( "T_EVENT       ", af->event2, af->event2_indx );
        case T_AUID:        PARSE_DEF2 ( "T_AUID        ", af->auid2, af->auid2_indx );
        case T_UID:         PARSE_DEF2 ( "T_UID         ", af->uid2, af->uid2_indx );
        case T_RUID:        PARSE_DEF2 ( "T_RUID        ", af->ruid2, af->ruid2_indx );
        case T_HOSTADDR:    PARSE_DEF2 ( "T_HOSTADDR    ", af->ipaddr2, af->ipaddr2_indx );
        case T_PID:         PARSE_DEF2 ( "T_PID         ", af->pid2, af->pid2_indx );
        case T_PPID:        PARSE_DEF2 ( "T_PPID        ", af->ppid2, af->ppid2_indx );
        case T_DEV:         PARSE_DEF2 ( "T_DEV         ", af->device2, af->device2_indx );
        case T_SUBEVENT:    PARSE_DEF2 ( "T_SUBEVENT    ", af->subevent2, af->subevent2_indx );
        case TP_VNODE_MODE: PARSE_DEF2 ( "TP_VNODE_MODE ", af->vnode_mode, af->vp_mode_indx );
        case T_GID:         PARSE_DEF2 ( "T_GID         ", af->gid, af->gid_indx );
        case T_MODE:        PARSE_DEF2 ( "T_MODE        ", af->mode, af->mode_indx );
        case TP_SET_UIDS:   PARSE_DEF2 ( "TP_SET_UIDS   ", af->set_uids, af->set_uids_indx );

        case TP_EVENTP:     PARSE_DEF3 ( "TP_EVENTP     ", af->eventp_len, af->eventp, af->eventp_indx );
        case TP_HABITAT:    PARSE_DEF3 ( "TP_HABITAT    ", af->habitat_len, af->habitat, af->habitat_indx );
        case TP_ADDRVEC:    PARSE_DEF3 ( "TP_ADDRVEC    ", af->addrvec_len, (char *)af->addrvec, af->addrvec_indx );
        case T_HOSTNAME:    PARSE_DEF3 ( "T_HOSTNAME    ", af->hostname_len, af->hostname, af->hostname_indx );

        case TP_LONG:       PARSE_DEF5 ( "TP_LONG       ", af->longparam, af->lngprm_indx );
#endif /* __osf__ */

#ifdef ultrix
        case T_EVENT:       PARSE_DEF1 ( "T_EVENT       ", af->event );
        case T_AUID:        PARSE_DEF1 ( "T_AUID        ", af->auid );
        case T_UID:         PARSE_DEF1 ( "T_UID         ", af->uid );
        case T_RUID:        PARSE_DEF1 ( "T_RUID        ", af->ruid );
        case T_HOSTADDR:    PARSE_DEF1 ( "T_HOSTADDR    ", af->ipaddr );
        case T_PID:         PARSE_DEF1 ( "T_PID         ", af->pid );
        case T_PPID:        PARSE_DEF1 ( "T_PPID        ", af->ppid );
        case T_DEV:         PARSE_DEF1 ( "T_DEV         ", af->device );
        case T_SUBEVENT:    PARSE_DEF1 ( "T_SUBEVENT    ", af->subevent );
        case TP_TZ_MIN:     PARSE_DEF1 ( "TP_TZ_MIN     ", af->timezone.tz_minuteswest );
        case TP_TZ_DST:     PARSE_DEF1 ( "TP_TZ_DST     ", af->timezone.tz_dsttime );

        case T_EVENT2:      PARSE_DEF2 ( "T_EVENT2      ", af->event2, af->event2_indx );
        case T_AUID2:       PARSE_DEF2 ( "T_AUID2       ", af->auid2, af->auid2_indx );
        case T_UID2:        PARSE_DEF2 ( "T_UID2        ", af->uid2, af->uid2_indx );
        case T_RUID2:       PARSE_DEF2 ( "T_RUID2       ", af->ruid2, af->ruid2_indx );
        case T_HOSTADDR2:   PARSE_DEF2 ( "T_HOSTADDR2   ", af->ipaddr2, af->ipaddr2_indx );
        case T_PID2:        PARSE_DEF2 ( "T_PID2        ", af->pid2, af->pid2_indx );
        case T_PPID2:       PARSE_DEF2 ( "T_PPID2       ", af->ppid2, af->ppid2_indx );
        case T_DEV2:        PARSE_DEF2 ( "T_DEV2        ", af->device2, af->device2_indx );
        case T_SUBEVENT2:   PARSE_DEF2 ( "T_SUBEVENT2   ", af->subevent2, af->subevent2_indx );
        case T_HOSTID2:     PARSE_DEF2 ( "T_HOSTID2     ", af->hostid2, af->hostid2_indx );
        case T_X_CLIENT_INFO:
                            PARSE_DEF2 ( "T_X_CLIENT_INFO", af->x_client, af->x_client_indx );

        case T_LOGIN2:      PARSE_DEF3 ( "T_LOGIN2      ", af->login2_len, af->login2, af->login2_indx );
#endif /* ultrix */


        /* token length field */
        case TP_LENGTH:     i += sizeof(int) + sizeof(token);
                            break;


        /* unknown data catcher */
        default:            if ( --badcnt > 0 )
                                fprintf ( stderr, "\007WARNING -- unknown value 0x%x @ byte offset %d\n", ch, i );
                            if ( badcnt == 0 ) {
                                fprintf ( stderr, "....\n" );
                                return;
                            }
                            i += sizeof token;
                            break;
        }

    }
}


/* recover from records being split across logs; return af.event/0/-1 */
int recover ( op, buf, rec_len, fd_p, logfile, flag )
int op;         /* operation            */
char *buf;      /* audit data           */
int *rec_len;   /* record length        */
int *fd_p;      /* data file descriptor */
char *logfile;  /* auditlog filename    */
int flag;       /* misc options         */
{
    static char bufsav[AUDBUFSIZ_L];    /* used for split records */
    static int sav_p = 0;               /* offset into bufsav     */
    struct audit_fields af;
    char *ptr;
    off_t posn;
    int len;
    int i, j;


    /* store last partial record; update fd_p */
    if ( op == RCVR_STORE ) {
        for ( i = 1; i < *rec_len && i < AUDBUFSIZ_L && (buf[i]&0xff) != TP_LENGTH; i++ );
        for ( sav_p = 0; sav_p < i; sav_p++ ) bufsav[sav_p] = buf[sav_p];
        lseek ( *fd_p, i-*rec_len, L_INCR );

        /* return next record's event field */
        posn = tell ( *fd_p );
        init_audit_fields ( &af );
        ptr = fetch_rec ( fd_p, &len, &af, flag, 0L, logfile );
        lseek ( *fd_p, posn, L_SET );
        if ( ptr == (char *)-1 || af.flag == AF_CORRUPTED ) return(-1);
        parse_rec ( ptr, len, &af, flag );
        return ( af.event );
    }


    /* store partial record from .hdr file (fd_p) into local buffer */
    if ( op == RCVR_HDRSTR ) {
        if ( read ( *fd_p, bufsav, *rec_len ) != *rec_len ) return(-1);
        sav_p = *rec_len;
        return ( *rec_len );
    }


    /* store partial record (and length) into .hdr file (fd_p) */
    if ( op == RCVR_HDRFETCH ) {
        write ( *fd_p, &sav_p, sizeof(sav_p) );
        write ( *fd_p, bufsav, sav_p );
        return(0);
    }


    /* recover last partial record */
    if ( op == RCVR_FETCH ) {
        if ( sav_p == 0 ) return(-1);
        lseek ( *fd_p, -1, L_INCR );
        bcopy ( bufsav, buf, sav_p );

        /* get enough data to get record length */
        for ( i = sav_p; i < 1+sizeof *rec_len; i++ )
            read ( *fd_p, buf[i], 1 );
        bcopy ( &buf[1], rec_len, sizeof *rec_len );

        /* read data */
        if ( (*rec_len < 0) || (*rec_len >= AUDBUFSIZ_L) ) {
            af.flag = AF_BADRECOVER;
            sav_p = 0;
            return(-1);
        };
        i = *rec_len - (sizeof *rec_len + sizeof *ptr);
        j = sav_p - (sizeof *rec_len + sizeof *ptr);
        do {
            j += read ( *fd_p, &buf[sizeof *rec_len + sizeof *ptr + j], i-j );
        } while ( (j < i) && flag&FLAG_FOLLOW );
        sav_p = 0;
        if ( j < i ) return(-1);

        /* consistency check */
        bcopy ( &buf[*rec_len - (sizeof *rec_len)], &i, sizeof(int) );
        if ( i != *rec_len ) {
            af.flag = AF_BADRECOVER;
            return(-1);
        }
        return(0);
    }

    return(0);
}


/* catch sig_int's */
sig_int1()
{
    char buf[2];

    fprintf ( stderr, "\n--interrupt:  exit (y/n)?  " );
    fflush ( stderr );
    read ( 0, buf, 1 );
    for ( buf[1] = '\0'; buf[0] != '\n' && buf[1] != '\n'; )
        read ( 0, &buf[1], 1 );

#ifdef __DEBUG1
    if ( buf[0] == '?' ) {
        fprintf ( stderr, "\n\n--debug mode: blk_maps:\n" );
        aud_mem_op ( AMO_DEBUG, (char *)0, 0 );
        fprintf ( stderr, "\n\n--debug mode: proc structures:\n" );
        aud_mem_proc ( AMP_DEBUG, (struct a_proc *)0, 0, NULL, 0 );
    }
#endif /* __DEBUG1 */

    if ( buf[0] == '1' || buf[0] == 'y' || buf[0] == 'Y' ) {
        if ( flag&FLAG_STAT ) statistic ( (struct audit_fields *)0, 1 );
        compress ( 1, (char *)0 );
        exit(0);
    }

    fprintf ( stderr, "\n\n--interactive mode--\n" );
    interact ( &selectn, &flag );
}


/* local sprintf() routine; modify offset to reflect # chars copied */
/* some stdio sprintf()'s return a ptr; some return # chars copied  */
int sprintf_l ( str, offset, fmt, va_alist )
char *str;
u_int *offset;
char *fmt;
#ifdef mips
va_dcl
#endif /* mips */
#ifdef __alpha
va_dcl
#endif /* __alpha */
{
    int count;
    FILE strbuf;
#ifdef vax
#define ARGS &va_alist
#endif /* vax */
#ifdef mips
#define ARGS ap
    va_list ap;
    va_start(ap);
#endif /* mips */
#ifdef __alpha
#define ARGS ap
    va_list ap;
    va_start(ap);
#endif /* __alpha */

    strbuf._flag = _IOWRT|_IOSTRG;
#ifdef __osf__
    strbuf._base = strbuf._ptr = (unsigned char *)&str[*offset];
    strbuf._cnt = INT_MAX;
#endif /* __osf__ */
#ifdef ultrix
    strbuf._base = strbuf._ptr = &str[*offset];
    strbuf._cnt = 32767;
#endif /* ultrix */

    /* very crude redzone of 4k; note this routine is specific to output_*() */
    if ( *offset > AUDBUFSIZ_L-4096 ) return(0);

    count = _doprnt ( fmt, ARGS, &strbuf );
    *strbuf._ptr = '\0';

    for (; str[(*offset)++]; );
    (*offset)--;
    return(count);
}


/* maintain process state */
state_maint ( af )
struct audit_fields *af;    /* audit record fields  */
{
    union {
        struct sockaddr sockbuf;
        char buf[MAXPATHLEN];
    } sock_un;
    struct sockaddr *sockptr;
    char *sockname;
    struct a_proc *a_ptr;
    struct a_proc *p_ptr;
    int event;
    int i, j;
    u_short family;

    if ( af->error ) return;


    /* deal with habitat events */
    if ( af->eventp_indx ) {

        /* open */
        if ( !(bcmp ( af->eventp[0], "open", af->eventp_len[0] )) )
            event = SYS_open;
    }
    else event = af->event;


    switch ( event ) {

    /* login event */
    case LOGIN:
        if ( af->homedir_indx )
            state_maint_path_change ( af->pid, af->ppid, af->ipaddr, af->homedir[0], SYS_chdir );
        if ( af->login_indx )
            state_maint_add ( af->pid, af->ppid, af->ipaddr, af->login_len[0], af->login[0], -1, -1, SMA_USER );
        a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af->pid, af->ipaddr, 0 );
        if ( a_ptr != (struct a_proc *)-1 ) a_ptr->login_proc = 1;
        break;


    /* free process' resources */
#ifdef __osf__
    case LOGOUT:
#endif /* __osf__ */
    case SYS_exit:
        a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af->pid, af->ipaddr, 0 );
        if ( a_ptr != (struct a_proc *)-1 ) {
            if ( a_ptr->login_proc ) af->login_proc = 1;
            if ( a_ptr->procname ) {
                for ( i = 0; a_ptr->procname[i]; i++ );
                bcopy ( a_ptr->procname, procnm_buf, i+1 );
            }
        }
        state_maint_close ( SMC_ALL, af->pid, af->ppid, af->ipaddr, 0, 0, 0 );
        break;


    /* new descriptor */
#ifdef ultrix
    case SYS_creat:
#endif /* ultrix */
    case SYS_open:
#ifdef __osf__
    case SYS_old_open:
#ifdef FDFS_MAJOR
        /* special case of FDFS open.  really a dup(i), where i = minor# */
        if ( af->vp_dev_indx && major(af->vnode_dev[0]) == FDFS_MAJOR ) {
            state_maint_add ( af->pid, af->ppid, af->ipaddr, 0, (char *)0, 
                af->result.val[0], minor(af->vnode_dev[0]), -1 );
            break;
        }
#endif /* FDFS_MAJOR */
#endif /* __osf__ */
        if ( af->charp_indx )
            state_maint_add ( af->pid, af->ppid, af->ipaddr, af->charlen[0], af->charparam[0], af->result.val[0], -1, -1 );
        if ( af->intp_indx && (af->intparam[0]&O_CREAT) == 0 )
            af->mode_indx = 0;
        break;


    /* manipulate descriptors */
    case SYS_close:
        state_maint_close ( SMC_FD, af->pid, af->ppid, af->ipaddr, af->descrip[0], 0, 0 );
        break;
    case SYS_dup:
    case SYS_fcntl:
        state_maint_add ( af->pid, af->ppid, af->ipaddr, 0, (char *)0, af->result.val[0], af->descrip[0], -1 );
        break;
    case SYS_dup2:
        state_maint_close ( SMC_FD, af->pid, af->ppid, af->ipaddr, af->descrip[1], af->vnode_id[1], af->vnode_dev[1] );
        state_maint_add ( af->pid, af->ppid, af->ipaddr, 0, (char *)0, af->descrip[1], af->descrip[0], -1 );
        break;


    /* update path */
    case SYS_chdir:
    case SYS_chroot:
        if ( af->charp_indx )
            state_maint_path_change ( af->pid, af->ppid, af->ipaddr, af->charparam[0], af->event );
        break;
#ifdef __osf__
    case SYS_fchdir:
        a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af->pid, af->ipaddr, 0 );
        if ( a_ptr != (struct a_proc *)-1 && af->descrip_indx ) {
            j = af->descrip[0];
            if ( j >= 0 && j < NUM_FDS && a_ptr->fd_nm[j/NOFILE] && a_ptr->fd_nm[j/NOFILE][j%NOFILE] )
                state_maint_path_change ( af->pid, af->ppid, af->ipaddr,
                a_ptr->fd_nm[j/NOFILE][j%NOFILE], af->event );
        }
        break;
#endif /* __osf__ */


    /* sockets */
    case SYS_bind:
    case SYS_connect:
        if ( af->socket_indx == 0 ) break;
        a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af->pid, af->ipaddr, 0 );
        if ( a_ptr == (struct a_proc *)-1 ) {
            state_maint_open ( af->pid, af->ppid, af->ipaddr );
            a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af->pid, af->ipaddr, 0 );
        }
        j = af->socketlen[0] < MAXPATHLEN-1 ? af->socketlen[0] : MAXPATHLEN-1;
        bcopy ( af->socketaddr[0], sock_un.buf, j );
        sock_un.buf[j] = '\0';
        sockptr = (struct sockaddr *)&sock_un.sockbuf;

        /* sa_family here is 4.3 style (u_short); if family in
           audit_record appears to be > UCHAR_MAX, it must be
           4.4 style sockaddr of length (byte) then family (byte)
        */
        family = sockptr->sa_family;
        if ( family > UCHAR_MAX ) family >>= NBBY;
        switch ( family ) {
        case AF_UNIX:
            sockname = sockptr->sa_data;
            break;
        case AF_INET:
            sockname = itoa(ntohs(((struct sockaddr_in *)sockptr)->sin_port));
            break;
        default:
            sockname = sockptr->sa_data;
            break;
        }

        for ( j = 0; sockname[j]; j++ );
        state_maint_add ( af->pid, af->ppid, af->ipaddr, j+1, sockname, af->descrip[0], -1, -1 );
        break;

    case SYS_accept:
#ifdef __osf__
#ifdef COMPAT_43
    case SYS_naccept:
#else
    case SYS_old_accept:
#endif /* COMPAT_43 */
#endif /* __osf__ */
        if ( af->descrip_indx )
            state_maint_add ( af->pid, af->ppid, af->ipaddr, 0, (char *)0, af->result.val[0], af->descrip[0], -1 );
        break;


    /* proc name */
    case SYS_execv:
    case SYS_execve:
#ifdef __osf__
    case SYS_exec_with_loader:
#endif /* __osf__ */
        if ( af->charp_indx )
            state_maint_add ( af->pid, af->ppid, af->ipaddr, af->charlen[0]+1, af->charparam[0], -1, -1, SMA_PROC );
        break;


    /* events which cause break in state maintenance */
    case AUDIT_SUSPEND:
    case AUDIT_LOG_CREAT:
    case AUDIT_LOG_OVERWRITE:
    case AUDIT_SHUTDOWN:
    case AUDIT_XMIT_FAIL:
        for ( ; (a_ptr = aud_mem_proc ( AMP_GETNEXT, (struct a_proc *)0, 0, 0, 0 )) != (struct a_proc *)-1; )
            state_maint_close ( SMC_ALL, a_ptr->pid, 0, a_ptr->ipaddr, 0, 0, 0 );
        break;

    /* get current process' state data (proc struct)
       if doesn't yet exist, get ppid's state data
       if ppid has stored a procname or username, then start building state
         data for the current process
    */
    default:
        a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af->pid, af->ipaddr, 0 );
        if ( a_ptr == (struct a_proc *)-1 ) {
            p_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af->ppid, af->ipaddr, 0 );
            if ( p_ptr != (struct a_proc *)-1 && ( p_ptr->procname || p_ptr->username ) ) {
                state_maint_open ( af->pid, af->ppid, af->ipaddr );
                a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, af->pid, af->ipaddr, 0 );
            }
        }
        if ( a_ptr != (struct a_proc *)-1 ) a_ptr->auid = af->auid;
        break;

    }
}


/* add new file descriptor->pathname translation, username, or procname to a_proc */
state_maint_add ( pid, ppid, ipaddr, namlen, name, fd, dup_fd, event )
pid_l pid;            /* pid of process whose state is to be updated      */
pid_l ppid;           /* ppid of process whose state is to be updated     */
ipaddr_l ipaddr;      /* ipaddr of process whose state is to be updated   */
int namlen;           /* length of name to be added to proc state         */
char *name;           /* name to be added to proc state                   */
int fd;               /* descriptor associated with name                  */
int dup_fd;           /* descriptor to be dup'ed                          */
int event;            /* SMA_USER: update username;  SMA_PROC: update procname */
{
    struct a_proc *a_ptr;
    char *f_ptr;
    int i;

    /* fd check */
    if ( (fd < 0 || fd >= NUM_FDS) && event == -1 ) return;

    /* fetch a_proc structure */
    a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, pid, ipaddr, 0 );
    if ( a_ptr == (struct a_proc *)-1 ) {
#ifdef debug2
        fprintf ( stderr, "(state_maint_add: no a_proc for pid %d ipaddr 0x%x; creating one)\n", pid, ipaddr );
#endif /* debug2 */
        state_maint_open ( pid, ppid, ipaddr );
        a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, pid, ipaddr, 0 );
        if ( a_ptr == (struct a_proc *)-1 ) return;
    }


    /* free previous name associated with descriptor */
    if ( fd >= 0 && fd < NUM_FDS && a_ptr->fd_nm[fd/NOFILE] 
    && a_ptr->fd_nm[fd/NOFILE][fd%NOFILE] ) {
        for ( i = 0; a_ptr->fd_nm[fd/NOFILE][fd%NOFILE][i]; i++ );
        aud_mem_op ( AMO_FREE, a_ptr->fd_nm[fd/NOFILE][fd%NOFILE], i+1 );
        a_ptr->fd_nm[fd/NOFILE][fd%NOFILE] = '\0';
    }

    /* fetch space for name; put in a_proc */
    if ( namlen > 0 ) {
        if ( fd >= 0 && fd < NUM_FDS && a_ptr->fd_nm[fd/NOFILE] == (char **)0 ) {
            a_ptr->fd_nm[fd/NOFILE] = (char **)aud_mem_op ( AMO_FETCH, (char *)0, NOFILE*sizeof(char *) );
            if ( a_ptr->fd_nm[fd/NOFILE] == (char **)0 ) {
                fprintf ( stderr, "warning: state_maint_add: no more mem; could not create field entry for pid %d\n", pid );
                return;
            }
            for ( i = 0; i < NOFILE; i++ ) a_ptr->fd_nm[fd/NOFILE][i] = '\0';
        }
        f_ptr = aud_mem_op ( AMO_FETCH, (char *)0, namlen );
        if ( f_ptr == (char *)0 ) {
            fprintf ( stderr, "warning: state_maint_add: no more mem; could not create field entry for pid %d\n", pid );
            return;
        }
        bcopy ( name, f_ptr, namlen );
        f_ptr[namlen-1] = '\0';
        if ( fd >= 0 && fd < NUM_FDS ) a_ptr->fd_nm[fd/NOFILE][fd%NOFILE] = f_ptr;
        else if ( event == SMA_USER ) {
            if ( a_ptr->username ) {
                for ( i = 0; a_ptr->username[i]; i++ );
                aud_mem_op ( AMO_FREE, a_ptr->username, i+1 );
            }
            a_ptr->username = f_ptr;
        }
        else if ( event == SMA_PROC ) {
            if ( a_ptr->procname ) {
                for ( i = 0; a_ptr->procname[i]; i++ );
                bcopy ( a_ptr->procname, procnm_buf, i+1 );
                aud_mem_op ( AMO_FREE, a_ptr->procname, i+1 );
            }
            a_ptr->procname = f_ptr;
        }
    }


    /* dup dup_fd to fd */
    if ( dup_fd >= 0 && dup_fd <= NUM_FDS && a_ptr->fd_nm[dup_fd/NOFILE]
    && a_ptr->fd_nm[dup_fd/NOFILE][dup_fd%NOFILE] ) {
        for ( i = 0; i < MAXPATHLEN && a_ptr->fd_nm[dup_fd/NOFILE][dup_fd%NOFILE][i]; i++ );
        f_ptr = aud_mem_op ( AMO_FETCH, (char *)0, i+1 );
        if ( f_ptr == (char *)0 ) {
            fprintf ( stderr, "warning: state_maint_add: no more mem; could not create field entry for pid %d\n", pid );
            return;
        }
        bcopy ( a_ptr->fd_nm[dup_fd/NOFILE][dup_fd%NOFILE], f_ptr, i+1 );
        if ( fd >= 0 && fd < NUM_FDS && a_ptr->fd_nm[fd/NOFILE] == (char **)0 ) {
            a_ptr->fd_nm[fd/NOFILE] = (char **)aud_mem_op ( AMO_FETCH, (char *)0, NOFILE*sizeof(char *) );
            if ( a_ptr->fd_nm[fd/NOFILE] == (char **)0 ) {
                fprintf ( stderr, "warning: state_maint_add: no more mem; could not create field entry for pid %d\n", pid );
                return;
            }
            for ( i = 0; i < NOFILE; i++ ) a_ptr->fd_nm[fd/NOFILE][i] = '\0';
        }
        a_ptr->fd_nm[fd/NOFILE][fd%NOFILE] = f_ptr;
    }
}


/* free resources associated with process' state */
state_maint_close ( op, pid, ppid, ipaddr, fd, vno_id, vno_dev )
int op;             /* SMC_ALL: release all resources; SMC_FD: free fd only */
pid_l pid;          /* pid of process whose state is to be updated          */
pid_l ppid;         /* ppid of process whose state is to be updated         */
ipaddr_l ipaddr;    /* ipaddr of process whose state is to be updated       */
int fd;             /* descriptor to be closed                              */
ino_t vno_id;       /* vnode id value (for dup2)                            */
dev_t vno_dev;      /* vnode dev value (for dup2)                           */
{
    struct a_proc *ptr;
    int i, j, k;

    /* fetch a_proc structure */
    ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, pid, ipaddr, 0 );
    if ( ptr == (struct a_proc *)-1 ) {
#ifdef debug2
        fprintf ( stderr, "(state_maint_close: no a_proc for pid %d ipaddr 0x%x)\n", pid, ipaddr );
#endif /* debug2 */
        state_maint_open ( pid, ppid, ipaddr );
        ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, pid, ipaddr, 0 );
        if ( ptr == (struct a_proc *)-1 ) return;
    }


    /* release single descriptor */
    if ( op == SMC_FD ) {
        if ( fd >= 0 && fd < NUM_FDS ) {
            if ( ptr->fd_nm[fd/NOFILE] && ptr->fd_nm[fd/NOFILE][fd%NOFILE] ) {
                for ( j = 0; ptr->fd_nm[fd/NOFILE][fd%NOFILE][j]; j++ );
                bcopy ( ptr->fd_nm[fd/NOFILE][fd%NOFILE], close_buf, j );
                close_buf[j] = '\0';
                last_vno_id = vno_id;
                last_vno_dev = vno_dev;
                aud_mem_op ( AMO_FREE, ptr->fd_nm[fd/NOFILE][fd%NOFILE], j+1 );
                ptr->fd_nm[fd/NOFILE][fd%NOFILE] = '\0';
            }
        }
        return;
    }

    /* release all descriptors */
    for ( i = 0; i < NUM_FDP; i++ ) {
        for ( j = 0; ptr->fd_nm[i] && j < NOFILE; j++ ) {
            if ( ptr->fd_nm[i][j] ) {
                for ( k = 0; ptr->fd_nm[i][j][k]; k++ );
                aud_mem_op ( AMO_FREE, ptr->fd_nm[i][j], k+1 );
                ptr->fd_nm[i][j] = '\0';
            }
        }
        if (i && ptr->fd_nm[i]) {
            aud_mem_op ( AMO_FREE, ptr->fd_nm[i], NOFILE*sizeof(char *) );
            ptr->fd_nm[i] = '\0';
        }
    }


    /* release cwd, root, username, and procname */
    if ( ptr->cwd ) {
        for ( j = 0; ptr->cwd[j]; j++ );
        aud_mem_op ( AMO_FREE, ptr->cwd, j+1 );
        ptr->cwd = '\0';
    }
    if ( ptr->root ) {
        for ( j = 0; ptr->root[j]; j++ );
        aud_mem_op ( AMO_FREE, ptr->root, j+1 );
        ptr->root = '\0';
    }
    if ( ptr->username ) {
        for ( j = 0; ptr->username[j]; j++ );
        aud_mem_op ( AMO_FREE, ptr->username, j+1 );
        ptr->username = '\0';
    }
    if ( ptr->procname ) {
        for ( j = 0; ptr->procname[j]; j++ );
        aud_mem_op ( AMO_FREE, ptr->procname, j+1 );
        ptr->procname = '\0';
    }


    /* release a_proc structure */
    aud_mem_proc ( AMP_FREE, ptr, pid, ipaddr, 0 );
}


/* establish new a_proc structure for <pid,ipaddr> */
state_maint_open ( pid, ppid, ipaddr )
pid_l pid;          /* pid of process whose state is to be stored       */
pid_l ppid;         /* ppid of process whose state is to be stored      */
ipaddr_l ipaddr;    /* ipaddr of process whose state is to be stored    */
{
    struct a_proc *a_ptr;
    struct a_proc *p_ptr;
    char *c_ptr;
    int i, j, k;

    /* get a_proc structure for <pid,ipaddr> */
    if ( (a_ptr = aud_mem_proc ( AMP_GETAPROC, (struct a_proc *)0, pid, ipaddr, 0 )) == (struct a_proc *)-1 ) {
        fprintf ( stderr, "warning: state_maint_open: no more mem; could not create a_proc for %d\n", pid );
        return;
    }

    /* fetch a_proc structure for ppid; load fd's from ppid to pid */
    if ( (p_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, ppid, ipaddr, 0 )) == (struct a_proc *)-1 )
        return;
    for ( i = 0; i < NUM_FDP; i++ )
        for ( j = 0; p_ptr->fd_nm[i] && j < NOFILE; j++ )
            if ( p_ptr->fd_nm[i][j] ) {
                if ( a_ptr->fd_nm[i] == (char **)0 ) {
                    a_ptr->fd_nm[i] = (char **)aud_mem_op ( AMO_FETCH, (char *)0, NOFILE*sizeof(char *) );
                    if ( a_ptr->fd_nm[i] == (char **)0 ) {
                        fprintf ( stderr, "warning: state_maint_add: no more mem; \
                            could not create field entry for pid %d\n", pid );
                        i = NUM_FDP;
                        break;
                    }
                    for ( k = 0; k < NOFILE; k++ ) a_ptr->fd_nm[i][k] = '\0';
                }
                for ( k = 0; k < MAXPATHLEN && p_ptr->fd_nm[i][j][k]; k++ );
                if ( (a_ptr->fd_nm[i][j] = aud_mem_op ( AMO_FETCH, (char *)0, k+1 )) == (char *)0 )
                    fprintf ( stderr, "warning: state_maint_open: no more mem; could not add fd field for %d\n", pid );
                else bcopy ( p_ptr->fd_nm[i][j], a_ptr->fd_nm[i][j], k+1 );
            }


    /* load cwd from ppid to pid */
    for ( j = 0; j < MAXPATHLEN && p_ptr->cwd && p_ptr->cwd[j]; j++ );
    if ( j ) {
        if ( (c_ptr = aud_mem_op ( AMO_FETCH, (char *)0, j+1)) == (char *)0 )
            fprintf ( stderr, "warning: state_maint_open: no more mem; could not add cwd field for %d\n", pid );
        else {
            a_ptr->cwd = c_ptr;
            bcopy ( p_ptr->cwd, a_ptr->cwd, j+1 );
        }
    }

    /* load root from ppid to pid */
    for ( j = 0; j < MAXPATHLEN && p_ptr->root && p_ptr->root[j]; j++ );
    if ( j ) {
        if ( (c_ptr = aud_mem_op ( AMO_FETCH, (char *)0, j+1 )) == (char *)0 )
            fprintf ( stderr, "warning: state_maint_open: no more mem; could not add root field for %d\n", pid );
        else {
            a_ptr->root = c_ptr;
            bcopy ( p_ptr->root, a_ptr->root, j+1 );
        }
    }

    /* load username from ppid to pid */
    for ( j = 0; j < MAXPATHLEN && p_ptr->username && p_ptr->username[j]; j++ );
    if ( j ) {
        if ( (c_ptr = aud_mem_op ( AMO_FETCH, (char *)0, j+1 )) == (char *)0 )
            fprintf ( stderr, "warning: state_maint_open: no more mem; could not add username field for %d\n", pid );
        else {
            a_ptr->username = c_ptr;
            bcopy ( p_ptr->username, a_ptr->username, j+1 );
        }
    }

    /* load procname from ppid to pid */
    for ( j = 0; j < MAXPATHLEN && p_ptr->procname && p_ptr->procname[j]; j++ );
    if ( j ) {
        if ( (c_ptr = aud_mem_op ( AMO_FETCH, (char *)0, j+1 )) == (char *)0 )
            fprintf ( stderr, "warning: state_maint_open: no more mem; could not add procname field for %d\n", pid );
        else {
            a_ptr->procname = c_ptr;
            bcopy ( p_ptr->procname, a_ptr->procname, j+1 );
        }
    }
}


/* change a_proc's cwd or root */
state_maint_path_change ( pid, ppid, ipaddr, newpath, event )
pid_l pid;          /* pid of process whose state is to be updated      */
pid_l ppid;         /* ppid of process whose state is to be updated     */
ipaddr_l ipaddr;    /* ipaddr of process whose state is to be updated   */
char *newpath;      /* new pathname for cwd or root                     */
int event;          /* chdir or chroot                                  */
{
    struct a_proc *a_ptr;
    char pathbuf[MAXPATHLEN];
    char **oldpath;
    int  pathlen = 0;
    int i, j;

    /* get field in a_proc structure to be changed */
    a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, pid, ipaddr, 0 );
    if ( a_ptr == (struct a_proc *)-1 ) {
        state_maint_open ( pid, ppid, ipaddr );
        a_ptr = aud_mem_proc ( AMP_GETADDR, (struct a_proc *)0, pid, ipaddr, 0 );
        if ( a_ptr == (struct a_proc *)-1 ) return;
    }
    if ( event == SYS_chdir ) oldpath = &a_ptr->cwd;
#ifdef __osf__
    else if ( event == SYS_fchdir ) oldpath = &a_ptr->cwd;
#endif /* __osf__ */
    else if ( event == SYS_chroot ) oldpath = &a_ptr->root;
    else return;


    /* parse newpath; can't fully parse due to links
       if cd'ing to an abs pathname, precede path with root
    */
    if ( newpath[0] == '/' ) {
        if ( oldpath == &a_ptr->cwd && a_ptr->root ) {
            for ( pathlen = 0; pathlen < MAXPATHLEN && a_ptr->root[pathlen]; pathlen++ );
            bcopy ( a_ptr->root, pathbuf, pathlen+1 );
        }
        else pathlen = 0;
    }
    else if ( *oldpath ) {
        for ( pathlen = 0; pathlen < MAXPATHLEN && (*oldpath)[pathlen]; pathlen++ );
        bcopy ( *oldpath, pathbuf, pathlen+1 );
    }
    for ( i = 0; i < MAXPATHLEN && pathlen < MAXPATHLEN && newpath[i]; i = j ) {

        /* get component of newpath */
        for ( j = i; newpath[j] != '/' && newpath[j] != '\0'; j++ );

        /* ignore "./" */
        if ( (bcmp ( &newpath[i], "./", 2 ) == 0) && (j-i == 1) );

        /* pathbuf <- pathbuf/component */
        else {
            if ( (pathlen == 0) || (pathbuf[pathlen-1] != '/' && pathlen < MAXPATHLEN) ) pathbuf[pathlen++] = '/';
            for ( j = i; pathlen < MAXPATHLEN && newpath[j] != '/' && newpath[j] != '\0'; j++ )
                pathbuf[pathlen++] = newpath[j];
        }

        /* remove trailing /'s */
        for ( ; pathlen > 1 && pathbuf[pathlen-1] == '/'; pathlen-- );
        pathbuf[pathlen] = '\0';
        for ( ; newpath[j] == '/'; j++ );
    }
    if ( pathlen == MAXPATHLEN ) {
        pathlen = 27;
        bcopy ( "MAXIMUM PATHLENGTH EXCEEDED", pathbuf, pathlen+1 );
    }


    /* swap pathnames */
    if ( *oldpath && **oldpath ) {
        for ( i = 0; i < MAXPATHLEN && (*oldpath)[i]; i++ );
        aud_mem_op ( AMO_FREE, *oldpath, i+1 );
    }
    if ( (*oldpath = aud_mem_op ( AMO_FETCH, (char *)0, pathlen+1 )) == (char *)0 ) {
        fprintf ( stderr, "warning: state_maint_path_change: no more mem; could not add pathname field for %d\n", pid );
        return;
    }
    if ( *oldpath ) bcopy ( pathbuf, *oldpath, pathlen+1 );
}


/* statistics routine */
statistic ( af, op )
struct audit_fields *af;
int op;     /* 0: gather; 1: dump */
{
    static int sysevent_stat[NUM_SYSCALLS][2]; /* stats for pass,fail */
    static int trustedevent_stat[N_TRUSTED_EVENTS][2]; /* stats for pass,fail */
#ifdef ultrix
    static int specevent_stat[MAX_SPECIAL+1-MIN_SPECIAL][2]; /* stats for pass,fail */
#endif /* ultrix */
    static int site_event = 0;
    static int added_syscall = 0;
    extern char *syscallnames[];
    extern char *trustedevent[];
    static int n_sevents;
    static int init = 0;
    int i;

    /* get site event info */
    if ( init == 0 ) {
        init++;
#ifdef __osf__
        if ( (n_sevents = audcntl ( GET_NSITEVENTS, (char *)0, 0, 0, 0, 0 )) == -1 )
            n_sevents = 0;
#endif /* __osf__ */
#ifdef ultrix
        if ( (n_sevents = getkval ( X_N_SITEVENTS )) == -1 ) n_sevents = 0;
#endif /* ultrix */
    }


    switch ( op ) {

    case 0: /* gather statistics */
        i = af->error ? 1 : 0;
        if ( af->event >= 0 && af->event < NUM_SYSCALLS )
            sysevent_stat[af->event][i]++;
        else if ( af->event >= MIN_TRUSTED_EVENT && af->event < MIN_TRUSTED_EVENT + N_TRUSTED_EVENTS )
            trustedevent_stat[af->event-MIN_TRUSTED_EVENT][i]++;
#ifdef ultrix
        else if ( af->event >= MIN_SPECIAL && af->event <= MAX_SPECIAL )
            specevent_stat[af->event-MIN_SPECIAL][i]++;
#endif /* ultrix */
        else if ( af->event >= MIN_SITE_EVENT && af->event < MIN_SITE_EVENT+n_sevents )
            site_event++;
        else if ( af->event == -1 && af->eventp_indx )
            added_syscall++;
        break;


    case 1: /* output statistics */
        fprintf ( stderr, "syscalls                        succeed         fail\n" );
        fprintf ( stderr, "--------                        -------         ----\n" );
        for ( i = 0; i < NUM_SYSCALLS; i++ )
            if ( sysevent_stat[i][0] || sysevent_stat[i][1] )
                fprintf ( stderr, "%-25s  %12d %12d\n", syscallnames[i], sysevent_stat[i][0], sysevent_stat[i][1] );
#ifdef ultrix
        for ( i = 0; i <= MAX_SPECIAL-MIN_SPECIAL; i++ )
            if ( specevent_stat[i][0] || specevent_stat[i][1] )
                fprintf ( stderr, "%-25s  %12d %12d\n", special_event[i], specevent_stat[i][0], specevent_stat[i][1] );
#endif /* ultrix */

        fprintf ( stderr, "\ntrusted events                  succeed         fail\n" );
        fprintf ( stderr, "--------------                  -------         ----\n" );
        for ( i = 0; i < N_TRUSTED_EVENTS; i++ )
            if ( trustedevent_stat[i][0] || trustedevent_stat[i][1] )
                fprintf ( stderr, "%-25s  %12d %12d\n", trustedevent[i], trustedevent_stat[i][0], trustedevent_stat[i][1] );
        fprintf ( stderr, "\n\nsite defined events:   %d\n", site_event );
        fprintf ( stderr, "dynamic syscalls:      %d\n\n\n", added_syscall );
        break;

    }
}
