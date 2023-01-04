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
static char *rcsid = "@(#)$RCSfile: mtu_msg.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:28:03 $";
#endif
/*  Title:	mtu_msg.c	- MTU Message function file
 **
 **  Copyright (c) Digital Equipment Corporation, 1992
 **  All Rights Reserved.  Unpublished rights reserved
 **  under the copyright laws of the United States.
 **  
 **  The software contained on this media is proprietary
 **  to and embodies the confidential technology of 
 **  Digital Equipment Corporation.  Possession, use,
 **  duplication or dissemination of the software and
 **  media is authorized only pursuant to a valid written
 **  license from Digital Equipment Corporation.
 **
 **  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 **  disclosure by the U.S. Government is subject to
 **  restrictions as set forth in Subparagraph (c)(1)(ii)
 **  of DFARS 252.227-7013, or in FAR 52.227-19, as
 **  applicable.
 **
 */

/*
 **++
 **  FACILITY:  EMF:  Polycenter Common Agent & DECmcc
 **
 **  MODULE DESCRIPTION:
 **
 **     Native Language (NL) message functions for MTU
 **
 **  AUTHORS:
 **
 **      Pete Burgess
 **
 **  CREATION DATE:  28-Sep-1992
 **
 **  MODIFICATION HISTORY:
 **
 **
 ** Edit#     Date	 By			    Description
 ** -----  ------------	----		----------------------------------
 **
 ** 01	  28-Sep-1992   Pete Burgess		    Creation
 */

char *msg0() { return("MTU INFORMATION -- Translating MIB file (%s)\n");}
char *msg1() { return("MTU WARNING -- -m switch is superceded by -a switch\n");}
char *msg2() { return("MTU ERROR -- Invalid entity code value (%d) -\nMust be a number between %d and %d\n");}
char *msg3() { return("MTU WARNING -- -s switch is superceded by -d switch\n");}
char *msg4() { return("MTU WARNING -- -a switch is superceded by -m switch\n");}
char *msg5() { return("MTU WARNING -- -d switch is superceded by -s switch\n");}
char *msg6() { return("\nUsage: [-c code] [-i] [-l] <mibfile> ...\n");}
char *msg6a() { return("\t-c code\t Provide starting EMA entity code\n");}
char *msg6c() { return("\t-i \t Produce MOMgen Info-file\n");}
char *msg6d() { return("\t-l \t Produce listing file\n");}
char *msg7()  { return("MTU ERROR -- Licence for DECMCC TCPIP AM is not loaded,");}
char *msg8() { return("MTU ERROR -- License for Common Agent Base Kit (COM-AGNT-BASE) is not loaded, or\nMTU ERROR -- License for Common Agent Developer's Toolkit (COM-AGNT-DEV) is not loaded.\n");}
char *msg9() { return("MTU ERROR -- Failure opening MIB input file (%s)\n%s\n");}
char *msg10() { return("MTU ERROR -- Failure opening ms file (%s)\n%s\n");}
char *msg11() { return("MTU ERROR -- Failure opening help file (%s)\n%s\n");}
char *msg12() { return("\n! Help file for %s.ms :\n! Produced by Polycenter Common Agent MIB Translation Utility, version %s\n\n");}
char *msg13() { return("MTU ERROR -- Failure opening infofile (%s)\n%s\n");}
char *msg14() { return("MTU ERROR -- Failure opening translation log_filename (%s)\n%s\n");}
char *msg15() { return("\n\tPolyCenter Common Agent MIB Translation Utility %s");}
char *msg16() { return("%s\tPage %2d\n\n");}
char *msg17() { return("RFC Input: %s  Management Specification output: %s\n\n");}
char *msg18() { return("Section 1\tRFC Listing:\n\n");}
char *msg19() { return("MTU SUCCESS -- Success translating MIB (%s)\n\n");}
char *msg20() { return("MTU ERROR -- Failure translating MIB (%s)\n\n");}
char *msg21() { return("MTU ERROR -- Invalid MIB constructed syntax for object (%s)\n");}
char *msg22() { return("MTU ERROR -- Invalid Object (%s) for trap enterprise (%s)\n");}
char *msg23() { return("MTU ERROR -- Internal logic error 10 (%s)\n");}
char *msg24() { return("MTU ERROR -- Undefined typedef (%s) for object (%s) \n");}
char *msg25() { return("MTU INFORMATION -- Entity Model tree is being pruned.\nThe following entities without attributes will be removed:\n");}
char *msg26() { return("Child entity %s\n");}
char *msg27() { return("MTU ERROR -- Table (%s) is empty\n");}
char *msg28() { return("MTU ERROR -- Table (%s) has more than one type of entry\n");}
char *msg29() { return("MTU INFORMATION -- Table (%s) has no indices\n");}
char *msg30() { return("MTU ERROR -- Invalid index object (%s)\n");}
char *msg31() { return("MTU INFORMATION -- Index (%s) is not contained within table (%s)\n");}
char *msg32() { return("MTU ERROR -- Index (%s) is not contained within mib (%s)\n");}
char *msg33() { return("Number of entity levels = %d\n");}
char *msg34() { return("MTU WARNING -- Too many entity levels (%d/%d). Forced pruning of the entity tree will occur.\n");}
char *msg35() { return("Number of entity levels after pruning = %d\n");}
char *msg36() { return("MTU ERROR -- Invalid table structure (%s, %s)\n");}
char *msg41() { return("MTU ERROR -- Duplicate object name (%s)\n");}
char *msg42() { return("MTU ERROR -- Parent of root object (%s) is %s.\n\tParent of root must be from this set:\n");}
char *msg43() { return("MTU ERROR -- Parent (%s) of object (%s) is not defined\n");}
char *msg44() { return("MTU ERROR -- Enterprise(%s) in trap object (%s) is not defined\n");}
char *msg45() { return("MTU ERROR -- Variable (%s) in trap object (%s) is not defined\n");}
char *msg46() { return("MTU ERROR -- Variable (%s) in trap object (%s) is not a leaf object\n");}
char *msg51() { return("RFC Input: %s  Management Specification Output: %s\n");}
char *msg52() { return("Section 2\tManagement Specification Summary:\n\n");}
char *msg60() { return("MTU warning -- %s. (line %d) Failed to process token (%d).\n");}
char *msg61() { return("MTU ERROR -- Aborting translation after %d errors\n");}
char *msg70() { return("Child Entity %s");}
char *msg71() { return("Entity Attribute %s");}
char *msg72() { return("Global Entity %s");}
