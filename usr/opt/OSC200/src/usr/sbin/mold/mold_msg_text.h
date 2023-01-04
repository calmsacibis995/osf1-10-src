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
/*
 * @(#)$RCSfile: mold_msg_text.h,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/11/10 21:23:24 $
 */
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
 *    This module contains extern definition of 'text' functions for MOLD
 *    to facilitate Internationalization of message output.
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

#ifdef NL
#define MSG(msg_name, string) msg_name()
#else
#define MSG(msg_name, string) string
#endif

extern char *mold_msg001() ;
extern char *mold_msg002() ;
extern char *mold_msg003() ;
extern char *mold_msg004() ;
extern char *mold_msg005() ;
extern char *mold_msg006() ;
extern char *mold_msg007() ;
extern char *mold_msg008() ;
extern char *mold_msg009() ;
extern char *mold_msg010() ;
extern char *mold_msg011() ;
extern char *mold_msg012() ;
extern char *mold_msg013() ;
extern char *mold_msg014() ;
extern char *mold_msg015() ;
extern char *mold_msg016() ;
extern char *mold_msg017() ;
extern char *mold_msg018() ;
extern char *mold_msg019() ;
extern char *mold_msg020() ;
extern char *mold_msg021() ;
extern char *mold_msg022() ;
extern char *mold_msg023() ;
extern char *mold_msg024() ;
extern char *mold_msg025() ;
extern char *mold_msg026() ;

