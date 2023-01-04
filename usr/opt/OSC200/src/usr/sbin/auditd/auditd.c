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
static char *rcsid = "@(#)$RCSfile: auditd.c,v $ $Revision: 1.1.4.11 $ (DEC) $Date: 1993/12/20 21:23:52 $";
#endif

/* Audit Daemon
    ULTRIX V4.x:    -Dultrix
    DEC OSF/1:      -D__osf__ -D_BSD

    to mmap /dev/audit:         -D__OSF_MMAP (osf only)
    KERBEROS authentication:    -D__KERBEROS -lkrb -ldes -lknet (ultrix only)

    debug daemon (do not fork): -D__DEBUG1
    debug "child" working net:  -D__DEBUG2
    function trace:             -D__DEBUG3
*/

/*
    This is NOT a complete calling tree, but to give the general idea...

    main()
    {
        client -                        operate as admin interface to auditd

        server -                        the actual audit daemon
          chk_input -                     continually poll for input
            handle_req -                    service admin requests
            chk_access, spawn_child -       handle new network connection
            chk_input_net -                 monitor established network connection
              output_rec -
            output_rec -                    output data
              monitor_space -                 check filesystem limits
              overflow -                      deal with overflow condition
    }
*/


#include <sys/audit.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <sys/reboot.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <nlist.h>
#include <syscall.h>
#include <setjmp.h>
#ifdef __KERBEROS
#include <krb.h>
#include <des.h>
#endif /* __KERBEROS */
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>


#define ADMIN_SOCK      "audS"
#define AUDITD_CLNTS    "/etc/sec/auditd_clients"
#define AUDITD_DIR      "/tmp/.audit"
#define AUDITD_LOGLOC   "/etc/sec/auditd_loc"
#define BUF_SIZ         512
#define CHILD_SOCK      "aud"

#define DEFAULT_ACTION  0
#define DEFAULT_LOG     "/var/adm/auditlog"
#define DEFAULT_LOG_REM "/var/adm/auditlog:remote"
#define DEFAULT_PCT     10
#define DEFAULT_TIME    30
#define DEFTIMEOUT      4

#define MAXPACKET       4096
#define MIN_AUD_BUF     2
#define MSG_OOB_SIZE    1
#define OVERRIDE        1
#define PRIORITY       -5
#define SUSPEND        -2
#define TIME_LEN        32
#ifdef ultrix
#define TOD_MAX_SECONDS 100000000
#endif /* ultrix */
#define WHITE_SPACE(x)  ((x) == ' ' || (x) == '\t' || (x) == '\n')

/* pathval options */
#define LOCAL           0x1
#define REMOTE          0x2

/* INDX is length required to represent MAX_LOG_INDX in ascii (000-999) */
#define INDX            4
#define MAX_LOG_INDX    1000

/* MAXCL_ITOA is length required to represent MAXCLIENTS in ascii (000-999) */
#define MAXCL_ITOA      3
#define MAXCLIENTS      1000

/* LOG_CHNG options */
#define LC_CHANGE       0
#define LC_GET_NEXTDIR  1
#define LC_SETLOC       2


/* audit_attr flag definitions */
#define ATTR_DUMP       0x001
                                  /* dump auditlog buffer         */
#define ATTR_HELP       0x002
                                  /* print help menu              */
#define ATTR_INCR       0x004
                                  /* increment log_indx           */
#define ATTR_KILL       0x008
                                  /* kill auditd process          */
#define ATTR_NETSRV     0x010
                                  /* toggle network server status */
#define ATTR_QUERY      0x020
                                  /* query server for log name    */
#define ATTR_SHOW       0x040
                                  /* show status                  */
#define ATTR_DEBUG      0x080
                                  /* toggle debug mode            */
#define ATTR_SETLOC     0x100
                                  /* read AUDITD_LOGLOC           */

#ifdef __KERBEROS
#define KRB_RECVAUTH    0x1
#define KRB_SENDAUTH    0x2
#define KRB_HDRSIZ      6
#define KRB_PKTSIZ      31
#define KRB_RECVSIZ     5
#define KRB_SENDSIZ     4
#define KRB_TIMEOUT     8
#define TKT_FILE_L      "/var/dss/kerberos/tkt/tkt.auditd"

AUTH_DAT auth_data;               /* KERBEROS authentication data */
MSG_DAT msg_data;                 /* KERBEROS: for rd_safe_msg    */
CREDENTIALS cred;                 /* KERBEROS credentials         */
#endif /* __KERBEROS */


#ifdef ultrix
struct nlist nlst[] = {
    { "_audsize" },
#define X_AUDSIZE      0
    { 0 },
};
#endif /* ultrix */

struct audit_attr {
    char   console[MAXPATHLEN];     /* device name for "console"    */
    int    flag;                    /* see ATTR flag defines        */
    char   flush_arg[TIME_LEN];     /* ascii flush frequency        */
    time_t flush_freq;              /* numeric flush frequency      */
    int    freepct;                 /* min free space w/o warning   */
    int    kerb_auth;               /* use kerberos authentication  */
    int    nkbytes;                 /* size of audit data buffer    */
    int    overflow;                /* action to take on overflow   */
    char   pathname[MAXPATHLEN];    /* audit data pathname          */
    char   pathval;                 /* LOCAL, REMOTE (inet)         */
    int    pid;                     /* auditd identifier            */
    int    timeout;                 /* ping timeout value           */
} attr_g;

struct {
    int indx;                       /* overflow action              */
    char *cmnd;                     /* overflow command             */
    char *cmnd_descr;               /* overflow command description */
} overflow_act[] = {
#define OA_NULL 0
    { OA_NULL, "", "" },
#define OA_CD 1
    { OA_CD, "changeloc", "change to next auditlog location" },
#define OA_SUSP 2
    { OA_SUSP, "suspend", "suspend audit" },
#define OA_OVERWRITE 3
    { OA_OVERWRITE, "overwrite", "overwrite current auditlog" },
#define OA_KILL 4
    { OA_KILL, "kill", "terminate auditing" },
#define OA_HALT 5
    { OA_HALT, "halt", "halt the system" },
};
#define N_OVERFLOW_OPTS ((sizeof overflow_act)/(sizeof *overflow_act))


struct arp {
    char *aud_rec_b;                /* base of audit data           */
    char *aud_rec;                  /* audit data ptr               */
};

struct hndshk {                     /* used to sync daemons         */
    char hostname[MAXHOSTNAMELEN];  /* used for self-check          */
    int  kerb_flag;                 /* 0: kerberos off; 1: on       */
};


char *clientsockname();             /* return client socket name    */
int client_timeout();               /* client timeout handler       */
char *gethost_l();                  /* convert ipaddr to hostname   */
char *inet_ntoa();                  /* inet address to ascii        */
char *itoa();                       /* convert int to alphanumeric  */
int kerb_timeout();                 /* kerberos timeout handler     */
char *log_chng();                   /* handle log directories       */
int ping_noanswer();                /* ping timeout handler         */
time_t scale_tm();                  /* scaled time string -> # secs */
int sig_hndlr();                    /* SIGALRM, SIGTERM handler     */
int sig_hndlr2();                   /* SIGHUP handler               */

extern int errno;
jmp_buf env;                        /* used in kerb_ops             */

int cw_switch = 1;                  /* console write switch         */
int fda = -1;                       /* auditlog file descriptor     */
int log_indx = -1;                  /* audit logname suffix         */
int nobuf = 1;                      /* don't buffer input data      */
int sigtermed = 0;                  /* no compress() on sigterm     */

/*  client::inet_out --> INET --> server::sd3 (--> child::sd3)    */
int inet_out = -1;                  /* inet descriptor for auditd   */
                                    /* to xfer data to net_server   */
pid_t *child;                       /* child auditd pids            */
int n_conn = 0;                     /* max # of potential children  */


/* audit daemon */
main ( argc, argv )
int argc;
char *argv[];
{
    char path_l[MAXPATHLEN];    /* current directory */
    int i, j, k;

    /* initialize attributes */
    init_attr ( &attr_g );

    /* process command line */
    for ( i = 1; i < argc; i++ ) {
        if ( argv[i][0] == '-' )
            for ( j = 1; (i < argc) && argv[i][j]; j++ ) switch ( argv[i][j] ) {

            case 'a':   /* use kerberos authentication */
            case 'A':   attr_g.kerb_auth = 1;
                        break;

            case 'c':   /* set console msg destination */
            case 'C':   if ( ++i == argc ) break;
                        for ( j = 0; j < MAXPATHLEN && (attr_g.console[j] = argv[i][j]); j++ );
                        j--;
                        break;

            case 'd':   /* dump current buffer */
            case 'D':   attr_g.flag |= ATTR_DUMP;
                        if ( audcntl ( FLUSH_AUD_BUF, (char *)0, 0, 0, 0, 0 ) == -1 )
                            if ( errno != ENOSYS ) perror ( "audcntl" );
                        if ( i == argc-1 ) break;
                        if ( isdigit(*argv[i+1]) ) {
                            i++; 
                            for ( j = 0; j < TIME_LEN && (attr_g.flush_arg[j] = argv[i][j]); j++ );
                            j--;
                            attr_g.flush_freq = scale_tm ( attr_g.flush_arg );
                            if ( attr_g.flush_freq == (time_t)-1 )
                                fprintf ( stderr, "auditd: invalid frequency value\n" );
                            if ( attr_g.flush_freq > TOD_MAX_SECONDS ) {
                                attr_g.flush_freq = -1;
                                fprintf ( stderr, "auditd: max dump frequency exceeded\n" );
                            }
                        }
                        break;

            case 'f':   /* change minimum % free space before warning */
            case 'F':   if ( ++i == argc ) break;
                        attr_g.freepct = atoi ( argv[i] );
                        for ( j = 0; argv[i][j+1]; j++ );
                        break;

            case 'h':   /* print help menu */
            case 'H':   attr_g.flag |= ATTR_HELP;
                        break;
                        
            case 'l':   /* change audit data pathname */
            case 'L':   if ( ++i == argc ) break;
                        for ( j = 0; j < MAXPATHLEN-INDX && (attr_g.pathname[j] = argv[i][j]); j++ );
                        if ( attr_g.pathname[--j] == ':' ) {
                            attr_g.pathname[j] = '\0';
                            attr_g.pathval = REMOTE;  /* inet */
                            if ( attr_g.kerb_auth ) attr_g.nkbytes = 4;
                        }
                        else attr_g.pathval = LOCAL;
                        break;

            case 'k':   /* kill auditd proc */
            case 'K':   attr_g.flag |= ATTR_KILL;
                        if ( audcntl ( FLUSH_AUD_BUF, (char *)0, 0, 0, 0, 0 ) == -1 )
                            if ( errno != ENOSYS ) perror ( "audcntl" );
                        break;

            case 'n':   /* size audit data buffer */
            case 'N':   if ( ++i == argc ) break;
                        if ( attr_g.kerb_auth == 0 || attr_g.pathval == LOCAL ) {
                            attr_g.nkbytes = atoi ( argv[i] );
                            nobuf = 0;
                        }
                        for ( j = 0; argv[i][j+1]; j++ );
                        break;

            case 'o':   /* set action to take on overflow; allow partial matches */
            case 'O':   if ( ++i == argc ) break;
                        for ( j = 0; j < N_OVERFLOW_OPTS; j++ ) {
                            for ( k = 0; argv[i][k]; k++ );
                            if ( strncmp ( overflow_act[j].cmnd, argv[i], k ) == 0 ) {
                                attr_g.overflow = overflow_act[j].indx;
                                break;
                            }
                        }
                        for ( j = 0; argv[i][j+1]; j++ );
                        break;

            case 'p':   /* specify proc to handle request */
            case 'P':   if ( ++i == argc ) break;
                        attr_g.pid = atoi ( argv[i] );
                        for ( j = 0; argv[i][j+1]; j++ );
                        break;

            case 'q':   /* query server for pathname */
            case 'Q':   attr_g.flag |= ATTR_QUERY;
                        break;

            case 'r':   /* read AUDITD_LOGLOC for alternate log directories */
            case 'R':   attr_g.flag |= ATTR_SETLOC;
                        break;

            case 's':   /* set network server status */
            case 'S':   attr_g.flag |= ATTR_NETSRV;
                        break;

            case 't':   /* set ping timeout value */
            case 'T':   if ( ++i == argc ) break;
                        attr_g.timeout = atoi ( argv[i] );
                        for ( j = 0; argv[i][j+1]; j++ );
                        break;

            case 'w':   /* show status */
            case 'W':   attr_g.flag |= ATTR_SHOW;
                        break;

            case 'x':   /* increment log_indx */
            case 'X':   attr_g.flag |= ATTR_INCR;
                        break;

            case 'z':   /* remove previous server-client sockets */
            case 'Z':   getwd ( path_l );
                        if ( chdir ( AUDITD_DIR ) == -1 ) break;
                        unlink ( ADMIN_SOCK );
                        /* possible connections range from aud[0..min((getdtablesize-5),MAXCLIENTS-1)] */
                        k = getdtablesize()-5 < MAXCLIENTS-1 ? getdtablesize()-5 : MAXCLIENTS-1;
                        for ( ; k >= 1; k-- )
                            unlink ( clientsockname(k) );
                        chdir ( path_l );
                        break;

#ifdef __DEBUG3
            case '?':   attr_g.flag |= ATTR_DEBUG;
                        break;
#endif /* __DEBUG3 */

            default:    /* unknown option */
                        fprintf ( stderr, "auditd: Unknown option: %c ignored\n", argv[i][j] );
                        break;
            }
    }

    /* act as client */
    if ( client ( &attr_g, (struct sockaddr *)0, 1, 0 ) != 0 ) exit(0);

    /* start server */
    server();
    exit(0);
}


#ifdef __osf__
/* build an auditd audit message */
audgen_l ( event, string1, string2 )
u_int event;            /* non-kernel event value           */
char *string1;          /* first char * parameter           */
char *string2;          /* second char * parameter          */
{
    char *argp[2];
    char tmask[3];
    char buf[AUDITD_RECSZ];
    int siz = sizeof(buf);
    int i;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: audgen_l" );
#endif /* __DEBUG3 */


    /* set up tokenmask and arg pointers */
    tmask[0] = tmask[1] = tmask[2] = '\0';
    if ( *string1 ) {
        argp[0] = string1;
        tmask[0] = T_CHARP;
    }
    if ( *string2 ) {
        argp[1] = string2;
        tmask[1] = T_CHARP;
    }

    /* build the audit record locally */
    if ( (i = audgen ( event, tmask, argp, buf, &siz )) == -1 )
        if ( errno != ENOSYS ) cwrite ( "audgen_l: unable to create auditd record" );

    /* output audit message in auditlog */
    if ( i == 0 && siz ) output_rec ( siz, buf, OVERRIDE );
}
#endif /* __osf__ */


#ifdef ultrix
/* build an auditd audit message */
audgen_l ( event, string1, string2 )
u_int event;            /* non-kernel event value           */
char *string1;          /* first char * parameter           */
char *string2;          /* second char * parameter          */
{
    char a_d[BUF_SIZ];                /* audit data buffer            */
    u_int a_d_len;                    /* pos'n in audit data buffer   */
    long auditd_auid;                 /* daemon's audit_id            */
    u_int auditd_hostaddr;            /* daemon's IP address          */
    uid_t auditd_euid;                /* daemon's euid                */
    short auditd_pid;                 /* daemon's pid                 */
    short auditd_ppid;                /* daemon's ppid                */
    uid_t auditd_ruid;                /* daemon's ruid                */
    char hostname[MAXHOSTNAMELEN];    /* local hostname               */
    struct hostent *hostent;
    struct timeval tp;
    struct timezone tzp;
    char token;                       /* token placeholder            */
    u_int len;                        /* len field for use in macros  */
    int i, j;

/* insert data into audit record; no console msgs with current audit messages */

/* insert fixed length element */
/* location:I_where  type:I_what1  value:I_what2 */
#define INSERT_AUD0(I_where,I_what1,I_what2) \
{\
    if ( (I_where) + sizeof token >= BUF_SIZ ) {\
        cwrite ( "audgen overflow" );\
        return;\
    }\
    token = (I_what1);\
    bcopy ( &token, &a_d[(I_where)], sizeof token );\
    (I_where) += sizeof token;\
    if ( (I_where) + sizeof *(I_what2) >= BUF_SIZ ) {\
        cwrite ( "audgen overflow" );\
        return;\
    }\
    bcopy ( (I_what2), &a_d[(I_where)], sizeof *(I_what2) );\
    (I_where) += sizeof *(I_what2);\
}

/* insert null-terminated string */
/* location:I_where  type:I_what1  value:I_what2 */
#define INSERT_AUD1(I_where,I_what1,I_what2) \
{\
    if ( (I_where) + sizeof token >= BUF_SIZ ) {\
        cwrite ( "audgen overflow" );\
        return;\
    }\
    token = (I_what1);\
    bcopy ( &token, &a_d[(I_where)], sizeof token );\
    (I_where) += sizeof token;\
    for ( len = 0; *((I_what2)+len); len++ );\
    if ( (I_where) + sizeof len >= BUF_SIZ ) {\
        cwrite ( "audgen overflow" );\
        return;\
    }\
    bcopy ( &len, &a_d[(I_where)], sizeof len );\
    (I_where) += sizeof len;\
    if ( (I_where) + len >= BUF_SIZ ) {\
        cwrite ( "audgen overflow" );\
        return;\
    }\
    bcopy ( (I_what2), &a_d[(I_where)], len );\
    (I_where) += len;\
}

#define TP_AUID         T_AUID
#define TP_RUID         T_RUID
#define TP_UID          T_UID
#define TP_HOSTADDR     T_HOSTADDR
#define TP_EVENT        T_EVENT
#define TP_PID          T_PID
#define TP_PPID         T_PPID
#define TP_TV_SEC       T_TV_SEC
#define TP_TV_USEC      T_TV_USEC
#define TP_TZ_MIN       T_TZ_MIN
#define TP_TZ_DST       T_TZ_DST
#define TP_LENGTH       T_LENGTH

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: audgen_l" );
#endif /* __DEBUG3 */


    /* initialize daemon identifying values */
    gethostname ( hostname, MAXHOSTNAMELEN );
    if ( hostent = gethostbyname(hostname) )
        bcopy ( hostent->h_addr, &auditd_hostaddr, hostent->h_length );
    auditd_euid = geteuid();
    auditd_ruid = getuid();
    auditd_pid = getpid();
    auditd_ppid = getppid();

    /* copy length, auid, hostaddr, event, uid, pid, ppid, pidlvl into audit record */
    a_d_len = (sizeof token + sizeof j); /* leave space for T_LENGTH and length */
    auditd_auid = audcntl ( GETPAID, (char *)0, 0, 0, NULL, 0 );
    INSERT_AUD0 ( a_d_len, TP_AUID, &auditd_auid );
    INSERT_AUD0 ( a_d_len, TP_RUID, &auditd_ruid );
    INSERT_AUD0 ( a_d_len, TP_UID, &auditd_euid );
    INSERT_AUD0 ( a_d_len, TP_HOSTADDR, &auditd_hostaddr );
    INSERT_AUD0 ( a_d_len, TP_EVENT, &event );
    INSERT_AUD0 ( a_d_len, TP_PID, &auditd_pid );
    INSERT_AUD0 ( a_d_len, TP_PPID, &auditd_ppid );

    /* insert parameters */
    if ( *string1 ) INSERT_AUD1 ( a_d_len, T_CHARP, string1 );
    if ( *string2 ) INSERT_AUD1 ( a_d_len, T_CHARP, string2 );

    /* insert timestamp */
    gettimeofday ( &tp, &tzp );
    INSERT_AUD0 ( a_d_len, TP_TV_SEC, &tp.tv_sec );
    INSERT_AUD0 ( a_d_len, TP_TV_USEC, &tp.tv_usec );
    INSERT_AUD0 ( a_d_len, TP_TZ_MIN, &tzp.tz_minuteswest );
    INSERT_AUD0 ( a_d_len, TP_TZ_DST, &tzp.tz_dsttime );

    /* insert total length */
    j = 0;
    a_d_len += (sizeof token + sizeof j);
    INSERT_AUD0 ( j, TP_LENGTH, &a_d_len );
    j = a_d_len - (sizeof token + sizeof j);
    INSERT_AUD0 ( j, TP_LENGTH, &a_d_len );

    /* output audit record */
    output_rec ( a_d_len, a_d, OVERRIDE );
}
#endif /* ultrix */


/* check hostname against list of allowed hosts; return 0 (pass) or -1 (fail) */
int chk_access ( sckt )
int sckt;                               /* connection to be checked     */
{
    struct sockaddr_in peername;        /* peername for sckt connection */
    int namelen = sizeof(peername);
    int found = 0;                      
    int len;
    int fd;                             /* fd for AUDITD_CLNTS          */
    int start, end;                     /* posn's in current buf_l      */
    char *hostp;                        /* remote hostname              */
    char buf_l[BUF_SIZ];                /* input from AUDITD_CLNTS      */
    char hostname[MAXHOSTNAMELEN];      /* local hostname               */
    struct hndshk hndshk;               /* daemons' handshake data      */
#ifdef __KERBEROS
    char realm[REALM_SZ];               /* KERBEROS realm               */
#endif /* __KERBEROS */
    int i, j;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: chk_access" );
#endif /* __DEBUG3 */


    /* socket is non-blocking; use alarm to protect against infinite read loop */
    alarm(DEFTIMEOUT);
    signal ( SIGALRM, client_timeout );
    if ( setjmp(env) ) {
        cwrite ( "client connection timeout" );
        signal ( SIGALRM, sig_hndlr );
        return(-1);
    }

    /* read hndshk data */
    for ( i = j = 0; j < MAXHOSTNAMELEN; ) {
        i = read ( sckt, &hndshk.hostname[j], MAXHOSTNAMELEN-j );
        if ( i > 0 ) j += i;
    }
    for ( i = j = 0; i < sizeof(hndshk.kerb_flag); ) {
        i = read ( sckt, &buf_l[j], sizeof(hndshk.kerb_flag)-j );
        if ( i > 0 ) j += i;
    }
    alarm(0);
    signal ( SIGALRM, sig_hndlr );
    bcopy ( buf_l, &hndshk.kerb_flag, sizeof(hndshk.kerb_flag) );


    /* 1) cmp local hostname with accepted connection's hostname
       used to disallow talking to self across network
    */
    if ( gethostname ( hostname, MAXHOSTNAMELEN ) == -1 ) quit ( "gethostname", 0 );
    if ( strcmp ( hostname, hndshk.hostname ) == 0 ) {
        send ( sckt, "BYE", MSG_OOB_SIZE, MSG_OOB );
        shutdown ( sckt, 2 );
        close ( sckt );
        cwrite ( "cannot send to self" );
        return(-1);
    }


    /* 2) kerberos status on sender and receiver must match */
    if ( attr_g.kerb_auth != hndshk.kerb_flag ) {
        send ( sckt, "BYE", MSG_OOB_SIZE, MSG_OOB );
        shutdown ( sckt, 2 );
        close ( sckt );
        sprintf ( buf_l, "remote connection from %s refused; KERBEROS being used only on %s daemon",
        hndshk.hostname, attr_g.kerb_auth ? "local" : "remote" );
        cwrite ( buf_l );
        return(-1);
    }


    /* 3) connection must be on a reserved port */
    if ( getpeername ( sckt, &peername, &namelen ) < 0 ) {
        quit ( "getpeername", 0 );
        return(-1);
    }
    if ( (hostp = gethost_l(peername.sin_addr.s_addr)) == (char *)0 )
        return(-1);
    if ( htons(peername.sin_port) >= IPPORT_RESERVED ) {
        sprintf ( buf_l, "refused remote connection from %s, non-reserved port %d",
        hostp, htons(peername.sin_port) );
        cwrite ( buf_l );
        return(-1);
    }


#ifdef __KERBEROS
    /* 4) kerberos authentication check */
    if ( attr_g.kerb_auth ) {
        if ( kerb_ops ( KRB_RECVAUTH, sckt, &peername ) == -1 ) return(-1);
        hostp = auth_data.pinst;
        if ( strcmp ( auth_data.pname, "auditd" ) ) {
            sprintf ( buf_l, "kerberos principal mismatch: %s - auditd", auth_data.pname );
            cwrite ( buf_l );
            return(-1);
        }
        krb_get_lrealm ( realm, 0 );
        if ( strcmp ( realm, auth_data.prealm ) ) {
            sprintf ( buf_l, "kerberos realm mismatch: %s - %s", realm, auth_data.prealm );
            cwrite ( buf_l );
            return(-1);
        }
    }
#endif /* __KERBEROS */


    /* 5) client must appear in list of approved hosts */
    if ( (fd = open ( AUDITD_CLNTS, 0 )) == -1 ) {
        sprintf ( buf_l, "unable to open %s for access check", AUDITD_CLNTS );
        cwrite ( buf_l );
        return(-1);
    }

    /* works by maintaining current auditd_clnt line completely in buffer */
    for ( len = 0; len < MAXHOSTNAMELEN && hostp[len]; len++ );
    i = read ( fd, buf_l, sizeof buf_l );
    for ( j = 0; j < i; j++ ) {

        /* update filename */
        if ( strncmp ( hostp, &buf_l[j], len ) == 0 )
            if ( buf_l[j+len] == ' ' || buf_l[j+len] == '\t' || buf_l[j+len] == '\n' ) {
                found = 1;
                break;
            }

        /* if don't have complete request, shift buffer and get more input */
        for ( ; j < i && buf_l[j] != '\n'; j++ );
        for ( end = j+1 < i ? j+1 : i; end < i && buf_l[end] != '\n'; end++ );
        if ( end == i ) {
            for ( start = j; start < end; start++ )
                buf_l[start-j] = buf_l[start];
            end -= j;
            j = -1;
            if ( (i = read ( fd, &buf_l[end], sizeof buf_l - end )) == 0 ) break;
            i += end;
        }
    }

    close ( fd );
    if ( found == 0 ) {
        sprintf ( buf_l, "host %s failed access check to auditd master", hostp );
        cwrite ( buf_l );
        return(-1);
    }
    return(0);
}


/* repeatedly check for input on sd[1-3] descriptors */
int chk_input ( sd1, sd2, sd3_p, ss )
int sd1;                /* server-client socket descriptor  */
int sd2;                /* /dev/audit descriptor            */
int *sd3_p;             /* server-inet socket descriptor    */
struct sockaddr *ss;    /* admin socket                     */
{
    struct audit_attr attr_l;       /* local audit attr     */
    struct sockaddr c_addr;         /* connection address   */
    struct sockaddr_in inet_in;     /* inet socket address  */
    int mask;                       /* read descriptor mask */
    int ns_c = 0;                   /* client socket        */
    struct arp arp;                 /* audit data ptrs      */
    char buf_l[BUF_SIZ];            /* local buffer         */
    int conn;                       /* connection number    */
    union wait status;              /* child status         */
    int lastpass = 0;               /* for final data flush */
    struct timeval timeout;         /* select timeout       */
    pid_t closepid = -1;            /* proc for final FLUSH */
    pid_t chldpid;                  /* child pid            */
#ifdef __OSF_MMAP
    struct audiocp audiocp;         /* audit ioctl struct   */
    char *ptr;                      /* ptr to mmap'ed buff  */
#endif /* __OSF_MMAP */
    int i, j, k;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: chk_input(a)" );
#endif /* __DEBUG3 */


    output_rec ( 0, "\0", 0 ); /* device may check address space */
    timeout.tv_sec = timeout.tv_usec = 0;

#ifdef __OSF_MMAP
    /* size and mmap the /dev/audit data buffer */
    if ( sd2 >= 0 ) {
        if ( ioctl ( sd2, AUDIOCGETN, &audiocp ) ) quit ( "ioctl - /dev/audit", 1 );
        if ( (ptr = mmap ( '\0', audiocp.size, PROT_READ, MAP_FILE|MAP_VARIABLE|MAP_PRIVATE,
        sd2, 0 )) == (char *)-1 )
            quit ( "mmap of /dev/audit", 1 );
        arp.aud_rec = &ptr[audiocp.offset];
    }
#else
    /* get space for audit data */
    if ( (arp.aud_rec = (char *)malloc(attr_g.nkbytes*1024)) == NULL ) {
        cwrite ( "-- insufficient mem --" );
        errno = -1; /* to avoid incorrect error msg from quit() */
        quit ( "malloc", 1 );
    }
#endif /* __OSF_MMAP */
    arp.aud_rec_b = arp.aud_rec;


    /* heart of the auditd
       continually check for input on descriptors
       terminate only after kill and flush completed
       (extra passes performed on server auditd only)
    */
    for ( ; (attr_g.flag&ATTR_KILL) == 0 || (lastpass < 3 && attr_g.pid == 0)
    || closepid != -1; ) {

#ifdef __DEBUG3
        if ( attr_g.flag & ATTR_DEBUG ) {
            sprintf ( buf_l, "debug: chk_input(b): flag = %d  lastpass = %d  closepid = %d",
            attr_g.flag&ATTR_KILL, lastpass, closepid );
            cwrite ( buf_l );
        }
#endif /* __DEBUG3 */

        /* 1) collect child signals; free up resource */
        if ( attr_g.pid == 0 ) do {
            chldpid = wait3 ( &status, WNOHANG, (struct rusage *)0 );
            for ( j = 1; chldpid && chldpid != -1 && j <= n_conn; j++ )
                if ( child[j] == chldpid ) child[j] = -1;
            if ( closepid == chldpid ) closepid = -1;
        } while ( chldpid && chldpid != -1 );

        setjmp(env);


        /* 2) check that admin socket still exists; if not, re-create it */
        if ( (attr_g.pid == 0) && (access ( ADMIN_SOCK, R_OK ) == -1) )
            if ( errno == ENOENT ) {
                if ( access ( AUDITD_DIR, X_OK ) == -1 )
                    if ( errno == ENOENT ) {
                        if ( chdir ( "/" ) ) quit ( "chdir", 1 );
                        mkdir ( AUDITD_DIR, 0700 );
                        if ( chdir ( AUDITD_DIR ) ) quit ( "chdir", 1 );
                    }
                close ( sd1 );
                if ( (sd1 = socket ( AF_UNIX, SOCK_STREAM, 0 )) < 0 ) quit ( "socket: admin socket", 1 );
                if ( bind (sd1, ss, sizeof(struct sockaddr)) < 0 ) quit ( "bind: admin socket", 1 );
                if ( listen ( sd1, 5 ) < 0 ) quit ( "listen: admin socket", 1 );
                cwrite ( "forced to re-create admin socket" );
            }


        /* 3) in order to read all audit data, must follow sequence:
            0: turn off audswitch (done in handle_req)
            1: read all audit data available
            2: spawn child to perform FLUSH_AUD_BUF
            3: read all audit data available
          therefore, 3 passes to close down auditd */
        if ( lastpass == 2 && attr_g.pid == 0 && (closepid = fork()) == 0 ) {
            if ( audcntl ( FLUSH_AUD_BUF, (char *)0, 0, 0, 0, 0 ) == -1 )
                if ( errno != ENOSYS ) quit ( "audcntl: flush", 1 );
            _exit(0);
        }


        /* 4) select for pending connections */
        mask = 0;
        if ( sd1 >= 0 ) mask |= (1 << sd1);
        if ( sd2 >= 0 ) mask |= (1 << sd2);
        if ( *sd3_p >= 0 ) mask |= (1 << *sd3_p);
        if ( inet_out >= 0 ) mask |= (1 << inet_out);
        j = sd1 > sd2 ? sd1 : sd2;
        j = j > *sd3_p ? j : *sd3_p;
        j = j > inet_out ? j : inet_out;
        if ( sigtermed && lastpass == 0 ) lastpass++;
        if ( lastpass == 0 && !(attr_g.flag&ATTR_KILL) ) {
            if ( attr_g.flush_freq == 0 )
                i = select ( j+1, &mask, (int *)0, (int *)0, (char *)0 );
            else {
                timeout.tv_sec = attr_g.flush_freq;
                i = select ( j+1, &mask, (int *)0, (int *)0, &timeout );
            }
        }
        else {
            timeout.tv_sec = 0;
            i = select ( j+1, &mask, (int *)0, (int *)0, &timeout );
            lastpass++;
        }
#ifdef __DEBUG3
        if ( attr_g.flag & ATTR_DEBUG ) {
            sprintf ( buf_l, "debug: chk_input(c): select returned %d   mask = 0x%x",
            i, mask );
            cwrite ( buf_l );
        }
#endif /* __DEBUG3 */

        /* timeout - flush buffer */
        if ( i == 0 ) {
            audcntl ( FLUSH_AUD_BUF, (char *)0, 0, 0, 0, 0 );
            continue;
        }
        if ( i < 0 ) {
            /* invalid timeout value */
            if ( errno == EINVAL ) {
#ifdef __DEBUG3
                if ( attr_g.flag & ATTR_DEBUG ) {
                    sprintf ( buf_l, "debug: chk_input(c2): invalid timeout value" );
                    cwrite ( buf_l );
                }
#endif /* __DEBUG3 */
                attr_g.flush_freq = 0;
            }
            continue;
        }


        /* 5a) read administrative request client descriptor (sd1) */
        if ( (sd1 >= 0) && (mask & (1 << sd1)) ) {

            /* accept connection */
            i = sizeof(struct sockaddr);
            if ( (ns_c = accept ( sd1, &c_addr, &i )) < 0 ) {
                if ( errno != EWOULDBLOCK ) quit ( "accept: client", 0 );
            }

            /* read attributes packet */
            else {
                cw_switch = 1; /* make certain console warnings are enabled */

                /* this is a blocking socket; provide timeout to protect
                   against malicious/broken applications */
                alarm(DEFTIMEOUT);
                signal ( SIGALRM, client_timeout );
                if ( setjmp(env) ) {
                    cwrite ( "client connection timeout" );
                    signal ( SIGALRM, sig_hndlr );
                    continue;
                }
                i = read ( ns_c, &attr_l, sizeof(struct audit_attr) );
                alarm(0);
                signal ( SIGALRM, sig_hndlr );

                /* handle client request */
                if ( i == sizeof(struct audit_attr) )
                    handle_req ( &attr_l, ns_c, &arp, sd3_p );
                else cwrite ( "invalid client request -- ignored" );

                /* shutdown socket */
                shutdown ( ns_c, 2 );
                close ( ns_c );

                /* start shutdown sequence */
                if ( attr_g.flag & ATTR_KILL ) {
                    lastpass = 1;
                    continue;
                }
            }
        }


        /* 5b) establish new connections with remote auditd's (sd3) (server operation) */
        if ( (*sd3_p >= 0) && (attr_g.flag & ATTR_NETSRV) && (mask & (1 << *sd3_p)) ) {

            /* accept new connections */
            j = sizeof(struct sockaddr_in);
            if ( (i = accept ( *sd3_p, &inet_in, &j )) < 0 ) {
                if ( errno != EWOULDBLOCK ) quit ( "accept: server", 0 );
            }

            else {
                /* check # connections to not exceed maximum */
                for ( conn = 1; (conn <= n_conn) && (child[conn] != -1); conn++ );
                if ( conn == n_conn+1 ) {
                    send ( i, "BYE", MSG_OOB_SIZE, MSG_OOB );
                    shutdown ( i, 2 );
                    close(i);
                }

                /* perform hostname/peername access check (kerberos optional) */
                else if ( chk_access(i) == -1 ) {
                    send ( i, "BYE", MSG_OOB_SIZE, MSG_OOB );
                    shutdown ( i, 2 );
                    close ( i );
                }

                /* spawn child daemon to take care of this connection */
                else spawn_child ( &i, &sd1, &sd2, sd3_p, &arp, conn );
            }
        }


        /* 5c) read data from kernel (sd2) */
        if ( (sd2 >= 0) && (mask & (1 << sd2)) ) {
#ifdef __OSF_MMAP
            do {
                if ( ioctl ( sd2, AUDIOCGETN, &audiocp ) ) {
                    quit ( "ioctl - AUDIOCGETN", 0 );
                    break;
                }

                arp.aud_rec_b = &ptr[audiocp.offset];
                arp.aud_rec = &ptr[audiocp.offset+audiocp.nbytes];
                k = output_rec ( arp.aud_rec-arp.aud_rec_b, arp.aud_rec_b, 0 );
                if ( ioctl ( sd2, AUDIOCSETN, &k ) ) {
                    quit ( "ioctl - AUDIOCSETN", 0 );
                    break;
                }
            } while ( audiocp.nbytes );
#else
            /* check for full buffer */
            if ( arp.aud_rec-arp.aud_rec_b == attr_g.nkbytes*1024 ) {
                k = output_rec ( arp.aud_rec-arp.aud_rec_b, arp.aud_rec_b, 0 );
                arp.aud_rec -= k;
            }  

            /* read audit data from kernel via /dev/audit */
            for ( ; (i = read ( sd2, arp.aud_rec, attr_g.nkbytes*1024 -
            (arp.aud_rec-arp.aud_rec_b) )) > 0; ) {
                /* output current set of audit records */
                arp.aud_rec += i;
                if ( ((arp.aud_rec-arp.aud_rec_b) >= (attr_g.nkbytes-4)*1024) || nobuf ) {
                    k = output_rec ( arp.aud_rec-arp.aud_rec_b, arp.aud_rec_b, 0 );
                    arp.aud_rec -= k;
                }
            }
            if ( i == -1 ) quit ( "read", 0 );
#endif /* __OSF_MMAP */
        }


        /* 5d) read data from remote auditd connection (child operation) */
        if ( (mask & (1 << *sd3_p)) && (*sd3_p >= 0) && (attr_g.pid > 0) )
            chk_input_net ( sd3_p, &arp );


        /* 5e) check for OOB data from auditd to whom data is being sent */
        if ( (inet_out >= 0) && (attr_g.pathval == REMOTE) && (mask & (1 << inet_out)) )
            if ( recv ( inet_out, buf_l, MSG_OOB_SIZE, MSG_OOB ) > 0 ) {
                sprintf ( buf_l, "%s not receiving", attr_g.pathname );
                cwrite ( buf_l );
                shutdown ( inet_out, 2 );
                close ( inet_out );
                inet_out = -1;
                attr_g.pathval = LOCAL;
                if ( log_chng ( (char *)0, 1, LC_CHANGE ) == (char *)-1 ) {
                    cwrite ( "data lost" );
                    cw_switch = 0;
                }
            }
    }


    /* 6) flush auditd buffer */
#ifdef __OSF_MMAP
    if ( attr_g.pid == 0 && sd2 >= 0 ) {
        if ( ioctl ( sd2, AUDIOCGETN, &audiocp ) ) quit ( "ioctl - AUDIOCGETN", 0 );
        arp.aud_rec_b = &ptr[audiocp.offset];
        arp.aud_rec = &ptr[audiocp.offset+audiocp.nbytes];
    }
#endif /* __OSF_MMAP */
    k = output_rec ( arp.aud_rec-arp.aud_rec_b, arp.aud_rec_b, 0 );
#ifdef __OSF_MMAP
    if ( attr_g.pid == 0 && sd2 >= 0 )
        if ( ioctl ( sd2, AUDIOCSETN, &k ) ) quit ( "ioctl - AUDIOCSETN", 0 );
#endif /* __OSF_MMAP */

#ifdef __osf__
    audgen_l ( AUDIT_STOP, "audit subsystem off", "" );
#endif /* __osf__ */
#ifdef ultrix
    audgen_l ( AUDIT_SHUTDOWN, "audit subsystem off", "" );
#endif /* ultrix */
}


/* read data from remote auditd connection (child-server operation) */
chk_input_net ( sd3_p, arp )
int *sd3_p;         /* server-inet socket descriptor */
struct arp *arp;    /* audit data ptrs               */
{
#ifdef __KERBEROS
    struct sockaddr_in sl, sf; /* addresses for KERBEROS    */
    unsigned long pktlen;
#endif /* __KERBEROS */
    char buf_l[BUF_SIZ];       /* local buffer              */
    int i, k;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: chk_input_net" );
#endif /* __DEBUG3 */


    /* read audit data from inet (non-blocking socket) */
    for ( k = 0; (i = read ( *sd3_p, arp->aud_rec, attr_g.nkbytes*1024 - 
    (arp->aud_rec - arp->aud_rec_b) )) > 0 || 
    attr_g.nkbytes*1024 == arp->aud_rec - arp->aud_rec_b; ) {

#ifdef __DEBUG3
        if ( attr_g.flag & ATTR_DEBUG ) {
            sprintf ( buf_l, "debug: chk_input_net(b): read %d bytes  nkbytes = 0x%x", i, attr_g.nkbytes*1024 );
            cwrite ( buf_l );
            sprintf ( buf_l, "       arp.aud_rec = 0x%x  arp.aud_rec_b = 0x%x",arp->aud_rec, arp->aud_rec_b );
            cwrite ( buf_l );
        }
#endif /* __DEBUG3 */

        k = 1;    /* mark that read occurred successfully */
        arp->aud_rec += i;

        /* if threshold exceeded or nobuffer flag set, output data */
        if ( attr_g.kerb_auth == 0 ) {
            if ( ((arp->aud_rec-arp->aud_rec_b) >= (attr_g.nkbytes-4)*1024)  || nobuf ) {
                i = output_rec ( arp->aud_rec - arp->aud_rec_b, arp->aud_rec_b, 0 );
                arp->aud_rec -= i;
            }
        }

#ifdef __KERBEROS
        /* KERBEROS turned on */

        /* make sure at least length is read (first 6 bytes) */
        else if ( arp->aud_rec - arp->aud_rec_b > KRB_HDRSIZ ) {

            /* get packet length; must have complete packet (pktlen+31) bytes */
            bcopy ( &arp->aud_rec_b[2], &pktlen, sizeof pktlen );
            for ( ; arp->aud_rec - arp->aud_rec_b >= pktlen+KRB_PKTSIZ; ) {

                /* unencapsulate data */
                i = sizeof(struct sockaddr_in);
                if ( getpeername ( *sd3_p, &sf, &i ) )
                    quit ( "getpeername (krb)", 0 );
                i = sizeof(struct sockaddr_in);
                if ( getsockname ( *sd3_p, &sl, &i ) )
                    quit ( "getsockname (krb)", 0 );
                if ( i = krb_rd_safe ( arp->aud_rec_b, pktlen+KRB_PKTSIZ,
                auth_data.session, &sf, &sl, &msg_data ) ) {
                    sprintf ( buf_l, "krb_rd_safe error %d", i );
                    cwrite ( buf_l );
                    send ( inet_out, "BYE", MSG_OOB_SIZE, MSG_OOB );
                    shutdown ( inet_out, 2 );
                    attr_g.flag |= ATTR_KILL;
                    break;
                }

                /* output safe data locally; shift remaining data */
                else {
                    output_rec ( msg_data.app_length, msg_data.app_data, 0 );
                    /* bcopy safe for overlapping strings */
                    bcopy ( &arp->aud_rec_b[pktlen+KRB_PKTSIZ], arp->aud_rec_b, arp->aud_rec - arp->aud_rec_b );
                    arp->aud_rec -= (pktlen+KRB_PKTSIZ);
                    bcopy ( &arp->aud_rec_b[2], &pktlen, sizeof pktlen );
                }
            }
        }
#endif /* __KERBEROS */
    }

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) {
        sprintf ( buf_l, "debug: chk_input_net(c): k = %d  i = %d  errno = %d\n",
            k, i, errno );
        cwrite ( buf_l );
    }
#endif /* __DEBUG3 */
    if ( k == 0 && i == -1 && errno != EWOULDBLOCK && errno != ECONNRESET )
        quit ( "read", 0 );


    /* exit on receiving OOB data from client auditd
       exit if selected and no data available
    */
    if ( (i = recv ( *sd3_p, buf_l, MSG_OOB_SIZE, MSG_OOB ) > 0) || (k == 0) ) {
        attr_g.flag |= ATTR_KILL;
        unlink ( clientsockname(attr_g.pid) );
    }

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: chk_input_net exit" );
#endif /* __DEBUG3 */
}


/* act as (local) client; transfer attribute values to auditd server
   return -1 on error; 1 on success
   return 0 if no auditd exists; must start auditd
*/
int client ( attr_l, conn_name, outd, timeoutval )
struct audit_attr *attr_l;      /* attribute structure  */
struct sockaddr *conn_name;     /* socket to connect to */
int outd;                       /* output descriptor    */
int timeoutval;                 /* timeout period       */
{
    int s_c;                    /* socket descr         */
    char buf_l[BUF_SIZ];        /* local buffer         */
    char path_l[MAXPATHLEN];    /* directory            */
    struct sockaddr ss;         /* admin socket         */
    int i, j;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: client" );
#endif /* __DEBUG3 */


#ifdef __osf__
    /* if able to open /dev/audit, no server running; device is exclusive use */
    if ( (i = open ( "/dev/audit", 0 )) >= 0 ) {
        close(i);
        if ( getwd ( path_l ) == 0 ) {
            quit ( "getwd", 0 );
            return(-1);
        }
        if ( chdir ( AUDITD_DIR ) == -1 ) return(0);
        unlink ( ADMIN_SOCK );
        j = getdtablesize()-5 < MAXCLIENTS-1 ? getdtablesize()-5 : MAXCLIENTS-1;
        for ( ; j >= 1; j-- )
            unlink ( clientsockname(j) );
        chdir ( path_l );
        return(0);
    }
#endif /* __osf__ */


    /* create socket */
    if ( (s_c = socket ( AF_UNIX, SOCK_STREAM, 0 )) < 0 ) {
        quit ( "socket: client", 0 );
        return(-1);
    }

    /* qaulify names relative to current directory */
    if (attr_l->pathval == LOCAL ) qual_name(attr_l->pathname);
    qual_name(attr_l->console);


    /* request connection; use conn_name if supplied, else ADMIN_SOCK */
    chdir ( AUDITD_DIR );
    if ( conn_name == (struct sockaddr *)0 ) {
        ss.sa_family = AF_UNIX;
        bcopy ( ADMIN_SOCK, ss.sa_data, sizeof ADMIN_SOCK );
        i = connect ( s_c, &ss, sizeof(struct sockaddr) );
    }
    else i = connect ( s_c, conn_name, sizeof(struct sockaddr) );
    if ( i < 0 ) {
        close ( s_c );
        if ( errno == ENOENT ) return(0);
        else {
            quit ( "connect: client", 0 );
            return(-1);
        }
    }


    /* transmit packet */
    if ( write ( s_c, attr_l, sizeof(struct audit_attr) ) != sizeof(struct audit_attr) ) {
        quit ( "transmit packet: client", 0 );
        close ( s_c );
        return(-1);
    }


    /* read response; this will block on i/o (reason for conditional)
       apply a timer to be careful
    */
    if ( attr_l->pathval || attr_l->console[0] || attr_l->nkbytes > 0
    || attr_l->flag & (ATTR_SHOW|ATTR_QUERY|ATTR_HELP|ATTR_SETLOC) ) {
        signal ( SIGALRM, client_timeout );
        if ( setjmp(env) ) {
            cwrite ( "client connection timeout" );
            signal ( SIGALRM, sig_hndlr );
            close ( s_c );
            return(-1);
        }
        do {
            alarm ( timeoutval );
            i = read ( s_c, buf_l, sizeof buf_l );
            write ( outd, buf_l, i );
            alarm(0);
        } while ( i > 0 );
        signal ( SIGALRM, sig_hndlr );
    }

    close ( s_c );
    return(1);
}


/* return client socket name suffixed with specified index */
char *clientsockname ( indx )
int indx;
{
    static char buf_l[BUF_SIZ];
    int i;

    for ( i = 0; buf_l[i] = CHILD_SOCK[i]; i++ );
    bcopy ( itoa(indx,MAXCL_ITOA), &buf_l[i], MAXCL_ITOA );
    buf_l[i+MAXCL_ITOA] = '\0';

    return ( buf_l );
}


/* client timeout signal handler */
client_timeout()
{
#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: client_timeout" );
#endif /* __DEBUG3 */

    longjmp ( env, 1 );
}


/* have child process compress auditlog */
compress ( filnam )
char *filnam;
{
    struct stat sbuf;
    int i;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: compress" );
#endif /* __DEBUG3 */


    /* don't compress devices */
    if ( stat ( filnam, &sbuf ) == 0 )
        if ( (sbuf.st_mode & S_IFREG) == 0 ) return;

    if ( fork() == 0 ) {

        setpriority ( PRIO_PROCESS, getpid(), 0 );

        /* disassociate from process group and controlling tty */
        if ( setpgrp ( 0, getpid() ) == -1 ) quit ( "setpgrp", 0 );
        if ( (i = open ( "/dev/tty", 2 )) >= 0 ) {
            ioctl ( i, TIOCNOTTY, 0 );
            close(i);
        }
        for ( i = getdtablesize()-1; i >= 0; i-- ) close(i);

        execl ( "/usr/ucb/compress", "compress", filnam, 0 );
        cwrite ( "failed to compress auditlog" );
        _exit(1);
    }
}


/* output string and timestamp to attr_g.console */
cwrite ( string )
char *string;
{
    long clock;
    int i, j;

    /* throttle repeated "data lost" messages" */
    if ( cw_switch == 0 ) return;

    time ( &clock );
    if ( (i = open ( attr_g.console, O_RDWR|O_CREAT|O_APPEND|O_NOCTTY|O_NDELAY, 0600 )) >= 0 ) {
        write ( i, "auditd", 6 );
        if ( attr_g.pid > 0 && attr_g.pid <= 9 ) {
            write ( i, "(", 1 );
            write ( i, itoa(attr_g.pid,1), 1 );
            write ( i, ")", 1 );
        }
        write ( i, ": ", 2 );

        for ( j = 0; string[j]; j++ );
        /* explicitly ignore error return from I/O
           e.g. if pty slave device failed to open
        */
        write ( i, string, j );
        write ( i, " - ", 3 );
        write ( i, ctime(&clock), 25 );
        close(i);
    }
}


/* return hostname associated with ipaddr */
char *gethost_l ( ipaddr )
u_int ipaddr;
{
    struct hostent *hostp;
    static u_int ipaddr_prev = 0;
    static char h_name_prev[MAXHOSTNAMELEN];
    int i;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: gethost_l" );
#endif /* __DEBUG3 */


    if ( ipaddr_prev && ipaddr == ipaddr_prev ) return ( h_name_prev );
    if ( (hostp = gethostbyaddr ( &ipaddr, sizeof(int), AF_INET )) == (struct hostent *)0 )
        return ( (char *)0 );

    /* save hostname, ipaddr for next iteration */
    ipaddr_prev = ipaddr;
    for ( i = 0; i < MAXHOSTNAMELEN && hostp->h_name[i]; i++ );
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
    int i;

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


/* handle client request */
handle_req ( attr_l, ns, arp, sd3_p )
struct audit_attr *attr_l;              /* local audit attr         */
int ns;                                 /* server-client socket     */
struct arp *arp;                        /* audit data pointers      */
int *sd3_p;                             /* inet socket ptr          */
{
    char pathnam_l[MAXPATHLEN];
    char pathnam2_l[MAXPATHLEN];
    char hostname[MAXHOSTNAMELEN];
    char buf_l[BUF_SIZ];                /* local buffer             */
    struct hostent *hostent;
    struct sockaddr_in peername;        /* peername across inet     */
    int namelen = sizeof(peername);
    struct sockaddr sl;
    struct stat statbuf;

    int i, j;
    char *ptr;
    char ch;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: handle_req" );
#endif /* __DEBUG3 */


    /* children: check connection to remote client.
       this only catches dead connections when admin is active;
       otherwise, must wait for tcp to reset connection.
    */
    if ( attr_g.pid > 0 ) {
        if ( getpeername ( *sd3_p, &peername, &namelen ) == 0 )
            if ( ptr = gethost_l(peername.sin_addr.s_addr) )
                if ( ping ( ptr ) ) {
                    audgen_l ( AUDIT_XMIT_FAIL, "connection broken by remote host", ptr );
                    sprintf ( buf_l, "connection broken by remote host %s", ptr );
                    cwrite ( buf_l );
                    attr_g.flag |= ATTR_KILL;
                    return;
                }
    }


    /* divert request to child daemon (must be first check for action to take) */
    if ( attr_l->pid > 0 ) {
        sl.sa_family = AF_UNIX;
        for ( i = 0; sl.sa_data[i] = CHILD_SOCK[i]; i++ );
        bcopy ( itoa(attr_l->pid,MAXCL_ITOA), &sl.sa_data[i], MAXCL_ITOA );
        sl.sa_data[i+MAXCL_ITOA] = '\0';
        attr_l->flag &= ~ATTR_NETSRV;
        attr_l->pid = -1;
        client ( attr_l, &sl, ns, attr_g.timeout );
        return;
    }


    /* change auditlog pathname */
    if ( attr_l->pathval == LOCAL ) {
#ifdef __OSF_MMAP
        if ( attr_g.pid ) {
#endif /* __OSF_MMAP */
            i = output_rec ( arp->aud_rec - arp->aud_rec_b, arp->aud_rec_b, 0 );
            arp->aud_rec -= i;
#ifdef __OSF_MMAP
        }
#endif /* __OSF_MMAP */
        log_chng ( attr_l->pathname, 0, LC_CHANGE );
    }


    /* check for routing to self by comparing 'official' names of hosts */
    if ( attr_l->pathval == REMOTE && attr_g.pid == 0 ) {
        gethostname ( hostname, MAXHOSTNAMELEN );
        if ( hostent = gethostbyname(hostname) )
            strncpy ( pathnam_l, (char *)hostent->h_name, MAXPATHLEN );
        else {
            attr_l->pathval = '\0';
            write ( ns, "-- invalid hostname --\n", 23 );
        }

        if ( hostent = gethostbyname(attr_l->pathname) )
            strncpy ( pathnam2_l, (char *)hostent->h_name, MAXPATHLEN );
        else {
            attr_l->pathval = '\0';
            write ( ns, "-- invalid hostname --\n", 23 );
        }

        if ( attr_l->pathval && strcmp ( pathnam_l, pathnam2_l ) == 0 ) {
            bcopy ( DEFAULT_LOG, attr_l->pathname, sizeof DEFAULT_LOG );
            attr_l->pathval = LOCAL;
        }
    }


    /* transmit data over inet */
    if ( attr_l->pathval == REMOTE && attr_g.pid == 0 ) {

        /* dump current buffers, close old log */
#ifndef __OSF_MMAP
        i = output_rec ( arp->aud_rec - arp->aud_rec_b, arp->aud_rec_b, 0 );
        arp->aud_rec -= i;
#endif /* __OSF_MMAP */
        audgen_l ( AUDIT_LOG_CHANGE, "auditlog change to host", attr_l->pathname );
        sprintf ( buf_l, "routing audit data to %s", attr_l->pathname );
        cwrite ( buf_l );

        if ( attr_g.pathval == LOCAL ) {
            close ( fda );
            fda = -1;
            compress ( attr_g.pathname );
        }
        if ( attr_g.pathval == REMOTE ) {
            send ( inet_out, "BYE", MSG_OOB_SIZE, MSG_OOB );
            shutdown ( inet_out, 2 );
            close ( inet_out );
            inet_out = -1;
        }

        /* set up new log */
        for ( i = 0; i < MAXPATHLEN && (attr_g.pathname[i] = attr_l->pathname[i]); i++ );
        attr_g.pathval = attr_l->pathval;

#ifdef __KERBEROS
        /* must be able to xfer complete KERBEROS packet, so set nbytes */
        if ( attr_g.kerb_auth ) attr_l->nkbytes = KRB_SENDSIZ;
#endif /* __KERBEROS */
        output_rec ( 0, "\0", 0 );
    }


    /* change idea of console */
    if ( attr_l->console[0] != '\0' ) {
        if ( (i = open ( attr_l->console, O_RDWR|O_CREAT|O_APPEND|O_NOCTTY|O_NDELAY, 0600 )) < 0 )
            write ( ns, "-- invalid console name --\n", 27 );
        else {
            close(i);
            for ( j = 0; attr_g.console[j] = attr_l->console[j]; j++ );
        }
    }


    /* dump audit buffer */
    if ( attr_l->flag & ATTR_DUMP ) {
#ifdef __OSF_MMAP
        if ( attr_g.pid ) {
#endif /* __OSF_MMAP */
            i = output_rec ( arp->aud_rec - arp->aud_rec_b, arp->aud_rec_b, 0 );
            arp->aud_rec -= i;
#ifdef __OSF_MMAP
        }
#endif /* __OSF_MMAP */
    }


    /* set dump frequency */
    if ( attr_l->flush_freq != -1 ) {
        attr_g.flush_freq = attr_l->flush_freq;
        strncpy ( attr_g.flush_arg, attr_l->flush_arg, TIME_LEN );
    }


    /* kill process */
    if ( attr_l->flag & ATTR_KILL ) attr_g.flag |= ATTR_KILL;
    if ( (attr_g.flag&ATTR_KILL) && attr_g.pid == 0 ) {
        if ( audcntl ( SET_AUDSWITCH, (char *)0, 0, 0, 0, 0 ) == -1 )
            if ( errno != ENOSYS ) quit ( "audcntl", 1 );
    }


    /* increment log_indx (suffix on pathname)
       if data going to device or across network, nop
    */
    if ( attr_l->flag & ATTR_INCR ) {
#ifdef __OSF_MMAP
        if ( attr_g.pid ) {
#endif /* __OSF_MMAP */
            i = output_rec ( arp->aud_rec - arp->aud_rec_b, arp->aud_rec_b, 0 );
            arp->aud_rec -= i;
#ifdef __OSF_MMAP
        }
#endif /* __OSF_MMAP */
        if ( stat ( attr_g.pathname, &statbuf ) == 0 )
            if ( (statbuf.st_mode & S_IFREG) && attr_g.pathval == LOCAL ) {
                for ( i = 0; i < MAXPATHLEN && attr_g.pathname[i]; i++ );
                bcopy ( attr_g.pathname, pathnam_l, i );
                pathnam_l[i-INDX] = '\0';
                log_chng ( pathnam_l, 0, LC_CHANGE );
            }
    }


    /* toggle use of kerberos authentication */
    if ( attr_l->kerb_auth && attr_g.pid == 0 ) attr_g.kerb_auth ^= 1;


    /* change size of audit data buffer */
    if ( attr_l->nkbytes == 0 ) nobuf = 1;
    if ( attr_l->nkbytes >= MIN_AUD_BUF && attr_g.pid == 0 ) {
        nobuf = 0;
#ifdef __OSF_MMAP
        attr_g.nkbytes = attr_l->nkbytes;
#else
        i = output_rec ( arp->aud_rec - arp->aud_rec_b, arp->aud_rec_b, 0 );
        arp->aud_rec -= i;

        /* do not use brk() because of other memory allocation - should use malloc */
        if ( attr_l->nkbytes > attr_g.nkbytes ) {
            if ( (arp->aud_rec = (char *)malloc(attr_l->nkbytes*1024)) == NULL ) {
                write ( ns, "-- insufficient mem --\n", 23 );
                arp->aud_rec = arp->aud_rec_b;
            }
            else {
                free ( arp->aud_rec_b );
                arp->aud_rec_b = arp->aud_rec;
                attr_g.nkbytes = attr_l->nkbytes;
            }
        }
        else attr_g.nkbytes = attr_l->nkbytes;
#endif /* __OSF_MMAP */
    }


    /* toggle net_server status */
    if ( (attr_l->flag & ATTR_NETSRV) && attr_g.pid == 0 ) {
        if ( attr_g.flag & ATTR_NETSRV ) attr_g.flag &= ~ATTR_NETSRV;
        else attr_g.flag |= ATTR_NETSRV;
        if ( net_serv ( attr_g.flag & ATTR_NETSRV, sd3_p ) == -1 )
            attr_g.flag &= ~ATTR_NETSRV;
    }


    /* set connection timeout value, used by ping() */
    if ( attr_l->timeout > 0 ) attr_g.timeout = attr_l->timeout;


    /* set overflow threshold */
    if ( attr_l->freepct >= 0 && attr_l->freepct <= 100 ) attr_g.freepct = attr_l->freepct;


    /* set alternate auditlog locations */
    if ( attr_l->flag & ATTR_SETLOC )
        if ( log_chng ( (char *)0, 0, LC_SETLOC ) == (char *)-1 )
            write ( ns, "-- unable to set dir list --\n", 29 );


    /* set overflow action */
    if ( attr_l->overflow && attr_l->overflow < N_OVERFLOW_OPTS ) {
        attr_g.overflow = attr_l->overflow;
        audgen_l ( AUDIT_SETUP, "overflow action", overflow_act[attr_g.overflow].cmnd_descr );
    }


#ifdef __DEBUG3
    /* toggle debug flag */
    if ( attr_l->flag & ATTR_DEBUG ) {
        if ( attr_g.flag & ATTR_DEBUG ) attr_g.flag &= ~ATTR_DEBUG;
        else attr_g.flag |= ATTR_DEBUG;
    }
#endif /* __DEBUG3 */


    /* query name from server */
    if ( attr_l->flag & ATTR_QUERY ) {
        for ( i = 0; i < MAXPATHLEN && attr_g.pathname[i]; i++ );
        write ( ns, attr_g.pathname, i );
        write ( ns, "\n", 1 );
    }


    /* process show status, help requests */
    if ( attr_l->flag & ATTR_SHOW ) show_stat ( ns, *sd3_p );
    if ( attr_l->flag & ATTR_HELP ) show_help ( ns );

}


/* initialize attributes structure */
init_attr ( attr_p )
struct audit_attr *attr_p;
{
    attr_p->console[0] = '\0';
    attr_p->flag = 0;                   
    attr_p->flush_arg[0] = '\0';
    attr_p->flush_freq = -1;
    attr_p->freepct = -1;
    attr_p->nkbytes = -1;
    attr_p->overflow = 0;
    attr_p->pathname[0] = '\0';
    attr_p->pathval = '\0';
    attr_p->pid = -1;
    attr_p->timeout = -1;
}


/* convert integer decimal to alphanumeric string
   return ptr to zero-filled string of width min_width
*/
char *itoa ( num, min_width )
int num;
int min_width;
{
    static char num_rep[13];
    int i;

    for ( i = 0; i < sizeof num_rep; i++ ) num_rep[i] = '0';
    for ( i = sizeof num_rep-1; (i == sizeof num_rep - 1) || (num > 0 && i >= 0); num = num/10, i-- )
        num_rep[i] = num%10 + '0';

    min_width++;
    i = i < sizeof num_rep - min_width ? i : sizeof num_rep - min_width;
    return ( &num_rep[++i] );
}


/* kerberos operations; return -1 on failure */
#ifdef __KERBEROS
int kerb_ops ( op, fd, sinp )
int op;
int fd;
struct sockaddr_in *sinp;
{
    char version[9];
    KTEXT_ST ticket;                    /* kerberos ticket          */
    Key_schedule ksched;
    char realm[REALM_SZ];               /* KERBEROS realm           */

    static int initialized = 0;         /* init flag                */
    struct sockaddr_in sl;              /* local inet address       */
    char hostname[MAXHOSTNAMELEN];
    struct timeval tp;
    struct timezone tzp;
    char buf_l[BUF_SIZ];
    char *hostp;
    int i;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: kerb_ops" );
#endif /* __DEBUG3 */


    gethostname ( hostname, MAXHOSTNAMELEN );
    for ( i = 0; i < MAXHOSTNAMELEN && hostname[i] != '.' && hostname[i]; i++ );
    if ( i < MAXHOSTNAMELEN ) hostname[i] = '\0';

    /* get socket attributes; make non-blocking for kerberos */
    if ( fd >= 0 ) {
        i = sizeof(struct sockaddr_in);
        if ( getsockname ( fd, &sl, &i ) ) {
            quit ( "getsockname: krb", 0 );
            return(-1);
        }
        else {
            i = 0;   /* make socket non-blocking for KERBEROS */
            if ( ioctl ( fd, FIONBIO, &i ) < 0 ) {
                quit ( "ioctl: failed to modify socket for KERBEROS", 0 );
                return(-1);
            }
        }
    }

    /* initiate time for kerberos operations */
    alarm(KRB_TIMEOUT);
    signal ( SIGALRM, kerb_timeout );
    if ( setjmp(env) ) {
        cwrite ( "kerberos timeout" );
        signal ( SIGALRM, sig_hndlr );
        return(-1);
    }

    /* kerberos initialization */
    if ( initialized == 0 ) {
        initialized++;
        i = krb_svc_init ( "auditd", hostname, NULL, 0, NULL, TKT_FILE_L );
        if ( i != RET_OK ) {
            sprintf ( buf_l, "krb_svc_init bombed with %d", i );
            cwrite ( buf_l );
        }
    }


    switch ( op ) {

    case KRB_RECVAUTH:
        if ( (i = krb_recvauth ( KOPT_DO_MUTUAL, fd, &ticket, "auditd",
        hostname, sinp, &sl, &auth_data, "", ksched, version )) != KSUCCESS ) {
            i = 1;
            if ( ioctl ( fd, FIONBIO, &i ) < 0 ) quit ( "ioctl: KRB_RECVAUTH", 0 );
            alarm(0);
            signal ( SIGALRM, sig_hndlr );
            return(-1);
        }

        /* restore socket to non-blocking state */
        i = 1;
        if ( ioctl ( fd, FIONBIO, &i ) < 0 ) quit ( "ioctl: KRB_RECVAUTH", 0 );

        /* compare given hostname with kerberos authenticated name */
        if ( (hostp = gethost_l(sinp->sin_addr.s_addr)) == (char *)0 ) {
            alarm(0);
            signal ( SIGALRM, sig_hndlr );
            return(-1);
        }
        i = strcmp ( hostp, auth_data.pinst );
        if ( i ) {
            alarm(0);
            signal ( SIGALRM, sig_hndlr );
            sprintf ( buf_l, "kerberos - %s and %s do not match", hostp, auth_data.pinst );
            cwrite ( buf_l );
            return(-1);
        }
        break;


    case KRB_SENDAUTH:
        for ( i = 0; i < MAXHOSTNAMELEN && attr_g.pathname[i] != '.' && attr_g.pathname[i]; i++ )
            hostname[i] = attr_g.pathname[i];
        hostname[i] = '\0';
        gettimeofday ( &tp, &tzp );

        krb_get_lrealm ( realm, 0 );
        if ( (i = krb_sendauth ( KOPT_DO_MUTUAL, fd, &ticket, "auditd", hostname,
        realm, tp.tv_usec, &msg_data, &cred, ksched, &sl, sinp, "" )) != KSUCCESS ) {
            alarm(0);
            signal ( SIGALRM, sig_hndlr );
            sprintf ( buf_l, "KERBEROS krb_sendauth error %s (%d)", krb_err_txt[i], i );
            cwrite ( buf_l );
            i = 1;
            if ( ioctl ( fd, FIONBIO, &i ) < 0 ) quit ( "ioctl: KRB_SENDAUTH", 0 );
            return(-1);
        }
        i = 1;
        if ( ioctl ( fd, FIONBIO, &i ) < 0 ) quit ( "ioctl: KRB_SENDAUTH", 0 );
        break;

    }

    alarm(0);
    signal ( SIGALRM, sig_hndlr );
    return(0);
}
#endif /* __KERBEROS */


/* kerberos timeout signal handler */
#ifdef __KERBEROS
kerb_timeout()
{
#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: kerb_timeout" );
#endif /* __DEBUG3 */

    longjmp ( env, 1 );
}
#endif /* __KERBEROS */


/* set auditlog and backup log directories */
char *log_chng ( log_new, severity, op )
char *log_new;
int severity;
int op;
{
    static char dirlist[MAXPATHLEN*8];
    static int dirlist_set = 0;
    static char *ptr = dirlist;
    static char *nextptr = dirlist;
    static char nextpath[MAXPATHLEN];
    struct stat statbuf;
    int retval;
    int fd;
    int i;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: log_chng" );
#endif /* __DEBUG3 */


    switch ( op ) {

    case LC_CHANGE:
        /* get next pathname from `dirlist` */
        if ( attr_g.overflow == OA_CD && dirlist_set && log_new == (char *)0 ) {

            for ( retval = -1; retval == -1; ) {
                for ( ; *ptr && WHITE_SPACE(*ptr); ptr++ );
                for ( i = 0; *ptr && !WHITE_SPACE(*ptr); ptr++, i++ )
                    nextpath[i] = *ptr;

                /* EOF - just return */
                if ( i == 0 || *ptr == '\0' ) {
                    if ( attr_g.pid == 0 ) strcpy ( nextpath, DEFAULT_LOG );
                    else strcpy ( nextpath, DEFAULT_LOG_REM );
                    retval = log_chng_next ( nextpath, severity );
                }

                /* check for hostname: */
                else if ( i && nextpath[i-1] == ':' ) {
                    nextpath[i-1] = '\0';
                    retval = log_chng_rmt ( nextpath, severity );
                }

                /* append basename(attr_g.pathname), remove index number */
                else {
                    if ( rindex(attr_g.pathname,'/') )
                        strcpy ( &nextpath[i], rindex(attr_g.pathname,'/') );
                    else if ( attr_g.pid && rindex(DEFAULT_LOG_REM,'/') )
                        strcpy ( &nextpath[i], rindex(DEFAULT_LOG_REM,'/') );
                    else if ( rindex(DEFAULT_LOG,'/') )
                        strcpy ( &nextpath[i], rindex(DEFAULT_LOG,'/') );
                    if ( stat ( attr_g.pathname, &statbuf ) == 0 )
                        if ( (statbuf.st_mode & S_IFREG) && attr_g.pathval == LOCAL ) {
                            for ( i = 0; nextpath[i]; i++ );
                            nextpath[i-INDX] = '\0';
                        }
                    retval = log_chng_next ( nextpath, severity );
                }
            }

            return ( (char *)retval );
        }

        /* get next pathname from "log_new", or just increment */
        return ( (char *)log_chng_next ( log_new, severity ) );
        break;


    case LC_GET_NEXTDIR:
        /* get AUDITD_LOGLOC entries, one at a time; list is null-terminated */
        if ( dirlist_set ) {
            if ( *nextptr == '\0' ) nextptr = ptr;
            for ( ; WHITE_SPACE(*nextptr); nextptr++ );
            for ( i = 0; i < sizeof nextpath && *nextptr && !WHITE_SPACE(*nextptr); nextptr++, i++ )
                nextpath[i] = *nextptr;
            nextpath[i] = '\0';
            return ( nextpath );
        }
        return ( (char *)0 );
        break;


    case LC_SETLOC:
        /* load dirlist from AUDITD_LOGLOC */
        if ( (fd = open ( AUDITD_LOGLOC, 0 )) == -1 ) dirlist_set = 0;
        else {
            i = read ( fd, dirlist, MAXPATHLEN*8 );
            close(fd);
            if ( i < MAXPATHLEN*8 ) dirlist[i] = '\0';
            else dirlist[MAXPATHLEN*8 - 1] = '\0';
            dirlist_set = 1;
            ptr = nextptr = dirlist;
        }
        break;

    };


    return ( (char *)0 );
}


/* switch to next auditlog */
int log_chng_next ( log_new, severity )
char *log_new;
int severity;
{
    int fdl;                        /* tmp descr      */
    char buf_l[BUF_SIZ];            /* local buffer   */
    char log_new_l[MAXPATHLEN];     /* local log name */
    struct stat sbuf;
    int dvc = 0;                    /* 1 for devices  */
    int i;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: log_chng_next" );
#endif /* __DEBUG3 */


    /* build local copy of logfile name */
    if ( log_new )
        for ( i = 0; i < MAXPATHLEN-INDX && (log_new_l[i] = log_new[i]); i++ );
    else {
        if ( attr_g.pid == 0 )
            for ( i = 0; i < MAXPATHLEN-INDX && (log_new_l[i] = DEFAULT_LOG[i]); i++ );
        else
            for ( i = 0; i < MAXPATHLEN-INDX && (log_new_l[i] = DEFAULT_LOG_REM[i]); i++ );
    }


    /* check for use of non-regular files */
    if ( stat ( log_new, &sbuf ) == 0 )
        if ( (sbuf.st_mode & S_IFREG) == 0 ) {
            dvc = 1;
            log_indx = -1;
        }


    /* on regular files, append log_indx to log name */
    if ( dvc == 0 ) {

        /* allow explicit specification of index number */
        if ( (i >= INDX) && (log_new_l[i-INDX] == '.') && 
        (log_new_l[i-(INDX-1)] >= '0') && (log_new_l[i-(INDX-1)] <= '9') ) {
            if ( strncmp ( log_new_l, attr_g.pathname, i ) == 0 ) return(0);
            log_indx = atoi(&log_new_l[i-(INDX-1)]);
            bcopy ( itoa(log_indx,INDX-1), &log_new_l[i-(INDX-1)], INDX-1 );
            log_new_l[i] = '\0';
        }

        /* set (or increment) log indx */
        else {
            if ( log_indx == -1 ) set_log_indx ( log_new_l );
            else {
                log_new_l[i++] = '.';
                log_indx = (log_indx+1)%MAX_LOG_INDX;
                bcopy ( itoa(log_indx,INDX-1), (char *)&log_new_l[i], INDX-1 );
                log_new_l[i+(INDX-1)] = '\0';
            }
        }
    }


    /* open new log */
    if ( (fdl = open ( log_new_l, O_RDWR|O_APPEND|O_CREAT, 0600 )) < 0 ) {
        sprintf ( buf_l, "could not set auditlog to %s", log_new_l );
        cwrite ( buf_l );
        return(-1);
    }

    /* put log change msg in old log, unless problem forced the switch */
    if ( severity == 0 && attr_g.pathval == LOCAL )
        audgen_l ( AUDIT_LOG_CHANGE, "auditlog change to file", log_new_l );


    /* switch logs */
    if ( attr_g.pathval == LOCAL ) {
        close ( fda );
        compress ( attr_g.pathname );
    }
    for ( i = 0; i < MAXPATHLEN && (attr_g.pathname[i] = log_new_l[i]); i++ );
    if ( attr_g.pathval == REMOTE ) {
        send ( inet_out, "BYE", MSG_OOB_SIZE, MSG_OOB );
        shutdown ( inet_out, 2 );
        close ( inet_out );
        inet_out = -1;
    }
    fda = fdl;
    sprintf ( buf_l, "setting auditlog to %s", attr_g.pathname );
    cwrite ( buf_l );
    attr_g.pathval = LOCAL;

    return(0);
}


/* switch to  next remote audit server */
log_chng_rmt ( nexthost, severity )
char *nexthost;
int severity;
{
    char hostname[MAXHOSTNAMELEN];
    struct hostent *hostent;
    char pathnam_l[MAXPATHLEN];
    char pathnam2_l[MAXPATHLEN];
    char buf_l[BUF_SIZ];            /* local buffer   */
    int i;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: log_chng_rmt" );
#endif /* __DEBUG3 */


    /* check for routing to self by comparing 'official' names of hosts */
    gethostname ( hostname, MAXHOSTNAMELEN );
    if ( hostent = gethostbyname(hostname) )
        strncpy ( pathnam_l, (char *)hostent->h_name, MAXPATHLEN );
    else return(-1);
    if ( hostent = gethostbyname(nexthost) )
        strncpy ( pathnam2_l, (char *)hostent->h_name, MAXPATHLEN );
    else return(-1);
    if ( strcmp ( pathnam_l, pathnam2_l ) == 0 ) return(-1);

    /* log change */
    if ( severity == 0 && attr_g.pathval == LOCAL )
        audgen_l ( AUDIT_LOG_CHANGE, "auditlog change to host", nexthost );

    /* set up new log */
    if ( attr_g.pathval == LOCAL ) {
        close ( fda );
        fda = -1;
        compress ( attr_g.pathname );
    }
    for ( i = 0; i < MAXPATHLEN && (attr_g.pathname[i] = nexthost[i]); i++ );
    attr_g.pathval = REMOTE;

#ifdef __KERBEROS
    /* must be able to xfer complete KERBEROS packet, so set nbytes */
    if ( attr_g.kerb_auth ) attr_l->nkbytes = KRB_SENDSIZ;
#endif /* __KERBEROS */


    sprintf ( buf_l, "routing audit data to %s", attr_g.pathname );
    cwrite ( buf_l );
}


/* return % free disk space for filesystem containing auditlog
   return 100 if filesystem is remote or if using device
*/
int monitor_space()
{
    struct stat sbuf;                       /* auditlog data  */
#ifdef __osf__
    struct statfs sfs;                      /* filesys data   */
#endif /* __osf__ */
#ifdef ultrix
    static struct fs_data fs_info;          /* filesys data   */
    int cntxt = 0;                          /* getmnt pointer */
#endif /* ultrix */
    char buf_l[BUF_SIZ];                    /* local buffer   */
    char path_l[MAXPATHLEN];                /* local pathname */
    int i, j;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: monitor_space" );
#endif /* __DEBUG3 */


    /* return 100 for remote filesystems and devices */
    if ( attr_g.pathval != LOCAL ) return(100);
    if ( stat ( attr_g.pathname, &sbuf ) == 0 ) {
        if ( (sbuf.st_mode & S_IFREG) == 0 ) return(100);
    }

    /* re-create auditlog if necessary */
    else if ( errno == ENOENT ) {                
        sprintf ( buf_l, "could not find %s", attr_g.pathname );
        cwrite ( buf_l );
        close ( fda );
        for ( j = 0; j < MAXPATHLEN-INDX && attr_g.pathname[j]; j++ );
        if ( log_indx == -1 ) set_log_indx ( attr_g.pathname );
        else {
            log_indx = (log_indx+1)%MAX_LOG_INDX;
            bcopy ( itoa(log_indx,INDX-1), &attr_g.pathname[j-(INDX-1)], INDX-1 );
            attr_g.pathname[j] = '\0';
        }
        sprintf ( buf_l, "setting auditlog to %s", attr_g.pathname );
        cwrite ( buf_l );
        if ( (fda = open ( attr_g.pathname, O_RDWR|O_APPEND|O_CREAT, 0600 )) < 0 )
            quit ( "open: auditlog", 0 );
        audgen_l ( AUDIT_LOG_CREAT, "re-created auditlog", attr_g.pathname );
    }


#ifdef __osf__
    /* get filesystem info */
    if ( statfs ( attr_g.pathname, &sfs, sizeof(struct statfs) ) == -1 ) {
        if ( errno != ENOENT ) {
            quit ( "statfs", 0 );
            return(0);
        }

        /* if not found; try parent directory */
        else {
            for ( i = 0; i < MAXPATHLEN && (path_l[i] = attr_g.pathname[i]); i++ );
            if ( i == MAXPATHLEN ) i--;
            for ( ; path_l[i] != '/' && i; i-- );
            path_l[i] = '\0';
            if ( statfs ( path_l, &sfs, sizeof(struct statfs) ) == -1 ) {
                quit ( "statfs", 0 );
                return(0);
            }
        }
    }

    /* return freespace; derived from df code */
    if ( sfs.f_blocks == 0 ) i = 100;
    else i = 100 - ( (sfs.f_blocks-sfs.f_bfree) * 100 /
    (sfs.f_blocks - (sfs.f_bfree - sfs.f_bavail)) );
#endif /* __osf__ */


#ifdef ultrix
    /* get filesystem info */
    if ( (i = getmnt ( &cntxt, &fs_info, sizeof(struct fs_data), STAT_ONE, attr_g.pathname )) != 1 ) {
        if ( errno != ENOENT ) {
            quit ( "getmnt", 0 );
            return(0);
        }

        /* if not found; try parent directory */
        else {
            for ( i = 0; i < MAXPATHLEN && (path_l[i] = attr_g.pathname[i]); i++ );
            if ( i == MAXPATHLEN ) i--;
            for ( ; path_l[i] != '/' && i; i-- );
            path_l[i] = '\0';
            cntxt = 0;
            if ( (i = getmnt ( &cntxt, &fs_info, sizeof(struct fs_data),
            STAT_ONE, path_l )) != 1 ) {
                quit ( "getmnt", 0 );
                return(0);
            }
        }
    }

    /* return freespace; derived from df code */
    if ( fs_info.fd_btot == 0 ) i = 100;
    else i = 100 - ( (fs_info.fd_btot-fs_info.fd_bfree) * 100 /
    (fs_info.fd_btot - (fs_info.fd_bfree - fs_info.fd_bfreen)) );
#endif /* ultrix */

    return(i);
}


/* open/close net_server auditd's receiving inet socket; return -1 on failure */
int net_serv ( toggle, sin_p )
int toggle;     /* !0 - open; 0 - close  */
int *sin_p;     /* inet socket ptr      */
{
    struct sockaddr_in si;      /* internet socket                  */
    struct servent *serventp;   /* service entry pointer            */
    int i = 1;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: net_serv" );
#endif /* __DEBUG3 */


    /* open net_server's receiving inet socket */
    if ( toggle ) {
        si.sin_family = AF_INET;
        if ( (serventp = getservbyname ( "auditd", "tcp" )) == (struct servent *)0 ) {
            cwrite ( "getservbyname: missing auditd/tcp entry in /etc/services" );
            return(-1);
        }
        else si.sin_port = serventp->s_port;
        si.sin_addr.s_addr = '\0';
        if ( (*sin_p = socket ( AF_INET, SOCK_STREAM, 0 )) < 0 ) {
            quit ( "socket: net server", 0 );
            return(-1);
        }
        if ( setsockopt ( *sin_p, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int) ) == -1 )
            quit ( "setsockopt(SO_REUSEADDR)", 0 );
        if ( bind (*sin_p, &si, sizeof(struct sockaddr_in)) < 0 ) {
            quit ( "bind: net server", 0 );
            close ( *sin_p );
            *sin_p = -1;
            return(-1);
        }
        if ( ioctl ( *sin_p, FIONBIO, &i ) < 0 ) {
            quit ( "ioctl: net server", 0 );
            close ( *sin_p );
            *sin_p = -1;
            return(-1);
        }
        listen ( *sin_p, 5 );
    }

    /* close receiving inet socket; tell connected auditd */
    else {
        send ( *sin_p, "BYE", MSG_OOB_SIZE, MSG_OOB );
        shutdown ( *sin_p, 2 );
        close ( *sin_p );
        *sin_p = -1;
    }
    return(0);
}


/* output audit data to log; return # bytes output */
int output_rec ( nbytes, aud_recp, override )
int nbytes;
char *aud_recp;
int override;                    /* override freespace check              */
{
    struct sockaddr_in ss;       /* inet socket address                   */
    struct servent *serventp;    /* service entry pointer                 */
    char pathname_l[MAXPATHLEN]; /* tmp copy of auditlog pathname         */
    char buf_l[BUF_SIZ];         /* local buffer                          */
    struct hostent *hp;          /* host entry                            */
    struct arp arp_l;            /* local audit record ptrs               */
    struct hndshk hndshk;        /* handshake between daemons             */
    static int suspend = 0;      /* suspend audit output                  */
    int lprt = IPPORT_RESERVED-1;/* top of reserved port space            */
    int bytes_out;
    char *ptr;
    int i, j;

#ifdef __KERBEROS
    struct sockaddr_in sl, sf;
    char krb_buf[KRB_RECVSIZ*1024]; /* must be > KRB_SENDSIZ; has KRB data   */
#endif /* __KERBEROS */

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: output_rec" );
#endif /* __DEBUG3 */


/* perform a log_chng() if unable to output data */
#define FAIL_OUTPUT(code,string,name) \
{ \
    if ( log_chng ( (char *)0, 1, LC_CHANGE ) == (char *)-1 ) cwrite ( "data lost" ); \
    else { \
        if ( code ) audgen_l ( code, string, name ); \
        nbytes = output_rec ( nbytes, aud_recp, 0 ); \
    } \
    shutdown ( inet_out, 2 ); \
    close ( inet_out ); \
    inet_out = -1; \
    return ( nbytes ); \
}


    /* sanity check */
    if ( nbytes < 0 ) {
        sprintf ( buf_l, "output_rec: nbytes = %d", nbytes );
        cwrite ( buf_l );
        return(0);
    }


    /* send audit data over inet */
    if ( attr_g.pathval == REMOTE ) {

        /* open a connection to the net_server auditd */
        if ( inet_out == -1 ) {

            /* get a reserved port */
            if ( (inet_out = rresvport(&lprt)) < 0 ) quit ( "socket: net_server", 0 );

            /* build socket struct */
            ss.sin_family = AF_INET;
            if ( (serventp = getservbyname ( "auditd", "tcp" )) == (struct servent *)0 ) 
                cwrite ( "getservbyname: missing auditd/tcp entry in /etc/services" );
            else ss.sin_port = serventp->s_port;
            if ( (hp = gethostbyname(attr_g.pathname)) == (struct hostent *)0 ) {
                cwrite ( "invalid host" );
                FAIL_OUTPUT ( 0, (char *)0, (char *)0 );
            }
            bcopy ((char *)hp->h_addr, &ss.sin_addr, hp->h_length );

            /* ping remote host */
            if ( ping(attr_g.pathname) ) {
                cwrite ( "unreachable host" );
                FAIL_OUTPUT ( 0, (char *)0, (char *)0 );
            }

            /* make a non-blocking connection */
            if ( connect ( inet_out, &ss, sizeof(struct sockaddr_in) ) < 0 ) {
                quit ( "connect", 0 );
                FAIL_OUTPUT ( 0, (char *)0, (char *)0 );
            }
            i = 1;
            if ( ioctl ( inet_out, FIONBIO, &i ) < 0 ) quit ( "ioctl: FIONBIO", 0 );

            /* send hostname to server for check against talking to self
               send kerberos status
            */
            if ( gethostname ( hndshk.hostname, MAXHOSTNAMELEN ) )
                quit ( "hostname", 0 );
            hndshk.kerb_flag = attr_g.kerb_auth;
            if ( write ( inet_out, &hndshk, sizeof(struct hndshk) ) != sizeof(struct hndshk) )
                quit ( "write", 0 );

#ifdef __KERBEROS
            /* krb_sendauth to establish trusted connection */
            if ( attr_g.kerb_auth )
                if ( kerb_ops ( KRB_SENDAUTH, inet_out, &ss ) == -1 )
                    FAIL_OUTPUT ( 0, (char *)0, (char *)0 );
#endif /* __KERBEROS */
        }

        /* set ptr to data, and size of data block */
        ptr = aud_recp;
        j = nbytes;

#ifdef __KERBEROS
        /* encapsulate data in KERBEROS packet */
        if ( attr_g.kerb_auth ) {
            i = sizeof(struct sockaddr_in);
            if ( getpeername ( inet_out, &sf, &i ) ) {
                quit ( "getpeername (krb)", 0 );
                for ( i = 0; i < MAXPATHLEN && (pathname_l[i] = attr_g.pathname[i]); i++ );
                FAIL_OUTPUT ( AUDIT_XMIT_FAIL, "failed to send data to host", pathname_l );
            }
            i = sizeof(struct sockaddr_in);
            if ( getsockname ( inet_out, &sl, &i ) )
                quit ( "getsockname (krb)", 0 );
            if ( (j = krb_mk_safe ( aud_recp, krb_buf, nbytes, cred.session, &sl, &sf )) < nbytes ) {
                sprintf ( buf_l, "krb_mk_safe returned %d; must output %d", i, nbytes );
                cwrite ( buf_l );
                for ( i = 0; i < MAXPATHLEN && (pathname_l[i] = attr_g.pathname[i]); i++ );
                FAIL_OUTPUT ( AUDIT_XMIT_FAIL, "failed to send data to host", pathname_l );
            }
            else ptr = krb_buf;
        }
#endif /* __KERBEROS */

        /* output data to net_server auditd; with KERBEROS, output all data */
        i = 0;
        do {
            if ( (bytes_out = write ( inet_out, &ptr[i], j-i )) < 0 ) {
                /* if EWOULDBLOCK, use ping just to check the connection
                   don't want to break connection due to busy network
                */
                if ( errno == EWOULDBLOCK )
                    if ( ping(attr_g.pathname) == 0 ) continue;
                sprintf ( buf_l, "could not send to %s", attr_g.pathname );
                cwrite ( buf_l );
                for ( i = 0; i < MAXPATHLEN && (pathname_l[i] = attr_g.pathname[i]); i++ );
                FAIL_OUTPUT ( AUDIT_XMIT_FAIL, "failed to send data to host", pathname_l );
            }
            else i += bytes_out;
#ifdef __DEBUG3
            if ( attr_g.flag & ATTR_DEBUG ) {
                sprintf ( buf_l, "output_rec: output %d bytes", bytes_out );
                cwrite ( buf_l );
            }
#endif /* __DEBUG3 */
        } while ( (i < j) && attr_g.kerb_auth );
    }


    /* output data into a single local file */
    else if ( attr_g.pathval == LOCAL ) {

        arp_l.aud_rec_b = aud_recp;
        arp_l.aud_rec = nbytes + aud_recp;

        /* monitor freespace on partition collecting auditlog */
        if ( override == 0 ) {
            i = monitor_space();
            if ( i <= attr_g.freepct && suspend == 0 && (attr_g.flag&ATTR_KILL) == 0 ) {
                sprintf ( buf_l, "filesystem containing %s @ %d%% capacity", attr_g.pathname, 100-i );
                cwrite ( buf_l );
                if ( overflow ( attr_g.overflow, &arp_l ) == SUSPEND ) {
                    audgen_l ( AUDIT_SUSPEND, "audit data output suspended", "" );
                    suspend = SUSPEND;
                    cwrite ( "audit data output suspended" );
                    bytes_out = nbytes;
                }
            }
            else if ( i > attr_g.freepct && suspend == SUSPEND ) {
                suspend = 0;
                audgen_l ( AUDIT_SUSPEND, "audit data output resumed", "" );
                cwrite ( "audit data output resumed" );
            }
        }

        /* write data */
        if ( fda == -1 )
            if ( log_chng ( (char *)0, 1, LC_CHANGE ) == (char *)-1 ) {
                cwrite ( "data lost" );
                cw_switch = 0;
                overflow ( attr_g.overflow, &arp_l );
            }
        if ( suspend != SUSPEND && (bytes_out = write ( fda, aud_recp, nbytes )) < 0 ) {
            sprintf ( buf_l, "could not write to %s", attr_g.pathname );
            cwrite ( buf_l );
            if ( log_chng ( (char *)0, 1, LC_CHANGE ) == (char *)-1 ) {
                cwrite ( "data lost" );
                cw_switch = 0;
                overflow ( attr_g.overflow, &arp_l );
            }
        }
        else cw_switch = 1;
        if ( nobuf ) fsync ( fda );
#ifdef __DEBUG3
        if ( attr_g.flag & ATTR_DEBUG ) {
            sprintf ( buf_l, "output_rec: output %d bytes", bytes_out );
            cwrite ( buf_l );
        }
#endif /* __DEBUG3 */
    }


#ifndef __OSF_MMAP
    /* shift any data which wasn't output */
    if ( bytes_out != nbytes && bytes_out > 0 )
        bcopy ( &aud_recp[bytes_out], aud_recp, nbytes-bytes_out );
#endif /* __OSF_MMAP */
    return ( bytes_out < 0 ? 0 : bytes_out );
}


/* take action when threshold exceeded; return SUSPEND to suspend auditing */
int overflow ( action, arp )
int action;         /* action to take on overflow   */
struct arp *arp;    /* audit data ptrs              */
{
    char buf_l[BUF_SIZ];    /* local buffer         */
    int  fdl;               /* local logfile desc   */
    static int recurs = 0;  /* recursion latch      */
    char *ptr;
    int retval = 0;
    int i;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: overflow" );
#endif /* __DEBUG3 */


    if ( recurs++ ) return(0);

    switch ( action ) {

    case OA_NULL: /* no action specified */
        break;


    case OA_CD: /* change to next dir in AUDITD_LOGLOC */
#ifdef __OSF_MMAP
        if ( attr_g.pid ) {
#endif /* __OSF_MMAP */
            i = output_rec ( arp->aud_rec - arp->aud_rec_b, arp->aud_rec_b, OVERRIDE );
            arp->aud_rec -= i;
#ifdef __OSF_MMAP
        }
#endif /* __OSF_MMAP */
        /* do log_chng only if a "next dir" exists */
        ptr = log_chng ( (char *)0, 0, LC_GET_NEXTDIR );
        if ( ptr && *ptr )
            if ( log_chng ( (char *)0, 0, LC_CHANGE ) == (char *)-1 )
                cwrite ( "no alternate auditlog set" );
        break;


    case OA_SUSP:   /* suspend audit until space becomes available */
        retval = SUSPEND;
        break;


    case OA_OVERWRITE:  /* overwrite current auditlog */
        if ( attr_g.pathval != LOCAL ) break;
        if ( fda >= 0 )
            if ( ftruncate ( fda, 0 ) == -1 )
                quit ( "ftruncate: overwrite auditlog", 0 );
        if ( (fdl = open ( attr_g.pathname, O_RDWR|O_APPEND|O_CREAT, 0600 )) < 0 ) {
            quit ( "open: overwrite auditlog", 0 );
            break;
        }
        close ( fda );
        fda = fdl;
#ifndef __OSF_MMAP
        i = output_rec ( arp->aud_rec - arp->aud_rec_b, arp->aud_rec_b, OVERRIDE );
        arp->aud_rec -= i;
#endif /* __OSF_MMAP */
        audgen_l ( AUDIT_LOG_OVERWRITE, "overwriting auditlog", attr_g.pathname );
        sprintf ( buf_l, "overwriting auditlog %s", attr_g.pathname );
        cwrite ( buf_l );
        break;


    case OA_KILL:   /* terminate auditing */
        attr_g.flag |= ATTR_KILL;
        if ( (attr_g.flag&ATTR_KILL) && attr_g.pid == 0 ) {
            if ( audcntl ( SET_AUDSWITCH, (char *)0, 0, 0, 0, 0 ) == -1 )
                if ( errno != ENOSYS ) quit ( "audcntl", 1 );
        }
        break;


    case OA_HALT:   /* halt the system */
#ifdef __OSF_MMAP
        if ( attr_g.pid ) {
#endif /* __OSF_MMAP */
            i = output_rec ( arp->aud_rec - arp->aud_rec_b, arp->aud_rec_b, OVERRIDE );
            arp->aud_rec -= i;
#ifdef __OSF_MMAP
        }
#endif /* __OSF_MMAP */
        cwrite ( "overflow condition -> shutdown" );
        audgen_l ( AUDIT_REBOOT, "audit overflow -> shutdown", "" );
        sync();                
        if ( fda > 0 ) fsync ( fda );
        if ( reboot ( RB_HALT ) == -1 ) quit ( "reboot", 0 );
        audgen_l ( AUDIT_REBOOT, "audit overflow -> shutdown", "failed" );
        break;

    }

    recurs = 0;
    return ( retval );
}


/* CODE TAKEN FROM PING(8) */
/* return 0 only if remote host pinged */
int ping ( who )
char *who;
{
	int pingpid;             /* child ping process  */
	struct sockaddr_in from;
	struct sockaddr whereto;/* Who to ping */
	struct sockaddr_in *to = (struct sockaddr_in *)&whereto;
	struct protoent *proto;
	struct hostent *hp;	/* Pointer to host info */
	int fromlen, size, timeout;
	union wait status;
	u_char packet[MAXPACKET];
	pid_t ident;
	int s;			/* Socket file descriptor */

	bzero( &whereto, sizeof(struct sockaddr) );

	to->sin_family = AF_INET;
	hp = gethostbyname(who);
	if (hp) {
		to->sin_family = hp->h_addrtype;
		bcopy(hp->h_addr, &to->sin_addr, hp->h_length);
	} else return (-1);

	ident = getpid() & 0xFFFF;

	if ((proto = getprotobyname("icmp")) == NULL) return (-2);
	if ((s = socket(AF_INET, SOCK_RAW, proto->p_proto)) < 0) return (-3);

	timeout = attr_g.timeout;

        if ((pingpid = fork()) < 0) return (-4);
        if (pingpid != 0) {         /* parent */
                while (1) {
			if ( pinger(&whereto,ident,s) == -1) {
                                close(s);
                                return(-5);
                        }
                        sleep(1);
                        if (wait3(&status, WNOHANG, 0) == pingpid) {
                                close(s);
                                return(status.w_retcode);
                        }
                }
        }

        if (pingpid == 0) {         /* child */
                alarm(timeout);
                signal(SIGALRM, ping_noanswer);
                while (1) {
			int len = sizeof(packet);
                        fromlen = sizeof(from);
			if ((size = recvfrom(s, packet, len, 0, &from, &fromlen)) < 0)
                                continue;
			if ( ping_pr_pack(packet,size,&from,ident) == 1 )
                        	_exit(0);
                }
        }

}

/*
 * 			P I N G E R
 * 
 * Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first 8 bytes
 * of the data portion are used to hold a UNIX "timeval" struct in VAX
 * byte-order, to compute the round-trip time.
 */
pinger(whereto,ident,s)
struct sockaddr *whereto;
pid_t ident;
int s;			/* Socket file descriptor */
{
	static u_char outpack[MAXPACKET];
	struct icmp *icp = (struct icmp *) outpack;
	int i, cc;
	struct timeval *tp = (struct timeval *) &outpack[8];
	u_char *datap = &outpack[8+sizeof(struct timeval)];
	static int ntransmitted = 0; /* sequence # for outbound packets = #sent */
	struct timezone tz;
        int datalen = 64-8;

	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_seq = ntransmitted++;
	icp->icmp_id = ident;		/* ID */

	cc = datalen+8;			/* skips ICMP portion */

	gettimeofday( tp, &tz );

	for( i=8; i<datalen; i++)	/* skip 8 for time */
		*datap++ = i;

	/* Compute ICMP checksum here */
	icp->icmp_cksum = ping_in_cksum( icp, cc );

	/* cc = sendto(s, msg, len, flags, to, tolen) */
	i = sendto( s, outpack, cc, 0, whereto, sizeof(struct sockaddr) );

	if( i < 0 || i != cc )
		return(-1);
	else return(0);
}

/*
 *			P R _ P A C K
 *
 * Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
int ping_pr_pack( buf, cc, from, ident )
char *buf;
int cc;
struct sockaddr_in *from;
pid_t ident;
{
	struct ip *ip;
	register struct icmp *icp;
	int hlen;

	from->sin_addr.s_addr = ntohl( from->sin_addr.s_addr );

	ip = (struct ip *) buf;
	hlen = ip->ip_hl << 2;
	if (cc < hlen + ICMP_MINLEN)
		return(0);
	cc -= hlen;
	icp = (struct icmp *)(buf + hlen);
	if( icp->icmp_id != ident )
		return(0);			/* 'Twas not our ECHO */

	return(1);
}

/*
 *			I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers (C Version)
 *
 */
ping_in_cksum(addr, len)
u_short *addr;
int len;
{
	register int nleft = len;
	register u_short *w = addr;
	register u_short answer;
	register int sum = 0;
 	u_short odd_byte = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while( nleft > 1 )  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if( nleft == 1 ) {
               *(&odd_byte) = *w;
               sum += odd_byte;
	}

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}

int ping_noanswer()
{
        exit(-10);
}


/* qualify name with respect to current directory
   'name' arg must be of MAXPATHLEN bytes
*/
int qual_name ( name )
char *name;
{
    char path_l[MAXPATHLEN];
    int i, j;

    /* get working directory */
    if ( getwd ( path_l ) == 0 ) {
        quit ( "getwd", 0 );
        return(-1);
    }

    /* qualify name (relative) with current directory */
    if ( name[0] && name[0] != '/' ) {
        for ( i = 0; name[i]; i++ );
        for ( j = 0; path_l[j]; j++ );
        if ( i+j >= MAXPATHLEN-1 ) return(-1);
        bcopy ( name, &name[j+1], i+1 );
        name[j] = '/';
        bcopy ( path_l, name, j );
    }

    return(0);
}


/* print error message on console; exit if severity set
   do not call quit() unless audit file descriptor is set
*/
quit ( string, severity )
char *string;
{
    extern int errno;
    extern int sys_nerr;
    extern char *sys_errlist[];
    char buf_l[BUF_SIZ];
    int i;

    /* output error message */
    if ( errno > 0 ) {
        if ( errno < sys_nerr ) sprintf ( buf_l, "auditd (%d): %s: %s", getpid(), string, sys_errlist[errno] );
        else sprintf ( buf_l, "auditd (%d): %s: error #%d", getpid(), string, errno );
        cwrite ( buf_l );
    }

    /* if severe, turn off audit & /dev/audit overflow msgs, remove sockets, make auditlog entry, exit */
    if ( severity ) {
        audcntl ( SET_AUDSWITCH, (char *)0, 0, 0, 0, 0 );
        unlink ( ADMIN_SOCK );
        for ( i = 1; i <= n_conn; i++ )
            unlink ( clientsockname(i) );

        /* kill child audit daemons */
        for ( i = 1; i <= n_conn; i++ )
            if ( child[i] != -1 ) kill ( child[i], SIGALRM );

        audgen_l ( AUDIT_DAEMON_EXIT, "audit daemon exiting", "" );
        cwrite ( "AUDIT DAEMON EXITING ABNORMALLY" );
        exit ( severity );
    }
}


/* convert time string into # seconds (scaled syntax) */
time_t scale_tm ( tm_str )
char *tm_str;
{
    time_t ret_tm, t;
    char c;

    for ( ret_tm = 0, c = *tm_str; *tm_str; ret_tm += t ) {

        for ( t = 0; isdigit(c = *tm_str++); t = t*10 + c-'0' );

        switch ( c ) {
            case 'W':   case 'w':   t *= 7;     /* weeks     */
            case 'D':   case 'd':   t *= 24;    /* days      */
            case 'H':   case 'h':   t *= 60;    /* hours     */
            case 'M':   case 'm':   t *= 60;    /* minutes   */
            case 'S':   case 's':   break;      /* seconds   */
            default :   return ( (time_t)-1 );  /* bad input */
        }

    }

    return  ( ret_tm );
}


/* perform server function */
server()
{
    int sd1;                    /* server-client socket descriptor  */
    int sd2;                    /* /dev/audit descriptor            */
    int sd3 = -1;               /* server-inet socket descriptor    */
    struct sockaddr ss;         /* admin socket                     */
    char path_l[MAXPATHLEN];    /* directory                        */
    pid_t pid;
#ifdef __DEBUG3
    char buf_l[BUF_SIZ];        /* local buffer                     */
#endif /* __DEBUG3 */
    int i, j;

    /* 1) turn off auditing for auditd */
    if ( audcntl ( SET_PROC_ACNTL, (char *)0, 0, AUDIT_OFF, 0, 0 ) == -1 )
        if ( errno != ENOSYS ) quit ( "audcntl", 1 );


    /* 2a) qualify names relative to current directory */
    if ( *attr_g.pathname && attr_g.pathval == LOCAL ) qual_name ( attr_g.pathname );
    if ( *attr_g.console ) qual_name ( attr_g.console );

    /* 2b) initialize environment */
    umask ( 077 );
    for ( i = 1; i <= NSIG; i++ ) signal ( i, SIG_IGN );
    signal ( SIGIO, SIG_DFL );
    signal ( SIGSEGV, SIG_DFL );
    signal ( SIGTERM, sig_hndlr );
    signal ( SIGALRM, sig_hndlr );
    signal ( SIGHUP, sig_hndlr2 );
#ifdef __DEBUG3
    signal ( SIGQUIT, SIG_DFL );
#endif /* __DEBUG3 */
    n_conn = getdtablesize()-5 < MAXCLIENTS-1 ? getdtablesize()-5 : MAXCLIENTS-1;
    if ( (child = (pid_t *)malloc ((n_conn+1) * sizeof(pid_t)) ) == NULL ) {
        n_conn = 0;
        cwrite ( "-- insufficient mem for clients --" );
    }
    else for ( i = 0; i <= n_conn; i++ ) child[i] = -1;
    if ( chdir ( "/" ) ) quit ( "chdir to /", 1 );
    mkdir ( AUDITD_DIR, 0700 );
    if ( chdir ( AUDITD_DIR ) ) quit ( "chdir", 1 );


    /* 3) initialize parameters */
    if ( attr_g.freepct == -1 ) attr_g.freepct = DEFAULT_PCT;
    if ( attr_g.nkbytes < MIN_AUD_BUF ) {
#ifdef __osf__
        if ( (i = audcntl ( GET_AUDSIZE, (char *)0, 0, 0, 0, 0 )) != -1 )
            attr_g.nkbytes = i/1024;
#endif /* __osf__ */
#ifdef ultrix
        if ( (i = getkval ( X_AUDSIZE )) != -1 )
            attr_g.nkbytes = i > AUD_BUF_SIZ/1024 ? i*4 : AUD_BUF_SIZ/1024*4;
#endif /* ultrix */
        if ( attr_g.nkbytes < MIN_AUD_BUF ) attr_g.nkbytes = MIN_AUD_BUF;
    }
    if ( attr_g.overflow == 0 ) attr_g.overflow = DEFAULT_ACTION;
    if ( *attr_g.pathname == '\0' ) for ( i = 0; attr_g.pathname[i] = DEFAULT_LOG[i]; i++ );
    if ( attr_g.pathval == '\0' ) attr_g.pathval = LOCAL;
    if ( attr_g.timeout == -1 ) attr_g.timeout = DEFTIMEOUT;
    attr_g.pid = 0;
    if ( *attr_g.console == '\0' ) for ( i = 0; attr_g.console[i] = "/dev/console"[i]; i++ );
    if ( attr_g.flush_freq == -1 ) attr_g.flush_freq = 0;


    /* 4) find first free file in sequence */
    if ( attr_g.pathval == LOCAL ) set_log_indx ( attr_g.pathname );


    /* 5) command line requests */
    if ( attr_g.flag & ATTR_QUERY ) {
        for ( i = 0; attr_g.pathname[i]; i++ );
        write ( 1, attr_g.pathname, i );
        write ( 1, "\n", 1 );
    }
    if ( attr_g.flag & ATTR_KILL ) attr_g.flag &= ~ATTR_KILL;
    if ( attr_g.flag & ATTR_SETLOC )
        if ( log_chng ( (char *)0, 0, LC_SETLOC ) == (char *)-1 )
            write ( 1, "-- unable to set dir list --\n", 29 );
    if ( attr_g.flag & ATTR_SHOW ) show_stat ( 1, -1 );
    if ( attr_g.flag & ATTR_HELP ) show_help ( 1 );


    /* 6) run as daemon */
#ifndef __DEBUG1
    if ( (pid = fork()) == 0 ) {

        /* 6a) disassociate from controlling tty and process group */
        if ( setpgrp ( 0, getpid() ) == -1 ) quit ( "setpgrp", 0 );
        if ( (i = open ( "/dev/tty", 2 )) >= 0 ) {
            ioctl ( i, TIOCNOTTY, 0 );
            close(i);
        }
        for ( i = getdtablesize()-1; i >= 0; i-- ) close(i);
#else
    if (1) {
#endif /* __DEBUG1 */

        /* 6b) open auditlog */
        if ( attr_g.pathval == LOCAL )
            if ( (fda = open ( attr_g.pathname, O_RDWR|O_APPEND|O_CREAT, 0600 )) < 0 ) {
                quit ( "open: auditlog", 0 );
                exit(1);
            }

        /* 6c) run daemon at a favorable priority */
        if ( setpriority ( PRIO_PROCESS, getpid(), PRIORITY ) == -1 )
            quit ( "set_priority", 0 );


        /* 6d) open /dev/audit for read access,
           non-fatal iff ENOENT or ENODEV to allow running on a system
           which has no audit subsystem
        */
        if ( (sd2 = open ( "/dev/audit", 0 )) < 0 ) {
            if ( errno == ENOENT || errno == ENODEV ) 
                quit ( "open /dev/audit", 0 );
            else quit ( "open /dev/audit", 1 );
        }


        /* 6e) create socket for client; bind, listen */
        if ( (sd1 = socket ( AF_UNIX, SOCK_STREAM, 0 )) < 0 ) quit ( "socket: server", 1 );
        ss.sa_family = AF_UNIX;
        bcopy ( ADMIN_SOCK, ss.sa_data, sizeof ADMIN_SOCK );
        if ( bind (sd1, &ss, sizeof(struct sockaddr)) < 0 ) quit ( "bind: server", 1 );
        if ( listen ( sd1, 5 ) < 0 ) quit ( "listen: server", 1 );

        /* 6f) open socket over which to receive connections from other daemons */
        if ( attr_g.flag & ATTR_NETSRV )
            if ( net_serv ( 1, &sd3 ) == -1 ) attr_g.flag &= ~ATTR_NETSRV;


        /* 6g) turns on system audit mechanism */
        if ( sd2 >= 0 ) {
            if ( audcntl ( SET_AUDSWITCH, (char *)0, 0, 1, 0, 0 ) == -1 )
                if ( errno != ENOSYS ) quit ( "audcntl", 1 );
            audgen_l ( AUDIT_START, "audit subsystem on", "" );
            cwrite ( "AUDIT DAEMON STARTING" );
        }
        else {
            audgen_l ( AUDIT_START, "audit server enabled", "" );
            cwrite ( "AUDIT DAEMON STARTING (network only)" );
        }


        /* 6h) read the data and perform the housekeeping tasks */
#ifdef __DEBUG3
        if ( attr_g.flag & ATTR_DEBUG ) {
            sprintf ( buf_l, "debug: server(b): sd1 = %d  sd2 = %d  sd3 = %d\n",
                sd1, sd2, sd3 );
            cwrite ( buf_l );
        }
#endif /* __DEBUG3 */
        chk_input ( sd1, sd2, &sd3, &ss );
        /* doesn't return until time to shut down audit */


        /* 6i) kill child daemons */
        for ( i = 1; i <= n_conn && attr_g.pid == 0; i++ )
            if ( child[i] != -1 ) kill ( child[i], SIGALRM );

        /* send "BYE" message to inet connected daemons */
        net_serv ( 0, &sd3 );
        if ( attr_g.pathval == REMOTE ) {
            send ( inet_out, "BYE", MSG_OOB_SIZE, MSG_OOB );
            shutdown ( inet_out, 2 );
            close ( inet_out );
        }


        /* 6j) compress auditlog */
        if ( attr_g.pathval == LOCAL && sigtermed == 0 ) {
            close(fda);
            compress ( attr_g.pathname );
        }

        /* 6k) close old socket descriptors */
        close(sd1);
        close(sd2);
        close(sd3);
        unlink ( clientsockname(attr_g.pid) );

        /* 6l) for master auditd... */
        if ( attr_g.pid == 0 ) {
            /* remove socket from filesystem */
            unlink ( ADMIN_SOCK );
            for ( i = 1; i <= n_conn; i++ )
                unlink ( clientsockname(i) );
            rmdir ( AUDITD_DIR );

            /* turn off system audswitch */
            if ( audcntl ( SET_AUDSWITCH, (char *)0, 0, 0, 0, 0 ) == -1 )
                if ( errno != ENOSYS ) quit ( "audcntl", 1 );
            cwrite ( "AUDIT DAEMON EXITING" );
        }
    }

    else if ( pid == -1 ) quit ( "fork", 1 );
}


/* update log_indx and pathname to first unused member in sequence */
set_log_indx ( pathname )
char *pathname;
{
    struct stat statbuf;
    extern int errno;
    char path_l[MAXPATHLEN];
    int i;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: set_log_indx" );
#endif /* __DEBUG3 */


    /* ignore non-regular files */
    if ( stat ( pathname, &statbuf ) == 0 )
        if ( (statbuf.st_mode & S_IFREG) == 0 ) return;

    for ( i = 0; i < MAXPATHLEN && pathname[i]; i++ );
    if ( i > INDX && pathname[i-INDX] == '.' && pathname[i-(INDX-1)] >= '0' && pathname[i-(INDX-1)] <= '9' )
        log_indx = atoi(&pathname[i-(INDX-1)]);
    else {
        pathname[i] = '.';
        for ( log_indx = 0; log_indx < MAX_LOG_INDX; log_indx++ ) {
            bcopy ( itoa(log_indx,INDX-1), &pathname[i+1], INDX-1 );
            pathname[i+INDX] = '\0';
            if ( (stat ( pathname, &statbuf ) == -1) && (errno == ENOENT) ) {
                bcopy ( pathname, path_l, i+INDX );
                bcopy ( ".Z", &path_l[i+INDX], 3 );
                if ( (stat ( path_l, &statbuf ) == -1) && (errno == ENOENT) )
                    break;
            }
        }
        if ( log_indx == MAX_LOG_INDX ) log_indx = 0;
    }
}


/* print help menu */
show_help ( ns )
int ns;
{
    char buf_l[BUF_SIZ];
    int i, j;

    write ( ns, "\nAudit data and msgs:\n", 22 );
    sprintf ( buf_l, "%s\n%s\n%s\n",
    "  -l `name`     Filename or hostname: to receive data",
    "  -c `name`     Pathname (device or file) to receive auditd console messages",
    "  -q            Query server for auditlog pathname/hostname" );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );


    write ( ns, "\nauditd control:\n", 17 );
    sprintf ( buf_l, "%s\n%s\n%s\n%s\n%s\n%s\n",
    "  -d [freq]     Dump auditlog buffer [set frequency]",
    "  -k            Kill audit daemon",
    "  -p `id`       Id # of daemon to receive command",
    "  -r            Reset auditlog locations according to '/etc/sec/auditd_loc'",
    "  -w            Show status",
    "  -x            Increment log index (switch auditlog)" );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );

#ifdef __DEBUG3
    sprintf ( buf_l, "  -?            Debug flag -- routine trace (toggle)\n" );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
#endif /* __DEBUG3 */
#ifndef __OSF_MMAP
    sprintf ( buf_l, "  -n            # kbytes for auditd buffer size\n" );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
#endif /* __OSF_MMAP */


    write ( ns, "\nNetwork:\n", 10 );
#ifdef __KERBEROS
    sprintf ( buf_l, "  -a            Kerberos authentication (toggle)\n" );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
#endif /* __KERBEROS */
#ifdef __OSF_MMAP
    sprintf ( buf_l, "  -n `#`        # kbytes for auditd buffer size (from network)\n" );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
#endif /* __OSF_MMAP */

    sprintf ( buf_l, "%s\n%s\n",
    "  -s            Network audit server status (toggle)",
    "  -t `#`        Timeout value (sec) for establishing remote connection" );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );


    write ( ns, "\nOverflow control:\n", 19 );
    sprintf ( buf_l, "%s\n%s",
    "  -f `#`        Min percent free space on which overflow condition triggered",
    "  -o `action`   Action to take on overflow condition; `action` is one of:\n\
                 " );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
    for ( j = 1; j < N_OVERFLOW_OPTS; j++ ) {
        sprintf ( buf_l, "'%s'%c ", overflow_act[j].cmnd,
            j < N_OVERFLOW_OPTS-1 ? ',' : ' ' );
        for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
    }
    write ( ns, "\n", 1 );
}


/* pass back status information to requesting (AF_UNIX) client */
show_stat ( ns, sd3 )
int ns;     /* client socket descriptor  */
int sd3;    /* remote network connection */
{
    struct sockaddr_in peername;
    int namelen = sizeof(peername);
    struct audit_attr attr_l;
    struct sockaddr sl;
    char buf_l[BUF_SIZ];
    char *ptr;
    int i, j;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: show_stat" );
#endif /* __DEBUG3 */

    /* server... */
    for ( i = 1; attr_g.pid == 0 && i <= n_conn; i++ )
        if ( child[i] != -1 ) {
            sprintf ( buf_l, "\nMASTER AUDIT DAEMON SERVER:\n" );
            for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
            break;
        }

    /* client... */
    if ( attr_g.pid > 0 ) {
        write ( ns, "\n--------\n", 10 );
        if ( getpeername ( sd3, &peername, &namelen ) == 0 ) {
            ptr = gethost_l(peername.sin_addr.s_addr);
            sprintf ( buf_l, "AUDIT DAEMON #%d SERVING %s:\n", attr_g.pid, ptr );
        }
        else sprintf ( buf_l, "AUDIT DAEMON #%d:\n", attr_g.pid );
        for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
    }


    /* ...status... */

    write ( ns, "\nAudit data and msgs:\n", 22 );
    sprintf ( buf_l, "  -l) audit data destination                 = %s%s\n",
        attr_g.pathname, attr_g.pathval == LOCAL ? "" : ":" );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );

    sprintf ( buf_l, "  -c) audit console messages                 = %s\n",
        attr_g.console );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );

    if ( attr_g.flush_freq > 0 ) {
        sprintf ( buf_l, "  -d) audit data dump frequency              = %s\n",
            attr_g.flush_arg );
        for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
    }

#ifdef __DEBUG3
    sprintf ( buf_l, "  -?) debug flag -- routine trace (toggle)   = %s\n",
        (attr_g.flag & ATTR_DEBUG) ? "on" : "off" );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
#endif /* __DEBUG3 */

#ifndef __OSF_MMAP
#ifdef __DEBUG3
    if ( attr_g.pid == 0 || attr_g.kerb_auth == 0 ) {
#else
    if ( nobuf == 0 && (attr_g.pid == 0 || attr_g.kerb_auth == 0) ) {
#endif /* __DEBUG3 */
        sprintf ( buf_l, "  -n) buffer size                            = %d kbytes\n",
            attr_g.nkbytes );
        for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
    }
#endif /* __OSF_MMAP */


    write ( ns, "\nNetwork:\n", 10 );
#ifdef __OSF_MMAP
#ifdef __DEBUG3
    if ( attr_g.pid == 0 || attr_g.kerb_auth == 0 ) {
#else
    if ( nobuf == 0 && (attr_g.pid == 0 || attr_g.kerb_auth == 0) ) {
#endif /* __DEBUG3 */
        sprintf ( buf_l, "  -n) buffer size (network only)             = %d kbytes\n",
            attr_g.nkbytes );
        for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
    }
#endif /* __OSF_MMAP */

    if ( attr_g.pid == 0 ) {
        sprintf ( buf_l, "  -s) network audit server status (toggle)   = %s\n",
            (attr_g.flag & ATTR_NETSRV) ? "on" : "off" );
        for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
    }

    sprintf ( buf_l, "  -t) connection timeout value (sec)         = %d\n",
        attr_g.timeout );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );

#ifdef __KERBEROS
    sprintf ( buf_l, "  -a) kerberos authentication (toggle)       = %s\n",
        attr_g.kerb_auth ? "on" : "off" );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
#endif /* __KERBEROS */


    write ( ns, "\nOverflow control:\n", 19 );
    sprintf ( buf_l, "  -f) %% free space before overflow condition = %d\n",
        attr_g.freepct );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );

    sprintf ( buf_l, "  -o) action to take on overflow             = %s\n",
        overflow_act[attr_g.overflow].cmnd_descr );
    for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );

    for ( j = 0; (ptr = log_chng ( (char *)0, 0, LC_GET_NEXTDIR )) && *ptr; j++ ) {
        if ( j == 0 ) sprintf ( buf_l, "      alternate auditlog locations           = %s\n", ptr );
        else sprintf ( buf_l, "                                               %s\n", ptr );
        for ( i = 0; buf_l[i]; i++ );   write ( ns, buf_l, i );
    }

    /* pass request for status down to all child auditd's serving inet clients */
    for ( i = 1; attr_g.pid == 0 && i <= n_conn; i++ )
        if ( child[i] != -1 ) {
            sl.sa_family = AF_UNIX;
            for ( j = 0; sl.sa_data[j] = CHILD_SOCK[j]; j++ );
            bcopy ( itoa(i,MAXCL_ITOA), &sl.sa_data[j], MAXCL_ITOA );
            sl.sa_data[j+MAXCL_ITOA] = '\0';
            init_attr ( &attr_l );
            attr_l.flag |= ATTR_SHOW;
            client ( &attr_l, &sl, ns, attr_g.timeout );
        }
}


/* kill daemon gracefully */
sig_hndlr ( sig )
int sig;
{
    attr_g.flag |= ATTR_KILL;
    if ( attr_g.pid == 0 ) {
        if ( audcntl ( SET_AUDSWITCH, (char *)0, 0, 0, 0, 0 ) == -1 )
            if ( errno != ENOSYS ) quit ( "audcntl", 1 );
    }
    if ( sig == SIGTERM ) sigtermed = 1;
}

/* caught HUP; restart chk_input loop */
sig_hndlr2()
{
    longjmp ( env, 1 );
}


/* fork auditd server to handle client auditd */
spawn_child ( conn, sd1_p, sd2_p, sd3_p, arp_p, conn_indx )
int *conn;                                  /* accepted connection  */
int *sd1_p;                                 /* local client socket  */
int *sd2_p;                                 /* kernel descriptor    */
int *sd3_p;                                 /* remote client socket */
struct arp *arp_p;                          /* audit record ptr's   */
int conn_indx;                              /* child-connect index  */
{
    struct sockaddr_in peername;            /* remote client name   */
    int namelen = sizeof(peername);
    pid_t newpid;                           /* child daemon pid     */
    struct sockaddr sl;                     /* for parent msgs      */
    struct stat sbuf;
    char *ptr;
    int i;

#ifdef __DEBUG3
    if ( attr_g.flag & ATTR_DEBUG ) cwrite ( "debug: spawn_child" );
#endif /* __DEBUG3 */

    newpid = fork();


    /* child daemon */
#ifndef __DEBUG2
    if ( newpid == 0 ) {
#else
    if ( newpid ) {
#endif /* __DEBUG2 */

        /* update descriptors */
        close ( *sd1_p );
        close ( *sd2_p );
        *sd1_p = *sd2_p = -1;
        *sd3_p = *conn;

        /* set attributes */
        attr_g.flag &= ~ATTR_NETSRV;
        attr_g.pid = conn_indx;
        i = 1;
        if ( setsockopt ( *sd3_p, SOL_SOCKET, SO_KEEPALIVE, &i, sizeof(i) ) == -1 )
            quit ( "setsockopt", 0 );
        if ( ioctl ( *sd3_p, FIONBIO, &i ) < 0 ) {
            quit ( "ioctl: net server", 0 );
            send ( *conn, "BYE", MSG_OOB_SIZE, MSG_OOB );
            _exit(-1);
        }
        signal ( SIGPIPE, sig_hndlr );

        /* allocate space for input data buffer */
#ifdef __KERBEROS
        /* make sure able to read complete KERBEROS packet */
        if ( attr_g.kerb_auth && attr_g.nkbytes < KRB_RECVSIZ ) {
            if ( (arp_p->aud_rec = (char *)malloc(KRB_RECVSIZ*1024)) == NULL ) {
                cwrite ( "-- insufficient mem --" );
                send ( *conn, "BYE", MSG_OOB_SIZE, MSG_OOB );
                _exit(-1);
            }
            else attr_g.nkbytes = KRB_RECVSIZ;
        }
        else
#endif /* __KERBEROS */
            if ( (arp_p->aud_rec = (char *)malloc(attr_g.nkbytes*1024)) == NULL ) {
                cwrite ( "-- insufficient mem --" );
                send ( *conn, "BYE", MSG_OOB_SIZE, MSG_OOB );
                _exit(-1);
            }
        arp_p->aud_rec_b = arp_p->aud_rec;

        /* build auditlog pathname, and open it */
        i = stat ( attr_g.pathname, &sbuf );
        if ( i == -1 || !(sbuf.st_mode & S_IFREG) ) {
            strcpy ( attr_g.pathname, DEFAULT_LOG_REM );
            for ( i = 0; i < MAXPATHLEN-INDX && attr_g.pathname[i]; i++ );
            attr_g.pathname[i] = '.';
            bcopy ( itoa(0,INDX-1), &attr_g.pathname[i+1], INDX-1 );
        }
        for ( i = 0; i < MAXPATHLEN && attr_g.pathname[i]; i++ );
        if ( i < INDX ) i = INDX;
        attr_g.pathname[i-INDX] = ':';
        if ( getpeername ( *sd3_p, &peername, &namelen ) < 0 ) {
            quit ( "getpeername: spawn_child", 0 );
            strcpy ( &attr_g.pathname[i-(INDX-1)], "remote" );
        }
        else if ( ptr = gethost_l(peername.sin_addr.s_addr) )
            strcpy ( &attr_g.pathname[i-(INDX-1)], ptr );
        else strcpy ( &attr_g.pathname[i-(INDX-1)], inet_ntoa(peername.sin_addr.s_addr) );
        for ( i = 0; i < MAXPATHLEN && attr_g.pathname[i]; i++ );
        attr_g.pathname[i] = '\0';
        set_log_indx ( attr_g.pathname );
        attr_g.pathval = LOCAL;
        close(fda);
        if ( (fda = open ( attr_g.pathname, O_RDWR|O_APPEND|O_CREAT, 0600 )) < 0 ) {
            quit ( "open: spawn_child", 0 );
            return;
        }

        /* establish socket to listen to parent */
        sl.sa_family = AF_UNIX;
        for ( i = 0; sl.sa_data[i] = CHILD_SOCK[i]; i++ );
        bcopy ( itoa(conn_indx,MAXCL_ITOA), &sl.sa_data[i], MAXCL_ITOA );
        sl.sa_data[i+MAXCL_ITOA] = '\0';
        if ( (*sd1_p = socket ( AF_UNIX, SOCK_STREAM, 0 )) < 0 ) quit ( "socket: spawn_child", 0 );
        if ( bind (*sd1_p, &sl, sizeof(struct sockaddr)) < 0 ) quit ( "bind: spawn_child", 0 );
        if ( listen ( *sd1_p, 1 ) < 0 ) quit ( "listen: spawn_child", 0 );
    }


    /* parent: close up network connection */
#ifndef __DEBUG2
    if ( newpid > 0 ) {
#else
    if ( newpid == 0 ) {
#endif /* __DEBUG2 */
        child[conn_indx] = newpid;
        close ( *conn );
    }


    /* fork error condition */
    if ( newpid == -1 ) {
        send ( *conn, "BYE", MSG_OOB_SIZE, MSG_OOB );
        shutdown ( *conn, 2 );
        close ( *conn );
        cwrite ( "unable to create child auditd for new connection" );
    }
}
