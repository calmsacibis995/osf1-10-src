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
static char *rcsid = "@(#)$RCSfile: evd_text.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:31:31 $";
#endif
/*
 **  Copyright (c) Digital Equipment Corporation, 1993
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
 *
 * MODULE DESCRIPTION:
 *
 * Common Agent Event Dispatcher
 *
 * Module EVD_TEXT.C
 *      Contains 'text' functions (translatable text) messages for the Event
 *      Dispatcher for the ULTRIX/OSF Common Agent.
 *
 * WRITTEN BY:
 *    Enterprise Management Frameworks Engineering
 *    D. McKenzie   February 1993   
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines accepts requests over a network,
 *       converts incoming information into standard Common Agent format
 *       and calls the Common Agent for processing of that request.
 *
 *    Purpose:
 *       This module contains the translatable text messages (as functions)
 *       for all diagnostic output of the Common Agent Event Dispatcher (EVD).
 *
 * History
 *      Feb 1993     D. McKenzie      Created for Common Agent Release 1.1
 *
 * NOTES:
 *
 */

/*
File EVD_TEXT.C contains all printable text output for the 'Exxx' messages
used throughout the event dispatcher for diagnostic and debugging use.  If you
modify the text of any message, or add new messages, or delete old messages,
you must:

   (a) Add the new message/modify the existing message/delete the old message
       in/in/from EVD_TEXT.C.  Be sure to follow the format used in
       this module when creating a new text message 'function'.

   (b) Use the 'MSG' macro defined in EVD.H; this macro invokes the
       appropriate emsgXXX() function which you added to this file
       in steb (a) above.  Be sure to add/delete the 'extern' for the 
       new/deleted message 'function' at the end of EVD.H.  Perform
       this step ONLY if you are adding a new message, or if you are
       deleting an existing message.  Modifications to existing messages
       require changes to the appropriate function in this file only.

*/

char *emsg001() { "\nE001 %s: License for Common Agent Base Kit (COM-AGNT-BAS) is not loaded, or\nLicense for Common Agent Developer's Toolkit (COM-AGNT-DEV) is not loaded.\n";}
char *emsg002() { "E002 - EVD Debug Mode enabled: Fork suppressed";}
char *emsg003() { "E003 - evd (V%s.%s) initialization complete";}
char *emsg004() { "E004 - Protocol setup failure '%s', %d, %d";}
char *emsg005() { "E005 - Interface registration failure '%s',%d, %d";}
char *emsg006() { "E006 - Exception TRUE from 'rpc_server_register_if'";}
char *emsg007() { "E007 - Binding vector acqusition failed '%s',%d, %d";}
char *emsg008() { "E008 - Exception TRUE from 'rpc_server_inq_bindings'";}
char *emsg009() { "E009 - Binding vector string conversion failure '%s',%d, %d";}
char *emsg010() { "E010 - Binding vector deallocation failure '%s',%d, %d";}
char *emsg011() { "E011 - Binding string deallocation failure '%s',%d, %d";}
char *emsg012() { "E012 - RPC listen returned w/code %d";}
char *emsg013() { "E013 - Exception TRUE from 'rpc_server_listen'";}
char *emsg014() { "E014 - invalid -d usage\n";}
char *emsg015() { "E015 - Missing debug logfile name\n";}
char *emsg016() { "E016 - invalid -c usage\n";}
char *emsg017() { "E017 - Missing configuration file name\n";}
char *emsg018() { "E018 - invalid command line argument: '%s'\n";}
char *emsg019() { "E019 - invalid debug class: '%s'\n";}
char *emsg020() { "E020 - pthread mutex initialization failed: %s\n";}
char *emsg021() { "E021 - attempt to open file '%s' failed: %s\n";}
char *emsg022() { "E022 - attempt to open system log failed %d\n";}
char *emsg023() { "E023 - open failed on config file: '%s'";}
char *emsg025() { "E025 - mutex lock failed in evd_log(): %m";}
char *emsg026() { "E026 - mutex unlock failed in evd_log(): %m";}
char *emsg027() { "E027 - NULL Event Queue Handle block address given for log";}
char *emsg028() { "E028 - mutex lock failed in log_queue_service(): %m";}
char *emsg029() { "E029 - mutex unlock failed in log_queue_service(): %m";}
char *emsg030() { "E030 - acquisition of Queue Handle List mutex failed, errno = %d";}
char *emsg031() { "E031 - release of Queue Handle List mutex failed, errno = %d";}
char *emsg032() { "E032 - acquisition of Queue Handle List mutex failed, errno = %d";}
char *emsg033() { "E033 - release of Queue Handle List mutex failed, errno = %d";}
char *emsg034() { "E034 - acquisition of Queue Handle List mutex failed, errno = %d";}
char *emsg035() { "E035 - release of Queue Handle List mutex failed, errno = %d";}
char *emsg036() { "E036 - acquisition of Event UID mutex failed, errno = %d";}
char *emsg037() { "E037 - release of Event UID mutex failed, errno = %d";}
char *emsg038() { "E038 - Invalid Queue Access Mode (%d)";}
char *emsg039() { "E039 - No memory for allocating queue handle (%d)";}
char *emsg040() { "E040 - Invalid queue handle supplied (%ld)";}
char *emsg041() { "E041 - Invalid queue handle supplied (%ld)";}
char *emsg042() { "E042 - Invalid object class OID passed in";}
char *emsg043() { "E043 - Invalid generic event_type arg passed in";}
char *emsg044() { "E044 - NULL event_parameters avl passed in";}
char *emsg045() { "E045 - moss_avl_reset on event_parameters failed";}
char *emsg046() { "E046 - Unable to obtain queue handle from protocol engine";}
char *emsg047() { "E047 - Unable to obtain queue handle from protocol engine";}
