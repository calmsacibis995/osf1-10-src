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
static char *rcsid = "@(#)$RCSfile: mir_interdmp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:16:27 $";
#endif
/*
 * Copyright © (C) Digital Equipment Corporation 1990, 1991.
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * MODULE DESCRIPTION:
 *
 * Common Agent Management Information Repository
 *
 * Module MIR_INTERDMP.C
 *      Contains all the internationalizable message phrases for the
 *      compiler's symbolic dump.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   October 1992
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *      This module is included into the link for the symbolic dump to provide
 *      for internationalization of messages.
 *
 *    Purpose:
 *       This module contains all the message phrases that are subject
 *       to internationalization, called on a per-phrase basis.
 *
 * History
 *      V1.98   Oct 1992                Created
 */

/*
|==============================================================================
| Message Phrases called from file mir_symdump.c
|==============================================================================
*/
char *mp300() {return("MIR Dump: Invalid reference from index cell %d\n");}
char *mp301() {return("MIR Dump: Attempt to open output file failed. %d\n");}
char *mp302() {return("MIR Dump: Attempt to open input file failed. %d\n");}
char *mp303() {return("MIR Dump: I/O Error on Read.\n");}
char *mp304() {return("MIR Dump: Invalid big/little \'endian\' indicator.\n");}
char *mp305() {return(">>>>>>>>>>>>>>>>>>>>>>>>>> WARNING <<<<<<<<<<<<<<<<<<<<<<<<<<\n");}
char *mp306() {return("        COMPILER BINARY OUTPUT FORCED WITH \'-X\' SWITCH\n");}
char *mp307() {return("               BINARY SHOULD BE CONSIDERED CORRUPTED\n");}
char *mp308() {return("                   BINARY IS NOT LOADABLE\n");}
char *mp309() {return("\n\n End of Dump Reached.\n");}
char *mp310() {return("MIR Dump: Attempt to close output file failed.\n");}
char *mp311() {return("MIR Dump: Attempt to close input file failed.\n");}
char *mp312() {return("MIR Dump: Unable to malloc memory\n");}
char *mp313() {return("Symbolic Dump of MIR Binary Database file \'%s\'\n");}
char *mp314() {return("   Database file produced by MIR Compiler V%d.%d\n");}
char *mp315() {return("     in Binary Output File Format Version 2\n\n");}
char *mp316() {return("This dump produced by Dump Facility for MIR Compiler V%d.%d\n\n");}
char *mp317() {return("Total Number of 4-byte Cells........................ %d\n");}
char *mp318() {return("Longest Object Identifier........................... %d arcs\n");}
char *mp370() {return("Largest General Non-Terminal........................ %d bytes\n");}
char *mp371() {return("Largest String Terminal............................. %d bytes\n\n");}
char *mp319() {return("Index Slice Count................................... %d\n");}
char *mp320() {return("Index Subregister Count............................. %d\n");}
char *mp321() {return("Terminal Signed-Number Count........................ %d\n");}
char *mp322() {return("Terminal Unsigned-Number Count...................... %d\n");}
char *mp323() {return("Terminal String Count............................... %d\n");}
char *mp324() {return("DataConstruct Non-Terminal Count.................... %d\n");}
char *mp325() {return("General Non-Terminal Count.......................... %d\n");}
char *mp326() {return("MIR Relationship Object NT Count.................... %d\n");}
char *mp327() {return("Remaining Preamble (%d bytes) Uninterpreted Dump:\n ");}
char *mp328() {return("Preamble Cell      Contents\n");}
char *mp329() {return("    [%d]:                %#x\n");}
char *mp330() {return("MIR Dump: Malloc failure, size = %d.\n");}
char *mp331() {return("MIR Dump: fread of input file failed. Code: %d (%s)\n");}
char *mp332() {return("\n--------------------------Partition-Table----------------------\n");}
char *mp333() {return("Structure\t\tStart Cell Address\tPercentage\n");}
char *mp334() {return(".........\t\t..................\t..........\n");}
char *mp335() {return("Index Slices\t\t\t%d\t\t%6.2f%%\n");}
char *mp336() {return("Index Subregisters\t\t%d\t\t%6.2f%%\n");}
char *mp337() {return("Terminal SIGNED Numbers\t\t%d\t\t%6.2f%%\n");}
char *mp338() {return("Terminal UNSIGNED Numbers\t%d\t\t%6.2f%%\n");}
char *mp339() {return("Terminal Strings\t\t%d\t\t%6.2f%%\n");}
char *mp340() {return("Non-Terminal DATA-CONSTRUCTS\t%d\t\t%6.2f%%\n");}
char *mp341() {return("Non-Terminal (GENERAL)\t\t%d\t\t%6.2f%%\n");}
char *mp342() {return("Non-Terminal MIR RELATIONSHIPS\t%d\t\t%6.2f%%\n\n");}
char *mp343() {return("Non-Terminal Relationship-Table Entry Count AND-Mask.. %#X\n");}
char *mp344() {return("Non-Terminal OID-Backpointer Count Right-SHIFT Count.. %d\n");}
char *mp345() {return("Non-Terminal OID-Backpointer AND-Mask................. %#X\n");}
char *mp346() {return("Non-Terminal OID-SMI Indicator Right-SHIFT Count...... %d\n");}
char *mp347() {return("Non-Terminal Synonym AND-Mask......................... %#X\n");}
char *mp348() {return("Non-Terminal Target Right-SHIFT Count................. %d\n");}
char *mp349() {return("\nTargets of Relationships (in a Non-Terminal) are displayed as follows:\n");}
char *mp350() {return("      Non-Terminal with a name -> NT(<name>)  (e.g. NT(Node)  )\n");}
char *mp351() {return("     Signed or Unsigned Number -> '<number>'  (e.g. '1'       )\n");}
char *mp352() {return("                        String -> ><string><  (e.g. >Hi There<)\n");}
char *mp353() {return("\fSLICE PARTITION\n");}
char *mp354() {return("\n%d:\tBackptr: %d\n");}
char *mp355() {return("%d:\tEntry Count: %d\n");}
char *mp356() {return("%d:\tArc: %d  Next: [%d]\n");}
char *mp357() {return("\fSUBREGISTER PARTITION\n");}
char *mp358() {return("\n%d:\tBackptr: %d\n");}
char *mp359() {return("%d:\tObject: [%d]\n");}
char *mp360() {return("%d;\tLLevel: [%d]\n");}
char *mp361() {return("\fTERMINAL SIGNED-NUMBERS PARTITION\n");}
char *mp362() {return("%d:\tNUMBER: %d\n");}
char *mp363() {return("\fTERMINAL UNSIGNED-NUMBERS PARTITION\n");}
char *mp364() {return("\fTERMINAL-STRINGS PARTITION\n");}
char *mp365() {return("\n%d:\tLEN: %d\n");}
/*
| NOTE: There is a bug in strmerge which prevents the preferred form of this
|       message from appearing:
|                               . . ."%d:\tVALUE: \"%s\"\n" . . .
|
| Message 352 has the same problem, if either are changed, they must be
| changed together, as 352 describes the output of 366.
*/
char *mp366() {return("%d:\tVALUE: >%s<\n");}
char *mp367() {return("MIR Dump: Out of memory\n");}
char *mp368() {return("\n%d: (Contents %#x)   OID Count...%d   Rel-Entry Count...%d\n");}
char *mp369() {return("%d: (Contents %#x) %s     (");}
/* 372 is the next free */
