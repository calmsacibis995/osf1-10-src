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
static char *rcsid = "@(#)$RCSfile: mkdb.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/07 15:07:50 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	mkdb.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.7.2.2  1992/06/11  16:05:39  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:05:07  hosking]
 *
 * Revision 1.7  1991/07/18  10:39:02  devrcs
 * 	Cleanup ifdef NLS
 * 	[91/07/08  04:36:17  aster]
 * 
 * Revision 1.6  91/03/23  17:21:31  devrcs
 * 	Merge up changes from 1.0.1
 * 	[91/03/11  16:10:34  seiden]
 * 
 * 	Make sure pagesize is at least 1024.
 * 	[91/02/21  15:45:16  seiden]
 * 
 * Revision 1.4  90/10/07  15:42:21  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  13:05:26  gm]
 * 
 * Revision 1.3  90/07/17  11:48:39  devrcs
 * 	Merged with SecureWare osc14
 * 	[90/07/10  13:14:44  staffan]
 * 
 * $OSF_EndLog$
 */

/*

	Copyright (c), 1988-90, SecureWare, Inc. All Rights Reserved.

	Security Policy Database Initialization Program-mkdb(1M)

	This program will create a database structure in a raw disk
	partition or as part of a regular file. The only option is
	the maximum number of 512 byte blocks that the file may be
	extended. For regular files, this will control the size of
	the file and for partitions this will control the amount of
	the partition that is used.

	Arguments: Database Name
		   Data Page Size (in Bytes)
		   Data Page Free Space Threshold (20-80 percent)
		   Optional Maximum 512 Byte Offset for Extension
*/

/* #ident "@(#)mkdb.c	3.1 10:02:27 6/7/90 SecureWare" */
/*
 * Based on:
 *   "@(#)mkdb.c	2.1 11:50:23 1/25/89"
 */

#include <sys/secdefines.h>

#include <locale.h>
#include "secpolicy_msg.h" 

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SECPOLICY,n,s) 

#if SEC_ARCH /*{*/

#include <sys/types.h>
#include <sys/security.h>
#include <sys/secpolicy.h>
#include "spdbm.h"
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <stdio.h>

struct database_header dbase_hdr;
struct stat statbuf;

char buffer[DBM_PAGESIZE];

struct passwd *getpwnam();
struct group *getgrnam();

int fd = -1;

main(argc,argv)
int argc;
char **argv;
{
	struct passwd *pw;
	struct group *gr;
	int tcbuid, tcbgid, l2size;

	(void) setlocale( LC_ALL, "" );
	catd = catopen(MF_SECPOLICY,NL_CAT_LOCALE);

	if(argc < 4) {
		printf(MSGSTR(MKDB_1, "Usage: mkdb filename pagesize threshold [blocks]\n"));
		exit(1);
	}

	fd = open(argv[1],O_CREAT|O_TRUNC|O_RDWR,0660);
	if(fd == -1) {
		perror(MSGSTR(MKDB_2, "Error on open"));
		return;
	}

	fstat(fd, &statbuf);

	if((statbuf.st_mode & S_IFMT) == S_IFCHR)
		dbase_hdr.flags = (DBM_INITIALIZED | DBM_PARTITION);
	else
		dbase_hdr.flags = DBM_INITIALIZED;

	if((pw = getpwnam("tcb")) == NULL) {
		printf(MSGSTR(MKDB_3, "Unable to locate password entry for tcb\n"));
		exit(2);
	}

	tcbuid = pw->pw_uid;

	if((gr = getgrnam("tcb")) == NULL) {
		printf(MSGSTR(MKDB_4, "Unable to locate group entry for tcb\n"));
		exit(2);
	}

	tcbgid = gr->gr_gid;

	chown(argv[1],tcbuid,tcbgid);

	/* Zero the buffer to be used for initializing the file */

	pgzero(buffer,DBM_PAGESIZE);

	/* Build the database header from arguments */

	dbase_hdr.pkey.daddr = DBM_PKEY_OFFSET;
	dbase_hdr.akey.daddr = DBM_AKEY_OFFSET;
	dbase_hdr.slevel_count = 0;
	dbase_hdr.slevel_block = DBM_SLEVEL_OFFSET;
	dbase_hdr.mlevel_count = 0;
	dbase_hdr.mlevel_block = DBM_MLEVEL_OFFSET;
	dbase_hdr.primary_pagesize = DBM_PAGESIZE;
	dbase_hdr.tag_cache_entries = DBM_TCACHE_ENTRIES;
	dbase_hdr.pagesize = atoi(argv[2]);
	dbase_hdr.thresh_percent = atoi(argv[3]);

        if (dbase_hdr.pagesize < 1024) {
                printf(MSGSTR(MKDB_4A,"Pagesize must be at least 1024\n"));
                exit(1);
        }

	if((dbase_hdr.thresh_percent < 20) || (dbase_hdr.thresh_percent > 80)) {
		printf(MSGSTR(MKDB_5, "Threshold percentage should be in the range 20 - 80\n"));
		exit(1);
	}

	dbase_hdr.threshold = (dbase_hdr.pagesize * dbase_hdr.thresh_percent)
		/ 100;

	if(argc > 4)
		dbase_hdr.max_filesize = atoi(argv[4]) * 512;

	l2size = ((dbase_hdr.pagesize / sizeof(daddr_t)) / DBM_NBPL) * 
		DBM_LEVEL1_INDICES * 4;

	/* Calculate the first available index or data page offset */

	dbase_hdr.l2size = l2size;
	dbase_hdr.high_offset = ((DBM_L2_MASK_OFFSET + l2size) +
		(DBM_PAGESIZE - 1)) & ~(DBM_PAGESIZE - 1);

	/* Output information on database build including computed values
	   for the high page offset and level 2 mask size.		*/

	printf(MSGSTR(MKDB_6, "\n\n\t*** Security Policy Database Initialization ***\n\n"));

	printf(MSGSTR(MKDB_7, "\t\tDatabase Name:\t\t%s\n"),argv[1]);
	printf(MSGSTR(MKDB_8, "\t\tIndex Page Size:\t%d\n"),DBM_IPAGESIZE);
	printf(MSGSTR(MKDB_9, "\t\tData Page Size:\t\t%d\n"),dbase_hdr.pagesize);
	printf(MSGSTR(MKDB_10, "\t\tPage Threshold:\t\t%d\n"),dbase_hdr.threshold);
	printf(MSGSTR(MKDB_11, "\t\tHigh Offset:\t\t%d (%x)\n"),dbase_hdr.high_offset,
		dbase_hdr.high_offset);
	printf(MSGSTR(MKDB_12, "\t\tLevel 2 Size:\t\t%d\n"),dbase_hdr.l2size);
	printf("\n");

	/* Zero the file from the beginning of the database to the highest
	   offset that will be used for control pages. The high offset value
	   computed for the database header marks the first file offset
	   that will be available for index and data pages.		*/

	clear_database(dbase_hdr.high_offset);

	/* Write the database header to the file */

	lseek(fd,(long) DBM_HEADER_OFFSET, 0);
	if(write(fd,&dbase_hdr,sizeof(struct database_header)) == -1) {
		perror(MSGSTR(MKDB_13, "Error on initializing control pages."));
		exit(1);
	}
	close(fd);
	exit(0);
}

/* Zero the buffer to be used for initializing database pages */

pgzero(src,count)
char *src;
int count;
{
	long *longword = (long *) src;
	int words = count / 4;
	int i;

	for(i=0; i < words; i++, longword++)
		*longword = 0;
}

/* Clear the control section of the database */

clear_database(high_offset)
int high_offset;
{
	int pages = high_offset / DBM_PAGESIZE;
	int remainder = high_offset % DBM_PAGESIZE;
	int i;

	/* Zero the control section a page at a time */

	lseek(fd,0L,0);		/* beginning oof the database */

	for(i=0; i < pages; i++) {
		if(write(fd,buffer,DBM_PAGESIZE) == -1) {
			perror(MSGSTR(MKDB_13, "Error on initializing control pages."));
			exit(1);
		}
	}

	/* Initialize the remainder of the dbase-partial page */

	if(write(fd,buffer,remainder) == -1) {
		perror(MSGSTR(MKDB_14, "Error on initializing remainder of control pages."));
		exit(1);
	}
}
#endif /*} SEC_ARCH */
