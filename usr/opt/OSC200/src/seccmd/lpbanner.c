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
static char	*sccsid = "@(#)$RCSfile: lpbanner.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 14:29:15 $";
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
/*
 * Copyright (c) 1989 SecureWare, Inc.  All Rights Reserved.
 */



#include <sys/secdefines.h>

#if SEC_MAC

#include <sys/types.h>
#include <sys/security.h>
#include <mandatory.h>
#include <prot.h>
#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <ctype.h>
#include <varargs.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "lpbanner_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LPBANNER,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#ifndef BANNERPRO
#define	BANNERPRO	"/tcb/files/SWbanner.pro"
#endif

extern char	*sys_errlist[];
extern char	*malloc(), *strrchr(), *strdup(), *getenv();
extern char	*optarg;
extern int	optind;
extern int	opterr;

#if SEC_ENCODINGS
int	WordLength = SHORT_WORDS;
#endif
int	NoLabel = 0;
int	Rotation = 0;
int	PostScript = 0;

int	PageWidth = 0;
int	PageLength = 0;

time_t	CurrentTime;

char	*Name;
char	*UserName;
char	*JobName;
char	*Title;
char	*Printer = "unknown";
char	*PSBanner;

char	*ProtectAsString;
#if SEC_ENCODINGS
char	*ProtectAsClass;
char	*ILString;
char	*BannerString;
char	*ChannelString;
#endif

char	XXXX[] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
char	Newlines[] = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
char	Label1[] = "This output must be protected as:";
char	Label2[] = "unless manually reviewed and downgraded";
#if SEC_ENCODINGS
char	Label3[] = "The system has labeled this data:";
#endif
char	Usage[] = "usage: %s [-lLPrR] [-B bannerpro] [-u user] [-h host]\n\t\t[-j job] [-p printer] [-t title] file\n";
char	obuf[BUFSIZ];
char	DateString[25];	/* long enough for ctime string excluding newline */
char	HostName[128];
char	InitSeq[256];
char	TermSeq[256];
char	*command_name;

main(argc, argv)
	int argc;
	char *argv[];
{
	register mand_ir_t	*sl;
#if SEC_ENCODINGS
	register ilb_ir_t	*il;
#endif
	register struct passwd	*pw;
	register int		opt, i;

#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_LPBANNER,NL_CAT_LOCALE);
#endif

	command_name = argv[0];
	CurrentTime = time(0);
	strncpy(DateString, ctime(&CurrentTime), sizeof DateString - 1);
	gethostname(HostName, sizeof HostName);

	setvbuf(stdout, obuf, _IOFBF, BUFSIZ);

        set_auth_parameters(argc, argv);
#if SEC_MAC
        mand_init();
#endif

	opterr = 0;
	while ((opt = getopt(argc, argv, "lLPrRh:u:j:p:t:B:")) != EOF) {
		switch (opt) {
		case 'l':
			NoLabel = 1;
			break;
		case 'L':
#if SEC_ENCODINGS
			WordLength = LONG_WORDS;
#endif
			break;
		case 'P':
			PostScript = 1;
			break;
		case 'r':
			Rotation = 1;
			break;
		case 'R':
			Rotation = -1;
			break;
		case 'h':
			strncpy(HostName, optarg, sizeof HostName - 1);
			break;
		case 'u':
			Name = optarg;
			if ((pw = getpwnam(Name)) && pw->pw_gecos &&
			    pw->pw_gecos[0] != '\0') {
				UserName = malloc(strlen(pw->pw_gecos) + 1);
				if (UserName)
					strcpy(UserName, pw->pw_gecos);
			}
			break;
		case 'j':
			JobName = optarg;
			break;
		case 'p':
			Printer = optarg;
			setupprinter(Printer);
			break;
		case 't':
			Title = optarg;
			break;
		case 'B':
			PSBanner = optarg;
			break;
		default:
			fprintf(stderr, Usage, argv[0]);
			exit(1);
		}
	}

	if (optind != argc - 1) {
		fprintf(stderr, Usage, argv[0]);
		exit(1);
	}

	/*
	 * Establish defaults for unspecified parameters.
	 * User name for banner defaults to that of current
	 * real user id.  Title for banner defaults to last
	 * component of file name.  Job name defaults to
	 * full file name.
	 */

	if (Name == NULL) {
		pw = getpwuid(getuid());
		if (pw) {
			Name = malloc(strlen(pw->pw_name) + 1);
			if (Name)
				strcpy(Name, pw->pw_name);
			if (pw->pw_gecos && pw->pw_gecos[0] != '\0') {
				UserName = malloc(strlen(pw->pw_gecos) + 1);
				if (UserName)
					strcpy(UserName, pw->pw_gecos);
			}
		}
		if (Name == NULL)
			Name = "Unknown";
	}
	endpwent();

	if (Title == NULL) {
		Title = strrchr(argv[optind], '/');
		if (Title)
			++Title;
		else
			Title = argv[optind];
	}

	if (JobName == NULL)
		JobName = argv[optind];
	
	if (PageWidth <= 0)
		PageWidth = PostScript ? 612 : 80;

	if (PageLength <= 0)
		PageLength = PostScript ? 792 : 66;

	/*
	 * Allocate space for SL and IL internal representations.
	 */

	sl = mand_alloc_ir();
#if SEC_ENCODINGS
	il = ilb_alloc_ir();
#endif

	if (sl == (mand_ir_t *) 0
#if SEC_ENCODINGS
	|| il == (ilb_ir_t *) 0
#endif
	   ) {
		fprintf(stderr, MSGSTR(LPBANNER_2, "%s: can't allocate space for label IRs\n"),
			argv[0]);
		exit(1);
	}

	/*
	 * Retrieve the labels from the file.
	 */

	if (statslabel(argv[optind], sl) == -1)
		if (errno == EINVAL)
			bcopy(mand_syshi, sl, mand_bytes());
		else {
			fprintf(stderr,
				MSGSTR(LPBANNER_3, "%s: can't get sensitivity label for %s: %s\n"),
				argv[0], argv[optind], sys_errlist[errno]);
			exit(1);
		}
	
#if SEC_ENCODINGS
	if (statilabel(argv[optind], il) == -1)
		if (errno == EINVAL)
			bcopy(mand_syshi, il, ilb_bytes());
		else {
			fprintf(stderr,
				MSGSTR(LPBANNER_4, "%s: can't get information label for %s: %s\n"),
				argv[0], argv[optind], sys_errlist[errno]);
			exit(1);
		}

	/*
	 * Allocate space for the various label strings.
	 */

	ILString = malloc(l_information_label_tables->l_max_length);
 	ProtectAsString = malloc(l_information_label_tables->l_max_length +
				 l_sensitivity_label_tables->l_max_length);
 	ChannelString = malloc(l_channel_tables->l_max_length);
 	BannerString = malloc(l_printer_banner_tables->l_max_length);
	
	/*
	 * Now set up the label strings.
	 *
	 * The "Protect As" classification, printed at the top and
	 * bottom of the banner page, is the greater of the encodings-
	 * defined minimimum and the classification from the file's
	 * sensitivity label.
	 */

	i = L_MAX(sl->class, *l_classification_protect_as);
	ProtectAsClass = l_long_classification[i];

	/*
	 * The "Protect As" string consists of the protect as classification,
	 * the category words from the file's sensitivity label, and the
	 * access-related marking words from the file's information label.
	 */

	strcpy(ProtectAsString, ProtectAsClass);
	l_convert(&ProtectAsString[strlen(ProtectAsString)],
			sl->class, NO_CLASSIFICATION,
			sl->cat, il->cat + CATWORDS,
			l_sensitivity_label_tables,
			NO_PARSE_TABLE, WordLength, ALL_ENTRIES);
	l_convert(&ProtectAsString[strlen(ProtectAsString)],
			sl->class, NO_CLASSIFICATION,
			sl->cat, il->cat + CATWORDS,
			l_information_label_tables,
			NO_PARSE_TABLE, WordLength, ACCESS_RELATED);

	/*
	 * The IL string is a straight conversion of the file's
	 * information label.
	 */

	l_convert(ILString, il->class, l_long_classification,
			il->cat, il->cat + CATWORDS,
			l_information_label_tables,
			NO_PARSE_TABLE, WordLength, ALL_ENTRIES);

	/*
	 * The banner string has no classification and category words
	 * from the file's sensitivity label.
	 */

	l_convert(BannerString, sl->class, NO_CLASSIFICATION,
			sl->cat, il->cat + CATWORDS,
			l_printer_banner_tables,
			NO_PARSE_TABLE, WordLength, ALL_ENTRIES);

	/*
	 * The "Handle Via" string also has no classification and
	 * contains category words from the sensitivity label.
	 */

	l_convert(ChannelString, sl->class, NO_CLASSIFICATION,
			sl->cat, il->cat + CATWORDS,
			l_channel_tables,
			NO_PARSE_TABLE, WordLength, ALL_ENTRIES);

#else /* !SEC_ENCODINGS */

	ProtectAsString = strdup(mand_ir_to_er(sl));

#endif /* !SEC_ENCODINGS */
	
	/*
	 * Generate a header page, followed by a copy of the
	 * standard input, followed by a trailer page.
	 */
	
	if (PostScript)
		psbanner(1);
	else
		asciibanner(1);

	copyfile(0);

	if (PostScript)
		psbanner(0);
	else
		asciibanner(0);

	exit(0);
}


/*
 * Set up the global variables describing relevant
 * printer characteristics.
 */

setupprinter(printer)
	char	*printer;
{
#ifdef USE_PRINTCAP
	static char	pcap[BUFSIZ], *bp;

	if (pgetent(pcap, printer) <= 0) {
		fprintf(stderr,
			MSGSTR(LPBANNER_1, "%s: Unknown printer type: %s\n"),
			command_name, printer);
		exit(1);
	}
	bp = InitSeq; pgetstr("is", &bp);
	bp = TermSeq; pgetstr("ts", &bp);
	PageWidth = pgetnum("pw");
	PageLength = pgetnum("pl");
#else
	register struct pr_lp	*prlp;

	if ((prlp = getprlpnam(printer)) == NULL) {
		fprintf(stderr,
			MSGSTR(LPBANNER_1, "%s: Unknown printer type: %s\n"),
			command_name, printer);
		exit(1);
	}
	if (prlp->uflg.fg_initseq)
		strcpy(InitSeq, prlp->ufld.fd_initseq);
	if (prlp->uflg.fg_termseq)
		strcpy(TermSeq, prlp->ufld.fd_termseq);
	if (prlp->uflg.fg_linelen)
		PageWidth = prlp->ufld.fd_linelen;
	if (prlp->uflg.fg_pagelen)
		PageLength = prlp->ufld.fd_pagelen;
	endprlpent();
#endif
}


/*
 * Generate a banner page.  The isheader argument controls whether or
 * not job identification information will be printed.
 */

asciibanner(isheader)
	int	isheader;
{
	register int	i, Lines = 0;

	/*
	 * Emit the initialization characters for the printer.
	 */

	printf("%s", InitSeq);

#if SEC_ENCODINGS
	/*
	 * Print the protect as classification.
	 */

	center(ProtectAsClass, 0);
	Lines = 1;
#endif

	/*
	 * Print the burst markers.
	 */

	for (i = 0; i < 4; ++i)
		printf("%.*s\n", PageWidth, XXXX);
	
	Lines += 4;

	/*
	 * Print out job identification information if this is a
	 * header page.
	 */

	if (isheader) {
		putchar('\n');
		blockletters(Name);	/* consumes 8 lines */

		putchar('\n');
		blockletters(Title);

		if (UserName)
			printf("\n\t\t%s: %s\n\n", MSGSTR(LPBANNER_5, "User"),
				UserName);
		else
			printf("\n\n\n");

		printf("\t\t%s: %s    %s: %s\n\n", MSGSTR(LPBANNER_6, "Job"),
			JobName, MSGSTR(LPBANNER_7, "Printer"), Printer);
		printf("\t\t%s: %s\n\n\n", MSGSTR(LPBANNER_8, "Date"),
			DateString);

		printf("%.*s\n", PageWidth, XXXX);
		printf("%.*s\n", PageWidth, XXXX);
		Lines += 28;
	}

	/*
	 * Generate enough blank lines to center the labels
	 * vertically in the remaining space.
	 */

#if SEC_ENCODINGS
	/*
	 * We assume that the labels will consume 20 lines; 11 lines
	 * of fixed length text and whitespace, and 9 lines for the 4
	 * variable length label strings.  We also take into account
	 * the 5 lines of burst marker and protect as classification
	 * at the bottom of the page.
	 */

	i = (PageLength - (Lines + 25)) / 2;
#else
	/*
	 * We assume that the label will consume 8 lines; 4 lines
	 * of fixed length text and whitespace, and 4 lines for the
	 * variable length sensitivity label string.  We also take
	 * into account the 4 lines of burst marker at the bottom
	 * of the page.
	 */
	
	i = (PageLength - (Lines + 12)) / 2;
#endif
	if (i > 0) {
		printf("%.*s", i, Newlines);
		Lines += i;
	}

	/*
	 * Print out the protect as label string.
	 */

	center(Label1, 0);
	putchar('\n');
	Lines += putlabel(ProtectAsString) + 4;
	putchar('\n');
	center(Label2, 0);

#if SEC_ENCODINGS
	/*
	 * Print out the actual information label.
	 */

	printf("\n\n");
	center(Label3, 0);
	putchar('\n');
	Lines += putlabel(ILString) + 6;
	printf("\n\n");

	/*
	 * Print out the banner string.
	 */

	Lines += putlabel(BannerString) + 1;
	putchar('\n');

	/*
	 * Print out the handle via string.
	 */

	Lines += putlabel(ChannelString);

	printf("%.*s", PageLength - (Lines + 5), Newlines);
#else
	printf("%.*s", PageLength - (Lines + 4), Newlines);
#endif

	/*
	 * Print out the bottom-of-page burst marker.
	 */

	for (i = 0; i < 4; ++i)
		printf("%.*s\n", PageWidth, XXXX);

#if SEC_ENCODINGS
	/*
	 * Print out the bottom-of-page protect as classification.
	 */

	center(ProtectAsClass, 0);
#endif

	/*
	 * Emit the termination characters for the printer.
	 */

	printf("%s", TermSeq);

}


/*
 * Generate a PostScript banner page.  The isheader argument controls
 * whether or not job identification information will be printed.
 */

psbanner(isheader)
	int	isheader;
{
	register int	fd;

	/*
	 * If this is a header page, copy the prologue file to the
	 * standard output and generate the LabelSetup call.
	 */
	
	if (isheader) {
		/*
		 * Open the banner prologue and copy it out
		 */
		if (PSBanner == (char *) 0)
			PSBanner = getenv("BANNERPRO");
		if (PSBanner == (char *) 0 || *PSBanner == '\0')
			PSBanner = BANNERPRO;
		fd = open(PSBanner, 0);
		if (fd < 0) {
			perror(PSBanner);
			exit(1);
		}
		copyfile(fd);
		close(fd);

		/*
		 * Generate the call to LabelSetup.  The arguments are:
		 *
		 * pagewidth pageheight rotation ProtectAsClass
		 * ProtectAsString ILString BannerString ChannelString
		 *
		 * The pagewidth and pageheight are in units of points
		 * (1/72nd of an inch).  The rotation argument is an
		 * integer that determines the position and orientation of
		 * the per-page label: a 0 means top and bottom edges,
		 * a positive number means rotate 90 degrees counter-
		 * clockwise so that the left edge is up, and a negative
		 * number means rotate 90 degrees clockwise so that the
		 * right edge is up.  All remaining arguments are
		 * PostScript strings.
		 */

		printf("%d %d\n", PageWidth, PageLength);
		printf("%d\n", Rotation);
#if SEC_ENCODINGS
		psarray(ProtectAsClass, 0, 0);
#else
		psarray(NULL, 0, 0);
#endif
		psarray(ProtectAsString, 0, 0);
#if SEC_ENCODINGS
		psarray(ILString, 0, 0);
		psarray(BannerString, 0, 0);
		psarray(ChannelString, 0, 0);
		if (NoLabel)
			psarray(NULL, 1, Rotation);
		else
			psarray(ILString, 1, Rotation);
#else
		psarray(NULL, 0, 0);
		psarray(NULL, 0, 0);
		psarray(NULL, 0, 0);
		if (NoLabel)
			psarray(NULL, 1, Rotation);
		else
			psarray(ProtectAsString, 1, Rotation);
#endif
		printf("LabelSetup\n");

		/*
		 * Save current VM state to restore for trailer page.
		 */
		printf("$LabelDict$ begin save /LabelSave exch def\n");
	} else
		/*
		 * Restore saved VM state for trailer page
		 */
		printf("$LabelDict$ begin LabelSave restore\n");

	/*
	 * Generate the call to Banner.  The arguments are:
	 *
	 * loginname username jobid title printer date
	 *
	 * All arguments are string values.
	 */

	psstrings(1, Name, "@", HostName, NULL);
	if (UserName)
		psstrings(1, "(", UserName, ")", NULL);
	else
		psstrings(1, UserName);
	psstrings(1, JobName, NULL);
	psstrings(1, Title, NULL);
	psstrings(1, Printer, NULL);
	psstrings(1, DateString, NULL);
	printf("Banner end\n");
}


/*
 * Print the specified label string on as many lines as necessary,
 * centering each line on the page and returning the number of
 * lines printed.
 */

putlabel(s)
	register char	*s;
{
	register int	len = strlen(s);
	register int	lines = 0;

	/*
	 * If the string is wider than the page, break it
	 * into multiple lines at word boundaries.
	 */
	while (len > PageWidth) {
		register int	i;

		/*
		 * Scan backwards for a word boundary starting at
		 * the page width.
		 */
		for (i = PageWidth; i > 0 && s[i] != ' ' && s[i] != '/'; --i)
			;

		/*
		 * If no word boundary was found, break the string in
		 * the middle of a word.
		 */
		if (i == 0)
			i = PageWidth;

		/*
		 * If the boundary was a '/' and there is room, cause the
		 * '/' to appear at the end of this line rather than at the
		 * start of the next one.
		 */
		if (i < PageWidth && s[i] == '/')
			++i;

		/*
		 * Put out this part of the string.
		 */
		center(s, i);

		while (s[i] == ' ')
			++i;
		len -= i;
		s += i;
		++lines;
	}

	/*
	 * Put out the remainder of the string.
	 */
	if (len) {
		center(s, len);
		++lines;
	}

	return lines;
}


/*
 * Print the first len characters of the specified string centered
 * on the page.  If len is 0, print the entire string.
 */

center(s, len)
	char	*s;
	int	len;
{
	if (len == 0)
		len = strlen(s);

	printf("%*.*s\n", (PageWidth + len) / 2, len, s);
}


/*
 * Definitions and data for the blockletters() function
 */

#define	BANNER_ROWS	7
#define	BANNER_COLS	8
#define	BANNER_CHAR	'#'

unsigned char ctab[][BANNER_ROWS] = {
	{    0,    0,    0,    0,    0,    0,    0 },	/*   */
	{  034,  034,  034,  010,    0,  034,  034 },	/* ! */
	{ 0167, 0167,  042,    0,    0,    0,    0 },	/* " */
	{  024,  024, 0177,  024, 0177,  024,  024 },	/* # */
	{  076, 0111, 0110,  076,  011, 0111,  076 },	/* $ */
	{ 0161, 0122, 0164,  010,  027,  045, 0107 },	/* % */
	{  030,  044,  030,  070, 0105, 0102,  071 },	/* & */
	{  034,  034,  010,  020,    0,    0,    0 },	/* ' */
	{  014,  020,  040,  040,  040,  020,  014 },	/* ( */
	{  030,   04,   02,   02,   02,   04,  030 },	/* ) */
	{    0,  042,  024, 0177,  024,  042,    0 },	/* * */
	{    0,  010,  010,  076,  010,  010,    0 },	/* + */
	{    0,    0,    0,  034,  034,  010,  020 },	/* , */
	{    0,    0,    0,  076,    0,    0,    0 },	/* - */
	{    0,    0,    0,    0,  034,  034,  034 },	/* . */
	{   01,   02,   04,  010,  020,  040, 0100 },	/* / */
	{  034,  042, 0101, 0101, 0101,  042,  034 },	/* 0 */
	{  010,  030,  050,  010,  010,  010,  076 },	/* 1 */
	{  076, 0101,   01,  076, 0100, 0100, 0177 },	/* 2 */
	{  076, 0101,   01,  076,   01, 0101,  076 },	/* 3 */
	{ 0100, 0102, 0102, 0102, 0177,   02,   02 },	/* 4 */
	{ 0177, 0100, 0100, 0176,   01, 0101,  076 },	/* 5 */
	{  076, 0101, 0100, 0176, 0101, 0101,  076 },	/* 6 */
	{ 0177, 0102,   04,  010,  020,  020,  020 },	/* 7 */
	{  076, 0101, 0101,  076, 0101, 0101,  076 },	/* 8 */
	{  076, 0101, 0101,  077,   01, 0101,  076 },	/* 9 */
	{  010,  034,  010,    0,  010,  034,  010 },	/* : */
	{  034,  034,    0,  034,  034,  010,  020 },	/* ; */
	{   04,  010,  020,  040,  020,  010,   04 },	/* < */
	{    0,    0,  076,    0,  076,    0,    0 },	/* = */
	{  020,  010,   04,   02,   04,  010,  020 },	/* > */
	{  076, 0101,   01,  016,  010,    0,  010 },	/* ? */
	{  076, 0101, 0135, 0135, 0136, 0100,  076 },	/* @ */
	{  010,  024,  042, 0101, 0177, 0101, 0101 },	/* A */
	{ 0176, 0101, 0101, 0176, 0101, 0101, 0176 },	/* B */
	{  076, 0101, 0100, 0100, 0100, 0101,  076 },	/* C */
	{ 0176, 0101, 0101, 0101, 0101, 0101, 0176 },	/* D */
	{ 0177, 0100, 0100, 0174, 0100, 0100, 0177 },	/* E */
	{ 0177, 0100, 0100, 0174, 0100, 0100, 0100 },	/* F */
	{  076, 0101, 0100, 0117, 0101, 0101,  076 },	/* G */
	{ 0101, 0101, 0101, 0177, 0101, 0101, 0101 },	/* H */
	{  034,  010,  010,  010,  010,  010,  034 },	/* I */
	{   01,   01,   01,   01, 0101, 0101,  076 },	/* J */
	{ 0102, 0104, 0110, 0160, 0110, 0104, 0102 },	/* K */
	{ 0100, 0100, 0100, 0100, 0100, 0100, 0177 },	/* L */
	{ 0101, 0143, 0125, 0111, 0101, 0101, 0101 },	/* M */
	{ 0101, 0141, 0121, 0111, 0105, 0103, 0101 },	/* N */
	{ 0177, 0101, 0101, 0101, 0101, 0101, 0177 },	/* O */
	{ 0176, 0101, 0101, 0176, 0100, 0100, 0100 },	/* P */
	{  076, 0101, 0101, 0101, 0105, 0102,  075 },	/* Q */
	{ 0176, 0101, 0101, 0176, 0104, 0102, 0101 },	/* R */
	{  076, 0101, 0100,  076,   01, 0101,  076 },	/* S */
	{ 0177,  010,  010,  010,  010,  010,  010 },	/* T */
	{ 0101, 0101, 0101, 0101, 0101, 0101,  076 },	/* U */
	{ 0101, 0101, 0101, 0101,  042,  024,  010 },	/* V */
	{ 0101, 0111, 0111, 0111, 0111, 0111,  066 },	/* W */
	{ 0101,  042,  024,  010,  024,  042, 0101 },	/* X */
	{ 0101,  042,  024,  010,  010,  010,  010 },	/* Y */
	{ 0177,   02,   04,  010,  020,  040, 0177 },	/* Z */
	{  076,  040,  040,  040,  040,  040,  076 },	/* [ */
	{ 0100,  040,  020,  010,   04,   02,   01 },	/* \ */
	{  076,   02,   02,   02,   02,   02,  076 },	/* ] */
	{  010,  024,  042,    0,    0,    0,    0 },	/* ^ */
	{    0,    0,    0,    0,    0,    0, 0177 },	/* _ */
	{  034,  034,  010,   04,    0,    0,    0 },	/* ` */
	{    0,  014,  022,  041,  077,  041,  041 },	/* a */
	{    0,  076,  041,  076,  041,  041,  076 },	/* b */
	{    0,  036,  041,  040,  040,  041,  036 },	/* c */
	{    0,  076,  041,  041,  041,  041,  076 },	/* d */
	{    0,  077,  040,  076,  040,  040,  077 },	/* e */
	{    0,  077,  040,  076,  040,  040,  040 },	/* f */
	{    0,  036,  041,  040,  047,  041,  036 },	/* g */
	{    0,  041,  041,  077,  041,  041,  041 },	/* h */
	{    0,   04,   04,   04,   04,   04,   04 },	/* i */
	{    0,   01,   01,   01,   01,  041,  036 },	/* j */
	{    0,  041,  042,  074,  044,  042,  041 },	/* k */
	{    0,  040,  040,  040,  040,  040,  077 },	/* l */
	{    0,  041,  063,  055,  041,  041,  041 },	/* m */
	{    0,  041,  061,  051,  045,  043,  041 },	/* n */
	{    0,  036,  041,  041,  041,  041,  036 },	/* o */
	{    0,  076,  041,  041,  076,  040,  040 },	/* p */
	{    0,  036,  041,  041,  045,  042,  035 },	/* q */
	{    0,  076,  041,  041,  076,  042,  041 },	/* r */
	{    0,  036,  040,  036,   01,  041,  036 },	/* s */
	{    0,  037,   04,   04,   04,   04,   04 },	/* t */
	{    0,  041,  041,  041,  041,  041,  036 },	/* u */
	{    0,  041,  041,  041,  041,  022,  014 },	/* v */
	{    0,  041,  041,  041,  055,  063,  041 },	/* w */
	{    0,  041,  022,  014,  014,  022,  041 },	/* x */
	{    0,  021,  012,   04,   04,   04,   04 },	/* y */
	{    0,  077,   02,   04,  010,  020,  077 },	/* z */
	{  034,  040,  040, 0140,  040,  040,  034 },	/* { */
	{  010,  010,  010,    0,  010,  010,  010 },	/* | */
	{  034,   02,   02,   03,   02,   02,  034 },	/* } */
	{  060, 0111,   06,    0,    0,    0,    0 },	/* ~ */
	{ 0377, 0377, 0377, 0377, 0377, 0377, 0377 } 	/* DEL */
};


/*
 * Print the specified string in block letters.
 */

blockletters(str)
	char	*str;
{
	register char	*s, *sMax;
	register int	row, col, bit, mask;

	if (strlen(str) > PageWidth / BANNER_COLS)
		sMax = &str[PageWidth / BANNER_COLS];
	else
		sMax = &str[strlen(str)];

	for (row = 0; row < BANNER_ROWS; ++row) {
		for (s = str; s < sMax; ++s) {
			if ((*s & 0177) < ' ')
				mask = ctab[' '][row];
			else
				mask = ctab[(*s & 0177) - ' '][row];
			bit = 0200;
			for (col = 0; col < BANNER_COLS; ++col) {
				if (mask & bit)
					putchar(BANNER_CHAR);
				else
					putchar(' ');
				bit >>= 1;
			}
		}
		putchar('\n');
	}
	putchar('\n');
}


/*
 * Copy the specified file to the standard output
 */

copyfile(fd)
	int	fd;
{
	register int	cc;
	char		buf[BUFSIZ];

	fflush(stdout);		/* force out buffered data from header page */

	while ((cc = read(fd, buf, sizeof buf)) > 0)
		write(1, buf, cc);
}


/*
 * Form a PostScript array of strings, where each element will fit on a
 * line.  The line length is determined by PageWidth.
 */

psarray(str, small_ps, rotate)
	char *str;
	int small_ps;
	int rotate;
{
	char *line;
	int char_in_line;
	int char_in_str = 0;
	int back_track;
	int str_len;
	int vll;
	int width;

	if (str == (char *) 0)
		str_len = 0;
	else
		str_len = strlen(str);

	/*
	 * Compute the virtual line length given the indication on the
	 * point size.  The line length is in characters.  Handle rotation.
	 */
	width = rotate ? PageLength : PageWidth;
	if (small_ps)
		vll = (int) ((double) width * 2.0 / 7.65 - 2.0);
	else
		vll = (int) ((double) width / 7.65 - 2.0);

	/*
	 * The initial stack mark for a PostScript array.
	 */
	putchar('[');
	putchar(' ');

	if (str_len < vll)  {
		psstrings(0, str, NULL);
		putchar(' ');
	}
	else {
		line = malloc(vll + 1);
		line[vll] = '\0';

		while (char_in_str < str_len)  {
			char_in_line = 0;
			while ((char_in_line < vll) &&
			       (char_in_str < str_len))  {
				line[char_in_line] = str[char_in_str];

				/*
				 * Get rid of white space at start of line.  This affects
				 * centering.
				 */
				if ((char_in_line > 0) ||
				       !isascii(line[char_in_line]) ||
					!isspace(line[char_in_line]))
					char_in_line++;

				char_in_str++;
			}
			line[char_in_line] = str[char_in_str];

			/*
			 * Pare back the line if we are not at a word break.
			 */
			if ((char_in_str < str_len) &&
			    (!isascii(line[char_in_line]) ||
			     !isspace(line[char_in_line]))) {
				back_track = 0;
				while ((char_in_line >= 0) &&
				       (!isascii(line[char_in_line]) ||
					!isspace(line[char_in_line]))) {
					char_in_line--;
					char_in_str--;
					back_track++;
				}
				if ((char_in_line >= 0) && (char_in_line < vll-1) &&
				    isascii(line[char_in_line]) &&
				     isspace(line[char_in_line])) {
					char_in_line++;
					char_in_str++;
					back_track--;
				}

				/*
				 * If the unsplit word is just too long for
				 * the line, print as is and hope for the best.
				 * It is split, but not at a word boundary.
				 */
				if (char_in_line < 0)  {
					char_in_line += back_track;
					char_in_str += back_track;
				}
			}

			/*
			 * Get rid of white space at end of line.  This affects
			 * centering.
			 */
			while((char_in_line > 0) && isascii(line[char_in_line-1]) &&
			      isspace(line[char_in_line-1]))
					char_in_line--;

			line[char_in_line] = '\0';
			psstrings(0, line, NULL);
			putchar(' ');
		}

		free(line);
	}

	/*
	 * The PostScript array constructor.
	 */
	putchar(']');
	putchar('\n');
}


/*
 * Put the specified strings on standard output as a PostScript string.
 * 
 */

psstrings(is_nl, va_alist)
	int is_nl;
	va_dcl
{
	va_list			ap;
	register unsigned char	*s;
	register unsigned int	c;

	putchar('(');

	for (va_start(ap); s = va_arg(ap, unsigned char *); )
		while ((c = *s++)) {
			if (isascii(c) && (isprint(c) || isspace(c))) {
				if (c == '(' || c == ')' || c == '\\')
					putchar('\\');
				putchar(c);
			} else
				printf("\\%3o", c);
		}
	putchar(')');
	if (is_nl)
		putchar('\n');
	va_end(ap);
}
#endif /* SEC_MAC */
