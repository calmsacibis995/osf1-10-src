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
static char *rcsid = "@(#)$RCSfile: PrintReq_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:33:46 $";
#endif
printreq_code( adr )
     Net_Entry **adr;
{
  extern void PrintReq();
  SfbRegType t_sfbReg;
  CmdType t_cmd;

  t_sfbReg.errorVals.e = (*(adr+65))->value;
  t_sfbReg.errorVals.e1 = (*(adr+64))->value;
  t_sfbReg.errorVals.e2 = (*(adr+63))->value;
  t_sfbReg.addrRegs.zBase = (*(adr+62))->value;
  t_sfbReg.addrRegs.za1 = (*(adr+61))->value;
  t_sfbReg.addrRegs.za2 = (*(adr+60))->value;
  t_sfbReg.addrRegs.a1 = (*(adr+59))->value;
  t_sfbReg.addrRegs.a2 = (*(adr+58))->value;
  t_sfbReg.colorRegs.zRegs.val.hi = (*(adr+57))->value;
  t_sfbReg.colorRegs.zRegs.val.lo = (*(adr+56))->value;
  t_sfbReg.colorRegs.zRegs.inc.hi = (*(adr+55))->value;
  t_sfbReg.colorRegs.zRegs.inc.lo = (*(adr+54))->value;
  t_sfbReg.colorRegs._colorVals.red = (*(adr+53))->value;
  t_sfbReg.colorRegs._colorVals.green = (*(adr+52))->value;
  t_sfbReg.colorRegs._colorVals.blue = (*(adr+51))->value;
  t_sfbReg.colorRegs.colorIncs.red = (*(adr+50))->value;
  t_sfbReg.colorRegs.colorIncs.green = (*(adr+49))->value;
  t_sfbReg.colorRegs.colorIncs.blue = (*(adr+48))->value;
  t_sfbReg.colorRegs.dither._row = (*(adr+47))->value;
  t_sfbReg.colorRegs.dither._col = (*(adr+46))->value;
  t_sfbReg.colorRegs.dither.dxGEdy = (*(adr+45))->value;
  t_sfbReg.colorRegs.dither.dxGE0 = (*(adr+44))->value;
  t_sfbReg.colorRegs.dither.dyGE0 = (*(adr+43))->value;
  t_sfbReg.tcMask = (*(adr+42))->value;
  t_sfbReg.zTest = (*(adr+41))->value;
  t_sfbReg.rop = (*(adr+40))->value;
  t_sfbReg.pixelShift = (*(adr+39))->value;
  t_sfbReg.pixelMask = (*(adr+38))->value;
  t_sfbReg.depth = (*(adr+37))->value;
  t_sfbReg.mode.z16 = (*(adr+36))->value;
  t_sfbReg.mode.rotate.src = (*(adr+35))->value;
  t_sfbReg.mode.rotate.dst = (*(adr+34))->value;
  t_sfbReg.mode.visual.src = (*(adr+33))->value;
  t_sfbReg.mode.visual.dst = (*(adr+32))->value;
  t_sfbReg.mode.mode = (*(adr+31))->value;
  t_sfbReg.mode.simple = (*(adr+30))->value;
  t_sfbReg.mode.stipple = (*(adr+29))->value;
  t_sfbReg.mode.line = (*(adr+28))->value;
  t_sfbReg.mode.copy = (*(adr+27))->value;
  t_sfbReg.mode.dmaRd = (*(adr+26))->value;
  t_sfbReg.mode.dmaWr = (*(adr+25))->value;
  t_sfbReg.mode.z = (*(adr+24))->value;
  t_sfbReg.stencil.sWrMask = (*(adr+23))->value;
  t_sfbReg.stencil.sRdMask = (*(adr+22))->value;
  t_sfbReg.stencil.sTest = (*(adr+21))->value;
  t_sfbReg.stencil.sFail = (*(adr+20))->value;
  t_sfbReg.stencil.zFail = (*(adr+19))->value;
  t_sfbReg.stencil.szPass = (*(adr+18))->value;
  t_sfbReg.stencil.zOp = (*(adr+17))->value;
  t_sfbReg.lineLength = (*(adr+16))->value;
  t_sfbReg._fg = (*(adr+15))->value;
  t_sfbReg._bg = (*(adr+14))->value;
  t_sfbReg.stencilRef = (*(adr+13))->value;
  t_sfbReg.blkStyle = (*(adr+12))->value;
  t_sfbReg.romWrite = (*(adr+11))->value;
  t_sfbReg.halfColumn = (*(adr+10))->value;
  t_sfbReg.dacSetup = (*(adr+9))->value;
  t_sfbReg.slowDac = (*(adr+8))->value;
  t_cmd.readFlag0 = (*(adr+7))->value;
  t_cmd.newAddr = (*(adr+6))->value;
  t_cmd.newError = (*(adr+5))->value;
  t_cmd.copy64 = (*(adr+4))->value;
  t_cmd.color = (*(adr+3))->value;
  t_cmd.planeMask = (*(adr+2))->value;

  PrintReq( (*(adr+67))->value, (*(adr+66))->value, t_sfbReg, t_cmd, (*(adr+1))->value, (*(adr+0))->value );
}
