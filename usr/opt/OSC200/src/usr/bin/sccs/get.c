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
static char	*sccsid = "@(#)$RCSfile: get.c,v $ $Revision: 4.2.5.5 $ (DEC) $Date: 1994/01/24 22:19:22 $";
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
 * FUNCTIONS: clean_up, cmrinsert, enter, escdodelt, fredck, get,
 *            gen_lfile, getser, idsetup, idsubst, in_pfile,
 *            makgdate, mk_qfile, newsid, prfx, trans, wrtpfile,
 *            main
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
 * get.c 1.8 com/cmd/sccs/cmd,3.1,9021 2/20/90 10:56:26";
 */

# include	<locale.h>
# include	<nl_types.h>
# include 	<sys/access.h>
# include	<unistd.h>

# include	"defines.h"
# include	"had.h"
# include 	"get_msg.h"

#define MSGSTR(Num, Str) catgets(catd, MS_GET, Num, Str)

#define GET_USAGE	" Specify the file to process.  (cm3)\n\
\tUsage: get [-Lbegkmnpst] [-c Cutoff] [-i List]\n\
\t\t[-r SID] [-w String] [-x List] [-l[p]] file... | -\n"

#define GET_OPTION	" Flag -%c is not valid. (cm1)\n\
\tUsage: get [-Lbegkmnpst] [-c Cutoff] [-i List]\n\
\t\t[-r SID] [-w String] [-x List] [-l[p] file... ] -\n"

#define REQ_ARG 	" Flag -%c requires an argument\n\
\tUsage: get [-Lbegkmnpst] [-c Cutoff] [-i List]\n\
\t\t[-r SID] [-w String] [-x List] [-l[p] file... | -\n"

struct stat Statbuf;
char Null[1];
char ErrMsg[512];

int	Debug = 0;
int	had_pfile;
struct packet gpkt;
struct sid sid;
unsigned	Ser;
int	num_files;
char	had[26];
char	Whatstr[BUFSIZ];
char	Pfilename[FILESIZE];
char	*ilist, *elist, *lfile = (char) NULL;
char	*sid_ab(), *auxf(), *logname();
char	*sid_ba(), *date_ba();
long	cutoff = 0X7FFFFFFFL;	/* max positive long */
int verbosity;
char	Gfile[FILESIZE];
char	*Type;
int	Did_id;
FILE *fdfopen();

#ifdef CASSI
/* Beginning of modifications made for CMF system. */
#define CMRLIMIT 128
char	cmr[CMRLIMIT];
int		cmri = 0;
/* End of insertion */
#endif

nl_catd catd;

main(argc,argv)
int argc;
register char *argv[];
{
	register char *p;
	register int i;
	int c;

	extern char *optarg;
	extern int optind, optopt, opterr;
	extern int Fcnt;
	extern get();

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	Fflags = FTLEXIT | FTLMSG | FTLCLN;

#ifdef CASSI
#  define OPTLIST ":CMFFLAG:r:w:c:ix:lLbegkmnpst"
#else
#  define OPTLIST ":r:w:c:i:x:lLbegkmnpst"
#endif
        /* look through arg list for -lp option and convert to -L */
	for (i=1; i<argc; i++)
        	if (!strcmp(argv[i],"-lp")) {
			lfile = Null;
			argv[i][2] = '\0';
                        break;
		}

	opterr = 0;
	while ((c = getopt(argc, argv, OPTLIST)) != -1) {
		switch (c) {
#ifdef CASSI
    			case CMFFLAG:
				/* Add the rest of this argument */
				/* to the existing CMR list. */
				p = optarg;
				while (*p) {
					if (cmri == CMRLIMIT)
						fatal (MSGSTR(CMRTOOLNG, 
					  "\nCMR list is too long.\n"));
					cmr[cmri++] = *p++;
				}
				cmr[cmri] = NULL;
				break;
#endif /* CASSI */

			case 'a':
				Ser = patoi(optarg);
				break;
			case 'r':
				chksid(sid_ab(optarg,&sid),&sid);
				if ((sid.s_rel < MINR) ||
					(sid.s_rel > MAXR))
					fatal(MSGCM(ROUTOFRNG, "\n Supply a value for the -r flag that is greater than 0 and\nless than 10,000. (cm23)\n"));
				break;
			case 'w':
				strcpy(Whatstr,optarg);
				break;
			case 'c':
				if (date_ab(optarg,&cutoff))
					fatal(MSGCM(BADDTTM, "\nThe date and time you specified are not in the correct format.\nThe correct format is: yy[mm[dd[hh[mm[ss]]]]] (cm5)\n"));
				break;
			case 'i':
				ilist = optarg;
				break;
			case 'x':
				elist = optarg;
				break;
			case 'L':
				c = 'l';	/* same as -lp */
				lfile = Null;
				break;
			case 'b':
			case 'e':
			case 'g':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'p':
			case 's':
			case 't':
				break;
			case ':':
				sprintf(ErrMsg,MSGSTR(ARGRQD, REQ_ARG), optopt);
				fatal(ErrMsg);

			case '?':
				sprintf(ErrMsg,MSGCM(GETOPTION, GET_OPTION),optopt);
				fatal(ErrMsg);
		} /* switch */

		if (had[c - 'a']++) {
			sprintf(ErrMsg,MSGCM(KEYLTRTWC, 
	"\n Use the -%c flag only once on the command line.  (cm2)\n"),optopt);
			fatal(ErrMsg);
		}
	} /* while */

	num_files = argc - optind;

	if(num_files == 0)
		fatal(MSGCM(GETUSAGE, GET_USAGE));
	if (HADE && HADM)
		fatal(MSGSTR(ENOTWITHM, "\n Do not use the -e flag with the -m flag on the command line. (ge3)\n"));
	if (HADE)
		HADK = 1;
	if (!HADS)
		verbosity = -1;
	if (HADE && ! logname())
		fatal(MSGSTR(USERID,
                  "\nThe /etc/passwd file is not accessible.(cm9)\n"));

	setsig();
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;

	for (i=optind; i<argc; i++) 
		do_file(argv[i],get);

	exit(Fcnt ? 1 : 0);
}

extern char *Sflags[];

get(file)
char *file;
{
	register char *p;
	register unsigned ser;
	extern char had_dir, had_standinp;
	struct stats stats;
	struct idel *dodelt();
	char	str[32];
	char *idsubst(), *auxf();

	Fflags |= FTLMSG;
	if (setjmp(Fjmp))
		return;
	if (HADE) {
		had_pfile = 1;
		/*
		call `sinit' to check if file is an SCCS file
		but don't open the SCCS file.
		If it is, then create lock file.
		*/
		sinit(&gpkt,file,0);
		if (lockit(auxf(file,'z'),10,getpid()))
			fatal(MSGSTR(LOCKCREAT, 
			   "\nCannot lock the specified file.(cm4)\n"));
	}
	/*
	Open SCCS file and initialize packet
	*/
	sinit(&gpkt,file,1);
	gpkt.p_ixuser = (HADI | HADX);
	gpkt.p_reqsid.s_rel = sid.s_rel;
	gpkt.p_reqsid.s_lev = sid.s_lev;
	gpkt.p_reqsid.s_br = sid.s_br;
	gpkt.p_reqsid.s_seq = sid.s_seq;
	gpkt.p_verbose = verbosity;
	gpkt.p_stdout = (HADP ? stderr : stdout);
	gpkt.p_cutoff = cutoff;
	gpkt.p_lfile = lfile;
	copy(auxf(gpkt.p_file,'g'),Gfile);

	if (gpkt.p_verbose && (num_files > 1 || had_dir || had_standinp))
		fprintf(gpkt.p_stdout,"\n%s:\n",gpkt.p_file);
	if (dodelt(&gpkt,&stats,(struct sid *) 0,0) == 0)
		fmterr(&gpkt);
	finduser(&gpkt);
	doflags(&gpkt);
	if (!HADA)
		ser = getser(&gpkt);
	else {
		if ((ser = Ser) > maxser(&gpkt))
			fatal(MSGSTR(SERNOTOOLRG,
                           "\nThe specified serial number does not exist.(ge19)\n"));
		gpkt.p_gotsid = gpkt.p_idel[ser].i_sid;
		if (HADR && sid.s_rel != gpkt.p_gotsid.s_rel) {
			zero((char *)&gpkt.p_reqsid, sizeof(gpkt.p_reqsid));
			gpkt.p_reqsid.s_rel = sid.s_rel;
		}
		else
			gpkt.p_reqsid = gpkt.p_gotsid;
	}
	doie(&gpkt,ilist,elist,(char *) 0);
	setup(&gpkt,(int) ser);
	if (!(Type = Sflags[TYPEFLAG - 'a']))
		Type = Null;
	if (!(HADP || HADG))
		if (exists(Gfile) && (S_IWRITE & Statbuf.st_mode)) {
			sprintf(ErrMsg,MSGSTR(WRITFILEEXST, 
                         "\nA writable version of %s exists.\n\
\tRemove the writable version or remove the file's write permission.(ge4)\n"),Gfile);
			fatal(ErrMsg);
		}

	if (gpkt.p_verbose) {
		sid_ba(&gpkt.p_gotsid,str);
		fprintf(gpkt.p_stdout,"%s\n",str);
	}
	if (HADE) {
		if (HADC || gpkt.p_reqsid.s_rel == 0)
			gpkt.p_reqsid = gpkt.p_gotsid;
		newsid(&gpkt,Sflags[BRCHFLAG - 'a'] && HADB);
		permiss(&gpkt);
		if (exists(auxf(gpkt.p_file,'p')))
			mk_qfile(&gpkt);
		else had_pfile = 0;
		wrtpfile(&gpkt,ilist,elist);
	}
	if (!HADG || HADL) {
		pid_t i;
		int status;

		if (gpkt.p_stdout)
			fflush(gpkt.p_stdout);

		if ((i = fork()) < 0)
			fatal(MSGSTR(CANTFORK,"\nCannot create another process at this time.(co20)\n"));
		if (i) {
			wait(&status);
			if (status) {
				Fflags &= ~FTLMSG;
				fatal(Null);
			}
			goto out;
		}

		Fflags |= FTLEXIT;
		Fflags &= ~FTLJMP;
		setuid(getuid());
		setgid(getgid());
		if (HADL)
			gen_lfile(&gpkt);
		if (HADG)
			exit(0);
		flushto(&gpkt,EUSERTXT,1);
		idsetup(&gpkt);
		gpkt.p_chkeof = 1;
		Did_id = 0;
		/*
		call `xcreate' which deletes the old `g-file' and
		creates a new one except if the `p' keyletter is set in
		which case any old `g-file' is not disturbed.
		The mod of the file depends on whether or not the `k'
		keyletter has been set.
		*/

		if (gpkt.p_gout == 0) {
			if (HADP)
				gpkt.p_gout = stdout;
			else
				gpkt.p_gout = xfcreat(Gfile,HADK ? 0644 : 0444);
		}
		while(readmod(&gpkt)) {
			prfx(&gpkt);
			p = idsubst(&gpkt,gpkt.p_line);
			if(fputs(p,gpkt.p_gout)==EOF)
				FAILPUT;
		}
		if (gpkt.p_gout)
			fflush(gpkt.p_gout);
		if (gpkt.p_gout && gpkt.p_gout != stdout)
			fclose(gpkt.p_gout);
		if (gpkt.p_verbose)
			fprintf(gpkt.p_stdout,
                             MSGSTR(ULINES,"%u lines\n"),gpkt.p_glnno);
		if (!Did_id && !HADK)
			if (Sflags[IDFLAG - 'a'])
				if(!(*Sflags[IDFLAG - 'a']))
					fatal(MSGSTR(NOIDKEYWRDS,"\nThe file must contain SCCS keywords.(cm6)\n"));
				else
					fatal(MSGSTR(INVIDKYWRDS, "\nThe SCCS file requires one or more specific keywords.(cm10)\n"));
			else if (gpkt.p_verbose)
				fprintf(stderr,MSGSTR(NOIDKEYWRDS7, 
"\nThere are no SCCS identification keywords in the file.(cm7)\n"));
		exit(0);
	}
out:    if (gpkt.p_iop)
		fclose(gpkt.p_iop);

	/*
	remove 'q'-file because it is no longer needed
	*/
	unlink(auxf(gpkt.p_file,'q'));
	ffreeall();
	unlockit(auxf(gpkt.p_file,'z'),getpid());
}


newsid(pkt,branch)
register struct packet *pkt;
int branch;
{
	int chkbr;

	chkbr = 0;
	/* if branch value is 0 set newsid level to 1 */
	if (pkt->p_reqsid.s_br == 0) {
		pkt->p_reqsid.s_lev += 1;
		/*
		if the sid requested has been deltaed or the branch
		flag was set or the requested SID exists in the p-file
		then create a branch delta off of the gotten SID
		*/
		if (sidtoser(&pkt->p_reqsid,pkt) ||
			pkt->p_maxr > pkt->p_reqsid.s_rel || branch ||
			in_pfile(&pkt->p_reqsid,pkt)) {
				pkt->p_reqsid.s_rel = pkt->p_gotsid.s_rel;
				pkt->p_reqsid.s_lev = pkt->p_gotsid.s_lev;
				pkt->p_reqsid.s_br = pkt->p_gotsid.s_br + 1;
				pkt->p_reqsid.s_seq = 1;
				chkbr++;
		}
	}
	/*
	if a three component SID was given as the -r argument value
	and the b flag is not set then up the gotten SID sequence
	number by 1
	*/
	else if (pkt->p_reqsid.s_seq == 0 && !branch)
		pkt->p_reqsid.s_seq = pkt->p_gotsid.s_seq + 1;
	else {
		/*
		if sequence number is non-zero then increment the
		requested SID sequence number by 1
		*/
		pkt->p_reqsid.s_seq += 1;
		if (branch || sidtoser(&pkt->p_reqsid,pkt) ||
			in_pfile(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			pkt->p_reqsid.s_seq = 1;
			chkbr++;
		}
	}
	/*
	keep checking the requested SID until a good SID to be
	made is calculated or all possibilities have been tried
	*/
	while (chkbr) {
		--chkbr;
		while (in_pfile(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			++chkbr;
		}
		while (sidtoser(&pkt->p_reqsid,pkt)) {
			pkt->p_reqsid.s_br += 1;
			++chkbr;
		}
	}
	if (sidtoser(&pkt->p_reqsid,pkt) || in_pfile(&pkt->p_reqsid,pkt))
		fatal(MSGSTR(BADSIDCALC, "\nbad SID calculated in newsid()"));
}


enter(pkt,ch,n,sidp)
struct packet *pkt;
char ch;
int n;
struct sid *sidp;
{
	char str[32];
	register struct apply *ap;

	sid_ba(sidp,str);
	if (pkt->p_verbose)
		fprintf(pkt->p_stdout,"%s\n",str);
	ap = &pkt->p_apply[n];
	switch(ap->a_code) {

	case SX_EMPTY:
		if (ch == INCLUDE)
			condset(ap,APPLY,INCLUSER);
		else
			condset(ap,NOAPPLY,EXCLUSER);
		break;
	case APPLY:
		sid_ba(sidp,str);
		sprintf(ErrMsg,MSGSTR(ALRDYINC,
                  "\n%s is already included.\n\
\tSpecify the delta only once with the -i flag. (ge9)\n"),str);
		fatal(ErrMsg);
		break;
	case NOAPPLY:
		sid_ba(sidp,str);
		sprintf(ErrMsg,MSGSTR(ALRDYEXCLD, 
                  "\n%s is already excluded.\n\
\tSpecify the delta only once with the -x flag. (ge10)\n"),str);
		fatal(ErrMsg);
		break;
	default:
		fatal(MSGSTR(INTRNERR11,
                  "\nThere is an internal software error.(ge11)\n"));
		break;
	}
}


gen_lfile(pkt)
register struct packet *pkt;
{
	char *n;
	int reason;
	char str[32];
	char line[BUFSIZ];
	struct deltab dt;
	FILE *in;
	FILE *out;

	in = xfopen(pkt->p_file,0);
	if (pkt->p_lfile) 
		out = stdout;
	else
		out = xfcreat(auxf(pkt->p_file,'l'),0444);
	fgets(line,sizeof(line),in);
	while (fgets(line,sizeof(line),in) != NULL && line[0] == CTLCHAR && line[1] == STATS) {
		fgets(line,sizeof(line),in);
		del_ab(line,&dt,pkt);
		if (dt.d_type == 'D') {
			reason = pkt->p_apply[dt.d_serial].a_reason;
			if (pkt->p_apply[dt.d_serial].a_code == APPLY) {
				putc(' ',out);
				putc(' ',out);
			}
			else {
				putc('*',out);
				if (reason & IGNR)
					putc(' ',out);
				else
					putc('*',out);
			}
			switch (reason & (INCL | EXCL | CUTOFF)) {
	
			case INCL:
				putc('I',out);
				break;
			case EXCL:
				putc('X',out);
				break;
			case CUTOFF:
				putc('C',out);
				break;
			default:
				putc(' ',out);
				break;
			}
			putc(' ',out);
			sid_ba(&dt.d_sid,str);
			fprintf(out,"%s\t",str);
			date_ba(&dt.d_datetime,str);
			fprintf(out,"%s %s\n",str,dt.d_pgmr);
		}
		while ((n = fgets(line,sizeof(line),in)) != NULL)
			if (line[0] != CTLCHAR)
				break;
			else {
				switch (line[1]) {

				case EDELTAB:
					break;
				default:
					continue;
				case MRNUM:
				case COMMENTS:
					if (dt.d_type == 'D')
						fprintf(out,"\t%s",&line[3]);
					continue;
				}
				break;
			}
		if (n == NULL || line[0] != CTLCHAR)
			break;
		putc('\n',out);
	}
	fclose(in);
	if (out != stdout)
		fclose(out);
}


char	Curdate[18];
char	*Curtime;
char	Gdate[9];
char	Chgdate[18];
char	*Chgtime;
char	Gchgdate[9];
char	Sid[32];
char	Mod[16];
char	Olddir[BUFSIZ];
char	Pname[BUFSIZ];
char	Dir[BUFSIZ];
char	*Qsect;

idsetup(pkt)
register struct packet *pkt;
{
	extern time_t Timenow;
	register int n;
	register char *p;

	date_ba(&Timenow,Curdate);
	Curtime = &Curdate[9];
	Curdate[8] = 0;
	makgdate(Curdate,Gdate);
	for (n = maxser(pkt); n; n--)
		if (pkt->p_apply[n].a_code == APPLY)
			break;
	if (n)
		date_ba(&pkt->p_idel[n].i_datetime,Chgdate);
	Chgtime = &Chgdate[9];
	Chgdate[8] = 0;
	makgdate(Chgdate,Gchgdate);
	sid_ba(&pkt->p_gotsid,Sid);
	if (p = Sflags[MODFLAG - 'a'])
		copy(p,Mod);
	else
		copy(Gfile,Mod);
	if (!(Qsect = Sflags[QSECTFLAG - 'a']))
		Qsect = Null;
}


makgdate(old,new)
register char *old, *new;
{
	if ((*new = old[3]) != '0')
		new++;
	*new++ = old[4];
	*new++ = '/';
	if ((*new = old[6]) != '0')
		new++;
	*new++ = old[7];
	*new++ = '/';
	*new++ = old[0];
	*new++ = old[1];
	*new = 0;
}


static char Zkeywd[5] = "@(#)";

char *
idsubst(pkt,line)
register struct packet *pkt;
char line[];
{
	static char tline[BUFSIZ];
	char hold[BUFSIZ];
	static char str[32];
	register char *lp, *tp;
	char *trans();
	int recursive = 0;
	extern char *Type;
	extern char *Sflags[];

	if (HADK || !any('%',line))
		return(line);

	tp = tline;
	for(lp=line; *lp != 0; lp++) {
		if(lp[0] == '%' && lp[1] != 0 && lp[2] == '%') {
			if((!Did_id) && (Sflags['i'-'a']) &&
				(!(strncmp(Sflags['i'-'a'],lp,strlen(Sflags['i'-'a'])))))
					++Did_id;
			switch(*++lp) {

			case 'M':
				tp = trans(tp,Mod);
				break;
			case 'Q':
				tp = trans(tp,Qsect);
				break;
			case 'R':
				sprintf(str,"%u",pkt->p_gotsid.s_rel);
				tp = trans(tp,str);
				break;
			case 'L':
				sprintf(str,"%u",pkt->p_gotsid.s_lev);
				tp = trans(tp,str);
				break;
			case 'B':
				sprintf(str,"%u",pkt->p_gotsid.s_br);
				tp = trans(tp,str);
				break;
			case 'S':
				sprintf(str,"%u",pkt->p_gotsid.s_seq);
				tp = trans(tp,str);
				break;
			case 'D':
				tp = trans(tp,Curdate);
				break;
			case 'H':
				tp = trans(tp,Gdate);
				break;
			case 'T':
				tp = trans(tp,Curtime);
				break;
			case 'E':
				tp = trans(tp,Chgdate);
				break;
			case 'G':
				tp = trans(tp,Gchgdate);
				break;
			case 'U':
				tp = trans(tp,Chgtime);
				break;
			case 'Z':
				tp = trans(tp,Zkeywd);
				break;
			case 'Y':
				tp = trans(tp,Type);
				break;
			case 'W':
				if((Whatstr[0] != NULL) && (!recursive)) {
					recursive = 1;
					lp += 2;
					strcpy(hold,Whatstr);
					strcat(hold,lp);
					lp = hold;
					lp--;
					continue;
				}
				tp = trans(tp,Zkeywd);
				tp = trans(tp,Mod);
				*tp++ = '\t';
			case 'I':
				tp = trans(tp,Sid);
				break;
			case 'P':
				copy(pkt->p_file,Dir);
				dname(Dir);
				if(curdir(Olddir) != 0)
					fatal(MSGSTR(CURDIRFAIL, "\nCannot determine the path name of the current directory.(cm21)\n"));
				if(chdir(Dir) != 0) {
					sprintf(ErrMsg,
					MSGSTR(CANTCHDIR, 
                                           "Cannot chdir to %s.(cm22)\n"),Dir);
					fatal(ErrMsg);
				}
				if(curdir(Pname) != 0)
					fatal(MSGSTR(CURDIRFAIL, "\nCannot determine the path name of the current directory.(cm21)\n"));
				if(chdir(Olddir) != 0) {
					sprintf(ErrMsg,
					MSGSTR(CANTCHDIR, 
                                           "Cannot chdir to %s.\n"),Olddir);
					fatal(ErrMsg);
				}
				tp = trans(tp,Pname);
				*tp++ = '/';
				tp = trans(tp,(sname(pkt->p_file)));
				break;
			case 'F':
				tp = trans(tp,pkt->p_file);
				break;
			case 'C':
				sprintf(str,"%u",pkt->p_glnno);
				tp = trans(tp,str);
				break;
			case 'A':
				tp = trans(tp,Zkeywd);
				tp = trans(tp,Type);
				*tp++ = ' ';
				tp = trans(tp,Mod);
				*tp++ = ' ';
				tp = trans(tp,Sid);
				tp = trans(tp,Zkeywd);
				break;
			default:
				*tp++ = '%';
				*tp++ = *lp;
				continue;
			}
			if (!(Sflags['i'-'a']))
				++Did_id;
			lp++;
		}
		else
			*tp++ = *lp;
	}

	*tp = 0;
	return(tline);
}


char *
trans(tp,str)
register char *tp, *str;
{
	while(*tp++ = *str++)
		;
	return(tp-1);
}


prfx(pkt)
register struct packet *pkt;
{
	char str[32];

	if (HADN)
		fprintf(pkt->p_gout,"%s\t",Mod);
	if (HADM) {
		sid_ba(&pkt->p_inssid,str);
		fprintf(pkt->p_gout,"%s\t",str);
	}
}


clean_up(n)
{
	/*
	clean_up is only called from fatal() upon bad termination.
	*/
	if (gpkt.p_iop)
		fclose(gpkt.p_iop);
	if (gpkt.p_gout)
		fflush(gpkt.p_gout);
	if (gpkt.p_gout && gpkt.p_gout != stdout) {
		fclose(gpkt.p_gout);
		unlink(Gfile);
	}
	if (HADE) {
		if (! had_pfile) {
			unlink(auxf(gpkt.p_file,'p'));
		}
		else if (exists(auxf(gpkt.p_file,'q')))
			unlink(auxf(gpkt.p_file,'q'));
	}
	ffreeall();
	unlockit(auxf(gpkt.p_file,'z'),getpid());
}


/* WARN_MSG is the default message for the catalog WARNMSG message */

#define WARN_MSG "WARNING:  %s is being edited.\n\
\tThis is an informational message only. (ge18)\n"

wrtpfile(pkt,inc,exc)
register struct packet *pkt;
char *inc, *exc;
{
	char line[64], str1[32], str2[32];
	char *user;
	FILE *in, *out;
	struct pfile pf;
	register char *p;
	int fd;
/*	extern long Timenow; */
	extern time_t Timenow;

	user = logname();
	if (exists(p = auxf(pkt->p_file,'p'))) {
		fd = xopen(p,0);
		in = fdfopen(fd,0);
		while (fgets(line,sizeof(line),in) != NULL) {
			p = line;
			p[length(p) - 1] = 0;
			pf_ab(p,&pf,0);
			if (!(Sflags[JOINTFLAG - 'a'])) {
				if ((pf.pf_gsid.s_rel == pkt->p_gotsid.s_rel &&
     				   pf.pf_gsid.s_lev == pkt->p_gotsid.s_lev &&
				   pf.pf_gsid.s_br == pkt->p_gotsid.s_br &&
				   pf.pf_gsid.s_seq == pkt->p_gotsid.s_seq) ||
				   (pf.pf_nsid.s_rel == pkt->p_reqsid.s_rel &&
				   pf.pf_nsid.s_lev == pkt->p_reqsid.s_lev &&
				   pf.pf_nsid.s_br == pkt->p_reqsid.s_br &&
				   pf.pf_nsid.s_seq == pkt->p_reqsid.s_seq)) {
					fclose(in);
					sprintf(ErrMsg,
					     MSGSTR(BNGEDT, 
                                               "Another user is editing %s.(ge17)\n"),
                                                 line);
					fatal(ErrMsg);
				}
				if (!equal(pf.pf_user,user))
					fprintf(stderr,MSGSTR(WARNMSG, WARN_MSG),line);
			}
			else fprintf(stderr,MSGSTR(WARNMSG, WARN_MSG),line);
		}
		fd = xopen(auxf(pkt->p_file,'q'),1);
		out = fdfopen(dup(fd),1);
		fclose(in);
	}
	else
		out = xfcreat(p,0644);
	fseek(out,0L,2);
	sid_ba(&pkt->p_gotsid,str1);
	sid_ba(&pkt->p_reqsid,str2);
	date_ba(&Timenow,line);
	fprintf(out,"%s %s %s %s",str1,str2,user,line);
	if (inc)
		fprintf(out," -i%s",inc);
	if (exc)
		fprintf(out," -x%s",exc);
#ifdef CASSI
	if (cmrinsert () > 0)	/* if there are CMRS and they are okay */
		fprintf (out, " -z%s", cmr);
#endif
	fprintf(out,"\n");
	fclose(out);
	if (pkt->p_verbose)
		fprintf(pkt->p_stdout,MSGSTR(NEWDELTA, "new delta %s\n"),str2);
	if (exists(auxf(pkt->p_file,'q'))) {
		copy(auxf(pkt->p_file,'p'),Pfilename);
		rename(auxf(pkt->p_file,'q'),Pfilename);
	}
}

getser(pkt)
register struct packet *pkt;
{
	register struct idel *rdp;
	int n, ser, def;
	char *p;
	extern char *Sflags[];

	def = 0;
	if (pkt->p_reqsid.s_rel == 0) {
		if (p = Sflags[DEFTFLAG - 'a'])
			chksid(sid_ab(p, &pkt->p_reqsid), &pkt->p_reqsid);
		else {
			pkt->p_reqsid.s_rel = MAXR;
			def = 1;
		}
	}
	ser = 0;
	if (pkt->p_reqsid.s_lev == 0) {
		for (n = maxser(pkt); n; n--) {
			rdp = &pkt->p_idel[n];
			if ((rdp->i_sid.s_br == 0 || HADT) &&
				pkt->p_reqsid.s_rel >= rdp->i_sid.s_rel &&
				rdp->i_sid.s_rel > pkt->p_gotsid.s_rel) {
					ser = n;
					pkt->p_gotsid.s_rel = rdp->i_sid.s_rel;
			}
		}
	}
	/*
	 * If had '-t' keyletter and R.L SID type, find
	 * the youngest SID
	 */
	else if ((pkt->p_reqsid.s_br == 0) && HADT) {
                for (n = maxser(pkt); n; n--) {
                        rdp = &pkt->p_idel[n];
                        if (rdp->i_sid.s_rel == pkt->p_reqsid.s_rel &&
                            rdp->i_sid.s_lev == pkt->p_reqsid.s_lev )
                                break;
		}
		ser = n;
	}
        else if (pkt->p_reqsid.s_br && pkt->p_reqsid.s_seq == 0) {
                for (n = maxser(pkt); n; n--) {
                        rdp = &pkt->p_idel[n];
                        if (rdp->i_sid.s_rel == pkt->p_reqsid.s_rel &&
                                rdp->i_sid.s_lev == pkt->p_reqsid.s_lev &&
                                rdp->i_sid.s_br == pkt->p_reqsid.s_br)
                                        break;
                }
                ser = n;
        }
	else {
		ser = sidtoser(&pkt->p_reqsid,pkt);
	}							/*GA002*/
	if (ser == 0)
                fatal(MSGSTR(SIDNOEXIST,
                   "\nThe SID specified does not exist.\n\
Use the sact command to check the \n\
p-file for existing SID numbers. (cm20)\n"));

	rdp = &pkt->p_idel[ser];
	pkt->p_gotsid = rdp->i_sid;
	if (def || (pkt->p_reqsid.s_lev == 0 && pkt->p_reqsid.s_rel == pkt->p_gotsid.s_rel))
		pkt->p_reqsid = pkt->p_gotsid;
	return(ser);
}

/* Null routine to satisfy external reference from dodelt() */

escdodelt()
{
}
#ifdef CASSI
/* NULL routine to satisfy external reference from dodelt() */
fredck()
{
}
#endif

in_pfile(sp,pkt)
struct	sid	*sp;
struct	packet	*pkt;
{
	struct	pfile	pf;
	char	line[BUFSIZ];
	char	*p;
	FILE	*in;

	if (Sflags[JOINTFLAG - 'a']) {
		if (exists(auxf(pkt->p_file,'p'))) {
			in = xfopen(auxf(pkt->p_file,'p'),0);
			while ((p = fgets(line,sizeof(line),in)) != NULL) {
				p[length(p) - 1] = 0;
				pf_ab(p,&pf,0);
				if (pf.pf_nsid.s_rel == sp->s_rel &&
					pf.pf_nsid.s_lev == sp->s_lev &&
					pf.pf_nsid.s_br == sp->s_br &&
					pf.pf_nsid.s_seq == sp->s_seq) {
						fclose(in);
						return(1);
				}
			}
			fclose(in);
		}
	}
	return(0);
}


mk_qfile(pkt)
register struct	packet *pkt;
{
	FILE	*in, *qout;
	char	line[BUFSIZ];

	in = xfopen(auxf(pkt->p_file,'p'),0);
	qout = xfcreat(auxf(pkt->p_file,'q'),0644);

	while ((fgets(line,sizeof(line),in) != NULL))
		if(fputs(line,qout)==EOF)
			FAILPUT;
	fclose(in);
	fclose(qout);
}
#ifdef CASSI

/* cmrinsert -- insert CMR numbers in the p.file. */

cmrinsert ()
{
	extern char *strrchr (), *Sflags[];
	extern int	cmrcheck ();
	char holdcmr[CMRLIMIT];
	char tcmr[CMRLIMIT];
	char *p;
	int bad;
	int valid;


	if (Sflags[CMFFLAG - 'a'] == 0)		/* CMFFLAG was not set. */
		return (0);

	if ( HADP && ( ! HADZ))	/* no CMFFLAG and no place to prompt. */
		fatal(MSGSTR(CKCASSINOCMR, "\nBackground CASSI get with no CMRs\n"));

retry:
	if (cmr[0] == NULL) {					/* No CMR list.  Make one. */
		if(HADZ && ((!isatty(0)) || (!isatty(1))))
		{
			fatal(MSGSTR(BKCASSIINVCMR, 
                          "\nBackground CASSI get with invalid CMR\n"));
		}
		fprintf (stdout, MSGSTR(COMMACMR, 
                   "\nInput Comma Separated List of CMRs: "));
		fgets (cmr, CMRLIMIT, stdin);
		p=strend(cmr);
		*(--p) = NULL;
		if ((int)(p - cmr) == CMRLIMIT) {
			fprintf (stdout, MSGSTR(TOOMANYCMR,"Too many CMRs.\n"));
			cmr[0] = NULL;
			goto retry;					/* Entry was too long. */
			}
		}

	/* Now, check the comma seperated list of CMRs for accuracy. */

	bad = 0;
	valid = 0;
	strcpy(tcmr,cmr);
	while(p=strrchr(tcmr,',')) {
		++p;
		if(cmrcheck(p,Sflags[CMFFLAG - 'a']))
			++bad;
		else {
			++valid;
			strcat(holdcmr,",");
			strcat(holdcmr,p);
		}
		*(--p) = NULL;
	}
	if(*tcmr)
		if(cmrcheck(tcmr,Sflags[CMFFLAG - 'a']))
			++bad;
		else {
			++valid;
			strcat(holdcmr,",");
			strcat(holdcmr,tcmr);
		}

	if(!bad && holdcmr[1]) {
		strcpy(cmr,holdcmr+1);
		return(1);
	}
	else {
		if((isatty(0)) && (isatty(1)))
			if(!valid)
			     fprintf(stdout,MSGSTR(ONEVALCMR, 
                                "\nMust enter at least one valid CMR.\n"));
			else
			     fprintf(stdout,MSGSTR(REENTCMR,
                                "\nRe-enter invalid CMRs, or press return.\n"));
		cmr[0] = NULL;
		goto retry;
	}
}
#endif
