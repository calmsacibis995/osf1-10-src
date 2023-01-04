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
next_ctl.sig.q2              = ctl.sig.q2 && ctl.sig.q0
                || ctl.sig.q2 && !ctl.sig.q1
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && readReg && !ctl.sig.wait && !ctl.sig.rq
;
#endif
#ifdef CLOCKED
next_ctl.sig.q1              = ((sfbreg.mode&2)!=0) && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && !done && !lastOne
                || ((sfbreg.mode&1)!=0) && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && !done && !lastOne
                || ctl.sig.q2 && ctl.sig.q0
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x20)!=0) && !((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg && !ctl.sig.wait && !ctl.sig.rq
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && !((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && writeReg
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && writeReg && !ctl.sig.wait && !ctl.sig.rq
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && writeReg
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.q0              = !ctl.sig.q2 && ctl.sig.q1 && ctl.sig.q0
                || !ctl.sig.q1 && !ctl.sig.q0 && writeFB && readFlag
                || !ctl.sig.q1 && !ctl.sig.q0 && writeFB && !ctl.sig.wait && !ctl.sig.rq
                || !((sfbreg.mode&1)!=0) && !ctl.sig.q1 && !ctl.sig.q0 && writeFB
                || !((sfbreg.mode&2)!=0) && !ctl.sig.q1 && !ctl.sig.q0 && writeFB
                || !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && ((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && ((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                || ctl.sig.q2 && !ctl.sig.q1
                || !ctl.sig.q2 && ctl.sig.q0 && ctl.sig.wait
                || ((sfbreg.mode&1)!=0) && !ctl.sig.q1 && ctl.sig.q0 && !done && !lastOne
                || ((sfbreg.mode&2)!=0) && !ctl.sig.q1 && ctl.sig.q0 && !done && !lastOne
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldPixelMask     = ((sfbreg.mode&2)!=0) && ((sfbreg.mode&1)!=0)
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && ((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && ((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg
                || ((sfbreg.mode&1)!=0) && ((sfbreg.mode&4)!=0)
                || !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && done
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ctl.sig.rq
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldMode          = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && ((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.flush           = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && ((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                 && readFlag
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && ((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                 && readFlag
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && writeFB && readFlag
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.setReadFlag     = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && ((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg
                || ((sfbreg.mode&2)!=0) && ((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && done
                 && !readFlag
                || ((sfbreg.mode&2)!=0) && ((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && !readFlag
                 && lastOne
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && readReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.resetReadFlag   = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg
                || ((sfbreg.mode&2)!=0) && ((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && done
                 && readFlag
                || ((sfbreg.mode&2)!=0) && ((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && readFlag
                 && lastOne
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldPixelShift    = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && ((tcAddr&0x10)!=0) && ((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg && !ctl.sig.wait
                 && !ctl.sig.rq
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldBres1         = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldBres2         = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldBresErr       = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && ((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg
                || ((sfbreg.mode&2)!=0) && !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && !done
;
#endif
#ifdef CLOCKED
next_ctl.sig.selAddr         = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && ((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                || ((sfbreg.mode&2)!=0) && !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && ctl.sig.q1 && ctl.sig.q0
                || !ctl.sig.q2 && ctl.sig.q0 && ctl.sig.selAddr
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldFore          = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldBack          = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldPlnmsk        = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && ((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg && !ctl.sig.wait
                 && !ctl.sig.rq
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldBoolop        = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && ((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg && !ctl.sig.wait
                 && !ctl.sig.rq
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldAddrReg       = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && ((tcAddr&0x10)!=0) && ((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.useAddrReg      = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && ((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && ((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q2 && ctl.sig.q0 && ctl.sig.useAddrReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldDeep          = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && ((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg && !ctl.sig.wait
                 && !ctl.sig.rq
;
#endif
#ifdef CLOCKED
next_ctl.sig.readRq          = ((sfbreg.mode&2)!=0) && ((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && !done
                 && readFlag
;
#endif
#ifdef CLOCKED
next_ctl.sig.rq              = !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && lastOne
                || !((sfbreg.mode&2)!=0) && !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait
                || !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && !done
;
#endif
#ifdef CLOCKED
next_ctl.sig.setMask         = !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !writeReg && ctl.sig.rq
                || !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x08)!=0) && ctl.sig.rq
                || !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x10)!=0) && ctl.sig.rq
                || !((sfbreg.mode&2)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((sfbreg.mode&4)!=0) && !writeReg && ctl.sig.rq
                || !((sfbreg.mode&2)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x08)!=0) && !((sfbreg.mode&4)!=0) && ctl.sig.rq
                || !((sfbreg.mode&2)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x10)!=0) && !((sfbreg.mode&4)!=0) && ctl.sig.rq
                || !((sfbreg.mode&2)!=0) && ctl.sig.q2 && !((sfbreg.mode&4)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&1)!=0) && ctl.sig.q2 && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&2)!=0) && ctl.sig.q1 && !((sfbreg.mode&4)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&1)!=0) && ctl.sig.q1 && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&2)!=0) && ctl.sig.q0 && !((sfbreg.mode&4)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&1)!=0) && ctl.sig.q0 && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&2)!=0) && ((tcAddr&0x10)!=0) && !((sfbreg.mode&4)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&1)!=0) && ((tcAddr&0x10)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&2)!=0) && !((tcAddr&0x08)!=0) && !((sfbreg.mode&4)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&1)!=0) && !((tcAddr&0x08)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&2)!=0) && !((sfbreg.mode&4)!=0) && !writeReg && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&1)!=0) && !writeReg && ctl.sig.setMask && ctl.sig.ldPixelMask
                || ((sfbreg.mode&2)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && done
                || !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait && done
                || !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !((sfbreg.mode&4)!=0) && !ctl.sig.wait && done
                || !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x20)!=0) && ((tcAddr&0x04)!=0) && ctl.sig.rq
                || !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && ctl.sig.rq
                || !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x04)!=0) && ctl.sig.rq
                || !((sfbreg.mode&2)!=0) && !((tcAddr&0x40)!=0) && !((tcAddr&0x04)!=0) && !((sfbreg.mode&4)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&2)!=0) && ((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && !((sfbreg.mode&4)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&2)!=0) && !((tcAddr&0x20)!=0) && ((tcAddr&0x04)!=0) && !((sfbreg.mode&4)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&2)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x20)!=0) && ((tcAddr&0x04)!=0) && !((sfbreg.mode&4)!=0) && ctl.sig.rq
                || !((sfbreg.mode&2)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && ((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && !((sfbreg.mode&4)!=0) && ctl.sig.rq
                || !((sfbreg.mode&2)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x04)!=0) && !((sfbreg.mode&4)!=0) && ctl.sig.rq
                || !((sfbreg.mode&1)!=0) && !((tcAddr&0x40)!=0) && !((tcAddr&0x04)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&1)!=0) && ((tcAddr&0x40)!=0) && ((tcAddr&0x20)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
                || !((sfbreg.mode&1)!=0) && !((tcAddr&0x20)!=0) && ((tcAddr&0x04)!=0) && ctl.sig.setMask && ctl.sig.ldPixelMask
;
#endif
#ifdef CLOCKED
next_ctl.sig.next            = ctl.sig.q1 && !ctl.sig.q0
                || !ctl.sig.q2 && !ctl.sig.q1 && ctl.sig.q0 && !ctl.sig.wait
;
#endif
#ifdef COMBINATIONAL
actl.sig.i_busy          = ctl.sig.q1 && ctl.sig.q0
                || !ctl.sig.q1 && !ctl.sig.q0 && writeFB && readFlag
                || !ctl.sig.q1 && !ctl.sig.q0 && writeFB && !ctl.sig.wait && !ctl.sig.rq
                || !((sfbreg.mode&1)!=0) && !ctl.sig.q1 && !ctl.sig.q0 && writeFB
                || !((sfbreg.mode&2)!=0) && !ctl.sig.q1 && !ctl.sig.q0 && writeFB
                || ctl.sig.q0 && ctl.sig.wait
                || ctl.sig.q2 && !ctl.sig.q1
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x20)!=0) && !((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && writeReg
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && writeReg && !ctl.sig.wait && !ctl.sig.rq
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && writeReg
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && readReg && !ctl.sig.wait && !ctl.sig.rq
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x20)!=0) && !((tcAddr&0x08)!=0) && writeReg && !ctl.sig.wait && !ctl.sig.rq
                || ((sfbreg.mode&2)!=0) && ctl.sig.q0 && !done && !lastOne
                || ((sfbreg.mode&1)!=0) && ctl.sig.q0 && !done && !lastOne
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.busy            = ctl.sig.q1 && ctl.sig.q0
                || !ctl.sig.q1 && !ctl.sig.q0 && writeFB && readFlag
                || !ctl.sig.q1 && !ctl.sig.q0 && writeFB && !ctl.sig.wait && !ctl.sig.rq
                || !((sfbreg.mode&1)!=0) && !ctl.sig.q1 && !ctl.sig.q0 && writeFB
                || !((sfbreg.mode&2)!=0) && !ctl.sig.q1 && !ctl.sig.q0 && writeFB
                || ctl.sig.q0 && ctl.sig.wait
                || ctl.sig.q2 && !ctl.sig.q1
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x08)!=0) && !((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && ((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x20)!=0) && !((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0) && writeReg
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && writeReg
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && writeReg && !ctl.sig.wait && !ctl.sig.rq
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && writeReg
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && readReg && !ctl.sig.wait && !ctl.sig.rq
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x20)!=0) && !((tcAddr&0x08)!=0) && writeReg && !ctl.sig.wait && !ctl.sig.rq
                || ((sfbreg.mode&2)!=0) && ctl.sig.q0 && !done && !lastOne
                || ((sfbreg.mode&1)!=0) && ctl.sig.q0 && !done && !lastOne
                || !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.stepBres        = ((sfbreg.mode&2)!=0) && !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && ctl.sig.q0
                || ((sfbreg.mode&2)!=0) && !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && writeFB
                || ((sfbreg.mode&2)!=0) && !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && ((tcAddr&0x10)!=0) && !((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0)
                 && writeReg
                || ((sfbreg.mode&2)!=0) && !((sfbreg.mode&1)!=0) && !ctl.sig.q2 && !ctl.sig.q1 && ((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x10)!=0) && ((tcAddr&0x08)!=0) && ((tcAddr&0x04)!=0)
                 && writeReg
;
#endif
#ifdef CLOCKED
next_sfbctl_shift           = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && ((tcAddr&0x04)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.rdCopyBuff      = !ctl.sig.q2 && ctl.sig.rdCopyBuff
                || !ctl.sig.q1 && ctl.sig.rdCopyBuff
                || ctl.sig.q0 && ctl.sig.rdCopyBuff
                || !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && readReg && !ctl.sig.wait && !ctl.sig.rq
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldCopyBuffHi    = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && ((tcAddr&0x04)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.ldCopyBuffLo    = !ctl.sig.q2 && !ctl.sig.q1 && !ctl.sig.q0 && !((tcAddr&0x40)!=0) && !((tcAddr&0x20)!=0) && !((tcAddr&0x04)!=0) && writeReg
;
#endif
#ifdef CLOCKED
next_ctl.sig.rdRdy           = ctl.sig.q2 && ctl.sig.q1 && ctl.sig.q0
;
#endif
#ifdef CLOCKED
next_ctl.sig.driveTC         = ctl.sig.q2 && ctl.sig.q0
;
#endif
