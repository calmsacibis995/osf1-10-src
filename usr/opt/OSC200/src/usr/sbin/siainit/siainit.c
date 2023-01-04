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
static char *rcsid = "@(#)$RCSfile: siainit.c,v $ $Revision: 1.1.8.3 $ (DEC) $Date: 1993/10/08 16:01:53 $";
#endif
/*****************************************************************************
* Usage:   siainit
*
* Description: The purpose of this command is to initialize, once per boot,
* the sia layers. This command will run as part of the boot sequence and
* will immediately follow the loading of the shared libraries. This sequencing
* is important to assure that the security mechanisms are avialable to recieve
* the siad_init callout.
*
* The primary purpose of this command is to give each configured security
* mechanism an opportunity to do system bootime setup or checking.
*
* If his command is used in sigle user mode the same sequencing of mounting
* /usr and loading the shared libraries must be followed.
*
* Parameter Descriptions:  none
*
* Success return: SIASUCCESS or SIAFAIL 
*
* Error Conditions and returns otherthan parameter syntax checks listed above:
*
* 	Condition1: loader failure to load libsia or a package within libsia
*	Return: SIAFAIL
*
*	Condition2: Mechanism initialization error.
*	Return:	SIAFAIL
*
*****************************************************************************/
#define	 SIAGLOBAL
#include <siad.h>
#ifdef MSG
#include <nl_types.h>
nl_catd catd;
#include "libc_msg.h"
#define MSGSTR(n,s) catgets(catd,MS_LIBSIA,n,s)
#else /* MSG */
#define MSGSTR(n,s) s
#endif /* MSG */
#ifdef SV_SHLIBS
#include <rld_interface.h>
#endif

main()
{
	struct stat *siastatbuf;
#ifndef SV_SHLIBS
        ldr_module_t    lib_handle = NULL;
        int             (*fp) ();
#else
        void *lib_handle = NULL;
        void *fp;
#endif
	int initgoodfile=0;
	int siaderr=0;			/* SIAD security mechanism returned value */
        int siastat=SIADFAIL;                  /* sia status between mechanism calls */
	int pkgind=0;			/* sia package switching index		*/

#ifdef MSG
	catd = catopen(MF_LIBC, NL_CAT_LOCALE);
#endif

	unlink(SIAIGOODFILE);           /* remove the SIA INITIALIZATION GOOD FILE FLAG */

	if(sia_mat_init() == SIAFAIL)
		{
		SIALOG(MSGSTR(SIA_MSG_LOGALERT,"ALERT"),MSGSTR(SIA_MSG_INITFAIL,"SIA initialization failure"));
		exit(1);
		}
	/* eventually do a get passport for well known system startup passport */
	/********     errno_sia=getsyspass(pahdl_other,paset_other); **************/
	/** call each of the security mechanisms **/ 
        while(sia_matrix[SIA_INIT][pkgind].pkgnam)
                {
		if(sia_matrix[SIA_INIT][pkgind].fp == NULL)
			{
			if(strcmp(sia_matrix[SIA_INIT][pkgind].libnam,SIALIBCSO) == 0)
                                {
                                sia_matrix[SIA_INIT][pkgind].fp=siad_init;
                                }
#ifndef SV_SHLIBS  /*********** OSF SHARED LIBRARIES *************/

			else	if((lib_handle=load(sia_matrix[SIA_INIT][pkgind].libnam,LDR_NOFLAGS)) >= 0)
				{
                        	fp = ldr_lookup_package(sia_matrix[SIA_INIT][pkgind].pkgnam,SIAD_INIT);
				if (( fp == NULL) | (fp && fp == (void *)-1))
					{
					SIALOG(MSGSTR(SIA_MSG_LOGALERT,"ALERT"),MSGSTR(SIA_MSG_CONFERR,"SIA configuration error pertaining to %s\n"),sia_matrix[SIA_INIT][pkgind].pkgnam);
					}
				else	sia_matrix[SIA_INIT][pkgind].fp=fp;
				}
#else    /*************** SVR4 SHARED LIBRARIES ****************/
                        else    {
				if((lib_handle = dlopen(sia_matrix[SIA_INIT][pkgind].libnam, RTLD_LAZY)) != NULL)
                                	{
				fflush(stderr);
				sleep(10);
                                	fp=dlsym(lib_handle,SIAD_INIT);
				fprintf(stderr,"siainit fp=%x\n",fp);
                                	if(fp == NULL)
                                        	{
                                        	SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"), MSGSTR(SIA_MSG_CONFERR,"SIA configuration error pertaining to %s\n"), SIAD_INIT);
                                        	}
                                	else    sia_matrix[SIA_INIT][pkgind].fp=fp;
                                	}
				}
#endif
			}
		if(sia_matrix[SIA_INIT][pkgind].fp != NULL)
			{
			siaderr = (*sia_matrix[SIA_INIT][pkgind].fp)();
			if(siaderr == SIADSUCCESS)
				siastat=SIADSUCCESS;
			else	SIALOG(MSGSTR(SIA_MSG_LOGALERT,"ALERT"),MSGSTR(SIA_MSG_CONFERR,"SIA configuration error pertaining to %s\n"),sia_matrix[SIA_INIT][pkgind].pkgnam);
			}
		pkgind++;
		}
	if(siastat != SIADSUCCESS)
		{
		SIALOG(MSGSTR(SIA_MSG_LOGALERT,"ALERT"),MSGSTR(SIA_MSG_INITFAIL,"SIA initialization failure"));
		fprintf(stderr,MSGSTR(SIA_MSG_INITFAIL,"SIA initialization failure\n"));
                exit(1);
		}
	/****** We get here if the sia initialization is all successful *******/
	 /* creat initialization good file */
        if( initgoodfile = open(SIAIGOODFILE,(O_CREAT|O_RDWR),S_IRWXU) < 0)
		{
		SIALOG(MSGSTR(SIA_MSG_LOGALERT,"ALERT"),MSGSTR(SIA_MSG_INITFAIL,"SIA initialization failure"));
		fprintf(stderr,MSGSTR(SIA_MSG_INITFAIL,"SIA initialization failure\n"));
                exit(1);
		}
	SIALOG(MSGSTR(SIA_MSG_LOGEVENT,"EVENT"),MSGSTR(SIA_MSG_INITGOOD,"Successful SIA initialization"));
	close(initgoodfile);
	fprintf(stderr,MSGSTR(SIA_MSG_INITGOOD,"Successful SIA initialization\n"));
	exit(0);
}
