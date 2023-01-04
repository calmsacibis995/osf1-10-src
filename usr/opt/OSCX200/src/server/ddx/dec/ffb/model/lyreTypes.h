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
 * @(#)$RCSfile: lyreTypes.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:40:44 $
 */
/* Hand declarations */
typedef int  signal;
typedef int  Signal;
/* Auto Generated declarations */

typedef int  Pointer;

typedef int  I32;

typedef int  I16;

typedef int  I9;

typedef int  B2;

typedef int  B3;

typedef int  B4;

typedef int  B5;

typedef int  B6;

typedef int  B7;

typedef int  B8;

typedef int  B9;

typedef int  B10;

typedef int  B11;

typedef int  B12;

typedef int  B13;

typedef int  B14;

typedef int  B15;

typedef int  B16;

typedef int  B17;

typedef int  B18;

typedef int  B19;

typedef int  B20;

typedef int  B21;

typedef int  B22;

typedef int  B23;

typedef int  B24;

typedef int  B25;

typedef int  B26;

typedef int  B27;

typedef int  B28;

typedef int  B29;

typedef int  B30;

typedef int  B31;

typedef int  B32;

typedef B32  B64 [2];

typedef B8  B8x8 [8];

typedef B8  B2x8 [2];

typedef B16  B2x16 [2];

typedef B9  B2x9 [2];

typedef B2x8  B4x2x8 [4];

typedef B32  B2x32 [2];

typedef B64  B2x64 [2];

typedef B8  B3x8 [3];

typedef B2  B4x2 [4];

typedef B8  B4x8 [4];

typedef B9  B4x9 [4];

typedef B16  B4x16 [4];

typedef B20  B4x20 [4];

typedef B32  B4x32 [4];

typedef struct {
  B4  hi;
  B32  lo;
} B36;

typedef struct {
  B8  top8;
  B2x32  low64;
} B72;

typedef int /*1..0*/ Bits2;

typedef int /*2..0*/ Bits3;

typedef int /*3..0*/ Bits4;

typedef int /*4..0*/ Bits5;

typedef int /*5..0*/ Bits6;

typedef int /*6..0*/ Bits7;

typedef int /*7..0*/ Bits8;

typedef int /*8..0*/ Bits9;

typedef int /*9..0*/ Bits10;

typedef int /*10..0*/ Bits11;

typedef int /*11..0*/ Bits12;

typedef int /*12..0*/ Bits13;

typedef int /*13..0*/ Bits14;

typedef int /*14..0*/ Bits15;

typedef int /*15..0*/ Bits16;

typedef int /*16..0*/ Bits17;

typedef int /*17..0*/ Bits18;

typedef int /*18..0*/ Bits19;

typedef int /*19..0*/ Bits20;

typedef int /*20..0*/ Bits21;

typedef int /*21..0*/ Bits22;

typedef int /*22..0*/ Bits23;

typedef int /*23..0*/ Bits24;

typedef int /*24..0*/ Bits25;

typedef int /*25..0*/ Bits26;

typedef int /*26..0*/ Bits27;

typedef int /*27..0*/ Bits28;

typedef int /*28..0*/ Bits29;

typedef int /*29..0*/ Bits30;

typedef int /*30..0*/ Bits31;

typedef int /*31..0*/ Bits32;

typedef int /*35..0*/ Bits36;

typedef int /*39..0*/ Bits40;

typedef Bits32  Bits64 [2];

typedef Bits4  Bits8x4 [8];

typedef Bits8  Bits8x8 [8];

typedef Bits2  Bits2x2 [2];

typedef Bits8  Bits2x8 [2];

typedef Bits8  Bits3x8 [3];

typedef Bits2  Bits4x2 [4];

typedef Bits4  Bits4x4 [4];

typedef Bits8  Bits4x8 [4];

typedef Bits4  Bits2x4 [2];

typedef Bits9  Bits2x9 [2];

typedef Bits16  Bits4x16 [4];

typedef Bits32  Bits2x32 [2];

typedef Bits2x4  Bits8x2x4 [8];

typedef Bits32  Bits4x32 [4];

typedef int  BitsTwo [2];

typedef struct {
  Bits2  rasBank;
  Bits9  row;
  Bits9  col;
  Bits2  casBank;
} PhysAddrType;

typedef struct {
  Bits8  s;
  Bits24  z;
} ZBufType;

typedef struct {
  int  zBool;
  B8  sVal;
} SFifoType;

typedef struct {
  B20  red;
  B20  green;
  B20  blue;
} RGB;

typedef struct {
  B5  row;
  B5  col;
  int  dxGEdy;
  int  dxGE0;
  int  dyGE0;
} CmdDitherType;

typedef struct {
  B5  _row;
  B5  _col;
  int  dxGEdy;
  int  dxGE0;
  int  dyGE0;
} ColorDitherType;

typedef struct {
  Bits2  src;
  Bits2  dst;
} RotateType;

typedef struct {
  Bits3  src;
  Bits2  dst;
} VisualType;

typedef struct {
  int  z16;
  RotateType  rotate;
  VisualType  visual;
  Bits7  mode;
  int  simple;
  int  stipple;
  int  line;
  int  copy;
  int  dmaRd;
  int  dmaWr;
  int  z;
} ModeType;

typedef struct {
  B8  sWrMask;
  B8  sRdMask;
  Bits3  sTest;
  Bits3  sFail;
  Bits3  zFail;
  Bits3  szPass;
  int  zOp;
} StencilType;

typedef struct {
  B36  val;
  B36  inc;
} ZType;

typedef struct {
  B17  e;
  B16  e1;
  B16  e2;
} ErrorValsType;

typedef struct {
  B24  zBase;
  B16  za1;
  B16  za2;
  B16  a1;
  B16  a2;
} AddrRegsType;

typedef struct {
  ZType  zRegs;
  RGB  _colorVals;
  RGB  colorIncs;
  ColorDitherType  dither;
} ColorRegsType;

typedef struct {
  ErrorValsType  errorVals;
  AddrRegsType  addrRegs;
  ColorRegsType  colorRegs;
  Bits4  tcMask;
  Bits3  zTest;
  B4  rop;
  Bits4  pixelShift;
  B32  pixelMask;
  int  depth;
  ModeType  mode;
  StencilType  stencil;
  B4  lineLength;
  B32  _fg;
  B32  _bg;
  B8  stencilRef;
  Bits4  blkStyle;
  int  romWrite;
  int  halfColumn;
  Bits5  dacSetup;
  int  slowDac;
} SfbRegType;

typedef struct {
  int  loadLoBuff;
  int  loadHiBuff;
  int  selCpuData;
  int  rdCopyBuff;
  int  flush;
  B32  dataIn;
} CbType;

typedef struct {
  int  _reset;
  int  _sel;
  int  _write;
  int  _ack;
  int  _err;
  int  _rdy;
  int  _rdyIn;
  int  _int;
  int  _intIn;
  int  _conflict;
  int  _conflictIn;
  int  _rReq;
  int  _rReqIn;
  int  _wReq;
  int  _wReqIn;
  int  clkIO;
  int  busOE;
  B32  ad;
  int  adP;
  B32  adIn;
  B32  adOut;
  int  adPIn;
  int  adPOut;
  B4x16  data;
  B4x16  dataIn;
  B2x9  addr;
  B4  _ras;
  B4  _cas;
  B2  _oe;
  B2  dsf;
  B8  _we;
  B4  _rasEn;
  B2  _casEn;
  B2  _addrEn;
  B2  mode;
  int  clkFB;
  Bits8  cursor;
  int  toggle;
  int  _hold;
  int  _hSync;
  int  _vSync;
  int  _blank;
  int  clkVid;
  int  _romCE;
  int  _romOE;
  int  _romWE;
  Bits2  _dacCE;
  int  dacC0;
  int  dacC1;
  int  dacC2;
  int  dacRW;
  int  _icsCE;
  int  _testIn;
  int  testOut;
} SFBioType;

typedef struct {
  int  _testIn;
  int  _reset;
  int  _sel;
  int  _write;
  int  _ack;
  int  _err;
  int  _rdy;
  int  _int;
  int  _conflict;
  int  _rReq;
  int  _wReq;
  int  clkIO;
  Bits32  ad;
  int  adP;
  Bits4x16  data;
  Bits2x9  addr;
  Bits4  _ras;
  Bits4  _cas;
  Bits2  _oe;
  Bits2  dsf;
  Bits8  _we;
  Bits4  _rasEn;
  Bits2  _casEn;
  Bits2  _addrEn;
  Bits2  mode;
  int  clkFB;
  Bits8  cursor;
  int  toggle;
  int  _hold;
  int  _hSync;
  int  _vSync;
  int  _blank;
  int  clkVid;
  int  _romCE;
  int  _romOE;
  int  _romWE;
  Bits2  _dacCE;
  int  dacC0;
  int  dacC1;
  int  dacC2;
  int  dacRW;
  int  _icsCE;
  int  testOut;
} SFBnandType;

typedef struct {
  B4  state;
  B12  count;
  B12  conflicts;
  B32  dmaAddr;
} TCstateType;

typedef struct {
  int  _sel;
  int  _write;
  int  _ack;
  int  _err;
} TCinType;

typedef struct {
  int  _rdy;
  int  _conflict;
  int  _rReq;
  int  _wReq;
} TCoutType;

typedef struct {
  int  _driveLo;
  int  _driveHi;
  int  driveDma;
  int  driveAddr;
  B32  addr;
  int  rdBank;
  int  idle;
  int  last;
  int  strobeData;
  int  grabData;
} TCctlDmaType;

typedef struct {
  int  _reset;
  int  clkIO;
  B4x16  data;
  B4x16  dataIn;
  B2x9  addr;
  B4  _ras;
  B4  _cas;
  B2  _oe;
  B2  dsf;
  B8  _we;
  B4  _rasEn;
  B2  _casEn;
  B2  _addrEn;
  B2  mode;
  int  clkFB;
  Bits8  cursor;
  int  toggle;
  int  _hold;
  int  _hSync;
  int  _vSync;
  int  _blank;
  int  clkVid;
  int  _romCE;
  int  _romOE;
  int  _romWE;
  Bits2  _dacCE;
  int  dacC0;
  int  dacC1;
  int  dacC2;
  int  dacRW;
  int  _icsCE;
  int  _testIn;
  int  testOut;
} TGAioType;

typedef struct {
  int  _reset;
  int  _testIn;
  Bits32  ad;
  Bits4  _cbe;
  int  _frame;
  int  _trdy;
  int  _irdy;
  int  _inta;
  int  _devsel;
  int  _stop;
  int  _gnt;
  int  _req;
  int  clkIO;
  int  par;
  int  idsel;
  Bits4x16  data;
  Bits2x9  addr;
  Bits4  _ras;
  Bits4  _cas;
  Bits2  _oe;
  Bits2  dsf;
  Bits8  _we;
  Bits4  _rasEn;
  Bits2  _casEn;
  Bits2  _addrEn;
  Bits2  mode;
  int  clkFB;
  Bits8  cursor;
  int  toggle;
  int  _hold;
  int  _hSync;
  int  _vSync;
  int  _blank;
  int  clkVid;
  int  _romCE;
  int  _romOE;
  int  _romWE;
  Bits2  _dacCE;
  int  dacC0;
  int  dacC1;
  int  dacC2;
  int  dacRW;
  int  _icsCE;
  int  testOut;
} TGAnandType;

typedef struct {
  int  sn_clka;
  int  sn_clkb;
  int  sn_clkc;
  int  sn_clkd;
} TGASneakClkType;

typedef struct {
  Bits32  ad;
  Bits4  _cbe;
  int  par;
  int  _frame;
  int  _trdy;
  int  _irdy;
  int  _stop;
  int  _devsel;
  int  idsel;
  int  _gnt;
} PCIInputType;

typedef struct {
  Bits32  ad;
  Bits2  _cbe0;
  Bits2  _cbe1;
  int  par;
  int  _frame;
  int  _trdy;
  int  _irdy;
  int  _stop;
  int  _devsel;
  int  _req;
} PCIOutputType;

typedef struct {
  int  ad0;
  int  ad1;
  int  cbe0;
  int  cbe1;
  int  par;
  int  frame;
  int  trdy;
  int  irdy;
  int  stop;
  int  devsel;
} PCIEnableType;

typedef struct {
  Bits32  ad;
  int  _cbe3;
  int  _cbe2;
  int  _cbe1;
  int  _cbe0;
  int  par;
  int  _frame;
  int  _trdy;
  int  _irdy;
  int  _stop;
  int  _devsel;
  int  _req;
} PCIOutToDriveType;

typedef struct {
  Bits32  ad;
  int  cbe3;
  int  cbe2;
  int  cbe1;
  int  cbe0;
  int  par;
  int  frame;
  int  trdy;
  int  irdy;
  int  stop;
  int  devsel;
} PCIEnToDriveType;

typedef struct {
  int  busy;
  int  rdy;
  int  empty;
  int  bank;
} CmdBufStatusType;

typedef struct {
  Bits32  data;
  Bits4  mask;
  int  next;
  int  _empty;
  int  isAddr;
} CmdBufEntryType;

typedef struct {
  B32  data;
  int  first;
  int  busy;
  int  last;
  int  empty;
} DmaBufEntryType;

typedef struct {
  int  rdBank;
  int  wrBank;
  int  last;
  int  full;
  int  empty;
  int  start;
  int  wrMode;
} DmaBufStatusType;

typedef struct {
  int  start;
  int  wrMode;
  int  twelveBit;
  int  inc;
} DmaBufCtlType;

typedef struct {
  B2x32  data;
  Bits2x4  mask;
  Bits2  next;
} BufferDataType;

typedef struct {
  Bits3  state;
  int  rdMode;
  int  wrMode;
  int  twelve;
  int  first;
  int  second;
} CmdReqStateType;

typedef struct {
  int  readFlag0;
  int  newAddr;
  int  newError;
  int  copy64;
  int  color;
  int  planeMask;
} CmdType;

typedef struct {
  B19  blkAddr;
  B5  pixAddr;
} CmdBlkAddrType;

typedef struct {
  Bits23  addr;
  int  ramOp;
  int  indirect;
  int  slope;
  int  fbCmd;
  int  miscCmd;
} CmdAddrType;

typedef struct {
  int  slope;
  int  color;
  int  dxGEdy;
  int  dxGE0;
  int  dyGE0;
} CmdRegCtlType;

typedef struct {
  B4  len;
  B17  e;
  B16  e1;
  B16  e2;
  B16  a1;
  B16  a2;
} CmdBresType;

typedef struct {
  Bits2  rotate;
  Bits2  visual;
  Bits4  rOp;
} CmdRopType;

typedef struct {
  int  cap;
  int  z16;
  int  ntLines;
  Bits2  rotate;
  Bits3  visual;
  Bits7  mode;
} CmdModeType;

typedef struct {
  int  first;
  int  second;
  int  last;
} DmaStatusType;

typedef struct {
  int  req0;
  B24  addrIn;
  B32  dataIn;
  SfbRegType  sfbReg;
  CbType  cb;
  DmaStatusType  dmaStatus;
  int  dmaStall;
  Bits3  cbAddr;
  CmdType  cmd;
  int  reset;
} SfbRequestType;

typedef struct {
  int  idle;
  int  wbEmpty;
  int  i_busy0;
  int  lockReg1;
  int  loadReg1;
  B2x32  copyData;
  int  dataRdy;
  int  firstData;
  B8  dmaMask;
  B2x32  behDmaData;
  B8  behDmaMask;
} SfbStatusType;

typedef struct {
  int  readFlag;
  int  selectZ;
  int  readZ;
  int  planeMask;
  int  color;
  int  block;
  int  fastFill;
  int  packed8bit;
  int  unpacked8bit;
  int  line;
} MemCmdType;

typedef Bits4x32  CursorDataType;

typedef struct {
  Bits2  rasBank;
  Bits18  vramAddr;
  Bits2  casBank;
} AsyncAddrType;

typedef struct {
  int  req;
  AsyncAddrType  addr;
  Bits8  data;
  Bits4  cmd;
  int  selHi;
} AsyncRequestType;

typedef struct {
  B32  asyncCount;
  B4  currCmd;
  B32  currAddr;
  B32  reqTimer;
  B32  currRefrAddr;
  B32  miscTimer;
  B32  extRdCount;
} AsyncTestType;

typedef struct {
  int  valid;
  Bits10  row;
} ShiftType;

typedef struct {
  int  vAck;
  int  curAck;
  int  testLFSR;
} VidAckType;

typedef struct {
  int  curReq;
  Bits6  curAddr;
  int  splitReq;
  int  vReq;
  Bits9  vidAdRow;
  Bits2  vidAdCol;
} VideoRequestType;

typedef struct {
  int  video;
  int  misc;
  Bits2  _curLat;
  B64  data;
} AsyncAckType;

typedef struct {
  int  _romCE;
  int  _romOE;
  int  _romWE;
  int  _dac0CE;
  int  _dac1CE;
  int  dacC0;
  int  dacC1;
  int  dacC2;
  int  dacRW;
  int  _icsCE;
} MiscPinType;

typedef struct {
  int  _romCE;
  int  _romOE;
  int  _romWE;
  int  _dacCE;
  int  _icsCE;
  int  _dac;
  int  _ics;
  int  _rom;
} LatchEnType;

typedef Bits4  MiscCmdType;

typedef struct {
  int  req;
  Bits23  addr;
  int  rom;
  int  io;
  int  dac;
  int  selHi;
} MiscReadType;

typedef struct {
  int  req;
  Bits18  addr;
  Bits8  data;
  int  rom;
  int  ics;
  int  dac;
} MiscWriteType;

typedef struct {
  Bits4  mode;
} MemCmdEncType;

typedef struct {
  B2  bank;
  B9  addr;
  B4x2x8  data;
  Bits4x2  mask;
  int  barrier;
  int  m4;
  B4  cmd;
  int  reRas;
} ToWrBuffType;

typedef struct {
  B2  bank;
  B9  addr;
  B2x8  data;
  Bits2  mask;
  int  barrier;
  int  m4;
  B4  cmd;
  int  reRas;
} WrBuffType;

typedef WrBuffType  WrBuff4xType [4];

typedef struct {
  Bits2  casBank;
  B2  mask;
  B4  cmd;
  B2  rasBank;
} memArbType;

typedef struct {
  int  rasIdle;
  int  row;
  int  ras;
  int  col;
  int  cas0;
  int  cas1;
  int  casIdle;
  int  ropData1;
  int  ropData2;
  int  ropCol;
  int  ropCas;
  int  rasWait;
  int  rasWait2;
  int  casWait;
} mCtrlStateType;

typedef struct {
  mCtrlStateType  mCtrlState;
  int  rop;
} mCtrlIntType;

typedef struct {
  int  rasOut;
  Bits4  rasEn;
  int  casOut;
  Bits2  casEn;
  int  DTOE;
  int  DSF;
  Bits2  WE;
  int  padOE;
  int  adrEn;
} memStateOutputType;

typedef memStateOutputType  memStateOutput4xType [4];

typedef struct {
  Bits4  _ras;
  Bits4  _rasEn;
  Bits4  _cas;
  Bits2  _casEn;
  Bits2  casMode;
  Bits2  _oe;
  Bits2  dsf;
  Bits8  _we;
  B9  addrA;
  B9  addrB;
  Bits2  _addrEn;
  B4x16  dataOut;
  Bits4  dataOE;
} MemPinsType;

typedef struct {
  B24  addr;
  B64  data;
  Bits8  mask;
  int  zWrite;
  int  sWrite;
  Bits3  zTest;
  MemCmdType  cmd;
} MemRequestType;

typedef struct {
  B2  bank;
  B9  addr;
} MuxAddrType;

typedef struct {
  MuxAddrType  row;
  MuxAddrType  col;
} RowColType;

typedef struct {
  RowColType  _addr;
  B4  cmd;
  int  m4;
  int  barrier;
} FromPixelGenType;

typedef struct {
  B64  data;
  B8  mask;
} DestType;

typedef struct {
  DestType  dest;
  int  dataReady;
  int  busy;
  int  idle;
  B9  stencil;
  int  stencilReady;
} MemStatusType;

typedef struct {
  B6  red6;
  B6  green6;
  B7  blue7;
} color19;

typedef struct {
  B8  red8;
  B8  green8;
  B8  blue8;
} color24;

typedef struct {
  B9  red9;
  B9  green9;
  B9  blue9;
} color27;

typedef struct {
  int  hSync;
  int  dma128;
  int  slowDac;
  int  blkOff;
  int  romW;
  int  parity;
  int  sam;
  int  cols;
  Bits4  blkStyle;
  Bits3  mask;
  int  fill;
  int  deep;
} DeepType;

typedef struct {
  Bits3  state;
  int  slopeSet;
  int  barrier;
  int  newAddr;
  int  pixMask;
  int  newError;
  int  cbReadFlag;
  int  chipBusy;
} CmdStateType;

typedef struct {
  int  interrupt;
  int  zBase;
  int  rOp;
  int  mode;
  int  deep;
  int  data;
  int  shiftAddr;
  int  binc;
  int  ginc;
  int  rinc;
  int  foreground;
  int  background;
  int  bresW;
  int  address;
  int  pixMask;
  int  stencil;
  int  zval0;
  int  zval1;
  int  zinc0;
  int  zinc1;
  int  pixShift;
  int  bval;
  int  gval;
  int  rval;
  int  horCtl;
  int  verCtl;
  int  curXY;
  int  curBase;
  int  vidBase;
  int  dacSetup;
  int  vidValid;
  int  bres1;
  int  bres2;
  int  bres3;
  int  bresSlope;
} CmdRegSelType;

typedef struct {
  int  reg;
  int  copy;
  int  _copyLo;
  int  _copyHi;
  int  _fg;
  int  _bg;
  int  _pixMask;
  int  _mode;
  int  _rOp;
  int  _pixShift;
  int  _address;
  int  _bres1;
  int  _bres2;
  int  _bres3;
  int  _bresZ;
  int  _deep;
  int  _rev;
  int  _stencil;
  int  _curBase;
  int  _horCtl;
  int  _verCtl;
  int  _vidValid;
  int  _curXY;
  int  _shiftAddr;
  int  _intStatus;
  int  data;
  int  _zinc0;
  int  _zinc1;
  int  _dmaB;
  int  _bresW;
  int  _zval0;
  int  _zval1;
  int  _rval;
  int  _gval;
  int  _bval;
  int  _dither;
  int  rom;
  int  dac;
  int  io;
  int  status;
} TCselRegType;

typedef struct {
  int  ramSpace;
  int  slope;
  int  slopeN;
  int  copyBuf;
  int  copyBuf0;
  int  copyBufLo;
  int  copy64;
  int  copy64S;
  int  copy64D;
  int  start;
  int  cont;
  int  color;
  int  planeMask;
  int  fbCmd;
  int  regCmd;
  int  miscCmd;
  int  slopeCmd;
  int  indirect;
  CmdRegSelType  reg;
} CmdDecodeType;

typedef struct {
  int  odd;
  Bits7  back;
  Bits7  sync;
  Bits5  front;
  Bits9  active;
} HorType;

typedef struct {
  int  stereoEn;
  Bits6  back;
  Bits6  sync;
  Bits5  front;
  Bits11  active;
} VerType;

typedef struct {
  int  testMode;
  int  _testMode;
  VerType  ver;
} TestVidInType;

typedef struct {
  int  _videoBlank;
  int  _videoSync;
  int  _verSync;
  int  videoHold;
  int  videoToggle;
  Bits8  curData;
} VideoPinType;

typedef struct {
  Bits12  x;
  Bits12  y;
  Bits6  rows;
  Bits6  base;
} CursorType;

typedef struct {
  Bits4  pixShift;
  B32  readPixMask;
  B32  _cmdPixMask;
  DeepType  deep;
  CmdRopType  rOp;
  CmdModeType  mode;
  B4  lineLength;
  ErrorValsType  bresE;
  AddrRegsType  bresA;
  B8  fg;
  B20  rInc;
  B20  gInc;
  B20  bInc;
  B20  rVal;
  B20  gVal;
  B20  bVal;
  ZType  z;
  Bits3  zTest;
  StencilType  stencil;
  B8  stencilRef;
  CmdDitherType  dither;
  Bits32  dmaBase;
  B24  address;
  B32  data;
  B16  bWidth;
  B16  zWidth;
  HorType  hor;
  VerType  ver;
  CursorType  cursor;
  Bits9  vBase;
  Bits3  video;
  Bits10  shiftAddr;
  Bits5  intMask;
  int  hardPixMask;
  Bits5  dacSetup;
  Bits4  testBits;
} CmdRegType;

typedef struct {
  HorType  hor;
  VerType  ver;
  CursorType  cursor;
  int  cursorOn;
  int  testMode;
  int  halfShft;
  int  horSyncSel;
  Bits9  base;
  int  valid;
  int  blank;
} VideoRegType;

typedef struct {
  int  idle;
  int  busy;
  int  data;
  int  backoff;
  int  turn;
} PCITarStType;

typedef struct {
  int  idle;
  int  addr;
  int  data;
  int  s_tar;
  int  turn;
  int  drive;
} PCIMasStType;

typedef struct {
  Bits32  ad;
  Bits4  cbe;
  Bits4  notCbe;
  int  idsel;
} PCIGrabType;

typedef struct {
  int  read;
  int  write;
  int  configRd;
  int  configWr;
} PCIOpType;

typedef struct {
  int  ID;
  int  Statcomm;
  int  RevID;
  int  L_Timer;
  int  Base;
  int  Rom;
  int  Interr;
  int  VGA;
  int  SizeReq;
} ConAddrType;

typedef struct {
  int  Status;
  int  Command;
  int  Interr;
  int  L_Timer;
  int  Base;
  int  Rom2;
  int  Rom3;
  int  RomZero;
  int  VGA0;
  int  VGA3;
  int  SizeReq;
} ConLatchSelType;

typedef struct {
  int  selError;
  int  saveCurrError;
  int  selSavedError;
  int  stepBres;
} CtlBresErrorType;

typedef struct {
  int  selConstant;
  int  sel4;
  int  sel1or4;
  int  selCounter;
  int  init;
  int  next;
  int  lastDmaMask;
} CtlIterationControlType;

typedef struct {
  int  blockMode;
  int  visual32;
  int  unaligned;
} CtlBuildOpMaskType;

typedef struct {
  int  blockMode;
  int  visual32;
  int  unaligned;
} CtlStipMaskType;

typedef struct {
  int  selAddr1;
  int  selAddr0;
  int  saveCurrentVals;
  int  selSavedVals;
  int  stepZ;
  int  notLine;
  int  bresError;
  int  stepBres;
  int  stepIndex;
  int  selDither;
  Bits2  dstVisual;
  Bits7  mode;
} CtlColorType;

typedef struct {
  int  selAddr;
  int  selCurAddr;
  int  saveCurrentVals;
  int  selSavedVals;
  int  selectZ;
  int  stepZ;
  int  stepBres;
  int  errorSign;
  int  lineMode;
  int  visual32;
  int  negateIncVal;
  int  plus8;
  int  plus4;
  int  plus1;
} CtlAddrType;

typedef struct {
  Bits2  selMode;
  Bits2  selAddrMask;
  int  enable;
  int  readZ;
  int  bigPixels;
  int  z16Sel;
} CtlByteMaskType;

typedef struct {
  int  visual32;
  int  lineMode;
  int  transparent;
  int  unaligned;
} CtlMakeStippleType;

typedef struct {
  int  maskLowNibble;
  int  maskHighNibble;
} BlkStyleType;

typedef struct {
  int  flush;
  int  wrMemData;
} CtlCopyLogicType;

typedef struct {
  int  req1;
  CtlBresErrorType  bresError;
  CtlStipMaskType  stipMask;
  CtlIterationControlType  iterationControl;
  int  selData;
  CtlBuildOpMaskType  buildOpMask;
  int  req2;
  CtlAddrType  addr;
  MemCmdType  memCmd;
  CtlColorType  color;
  struct {
    int  notZ;
    int  notData32;
    Bits2  data64;
  } selDataMux;
  CtlByteMaskType  byteMask;
  CtlMakeStippleType  makeStipple;
  int  selDmaRdData;
  Bits2  selEdge;
  int  selOnes;
  int  copy64;
  BlkStyleType  blkStyle;
} CtlAddrGenType;

typedef struct {
  int  req1;
  int  copy64;
  CtlBresErrorType  bresError;
  CtlStipMaskType  stipMask;
  int  req2;
  CtlAddrType  addr;
  MemCmdType  memCmd;
  CtlColorType  color;
  struct {
    int  notZ;
    int  notData32;
    Bits2  data64;
  } selDataMux;
  CtlByteMaskType  byteMask;
  CtlMakeStippleType  makeStipple;
  int  selDmaRdData;
  Bits2  selEdge;
  int  selOnes;
} CtlAddrGen1Type;

typedef struct {
  int  selData;
  CtlBuildOpMaskType  buildOpMask;
} CtlAddrGen2Type;

typedef struct {
  int  selSavedVals;
  int  saveCurrentVals;
  int  selAddr;
  int  stepBres;
  int  stepZ;
  int  selectZ;
  int  readZ;
  int  enableZ;
  int  readFlag;
  int  color;
  int  planeMask;
  int  block;
  int  newAddr;
  int  lastDma;
  int  first;
  int  unaligned;
} CtlPipe2Type;

typedef struct {
  int  pixel;
  int  sl;
  int  hfp;
  int  hsyn;
  int  hbp;
  int  vfp;
  int  vsyn;
  int  vbp;
} MTCHType;

typedef struct {
  int  memLat;
  int  wrDatLat;
  int  write;
  int  padOE;
  int  curCycle;
  int  read;
  int  allCas;
  memStateOutputType  signalsOut;
  int  zLat;
  int  dataRdy;
  int  idleRAS;
  int  cmdReq;
  AsyncAckType  asyncAck;
  int  idle;
  int  incCnt;
  int  rowSel;
  int  sampleAsync;
  int  setUseOld;
  int  clrUseOld;
  int  rop;
} mCtrlDecodeType;
