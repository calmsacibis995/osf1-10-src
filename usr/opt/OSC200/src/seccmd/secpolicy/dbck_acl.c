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
	Copyright (c) 1988-90, SecureWare, Inc. All Rights Reserved.

	dbck_acl(1M)-tag/IR database dump program

	This program works in conjunction with dbck(1M) to display the
	tags and the associated external representations for a policy
	database. The dbck(1M) program, when run with the -v option
	will create a dump file in the current directory ".dbck_debug"
	which can be used as input to this program. The dump file has
	all of the tag/IR pairs which this program converts to the
	external labels. The dbck(1M) program is policy independent.
*/

/* #ident "@(#)dbck_acl.c	3.2 11:26:47 6/7/90 SecureWare" */
/*
 * Based on:
 *   "@(#)dbck_acl.c	2.1.1.1 23:32:52 1/9/90 SecureWare, Inc."
 */

#include <sys/secdefines.h>
#include <locale.h>
#include "secpolicy_msg.h" 

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SECPOLICY,n,s) 

#if SEC_ACL /*{*/

#include "sys/types.h"
#include "stdio.h"
#include "fcntl.h"
#include "acl.h"
#include "sys/security.h"
#include "sys/secpolicy.h"


#define DEBUG	".dbck_debug"

char *er;
char ir[8192];

int recsize;
int irsize;
tag_t tag;
#if SEC_ACL_POSIX
int numents, i;
uid_t   ouid;
gid_t   ogid;
acle_t  *ent;
#endif


#if SEC_GROUPS
char		*acl_subj_ir_to_er();
extern char	*gr_idtoname(), *pw_idtoname();
#endif

main()
{
	int debug_fd;

	(void) setlocale( LC_ALL, "" );
	catd = catopen(MF_SECPOLICY,NL_CAT_LOCALE);

	if((debug_fd = open(DEBUG,O_RDONLY)) == -1) {
		perror(MSGSTR(DBCK_ACL_1, "Error on debug file open"));
		exit(1);
	}

	acl_init();

	printf(MSGSTR(DBCK_ACL_2, "\n*** Primary index partition mappings. ***\n\n"));

	while(read(debug_fd,&recsize,sizeof(int)) == sizeof(int)) {

	   if(read(debug_fd,&tag,sizeof(tag_t)) != sizeof(tag_t)) {
		perror(MSGSTR(DBCK_ACL_3, "Error on read of tag"));
		exit(1);
	   }

	   if(tag == 0xffffffff) {
		printf(MSGSTR(DBCK_ACL_4, "\n*** Alternate index partition mappings. ***\n\n"));
		continue;
	   }

	   recsize -= sizeof(tag_t);
	   if (recsize <= 0 || recsize > sizeof ir) {
		fprintf(stderr,MSGSTR(DBCK_ACL_5, "IR for tag %x too big (%d bytes)\n"),
			tag, recsize);
		lseek(debug_fd,recsize,1);
		continue;
	   }
	   if(read(debug_fd,ir,recsize) != recsize) {
		perror(MSGSTR(DBCK_ACL_6, "Error on read of ir"));
		exit(1);
	   }
#if SEC_GROUPS
	   /* Try first to convert to a subject er */
	   if ((er = acl_subj_ir_to_er((dacid_t *) ir,
			recsize / sizeof(short), 0)) == (char *) 0)
#endif
	   {
#if SEC_ACL_SWARE
		udac_t *dac = (udac_t *) ir;
		irsize = recsize - ACL_HEADERSIZE;
#endif
#if SEC_ACL_POSIX
                cdac_t *dac = (cdac_t *) ir;
                irsize = recsize - PACL_HEADERSIZE;
#endif
		if (irsize < 0)
		    er = acl_subj_ir_to_er((dacid_t *) ir,
				recsize / sizeof(short), 1);
		else
#if SEC_ACL_SWARE
		    er = acl_ir_to_er(ir + ACL_HEADERSIZE,irsize / sizeof(acle_t));
#endif
#if SEC_ACL_POSIX
                    er = acl_ir_to_er(ir + PACL_HEADERSIZE,irsize / sizeof(acle_t));
                    ent = (acle_t *)(ir + PACL_HEADERSIZE);
                    numents = irsize / sizeof(acle_t);
                    for (i=0; i < numents; i++, ent++) {
                        switch(ent->acl_tag) {
                                case USER_OBJ:
                                        ouid = ent->acl_uid;
                                        break;
                                case GROUP_OBJ:
                                        ogid = ent->acl_gid;
                                        break;
                        }
                }

#endif
                if (er == (char *) 0) {
                    fprintf(stderr,MSGSTR(DBCK_ACL_7,
			 "Error on ir conversion for %x\n"),tag);
                    continue;
                }
#if SEC_ACL_SWARE
	   	printf(MSGSTR(DBCK_ACL_8, "Tag: %8x	u %hd %hd g %hd %hd m 0%o Label: %s\n"),
		  tag, dac->uid, dac->cuid, dac->gid, dac->cgid, dac->mode, er);
#endif
#if SEC_ACL_POSIX
                /*
                 * If creator uid/gid set, this is an IPC access acl
                 * Print creator and owner uid/gid
                 */
                if ((dac->cuid != -3) && (dac->cgid != -3))
                        printf (MSGSTR(DBCK_ACL_12,
"IPC ACCESS ACL: Tag %8x cuid %hd uid %hd cgid %hd gid %hd Label:\n%s\n"),
                          tag,dac->cuid,ouid,dac->cgid,ogid,er);
                /*
                 * If creator uid/gid bogus and uid/gid set
                 * this is an non-IPC access acl
                 * Print owner uid/gid
                 */
                else if (( ouid != -1) && (ogid != -1))
                        printf (MSGSTR(DBCK_ACL_13,
	"ACCESS ACL: Tag %8x uid %hd gid %hd Label:\n%s\n"),
                          tag,ouid,ogid,er);
                /*
                 * If creator uid/gid bogus and uid/gid not set
                 * this is an default access acl
                 */
                else if (( ouid == -1) && (ogid == -1))
                        printf (MSGSTR(DBCK_ACL_14,
				"DEFAULT ACL: Tag %8x Label:\n%s\n"),
                          tag,er);
                else
                /*
                 *  If not caught earlier, we don't know what type of
                 *  acl this is, so print out all information.
                 */
                        printf(MSGSTR(DBCK_ACL_15,
"UNKNOWN ACL TYPE: Tag %8x cuid %hd uid %hd cgid %hd gid %hd Label:\n%s\n"),
                          tag,dac->cuid,ouid,dac->cgid,ogid,er);
#endif
	   } else
	   	printf(MSGSTR(DBCK_ACL_9, "Tag: %8x	Label: %s\n"),tag,er);
	}

	return;
}

#if SEC_GROUPS
/*
 * Convert an ACL subject IR into printable form.
 */
char *
acl_subj_ir_to_er(ir, len, badidsok)
	register dacid_t	*ir;
	int			len, badidsok;
{
	static char	erbuf[(NGROUPS_MAX + 2) * 9 + 2];
	register char	*name, *bp, delim;
	register int	i;

	/* Must have at least the uid and primary gid */
	if (len < 2 || len > NGROUPS_MAX + 2)
		return (char *) 0;

	bp = erbuf;
	if ((name = pw_idtoname(ir->uid)) == (char *) 0) {
		if (!badidsok)
			return (char *) 0;
		sprintf(bp, MSGSTR(DBCK_ACL_10, "uid %d,"), ir->uid);
	} else
		sprintf(bp, "%s,", name);
	bp += strlen(bp);

	if ((name = gr_idtoname(ir->gid)) == (char *) 0) {
		if (!badidsok)
			return (char *) 0;
		sprintf(bp, MSGSTR(DBCK_ACL_11, "gid %d "), ir->gid);
	} else
		sprintf(bp, "%s ", name);
	bp += strlen(bp);

	if (len == 0) {
		*--bp = '\0';
		return erbuf;
	}

	delim = '[';
	for (i = 0; i < len - 2; ++i) {
		if ((name = gr_idtoname(ir->groups[i])) == (char *) 0) {
			if (!badidsok)
				return (char *) 0;
			sprintf(bp, "%c%d", delim, ir->groups[i]);
		} else
			sprintf(bp, "%c%s", delim, name);
		bp += strlen(bp);
		delim = ',';
	}
	
	if (delim == ',')
		strcpy(bp, "]");
	return erbuf;
}
#endif /* SEC_GROUPS */

#endif /*} SEC_ACL */
