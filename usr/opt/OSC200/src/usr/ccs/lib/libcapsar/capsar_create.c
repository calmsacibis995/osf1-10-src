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
/*LINTLIBRARY*/

#ifndef lint
static char *sccsid = "@(#)$RCSfile: capsar_create.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/09/21 18:51:56 $";
#endif lint

/*
 * capsar_create.c 01/25/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part Messages in Ultrix Mail.
 *
 * syntax : MM  *capsar_create ( char *source , *name )
 *
 * this routine creates a message from the file or directory "source".
 * The file (or files) are checked in the /usr/lib/capsar database to
 * find out how to encode them for the flat representation. The create
 * routine knows about directories and creates a COMPOUND_MESSAGE for
 * each directory to hold the files in that directory; individual
 * files are held in a SIMPLE_MESSAGE. If name is non null, it it used
 * to name the message.
 *
 * Modification History:
 * 000 - Gary A. Gaudet - Tue Jun 25 14:16:08 EDT 1991
 *	Added casts to placate compiler
 * AJ01 - Aju John - Thu Dec 5, 1991
 *      Added -s option to ctod 
 */

/*	Generic Includes
 */
#include <stdio.h>
#include <locale.h> /*GAG*/
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/param.h>
#include <syslog.h>
#include "./dtif.h"
#include "libcapsar_msg.h" /*GAG*/

/*	Capsar Specific Includes
*/
#include <capsar.h>

#ifndef	TRUE
#define	TRUE	(1)
#endif

/* Variables which we allow to be externally accessible to the user */
int	CapsarResolveDdifExternals = TRUE;

MM *capsar_create(source,name)
char	*source;	/* file or directory */
char	*name;		/* name of message */
{
/*------*\
  Locals
\*------*/

	struct	stat	stbuf;		/* for holding stat information */
	MM		*m;		/* mail structure */
	char		*concat(),	/* concatenate strings */
			*code(),
			*codetag,
			*r1bindex(),
			*tagfld,
			*match(),
			buf[BUFSIZ];
	int		DDIFCNT=0,
			DOTSCNT=0,
	                DTIFCNT=0,
			dotit=0,
			ifile;			
	
	/* get file status from argument descriptor */
	if(stat(source, &stbuf) < 0){
		switch (errno) {
			case EACCES:
				(void) error_create(M_MSG_5,"search permission denied to %s\n",source); /*GAG*/
				return(NULL);
			case ENOENT:
				(void) error_create(M_MSG_6,"%s does not exist\n",source);	 /*GAG*/
				return(NULL);
			default:
				(void) error_create(M_MSG_7,"cannot stat %s\n",source); /*GAG*/
				return(NULL);
		}/*E of switch */
	}/* E of if fstat (...) < 0 */
	
	if(! name) name = getcpy(source);

	/* hard wired the ddif/dots recognition code into capsar
	instead of execing the file(1) command */

	if((ifile = open(source,0)) <0 ){
		(void) error_create(M_MSG_8,"cannot open  %s\n",source); /*GAG*/
		return(NULL);
	}

	if(read (ifile, buf, sizeof (buf)) < sizeof (int)) {
		close (ifile);
		(void) error_create (M_MSG_9,"read error on %s\n",source); /*GAG*/
		return (NULL);
	}
	close (ifile);	/* finished with it now */

	switch(*(int *) buf){
		case DDIF_MAGICNO:
			DDIFCNT++;
			tagfld = DOTSTAG;
			dotit++;
			break;
		case DTIF_MAGICNO:
			DTIFCNT++;
			tagfld = DOTSTAG;
			dotit++;
			break;
		case DOTS_MAGICNO:
			DOTSCNT++;
			tagfld = DOTSTAG;
			break;
		default:
			(void) error_create(M_MSG_10,"%s is not a DDIF/DOTS file\n",source); /*GAG*/
			return(NULL);
	}

	if(DDIFCNT || DTIFCNT || DOTSCNT){
		m = (MM *)malloc((unsigned ) sizeof *m);
		if(m == NULL){
			(void) error_create(M_MSG_11,"unable to allocate mail structure\n",NULL); /*GAG*/
			return(NULL);
		}
		capsar_setup(m);
		if((stbuf.st_mode & S_IFMT)== S_IFREG){
			m->size = stbuf.st_size;
			if((codetag=code(source,stbuf.st_size,m,dotit)) == NULL){
					(void) error_create(M_MSG_12,"capsar_create failed\n",NULL); /*GAG*/
					return(NULL);
			}
			m->message_type = SIMPLE_MESSAGE;
			m->body_type = concat(tagfld,NULLCP);
			m->name = concat(r1bindex(name,'/'),NULLCP);
			m->start = concat(r1bindex(name,'/')," : ",tagfld,".",codetag," message \n",NULLCP);
			m->stop = concat("End of ",m->start,NULLCP);
			return(m);
		}
	}
	return(NULL);
}

/*char	create_file[] = "/tmp/createXXXXXX";*/
char	create_file[80];

char *code(source,orig_size,m,dotit)
char		*source;
int		orig_size;
MM		*m;
int		dotit;
{
	char	*command, *command_default;
	char	*concat();
	FILE	*pipeptr;
	int	new_size=0;
	char	*dataptr;
	char	*realloc();
	char	source1[80];
	char	*tag;
	char	*fp;
	int	nocomp=0;
	FILE	*pwd;
	char	Home[MAXPATHLEN];
	char	Pathname[MAXPATHLEN+1];
	char	*cp,*sp,*rindex();
	char	*Fname;
	struct	stat stbuf;
	char	buf[BUFSIZ];

	pwd = popen("/bin/pwd","r");
	if(pwd < (FILE*)0){ /* GAG - 000 */
		error_create(M_MSG_13,"/bin/pwd",NULL); /*GAG*/
		return(NULL);
	}
	if(fgets(Home,sizeof Home,pwd) == NULL){
		error_create(M_MSG_14,"fgets",NULL); /*GAG*/
		return(NULL);
	}
	(void)pclose(pwd);
	Home[strlen(Home)-1] = '\0';

	(void) strcpy(Pathname,source);
	if(cp = rindex(Pathname,'/')){
		sp = cp+1;
		*cp = '\0';
		if(chdir(*Pathname ? Pathname : "/") == -1){
			(void) error_create(M_MSG_15,"bad starting directory\n",NULL); /*GAG*/
			return(NULL);
		}
		*cp = '/';
	}
	else sp =0;

	Fname = sp ? sp : Pathname;
	(void) strcpy(source1,Fname);
	
	if(dotit){
		if(access(CTOD,X_OK) == NOTOK){
			(void) error_create(M_MSG_16,"cannot access cmd : %s\n",CTOD); /*GAG*/
			return(NULL);
		}
		strcpy(create_file,"/tmp/createXXXXXX");
		(void)unlink(mktemp(create_file));
		command_default = concat ( CTOD, " -s ", NULLCP); /* AJ 01 */
		command = concat(
				command_default,
				CapsarResolveDdifExternals ? " " : " -x ",
				Fname,
				" > ",
				create_file,
				NULLCP
		);

		/* replaced code that used popen and pclose since the
		   output file was not found after pclose. The system call
		  does a wait until the child is complete
		*/
		if ((system(command)) < 0)
		{
			(void) error_create(M_MSG_17,"cannot execute command \n %s\n",command); 
			if(access(create_file,F_OK) == OK)
				(void) unlink(create_file);
			return(NULL);
		}

		if( stat (create_file, &stbuf) < 0 || stbuf.st_size <= 0){
			error_create(M_MSG_18,"%s failed  stbuf.st_size = %ld\n",CTOD,(long)(stbuf.st_size)); /*GAG*/
			return(NULL);
		}
		
		orig_size = stbuf.st_size;

		Fname = concat(create_file,NULLCP);	

		free(command);
				
	}
	if(access(CAT,X_OK) == NOTOK){
		(void) error_create(M_MSG_19,"cannot access cmd : %s\n",CAT); /*GAG*/
		return(NULL);
	}
	if(dotit)
		fp = concat(CTOD,NULLCP);
	else 
		fp = concat(CAT,NULLCP);

	if(access(COMPRESS_CMD,X_OK) == NOTOK)
		nocomp++;

	if(access(UUENCODE_CMD,X_OK) == NOTOK){
		(void) error_create(M_MSG_19,"cannot access cmd : %s\n",UUENCODE_CMD); /*GAG*/
		return(NULL);
	}

	if(!nocomp){
		tag = concat(r1bindex(fp,'/'),".",r1bindex(COMPRESS_CMD,'/'),".",r1bindex(UUENCODE_CMD,'/'),NULLCP);
		command = concat(CAT," ",Fname," | ", COMPRESS_CMD," | ",UUENCODE_CMD," ",source1,NULLCP);
	}
	else {
		tag = concat(r1bindex(fp,'/'),".",r1bindex(UUENCODE_CMD,'/'),NULLCP);
		command = concat(CAT," ",Fname," | ",UUENCODE_CMD," ",source1,NULLCP);
	}

	if((pipeptr=popen(command,"r")) <= (FILE*)0 ){ /* GAG - 000 */
		(void) error_create(M_MSG_17,"cannot execute command\n %s\n",command); /*GAG*/
		return(NULL);
	}
	
	dataptr = (char *)malloc((unsigned ) orig_size);
	if(dataptr == NULL){
		(void) error_create(M_MSG_20,"cannot allocate memory\n",NULL); /*GAG*/
		return(NULL);
	}
	
	while( fgets(buf,sizeof buf,pipeptr) !=NULL ){
		if(new_size + strlen(buf) >= orig_size){
			dataptr = (char *)realloc(dataptr,orig_size + sizeof buf);
			if(dataptr == NULL){
				(void)error_create(M_MSG_21,"cannot reallocate any space\n",NULL); /*GAG*/
				return(NULL);
			}
			orig_size = orig_size + sizeof buf;
		}
		bcopy(buf, dataptr + new_size, strlen(buf));
		new_size += strlen(buf);
	}

	if(new_size <=0){
		(void) error_create(M_MSG_22,"encoding error on ddif/dots document\n",NULL); /*GAG*/
		return(NULL);
	}

	m->size = new_size;
	m->dataptr = (char *)realloc(dataptr,(unsigned int) new_size);
	if(m->dataptr==NULL){
		(void) error_create(M_MSG_21,"cannot reallocate space\n",NULL); /*GAG*/
		return(NULL);
	}

	(void) free(command);

	(void)pclose(pipeptr);

	if(chdir(Home) == -1){
		(void) error_create(M_MSG_23,"bad change to home directory\n"); /*GAG*/
		return(NULL);
	}

	if(access(create_file,F_OK) == OK)
		(void)unlink(create_file);

	return(tag);	
}

error_create(msg_num,s1,s2) /*GAG*/
int msg_num;
char	*s1,*s2;
{
	capsar_log(LOG_INFO,msg_num,s1,s2); /*GAG*/
	if(access(create_file,F_OK) == OK)
		(void) unlink(create_file);
}
