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
/* @(#)$RCSfile: lprsetup.h,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/01/08 13:36:01 $  */

/*
 * lprsetup.h
 */

/**********************************************************************
 *
 * Modification history:
 * 28-May-91 Mike Ardehali
 *      Porting to ULTRIX/OSF (tin) release, commented out the
 *      unsupported items under (tin), printservers, lcg01, ln01 and 
 *      ln01s.
 *
 *  8-Jul-88 Dave Maxwell (EUEG)
 *      added support for new postscript capabilities and lps40 printer
 *
 **********************************************************************/

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <time.h>

/*********
* debug
*********/
#define dprintf if (debug) fprintf
extern int debug;

/*********
* Testing * De-Comment to test as a regular user. *
*#define LOCAL
**********/

/*
 * file name defines
 */

#ifdef LOCAL
/* SEE ALSO printlib.c for PRINTCAP defined again */ 

#define PRINTCAP "printcap"     /* printcap pathname            */
#define COPYCAP "copycap"       /* copy file printcap pathname  */
#define LOGCAP  "printcap.log"  /* copy file printcap pathname  */
#define DATAFILE "lprsetup.dat" /* printer template database    */
#define ROOTUSER 1              /* allows a normal user to test */

#else /* NO LOCAL */

#define PRINTCAP "/etc/printcap"        /* printcap pathname            */
#define COPYCAP "/etc/pcap.tmp"         /* copy file printcap pathname  */
#define LOGCAP  "/usr/adm/printcap.log" /* copy file printcap pathname  */
#define DATAFILE "/etc/lprsetup.dat"    /* printer template database    */
#define ROOTUSER 0                      /* must be superuser to use     */

#endif /* LOCAL */

#define DAEMON  "daemon"        /* daemon passwd name           */
#define UNKNOWN "unknown"       /* unknown printer name         */
#define LPF     "/usr/sbin/lpf" /* default output filter        */
#define EDTTY   "/bin/ed - /etc/ttys 1>/dev/null 2>/dev/null" /* edit /etc/ttys file  */
#define MKDEV   "/dev/MAKEDEV"  /* device MAKEDEV script */

#define SUPERUSER       0       /* root pid                      */
#define LEN             256     /* entry string length           */
#define LINES           10      /* 10 lines of comments          */
#define COLUMNS         80      /* comment line length           */
#define DIRMODE         0755    /* directory mode                */
#define MAXPATH         256     /* max file name length          */
#define BUFLEN          80      /* string buffer length          */
#define ALPHANUMLEN     16      /* len of a alphanumeric string  */
#define CMD_LEN         16      /* Maximum length of the command */
                                /*  aliases string               */
#define MAX_CMDS        60      /* Maximum number of commands and aliases */

/*
 * getcmd returns
 */
#define NO              0
#define YES             1
#define NOREPLY         2
#define PRINT           3
#define HELP            4
#define GOT_SYMBOL      5       /* add a symbol */
#define QUIT            6
#define CTRLD           7
#define USED            8       /* only print used symbols */
#define ALL             9       /* print all symbols */
#define BOOL            10      /* boolean symbol */
#define INT             11      /* integer symbol */
#define STR             12      /* string symbol */
#define OFF             13      /* boolean off */
#define ON              14      /* boolean on */
#define ADD             15      /* add entry */
#define MODIFY          16      /* modify entry */
#define DELETE          17      /* delete entry */
#define LIST            18      /* list all possible symbols */
#define VIEW            19      /* view the current contents of the printcap file */
#define PRINTER_INFO    20      /* to request more printer type information */
#define DEFAULTROWS     22      /* default number of usable rows on the screen */
#define EO_LIST         1000    /* End of list token numeric identifier */


#define BUF_LINE        250     /* line length buffer */
#define BUF_WORD        40      /* word length buffer */
#define BUF_HELP        2048    /* max printer help string size */
#define TRUE            1       /* return(...) codes */
#define FALSE           0
#define BAD             -1      /* used in misc: validate() routine */
#define ERROR           1
#define OK              0
#define NOT             !

int     modifying;              /* TRUE when modifying in modify() routine */

int     rows;                   /* the number of usable rows on the screen */

struct table
{
    char   *name;               /* symbol name goes here */
    char   *svalue;             /* default value of symbol */
    int     stype;              /* type of symbol: BOOL, INT, or STR */
    int     used;               /* True if using symbol for this printcap */
    char   *nvalue;             /* new value of symbol */
    int     nun;                /* true if new value is "none" = delete */
};

struct nameval
{
    char   *name;               /* symbol name */
    char   *svalue;             /* value of symbol */
};

struct cmdtyp
{
    char cmd_name[CMD_LEN];
    int  cmd_id;
};

/*
 * Do not add help codes here
 * without first updating "globals.h".
 */
#define H_af    0
#define H_br    1
#define H_cf    2
#define H_ct    3
#define H_df    4
#define H_dn    5
#define H_du    6
#define H_fc    7
#define H_ff    8
#define H_fo    9
#define H_fs    10
#define H_gf    11
#define H_ic    12
#define H_if    13
#define H_lf    14
#define H_lo    15
#define H_lp    16
#define H_mc    17
#define H_mx    18
#define H_nc    19
#define H_nf    20
#define H_of    21

/*** 001 gray ****
#define H_op    22
#define H_os    23
******************/

#define H_pp    25
#define H_pl    24
#define H_ps    26
#define H_pw    27
#define H_px    28
#define H_py    29
#define H_rf    30
#define H_rm    31
#define H_rp    32
#define H_rs    33
#define H_rw    34
#define H_sb    35
#define H_sc    36
#define H_sd    37
#define H_sf    38
#define H_sh    39
#define H_st    40
#define H_tf    41
#define H_tr    42

/*** 001 gray ****
#define H_ts    43
******************/

#define H_uv    44
#define H_vf    45
#define H_xc    46
#define H_xf    47
#define H_xs    48
#define H_Da    49
#define H_Dl    50
#define H_It    51
#define H_Lf    52
#define H_Lu    53
#define H_Ml    54
#define H_Nu    55
#define H_Or    56
#define H_Ot    57
#define H_Ps    58
#define H_Sd    59
#define H_Si    60
#define H_Ss    61
#define H_Ul    62
#define H_Xf    63

/* end of help codes */


/* general help messages */
extern char h_help[];
extern char h_helps[];
extern char h_doadd[];
extern char h_dodel[];
extern char h_domod[];
extern char h_synonym[];
extern char h_default[];
extern char h_addcmnts[];
extern char h_printype[];

/* more specific help messges */
extern char h_af[];
extern char h_br[];
extern char h_ct[];
extern char h_cf[];
extern char h_df[];
extern char h_dn[];
extern char h_du[];
extern char h_fc[];
extern char h_ff[];
extern char h_fo[];
extern char h_fs[];
extern char h_gf[];
extern char h_ic[];
extern char h_if[];
extern char h_lf[];
extern char h_lo[];
extern char h_lp[];
extern char h_mc[];
extern char h_mx[];
extern char h_nc[];
extern char h_nf[];
extern char h_of[];
/* extern char h_op[];   001 - gray */
/* extern char h_os[];   001 - gray */
extern char h_pp[];
extern char h_pl[];
extern char h_ps[];
extern char h_pw[];
extern char h_px[];
extern char h_py[];
extern char h_rf[];
extern char h_rm[];
extern char h_rp[];
extern char h_rs[];
extern char h_rw[];
extern char h_sb[];
extern char h_sc[];
extern char h_sd[];
extern char h_sf[];
extern char h_sh[];
extern char h_st[];
extern char h_tf[];
extern char h_tr[];
/* extern char h_ts[];  001 - gray */
extern char h_uv[];
extern char h_vf[];
extern char h_xc[];
extern char h_xf[];
extern char h_xs[];
extern char h_Da[];
extern char h_Dl[];
extern char h_It[];
extern char h_Lf[];
extern char h_Lu[];
extern char h_Ml[];
extern char h_Nu[];
extern char h_Or[];
extern char h_Ot[];
extern char h_Ps[];
extern char h_Sd[];
extern char h_Si[];
extern char h_Ss[];
extern char h_Ul[];
extern char h_Xf[];

/*
 *  end of lprsetup.h
 */
