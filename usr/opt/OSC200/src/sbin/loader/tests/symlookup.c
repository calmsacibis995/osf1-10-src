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
static char	*sccsid = "@(#)$RCSfile: symlookup.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:46:04 $";
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

/* currently, only supports one symbols section and one strings section */

/*
 * The symlookup program is a simple program that looks up symbols in
 * an OSF/Mach-O object file.  You can give it either a symbol name
 * (e.g main or _main), or a numeric value (e.g. 0x40048, 0468 or 1024
 * for hexadecimal, octal or decimal).  When given a symbol name, the
 * symlookup programs prints the value of the symbol.  When given a
 * numeric value, the symlookup program prints the name of the symbol
 * that is closest to the numeric, plus the offset from the value of
 * that symbol.
 */

#include <sys/types.h>

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include <mach_o_header.h>
#include <mach_o_format.h>

char *program_name;
int   file_descriptor = -1;

char          *strings;
symbol_info_t *symbols;
int            symbols_count;

void  init(char *);
void  loop(void);
void  lookup(char *);

void  sym2num(char *);
void  num2sym(int);
int   get_number(char *, int *);

void  file_open(char *);
void  file_read(int, char *, int);
void  file_close(void);
void *mem_malloc(int);

char *program_name;

main(argc, argv)
	char *argv[];
{
	program_name = argv[0];

	if (argc < 2) {
		(void)fprintf(stderr, "usage: %s <object-file-name>\n",
			program_name);
		exit(1);
	}

	init(argv[1]);
	loop();
	exit(0);
}

void
loop()
{
	char buffer[1024];
	int  number;

	printf("> ");
	(void)fflush(stdout);
	while (gets(buffer) != NULL) {

		if (get_number(buffer, &number)) {
			/* number failed so must be a symbol */
			sym2num(buffer);
		} else {
			/* must be a number */
			num2sym(number);
		}

		printf("> ");
		(void)fflush(stdout);
	}
	(void)putchar('\n');
}

void
sym2num(name)
	char *name;
{
	symbol_info_t *sip;
	int            i;


	sip = symbols;
	for (i = 0; i < symbols_count; i++) {
		if (!strcmp(name, (strings + sip->si_symbol_name))) {
			if ((sip->si_flags & SI_ABSOLUTE_VALUE_F)
			    || (sip->si_flags & SI_LITERAL_F)) {
				printf("%#x\n", sip->si_abs_val);
			} else {
				/* def val */
				printf("%d.%#x\n", sip->si_def_val.adr_lcid,
					sip->si_def_val.adr_sctoff);
			}
			return;
		}
		sip++;
	}
	if (!strcmp(name, "quit"))
		exit(0);
	printf("not found\n");
}



void
num2sym(number)
{
	int            i, delta, new_delta;
	symbol_info_t *sip;
        char          *name;
	int            found = 0;

	sip = symbols;
	for (i = 0; i < symbols_count; i++, sip++) {
		if (!((sip->si_flags & SI_ABSOLUTE_VALUE_F)
		     || (sip->si_flags & SI_LITERAL_F)))
			continue;
		if (number < (int)sip->si_abs_val)
			continue;
		if (found) {
			new_delta = number - (int)sip->si_abs_val;
			if (new_delta < delta) {
				delta = new_delta;
				name = strings + sip->si_symbol_name;
			}
		} else {
			delta = number - (int)sip->si_abs_val;
			name = strings + sip->si_symbol_name;
			found = 1;
		}
	}

	if (found) {
		if (delta)
			printf("%s+%#x\n", name, delta);
		else
			printf("%s\n", name);
	} else
		printf("not found\n");
}

void
init(file)
	char *file;
{
	char                    buffer[MO_SIZEOF_RAW_HDR];
	char                   *cmds, *cp;
	load_cmd_map_command_t *map;
	mo_header_t             moh;
	int                     error, i;
	symbols_command_t      *syp;
	strings_command_t      *stp;
	ldc_header_t           *hp;

	file_open(file);

	file_read(0, buffer, MO_SIZEOF_RAW_HDR);

	if (error = decode_mach_o_hdr((void *)buffer, MO_SIZEOF_RAW_HDR,
	    MOH_HEADER_VERSION, &moh)) {
		(void)fprintf(stderr, "%s: decode_mach_o_hdr() failed with error code %d\n",
			program_name);
		exit(1);
	}

	if (moh.moh_magic != MOH_MAGIC) {
		(void)fprintf(stderr, "%s: bad magic number\n", program_name);
		exit(1);
	}

	cmds = (char *)mem_malloc(moh.moh_sizeofcmds);
	file_read(moh.moh_first_cmd_off, cmds, moh.moh_sizeofcmds);

#define	XLATE(offset) \
	(((char *)cmds) + (((int)(offset)) - ((int)moh.moh_first_cmd_off)))

	map = (load_cmd_map_command_t *)XLATE(moh.moh_load_map_cmd_off);

	for (i = 0; i < map->lcm_nentries; i++) {
		hp = (ldc_header_t *)XLATE(map->lcm_map[i]);
		if (hp->ldci_cmd_type == LDC_SYMBOLS) {
			if (symbols)
				continue;
			syp = (symbols_command_t *)hp;
			if (syp->symc_kind != SYMC_DEFINED_SYMBOLS)
				continue;
			symbols = (symbol_info_t *)mem_malloc(syp->ldc_section_len);
			file_read(syp->ldc_section_off, (char *)symbols,
				syp->ldc_section_len);
			symbols_count = syp->symc_nentries;
		} else if (hp->ldci_cmd_type == LDC_STRINGS) {
			if (strings)
				continue;
			stp = (strings_command_t *)hp;
			strings = (char *)mem_malloc(stp->ldc_section_len);
			file_read(stp->ldc_section_off, strings,
				stp->ldc_section_len);
		}
	}

	if (!symbols) {
		(void)fprintf(stderr, "%s: no symbols\n", program_name);
		exit(1);
	}

	if (!strings) {
		(void)fprintf(stderr, "%s: no strings\n", program_name);
		exit(1);
	}

	file_close();
}

void
file_open(file)
	char *file;
{
	int fd;

	if ((fd = open(file, O_RDONLY)) == -1) {
		(void)fprintf(stderr, "%s: open() failed: %s\n", program_name,
			strerror(errno));
		exit(1);
	}

	file_descriptor = fd;
}

void
file_read(offset, buffer, size)
	char *buffer;
{
	int count;

	if (lseek(file_descriptor, offset, SEEK_SET) == -1) {
		(void)fprintf(stderr, "%s: lseek() failed: %s\n", program_name,
			strerror(errno));
		exit(1);
	}
	if ((count = read(file_descriptor, buffer, size)) == -1) {
		(void)fprintf(stderr, "%s: read() failed: %s\n", program_name,
			strerror(errno));
		exit(1);
	}
	if (count != size) {
		(void)fprintf(stderr, "%s: short read\n", program_name);
		exit(1);
	}
}

void
file_close()
{
	(void)close(file_descriptor);
}

void *
mem_malloc(size)
{
	void *p;

	if ((p = malloc((size_t)size)) == NULL) {
		(void)fprintf(stderr, "%s: malloc() failed\n", program_name);
		exit(1);
	}
	return(p);
}

int
get_number(cp, ip)
	char *cp;
	int *ip;
{
	int c, i;

	i = 0;
	if ((cp[0] == '0') && ((cp[1] == 'x') || (cp[1] == 'X')))
		for (cp = &cp[2]; c = *cp; cp++)
			if (('0' <= c) && (c <= '9'))
				i  = (i*16) + c - '0';
			else if (('a' <= c) && (c <= 'f'))
				i = (i*16) + c - 'a' + 10;
			else if (('A' <= c) && (c <= 'F'))
				i = (i*16) + c - 'A' + 10;
			else
				return(-1);
	else if (cp[0] == '0')
		for (cp = &cp[1]; c = *cp; cp++)
			if (('0' <= c) && (c <= '7'))
				i  = (i*8) + c - '0';
			else
				return(-1);
	else
		for (; c = *cp; cp++)
			if (('0' <= c) && (c <= '9'))
				i  = (i*10) + c - '0';
			else
				return(-1);
	*ip = i;
	return(0);
}
