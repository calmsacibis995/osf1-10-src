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
#ifndef	_C2_SIA_H_
#define	_C2_SIA_H_	1

#ifndef	_PWD_H_
#include <pwd.h>
#endif	/* _PWD_H_ */
#ifndef	__SECURITY_
#include <sys/security.h>
#endif	/* __SECURITY_ */
#ifndef	P_ENT
#include <siad.h>
#endif

#ifdef	MSGSTR
#undef	MSGSTR
#endif
#ifdef	GETMSG
#undef	GETMSG
#endif

#ifdef	MSG
#include "libsec_msg.h"
#define	MSGSTR(n,s)	sia_getamsg(MF_LIBSEC,MS_SIA,n,s)
#define	GETMSG(m,n,s)	sia_getamsg(MF_LIBSEC,m,n,s)
#else
#define	MSGSTR(n,s)	s
#define	GETMSG(m,n,s)	s
#endif

#ifndef	LAST	/* needed for N_SYSCALLS in audit.h for prot.h */
#include <sys/syscall.h>
#endif

#ifndef	__PROT__

#include <prot.h>
#endif /* __PROT__ */
#ifndef	AUDIT_MASK_LEN
#include <sys/audit.h>
#endif
#ifndef	AUDIT_MASK_TYPE
#define	AUDIT_MASK_TYPE	char
#endif

#define	MAX_LOGIN_LENGTH	8
#define	MAX_PWD_LENGTH		16
#define	DEF_DIR			"/"

typedef struct passwd PASSWD;
typedef struct pr_passwd PR_PASSWD;
typedef struct pr_term PR_TERM;

struct c2_mech_struct {
	PR_PASSWD *prpwd;
	PR_TERM *prterm;
	unsigned count;
	int auth_type;
	int setup_done;
	int new_nice;
	int audit_cntl;
	AUDIT_MASK_TYPE audmask[AUDIT_MASK_LEN];
};
typedef struct c2_mech_struct C2_MECH;

#define	EN_MECH(x,y)	(*(C2_MECH **)&(x->mech[y]))
#define	EN_PR_PWD(x,y)	(((C2_MECH *)(x->mech[y]))->prpwd)
#define	EN_PR_TRM(x,y)	(((C2_MECH *)(x->mech[y]))->prterm)

#ifdef	_NO_PROTO

void login_die();
int login_need_passwd();
PR_PASSWD *login_check_expired();
PR_PASSWD *login_bad_user();
int login_set_user();
void login_delay();
int login_fillin_user();
int login_vaildate();
int login_set_sys();
PR_TERM *login_term_params();
#ifdef	SEC_NET_TTY
PR_TERM *login_net_term_params();
#endif	/* SEC_NET_TTY */
void login_do_sublogin();
PR_TERM *login_bad_tty();
PR_TERM *login_good_tty();
int login_good_user();
char *login_crypt();
int c2_make_mech();
int sia_entity_audit();

#else

void login_die(int code);
int login_need_passwd(PR_PASSWD *pr, PR_TERM *prtc, int *pnopassword);
PR_PASSWD *login_check_expired(PR_PASSWD *pr, PR_TERM *prtc);
PR_PASSWD *login_bad_user(PR_PASSWD *pr, PR_TERM *prtc);
int login_set_user(PR_PASSWD *pr, PR_TERM *prtc, PASSWD *p);
void login_delay(char *reason);
int login_fillin_user(char *user, PR_PASSWD **ppr, PASSWD **ppwd);
int login_vaildate(PR_PASSWD **pprpwd, PR_TERM **pprtc, int *pnopassword);
int login_set_sys();
PR_TERM *login_term_params(char *tty_path, char *tty_name);
#ifdef	SEC_NET_TTY
PR_TERM *login_net_term_params(char *tty_path, char *hostname);
#endif	/* SEC_NET_TTY */
void login_do_sublogin(char **envinit);
PR_TERM *login_bad_tty(PR_TERM *pr, PR_PASSWD *prpw);
PR_TERM *login_good_tty(PR_TERM *pr, PR_PASSWD *prpw);
int login_good_user(PR_PASSWD **pprpwd, PR_TERM **pprtc, PASSWD *pwd);
char *login_crypt(char *prompt, char *seed, int alg);
int c2_make_mech(SIAENTITY *entity, int pkgind, PR_PASSWD *pr_pwd, PR_TERM *pr_term);
int sia_entity_audit(SIAENTITY *entity, int pkgind, char *message);

#endif	/* _NO_PROTO */

#endif /* _C2_SIA_H_ */
