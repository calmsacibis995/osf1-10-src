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
static char rcsid[] = "@(#)$RCSfile: val.c,v $ $Revision: 4.2.3.3 $ (OSF) $Date: 1993/10/11 19:00:44 $";
#endif
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: escdodelt, findsid, fredck, initarg, nextarg, process,
 *            report, validate, main
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *    
 * val.c  1.8 com/cmd/sccs/cmd/val.c, 9121320k, bos320 4/25/91 19:26:21";
 */

/************************************************************************/
/*									*/
/*  val -                                                               */
/*  val [-mname] [-rSID] [-s] [-ytype] file ...                         */
/*                                                                      */
/************************************************************************/

#include 	<locale.h>
#include	<wchar.h>
#include	<stdlib.h>
#include	<string.h>
#include	"defines.h"
#include	"had.h"

#include 	"val_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_VAL, Num, Str)

# define	FILARG_ERR	0200	/* no file name given */
# define	UNKDUP_ERR	0100	/* unknown or duplicate keyletter */
# define	CORRUPT_ERR	040	/* corrupt file error code */
# define	FILENAM_ERR	020	/* file name error code */
# define        INVALSID_ERR    010     /* invalid SID error */
# define	NONEXSID_ERR	04	/* non-existent SID error code */
# define	TYPE_ERR	02	/* type arg value error code */
# define	NAME_ERR	01	/* name arg value error code */
# define	TRUE		1
# define	FALSE		0

int	ret_code;	/* prime return code from 'main' program */
int	inline_err;	/* input line error code (from 'process') */
int	infile_err;	/* file error code (from 'validate') */
char    **xargv;        /* command line arguments */
int     silent;         /* set to suppress msgs */
char	*nargv[NVARGS];	/* XPG4: construct new argv in case of '-' */
int	nargc;		/* XPG4: construct new argv in case of '-' */

struct packet gpkt;
struct sid sid;

char	had[26];	/* had flag used in 'process' function */
char    *type;          /* ptr to type (-y) value */
char    *name;          /* ptr to name (-m) value */
char    *prline;        /* line to print upon error */
char	prline_buf[BUFSIZ];	/* XPG4 */
char	line[BUFSIZ];

char    ErrMsg[512];
struct  stat Statbuf;

nl_catd catd;

extern void create_new_argv(char **, char *);

/* This is the main program that determines whether the command line
 * comes from the standard input or read off the original command
 * line.  See VAL(I) for more information.
*/
main(argc,argv)
int argc;
char	*argv[];
{
	Fflags = FTLJMP;

        (void)setlocale(LC_ALL,"");
	catd =catopen(MF_SCCS, NL_CAT_LOCALE);

	if (argc == 2 && argv[1][0] == '-' && !(argv[1][1])) {
		while (fgets(line, sizeof(line) ,stdin))
			if (line[0] != '\n') {
				if (strlen(line) >= (sizeof(line) -1)) {
					printf(MSGSTR(TOOMNY, "val: Too many arguments in one command line.\n"));
					exit(0);
				}
				prline = strcpy(prline_buf, line);
				create_new_argv((char **)nargv, line);
				xargv = (char **)nargv;
				process();
			}
	}
	else {
		xargv = argv;
		nargc = argc;
		prline = NULL;
		process();
	}
	exit(ret_code);
}


void create_new_argv(char *nargv[], char *line)
{
	int	n;
	int	mb_cur_max = MB_CUR_MAX;
	wchar_t	wc;

	optind = 1;

	nargc = 1;
	nargv[0] = "val";

	while (*line != '\n') {
		n = mbtowc(&wc, line, mb_cur_max);
		if (n > 0 && iswspace(wc)) {		/* Skip blanks */
			*line = '\0';
			line += n;
			continue;
		}
		else if (n <= 0) {
			printf(MSGSTR(INVCHR, "val: Invalid character was found in command line.\n"));
			exit(0);
		}
		if (nargc >= (NVARGS - 1)) {
			printf(MSGSTR(TOOMNY, "val: Too many arguments in one command line.\n"));
			exit (0);
		}
		nargv[nargc++] = line;

		/* Advance command line for token */
               while (!iswspace(wc) && *line != '\n') {
                       line += n;      /* Advance command line for token */
                       n = mbtowc(&wc, line, mb_cur_max);
			if (n <= 0) {
			       printf(MSGSTR(INVCHR, "val: Invalid character was found in command line.\n"));
			       exit(0);
		       }
               }

	}
	*line = '\0';
	nargv[nargc] = "";
}

/* This function processes a command line.  It
 * determines which keyletter values are present on the command
 * line and assigns the values to the correct storage place.  It
 * then calls validate for each file name on the command line.
*/
process()
{
	register int	testklt;
	int	num_files;
	int	c, i;

	silent = FALSE;
	num_files = inline_err = 0;

	/*
	clear out had flags for each 'line' processed
	*/
	zero(had,sizeof(had));
	/*
	scan for all flags.
	*/
	while ((c = getopt(nargc, xargv, "m:r:sy:")) != -1) {
		testklt = TRUE;
		switch (c) {
			case 's':
				testklt = 0;
				/*
				turn on 'silent' flag.
				*/
				silent = TRUE;
				break;
			case 'r':
				/*
				check for invalid SID.
				*/
				if (setjmp(Fjmp))
					inline_err |= INVALSID_ERR;
				else {
					extern char *sid_ab();

					chksid(sid_ab(optarg,&sid),&sid);
					if ((sid.s_rel < MINR) ||
					    (sid.s_rel > MAXR))
						inline_err |= INVALSID_ERR;
				}
				break;
			case 'y':
				type = optarg;
				break;
			case 'm':
				name = optarg;
				break;
			default:
				inline_err |= UNKDUP_ERR;
		}
		/*
		use 'had' array and determine if the keyletter
		was given twice.
		*/
		if (had[c - 'a']++ && testklt++)
			inline_err |= UNKDUP_ERR;
	}
	num_files = nargc - optind;

	/*
	check if any files were named as arguments
	*/
	if (num_files == 0)
		inline_err |= FILARG_ERR;
	/*
	report any errors in command line.
	*/
	report(inline_err, "");
	/*
	loop through 'validate' for each file on command line.
	*/
	for (i = optind; i < nargc; i++) {
		int validate();

		do_file(xargv[i],validate);
	}
}


/* This function actually does the validation on the named file.
 * It determines whether the file is an SCCS-file or if the file
 * exists.  It also determines if the values given for type, SID,
 * and name match those in the named file.  An error code is returned
 * if any mismatch occurs.  See VAL(I) for more information.
*/
validate(file)
char    *file;
{
	extern char             *auxf(), *sname();
	extern struct idel      *dodelt();
	extern char             *Sflags[];
	register char           *p;
	struct stats            stats;

	infile_err = 0;

	if (setjmp(Fjmp)) {
		infile_err |= gpkt.p_iop? CORRUPT_ERR : FILENAM_ERR;
		goto out;
	}
	sinit(&gpkt,file,2);
	/*
	read delta table checking for errors and/or
	SID.
	*/
	if (!dodelt(&gpkt,&stats,(struct sid *)NULL,0))
		fmterr(&gpkt);

	finduser(&gpkt);
	doflags(&gpkt);
	/*
	check if 'y' flag matched '-y' arg value.
	*/
	if (HADY)
		if (!(p = Sflags[TYPEFLAG - 'a']) || !equal(type,p))
			infile_err |= TYPE_ERR;
	/*
	check if 'm' flag matched '-m' arg value.
	*/
	if (HADM)
		if (!equal(name,
		    (p = Sflags[MODFLAG - 'a'])? p : auxf(sname(file),'g')))
			infile_err |= NAME_ERR;

	if (gpkt.p_line[1] != BUSERTXT || gpkt.p_line[0] != CTLCHAR)
		fmterr(&gpkt);
	flushto(&gpkt,EUSERTXT,1);

	/*
	If a valid sid was specified, determine whether it occurs.
	*/
	if (HADR && !(inline_err & INVALSID_ERR))
		if (!findsid())
			infile_err |= NONEXSID_ERR;

	/*
	read remainder of file so 'readmod'
	can check for corruptness.
	*/
	gpkt.p_chkeof = 1;
	while (readmod(&gpkt))
		;

out:    if (gpkt.p_iop) fclose(gpkt.p_iop);
	report(infile_err,file);
	ffreeall();
}


/* Determine whether the requested sid exists.
 * If the sid is unambiguous (R.L or R.L.B.S) it must occur as is.
 * If R is given, any R.... is sufficient.
 * If R.L.B is given, any R.L.B.... is sufficient.
 */
findsid()
{
	register struct idel *rdp;
	register int n;

	for (n = maxser(&gpkt); n; n--) {
		rdp = &gpkt.p_idel[n];
		if (rdp->i_sid.s_rel == sid.s_rel)
			if (!sid.s_lev ||
			    sid.s_lev == rdp->i_sid.s_lev &&
			    sid.s_br == rdp->i_sid.s_br &&
			    (!sid.s_seq || sid.s_seq == rdp->i_sid.s_seq))
				return(TRUE);
	}
	return(FALSE);
}


/* This function will report the error that occurred on the command
 * line.  It will print one diagnostic message for each error that
 * was found in the named file.
*/
report(code,file)
register int	code;
register char	*file;
{
	char	percent;

	ret_code |= code;
	if (!code || silent) return;
	percent = '%';		/* '%' for -m and/or -y messages */
	if (prline) {
		printf("%s\n",prline);
		prline = NULL;
	}
	if (code & NAME_ERR)
		printf(MSGSTR(MMSMTCH, " %s: The value specified by -m does not match\n\
\tthe %cM%c identification keyword value specified in the SCCS file.\n"),file,percent,percent);
	if (code & TYPE_ERR)
		printf(MSGSTR(YMSMTCH, " %s: The parameter specified by -y\n\
\tdoes not match the text specified by %cY%c in the SCCS file.\n"),file,percent,percent);
	if (code & NONEXSID_ERR)
		printf(MSGSTR(SIDNOEXST, " %s: The specified SID does not exist.\n\
\tCheck the p-file for the correct SID number.\n"),file);
	if (code & INVALSID_ERR)
		printf(MSGSTR(SIDINVLD, " %s: The specified SID is not valid.\n\
\tCheck the p-file for the correct SID number.\n"),file);
	if (code & FILENAM_ERR)
		printf(MSGSTR(OPNSCCS, " %s: Cannot open the file or\n\
\tthe file is not an SCCS file.\n\
\tCheck path name and permissions.\n\
\tThe val command validates SCCS files only.\n"),file);
	if (code & CORRUPT_ERR)
		printf(MSGSTR(CRPTSCCS, " %s is a damaged SCCS file.\n\
\tThe specified SCCS file has been edited without use of SCCS conventions.\n\
\tRestore the most recent backup copy of this file.\n"),file);
	if (code & UNKDUP_ERR)
		printf(MSGSTR(DUPKEYLTR,
			"ERROR: Flag is not valid. (cm1)\n"));
	if (code & FILARG_ERR && (code &UNKDUP_ERR) == 0)
		printf(MSGSTR(MSGFILARG,
			"ERROR: Specify the file to process. (cm3)\n")); 
	if (code & (FILARG_ERR | UNKDUP_ERR))
		printf(MSGSTR(VALUSAGE,
"\tUsage: val [-m Identifier] [-r SID] [-s] [-y Type...] file...  | -\n\n"));
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
