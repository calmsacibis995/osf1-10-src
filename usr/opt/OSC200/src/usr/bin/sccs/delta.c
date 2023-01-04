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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: delta.c,v $ $Revision: 4.2.6.3 $ (OSF) $Date: 1993/10/11 19:06:44 $";
#endif
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: after, before, clean_up, delete, delta, dodiff, enter,
 *            escdodelt, fgetchk, fredck, getdiff, insert, linerange,
 *            mkdelt, mkixg, putcmrs, putmrs, rddiff, rdpfile, skipline,
 *            main
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *    
 *delta.c 1.13  com/cmd/sccs/cmd/delta.c, cmdsccs, bos320,9126320 6/19/91 08:04:54";
 */

#include 	<locale.h>
#include 	<nl_types.h>
#include 	<sys/access.h>
#include 	<unistd.h>
#include	"defines.h"
#include	"had.h"

#include 	"delta_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_DELTA, Num, Str)

extern char *Sflags[];

struct stat Statbuf;
char Null[1];
char ErrMsg[512];

char	Diffpgm[]   =   "/usr/bin/bdiff";
FILE	*Diffin;
FILE	*Gin;
int	Debug = 0;
struct packet gpkt;
struct sid sid;
int	num_files;
char	had[26];
char	*ilist, *elist, *glist;
char    *Comments,*Mrs;
#ifdef CASSI
char    Cmrs[300],*Nsid;
#endif
char	*auxf(), *logname(), *sid_ba();
int verbosity;
int	Did_id;
long	Szqfile;
char	Pfilename[FILESIZE];
FILE	 *fdfopen();
#ifdef CASSI
char *Cassin;
#endif
extern FILE	*Xiop;
extern int	Xcreate;

nl_catd catd;

/* _AMBIG is the default message string for the AMBIG catalog message */
#define _AMBIG "\nThere is more than one outstanding delta.\n\
\tSpecify the SID number that will be created. (de15)\n"

/*ILL_DATA is the default message string for the ILLDAT catalog message */
#define ILL_DATA " The SOH character is in the first position of line %1$d of file %2$s.\n\
\tRemove this character or precede it with a \\ (backslash).(de14)"

#define REQ_ARG 	" Flag -%c requires an argument\n\
\tUsage: delta [-g List] [-m MRlist] [-nps] [-r SID]\n\
\t\t[-y[comment]] file... | -" 




main(argc,argv)
int argc;
register char *argv[];
{
	register int i;
	register char *p;
	char *sid_ab();
	int	cc;	/* XPG4 */

	extern delta();
	extern int Fcnt;
	extern int optopt, opterr;
	extern char *optarg;

        (void) setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	Fflags = FTLEXIT | FTLMSG | FTLCLN;
	if (argc < 2) {
		sprintf(ErrMsg,MSGCM(DELTAUSAGE,
			" Specify the file to process. (cm3)\n\
\tUsage: delta [-g List] [-m MRlist] [-nps] [-r SID]\n\t[-y[comment]] file... | -"));
		fatal(ErrMsg);
	};

	opterr = 0;
	while ((cc = getopt(argc, argv, ":g:m:y:npsr:z:")) != -1) {
		switch (cc) {
		case 'r':
			chksid(sid_ab(optarg,&sid),&sid);
			break;
		case 'g':
			glist = optarg;
			break;
		case 'y':
			savecmt(optarg);
			break;
		case 'm':
			Mrs = optarg;
			repl(Mrs,'\n',' ');
			break;
		case 'p':
		case 'n':
		case 's':
			break;
#ifdef CASSI
		case 'z':
			Cassin = optarg;
			break;
#endif
                case ':':
                        if (optopt == 'y') {
                            cc = optopt;
                            break;
			}
			sprintf(ErrMsg,MSGSTR(ARGRQD, REQ_ARG), optopt);
			fatal(ErrMsg);

		case '?':
			sprintf(ErrMsg,MSGCM(UNKKEYLTR,
				" Flag -%c is not valid.(cm1)\n\
\tUsage: delta [-g List] [-m MRlist] [-nps] [-r SID]\n\t\t[-y[comment]] file... | -"), optopt); 
			fatal(ErrMsg);
		}

		if (had[cc - 'a']++) {
			sprintf(ErrMsg,MSGCM(KEYLTRTWC, "Use the -%c flag only once on the command line. (cm2)\n"),cc);  /* MSG */
			fatal(ErrMsg);
		}
	}
	num_files = argc - optind;

	if(num_files == 0)
		fatal(MSGCM(MISSFLNAM, "Specify the file to process.  (cm3)\n"));  /* MSG */
	if (!HADS)
		verbosity = -1;
	setsig();
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;
	for (i=optind; i<argc; i++)
			do_file(argv[i],delta);
	exit(Fcnt ? 1 : 0);
}


delta(file)
char *file;
{
	int n, linenum;
	char type;
	register int ser;
	extern char had_dir, had_standinp;
#ifdef CASSI
	char nsid[50];
#endif
	char dfilename[FILESIZE];
	char gfilename[FILESIZE];
	char line[512];
	char *getline();
	FILE  *dodiff();
	struct stats stats;
	struct pfile *pp, *rdpfile();
	struct stat sbuf;
	struct idel *dodelt();
	int inserted, deleted, orig;
	int newser;
	int status;
	int diffloop;
	int difflim;

	char 		*sptr;

	if (setjmp(Fjmp))
		return;
	sinit(&gpkt,file,1);
	if (lockit(auxf(gpkt.p_file,'z'),2,getpid()))
		fatal(MSGCM(LOCKCREAT, "Cannot lock the specified file.\n\
\tCheck path name and permissions or \n\
\twait until the file is not in use. (cm4)\n"));  /* MSG */
	gpkt.p_reopen = 1;
	gpkt.p_stdout = stdout;
	copy(auxf(gpkt.p_file,'g'),gfilename);

	fgetchk(gfilename, &gpkt);

	Gin = xfopen(gfilename,0);
	pp = rdpfile(&gpkt,&sid);
#ifdef CASSI
	strcpy(Cmrs,pp->pf_cmrlist);
	if(!pp->pf_nsid.s_br)
		{
		 sprintf(nsid,"%d.%d",pp->pf_nsid.s_rel,pp->pf_nsid.s_lev);
		}
	else
		{
			sprintf(nsid,"%d.%d.%d.%d",pp->pf_nsid.s_rel,pp->pf_nsid.s_lev,pp->pf_nsid.s_br,pp->pf_nsid.s_seq);
		}
	Nsid=nsid;
#endif
	gpkt.p_cutoff = pp->pf_date;
	ilist = pp->pf_ilist;
	elist = pp->pf_elist;

	if (dodelt(&gpkt,&stats,(struct sid *) 0,0) == 0)
		fmterr(&gpkt);
	if ((ser = sidtoser(&pp->pf_gsid,&gpkt)) == 0 ||
		sidtoser(&pp->pf_nsid,&gpkt))
			fatal(MSGCO(BADPFILE, "\n The p-file is damaged.\n\
\tRestore a backup copy. (co17)\n"));  /* MSG */
	doie(&gpkt,ilist,elist,glist);
	setup(&gpkt,ser);
	finduser(&gpkt);
	doflags(&gpkt);
	gpkt.p_reqsid = pp->pf_nsid;
	permiss(&gpkt);
	flushto(&gpkt,EUSERTXT,1);
	gpkt.p_chkeof = 1;
	copy(auxf(gpkt.p_file,'d'),dfilename);
	gpkt.p_gout = xfcreat(dfilename,0444);
	while(readmod(&gpkt)) {
		if(fputs(gpkt.p_line,gpkt.p_gout)==EOF)
			FAILPUT;
	}
	fclose(gpkt.p_gout);
	orig = gpkt.p_glnno;
	gpkt.p_glnno = 0;
	gpkt.p_verbose = verbosity;
	Did_id = 0;
	while (fgets(line,sizeof(line),Gin) != NULL &&
			 !chkid(line,Sflags['i'-'a']))
		;
	fclose(Gin);
	dohist();
	if (gpkt.p_verbose && (num_files > 1 || had_dir || had_standinp))
		fprintf(gpkt.p_stdout,"\n%s:\n",gpkt.p_file);
	if (!Did_id)
		if (Sflags[IDFLAG - 'a'])
			if(!(*Sflags[IDFLAG - 'a']))
				fatal(MSGCM(NOIDKEYWRDS, "The file must contain SCCS identification keywords.\n\
\tInsert one or more SCCS identification keywords into the file. (cm6)\n"));  /* MSG */
			else
				fatal(MSGCM(INVIDKYWRDS, "The SCCS file requires one or more specific\n\
\tidentification keywords.\n\
\tAdd the keywords to the file. (cm10)\n"));  /* MSG */
		else if (gpkt.p_verbose)
			fprintf(stderr,MSGCM(NOIDKEYWRDS7, "There are no SCCS identification keywords in the file. (cm7)\n"));  /* MSG */

	/*
	The following while loop executes 'bdiff' on g-file and
	d-file. If 'bdiff' fails (usually because segmentation
	limit it is using is too large for 'diff'), it is
	invoked again, with a lower segmentation limit.
	*/
	difflim = 3500;
	diffloop = 0;
	while (1) {
		inserted = deleted = 0;
		gpkt.p_glnno = 0;
		gpkt.p_upd = 1;
		gpkt.p_wrttn = 1;
		getline(&gpkt);
		gpkt.p_wrttn = 1;
		newser = mkdelt(&gpkt,&pp->pf_nsid,&pp->pf_gsid,
						diffloop,orig);
		diffloop = 1;
		flushto(&gpkt,EUSERTXT,0);
		Diffin = dodiff(auxf(gpkt.p_file,'g'),dfilename,difflim);
		while (n = getdiff(&type,&linenum)) {
			if (type == INS) {
				inserted += n;
				insert(&gpkt,linenum,n,newser);
			}
			else {
				deleted += n;
				delete(&gpkt,linenum,n,newser);
			}
		}
		fclose(Diffin);
		if (gpkt.p_iop)
			while (readmod(&gpkt))
				;
		wait(&status);
		if (status) {		/* diff failed */
			/*
			Check top byte (exit code of child).
			*/
			if (((status >> 8) & 0377) == 32) { /* 'execl' failed */
				sprintf(ErrMsg,MSGCO(CANTEXEC, " Cannot execute %s.\n\
\tCheck path name and permissions or\n\
\tuse local problem reporting procedures. (co50)\n"),Diffpgm);  /* MSG */
				fatal(ErrMsg);
			}
			/*
			Re-try.
			*/
			if (difflim -= 500) {	/* reduce segmentation */
				fprintf(stderr,
			MSGSTR(RETRYSEG, " %s failed.  The system is trying again\n\twith a segmentation size of %d. (de13)\n"),  /* MSG */
					Diffpgm,difflim);
				fclose(Xiop);	/* set up */
				Xiop = 0;	/* for new x-file */
				Xcreate = 0;
				/*
				Re-open s-file.
				*/
				gpkt.p_iop = xfopen(gpkt.p_file,0);
				setbuf(gpkt.p_iop,gpkt.p_buf);
				/*
				Reset counters.
				*/
				gpkt.p_slnno = 0;
				gpkt.p_ihash = 0;
				gpkt.p_chash = 0;
				gpkt.p_nhash = 0;
				gpkt.p_keep = 0;
			}
			else
				/* tried up to 500 lines, can't go on */
				fatal(MSGSTR(BADDIFF, "\n The diff program failed when SCCS tried to create the delta.\n\tUse local problem reporting procedures. (de4)\n"));  /* MSG */
		}
		else {		/* no need to try again, worked */
			break;			/* exit while loop */
		}
	}
	fgetchk(gfilename,&gpkt);
	unlink(dfilename);
	stats.s_ins = inserted;
	stats.s_del = deleted;
	stats.s_unc = orig - deleted;
	if (gpkt.p_verbose) {
		fprintf(gpkt.p_stdout,MSGSTR(INSM,"%u inserted\n"),stats.s_ins);
		fprintf(gpkt.p_stdout,MSGSTR(DELM,"%u deleted\n"),stats.s_del);
		fprintf(gpkt.p_stdout,MSGSTR(UNC,"%u unchanged\n"),stats.s_unc);
	}
	flushline(&gpkt,&stats);
	stat(gpkt.p_file,&sbuf);
	rename(auxf(gpkt.p_file,'x'),gpkt.p_file);
	chown(gpkt.p_file,sbuf.st_uid,sbuf.st_gid);
	if (Szqfile)
		rename(auxf(gpkt.p_file,'q'),Pfilename);
	else {
		xunlink(Pfilename);
		xunlink(auxf(gpkt.p_file,'q'));
	}
	clean_up(0);
	if (!HADN) {
		pid_t i;

		fflush(gpkt.p_stdout);
		if ((i = fork()) < 0)
			fatal(MSGCO(CANTFORK, "\n Cannot create another process at this time.\n\tTry again later or\n\tuse local problem reporting procedures. (co20)\n"));  /* MSG */
		if (i == 0) {
			setuid(getuid());
			setgid(getgid());
			unlink(gfilename);
			exit(0);
		}
		else {
			wait(&status);
		}
	}
}


mkdelt(pkt,sp,osp,diffloop,orig_nlines)
struct packet *pkt;
struct sid *sp, *osp;
int diffloop;
int orig_nlines;
{
	extern time_t Timenow;
	struct deltab dt;
	char str[BUFSIZ];
	char *del_ba(), *strncpy();
	int newser;
	register char *p;
	int ser_inc, opred, nulldel;

	if (!diffloop && pkt->p_verbose) {
		sid_ba(sp,str);
		fprintf(pkt->p_stdout,"%s\n",str);
		fflush(pkt->p_stdout);
	}
	sprintf(str,"%c%c00000\n",CTLCHAR,HEAD);
	putline(pkt,str);
	newstats(pkt,str,"0");
	dt.d_sid = *sp;

	/*
	Check if 'null' deltas should be inserted
	(only if 'null' flag is in file and
	releases are being skipped) and set
	'nulldel' indicator appropriately.
	*/
	if (Sflags[NULLFLAG - 'a'] && (sp->s_rel > osp->s_rel + 1) &&
			!sp->s_br && !sp->s_seq &&
			!osp->s_br && !osp->s_seq)
		nulldel = 1;
	else
		nulldel = 0;
	/*
	Calculate how many serial numbers are needed.
	*/
	if (nulldel)
		ser_inc = sp->s_rel - osp->s_rel;
	else
		ser_inc = 1;
	/*
	Find serial number of the new delta.
	*/
	newser = dt.d_serial = maxser(pkt) + ser_inc;
	/*
	Find old predecessor's serial number.
	*/
	opred = sidtoser(osp,pkt);
	if (nulldel)
		dt.d_pred = newser - 1;	/* set predecessor to 'null' delta */
	else
		dt.d_pred = opred;
	dt.d_datetime = Timenow;
	strncpy(dt.d_pgmr,logname(),LOGSIZE-1);
	dt.d_type = 'D';
	del_ba(&dt,str);
	putline(pkt,str);
	if (ilist)
		mkixg(pkt,INCLUSER,INCLUDE);
	if (elist)
		mkixg(pkt,EXCLUSER,EXCLUDE);
	if (glist)
		mkixg(pkt,IGNRUSER,IGNORE);
	if (Mrs) {
		if (!(p = Sflags[VALFLAG - 'a']))
			fatal(MSGCM(MRNOTALD, "The SCCS file you specified does not allow MR numbers. (cm24)\n"));  /* MSG */
		if (*p && !diffloop && valmrs(pkt,p))
			fatal(MSGCM(INVMRS, "Use a valid MR number or numbers. (cm25)\n"));  /* MSG */
		putmrs(pkt);
	}
	else if (Sflags[VALFLAG - 'a'])
		fatal(MSGCM(MRSREQ, "Specify an MR number or numbers on the command line or as\n\tstandard input. (cm26)\n"));  /* MSG */
#ifdef CASSI
/*
*
* CMF enhancement
*
*/
	if(Sflags[CMFFLAG - 'a'])
		{
		 if(Mrs)
			{
			 error(MSGSTR(INPTCMRIGNRD, " input CMR's ignored"));  /* MSG */
			 Mrs = "";
			}
		if(!deltack(pkt->p_file,Cmrs,Nsid,Sflags[CMFFLAG - 'a']))
			{
			 fatal(MSGSTR(DELTADND, "\n Delta denied due to CMR difficulties\n"));  /* MSG */
			}
		 putcmrs(pkt); /* this routine puts cmrs on the out put file */
		}
#endif
	putline(pkt,Comments);
	sprintf(str,CTLSTR,CTLCHAR,EDELTAB);
	putline(pkt,str);
	if (nulldel)			/* insert 'null' deltas */
		while (--ser_inc) {
			sprintf(str,"%c%c %s/%s/%05u\n", CTLCHAR, STATS,
				"00000", "00000", orig_nlines);
			putline(pkt,str);
			dt.d_sid.s_rel -= 1;
			dt.d_serial -= 1;
			if (ser_inc != 1)
				dt.d_pred -= 1;
			else
				dt.d_pred = opred;	/* point to old pred */
			del_ba(&dt,str);
			putline(pkt,str);
			sprintf(str,"%c%c ",CTLCHAR,COMMENTS);
			putline(pkt,str);
			putline(pkt,MSGSTR(AUTONDELTA,"AUTO NULL DELTA\n"));
			sprintf(str,CTLSTR,CTLCHAR,EDELTAB);
			putline(pkt,str);
		}
	return(newser);
}


mkixg(pkt,reason,ch)
struct packet *pkt;
int reason;
char ch;
{
	int n;
	char str[512];

	sprintf(str,"%c%c",CTLCHAR,ch);
	putline(pkt,str);
	for (n = maxser(pkt); n; n--) {
		if (pkt->p_apply[n].a_reason == reason) {
			sprintf(str," %u",n);
			putline(pkt,str);
		}
	}
	putline(pkt,"\n");
}


# define	LENMR	60

putmrs(pkt)
struct packet *pkt;
{
	register char **argv;
	char str[LENMR+6];
	extern char **Varg;

	for (argv = &Varg[VSTART]; *argv; argv++) {
		sprintf(str,"%c%c %s\n",CTLCHAR,MRNUM,*argv);
		putline(pkt,str);
	}
}


#ifdef CASSI

/*
*
*	putcmrs takes the cmrs list on the Mrs line built by deltack
* 	and puts them in the packet
*	
*/
	putcmrs(pkt)    
	struct packet *pkt;
	{
		char str[510];
		sprintf(str,"%c%c %s\n",CTLCHAR,MRNUM,Cmrs);
		putline(pkt,str);
	}


#endif

struct pfile *
rdpfile(pkt,sp)
register struct packet *pkt;
struct sid *sp;
{
	char *user;
	struct pfile pf;
	static struct pfile goodpf;
	char line[BUFSIZ];
	int cnt, uniq;
	FILE *in, *out;

	uniq = cnt = -1;
	user = logname();
	zero((char *)&goodpf,sizeof(goodpf));
	in = xfopen(auxf(pkt->p_file,'p'),0);
	out = xfcreat(auxf(pkt->p_file,'q'),0644);
	while (fgets(line,sizeof(line),in) != NULL) {
		pf_ab(line,&pf,1);
		if (equal(pf.pf_user,user)) {
			if (sp->s_rel == 0) {
				if (++cnt) {
					fclose(out);
					fclose(in);
					fatal(MSGSTR(MISSRKYLTR, "\n Specify an SID with the -r flag. (de1)\n"));  /* MSG */
				}
				goodpf = pf;
				continue;
			}
			else if ((sp->s_rel == pf.pf_nsid.s_rel &&
				sp->s_lev == pf.pf_nsid.s_lev &&
				sp->s_br == pf.pf_nsid.s_br &&
				sp->s_seq == pf.pf_nsid.s_seq) ||
				(sp->s_rel == pf.pf_gsid.s_rel &&
				sp->s_lev == pf.pf_gsid.s_lev &&
				sp->s_br == pf.pf_gsid.s_br &&
				sp->s_seq == pf.pf_gsid.s_seq)) {
					if (++uniq) {
						fclose(in);
						fclose(out);
						fatal(MSGSTR(AMBIG, _AMBIG));
					}
					goodpf = pf;
					continue;
			}
		}
		if(fputs(line,out)==EOF)
			FAILPUT;
	}
	fflush(out);
	fstat(fileno(out),&Statbuf);
	Szqfile = Statbuf.st_size;
	copy(auxf(pkt->p_file,'p'),Pfilename);
	fclose(out);
	fclose(in);
	if (!goodpf.pf_user[0])
		fatal(MSGSTR(NAMORSID, "\n The specified SID or your user name is not listed in the p-file.\n\tUse the correct user name or SID number. (de2)\n"));  /* MSG */
	return(&goodpf);
}


FILE *
dodiff(newf,oldf,difflim)
char *newf, *oldf;
int difflim;
{
	register int i;
	int pfd[2];
	extern char Diffpgm[];
	char num[10];

	xpipe(pfd);
	if ((i = fork()) < 0) {
		close(pfd[0]);
		close(pfd[1]);
		fatal(MSGCO(CANTFORK, "\n Cannot create another process at this time.\n\tTry again later or\n\tuse local problem reporting procedures. (co20)\n"));  /* MSG */
	}
	else if (i == 0) {
		close(pfd[0]);
		close(1);
		dup(pfd[1]);
		close(pfd[1]);
		for (i = 5; i < 15; i++)
			close(i);
		sprintf(num,"%d",difflim);
		execl(Diffpgm,Diffpgm,oldf,newf,num,"-s", (char *)0);
		close(1);
		exit(32);	/* tell parent that 'execl' failed */
	}
	close(pfd[1]);
	return fdfopen(pfd[0],0);
}


getdiff(type,plinenum)
register char *type;
register int *plinenum;
{
	char line[514];   /* 512 +2 space for bdiff A13759 */ 
	register char *p;
	char *rddiff(), *linerange();
	int num_lines = 0;    /* A13759 */
	static int chg_num, chg_ln;
	int lowline, highline;

	if ((p = rddiff(line,514)) == NULL)    /* A13759 */
		return(0);

	if (*p == '-') {
		*type = INS;
		*plinenum = chg_ln;
		num_lines = chg_num;
	}
	else {
		p = linerange(p,&lowline,&highline);
		*plinenum = lowline;

		switch(*p++) {
		case 'd':
			num_lines = highline - lowline + 1;
			*type = DEL;
			skipline(line,num_lines);
			break;

		case 'a':
			linerange(p,&lowline,&highline);
			num_lines = highline - lowline + 1;
			*type = INS;
			break;

		case 'c':
			chg_ln = lowline;
			num_lines = highline - lowline + 1;
			linerange(p,&lowline,&highline);
			chg_num = highline - lowline + 1;
			*type = DEL;
			skipline(line,num_lines);
			break;
		}
	}

	return(num_lines);
}


insert(pkt,linenum,n,ser)
register struct packet *pkt;
register int linenum;
register int n;
int ser;
{
	char str[514];   /* A13759 */
        char *rddiff();

	after(pkt,linenum);
	sprintf(str,"%c%c %u\n",CTLCHAR,INS,ser);
	putline(pkt,str);
	for (++n; --n; ) {
		rddiff(str,sizeof(str));
		putline(pkt,&str[2]);
	}
	sprintf(str,"%c%c %u\n",CTLCHAR,END,ser);
	putline(pkt,str);
}


delete(pkt,linenum,n,ser)
register struct packet *pkt;
register int linenum;
int n;
register int ser;
{
	char str[512];

	before(pkt,linenum);
	sprintf(str,"%c%c %u\n",CTLCHAR,DEL,ser);
	putline(pkt,str);
	after(pkt,linenum + n - 1);
	sprintf(str,"%c%c %u\n",CTLCHAR,END,ser);
	putline(pkt,str);
}


after(pkt,n)
register struct packet *pkt;
register int n;
{
	before(pkt,n);
	if (pkt->p_glnno == n)
		putline(pkt,(char *) 0);
}


before(pkt,n)
register struct packet *pkt;
register int n;
{
	while (pkt->p_glnno < n) {
		if (!readmod(pkt))
			break;
	}
}


char *
linerange(cp,low,high)
register char *cp;
register int *low, *high;
{
	cp = satoi(cp,low);
	if (*cp == ',')
		cp = satoi(++cp,high);
	else
		*high = *low;

	return(cp);
}


skipline(lp,num)
register char *lp;
register int num;
{
       char *rddiff();
	for (++num;--num;)
		rddiff(lp,514);  /* A13759 */ 
}


char *
rddiff(s,n)
register char *s;
register int n;
{
	register char *r;

	if ((r = fgets(s,n,Diffin)) != NULL && HADP)
		if(fputs(s,gpkt.p_stdout)==EOF)
			FAILPUT;
	return(r);
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
	ap = &pkt->p_apply[n];
	if (pkt->p_cutoff >= pkt->p_idel[n].i_datetime)
		switch(ap->a_code) {
	
		case SX_EMPTY:
			switch (ch) {
			case INCLUDE:
				condset(ap,APPLY,INCLUSER);
				break;
			case EXCLUDE:
				condset(ap,NOAPPLY,EXCLUSER);
				break;
			case IGNORE:
				condset(ap,SX_EMPTY,IGNRUSER);
				break;
			}
			break;
		case APPLY:
			fatal(MSGSTR(INTRNERR5, "\n There is an internal software error.\n\tUse local problem reporting procedures. (de5)\n"));  /* MSG */
			break;
		case NOAPPLY:
			fatal(MSGSTR(INTRNERR6, "\n There is an internal software error.\n\tUse local problem reporting procedures. (de6)\n"));  /* MSG */
			break;
		default:
			fatal(MSGSTR(INTRNERR7, "\n There is an internal software error.\n\tUse local problem reporting procedures. (de7)\n"));  /* MSG */
			break;
		}
}


escdodelt()	/* dummy routine for dodelt() */
{
}
#ifdef CASSI
fredck()	/*dummy routine for dodelt()*/
{
}
#endif

clean_up(n)
{
	if (mylock(auxf(gpkt.p_file,'z'),getpid())) {
		if (gpkt.p_iop)
			fclose(gpkt.p_iop);
		if (Xiop) {
			fclose(Xiop);
			unlink(auxf(gpkt.p_file,'x'));
		}
		if(Gin)
			fclose(Gin);
		unlink(auxf(gpkt.p_file,'d'));
		unlink(auxf(gpkt.p_file,'q'));
		xrm(&gpkt);
		ffreeall();
		unlockit(auxf(gpkt.p_file,'z'),getpid());
	}
}


fgetchk(file,pkt)
register char	*file;
register struct packet *pkt;
{
	FILE	*iptr;
	register int c, k, l;
	int lastc = '\n';

	iptr = xfopen(file,0);
	k = 1;
	l = 0;
	while ( (c=getc(iptr)) != EOF ) {
		/*
		 * Make sure line does not start with a Ctrl-A
		 */
		if ( l == 0 && c == CTLCHAR ) {
			fclose(iptr);
			sprintf(ErrMsg, 	/* MSG */
			  MSGSTR(ILLDAT,ILL_DATA),k, auxf(pkt->p_file,'g'));
			fatal(ErrMsg);
		}


		/*
		 * Make sure file does not have any embedded nulls
		 */
		
		if ( c == '\0' ) {
			fclose(iptr);
			sprintf(ErrMsg, 	/* MSG */
			  MSGSTR(ILLDAT,ILL_DATA),k, auxf(pkt->p_file,'g'));
			fatal(ErrMsg);
		}

		/*
		 * Check line length
		 */

		if (++l >= 512) {
			fclose(iptr);
		}

		if( l == 512 ) {
			printf("\nmax line length (510 bytes) exceeded \n");
			fatal(ErrMsg);
			}
		
		if ( c == '\n' ) {
			l = 0;
			++k;
		}
		lastc = c;
	}

	fclose(iptr);
	/*
	 * The last character in the file must be a newline
	 */
	if ( lastc != '\n')
		fatal(MSGCO(PRMTREOF,"\n The end of the file was premature.\n\tMake sure that the last line of the file ends with a newline character or\n\tuse local problem reporting procedures. (co5)\n"));	/*MSG*/
}
