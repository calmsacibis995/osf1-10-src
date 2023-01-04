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
static char	*sccsid = "@(#)$RCSfile: dbck_mac.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 15:07:31 $";
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

/*
	Copyright (c) 1988-90, SecureWare, Inc. All Rights Reserved.

	dbck_mac(1M)-tag/IR database dump program

	This program works in conjunction with dbck(1M) to display the
	tags and the associated external representations for a policy
	database. The dbck(1M) program, when run with the -v option
	will create a dump file in the current directory ".dbck_debug"
	which can be used as input to this program. The dump file has
	all of the tag/IR pairs which this program converts to the
	external labels. The dbck(1M) program is policy independent.
*/


/*
 * Based on:

 */

#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "secpolicy_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SECPOLICY,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_ARCH /*{*/
#include <sys/types.h>
#include <sys/security.h>
#include <stdio.h>
#include <fcntl.h>
#include <mandatory.h>

#define DEBUG	".dbck_debug"

char ir[8192];

int recsize;
int irsize;
tag_t tag;

main()
{
	int debug_fd;
	char *er;

#ifdef NLS
	(void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
	catd = catopen(MF_SECPOLICY,NL_CAT_LOCALE);
#endif
	if((debug_fd = open(DEBUG,O_RDONLY)) == -1) {
		perror(MSGSTR(DBCK_MAC_1, "Error on debug file open"));
		exit(1);
	}

	mand_init();

	printf(MSGSTR(DBCK_MAC_2, "\n*** Primary index partition mappings. ***\n\n"));

	while(read(debug_fd,&recsize,sizeof(int)) == sizeof(int)) {

	   if(read(debug_fd,&tag,sizeof(tag_t)) != sizeof(tag_t)) {
		perror(MSGSTR(DBCK_MAC_3, "Error on read of tag"));
		exit(1);
	   }

	   if(tag == 0xffffffff) {
		printf(MSGSTR(DBCK_MAC_4, "\n*** Alternate index partition mappings. ***\n\n"));
		continue;
	   }

	   irsize = recsize - sizeof(tag_t);

	   if(read(debug_fd,ir,irsize) != irsize) {
		perror(MSGSTR(DBCK_MAC_5, "Error on read of ir"));
		exit(1);
	   }

#if SEC_ILB
	   if (irsize == ilb_bytes())
		er = ilb_ir_to_er(ir);
	   else
#endif
		er = mand_ir_to_er(ir);
	
	   if (er == (char *) 0) {
		fprintf(stderr,MSGSTR(DBCK_MAC_6, "Error on ir conversion for %x\n"),tag);
		continue;
	   }

	   printf(MSGSTR(DBCK_MAC_7, "Tag: %x	Label: %s\n"),tag,er);
	}

	return;
}

#endif /*} SEC_ARCH */
