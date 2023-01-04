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
static char	*sccsid = "@(#)$RCSfile: Message.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:04:32 $";
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

#ifdef SEC_BASE

/*
	filename:
		Message.c
	
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		provide mechanism to pick up text from an external file
		
	notes:
		message handler is not explicitly closed by the program 
		implicit program shutdown must free malloc'ed memory 
			and fclose message file

	global functions:

		void InitializeMessageHandling(message_file)
			assumes that no time check of message file is required
			otherwise direct hook into MessageOpen()
					
		void MessageOpen(char *message_file, long time_check)
			provides error messages printed out
			returns 0 on success, else coded failure #
			note that if time_check == -1, then time check is bypassed
			
		void LoadMessage(class_name, class_msg, class_text)
			char *class_name, 
				 ***class_msg, 
				 **class_text;
			call MessageLoadClasses(), provides error messages if failure
				  
		char **MessageLoadClasses(char *class, char **ptext)
			returns pointer on success, else NULL
			memory spaces to free() when finished are:
				array of pointers returned by function
				text area which *ptext points to
				
		void MessageClose()
			closes message file and free()s static data structure used
			to track messages
*/

/******** COMMON HEADER INFO *********/
/* Common C include files */
#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <ctype.h>
#include <fcntl.h>
#ifdef AUX
#include <string.h>
#else
#include <strings.h>
#endif
#include <time.h>
#include <sys/security.h>
#include <sys/audit.h>

#ifdef AUX
#ifndef rewinddir
#include <sys/dir.h>
#endif
#define dirent  direct
#else
#include <dirent.h>
#endif

extern void
	MemoryError();

extern char
	*malloc();
	
extern FILE
	*fopen();
	
extern int stricmp();

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
/******** END COMMON HEADER INFO *********/

#ifdef SEC_MAC
#define access(A,B) eaccess(A,B)
#endif /* SEC_MAC */

#define CHECK 19890301L
#define TAG_LENGTH 49

typedef struct {
	 char           c_tag[TAG_LENGTH+1];    /* help class */
	 long           c_pos;                  /* offset in message file */
	 unsigned int   c_len;                  /* length of class entry */
	 int            c_num;                  /* number of message entries */
} CLASS_REC;
static CLASS_REC *class;

static FILE *cfp;
int         num_class;

static char *msg_control_msg[] = {
/* 0*/ "MESSAGE ERROR 0000: Message file %s not found\n",
/* 1*/ "MESSAGE ERROR 0001: Message file %s access denied\n",
/* 2*/ "MESSAGE ERROR 0002: Message file %s corrupted\n",
/* 3*/ "MESSAGE ERROR 0003: Message file %s not up-to-date\n",
/* 4*/ "MESSAGE ERROR 0004: Message file is missing class %s\n",
/* 5*/ "Message file is missing",
/* 6*/ "Lack permission to access the message file",
/* 7*/ "Message file is corrupted",
/* 8*/ "Message file is not up-to-date",
"" 
};

extern void InitializeMessageHandling(),
			MessageOpen(),
			MessageClose();

extern char **MessageLoadClass();

static int  ClassCmp();

void 
InitializeMessageHandling(message_file) 
	char    *message_file;
{
	/* Argument of -1L bypasses time check */
	MessageOpen(message_file, -1L);
}

void 
MessageOpen(message_file, timecheck)
	char    *message_file;
	long    timecheck;
{
	long    check;
	int     rc;
	long    timecheck_msg;

	/* Does message file exist */
	if (access(message_file, 00))
		goto no_exist;
	
	/* Do we have permission to access the message file */
	if (access(message_file, 04))
		goto no_access;
	
	/* Open up message file */
	cfp = fopen(message_file, "r");
	if (! cfp) 
		goto no_access;
		
	/* Pick up MAGIC */
	rc = fseek(cfp, -(long) (sizeof(long)), SEEK_END);
	if (rc)
		goto corrupted;
	if (fread(&check, sizeof(long), 1, cfp) != 1)
		goto corrupted;
	if (check != CHECK)
		goto corrupted;
	
	/* Pick up and check time matches */
	rc = fseek(cfp, -(long) (sizeof(long) + sizeof(long)), SEEK_END);
	if (rc)
		goto corrupted;
	if (fread(&timecheck_msg, sizeof(long), 1, cfp) != 1)
		goto corrupted;
	if (timecheck != -1l && timecheck != timecheck_msg)
		goto bad_time_stamp;
	
	/* Pick up number of classes */
	rc = fseek(cfp, -(long) (sizeof(long)*2 + sizeof(int)), SEEK_END);
	if (rc)
		goto corrupted;
	if (fread(&num_class, sizeof(int), 1, cfp) != 1)
		goto corrupted;
	
	/* Pick up class structure */
	rc = fseek(cfp, 
				- (long) (num_class*sizeof(CLASS_REC) 
					  + sizeof(int)+sizeof(long)*2), 
				SEEK_END);
	if (rc)
	   goto corrupted;
	class = (CLASS_REC *) Malloc(sizeof(CLASS_REC) * num_class);
	if (! class)
	MemoryError();
	if (fread(class, sizeof(CLASS_REC), num_class, cfp) != num_class)
		goto corrupted;
	
	/* Good exit */
	return;

no_exist:
	fprintf(stderr, msg_control_msg[0], message_file);
	audit_no_resource(message_file, OT_REGULAR, msg_control_msg[5], ET_SYS_ADMIN);
	goto die;

no_access:
	fprintf(stderr, msg_control_msg[1], message_file);
	audit_no_resource(message_file, OT_REGULAR, msg_control_msg[6], ET_SYS_ADMIN);
	goto die;

corrupted:
	fprintf(stderr, msg_control_msg[2], message_file);
	audit_no_resource(message_file, OT_REGULAR, msg_control_msg[7], ET_SYS_ADMIN);
	goto die;

bad_time_stamp:
	fprintf(stderr, msg_control_msg[3], message_file);
	audit_no_resource(message_file, OT_REGULAR, msg_control_msg[8], ET_SYS_ADMIN);
	goto die;
	
die:
	sleep(5);  /* Give chance for info to hit screen before die */
	exit(1);
}

void 
LoadMessage(class_name, class_msg, class_text)
	char *class_name, 
		 ***class_msg, 
		 **class_text;
{
	*class_msg = MessageLoadClass(class_name, class_text);
	if (! *class_msg) {
		MessageClose();
		fprintf(stderr, msg_control_msg[4], class_name);
		sleep(5);  /* Give chance for info to hit screen before die */
		exit(1);
	}
}

char  **
MessageLoadClass(class_name, p_text)
	char        *class_name;
	char        **p_text;
{
	char        **messagep;   
	int         j,
				rc,
				class_number;
	unsigned int class_length,
				i;
	long        class_offset;
	char        *text,            
				*cp;
	CLASS_REC   *p_class_rec;
	
	/* Was the message file opened ? */
	if (! cfp) 
		return (char **) NULL;
		
	/* Find particular class record */
	p_class_rec = (CLASS_REC *) bsearch(class_name, class, num_class,
					                     sizeof(CLASS_REC), ClassCmp);
	if (! p_class_rec)
		return (char **) NULL;
	
	/* Initialize commonly used variables */
	class_length = p_class_rec->c_len;
	class_offset = p_class_rec->c_pos;
	class_number = p_class_rec->c_num;
	
	rc = fseek(cfp, class_offset, SEEK_SET);
	text = (char *) Malloc(sizeof(char) * class_length);
	if (! text)
		MemoryError();
		if (fread(text, class_length, 1, cfp) != 1) {
		free(text);
		return (char **) NULL;
	}    

	messagep = (char **) Malloc(sizeof(char*) * class_number);
	if (! messagep) {
		free (text);
		MemoryError();
	}
	j = 0;
	messagep[j++] = text;
	for(i = class_length, cp = text; 
		 i-- && j < class_number; 
		 cp++ 
	) {
		if (! *cp)
			messagep[j++] = cp + 1;
	} 
	*p_text = text;
	return messagep;
}

void 
MessageClose() 
{

	if (class) {
		free(class);
		class = NULL;
	}
	if (cfp) {
		fclose(cfp);
		cfp = (FILE *) NULL;
	}
}

static int 
ClassCmp(a, b) 
	char        *a;
	CLASS_REC   *b;
{
	return stricmp(a, b->c_tag);
}    

#endif /* SEC_BASE */
