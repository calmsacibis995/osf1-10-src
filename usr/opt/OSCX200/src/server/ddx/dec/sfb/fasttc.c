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
#ifdef CLOCKED
next_ctl.sig.tc3             = ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && !ctl.sig.tc0 && sel
                || !ctl.sig.tc3 && ctl.sig.tc2 && ctl.sig.tc1 && !ctl.sig.tc0 && sel && !wr && (!ctl.sig.busy && actl.sig.i_busy)
                || !ctl.sig.tc2 && !ctl.sig.tc1 && !ctl.sig.tc0 && sel && !wr
                || !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && sel && !wr && (!ctl.sig.busy && actl.sig.i_busy)
;
#endif
#ifdef CLOCKED
next_ctl.sig.tc2             = !ctl.sig.tc3 && !ctl.sig.tc2 && ctl.sig.tc1 && !ctl.sig.tc0
                || !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && ctl.sig.tc0 && sel && !(!ctl.sig.busy && actl.sig.i_busy)
                || !ctl.sig.tc3 && ctl.sig.tc1 && !ctl.sig.tc0 && sel && !(!ctl.sig.busy && actl.sig.i_busy)
                || !ctl.sig.tc3 && ctl.sig.tc1 && !ctl.sig.tc0 && sel && wr
;
#endif
#ifdef CLOCKED
next_ctl.sig.tc1             = !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && ctl.sig.tc0 && sel && !(!ctl.sig.busy && actl.sig.i_busy)
                || !ctl.sig.tc3 && ctl.sig.tc2 && ctl.sig.tc1 && !ctl.sig.tc0 && sel && !(!ctl.sig.busy && actl.sig.i_busy)
                || !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && sel && wr
;
#endif
#ifdef CLOCKED
next_ctl.sig.tc0             = !ctl.sig.tc3 && ctl.sig.tc2 && !ctl.sig.tc1 && !ctl.sig.tc0 && !(!ctl.sig.busy && actl.sig.i_busy)
                || !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && ctl.sig.tc0 && !sel && !(!ctl.sig.busy && actl.sig.i_busy)
                || !ctl.sig.tc3 && ctl.sig.tc2 && !ctl.sig.tc0 && !sel && !(!ctl.sig.busy && actl.sig.i_busy)
;
#endif
#ifdef CLOCKED
next_ctl.sig.endat           = !ctl.sig.tc3 && !ctl.sig.tc2 && ctl.sig.tc1 && !ctl.sig.tc0
                || !ctl.sig.tc3 && ctl.sig.tc1 && !ctl.sig.tc0 && sel && wr && (!ctl.sig.busy && actl.sig.i_busy)
;
#endif
#ifdef CLOCKED
next_ctl.sig.enadr           = !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && sel && (!ctl.sig.busy && actl.sig.i_busy)
                || !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && !ctl.sig.tc0 && sel
;
#endif
#ifdef CLOCKED
next_ctl.sig.tcreq           = !ctl.sig.tc3 && !ctl.sig.tc2 && ctl.sig.tc1 && !ctl.sig.tc0
                || !(!ctl.sig.busy && actl.sig.i_busy) && ctl.sig.tcreq
                || !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc0 && sel && !wr
                || !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && sel && !wr && (!ctl.sig.busy && actl.sig.i_busy)
                || !ctl.sig.tc3 && ctl.sig.tc1 && !ctl.sig.tc0 && sel && (!ctl.sig.busy && actl.sig.i_busy)
;
#endif
#ifdef COMBINATIONAL
actl.sig.i_enhld         = !ctl.sig.tc3 && !ctl.sig.tc0 && sel && wr && (!ctl.sig.busy && actl.sig.i_busy)
                || !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc0
                || !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1
;
#endif
#ifdef COMBINATIONAL
actl.sig.i_enadr         = ctl.sig.enadr
                || !ctl.sig.tc3 && ctl.sig.tc2 && ctl.sig.tc1 && !ctl.sig.tc0 && sel && (!ctl.sig.busy && actl.sig.i_busy)
;
#endif
#ifdef CLOCKED
next_ctl.sig.wrRdy           = !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && !ctl.sig.tc0 && sel && wr
                || !ctl.sig.tc3 && ctl.sig.tc2 && ctl.sig.tc1 && !ctl.sig.tc0 && sel && wr && (!ctl.sig.busy && actl.sig.i_busy)
                || !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && sel && wr && (!ctl.sig.busy && actl.sig.i_busy)
;
#endif
#ifdef COMBINATIONAL
actl.sig.i_selBuff       = !ctl.sig.tc3 && !ctl.sig.tc2 && !ctl.sig.tc1 && ctl.sig.tc0
                || !ctl.sig.tc3 && ctl.sig.tc2 && !ctl.sig.tc0
;
#endif
