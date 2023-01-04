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
/*
 * @(#)$RCSfile: siad.h,v $ $Revision: 1.1.12.5 $ (DEC) $Date: 1993/11/19 21:44:28 $
 */
/***********************************************************************
*	siad.h - SIA constants, structures, and macro definitions 
*		internally used by the interfaces and security mechanisms
************************************************************************/
#include <sia.h>
#ifdef SV_SHLIBS
#include <dlfcn.h>
#endif

#ifndef _SIAD_H_
#define _SIAD_H_

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
#endif
                                  
/***** Authentication types ******/

#define	SIA_A_NONE	0
#define	SIA_A_AUTH	1
#define	SIA_A_REAUTH	2
#define	SIA_A_SUAUTH	3

/***** entity freeing parameter *****/

#define SIAAUTHFREE	1	/* used when freeing authentication data only */
#define SIAFREEALL	0	/* used to free the whole entity */

/***** sia initialization flags *****/
#define SIANEW	0
#define SIABAD  -1
#define SIAGOOD 1

/***** siad returns ********/

/***** bit0 success or fail*/
#define SIADSUCCESS	1		/* The siad function was successful	*/
#define SIADFAIL	0		/* The siad function failed		*/
/***** bit1 continue or stop */
#define SIADCONTINUE	0		/* Continue to the next vectored siad function	*/
#define SIADSTOP	2		/* Stop the vector processing */

/***** Flags for specifying changes to user account information and password **/

#define	CHGPASSWD	1		/* change password flag */
#define CHGSHELL	2		/* change shell flag    */
#define CHGFINGER	4		/* change finger flag   */

/***** sia log formats *****/

#define	SIALOGENTFMT	"\nSIA:%s %s"	/* SIA log entry header */

/***** sia security mechanisms must match global sia_mechs table *****/

#define BSD_SIA_NUM		    1
#define BSD_SIA			    "BSD"
#define DCE_NUM			    2
#define DCE                         "DCE"
#define X_509_NUM                   3
#define X_509                       "X.509"
#define OSFC2_NUM                  4
#define OSFC2                      "OSFC2"
#define NIS_NUM                     5
#define NIS                         "NIS"
#define LAST_NUM		    6
#define MATRIX_LAST                 "LAST"

/***** siad capabilities  must match global sia_caps table*****/

#define	SIA_INIT		0
#define	SIAD_INIT		"siad_init"
#define SIA_CHK_INVOKER		1
#define SIAD_CHK_INVOKER	"siad_chk_invoker"
#define SIA_SES_INIT		2
#define SIAD_SES_INIT		"siad_ses_init"
#define SIA_SES_AUTHENT		3
#define SIAD_SES_AUTHENT	"siad_ses_authent"
#define	SIA_SES_ESTAB		4
#define SIAD_SES_ESTAB		"siad_ses_estab"
#define SIA_SES_LAUNCH		5
#define SIAD_SES_LAUNCH		"siad_ses_launch"
#define SIA_SES_SUAUTHENT	6
#define	SIAD_SES_SUAUTHENT	"siad_ses_suauthent"
#define SIA_SES_REAUTHENT	7
#define	SIAD_SES_REAUTHENT	"siad_ses_reauthent"
#define	SIA_CHG_FINGER		8
#define SIAD_CHG_FINGER		"siad_chg_finger"
#define SIA_CHG_PASSWORD	9
#define SIAD_CHG_PASSWORD	"siad_chg_password"
#define SIA_CHG_SHELL		10
#define SIAD_CHG_SHELL		"siad_chg_shell"
#define SIA_GETPWENT		11
#define SIAD_GETPWENT		"siad_getpwent"
#define SIA_GETPWUID		12
#define SIAD_GETPWUID		"siad_getpwuid"
#define SIA_GETPWNAM		13
#define SIAD_GETPWNAM		"siad_getpwnam"
#define SIA_SETPWENT            14
#define SIAD_SETPWENT           "siad_setpwent"
#define SIA_ENDPWENT            15
#define SIAD_ENDPWENT           "siad_endpwent"
#define SIA_GETGRENT		16
#define SIAD_GETGRENT		"siad_getgrent"
#define SIA_GETGRGID		17
#define SIAD_GETGRGID		"siad_getgrgid"
#define SIA_GETGRNAM		18
#define SIAD_GETGRNAM		"siad_getgrnam"
#define SIA_SETGRENT            19
#define SIAD_SETGRENT           "siad_setgrent"
#define SIA_ENDGRENT            20
#define SIAD_ENDGRENT           "siad_endgrent"
#define SIA_SES_RELEASE		21
#define SIAD_SES_RELEASE	"siad_ses_release"
#define SIA_CHK_USER		22
#define SIAD_CHK_USER		"siad_chk_user"

/***** sia macros ********/

#define SIALOG	sia_log


/*	SIA mutex locks		*/

#define SIA_ENTITY_LOCK		1 /* lock for allocation and freeing entities*/
#define SIA_LOG_LOCK		2 /* lock for writing log entries	*/ 
#define SIA_AUTHENT_LOCK	3 /* locks authentication processes 	*/
#define SIA_ESTAB_LOCK		4 /* locks session establishment    	*/
#define SIA_LAUNCH_LOCK		5 /* locks session launching	    	*/
#define SIA_INIT_LOCK		6 /* locks SIA initialization		*/
#define SIA_CHANGE_LOCK		7 /* change finger,shell,passowrd  mutex*/
#define SIA_GROUP_LOCK          8 /* locks SIA getgr* calls             */
#define SIA_PASSWD_LOCK         9 /* locks SIA getpw* calls             */
#define SIA_SES_LOCK           10

/***** sia collect definitions *****/

/***** sia collect routine interface definition *****/

extern int sia_collect (
        int             timeout,
        int             rendition,
        unsigned char   *title,
        int             num_prompts,
        prompt_t        *prompt);

#define MAX_PROMPTS	8

/*	timeout - how long to wait for user    */
/* 	if 0 then wait forever if nonzero then */
/*	that number of seconds		       */

#define	SIAWAITFOREVER	0
#define SIAONEMIN	60
#define	SIA_DEF_TIMEOUT	(2*60)

/*	rendition - how to run the parameter collection */
/*							*/

#define SIAMENUONE      1	/* select one of the choices given */
#define SIAMENUANY      2	/* select any of the choices given */
#define SIAFORM         3	/* fill out the form */
#define SIAONELINER     4	/* One question with one answer */
#define SIAINFO         5	/* Information only */
#define SIAWARNING      6	/* ERROR or WARNING message  (possibly usage)*/
#define LAST_RENDITION  6

/*** collect return values ***/

#define SIACOLSUCCESS   1       /* the collection was successful */
#define SIACOLABORT     2       /* the collection was aborted */
#define SIACOLTIMOUT    3       /* the collection timed out */
#define SIACOLPARAMS    4       /* bad parameters to sia_collect */


/* control_flags are used to control the display a bit */

/*	NOTE: non-NULL result pointer => a selected menu item */

#define SIARESINVIS     0x2     /*result is invisible */
#define SIARESANY       0x10    /*result can contain any ASCII chars*/
#define SIAPRINTABLE    0x20    /*result can contain only printable chars*/
#define SIAALPHA        0x40    /*result can contain only letters */
#define SIANUMBER       0x80    /*result can contain only numbers*/
#define SIAALPHANUM     0x100   /*result can contain only letters and numbers*/
#define FLAG_MAX        0x200

#ifdef	_NO_PROTO
extern  int siad_init();
extern  int siad_chk_invoker();
extern  int siad_ses_init();
extern  int siad_ses_authent();
extern  int siad_ses_estab();
extern  int siad_ses_launch();
extern  int siad_ses_suauthent();
extern  int siad_ses_reauthent();
extern  int siad_chg_finger();
extern  int siad_chg_password();
extern  int siad_chg_shell();
extern int siad_getpwent();
extern int siad_getpwuid();
extern int siad_getpwnam();
extern int siad_setpwent();
extern int siad_endpwent();
extern int siad_getgrent();
extern int siad_getgrgid();
extern int siad_getgrnam();
extern int siad_setgrent();
extern int siad_endgrent();
extern int siad_ses_release();
extern int siad_chk_user();

extern int sia_make_entity_pwd();
extern int sia_setupfp();
extern int sia_audit();
extern int sia_chdir();
extern int sia_timed_action();
extern char *sia_getamsg();

#else	/* _NO_PROTO */

extern  int siad_init (void);
extern  int siad_chk_invoker (void);
extern  int siad_ses_init (SIAENTITY *entity, int pkgind);
extern  int siad_ses_authent (int (*collect) (int timeout, 
					      int rendition, 
					      unsigned const char *title, 
					      int num_prompts, 
					      prompt_t *prompts), 
			      SIAENTITY *entityhdl, int siastat,
				int pkgind);
extern  int siad_ses_estab (int (*collect)(int timeout, 
					   int rendition, 
					   unsigned const char *title, 
					   int num_prompts, 
					   prompt_t *prompts), 
			    SIAENTITY *entity, int pkgind);
extern  int siad_ses_launch (int (*collect)(int timeout, 
					    int rendition, 
					    unsigned const char *title, 
					    int num_prompts, 
					    prompt_t *prompts),
			     SIAENTITY *entity,
			     int pkgind);
extern  int siad_ses_suauthent (int (*collect) (int timeout, 
						int rendition, 
						unsigned const char *title, 
						int num_prompts, 
						prompt_t *prompts),
				SIAENTITY *entity,
				int siastat,
				int pkgind);
extern  int siad_ses_reauthent (int (*collect) (int timeout, 
						int rendition, 
						unsigned const char *title, 
						int num_prompts, 
						prompt_t *prompts),
				SIAENTITY *entity,
				int siastat,
				int pkgind);
extern  int siad_chg_finger (int (*collect) (int timeout, 
					     int rendition, 
					     unsigned const char *title, 
					     int num_prompts, 
					     prompt_t *prompts),
			       const char *username, int argc, char *argv[]);
extern  int siad_chg_password (int (*collect) (int timeout, 
					       int rendition, 
					       unsigned const char *title, 
					       int num_prompts, 
					       prompt_t *prompts),
			       const char *username, int argc, char *argv[]);
extern  int siad_chg_shell (int (*collect) (int timeout, 
					    int rendition, 
					    unsigned const char *title, 
					    int num_prompts, 
					    prompt_t *prompts),
			       const char *username, int argc, char *argv[]);
extern int siad_getpwent (struct passwd *result, char *buf, int bufsize, FILE **context);
extern int siad_getpwuid (uid_t uid, struct passwd *result, char *buf, int bufsize);
extern int siad_getpwnam (const char *name, struct passwd *result, char *buf, int bufsize);
extern int siad_setpwent (FILE **context);
extern int siad_endpwent (FILE **context);
extern int siad_getgrent (struct group *result, char *buf, int bufsize, FILE **context);
extern int siad_getgrgid (gid_t gid, struct group *result, char *buf, int bufsize);
extern int siad_getgrnam (const char *name, struct group *result, char *buf, int bufsize);
extern int siad_setgrent (FILE **context);
extern int siad_endgrent (FILE **context);
extern int siad_ses_release (SIAENTITY *entity, int pkgind);
extern int siad_chk_user (const char *logname, int checkflag);

extern int sia_make_entity_pwd (struct passwd *pwd, SIAENTITY *entity);
extern int sia_setupfp (int capind, int pkgind);
extern int sia_audit (u_int atype, ...);
extern int sia_chdir (const char *directory, time_t timeout);
extern int sia_timed_action (int (*action)(), void *param, time_t timeout);
extern char *sia_getamsg (const char *path, int set, int msgnum, const char *defstr);

#endif	/* _NO_PROTO */

/**** sia globals and externs ****/
/*
 * only defined in sia_init.c.
 * note only the sia_mutex's are global.
 * the others are extern's for the thread
 * safe case and are picked up from libc.
 */

#ifdef SIA_THREAD_GLOBAL
struct rec_mutex sia_mutex[SIAMUTMAX];
#endif /* _THREAD_SAFE */

#ifdef SIAGLOBAL /****** only defined in sia_init.c *****/

struct sia_matrix_t sia_mat;
int	sia_initialized=SIANEW;
ldr_module_t    sia_handle=NULL;
char *sia_mechs[] = {	/* must be in same order as NUMs above */
  BSD_SIA,
  DCE,
  X_509,
  OSFC2,
  NIS,
  0};

/***** This is the sia capabilities list which maps to the sia matrix *****/
char *sia_caps[] = {
  SIAD_INIT,
  SIAD_CHK_INVOKER,
  SIAD_SES_INIT,
  SIAD_SES_AUTHENT,
  SIAD_SES_ESTAB,
  SIAD_SES_LAUNCH,
  SIAD_SES_SUAUTHENT,
  SIAD_SES_REAUTHENT,
  SIAD_CHG_FINGER,
  SIAD_CHG_PASSWORD,
  SIAD_CHG_SHELL,
  SIAD_GETPWENT,
  SIAD_GETPWUID,
  SIAD_GETPWNAM,
  SIAD_SETPWENT,
  SIAD_ENDPWENT,
  SIAD_GETGRENT,
  SIAD_GETGRGID,
  SIAD_GETGRNAM,
  SIAD_SETGRENT,
  SIAD_ENDGRENT,
  SIAD_SES_RELEASE,
  SIAD_CHK_USER,
  0};

/***** This is the sia capabilities entry point list for LIBCSO *****/
int (*sia_cap_fps[])() = {
  siad_init,
  siad_chk_invoker,
  siad_ses_init,
  siad_ses_authent,
  siad_ses_estab,
  siad_ses_launch,
  siad_ses_suauthent,
  siad_ses_reauthent,
  siad_chg_finger,
  siad_chg_password,
  siad_chg_shell,
  siad_getpwent,
  siad_getpwuid,
  siad_getpwnam,
  siad_setpwent,
  siad_endpwent,
  siad_getgrent,
  siad_getgrgid,
  siad_getgrnam,
  siad_setgrent,
  siad_endgrent,
  siad_ses_release,
  siad_chk_user,
  0};


#else /* SIAGLOBAL */

#ifdef _THREAD_SAFE
extern	struct rec_mutex sia_mutex[SIAMUTMAX]; 
#endif /* _THREAD_SAFE */

extern  struct sia_matrix_t sia_mat;
extern  sia_initialized;
extern  sia_handle;
extern	char *sia_mechs[];
extern	char *sia_caps[];
extern	int (*sia_cap_fps[])();
extern  int siad_group[];
extern  int siad_passwd[];

#endif /* SIAGLOBAL */

/* Get call switching */
#define G_ENT	0
#define G_GID	1
#define G_NAM	2
#define G_SET   3
#define G_END   4

#define P_ENT	0
#define P_UID	1
#define P_NAM	2
#define P_SET   3
#define P_END   4

#define REENTRANT       1
#define NON_REENTRANT   0

typedef struct {
  char *name;
  gid_t gid;
  struct group *result;
  char *buffer;
  int len;
  int pkgind;
} group_params;

typedef struct {
  char *name;
  uid_t uid;
  struct passwd *result;
  char *buffer;
  int len;
  int pkgind;
} passwd_params;

union sia_get_params {
  group_params group;
  passwd_params passwd;
};
			  
extern  int sia_log();
extern  int sia_init();
	
#define sia_matrix	sia_mat.matrix_ent

/***** sia thread locking *****/
#ifdef _THREAD_SAFE

 
#define SIATHREADLOCK(x) 	rec_mutex_lock(&sia_mutex[(x)]);
#define SIATHREADREL(x)		rec_mutex_unlock(&sia_mutex[(x)]);

#else /* _THREAD_SAFE */

#define SIATHREADLOCK(x)
#define SIATHREADREL(x)

#endif /* _THREAD_SAFE */


/* sia matrix definitions */

#ifndef SIA_MATRIX_H
#define SIA_MATRIX_H
#endif /* SIA_MATRIX_H */

typedef struct sia_matrix_ent {	   /* sia matrix conf entry */
  char *libnam;
  char *pkgnam;   
  int (*fp)(); 
} sia_matrix_ent;		   
			
typedef struct sia_matrix_t {
  int                matrix_date; /* Last mod date of matrix.conf */
  sia_matrix_ent     matrix_ent [SIADCAPMAX] [SIASWMAX]; 
  struct stat	     matstat;
} sia_matrix_t;

#endif /* _SIAD_H_ */
