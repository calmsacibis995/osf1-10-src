$
$	*****************************************************************
$	*                                                               *
$	*    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
$	*                                                               *
$	*   All Rights Reserved.  Unpublished rights  reserved  under   *
$	*   the copyright laws of the United States.                    *
$	*                                                               *
$	*   The software contained on this media  is  proprietary  to   *
$	*   and  embodies  the  confidential  technology  of  Digital   *
$	*   Equipment Corporation.  Possession, use,  duplication  or   *
$	*   dissemination of the software and media is authorized only  *
$	*   pursuant to a valid written license from Digital Equipment  *
$	*   Corporation.                                                *
$	*                                                               *
$	*   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
$	*   by the U.S. Government is subject to restrictions  as  set  *
$	*   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
$	*   or  in  FAR 52.227-19, as applicable.                       *
$	*                                                               *
$	*****************************************************************
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
$ ALL RIGHTS RESERVED 
$
$
$ OSF/1 1.2
$

$ HISTORY
$
$ @(#)$RCSfile: patch.msg,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1993/07/29 22:51:24 $

$
$ messages for the patch program
$
$set MS_PATCH
$quote "

$
$ message for inp.c
$
PLAN		"Patching file %s using Plan %s...\n"
CREAT		"(Creating file %s...)\n"
TRYRCS		"Can't find %s--attempting to check it out from RCS.\n"
NOCHECK		"Can't check out %s.\n"
TRYSCCS		"Can't find %s--attempting to get it from SCCS.\n"
NOGET		"Can't get %s.\n"
NOFIND		"Can't find %s.\n"
NOTNORM		"%s is not a normal file--can't patch.\n"
NOOPEN		"Can't open file %s\n"
VERS1	"Warning: this file doesn't appear to be the %s version--patching anyway.\n"
VERS2	"This file doesn't appear to be the %s version--patch anyway? [%s] "
$ the %s in the brakets is replace with the locales no string
$
ABORTED		"Aborted.\n"
VERS3		"Good.  This file appears to be the %s version.\n"
NOMEM		"Can't seem to get enough memory.\n"
TMPWR		"patch: can't write temp file.\n"
NOREOPEN	"Can't reopen file %s\n"
TMPRD		"Error reading tmp file %s.\n"

$
$ messages for patch.c
$
UNSWAP		"Not enough memory to try swapped hunk!  Assuming unswapped.\n"
ALLOCE		"Lost hunk on alloc error!\n"
REVERSE	"Reversed (or previously applied) patch detected!  Assume -R? [%s] "
UNREVERSE "Unreversed (or previously applied) patch detected!  Ignore -R? [%s] "
ANYWAY		"Apply anyway? [%s] "
IGNORE		"Ignoring previously applied (or reversed) patch.\n"
HIGNORE		"Hunk #%d ignored at %ld.\n"
HFAIL		"Hunk #%d failed at %ld.\n"
HGOOD		"Hunk #%d succeeded at %ld"
FUZZ		" with fuzz %ld"
OFFSET1		" (offset %ld line)"
OFFSET2		" (offset %ld lines)"
TRYAGAIN	"\n\nRan out of memory using Plan A--trying again...\n\n"
REJECT		"%d out of %d hunks ignored--saving rejects to %s\n"
REJECT2		"%d out of %d hunks failed--saving rejects to %s\n"
NMORE		"You may not change to a different patch file.\n"
TOOMANY		"Too many file arguments.\n"
CANTCD		"Can't cd to %s.\n"
BADD		"Argument to -D not an identifier.\n"
USAGE	"Usage: patch [-NRSbflsv] [-c|-e|-n] [-B prefix] [-d dir] [-D define]\n\
	     [-F fuzz] [-i patchfile] [-o outfile] [-p num] [-r rejectfile]\n\
	     [file]\n"));
FHUNK		"Fatal internal error in abort_hunk().\n"
BADSYNC		"Out-of-sync patch, lines %ld,%ld--mangled text or line numbers, maybe?\n"
NOCREAT		"patch: can't create %s.\n"
BADORDER	"patch: misordered hunks! output will be garbled.\n"

$
$ messages for pch.c
$
NOTFOUND	"patch file %s not found\n"
GNOMEM		"patch: out of memory\n"
DONE		"done\n"
HUMM		"Hmm..."
$ printed while patch is determining what to do, ie "Thinking..."
TRAIL		"  Ignoring the trailing text.\ndone\n"
NOPATCH		"  I can't seem to find a patch in there anywhere.\n"
TYPE1		"  Looks like a context diff to me...\n"
TYPE2		"  The next patch looks like a context diff to me...\n"
TYPE3		"  Looks like a normal diff to me...\n"
TYPE4		"  The next patch looks like a normal diff to me...\n"
TYPE5		"  Looks like an ed script to me...\n"
TYPE6		"  The next patch looks like an ed script to me...\n"
TYPE7		"  Looks like a new-style context diff to me...\n"
TYPE8	"  The next patch looks like a new-style context diff to me...\n"
INDENT1		"(Patch is indented 1 space.)\n"
INDENT2		"(Patch is indented %d spaces.)\n"
NOFILE		"No file to patch.  Skipping...\n"
ASKFILE		"File to patch: "
SKIPTHIS	"No file found--skip this patch? [%s] "
SKIPPING	"Skipping patch...\n"
LEADT		"The text leading up to this was:\n"
EXPEOF		"Unexpected end of file in patch.\n"
EXPHUNK		"Unexpected end of hunk at line %ld.\n"
EXPSTAR		"Unexpected *** at line %ld: %s"
DUPDASH	"Duplicate \"---\" at line %ld--check line numbers at line %ld.\n"
PREDASH	"Premature \"---\" at line %ld--check line numbers at line %ld.\n"
DUEDASH	"Overdue \"---\" at line %ld--check line numbers at line %ld.\n"
TOLARGE		"Hunk too large (%ld lines) at line %ld: %s"
NODASH		"No --- found in patch at line %ld\n"
NOVEL	"(Fascinating--this is really a new-style context diff but without\n\
the telltale extra asterisks on the *** line that usually indicate\n\
the new style...)\n"
MANGLED	"Replacement text or line numbers mangled in hunk at line %ld\n"
EXPEOF2		"Unexpected end of file in patch at line %ld.\n"
EXPLESS		"< expected at line %ld of patch.\n"
EXPDASH		"--- expected at line %ld of patch.\n"
EXPGTR		"> expected at line %ld of patch.\n"
CANTSWAP	"Not enough memory to swap next hunk!\n"
MALFORMED	"Malformed patch at line %ld: %s"

$
$ messages for util.c
$
NOREOPEN2	"patch: internal error, can't reopen %s\n"
WRFAIL		"patch: write failed\n"
NOBACKUP	"patch: can't backup %s, output is in %s\n"
CANTCR		"patch: can't create %s, output is in %s.\n"
WRFAIL2		"patch: write (%s) failed\n"
SSNOMEM		"patch: out of memory (savestr)\n"



$
$ message for version.c
$
OSFVERSION	"OSF/1 version 1.0 - based on:\n"
VERSION		"%s\nPatch level: %d\n"

