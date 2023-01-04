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
static char *rcsid = "@(#)$RCSfile: mold_msg_text.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:08:15 $";
#endif
/****************************************************************************
 *
 * Copyright (c) Digital Equipment Corporation, 1992.
 * All Rights Reserved.  Unpublished rights reserved
 * under the copyright laws of the United States.
 *
 * The software contained on this media is proprietary
 * to and embodies the confidential technology of
 * Digital Equipment Corporation.  Possession, use,
 * duplication or dissemination of the software and
 * media is authorized only pursuant to a valid written
 * license from Digital Equipment Corporation.
 *
 * RESTRICTED RIGHTS LEGEND   Use, duplication, or
 * disclosure by the U.S. Government is subject to
 * restrictions as set forth in Subparagraph (c)(1)(ii)
 * of DFARS 252.227-7013, or in FAR 52.227-19, as
 * applicable.
 *
 ****************************************************************************
 *
 * Facility:
 *
 *    Management - POLYCENTER (tm) Common Agent
 *
 * Abstract:
 *
 *    This module contains 'text' functions for MOLD to facilitate
 *    Internationalization of message output.
 */

/*
 * File mold_msg_text.c contains all printable text output from MOLD
 * which user will see. If you modify the text of any message, or add new
 * messages, or delete old messages, you must:
 *
 *  (a) Add the new message/modify the existing message/delete the old
 *      message in this file. Be sure to follow the format used in this
 *      module when creating a new text message 'function'.
 *
 *  (b) Add the corresponding extern of the function in mold_msg_text.h
 *      file.
 *
 *  (c) Use the 'MSG' macro defined in mold_msg_text.h to invoke the
 *      correct function.
 *
 */

char *mold_msg001() { "\n%s:  License for Common Agent Base Kit (COM-AGNT-BAS) is not loaded, or \nLicense for Common Agent Developer's Toolkit (COM-AGNT-DEV) is not loaded\n" ; }
char *mold_msg002() { "%s: not super user.\n" ; }
char *mold_msg003() { "\nUsage: [-d dumpfile]\n" ; }
char *mold_msg004() { "\nUsage: [-d dumpfile]\n" ; }
char *mold_msg005() { "\nUsage: [-d dumpfile]\n" ; }
char *mold_msg006() { "\nUsage: [-d dumpfile]\n" ; }
char *mold_msg007() { "%s - Error populating mold from mir...\n" ; }
char *mold_msg008() { "MOLD: main: memory allocation error\n\n" ; }
char *mold_msg009() { "%s - Error dumping the MOLD.\n" ; }
char *mold_msg010() { "%s - Error inquiring about bindings...\n" ; }
char *mold_msg011() { "%s - Error unregistering with ep mapper...\n" ; }
char *mold_msg012() { "%s - Error freeing binding vector...\n" ; }
char *mold_msg013() { "%s - Error unregistering with runtime...\n" ; }
char *mold_msg014() { "MOLD: mold_dump_init_mold: error opening file %s\n\n" ; }
char *mold_msg015() { "error dumping hash table\n\n" ; }
char *mold_msg016() { "error dumping lexigraphic queue\n\n" ; }
char *mold_msg017() { "error dumping containment tree\n\n" ; }
char *mold_msg018() { "%s Received a signal...\n" ; }
char *mold_msg019() { "%s Unregistering locally and exiting...\n" ; }
char *mold_msg020() { "MOLD: remove_object: Fatal error. Object not found in hash table. Exiting.\n" ; }
char *mold_msg021() { "MOLD: out of memory. \n" ; }
char *mold_msg022() { "MOLD: out of memory. \n" ; }
char *mold_msg023() { "MOLD: Attempting deregistration from rpcd\n" ; }
char *mold_msg024() { "MOLD: out of memory. \n" ; }
char *mold_msg025() { "MOLD: Attempting deregistration from rpcd\n" ; }
char *mold_msg026() { "mold (V%s.%s) initialization complete\n" ; }

