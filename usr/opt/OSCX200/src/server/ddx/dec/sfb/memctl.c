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
next_ctl.sig.wait            = ctl.sig.rq && !actl.sig.rowMatch && !ctl.sig.mc3 && !ctl.sig.mc1 && !ctl.sig.mc0
                || !ctl.sig.mc2 && ctl.sig.mc0
                || ctl.sig.rq && !ctl.sig.mc3 && !ctl.sig.mc2 && !ctl.sig.mc1
;
#endif
#ifdef CLOCKED
next_memctl_shift           = !ctl.sig.mc3 && ctl.sig.mc2 && ctl.sig.mc1 && !ctl.sig.mc0
;
#endif
#ifdef COMBINATIONAL
i_doRead        = !ctl.sig.mc3 && ctl.sig.mc2 && ctl.sig.mc1 && !ctl.sig.mc0
;
#endif
#ifdef COMBINATIONAL
i_doWrite       = ctl.sig.mc3 && ctl.sig.mc2 && ctl.sig.mc1 && !ctl.sig.mc0
;
#endif
#ifdef CLOCKED
next_ctl.sig.mc3             = ctl.sig.mc3 && ctl.sig.mc1 && ctl.sig.mc0
                || ctl.sig.rq && !ctl.sig.readRq && !ctl.sig.mc3 && !ctl.sig.mc1 && !ctl.sig.mc0
                || ctl.sig.mc3 && !ctl.sig.mc2 && ctl.sig.mc0
;
#endif
#ifdef CLOCKED
next_ctl.sig.mc2             = ctl.sig.mc1 && ctl.sig.mc0
                || actl.sig.rowMatch && !ctl.sig.mc3 && ctl.sig.mc2 && !ctl.sig.mc0
                || !ctl.sig.rq && !ctl.sig.mc3 && ctl.sig.mc2 && !ctl.sig.mc0
                || ctl.sig.mc2 && ctl.sig.mc1
;
#endif
#ifdef CLOCKED
next_ctl.sig.mc1             = ctl.sig.rq && actl.sig.rowMatch && !ctl.sig.mc3 && ctl.sig.mc2 && !ctl.sig.mc1 && !ctl.sig.mc0
                || ctl.sig.mc1 && ctl.sig.mc0
                || !ctl.sig.mc2 && ctl.sig.mc0
;
#endif
#ifdef CLOCKED
next_ctl.sig.mc0             = ctl.sig.rq && !actl.sig.rowMatch && !ctl.sig.mc3 && !ctl.sig.mc1 && !ctl.sig.mc0
                || !ctl.sig.mc2 && ctl.sig.mc0
                || ctl.sig.rq && !ctl.sig.mc3 && !ctl.sig.mc2 && !ctl.sig.mc1
;
#endif
#ifdef CLOCKED
next_ctl.sig.memidle         = ctl.sig.mc2 && ctl.sig.mc1 && !ctl.sig.mc0
                || !ctl.sig.rq && !ctl.sig.mc3 && !ctl.sig.mc1 && !ctl.sig.mc0
;
#endif
