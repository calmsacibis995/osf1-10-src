/* mts.h - definitions for the mail system */

#ifdef OSF
#include <stdlib.h>   /* for malloc declarations */
#endif

/* Local and UUCP Host Name */

char   *LocalName (), *SystemName (), *UucpChan ();


/* Mailboxes */

extern char *mmdfldir,
            *mmdflfil,
            *uucpldir,
            *uucplfil;

#define	MAILDIR	(mmdfldir && *mmdfldir ? mmdfldir : getenv ("HOME"))
#define	MAILFIL	(mmdflfil && *mmdflfil ? mmdflfil : getusr ())
#define	UUCPDIR	(uucpldir && *uucpldir ? uucpldir : getenv ("HOME"))
#define	UUCPFIL	(uucplfil && *uucplfil ? uucplfil : getusr ())

char   *getusr (), *getfullname ();
char   *getenv ();


/* Separators */

extern char *mmdlm1,
            *mmdlm2;

#define	isdlm1(s)	(strcmp (s, mmdlm1) == 0)
#define	isdlm2(s)	(strcmp (s, mmdlm2) == 0)


/* Filters */

extern char *umincproc;


#if (defined OSF || defined OSF1) && !defined DOUBLE_LK
#define	DOUBLE_LK		/* Allow the use of both .lock & lockf */
#endif

/* Locking Directory */

#define	LOK_UNIX	0	/* Use internel locking */
#define	LOK_BELL	1	/* Use username.lock file */
#define	LOK_MMDF	2	/* Use mmdf style lock file */
#ifdef	DOUBLE_LK
#define LOK_KERNEL      4       /* Same as LOK_UNIX (but allows bit fields) */
#define LOK_VALID(lok)  ((lok)==LOK_UNIX||(lok)==LOK_BELL||(lok)==LOK_MMDF||\
			 (lok)==LOK_KERNEL||(lok)==(LOK_KERNEL|LOK_BELL))
#define LOK_DFLT        (LOK_KERNEL|LOK_BELL)
#define	LOK_INVAL	-1
#else
#define LOK_VALID(lok)  ((lok)>=LOK_UNIX && (lok)<=LOK_MMDF))
#define LOK_DFLT        (LOK_UNIX)
#endif	/* DOUBLE_LK */

#ifndef	MMDFONLY
extern int   lockstyle;
#endif	MMDFONLY
extern char *lockldir;

#ifndef	MMDFONLY
extern int   lockstyle;
#endif	MMDFONLY
extern char *lockldir;

int	lkopen (), lkclose ();
FILE   *lkfopen ();
int	lkfclose ();

/*  */

/* MTS specific variables */

#ifdef	MHMTS
extern char *Mailqdir;
extern char *TMailqdir;
extern int Syscpy;
extern char *Overseer;
extern char *Mailer;
extern char *Fromtmp;
extern char *Msgtmp;
extern char *Errtmp;
extern int Tmpmode;
extern char *Okhosts;
extern char *Okdests;
#endif	MHMTS

#ifdef	MMDFMTS
#endif	MMDFMTS

#ifdef	SENDMTS
extern char *hostable;
extern char *sendmail;
#endif SENDMTS


/* SMTP/POP stuff */

extern char *servers;
extern char *pophost;
extern char *x400host;


/* BBoards-specific variables */

extern char *bb_domain;


/* POP BBoards-specific variables */

#ifdef	BPOP
extern char *popbbhost;
extern char *popbbuser;
extern char *popbblist;
#endif	BPOP


/* MailDelivery */

extern char *maildelivery;


/* Aliasing Facility (doesn't belong here) */

extern int Everyone;
extern char *NoShell;
