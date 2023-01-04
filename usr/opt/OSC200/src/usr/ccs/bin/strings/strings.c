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
static char *sccsid = "@(#)$RCSfile: strings.c,v $ $Revision: 4.2.11.4 $ (DEC) $Date: 1993/10/07 19:16:15 $";
#endif

/* static char *sccsid = "@(#)strings.c	4.1 (Berkeley) 10/1/80"; */

/*
 * strings - 
 *
 * EDIT HISTORY
 * 	1/19/89 Pradeep Chetal (chetal)
 *		Octal 0200 is no longer passed thru the input (SMU-2452)
 *	        Historically '\n' & '\f' are allowed thru the input.
 *		Added DEC Copyright and SCCS keywords.
 *		Also merged MIPS and VAX version.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <a.out.h>
#include <ctype.h>
#include <sys/stat.h>		/* 001 */
#include <wchar.h>
#include <alpha/machlimits.h>
#include "strings_msg.h"	/* 001 */

#define MSGSTR(n,s) catgets(catd, MS_STRINGS, n, s)	/* 001 */

nl_catd catd;			/* 001 */

long	ftell();

#ifdef	vax
struct	exec header;
#endif
#if defined(mips) || defined(__alpha)
FILHDR fheader;
AOUTHDR	aheader;
SCNHDR sheader;
#endif

static	void	usage(void);

char	*infile = "Standard input";
int	asdata;
long	offset;
int	minlength = 4;
char	*format=NULL;

main(int argc, char *argv[])
{
	struct stat statb;			/* 001 */
	register int i;
	int altlen = 0;
	int flag;

        setlocale( LC_ALL, "" );		/* 001 */
        catd = catopen(MF_STRINGS, NL_CAT_LOCALE);

        /*
         * Handle special meaning of "-"
         */
        for (i=1; i<argc; i++)
          if (!strcmp(argv[i],"-"))
            argv[i] = "-a";


        while ((flag = getopt(argc, argv, ":oat:n:0123456789")) != -1) {
            switch (flag) {
                /*
                 * Option to preceed each buffer print by its offset in the
                 * file; in octal.
                 */
              case 'o':
                format = "%7o ";
                break;

                /*
                 * Option to search the entire file, not just the data
                 * section for printable characters.
                 */
              case 'a':
                asdata++;
                break;

              case '0': case '1': case '2': case '3': case '4':
              case '5': case '6': case '7': case '8': case '9':
                altlen = altlen*10 + (flag - '0');
                break;

              case 't':
                switch(*optarg) {
                case 'd':
                        format = "%7d ";
                        break;
                case 'o':
                        format = "%7o ";
                        break;
                case 'x':
                        format = "%7x ";
                        break;
                default:
                        fprintf(stderr, MSGSTR(BADFORMAT,
                          "strings: Bad format, must be one of d, x, or o.\n"));
                        exit(1);
                        break;
                }
                if (*(optarg+1) != '\0')
                        usage();
                break;

              case 'n':
                altlen = atoi(optarg);
                if (!(altlen > 0)) {
                        fprintf(stderr, MSGSTR(BADNUMBER,
                         "strings: Bad number, must be a positive integer.\n"));
                        exit(1);
                }
                break;

              case '?':
              case ':':
              default:
                usage();
            }
        }

	if (altlen) minlength = altlen;
	argv += optind;
	argc -= optind;

	do {
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			infile = argv[0];
			argc--, argv++;
		}
		fseek(stdin, (long) 0, 0);
#ifdef vax
		if (asdata ||
		    fread((char *)&header, sizeof header, 1, stdin) != 1 || 
		    N_BADMAG(header)) {
			fseek(stdin, (long) 0, 0);
			find((long) 100000000L);
			continue;
		}
		fseek(stdin, (long) N_TXTOFF(header)+header.a_text, 0);
		find((long) header.a_data);
#endif
#if defined(mips) || defined(__alpha)

		/* 001
		 * Call stat() to determine if file is regular so that
		 * can seek into it. Otherwise, will just scan from the start.
		 */
		fstat(fileno(stdin), &statb);

		if (asdata ||
		    S_ISREG(statb.st_mode) == 0 ||		/* 001 */
		    fread((char *)&fheader, sizeof fheader, 1, stdin) != 1 || 
		    fread((char *)&aheader, sizeof aheader, 1, stdin) != 1 || 
		    N_BADMAG(aheader) ||
		    read_scnhdrs(fheader.f_nscns, statb.st_size)) { /* 001 */
			fseek(stdin, (long) 0, 0);
			find((long) 100000000L);
			continue;
		}

		search_data_sections();			/* 001 */
#endif
	} while (argc > 0);
}


find(cnt)
	long cnt;
{
	wchar_t c;
	static wchar_t buf[BUFSIZ];
	register wchar_t *cp;
	register int cc;
	static char ch[MB_LEN_MAX];
	register int mb;
	static int mb_max;

	mb_max=MB_CUR_MAX;
	cp = buf, cc = 0;
	for (; cnt > 0; ) {
                /*
                 * Get the character and check to see if it is valid.
                 */

		mb=0;
		if ((ch[mb++]=getc(stdin)) != EOF)
		    while (mbtowc(&c, ch, mb) == -1) {
		        if (mb==mb_max) {
			    while (mb>1) ungetc(ch[--mb]&0xff, stdin);
			    c=(wchar_t)(ch[mb-1] & 0xff);
			    break;
		        }
		        if ((ch[mb++]=getc(stdin)) == EOF) {
			    c='\0';
			    break;
			}
		    }
		else c='\0';
		cnt -= mb;
		if (c == '\n' ||			/* string termination */
		    c == '\0' ||
		    (!iswprint(c) && (c != '\f')) ||	/* dirt? */
		    cnt <= 0) {
#if 0		/* in silver, seems not necessary */
			if (cp > buf && cp[-1] == '\n')
				--cp;
#endif
                /*
                 * If the character read is "dirt" or a terminating character,
                 * then, if buf contains more than minlength characters
                 * print the buffer with newline termination, and reset the
                 * buffer.
                 */
			*cp++ = 0;
			if (cp > &buf[minlength]) {
				if (format)
					printf(format, ftell(stdin) - cc - 1);
				fputws(buf, stdout);
				fputwc(L'\n', stdout);
			}
			cp = buf, cc = 0;
		} else {
                        /*
                         * As long as the current line being read is less
                         * than BUFSIZ - 2 (to allow space for terminating
                         * null when the buffer becomes full) then add the
                         * character to the buffer.
                         *
                         * If the last valid character of a file falls on a
                         * buffer boundary, then the next character will be
                         * EOF, and the above "dirt" condition will occur.
                         */
			if (cp < &buf[sizeof buf - 2]) {
				*cp++ = c;
				cc++;
                        } else {
                                /*
                                 * If the line read is too long, then
                                 * print it out as if a terminating
                                 * character were found.
                                 */
                                *cp++ = c;
                                *cp++ = 0;
                                if (format)
                                        printf(format, ftell(stdin) - cc - 1);

                                fputws(buf, stdout);

                                cp = buf, cc = 0;
                        }
		}
		if (ferror(stdin) || feof(stdin))
			break;
	}
}


/* 001
 * Create a sorted list for tracking init data sections.
 */
struct s_node
{
	long s_size;
	long s_scnptr;
	struct s_node *next, *prev;
};

extern struct s_node Bot;
struct s_node Top = { 0, 0, &Bot, NULL };
struct s_node Bot = { 0, 0, NULL, &Top };

/* 001
 * Add data section to sorted list.
 */
add_list(s_scnptr, s_size)
long s_scnptr, s_size;
{
	struct s_node *npnew, *np;

	if ((npnew = (struct s_node *)malloc(sizeof(struct s_node))) == NULL)
		return;
	npnew->s_scnptr = s_scnptr;
	npnew->s_size = s_size;

	for (np = Top.next; np != &Bot; np = np->next) {
		if (npnew->s_scnptr < np->s_scnptr)
			break;
	}

	np->prev->next = npnew;
	npnew->next = np;
	npnew->prev = np->prev;
	np->prev = npnew;
}


/* 001
 * Set up a new list; freeing all nodes if necessary.
 */
new_list()
{
	struct s_node *np, *next;

	for (np = Top.next; np != &Bot; np = next) {
		next = np->next;
		free(np);
	}
	Top.next = &Bot;
	Bot.prev = &Top;
}

/* 001
 * OSF/1 Alpha object files use COFF file format where
 * initialized data may be found in:
 *		.data, .rdata, .sdata, .lit4 and .lit8.
 *
 * Max limit on sections has not been specified, so picked a
 * limit larger than the number of different sections currently defined.
 *
 * NOTE: if the object file format changes then the following constants
 * 	and/or code should be updated.
 */
#define INIT_DATA	(STYP_DATA|STYP_RDATA|STYP_SDATA|STYP_LIT4|STYP_LIT8)
#define MAX_SECTIONS	(50)

/* 001
 * Read all section headers (COFF format) looking for init data.
 * Returns 1 on format error; 0 on success.
*/
read_scnhdrs(f_nscns, st_size)
unsigned short f_nscns;
off_t st_size;
{
	int	scn_num;

	if (f_nscns > MAX_SECTIONS) {
		bad_object();
		return (1);
	}

	new_list();
	for (scn_num = 1; scn_num <= f_nscns; scn_num++) {
		if (fread((char *)&sheader, sizeof sheader, 1, stdin) != 1) {
			bad_object();
			return (1);
		}

		if ((sheader.s_flags & INIT_DATA) && (sheader.s_size > 0)) {
			if (sheader.s_scnptr < st_size) {
				add_list(sheader.s_scnptr, sheader.s_size);
			}
			else {
				bad_object();
				return (1);
			}
		}
	}

	return (0);
}


/* 001
 * Search each data section for strings.
 */
search_data_sections()
{
	struct s_node *np;

	for (np = Top.next; np != &Bot; np = np->next) {
		if (fseek(stdin, np->s_scnptr, 0)) {
			bad_object();
			break;
		}
		find((long) np->s_size);
	}
}


bad_object()
{
	fprintf(stderr, MSGSTR(NOT_MACHO,
		"strings: file %s is a damaged object file\n"), infile);
}


static void usage(void)
{
	fprintf(stderr, MSGSTR(USAGE,
	"Usage: strings [ -ao ] [ -t Format ] [ -n Number ] [ File ... ]\n"));
	exit(1);
}
