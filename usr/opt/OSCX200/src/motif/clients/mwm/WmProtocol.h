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
/*   $RCSfile: WmProtocol.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:03:49 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void SetupWmICCC ();
extern void SendConfigureNotify ();
extern void SendClientOffsetMessage ();
#ifdef DEC_MOTIF_EXTENSION
extern void SendClientFrameMessage ();
#endif
#ifdef DEC_MOTIF_EXTENSION
extern void SendClientFrameMessage ();
#endif
extern void SendClientMsg ();
extern Boolean AddWmTimer ();
extern void DeleteClientWmTimers ();
extern void TimeoutProc ();
#else /* _NO_PROTO */
extern void SetupWmICCC (void);
extern void SendConfigureNotify (ClientData *pCD);
extern void SendClientOffsetMessage (ClientData *pCD);
#ifdef DEC_MOTIF_EXTENSION
extern void SendClientFrameMessage (ClientData *pCD);               
#endif
extern void SendClientMsg (Window window, long type, long data0, Time time, long *pData, int dataLen);
extern Boolean AddWmTimer (unsigned int timerType, unsigned long timerInterval, ClientData *pCD);
extern void DeleteClientWmTimers (ClientData *pCD);
extern void TimeoutProc (caddr_t client_data, XtIntervalId *id);
#endif /* _NO_PROTO */
