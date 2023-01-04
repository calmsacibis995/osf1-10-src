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
/***** Until this works its commented out dupdef in Date variable
***#ifndef lint
***static char *rcsid = "@(#)$RCSfile: sia.h,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/08/04 21:19:25 $";
***#endif
***************/
/***********************************************************************
*	sia.h - SIA constants, structures, and macro definitions 
************************************************************************/
#ifndef _SIA_H_
#define _SIA_H_

/***** sia common sys includes **/

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/sem.h>
#include <sys/uio.h>
#include <sys/audit.h>

/***** sia common usr includes **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sgtty.h>
#include <utmp.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <lastlog.h>
#include <errno.h>
#include <ttyent.h>
#include <syslog.h>
#include <limits.h>
#include <strings.h>
#include <loader.h>
#include <nl_types.h>

#ifdef _THREAD_SAFE
#include <cma.h>
#include <pthread.h>
#endif

/**** sia logical definitions ****/

#ifndef	NULL
#define NULL 0L
#endif
#define YES 1
#define NO 0
#define TRUE 1
#define FALSE 0

/**** sia library and filenames ****/

#define SIALOGFILE      "/var/adm/sialog"          /* SIA logging file        */
#define SIAIGOODFILE    "/etc/sia/siainitgood"     /* sia init good file flag */
#define MATRIX_CONF     "/etc/sia/matrix.conf"     /* sia matrix config file  */
#define SIALIBCSO       "libc.so"                  /* local security in libc  */

/***** sia limits ***********/

#define SIABUFSIZ       1024    /* buffer size for loginname, password, and   */
                                /* one line of the SIA matrix config file     */
#define SIAPKGNAMAX     64      /* Libsia packagename maximum size            */
#define SIALIBNAMAX     64      /* Libsia packagename maximum size            */
#define SIADCAPMAX      32      /* max sec mech sia capabilities              */
#define SIASWMAX        4       /* max switches per capability                */
#define SIAMUTMAX       16      /* number mutexes utilized by SIA             */
#define SIAMXACCTNAME   64      /* maximum size of an account name            */
#define SIAMXPASSWORD   80      /* maximum size of a password                 */
#define SIANAMEMIN      16      /* minimum space allocated for entity->name   */


/***** sia entity structure *******/ 

typedef struct siaentity {
                        char *name;             /* collected name             */
			char *password;         /* entered or collected password */
                        char *acctname;         /* account name               */
                        char **argv;            /* calling command argument   */
                                                /* strings                    */
                        int  argc;              /* number of arguments        */
                        char *hostname;         /* requesting host NULL=>local*/
                        char *tty;              /* pathname of local tty      */
                        int colinput;           /* 1 => yes 0 => no input     */
                        int error;              /* error message value        */
                        int authcount;          /* Number of consecutive      */
                                                /* Failed authen attempts     */
                        int authtype;           /* Type of last authent       */
                        struct passwd *pwd;     /* pointer to pwent struct    */
			char *gssapi;
                        char *sia_pp;            /* for passport implementation*/
                        int *mech[SIASWMAX];    /* pointers to mechanism      */
                                                /* specific data              */
                        } SIAENTITY;

/***** sia collect routine interface definition *****/

typedef struct prompt_t
{
        unsigned char *prompt;
        unsigned char *result;
        int max_result_length;
        int min_result_length;
        int control_flags;
} prompt_t;

/* top level returns from sia routines */

#define SIASUCCESS      1  		/* success return */
#define SIAFAIL         0 		/* failure return */
#define SIASTOP		2		/* stop processing */


/**************************************/
/***** sia top level interface    *****/
/***** special purpose routines   *****/
/**************************************/

#ifdef	_NO_PROTO
extern int sia_init();
extern int sia_authorize();
extern int sia_chk_invoker();
extern int sia_collect_trm();
extern int sia_chg_finger();
extern int sia_chg_password();
extern int sia_chg_shell();
extern int sia_ses_init();
extern int sia_ses_authent();
extern int sia_ses_reauthent();
extern int sia_ses_suauthent();
extern int sia_ses_estab();
extern int sia_ses_launch();
extern int sia_ses_release();
#else	/* _NO_PROTO */
extern int sia_init(void);		/* only called from siainit command at*/
					/* boot time			      */

extern int sia_authorize(SIAENTITY *entity); /* only called internally for sia*/
					     /* authorization */

extern int sia_chk_invoker(void);	     /* called to check if process is */
					     /* sufficiently privialged	      */

extern int sia_collect_trm(int timeout, int rendition, unsigned char *title, int num_prompts, prompt_t *prompts); /* general collection routine for*/
					     /* terminals */
/**************************************/
/***** sia change secure info calls ***/
/**************************************/

extern int sia_chg_finger(int (*collect)(), char *username, int argc, char *argv[]);
extern int sia_chg_password(int (*collect)(), char *username, int argc, char *argv[]);
extern int sia_chg_shell(int (*collect)(), char *username, int argc, char *argv[]);

/*************************************/
/**** session processing calls	******/
/*************************************/


extern int sia_ses_init(SIAENTITY **entityhdl, int argc, char *argv[], char *hostname, char *username, char *tty, int colinput, char *gssapi);
extern int sia_ses_authent(int (*collect)(), char *password, SIAENTITY *entity);
extern int sia_ses_reauthent(int (*collect)(),  SIAENTITY *entity);
extern int sia_ses_suauthent(int (*collect)(),  SIAENTITY *entity);
extern int sia_ses_estab(int (*collect)(), SIAENTITY *entity);
extern int sia_ses_launch(int (*collect)(), SIAENTITY *entity);
extern int sia_ses_release(SIAENTITY **entityhdl);
#endif	/* _NO_PROTO */

#endif /* _SIA_H_ */
