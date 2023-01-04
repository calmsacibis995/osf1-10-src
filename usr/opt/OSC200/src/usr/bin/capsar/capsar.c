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
static char	*sccsid = "@(#)$RCSfile: capsar.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/11 16:00:30 $";
#endif 
/*
 */
/*
 * OSF/1 Release 1.0
 */

#include <stdio.h>
#include <locale.h> /*GAG*/
#include <sys/file.h>
#include <sys/types.h>
#include <capsar.h>

#include "capsar_msg.h" /*GAG*/
nl_catd scmc_catd;
#define MSGSTR(Num,Str) catgets(scmc_catd,MS_capsar,Num,Str)

#define MAXPATHLEN	1024


/*-------*\
 Constants
\*-------*/

#define	FALSE	0
int	HELP;
char	*malloc();
int	rflag;
char	*rindex();
char	*strcat();
int	tflag=0;
int	fflag=0;
int	xflag=0;
int	Tflag=0;
int	hflag=0;
int	cflag=0;
int	DOTSflag=0;
int	DDIFflag=0;
char	*progname;

main(argc, argv)
	int	argc;
	char	*argv[];
{
/*------*\
  Locals
\*------*/

	char	*cp,
		*usefil,
		filetag[BUFSIZ],
		mtype[BUFSIZ],
		**pp,**pp0,
		*pps;
	int	i,
		c,
		errflg=0,
		fd;
	MM	*m,*m0;
extern	int	optind;
extern	char	*optarg;

/*------*\
   Code
\*------*/

	(void) setlocale(LC_ALL,""); /*GAG*/
	scmc_catd = catopen(MF_CAPSAR,NL_CAT_LOCALE);

	progname = argv[0];	/* Get our name*/


	if (argc < 2)
		usage();


	while((c = getopt(argc,argv,"ctx:")) !=EOF){
		switch(c){
			case 'c':	/* create option */
				cflag++;
				break;
			case 't':	/* check option */
				tflag++;
				break;
			case 'x':	/* extract option */
				xflag++;
				switch(*optarg){
					case 'h': /* headers */
						hflag++;
						break;
					case 'T': /* headers */
						Tflag++;
						break;
					case 'D': /* DOTS document */
						DOTSflag++;
						break;
					default:
						errflg++;
						break;
				}
				break;
			default :
				errflg++;
				break;
		}
	}

	if(errflg)
		usage();

	if(((argc-optind) <1)){
		if(xflag || tflag )
			fflag=0,
			usefil = getcpy("-");	
		else if(cflag)
			fprintf(stderr,MSGSTR(M_MSG_0, /*GAG*/
				"%s: no file name specified\n"),
				progname);
			usage();
	}
	else 
		fflag++,
		usefil = argv[optind];

	

	if((DOTSflag + DDIFflag + Tflag + hflag) != 1 && xflag)
		usage();

	if(!xflag && (hflag || DOTSflag || DDIFflag))
		usage();

	if(!xflag && !cflag && !tflag)usage();
	if(xflag && cflag )usage();
	if(xflag && tflag)usage();

/*	capsar_setlogging(STDERR_LOGGING); */
	
	if(cflag){
		strcpy(filetag,usefil);
		m = capsar_create(usefil,NULL);
		if(!m){
			fprintf(stderr,MSGSTR(M_MSG_1, /*GAG*/
				"%s: create failed\n"),progname);
			exit(0);
		}
		if(capsar_unparse_file(m,fileno(stdout))== NOTOK){
			fprintf(stderr,MSGSTR(M_MSG_2, /*GAG*/
				"%s: unparse failed\n"),progname);
			exit(0);
		}
		capsar_Destroy(m);
		exit(1);
	}

	if(fflag){
		if(strcmp(usefil,"-")==0)
			fd = fileno(stdin);
		else {
			if((fd = open(usefil,O_RDONLY)) == -1){
				fprintf(stderr,MSGSTR(M_MSG_3, /*GAG*/
					"%s: unparse failed\n"),
					progname,argv[1]);
				exit(0);
			}
		}
	}
	else if(!fflag)
		fd = fileno(stdin);

	m=capsar_parse_file(fd,NOTSMTP);

	if(!m){
		fprintf(stderr,MSGSTR(M_MSG_4, /*GAG*/
			"%s: message parsing failed\n"), progname);
		exit(0);
	}
	if(capsar_limitations(m,mtype) == NOTOK){
		fprintf(stderr,MSGSTR(M_MSG_5, /*GAG*/
			"%s: illegal message type\n"), progname);
	}
	if(tflag){
		fputs(mtype,stdout);
		fputs("\n",stdout);
		capsar_Destroy(m);
		exit(1);
	}	

	if(xflag && hflag){
		if(m->message_type == MAIL_MESSAGE){
			pp = capsar_get_header_list(m);
			if(pp == NULL)exit(1);
			pp0 = pp;
			while(pps = *pp++){
				fputs(pps,stdout);
				free(pps);
			}
		}
		else{
			fprintf(stderr,MSGSTR(M_MSG_6, /*GAG*/
				"%s: bad mail message\n"), progname);
		}
		capsar_Destroy(m);
		exit(0);
	}

	m0=m;
	while (m0 != NULL){
		if(Tflag){
			if(strcmp(m0->body_type,BODY_TYPE_DEF)==0)
				capsar_extract(m0,stdout);
		}

		else if(DOTSflag){
			if(strcmp(m0->body_type,DOTSTAG)==0)
				capsar_extract(m0,stdout);
		}

		else if(DDIFflag){
			if(strcmp(m0->body_type,DDIFTAG)==0)
				capsar_extract(m0,stdout);
		}
		m0 = m0->mm_next;
	}

	capsar_Destroy(m);
	
	
}
usage()
{
	fprintf(stderr,  MSGSTR(M_MSG_7, /*GAG*/
		"%s: usage: capsar [-c] [-t] [-x[hTD]] [file]\n"),
		progname);
	exit(1);
}
