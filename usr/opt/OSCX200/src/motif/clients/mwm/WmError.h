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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmError.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 21:36:20 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void WmInitErrorHandler ();
extern int WmXErrorHandler ();
extern int WmXIOErrorHandler ();
extern void WmXtErrorHandler ();
extern void WmXtWarningHandler ();
extern void Warning ();
#ifdef DEC_MOTIF_EXTENSION
extern void WarningStr ();
#endif
#else /* _NO_PROTO */
extern void WmInitErrorHandler (Display *display);
extern int WmXErrorHandler (Display *display, XErrorEvent *errorEvent);
extern int WmXIOErrorHandler (Display *display);
extern void WmXtErrorHandler (char *message);
extern void WmXtWarningHandler (char *message);
extern void Warning (char *message);
#ifdef DEC_MOTIF_EXTENSION
extern void WarningStr (char *message, char *string);
#endif
#endif /* _NO_PROTO */
