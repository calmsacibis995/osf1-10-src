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
static char *sccsid = "@(#)$RCSfile: rpc_tblout.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/09 16:02:47 $";
#endif
/*
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Portions of this software have been licensed to
 * Digital Equipment Company, Maynard, MA.
 * Copyright (C) 1988, Sun Microsytsems, Inc.
 */

/*
 * rpc_tblout.c, Dispatch table outputter for the RPC protocol compiler
 */
#include <stdio.h>
#include <strings.h>
#include "rpc_parse.h"
#include "rpc_util.h"

#define TABSIZE		8
#define TABCOUNT	5
#define TABSTOP		(TABSIZE*TABCOUNT)

static char tabstr[TABCOUNT+1] = "\t\t\t\t\t";

static char tbl_hdr[] = "struct rpcgen_table %s_table[] = {\n";
static char tbl_end[] = "};\n";

static char null_entry[] = "\n\t(char *(*)())0,\n\
\txdr_void,\t\t\t0,\n\
\txdr_void,\t\t\t0,\n";

static char tbl_nproc[] = "int %s_nproc =\n\tsizeof(%s_table)/sizeof(%s_table[0]);\n\n";

void
write_tables()
{
	list *l;
	definition *def;

	f_print(fout, "\n");
	for (l = defined; l != NULL; l = l->next) {
		def = (definition *) l->val;
		if (def->def_kind == DEF_PROGRAM) {
			write_table(def);
		}
	}
}

static
write_table(def)
	definition *def;
{
	version_list *vp;
	proc_list *proc;
	int current;
	int expected;
	char progvers[100];
	int warning;

	for (vp = def->def.pr.versions; vp != NULL; vp = vp->next) {
		warning = 0;
		s_print(progvers, "%s_%s",
		    locase(def->def_name), vp->vers_num);
		/* print the table header */
		f_print(fout, tbl_hdr, progvers);

		if (nullproc(vp->procs)) {
			expected = 0;
		} else {
			expected = 1;
			f_print(fout, null_entry);
		}
		for (proc = vp->procs; proc != NULL; proc = proc->next) {
			current = atoi(proc->proc_num);
			if (current != expected++) {
				f_print(fout,
			"\n/*\n * WARNING: table out of order\n */\n");
				if (warning == 0) {
					f_print(stderr,
				    "WARNING %s table is out of order\n",
					    progvers);
					warning = 1;
					nonfatalerrors = 1;
				}
				expected = current + 1;
			}
			f_print(fout, "\n\t(char *(*)())RPCGEN_ACTION(");

			/* routine to invoke */
			pvname(proc->proc_name, vp->vers_num);
			f_print(fout, "),\n");

			/* argument info */
			printit(proc->arg_prefix, proc->arg_type);

			/* result info */
			printit(proc->res_prefix, proc->res_type);
		}

		/* print the table trailer */
		f_print(fout, tbl_end);
		f_print(fout, tbl_nproc, progvers, progvers, progvers);
	}
}

static
printit(prefix, type)
	char *prefix;
	char *type;
{
	int len;
	int tabs;

	len = fprintf(fout, "\txdr_%s,", stringfix(type));
	/* account for leading tab expansion */
	len += TABSIZE - 1;
	/* round up to tabs required */
	tabs = (TABSTOP - len + TABSIZE - 1)/TABSIZE;
	f_print(fout, "%s", &tabstr[TABCOUNT-tabs]);

	if (streq(type, "void")) {
		f_print(fout, "0");
	} else {
		f_print(fout, "sizeof ( ");
		/* XXX: should "follow" be 1 ??? */
		ptype(prefix, type, 0);
		f_print(fout, ")");
	}
	f_print(fout, ",\n");
}
