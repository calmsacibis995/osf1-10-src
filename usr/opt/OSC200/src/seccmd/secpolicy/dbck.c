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
static char *rcsid = "@(#)$RCSfile: dbck.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/07 15:07:17 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	dbck.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.5.2.2  1992/06/10  20:36:10  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/10  20:28:58  hosking]
 *
 * Revision 1.5  1991/01/07  13:05:28  devrcs
 * 	Cleanup ifdef NLS
 * 	[90/11/27  08:34:30  aster]
 * 
 * Revision 1.4  90/10/07  15:42:02  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  13:04:45  gm]
 * 
 * Revision 1.3  90/07/17  11:48:16  devrcs
 * 	Merged with SecureWare osc14
 * 	[90/07/10  13:13:14  staffan]
 * 
 * $OSF_EndLog$
 */

/*
	Copyright (c) 1988-90, SecureWare, Inc.   All Rights Reserved.

	dbck(1M)-Database Consistency Check Program

	This program traverses the database specified and matches each
	database tag to the corresponding internal representation
	record. If a tag does not have a corresponding ir, this is
	recorded and potentially corrected. Likewise, an ir that does
	not have a tag is similarly handled. The verbose option -v
	allows a report of tag to ir combinations that exist in the
	database to be reported. The -y and -n options work like the
	options used for fsck(1M) to effect repair or treat as read-only
	or query the administrator.

	-v	verbose-list the tag/ir pairs
	-y	repair the database automatically
	-n	treat the database as read-only

*/

/* #ident "@(#)dbck.c	3.2 09:40:06 6/15/90 SecureWare" */
/*
 * Based on:
 *   "@(#)dbck.c	2.2.1.1 17:51:20 2/2/90 SecureWare"
 */

#include "sys/secdefines.h"
#include <locale.h>
#include "secpolicy_msg.h" 

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SECPOLICY,n,s) 


#if SEC_ARCH /*{*/

#include "sys/types.h"
#include "sys/security.h"
#include "sys/secpolicy.h"
#include "spdbm.h"
#include "prot.h"

#include "stdio.h"
#include "fcntl.h"
#include "malloc.h"

/* to align on power-of-two boundaries */

#include "sys/param.h"
#ifndef roundup
#define roundup(x,y)	((((x)+((y)-1))/(y))*(y))
#endif

/* If no file is specified, pick up databases from "dbtab" file. */

#define DBMAX	12
#define DBNAME_MAX	80

#define DBTAB	"/tcb/files/dbtab"
#define DEBUG	".dbck_debug"

FILE *dbtab, *debug;
int dbase_files = 0;
char dbase_file[DBMAX][DBNAME_MAX];

int debug_fd = -1;
int tag_marker = 0;

char input[132];

/* Defined by the database manager program */

extern int dbm_fd;
extern int dbm_l2_indices;
extern struct database_header dbase_hdr;
extern daddr_t tag_level1 [DBM_IPAGESIZE / sizeof(daddr_t)];
extern daddr_t  ir_level1 [DBM_IPAGESIZE / sizeof(daddr_t)];
extern struct dbase_rechdr *spdbm_lookup();
extern int spdbm_insert();

/* Control values for Database page sizes */

int level1_pgsize = DBM_IPAGESIZE;
int level2_pgsize = 0;

int level1_entries = DBM_LEVEL1_INDICES;
int level2_entries = 0;

daddr_t *indexpage;
caddr_t datapage;
int	datapage_size = 0;

tag_t	ck_tag[SEC_TAG_SIZE];
char	*ck_ir;

int ir_count = 0;
int ir_unmatch = 0;
int tag_count = 0;
int tag_unmatch = 0;
int pair_count = 0;

/* Option controls */

#define QUERY		0
#define NO_CHECK	1
#define AUTO_CHECK	2

int mode = 0;
int checkmode = QUERY;
int buffers = 10;
int verbose = 0;

/*
	main()-process the options for the command and perform the
	database consistency checks and optional repairs

	The databse open will cause the tag and ir index pages to be
	loaded. These are then traversed looking for non-zero index
	entries. When found, the index page is read. It is then
	traversed looking for non-zero index values. When found, the
	page is read in a search of the page is made to extract all
	valid records. If a tag page, the ir from the record is used
	in a spdbm_lookup() to check for consistency. The same is done
	in the ir data page lookup case.
*/

main(argc,argv)
int argc;
char *argv[];
{
	int i, j, c, nflg = 0, yflg = 0, file_numb = 0;
	extern int optind, opterr;
	extern char *optarg;

	(void) setlocale( LC_ALL, "" );
	catd = catopen(MF_SECPOLICY,NL_CAT_LOCALE);

	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, MSGSTR(DBCK_1, "%s: need sysadmin authorization\n"),
			command_name);
		exit(1);
	}

	if (forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(DBCK_2, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}

	opterr = 0;
	while((c = getopt(argc,argv,"ynvf:")) != EOF) {

	   switch(c) {

	   case 'y':
		if(nflg) {
			fprintf(stderr,MSGSTR(DBCK_3, "usage: dbck [-y|-n] [-v] -f file\n"));
			exit(1);
		}
		yflg++;
		checkmode = AUTO_CHECK;
		break;

	   case 'n':
		if(yflg) {
			fprintf(stderr,MSGSTR(DBCK_3, "usage: dbck [-y|-n] [-v] -f file\n"));
			exit(1);
		}

		nflg++;
		checkmode = NO_CHECK;
		break;

	   case 'v':
		verbose = 1;
		break;

	   case 'f':
		strcpy(dbase_file[dbase_files++],optarg);
		break;

	   }
	}

	if((verbose) && (debug_fd == -1)) {
		if((debug_fd = open(DEBUG,O_RDWR | O_CREAT | O_TRUNC,
		   0600)) == -1) {
			perror(MSGSTR(DBCK_4, "Error opening debug file"));
			verbose = 0;
		}
	}

	if(dbase_files == 0) {
		if((dbtab = fopen(DBTAB,"r")) == NULL) {
			fprintf(stderr,MSGSTR(DBCK_3, "usage: dbck [-y|-n] [-v] -f file\n"));
			exit(1);
		}

	/* Read each dbtab line ignoring comments and blank lines and
	   converting each database file name placing it in the array
	   of the file names to be checked.			   */

		while(fgets(input,sizeof(input),dbtab) != NULL) {

			if((input[0] == '\n') || (input[0] == '#'))
				continue;

			if(sscanf(input,"%s",dbase_file[dbase_files]) != 1)
				continue;
			dbase_files++;
		}

		fclose(dbtab);

	/* Check for valid entries in the dbtab or exit */

		if(dbase_files == 0) {
			fprintf(stderr,MSGSTR(DBCK_5, "usage: dbck [-y|-n] [-v] -f file [-d debug]\n"));
			exit(1);
		}
	}

	if(checkmode == NO_CHECK)
		mode = DBM_RDONLY;
	else mode = DBM_RDWR;

	/* While files are left in the list, check them */

	while(file_numb < dbase_files) {

	   tag_count = tag_unmatch = ir_count = ir_unmatch = 0;

	/* Open the database to begin the checking */

	   if(spdbm_open(dbase_file[file_numb],mode,buffers) != 0) {
		printf(MSGSTR(DBCK_6, "Error on spdbm_open() of %s\n"),dbase_file[file_numb]);
		exit(1);
	   }

	   fprintf(stderr,MSGSTR(DBCK_7, "\n\t*** Database Check for: %s *** \n\n"),
		dbase_file[file_numb]);
	   file_numb++;

	/* The primary index pages are already loaded-use them */

	   level2_pgsize = dbase_hdr.pagesize;
	   level2_entries = dbm_l2_indices;

	/* Allocate the space for the index page scan */

	   if((indexpage = (daddr_t *) malloc(level2_pgsize)) == 
	     (daddr_t *)NULL) {
		fprintf(stderr,MSGSTR(DBCK_8, "Error on index page malloc\n"));
		exit(1);
	   }

	/* Allocate the space for the data page scan */

	   if((datapage = (char *) malloc(level2_pgsize)) == (char *) NULL) {
		fprintf(stderr,MSGSTR(DBCK_8, "Error on index page malloc\n"));
		exit(1);
	   }

	   datapage_size = level2_pgsize;

	/* Walk the database pages for the Record Tags first. For
	   each level 1 entry that is non-zero, read the index page
	   into memory and scan for non-zero datapage indices.	     */

	   fprintf(stderr,MSGSTR(DBCK_9, "\t*** Phase 1 Checking TAG to IR Mappings\n"));

	   for(i=0; i < level1_entries; i++) {

	      if(tag_level1[i] != (daddr_t) 0) {

		lseek(dbm_fd,tag_level1[i],0);
		if(read(dbm_fd,indexpage,level2_pgsize) == -1) {
			perror(MSGSTR(DBCK_10, "Error on index page read"));
			continue;
		}

	/* Scan the index page for non-zero data page pointers */

		for(j=0; j < level2_entries; j++) {

		   if(indexpage[j] != (daddr_t) 0) {

			if(read_page(indexpage[j],DBM_TAG_PAGE,j) != 0)
				continue;

			if(search_page(DBM_TAG,i,j) == 1) {
				lseek(dbm_fd,(long) tag_level1[i],0);
				if(write(dbm_fd,indexpage,level2_pgsize) == -1) {
					perror(MSGSTR(DBCK_11, "Error on index page write"));
					continue;
				}
			}
		   }
		}
	      }
	   }

	/* Walk the database beginning from the IR pages */

	   fprintf(stderr,MSGSTR(DBCK_12, "\t*** Phase 2 Checking IR to TAG Mappings\n"));

	   for(i=0; i < level1_entries; i++) {

	      if(ir_level1[i] != (daddr_t) 0) {

		lseek(dbm_fd,ir_level1[i],0);
		if(read(dbm_fd,indexpage,level2_pgsize) == -1) {
			perror(MSGSTR(DBCK_10, "Error on index page read"));
			continue;
		}

	/* Scan the index page for non-zero data page pointers */

		for(j=0; j < level2_entries; j++) {

		   if(indexpage[j] != (daddr_t) 0) {

			if(read_page(indexpage[j],DBM_IR_PAGE,j) != 0)
				continue;

			if(search_page(DBM_IR,i,j) == 1) {
				lseek(dbm_fd,(long) ir_level1[i],0);
				if(write(dbm_fd,indexpage,level2_pgsize) == -1) {
					perror(MSGSTR(DBCK_11, "Error on index page write"));
					continue;
				}
			}
		   }
		}
	      }
	   }

	/* Phase 3 Checks the High Database Offset Against Index Values */

	   fprintf(stderr,MSGSTR(DBCK_13, "\t*** Phase 3 Checking Index Value Ranges\n"));

	/* Check the tag index pages first */

	   for(i=0; i < level1_entries; i++) {

	      if(tag_level1[i] != (daddr_t) 0) {

		if(tag_level1[i] >= dbase_hdr.high_offset)
		    fprintf(stderr,MSGSTR(DBCK_14, "\t\tTAG Index page out of range-%d\n"),i);

		lseek(dbm_fd,tag_level1[i],0);
		if(read(dbm_fd,indexpage,level2_pgsize) == -1) {
			perror(MSGSTR(DBCK_15, "Error on tag index page read"));
			continue;
		}

	/* Scan the index page for non-zero data page pointers */

		for(j=0; j < level2_entries; j++) {

		   if(indexpage[j] > dbase_hdr.high_offset)
		      fprintf(stderr,MSGSTR(DBCK_14, "\t\tTAG Index page out of range-%d\n"),i);

		}
	      }
	   }

	/* Check the IR index pages next */

	   for(i=0; i < level1_entries; i++) {

	      if(ir_level1[i] != (daddr_t) 0) {

		if(ir_level1[i] >= dbase_hdr.high_offset)
			fprintf(stderr,MSGSTR(DBCK_16, "\t\tIR Index page out of range-%d\n"),i);

		lseek(dbm_fd,ir_level1[i],0);
		if(read(dbm_fd,indexpage,level2_pgsize) == -1) {
			perror(MSGSTR(DBCK_10, "Error on index page read"));
			continue;
		}

	/* Scan the index page for non-zero data page pointers */

		for(j=0; j < level2_entries; j++) {

		   if(indexpage[j] > dbase_hdr.high_offset)
			fprintf(stderr,MSGSTR(DBCK_16, "\t\tIR Index page out of range-%d\n"),i);

		}
	      }
	   }

	/* Output the tag and IR count */

	   fprintf(stderr,MSGSTR(DBCK_17, "\n\t*** Tag Count: %d, Tags Unmatched: %d\n"),
		tag_count,tag_unmatch);

	   fprintf(stderr,MSGSTR(DBCK_18, "\t*** IR  Count: %d, IRs  Unmatched: %d\n"),
		ir_count,ir_unmatch);

	   spdbm_close();

	}	/* end of while loop */
	exit(0);
}

/*
	read_page()-read the page that the file pointer is currently
	set to and re-read if the entire page was not read in.
*/

read_page(daddr,type,index)
daddr_t daddr;
int type, index;
{
	struct datapage_header *dphdr;
	int need;

	lseek(dbm_fd,(long) daddr,0);

	if(read(dbm_fd,datapage,level2_pgsize) == -1) {
		perror(MSGSTR(DBCK_19, "Error on data page read"));
		return(-1);
	}

	dphdr = (struct datapage_header *) datapage;

	/* Check Page Validity */

	if(dphdr->pgsize.pgtype != type) {

		fprintf(stderr,MSGSTR(DBCK_20, "\t\tIncorrect Page Type, Clear? "));

		if(doit() == 0)
			return(-1);

		/* clear the index page pointer */

		indexpage[index] = 0;
		if(write(dbm_fd,indexpage,level2_pgsize) == -1)
			perror(MSGSTR(DBCK_21, "Error on re-write of index page"));

		return(-1);
	}

	/* If the page has multiple extents, read the whole block */

	if(dphdr->pgsize.extent != 1) {
		need = level2_pgsize * dphdr->pgsize.extent;
		if(need > datapage_size) {
			free(datapage);
			if((datapage = (char *) malloc(need)) == (char *) NULL) {
				fprintf(stderr,MSGSTR(DBCK_22, "Error on page remalloc\n"));
				return(-1);
			}

			datapage_size = need;
		}

		lseek(dbm_fd,daddr,0);
		if(read(dbm_fd,datapage,need) == -1) {
			perror(MSGSTR(DBCK_19, "Error on data page read"));
			return(-1);
		}

	}

	return(0);
}

/*
	search_page()-search the page type specified located at datapage
	and perform the actions specified by the options.
*/

search_page(type,l1,l2)
int type, l1, l2;
{
	struct dbase_rechdr *recp;
	struct tag_header *thdr;
	struct ir_header *irhdr;
	int pagesize;
	int space_occupied;

	/* Set the page record header pointers based on page type */

	if(type == DBM_TAG) {
		thdr = (struct tag_header *) datapage;
		recp = (struct dbase_rechdr *) ((int)thdr + THDR_SIZE);
		pagesize = thdr->pgsize.pgsize - THDR_SIZE;
	}
	else {
		irhdr = (struct ir_header *) datapage;
		recp = (struct dbase_rechdr *) ((int)irhdr + IRHDR_SIZE);
		pagesize = irhdr->pgsize.pgsize - IRHDR_SIZE;
	}

	/* Search the page type and for each record, locate the counterpart
	   record in the alternate index partition. In case or error,
	   perform the action dictated by the caller.			*/

	if(type == DBM_TAG) {
	   while(pagesize > 0) {

	/* Sanity Check on the data page contents */

		if((recp->recsize <= 0) || (thdr->pgsize.extent == 0)) {
			fprintf(stderr,
			  MSGSTR(DBCK_23, "\t\tBad TAG DATAPAGE Format: %d-%d, Clear? "),l1,l2);

			if(doit() == 0)
				return(0);

			/* Clear the index pointer */

			indexpage[l2] = 0;
			return(1);	/* page modified */
		}

		tag_count++;

	/* If verbose option specified, then output the record pair */

		if(verbose)
			format_record(recp,0);

	/* Lookup the counterpart record and perform the required action */

		space_occupied = roundup(recp->recsize,4);

		if(spdbm_lookup(DBM_IR,(int)recp + RECSIZE,
		   recp->recsize - RECSIZE,0) == NULL_RECP) {

			tag_unmatch++;
			fprintf(stderr,
				MSGSTR(DBCK_24, "\t\tMissing IR for TAG: %x, Insert IR? "),
				recp->tag[0]);

			if(doit() == 1) {
				if(spdbm_insert(DBM_IR,recp->tag,
				   (int)recp + RECSIZE,
				   recp->recsize - RECSIZE,1) != -1) {
					pagesize -= space_occupied;
					recp = (struct dbase_rechdr *) 
					   ((int)recp + space_occupied);
					continue;
				}

				fprintf(stderr,MSGSTR(DBCK_25, "\t\tFailed Insert, Clear? "));
			}
			else fprintf(stderr,MSGSTR(DBCK_26, "\t\tClear? "));

			if(doit() == 0) {
				pagesize -= space_occupied;
				recp = (struct dbase_rechdr *) ((int)recp + 
					space_occupied);
				continue;
			}

		/* Attempt to the delete the tag entry */

			if(spdbm_delete(DBM_TAG,recp->tag,(int) recp + RECSIZE,
			   recp->recsize - RECSIZE,1) == -1)
				fprintf(stderr,MSGSTR(DBCK_27, "Failed to Clear Entry\n"));

		}

		pagesize -= space_occupied;
		recp = (struct dbase_rechdr *) ((int)recp + space_occupied);
	   }
	}
	else {
	   while(pagesize > 0) {

	/* Sanity Check on the data page contents */

		if((recp->recsize <= 0) || (irhdr->pgsize.extent == 0)) {
			fprintf(stderr,
			   MSGSTR(DBCK_28, "\t\tBad IR DATAPAGE Format: %d-%d, Clear? "),l1,l2);

			if(doit() == 0)
				return(0);

			/* Clear the index pointer */

			indexpage[l2] = 0;
			return(1);	/* page modified */
		}
		space_occupied = roundup(recp->recsize,4);

		ir_count++;

	/* If verbose option specified, then output the record pair */

		if(verbose)
			format_record(recp,1);

	/* Lookup the counterpart record and perform the required action */

		if(spdbm_lookup(DBM_TAG,(int)recp->tag,SEC_TAG_SIZE,0)
		   == NULL_RECP) {

			ir_unmatch++;
			fprintf(stderr,
				MSGSTR(DBCK_29, "\t\tMissing TAG for IR: %x, Insert TAG? "),
				recp->tag[0]);

			if(doit() == 1) {
				if(spdbm_insert(DBM_TAG,recp->tag,
				   (int)recp + RECSIZE,
				   recp->recsize - RECSIZE,1) != -1) {
					pagesize -= space_occupied;
					recp = (struct dbase_rechdr *) 
					   ((int)recp + space_occupied);
					continue;
				}

				fprintf(stderr,MSGSTR(DBCK_25, "\t\tFailed Insert, Clear? "));
			}
			else fprintf(stderr,MSGSTR(DBCK_26, "\t\tClear? "));

			if(doit() == 0) {
				pagesize -= space_occupied;
				recp = (struct dbase_rechdr *) ((int)recp + 
					space_occupied);
				continue;
			}

		/* Attempt to the delete the IR entry */

			if(spdbm_delete(DBM_IR,recp->tag,(int) recp + RECSIZE,
			   recp->recsize - RECSIZE,1) == -1)
				fprintf(stderr,MSGSTR(DBCK_27, "Failed to Clear Entry\n"));

		}

		pagesize -= space_occupied;
		recp = (struct dbase_rechdr *) ((int)recp + space_occupied);
	   }
	}

	return(0);	/* good page scan */

}

/*
	format_record()-format and output the contents of the record to
	the database pair file.
*/

format_record(recp,marker)
struct dbase_rechdr *recp;
int marker;
{
	tag_t tag_mark = 0xffffffff;
	int recsize;

	if((marker) && (tag_marker == 0)) {
		tag_marker = 1;
		recsize = sizeof(tag_t);
		write(debug_fd,&recsize,sizeof(int));
		write(debug_fd,&tag_mark,sizeof(tag_t));
	}

	/* Format the tag/IR pairs to the output file */

	recsize = sizeof(long) + (recp->recsize - RECSIZE);
	write(debug_fd,&recsize,sizeof(int));

	write(debug_fd,recp->tag,sizeof(tag_t));
	write(debug_fd,(int) recp + RECSIZE,recp->recsize - RECSIZE);
}

/*
	doit()-Check the mode word to determine if the check should or should
	not be performed. If necessary, query the user for a response.
*/

doit()
{
	if(checkmode == AUTO_CHECK)
		return(1);

	if(checkmode == NO_CHECK)
		return(0);

	if((gets(input) == NULL) || (input[0] != 'y'))
		return(0);

	return(1);
}
#endif /*} SEC_ARCH */
