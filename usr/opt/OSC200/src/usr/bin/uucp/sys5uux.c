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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: sys5uux.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:08:04 $";
#endif
/* 
 * COMPONENT_NAME: UUCP sys5uux.c
 * 
 * FUNCTIONS: sys5uux 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
1.6  sys5uux.c, bos320 6/16/90 00:01:04";
*/

#include "uucp.h"
/* VERSION( sys5uux.c	1.1 -  -  ); */

char	_Grade = 'N';

/*
 *	sys5uux
 *
 *	Translate a SDNFILE line from System 5 uucp ( version 2 uucp)
 *   	containing multi-hop syntax into a BNU multi-hop request. 
 */

#define W_TYPE		wrkvec[0]
#define W_FILE1		wrkvec[1]
#define W_FILE2		wrkvec[2]
#define W_USER		wrkvec[3]
#define W_OPTNS		wrkvec[4]
#define W_DFILE		wrkvec[5]
#define W_MODE		wrkvec[6]
#define W_NUSER		wrkvec[7]
#define W_SFILE         wrkvec[8]
#define W_RFILE         wrkvec[5]
#define W_XFILE         wrkvec[5]
char    *mf;
/*
main()
{


	char *k[12];
	int Uid, Euid;


	k[0] = "S";
	k[1] = ".profile";
	k[2] = "!haddock!snapper!/usr/spool/uucppublic/out";
	k[3] = "scottk";
	k[4] = "-dCnm";
	k[5] = "D.xxx";
	k[6] = "666";
	k[7] = "bob";

	strcpy(Rmtname,"snapper");
	Debug = 9;
	Uid = getuid();
	Euid = geteuid();
	guinfo(Uid, Loginuser);
	guinfo(Euid, User);
	mchFind("carp");
	sys5uux( k, "uucp.c");
	exit(0);
}
cleanup(){}
*/



sys5uux(wrkvec, dfile)
char *wrkvec[];
char *dfile;
{
	FILE *fprx = NULL;
	int ret, i;
	char rxfile[MAXNAMESIZE*4];	/* file for X_ commands */
	char ndfile[MAXNAMESIZE*4];	/* file for X_ commands */
	char optsbuf[NAMESIZE];
	char cmd[BUFSIZ];
	char *bDfile, *bcfile, *brxfile, *bwfile1;
	char *cr, *op;

	/*
	 * determine id of user starting remote 
	 * execution
	 */

	DEBUG(6, "Loginuser %s\n", Loginuser);

	/*
	 * initialize command buffer
	 */
	*cmd = '\0';

	/*
	 * generate JCL files to work from
	 */


	/*
	 * rxfile is the X. file for the job, fprx is its stream ptr.
	 * The uucp command is to be executed locally!
	 */
	
	gename(XQTPRE, Rmtname, 'X', rxfile);
	DEBUG(9, "rxfile = %s\n", rxfile);
	if(access(rxfile, 0) == 0 ) {
		assert( Fl_EXISTS, rxfile, errno);
		return(1);
	}
	fprx = fdopen(ret = creat(rxfile, DFILEMODE), "w");
	if(ret < 0 || fprx == NULL ) {
		assert( Ct_WRITE, rxfile, errno);
		return(1);
	}
	setbuf(fprx, CNULL);
	clearerr(fprx);

	/* Real uid from uucico */

	(void) fprintf(fprx,"%c %s %s\n", X_USER, Loginuser, Myname);

	(void) fprintf(fprx, MSGSTR(MSG_SYS5_1, 
		"%c return status on failure\n"), X_COMMENT);
	fprintf(fprx,"%c\n", X_NONZERO);


	(void) fprintf(fprx, MSGSTR(MSG_SYS5_2, 
		"%c return address for status or input return\n"), X_COMMENT);
	/* User name from the wrkvec */
	(void) fprintf(fprx,"%c %s\n", X_RETADDR, W_USER);

	gename(DATAPRE, Rmtname, _Grade, ndfile);
	DEBUG(9, "ndfile = %s\n", ndfile);
	if( access(ndfile, 0) == 0 ) {
		assert( Fl_EXISTS, ndfile, errno);
		unlink(rxfile);
		return(1);
	}
	if(xmv(dfile, ndfile) != 0 ) {
		assert( Ct_LINK, ndfile, errno);
		unlink(rxfile);
		return(1);
	}
	
	bDfile = BASENAME( ndfile, '/' );
	bwfile1 = BASENAME( W_FILE1, '/' );
	(void) fprintf(fprx, "%c %s %s\n", X_RQDFILE, bDfile, bwfile1 );


	/*
	 * Parse out relevant System V options
	 *
	 */
	op = W_OPTNS;
	optsbuf[0] = NULLCHAR;
	while ( *op != NULLCHAR ) {

		switch ( *op++ ) {

		case 'C':
			break;
		case 'c':
			break;
		case 'd':
			strcat(optsbuf, "-d "); 
			break;
		case 'f':
			strcat(optsbuf, "-f "); 
			break;
		case 'm':
			strcat(optsbuf, "-m "); 
			break;
		case 'n':
			strcat(optsbuf, "-n"); 
			strcat(optsbuf, W_NUSER);
			strcat(optsbuf, " ");
			break;
		default:
			break;
				
		}
	}
	
	/*
	 * place command to be executed in JCL file
	 */
	
	(void) sprintf(cmd, "uucp -C %s %s %s ", optsbuf, bwfile1, W_FILE2+1);
	(void) fprintf(fprx, "%c %s\n", X_CMD, cmd);
	if(ferror(fprx) != 0 ) {
		assert( Ct_WRITE, rxfile, errno);
		unlink(rxfile);
		unlink(ndfile);
		return(1);
	}
	(void) fclose(fprx);		/* rxfile is ready for commit */

	logent(cmd, MSGSTR(MSG_SYS5_3, "QUEUED"));


	return(0);
}
