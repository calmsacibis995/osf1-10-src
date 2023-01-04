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
static char	*sccsid = "@(#)$RCSfile: symbols.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:37 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include "mda.h"
#include <sys/types.h>
#include <filehdr.h>
#include <syms.h>


#define MAXGLOBALS 50000
#define MAXDATA 50000

extern char kernel_file[];

int kernel_fd = -1;		/* File Descriptor for Kernel Image */
int kernel_size;		/* Size of Kernel image file        */
char *kernel_address;		/* Address where kernel is mapped   */


struct sym_entry {
	char *address;
	char *name;
	int	size;
};

static int nglobals;		/* Number of globals in global_syms    */
static int ndata;		/* Number of data items in global_data */
static struct sym_entry global_syms[MAXGLOBALS];
static struct sym_entry global_data[MAXDATA];

valcmp(sym1, sym2)
struct sym_entry *sym1, *sym2;
{
	return(strcmp(sym1->name, sym2->name));
}


symcmp(sym1, sym2)
struct sym_entry *sym1, *sym2;
{
	if ((int)(sym1->address) < (int)(sym2->address))
		return(-1);
	else if (sym1->address == sym2->address)
		return(0);
	else return(1);
}



setup_global_symbols()
{

	int result, slen;
	struct filehdr *file_hdr;
	struct syment *symtab, *sp;
	char *strtab, *symbol_name;
	int nsyms, st;
	int i, j, diff;


	if (kernel_fd == -1) {
		result = map(kernel_file, &kernel_fd, &kernel_size, &kernel_address);
		if (result != SUCCESS) {
			printf("mda: Could not map kernel image file %s\n", kernel_file);
			return(FAILED);
		}
	}
	file_hdr = (struct filehdr *)kernel_address;
	symtab = (struct syment *)file_hdr->f_symptr;
	nsyms = file_hdr->f_nsyms;
	strtab = (char *)symtab + nsyms * SYMESZ;
	printf("%d symbols in symbol table\n", nsyms);
	strtab = (char *)IMAGE(strtab);
	sp=(struct syment *)(IMAGE(symtab));
	ndata = nglobals = 0;
	for (i=0; i<nsyms; i++,sp++) {
		if ((nglobals < MAXGLOBALS) && (ndata < MAXDATA)) {
			if(sp->n_zeroes == 0) {
				st = (int)(sp->n_offset);
				symbol_name = (char *)(strtab+st);
				slen = strlen(symbol_name);
			}
			else {
				symbol_name = &sp->n_name[0];
				slen = strlen(symbol_name);
				slen = (slen > 8) ? 8 : slen;
			}
			if ((sp->n_scnum == 1) && (*symbol_name != '.')) {
				global_syms[nglobals].address = (char *)sp->n_value;
				global_syms[nglobals].name = symbol_name;
				global_syms[nglobals].size = slen;
				nglobals++;
			}

			if (*symbol_name == '_') {
				if (sp->n_sclass == C_AUTO || sp->n_sclass == C_EXT
				    || sp->n_sclass == C_STAT) {
					global_data[ndata].address = (char *)sp->n_value;
					global_data[ndata].name = symbol_name;
					global_data[ndata].size = slen;
					ndata++;
				}
			}

		}
		else { 
			printf("Exceeded maximum sorted symbol table size\n");
			return(SUCCESS);
		};

		i += sp->n_numaux;
		sp += sp->n_numaux;
	}

	printf("%d entries added to sorted symbol table\n", nglobals);
	printf("%d entries added to sorted data table\n", ndata);

	qsort(&global_syms[0], nglobals, sizeof(struct sym_entry), symcmp);
	qsort(&global_data[0], ndata, sizeof(struct sym_entry), valcmp);

	return(SUCCESS);
}  


get_symbol(symbol,size,value,address)
char **symbol;		/* OUT: Symbol Name found     */
int  **size;		/* OUT: Length of symbol name */
char **value;		/* OUT: Value of the symbol   */
int  *address;		/* IN:  Address to lookup     */
{

	int i, diff, result;

	for (i=0; i<nglobals-1; i++) {
		if (address >= global_syms[i].address &&
		    address < global_syms[i+1].address) {
			*symbol = (char *)global_syms[i].name;
			*value = global_syms[i].address;
			*size = (int *)global_syms[i].size;
			return(SUCCESS);
		}
	}
	return(FAILED);
}


#define MINDELTA 0x100

get_data_symbol(symbol,size,value,address)
char **symbol;		/* OUT: Symbol Name found     */
int  **size;		/* OUT: Length of symbol name */
char **value;		/* OUT: Value of the symbol   */
int  *address;		/* IN:  Address to lookup     */
{

	int i, diff, result, mindiff, mini;

	mini = -1;
	mindiff = 0x7fffffff;
	for (i=0; i<ndata-1; i++) {
		if (address == global_data[i].address) {
			*symbol = (char *)global_data[i].name;
			*value = global_data[i].address;
			*size = (int *)global_data[i].size;
			return(SUCCESS);
		}
		diff = (int)((int)address - (int)global_data[i].address);
		if ((diff > 0) && (diff < MINDELTA))
			if (diff < mindiff) {
				mini = i;
				mindiff = diff;
			}
	}
	if (mini == -1)
		return(FAILED);
	else {
		*symbol = (char *)global_data[mini].name;
		*value = global_data[mini].address;
		*size = (int *)global_data[mini].size;
		return(SUCCESS);
	}
}


get_value(symbol,value)
char *symbol;		/* IN: Symbol to look up    */
char **value;		/* OUT: Value of the symbol */
{

	int i, diff, result, len;

	len = strlen(symbol);
	for (i=0; i<ndata-1; i++) {
		result = strncmp(symbol,global_data[i].name,len);
		if (result == 0) {
			*value = global_data[i].address;
			return(SUCCESS);
		}
		else if (result < 0) {
			return(FAILED);
		}
	}
	return(FAILED);
}

