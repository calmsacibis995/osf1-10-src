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
static char	*sccsid = "@(#)$RCSfile: unget.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1993/10/11 19:00:42 $";
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
 * FUNCTIONS: catpfile, clean_up, edpfile, unget, main
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
 * unget.c 1.5 com/cmd/sccs/cmd,3.1,9021 1/4/90 18:11:25";
 */
#include 	<locale.h>
#include        <nl_types.h>
#include	"defines.h"
#include	"had.h"
#include 	"unget_msg.h"
#include 	<stdlib.h>
#include 	<unistd.h>
#define MSGSTR(Num, Str) catgets(catd, MS_UNGET, Num, Str)

/*
		Program can be invoked as either "unget" or
		"sact".  Sact simply displays the p-file on the
		standard output.  Unget removes a specified entry
		from the p-file.
*/

struct stat Statbuf;
char ErrMsg[512];

int	verbosity;
int	num_files;
int	cmd;
long	Szqfile;
char	had[26];
char	Pfilename[FILESIZE];
char	*auxf();
struct	packet gpkt;
struct	sid sid;
extern int opterr;
nl_catd catd;

main(argc,argv)
int argc;
char *argv[];
{
	int	i;
	char	*p;
	char	*sid_ab();
	wchar_t wc_flag;
	extern	unget();
	extern	int Fcnt;
	extern int optind;
	extern int opterr;
	int	c;		/* XPG4 */

        (void)setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	Fflags = FTLEXIT | FTLMSG | FTLCLN;

	if(strcmp(argv[0], "sact") == 0)
	        opterr = 0;
	else
	        opterr = 1;

	while ((c = getopt(argc, argv, "nsr:")) != -1) {
		if((char *)strstr(argv[0],"sact") != (char *)NULL) {
		        sprintf(ErrMsg,MSGCM(SACTUSAGE,
			"Usage: sact file... | -\n"),c);
			fatal(ErrMsg);
	        }
		switch (c) {

		case 'r':
			if (!isdigit((int)*optarg)) {
				sprintf(ErrMsg,MSGCM(UNGETBADSID,
"Invalid SID:( %c ) The SID must be a numeric value. (cm1)\n\t"),*optarg);
			        strcat(ErrMsg,MSGCM(UNGETUSAGE,
				"Usage: unget [-n] [-s] [-r SID] file... | -\n"));
				fatal(ErrMsg);
			}
			chksid(sid_ab(optarg,&sid),&sid);
			break;
		case 'n':
		case 's':
			break;
		case '?':
		default:
		        sprintf(ErrMsg,MSGCM(UNGETUSAGE,
			"Usage: unget [-n] [-s] [-r SID] file... | -\n"));
			fatal(ErrMsg);
			break;
		}

		if (had[c - 'a']++) {
			sprintf(ErrMsg,MSGCM(KEYLTRTWC, "Use the -%c flag only once on the command line. (cm2)\n"),c);
			fatal(ErrMsg);
		}
	}
	num_files = argc - optind;

	if(num_files == 0)
		if((char *)strstr(argv[0],"sact") != (char *)NULL)
			fatal(MSGCM(SACTUSAGE,
"Usage: sact file... | -\n"));
		else
			fatal(MSGCM(UNGETUSAGE,
"Usage: unget [-n] [-s] [-r SID] file... | -\n"));

	/*	If envoked as "sact", set flag
		otherwise executed as "unget".
	*/
	if (equal(sname(argv[0]),"sact")) {
		cmd = 1;
		HADS = 0;
	}

	if (!HADS)
		verbosity = -1;
	setsig();
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;
	for (i=optind; i<argc; i++)
		do_file(argv[i],unget);
	exit(Fcnt ? 1 : 0);
}

unget(file)
char *file;
{
	extern	char had_dir, had_standinp;
	extern	char *Sflags[];
	int	i, status;
	char	gfilename[FILESIZE];
	char	str[BUFSIZ];
	char	*sid_ba();
	struct	pfile *pp, *edpfile();

	if (setjmp(Fjmp))
		return;

	/*	Initialize packet, but do not open SCCS file.
	*/
	sinit(&gpkt,file,0);
	gpkt.p_stdout = stdout;
	gpkt.p_verbose = verbosity;

	copy(auxf(gpkt.p_file,'g'),gfilename);
	if (gpkt.p_verbose && (num_files > 1 || had_dir || had_standinp))
		fprintf(gpkt.p_stdout,"\n%s:\n",gpkt.p_file);
	/*	If envoked as "sact", call catpfile() and return.
	*/
	if (cmd) {
		catpfile(&gpkt);
		return;
	}

	if (lockit(auxf(gpkt.p_file,'z'),2,getpid()))
		fatal("cannot create lock file (cm4)");
	pp = edpfile(&gpkt,&sid);
	if (gpkt.p_verbose) {
		sid_ba(&pp->pf_nsid,str);
		fprintf(gpkt.p_stdout,"%s\n",str);
	}

	/*	If the size of the q-file is greater than zero,
		rename the q-file the p-file and remove the
		old p-file; else remove both the q-file and
		the p-file.
	*/
	if (Szqfile)
		rename(auxf(gpkt.p_file,'q'),Pfilename);
	else {
		xunlink(Pfilename);
		xunlink(auxf(gpkt.p_file,'q'));
	}
	ffreeall();
	unlockit(auxf(gpkt.p_file,'z'),getpid());

	/*	A child is spawned to remove the g-file so that
		the current ID will not be lost.
	*/
	if (!HADN) {
		if ((i = fork()) < 0)
			fatal(MSGSTR(CANTFORK, 
                          "\nCannot create another process at this time.(co20)\n"));
		if (i == 0) {
			setuid(getuid());
			unlink(gfilename);
			exit(0);
		}
		else {
			wait(&status);
		}
	}
}


struct pfile *
edpfile(pkt,sp)
struct packet *pkt;
struct sid *sp;
{
	static	struct pfile goodpf;
	char	*user, *logname();
	char	line[BUFSIZ];
	struct	pfile pf;
	int	cnt, name;
	FILE	*in, *out, *fdfopen();

	cnt = -1;
	name = 0;
	user = logname();
	zero((char *)&goodpf,sizeof(goodpf));
	in = xfopen(auxf(pkt->p_file,'p'),0);
	out = xfcreat(auxf(pkt->p_file,'q'),0644);
	while (fgets(line,sizeof(line),in) != NULL) {
		pf_ab(line,&pf,1);
		if (equal(pf.pf_user,user)) {
			name++;
			if (sp->s_rel == 0) {
				if (++cnt) {
					fclose(out);
					fclose(in);
					fatal(MSGSTR(SIDSPEC1,"\nThere is more than one outstanding SID in the p-file.(un1)\n"));
				}
				goodpf = pf;
				continue;
			}
			else if (sp->s_rel == pf.pf_nsid.s_rel &&
				sp->s_lev == pf.pf_nsid.s_lev &&
				sp->s_br == pf.pf_nsid.s_br &&
				sp->s_seq == pf.pf_nsid.s_seq) {
					goodpf = pf;
					continue;
			}
		}
		fputs(line,out);
	}
	fflush(out);
	fstat(fileno(out),&Statbuf);
	Szqfile = Statbuf.st_size;
	copy(auxf(pkt->p_file,'p'),Pfilename);
	fclose(out);
	fclose(in);
	if (!goodpf.pf_user[0])
		if (!name)
			fatal(MSGSTR(LOGNAME, 
			    "\nUser name is not in the p-file.\n\
\tLog in with the user name shown in the p-file. (un2)\n"));
		else fatal(MSGSTR(SIDSPEC3, 
			    "\nThe specified SID does not exist.\n\
\tCheck the p-file for existing SID numbers. (un3)\n"));
	return(&goodpf);
}


/* clean_up() only called from fatal().
*/
clean_up(n)
{
	/*	Lockfile and q-file only removed if lockfile
		was created by this process.
	*/
	if (mylock(auxf(gpkt.p_file,'z'),getpid())) {
		unlink(auxf(gpkt.p_file,'q'));
		ffreeall();
		unlockit(auxf(gpkt.p_file,'z'),getpid());
	}
}


catpfile(pkt)
struct packet *pkt;
{
	int c;
	FILE *in;

	if(!(in = fopen(auxf(pkt->p_file,'p'),"r")))
		fprintf(stderr,MSGSTR(NOOUTDELTA, 
                    "\nThere are no outstanding deltas for %s.\n"),pkt->p_file);
	else {
		while ((c = getc(in)) != EOF)
			putc(c,pkt->p_stdout);
		fclose(in);
	}
}
