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
static char	*sccsid = "@(#)$RCSfile: accton.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:59:53 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */ 
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: prmsg
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	accton - calls syscall with syschk()  privileges
 */
#include        <sys/secdefines.h>
#if SEC_BASE
#include        <sys/security.h>
#endif

#include	<sys/types.h>
#include	<errno.h>
#include	<sys/stat.h>
#include	<stdio.h>
#include	<pwd.h>

#define ADM_NAME	"adm"
#define	ERR	(-1)

extern errno;
#if SEC_BASE
extern priv_t   *privvec();
#endif

#include <locale.h>
#include "acct_msg.h"
#define	MSGSTR(Num, Str)	NLgetamsg(MF_ACCT, MS_ACCT, Num, Str)
/* make this a function so we can save the string and errno */
prmsg(num, string) 
int num;
char *string;
{
  extern char *arg0;
  int saverrno = errno; /* the call to fprintf might change errno! */
  (void)fprintf(stderr, "%s: %s: ",arg0,MSGSTR(num,string));
  errno = saverrno;
  perror("");
}

char		*arg0;

main(int argc, char **argv) 
{
  struct passwd 	*adm_pw;  
  struct stat		stbuf;
  register struct stat	*s = &stbuf;
  
  (void) setlocale (LC_ALL,"");
#if SEC_BASE
        set_auth_parameters(argc, argv);
        initprivs();

        if (!authorized_user("acct")) {
                fprintf(stderr, "accton: need acct authorization\n");
                exit(1);
        }
        if (forceprivs(privvec(SEC_ACCT, SEC_ALLOWDACACCESS,
#if SEC_MAC
                                SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
                                SEC_ALLOWNCAVACCESS,
#endif
                                -1), (priv_t *) 0)) {
                fprintf(stderr, "accton: insufficient privileges\n");
                exit(1);
        }
#endif

  arg0 = argv[0];    /* capture command name for error message(s) */
  if (argc > 1)      /* file name argument */
    {
      if(stat(argv[1], s) == ERR)     
	{    /* file not found */
	  (void)fprintf(stderr, MSGSTR( CANTSTAT, "%s: Cannot stat %s\n"),
		    arg0, argv[1]);
	  exit(1);
	}
	if ( (adm_pw = getpwnam( ADM_NAME )) == ((struct passwd *)0) ) 
	{
          (void)fprintf(stderr, MSGSTR( NOPWENT, 
			"No entry for %s in password file.\n"), ADM_NAME); 
	  exit(1);
	} 
        if(s->st_uid != adm_pw->pw_uid || s->st_gid != adm_pw->pw_gid)    
	{    /* file does not have proper uid/gid */
	  (void)fprintf(stderr, MSGSTR(BADUIDGID, "%s: uid/gid not %s\n"), 
			arg0, ADM_NAME);
	  exit(1);
	}
      
      if((s->st_mode & 0777) != 0664)    
	{    /* file does not have proper permissions */
          (void)fprintf(stderr, "%s: %s\n", arg0,
		    MSGSTR(BADMODE, "file mode not 0664"));
	  exit(1);
	}
      
      if(acct(argv[1]) == ERR) 
	{    /* turn cmd accounting on */
	  if(errno == EBUSY)
	    (void)fprintf(stderr, "%s: %s\n", arg0,
		      MSGSTR( ACCTBUSY,
			     "accounting is busy: cannot turn accounting ON"));
	  else
	    prmsg(ACCTCANTON, "cannot turn accounting ON");
	  exit(1);
	}
    }
  /*
   * The following else branch currently never returns
   * an ERR.  In other words, you may turn the accounting
   * off to your heart's content.
   */
  else if(acct((char *)0) == ERR) 
    {    /* no file name argument, turn cmd acct off */
      prmsg(ACCTNOOFF, "cannot turn accounting OFF");
      exit(1);
    }
  return(0);
}
