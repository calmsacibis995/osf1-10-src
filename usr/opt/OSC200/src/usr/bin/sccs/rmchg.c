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
static char	*sccsid = "@(#)$RCSfile: rmchg.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/11 19:06:52 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: ckmrs, clean_up, esccmfdelt, escdodelt, fredck, 
 *            getvalflag, msg, put_delmrs, putmrs, rdpfile, rmchg,
 *            split_mrs, testfred, verif, main
 *            
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * rmchg.c 1.7 com/cmd/sccs/cmd,3.1,9021 1/4/90 18:10:39";
 */


# include <locale.h>
# include <nl_types.h>
# include "defines.h"
# include <stdlib.h>
# include <unistd.h>
# include "had.h"
# include "rmchg_msg.h"

#define MSGSTR(Num, Str) catgets(catd, MS_RMCHG, Num, Str)
char *getenv();
char *strncpy();

#ifdef CASSI
# include "filehand.h"
#endif

/*
	Program to remove a specified delta from an SCCS file,
	when invoked as 'rmdel',
	or to change the MRs and/or comments of a specified delta,
	when invoked as 'cdc'.
	(The program has two links to it, one called 'rmdel', the
	other 'cdc'.)

	The delta to be removed (or whose MRs and/or comments
	are to be changed) is specified via the
	r argument, in the form of an SID.

	If the delta is to be removed, it must be the most recent one
	in its branch in the delta tree (a so-called 'leaf' delta).
	For either function, the delta being processed must not
	have any 'delivered' MRs, and the user must have basically
	the same permissions as are required to make deltas.

	If a directory is given as an argument, each SCCS file
	within the directory will be processed as if it had been
	specifically named. If a name of '-' is given, the standard
	input will be read for a list of names of SCCS files to be
	processed. Non SCCS files are ignored.
#ifdef CASSI
	If the file is CASSI controlled special processing is performed:
	if the cdc command is being executed then the cmrs for the delta are
	list with the choice given about which to delete. This choice is checked 
	against the fred file for validity. If the rmdel command is being performed
	then all cmrs for the delta are checked against fred, if there are any
	non editable cmrs then the change is rejected.
#endif
*/

# define COPY 0
# define NOCOPY 1

struct stat Statbuf;
char Null[1];
char ErrMsg[512];

struct sid sid;
int num_files;
char had[26];
char D_type;
#ifdef CASSI
char 	*Sidhold;
char	*testcmr[25];
#endif
char	*auxf();
char    *logname();
char	*Mrs;
char	*Comments;
char	*Darg[NVARGS];
char    *Earg[NVARGS], **eargv;
char	*NVarg[NVARGS];
extern char *Sflags[];
int D_serial;

nl_catd catd;

main(argc,argv)
int argc;
char *argv[];
{
	register int i;
	register char *p;
	int c;
	char *sid_ab();
        wchar_t wc_flag;

	extern rmchg();
	extern int Fcnt;
	extern int optopt, opterr;
        extern char *optarg;

        (void) setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	/*
	Set flags for 'fatal' to issue message, call clean-up
	routine, and terminate processing.
	*/
	Fflags = FTLMSG | FTLCLN | FTLEXIT;

#ifdef CASSI
#define OPTLIST ":r:m:y:z"
#else  
#define OPTLIST ":r:m:y:"
#endif 

	Mrs = "";

	opterr = 0;
        while ((c = getopt(argc, argv, OPTLIST)) != EOF) {
                if ((c == 'm' || c == 'y') &&
		    ((char *)strstr(argv[0], "rmdel") != (char *)NULL)) {
			sprintf(ErrMsg,MSGSTR(UNKKEYLTR,
			"Flag -%c is not valid. (cm1)\n"),c); /*MSG*/
			fatal(ErrMsg);
		}

        	switch (c) {
		case 'r': 
			p = optarg;
			if (!isdigit((int)*p)) {
                                sprintf(ErrMsg,MSGSTR(UNKKEYLTR,
				"Flag -%c is not valid. (cm1)\n"),c); /*MSG*/
                                fatal(ErrMsg);
			}
#ifdef CASSI
			Sidhold=p;
#endif
			chksid(sid_ab(p,&sid),&sid);
			break;

		case 'm':	/* MR entry */
			Mrs = optarg;
			repl(Mrs,'\n',' ');
			break;

		case 'y':	/* comment line */
			savecmt(optarg);
			break;

#ifdef CASSI
		case 'z':
			break;
#endif

                case ':':
			if (optopt == 'r') {
				fatal(MSGSTR(RNOSID,
                "Specify a SID with the -r flag.(rc11)\n"));  /* MSG */
                        } else if (optopt == 'm')  {
			   	repl(Mrs,'\n',' ');
                        }
                        c = optopt;
                        break;

                case '?':
			sprintf(ErrMsg,MSGSTR(UNKKEYLTR,
			"Flag -%c is not valid. (cm1)\n"), optopt);/*MSG*/
			fatal(ErrMsg);
                        break;
		}

		if (had[c - 'a']++) {
			sprintf(ErrMsg,MSGSTR(KEYLTRTWC,  
                        "Use the -%c flag only once on the command line. (cm2)\n"),c);/*MSG*/
			fatal(ErrMsg);
		}
	}
	num_files = argc - optind;

#ifdef CASSI
	if(HADM && HADZ)
	{
	      fatal(MSGSTR(NOTCOMCMR, 
                 "CASSI not compatible with CMRs on command line.\n"));/*MSG*/
	}
#endif

	if(num_files == 0)
		fatal(MSGSTR(MISSFLNAM, 
                   "Specify the file to process.(cm3)\n"));  /* MSG */
	if (!HADR)
		fatal(MSGSTR(MISSINGR, 
                   "Use the -r flag with this command.(rc1)\n"));  /* MSG */


	if (*(p = sname(argv[0])) == 'n')
		p++;
	if (equal(p,"rmdel"))
		D_type = 'R';		/* invoked as 'rmdel' */
	else if (equal(p,"cdc"))
		D_type = 'D';		/* invoked as 'cdc' */
	else
		fatal(MSGSTR(BADINVOKE,"\nYou cannot invoke rmdel or cdc except by the commands rmdel or cdc.(rc10)\n"));  /* MSG */
	if (! logname())
		fatal(MSGSTR(USERID, 
                  "\nThe /etc/passwd file is not accessible.(cm9)\n")); /*MSG*/

	setsig();

	/*
	Change flags for 'fatal' so that it will return to this
	routine (main) instead of terminating processing.
	*/
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;

	/*
	Call 'rmchg' routine for each file argument.
	*/
	for (i=optind; i<argc; i++)
		do_file(argv[i],rmchg);

	exit(Fcnt ? 1 : 0);
}


/*
	Routine that actually causes processing of the delta.
	Processing on the file takes place on a
	temporary copy of the SCCS file (the x-file).
	The name of the x-file is the same as that of the
	s-file (SCCS file) with the 's.' replaced by 'x.'.
	At end of processing, the s-file is removed
	and the x-file is renamed with the name of the old s-file.

	This routine makes use of the z-file to lock out simultaneous
	updates to the SCCS file by more than one user.
*/

struct packet gpkt;	/* see file s.h */
char line[BUFSIZ];

rmchg(file)
char *file;
{
	struct stats stats;	/* see file s.defines.h */
	struct stat sbuf;
	struct idel *dodelt();
	extern char *Sflags[];
	int n;
	char *p, *cp, *getline();
	int keep;
	extern char Pgmr[LOGSIZE];
	int fowner, downer, user;

	char		*sptr;

	if (setjmp(Fjmp))	/* set up to return here from 'fatal' */
		return;		/* and return to caller of rmchg */

	sinit(&gpkt,file,1);	/* initialize packet and open s-file */
	/*
	Lock out any other user who may be trying to process
	the same file.
	*/
	if (lockit(auxf(file,'z'),2,getpid()))
		fatal(MSGSTR(LOCKCREAT, 
                   "\nCannot lock the specified file.(cm4)\n"));  /* MSG */

	/*
	Flag for 'putline' routine to tell it to open x-file
	and allow writing on it.
	*/
	gpkt.p_upd = 1;

	/*
	Save requested SID for later checking of
	permissions (by 'permiss').
	*/
	gpkt.p_reqsid = sid;

	/*
	Now read-in delta table. The 'dodelt' routine
	will read the table and change the delta entry of the
	requested SID to be of type 'R' if this is
	being executed as 'rmdel'; otherwise, for 'cdc', only
	the MR and comments sections will be changed 
	(by 'escdodelt', called by 'dodelt').
	*/
	if (dodelt(&gpkt,&stats,&sid,D_type) == 0)
		fmterr(&gpkt);

	/*
	Get serial number of requested SID from
	delta table just processed.
	*/
	D_serial = sidtoser(&gpkt.p_reqsid,&gpkt);

	/*
	If SID has not been zeroed (by 'dodelt'),
	SID was not found in file.
	*/
	if (sid.s_rel != 0)
		fatal(MSGSTR(SIDNOEXIST, 
                  "\nThe SID you specified does not exist.\n\
\tUse the sact command to check the p-file for existing SID numbers. (cm20)\n"));  /* MSG */
	/*
	Replace 'sid' with original 'sid'
	requested.
	*/
	sid = gpkt.p_reqsid;

	/*
	Now check permissions.
	*/
	finduser(&gpkt);
	doflags(&gpkt);
	permiss(&gpkt);

	/*
	Check that user is either owner of file or
	directory, or is one who made the delta.
	*/
	fstat(fileno(gpkt.p_iop),&Statbuf);
	fowner = Statbuf.st_uid;
	copy(gpkt.p_file,line);		/* temporary for dname() */
	if (stat(dname(line),&Statbuf))
		downer = -1;
	else
		downer = Statbuf.st_uid;
	user = getuid();
	if (user != fowner && user != downer)
		if (!equal(Pgmr,logname())) {
		   sprintf(ErrMsg,MSGSTR(NOTOWNER, 
                     "\nMust be owner of file or %s to use this command.(rc4)\n"),Pgmr);/*MSG*/
		   fatal(ErrMsg);
		}

	/*
	For 'rmdel', check that delta being removed is a
	'leaf' delta, and if ok,
	process the body.
	*/
	if (D_type == 'R') {
		struct idel *ptr;
		for (n = maxser(&gpkt); n > D_serial; n--) {
			ptr = &gpkt.p_idel[n];
			if (ptr->i_pred == D_serial)
			    fatal(MSGSTR(NOTLEAF, "\nYou can remove only the most recently created delta on a branch, \n\
\tor the latest trunk delta if it has no branches. (rc5)\n"));  /* MSG */
		}

		/*
		   For 'rmdel' check that the sid requested is
		   not contained in p-file, should a p-file
		   exist.
		*/

		if (exists(auxf(gpkt.p_file,'p')))
			rdpfile(&gpkt,&sid);

		flushto(&gpkt,EUSERTXT,COPY);

		keep = YES;
		gpkt.p_chkeof = 1;		/* set EOF is ok */
		while ((p = getline(&gpkt)) != NULL) {
			if (*p++ == CTLCHAR) {
				cp = p++;
				NONBLANK(p);
				/*
				Convert serial number to binary.
				*/
				if (*(p = satoi(p,&n)) != '\n')
					fmterr(&gpkt);
				if (n == D_serial) {
					gpkt.p_wrttn = 1;
					if (*cp == INS)
						keep = NO;
					else
						keep = YES;
				}
			}
			else
				if (keep == NO)
					gpkt.p_wrttn = 1;
		}
	}
	else {
		/*
		This is for invocation as 'cdc'.
		Check MRs.
		*/
		if (Mrs && *Mrs) {
			if (!(p = Sflags[VALFLAG - 'a']))
				fatal(MSGSTR(MRNOTALD, "\nThe SCCS file specified does not allow MR numbers.(cm24)\n"));  /* MSG */
			if (*p && valmrs(&gpkt,p))
				fatal(MSGSTR(INVMRS, "\nUse a valid MR number or numbers.(cm25)\n"));  /* MSG */
		}

		/*
		Indicate that EOF at this point is ok, and
		flush rest of s-file to x-file.
		*/
		gpkt.p_chkeof = 1;
		while (getline(&gpkt))
			;
	}

	flushline(&gpkt,(struct stats *) 0);
	
#ifdef CASSI
	/* if z flag on and -fz exists on the sccs s.file 
		verify the deleted cmrs and send message
	*/
	if((HADZ && !Sflags[CMFFLAG - 'a']) ||
		(!(HADZ) && Sflags[CMFFLAG - 'a']))
		{
			fatal(MSGSTR(INVUSEZFLG, 
                            "\nInvalid use of 'z' flag\n CASSI incompatability\n"));  /* MSG */
		}
	else
		{
		if(HADZ)
			{
			/*now process the testcmr table and send messages*/
				if(!verif(testcmr,&gpkt))
				{
					fatal(MSGSTR(CNTRLFIL, "\nUneditable CMR for CASSI controlled file.\n"));  /* MSG */
				}
			}
		}

#endif
	/*
	Delete old s-file, change x-file name to s-file.
	*/
	stat(gpkt.p_file,&sbuf);
	rename(auxf(gpkt.p_file,'x'),gpkt.p_file);
	chown(gpkt.p_file,sbuf.st_uid,sbuf.st_gid);
	clean_up();
}


escdodelt(pkt)
struct packet *pkt;
{
	extern int First_esc;
	extern int First_cmt;
	extern int CDid_mrs;
	char	*p;
	char	*date_ba();
	extern char *Sflags[];
	extern time_t Timenow;

#ifdef CASSI
	static short int doneesc;
	/* the CMF -z option is on and MR numb being processed*/
	if((HADZ) && (pkt->p_line[1] == MRNUM) && (D_type == 'D') && (!doneesc))
		{
		 doneesc = 1;
		 if(!esccmfdelt(pkt))
			{
			 fatal(MSGSTR(BADRPLC, 
                            "\nBad cdc replace of CMRs (CASSI)\n"));  /* MSG */
			}
		 else
			return;
		}
	/* non cassi processing begins*/	
#endif
	if (pkt->p_line[1] == MRNUM) {		/* line read is MR line */
		p = pkt->p_line;
		while (*p)
			p++;
		if (*(p - 2) == DELIVER)
		        fatal(MSGSTR(DELTSPEC, "\nYou cannot remove this delta or change its commentary.\n\
\tThe delta contains an MR number which has been marked \"delivered\"\n\
\t(unchangeable).  (rc9)\n"));  /* MSG */
	}
	if (D_type == 'D' && First_esc) {	/* cdc, first time */
		First_esc = 0;
		getvalflag();
		dohist();
		if (Mrs && *Mrs) {
			/*
			 * if adding MRs then put out the new list
			 * from `split_mrs' (if any)
			*/

			split_mrs();
			putmrs(pkt);
			CDid_mrs = 1;	/* indicate that some MRs were read */
			eargv = Earg;
		}
	}

	if (pkt->p_line[1] == MRNUM) {		/* line read is MR line */
		if (!CDid_mrs)	/* if no MRs list from `dohist' then return */
			return;
		else
			/*
			 * check to see if this MR should be removed
			*/

			ckmrs(pkt,pkt->p_line);
	}
	else if (D_type == 'D') {               /* this line is a comment
						 * or the end of the entry
						 */
		if (First_cmt) {		/* first comment encountered */
			First_cmt = 0;
			/*
			 * if comments were read by `dohist' then
			 * put them out.
			*/

			if (*Comments)
				putline(pkt,Comments);

			/*
			 * if any MRs were deleted, print them out
			 * as comments for this invocation of `cdc'
			*/

			put_delmrs(pkt);
			/*
			 * if comments were read by `dohist' and
			 * there were previous comments then
			 * indicate that comments were CHANGED.
			*/

			if (*Comments && pkt->p_line[1] == COMMENTS) {
				sprintf(line,"%c%c ",CTLCHAR,COMMENTS);
				putline(pkt,line);
				putline(pkt,MSGSTR(CHANGED,"*** CHANGED *** "));
				/* get date and time */
				date_ba(&Timenow,line);	
				putline(pkt,line);
				sprintf(line," %s\n",logname());
				putline(pkt,line);
			}
		}
		else return;
	}
}


/* find VALFLAG for dohist */
getvalflag()
{
	char line[BUFSIZ];
	register char *p;
	FILE *in, *fdfopen();

	Sflags[VALFLAG - 'a'] = NULL;
	in = xfopen(gpkt.p_file,0);
	while ((p = fgets(line,sizeof(line),in)) != NULL)
		if (line[0] == CTLCHAR && line[1] == EUSERNAM)
			break;
	if (p) {
		while (fgets(line,sizeof(line),in) && line[1] == FLAG &&
		    line[0] == CTLCHAR)
			if (line[3] == VALFLAG) {
				Sflags[VALFLAG - 'a'] = Null;
				break;
			}
	}
	fclose(in);
}


#ifdef CASSI
/* esccmfdelt(pkt) takes a packet line of cassi cmrs 
*  prompts for those which will be deleted from the delta
*  checks the ones to be deleted against fred
*  it turns the old line into a comment line
*  and makes a new line containing the non deleted cmrs
*  a chpost message is sent for the deleted cmrs
*  whole function 
*/
	esccmfdelt(pkt)
		struct packet *pkt;
		{
 		 extern char *Sidhold; 
		 extern char *Sflags[];
		 extern char *gf();
		  char *strtok();
		 char *p,*pp,*pend,holder[100],outhold[110],answ[80],*holdptr[25],*fred,str[80];
		 int numcmrs,i,n,j,changed=0;		 
		 char *yesstr = "y";
		 char *nostr = "n";
		 /* move the cmr data to a holder*/
		 p = pkt->p_line;
		 p += 3;
		 strcpy(holder,p);
		 i=0;
		 outhold[0]=NULL;
		 holdptr[0]=strtok(holder,",\n");
		 while(holdptr[++i] = strtok(0,",\n"));
		 if(!holdptr[1])
			{
			 /* only one mr on the list */
			 printf(MSGSTR(NOCHNG,"\nOnly one CMR left for this delta no change possible.\n"));  /* MSG */
			 return(1);
			}
		 numcmrs = i;
		 n=0;
 		 j=0;
		 while (holdptr[j])
			{
			if (yesstr = getenv("YESSTR")) {
				i = strcspn(yesstr, ":");
				yesstr = strncpy(calloc(i+1,1),yesstr,i);
			} else	yesstr = "y";
			if (nostr = getenv("NOSTR")) {
				i = strcspn(nostr, ":");
				nostr = strncpy(calloc(i+1,1),nostr,i);
			} else	nostr = "n";
			 printf(MSGSTR(DELETEYN, 
			      "\nDo you want to delete %1$s (%2$s or %3$s)?\n"),
			 		holdptr[j],yesstr, nostr);  /* MSG */
			 gets(answ);
			 if(yesno(answ) == 1)
				{
				 if(numcmrs <= 1)
					{
						printf(MSGSTR(NOTEDITBL, "%s  is not editable now\n"),holdptr[j]);  /* MSG */
						 cat(outhold,outhold,holdptr[j],",",0);
						 break;
						}
					else
				 		{
						 /* store the cmrnumber in a test table*/
						 testcmr[n]=(char *)malloc((unsigned)strlen(holdptr[j]) +1);
						 strcpy(testcmr[n++],holdptr[j]);
						 numcmrs--;
						 changed=1;
						}
				}
			 else
				{
				 cat(outhold,outhold,holdptr[j],",",0);
				}
			j++;
			}
			if(!changed)
				{
				 return;
				}
			/* turn old line into comment */
			p -= 2;
			*p = 'c';
			pend=strend(p);
			*--pend='\n';
			putline(pkt,0);
			/* place new line */
			pp=strend(outhold);
			*pp=NULL;
			*--pp=NULL;
			sprintf(str,"%c%c %s\n",CTLCHAR,MRNUM,outhold);
			putline(pkt,str);
			return;
		}
/*the fredchk routine verifies that the cmrs for the delta are all editableor
*		 degenerate  before changing the delta*/
fredck(pkt)
struct packet *pkt;
{
	char *fred,dhold[20],*mrhold[20],*p,holder[80];
	extern char *Sidhold;
	int i,j,n;
	p = pkt->p_line;
	p += 3;
	strcpy(holder,p);
	mrhold[0]=strtok(holder,"\n,");
	i=0;
	while(mrhold[++i] = strtok(0,"\n,"));
	for(j=0;j<i;j++)
	{
		testcmr[j]=(char *)malloc((unsigned)strlen(mrhold[j]) + 1);
		strcpy(testcmr[j],mrhold[j]);
	}
}
/*verif takes the list of deleted cmrs and checks them agains the fredfile
  if any are invalid the function returns 0
*/
verif(test,pkt)
char *test[];
struct packet *pkt;
{
	char *fred;
	int i;
	/* get the fred file name*/
	if(!(fred=(char *)gf(Sflags[CMFFLAG - 'a'])))
		fatal(MSGSTR(NOFREDF,
			"\nNo fred file found- see sccs administrator\n"));	/*MSG*/
	/* check validity of values in test */
	for(i=0;test[i];i++)
		{
			if(!(testfred(test[i],fred)))
				{
					fatal(MSGSTR(NTEDITBLE, "\n%s is not editable \n"),test[i]);  /* MSG */
					return(0);
				}
		}
	/* write the messages for the valid cmrs */
	for(i=0;test[i];i++)
		{
			msg(Sflags[CMFFLAG - 'a'],pkt->p_file,test[i],"nc",Sidhold,fred);
		}
	return(1);
}

/*the  msg subroutine creates the command line to go to the mr subsystem*/
msg(app,name,cmrs,stats,sids,fred)
	char *app,*name,*cmrs,*stats,*sids,*fred;

	{
 	FILE *fd;
	char holdit[200],stt[200];
	 char pname[FILESIZE],*ptr,holdfred[100],dir[100],path[FILESIZE];
	 struct stat stbuf;
	 int noexist = 0;
	 char *k;
	extern char *strchar();
	/*if -fm its value is made the file name */
	if(k=Sflags[MODFLAG -'a'])
	{
		name = k;
	}
	if( *name != '/' )   /* NOT full path name */
	{
		curdir(path);
		cat(pname,path,"/",name,0);
	}
	else
	{
		strcpy(pname,name);
	}
	strcpy(holdfred,fred);
	ptr=(char *)strchr(holdfred,'.');
	*ptr=(char)NULL;
	 strcat(holdfred,"source");
	 strcpy(dir,holdfred);
	 strcat(holdfred,"/termLOG");
	 if(stat(holdfred,&stbuf) == -1)
		noexist = 1; /*new termLOG */
	if(!(fd=fopen(holdfred,"a")))
		{
			fatal(MSGSTR(MSGNTWRTBL, 
                           "\nCASSI msg not writable.\n"));  /* MSG */
			return(0);
		}
	fprintf(fd,"%s chpost %s q %s sw MID=%s MFS=%s MPA=%s q q\n",app,cmrs,pname,sids,stats,logname());
	fclose(fd);
	 if(noexist) /*new termLOG make owner of /BD/source owner of file */
	 {
		if(stat(dir,&stbuf) == -1)
		{
			fatal(MSGSTR(CASSIBD, 
                           "\nCASSI BD/source not writeable.\n"));  /* MSG */
		}
		chmod(holdfred,0666);
		chown(holdfred,(int)stbuf.st_uid,(int)stbuf.st_gid); 
	}
	return(1);
}
/* testfred takes the fredfile and the cmr and checks to see if the cmr is in 
*the fred  file
*/
testfred(cmr,fredfile)
	char *cmr,*fredfile;
	{
		extern sweep();
		char dcmr[50];
		int hold;
		char *cmrh[2],*dcmrh[2];
		cmrh[1] = (char *) NULL;
		dcmrh[1] = (char *) NULL;
		cat(dcmr,"D",cmr,0);
		cmrh[0]=cmr;
		dcmrh[0]=dcmr;
		return(sweep(SEQVERIFY,fredfile,NULL,'\n',WHITE,80,cmrh,NULL,NULL,
		(int(*)())NULL,(int(*)())NULL) == FOUND || sweep(SEQVERIFY,fredfile
		,NULL,'\n',WHITE,80,dcmrh,NULL,NULL,(int(*)())NULL,(int(*)())NULL)
		== FOUND);
	}
	

#endif  /* CASSI */

extern char **Varg;

split_mrs()
{
	register char **argv;
	char	**dargv;
	char	**nargv;
	char	*ap, *stalloc();

	if (Varg == NVarg) return;

	dargv = Darg;
	nargv = &NVarg[VSTART];
	for (argv = &Varg[VSTART]; *argv; argv++)
		if (*argv[0] == '!') {
			*argv += 1;
			ap = *argv;
			copy(ap,*dargv++ = stalloc(size(ap)));
			*dargv = 0;
			continue;
		}
		else {
			copy(*argv,*nargv++ = stalloc(size(*argv)));
			*nargv = 0;
		}
	Varg = NVarg;
}


putmrs(pkt)
struct packet *pkt;
{
	register char **argv;
	char	str[64];

	for(argv = &Varg[VSTART]; *argv; argv++) {
		sprintf(str,"%c%c %s\n",CTLCHAR,MRNUM,*argv);
		putline(pkt,str);
	}
}


clean_up()
{
	if(gpkt.p_iop)
		fclose(gpkt.p_iop);
	xrm(&gpkt);
	if (exists(auxf(gpkt.p_file,'x')))
		xunlink(auxf(gpkt.p_file,'x'));
	ffreeall();
	if (gpkt.p_file[0])
		unlockit(auxf(gpkt.p_file,'z'),getpid());
}


rdpfile(pkt,sp)
register struct packet *pkt;
struct sid *sp;
{
	struct pfile pf;
	char rline[BUFSIZ];
	FILE *in, *fdfopen();

	in = xfopen(auxf(pkt->p_file,'p'),0);
	while (fgets(rline,sizeof(line),in) != NULL) {
		pf_ab(rline,&pf,1);
		if (sp->s_rel == pf.pf_gsid.s_rel &&
			sp->s_lev == pf.pf_gsid.s_lev &&
			sp->s_br == pf.pf_gsid.s_br &&
			sp->s_seq == pf.pf_gsid.s_seq) {
				fclose(in);
				fatal(MSGSTR(BEINGEDIT, "\nAnother user is working on the delta you specified.(rc12)\n"));  /* MSG */
		}
	}
	fclose(in);
	return;
}


ckmrs(pkt,p)
struct packet *pkt;
char *p;
{
	register char **argv;
	char mr_no[BUFSIZ];
	char *mr_ptr, *fmalloc();

	copy(p,mr_no);
	mr_ptr = mr_no;
	repl(mr_ptr,'\n','\0');
	mr_ptr += 3;
	for (argv = Darg; *argv; argv++)
		if (equal(mr_ptr,*argv)) {
			pkt->p_wrttn = 1;
			copy(mr_ptr,*eargv++ = fmalloc(size(mr_ptr)));
			*eargv = 0;
		}
}


put_delmrs(pkt)
struct	packet	*pkt;
{

	register char	**argv;
	register int first;
	char	str[64];

	first = 1;
	for(argv = Earg; *argv; argv++) {
		if (first) {
			putline(pkt,"c *** LIST OF DELETED MRS ***\n");
			first = 0;
		}
		sprintf(str,"%c%c %s\n",CTLCHAR,COMMENTS,*argv);
		putline(pkt,str);
	}
}
