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
static char *rcsid = "@(#)$RCSfile: parse.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:41:26 $";
#endif

# line 2 "gram.dwgs.y"
/*
 * YACC input for the front end parser for the
 * color frame buffer behavioral model
 */

/*
 under RCS

 $Author: Robert_Lembree $
 $Date: 1993/11/19 21:41:26 $, $Revision: 1.1.2.2 $
 $Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/ffb/model/parse.c,v 1.1.2.2 1993/11/19 21:41:26 Robert_Lembree Exp $
 $Locker:  $
 $Log: parse.c,v $
 * Revision 1.1.2.2  1993/11/19  21:41:26  Robert_Lembree
 * 	SFB+ Initial version
 * 	[1993/11/19  16:34:43  Robert_Lembree]
 *
 * Revision 1.1.1.2  1993/11/19  16:34:43  Robert_Lembree
 * 	SFB+ Initial version
 *
 * Revision 2.39  1993/06/10  19:34:37  pcharles
 * extricated a plethora of spurious #ifdef's
 *
 * Revision 2.38  1993/06/10  18:53:23  pcharles
 * added GenerateRandomEvents() option to interactive mode
 * eventMask command to select events
 *
 * Revision 2.37  1993/06/10  18:16:18  rsm
 * added a memsize command
 *
 * Revision 2.36  1993/06/10  15:52:27  pcharles
 * added argument after -batch filename to mask in random events
 *
 * Revision 2.35  1993/06/02  14:41:08  pcharles
 * added ramdac, ics and eeprom writes
 *
 * Revision 2.34  1993/06/02  13:43:59  rsm
 * added persistent pixelmask
 *
 * Revision 2.33  1993/06/01  21:38:51  chris
 * added firstTick and lastTick to -dtrace command
 *
 * Revision 2.32  1993/05/28  17:53:32  chris
 * Added ChipReset function
 *
 * Revision 2.31  1993/05/25  17:27:45  rsm
 * removed diffCompare flag (no longer being used)
 *
 * Revision 2.30  1993/05/20  20:41:48  rsm
 * modified scriptz command to look for script along the search path
 * added a checkInit command with no number which does the same
 * as checkInit foo 0
 *
 * Revision 2.29  1993/05/14  17:45:45  chris
 * call ChipInit instead of WrapperInit when doing init checking
 *
 * Revision 2.28  1993/05/14  13:04:20  chris
 * made rasterVid extern
 *
 * Revision 2.27  1993/05/14  12:51:26  chris
 * added checkInit command
 *
 * Revision 2.26  1993/05/13  19:23:50  rsm
 * added a width command
 *
 * Revision 2.25  1993/05/13  18:59:34  rsm
 * added FILLMEMORY command with planemask
 *
 * Revision 2.24  1993/05/13  15:37:24  siegrist
 * add no vramcompareflag if pct_commands
 *
 * Revision 2.23  1993/05/11  14:06:05  rsm
 * added video command to open up a pipe to "video"
 *
 * Revision 2.22  1993/05/10  19:33:39  chris
 * set cmdFile to yyin when running interactive
 *
 * Revision 2.21  1993/04/29  13:17:47  mullaly
 * moved the extern int declaration for vramCompareFlag
 * to the top of the file
 *
 * Revision 2.20  1993/04/13  14:25:14  pcharles
 * added parameter (planes) to effect 8/32 plane testing via testgen
 *
 * Revision 2.19  1993/04/07  00:22:18  rsm
 * fixed read command so that it doesn't unconditionally add
 * an offset to the address
 *
 * Revision 2.18  1993/03/31  21:54:27  rsm
 * removed do_rams prototype definition (unreferenced, unnecessary)
 *
 * Revision 2.17  1993/03/30  20:50:18  chris
 * replaced VIDEOCOUNT with INTERRUPT register
 *
 * Revision 2.16  1993/03/26  22:01:56  rsm
 * implemented register re-assignment
 *
 * Revision 2.15  1993/03/16  23:39:53  rsm
 * changed framebuffer offset from 0x200000 to 0x800000
 *
 * Revision 2.14  1993/03/15  22:09:14  rsm
 * added batch command for firing off scripts interactively
 * with checking enabled.
 *
 * Revision 2.13  1993/03/10  16:37:41  bret
 * added sim n for multiple ticks
 *
 * Revision 2.12  1993/03/10  15:00:08  rsm
 * deleted wfc stuff
 *
 * Revision 2.11  1993/03/04  21:58:20  rsm
 * fixed writebyte to conditionally add 0x200000 to the address
 *
 * Revision 2.10  1993/03/03  20:57:52  rsm
 * ifdef'd out the stuff that Dave Siegrist just added
 *
 * Revision 2.9  1993/03/03  15:42:13  siegrist
 * added vga and io commands
 *
 * Revision 2.8  1993/03/03  15:07:30  rsm
 * moved storage declaration for cmdFile and dmaBuff to fix linking
 * problem for Jim Peacock.
 *
 * Revision 2.7  1993/03/01  16:53:06  rsm
 * turn interactiveFlag off during scriptz processing
 *
 * Revision 2.6  1993/02/26  14:04:40  rsm
 * added VECTOR command for tracing behavioral vectors
 *
 * Revision 2.5  1993/02/25  18:49:30  chris
 * changed -trace to -dtrace and allow name options
 *
 * Revision 2.4  1993/02/25  18:05:53  chris
 * Added PRESTO command to produce presto pattern files
 *
 * Revision 2.3  1993/02/22  20:31:38  siegrist
 * added dinotrace for pct batch
 *
 * Revision 2.2  1993/02/17  20:12:52  rsm
 * got rid of totalClocks (use ticks instead)
 * moved storage definition of nodeEvals and nodeChanges to debug.c
 *
 * Revision 2.1  1993/02/17  18:08:22  rsm
 * added dinotrace command
 *
 * Revision 2.0  1993/02/15  20:11:59  rsm
 * rev 2.0
 *
 * Revision 1.19  1993/02/03  17:22:47  chris
 * Ifdefed out BusCycle routine with NEW_BUS_INTERFACE
 *
 * Revision 1.18  1993/01/14  16:49:31  rsm
 * added dmaRd and dmaWr commands
 *
 * Revision 1.17  1993/01/12  23:27:35  scott
 * added pct batch specific mode
 *
 * Revision 1.16  1992/12/23  13:46:06  rsm
 * added a "source" command for taking input from a file
 *
 * Revision 1.15  1992/12/22  14:15:35  rsm
 * changes for wicked fast copy implementation
 *
 * Revision 1.14  1992/12/17  17:24:18  rsm
 * added code to exercise the 64 byte copy logic
 *
 * Revision 1.13  1992/12/04  01:49:09  rsm
 * ifdef'd out the stuff the PCI guys added so I could link.
 * added block fill command.
 *
 * Revision 1.12  1992/12/03  21:55:16  scott
 * added PCI configuration reads/writes
 *
 * Revision 1.11  1992/12/03  20:42:33  pcharles
 * Placed z-test signals in new stencil register.
 *
 * Revision 1.10  1992/12/01  16:57:22  rsm
 * moved storage definition for some global variables to simplify
 * building an X-server with our model.
 *
 * Revision 1.9  1992/11/30  23:07:27  rsm
 * added a "ticks" command to print number of ticks
 *
 * Revision 1.8  1992/11/17  22:53:45  rsm
 * added command to execute compressed scripts interactively
 *
 * Revision 1.7  1992/11/17  15:00:16  rsm
 * changes for running logged scripts interactively
 *
 * Revision 1.6  1992/11/16  19:06:49  pcharles
 * Moved dynamic comparison routines external (diffram.c)
 * and added -silent option for use with said routines.
 *
 * Revision 1.5  1992/11/13  02:54:55  rsm
 * changes for generating traces
 *
 * Revision 1.4  1992/11/06  15:55:58  rsm
 * changes for Z-buffer support
 *
 * Revision 1.3  1992/11/04  15:53:39  pcharles
 * Added bob's changes to pass log file names in through -batch
 *
 * Revision 1.2  1992/10/27  15:15:11  pcharles
 * Added -nolog, -difflog, -dynamic command line options for
 * vram comparison, implemented in batch.c and vram.c.
 *
*/

#include <stdio.h>
#include "vram.h"

extern FILE *sfb;
extern FILE *yyin;
extern FILE *yyout;

extern FILE *cmdFile;

extern char *malloc(), *strcpy(), *strcat();
extern char *Copy();

extern unsigned BusRead();
extern unsigned PCIConfigRead();

extern int debugFlag;
extern int interactiveFlag;
extern int historyFlag;
extern int vramCompareFlag;
int displaySize = 64;
int eventMask = 0;

/*********************************************
 * customize with your own declarations here *
 *********************************************/

#include "defs.h"
#include "types.h"
#include "vars.h"

void BusWrite(
	      unsigned addr,
	      unsigned data,
	      unsigned mask
	      );

void PCIConfigWrite(
	      unsigned addr,
	      unsigned data,
	      unsigned mask
	      );

void FillMemory(
		unsigned addr,		/* Byte address		    */
		int     width,		/* width in bytes	    */
		int     height,		/* height in bytes	    */
		int     scanlineWidth,	/* # bytes to next scanline */
		int     pixel,		/* 32-bit pixel value       */
		unsigned planemask	/* 32-bit planemask	    */
		);

void SetLogFlag(
		int n,
		char *s );

void MakeIdle();
void MakeIdle1();

extern int logFlag;	/* don't write a log of commands executed */

extern FILE *vid;
extern FILE *rasterVid;

extern char dmaBuff[];

/*********************************************
 *         end of custom declarations        *
 *********************************************/

# line 273 "gram.dwgs.y"
typedef union  {
	char *sval;
	int ival;
} YYSTYPE;
# define IDENT 257
# define NUMBER 258
# define INIT 259
# define TRACE 260
# define DELETE 261
# define DISPLAY 262
# define DEBUG 263
# define HISTORY 264
# define DEPOSIT 265
# define EXAMINE 266
# define SCALE 267
# define QUEER 268
# define DUMP 269
# define READ 270
# define WRITE 271
# define LINE 272
# define FILL 273
# define COPYAREA 274
# define IDLE 275
# define WRITEBYTE 276
# define LOG 277
# define FILLMEMORY 278
# define ABSREAD 279
# define ABSWRITE 280
# define OLDLINE 281
# define CLINE 282
# define CPOINT 283
# define FLUSH 284
# define VGAWRITE 285
# define IOWRITE 286
# define VGAREAD 287
# define IOREAD 288
# define ABSIOWRITE 289
# define ABSIOREAD 290
# define BUSCYCLE 291
# define SCRIPT 292
# define SCRIPTZ 293
# define CREAD 294
# define CWRITE 295
# define DMARD 296
# define DMAWR 297
# define EVENTMASK 298
# define REGISTER 299
# define TICKS 300
# define BFILL 301
# define SOURCE 302
# define DINO 303
# define PRESTO 304
# define VECTOR 305
# define SIM 306
# define BATCH 307
# define VIDEO 308
# define WIDTH 309
# define CHECKINIT 310
# define RESET 311
# define MEMSIZE 312
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 732 "gram.dwgs.y"


#include "hash.h"
HASHTAB *symTable;

extern int ticks, nodeChanges, nodeEvals;
/*extern int eval_flop, change_flop;*/

                            /* new flags...                                  */
extern int noLog;           /* no files, synchronized popen sfbtgen-cmd/vram */
extern int noHaltOnError;   /* don't halt on static miscomare & output       */
                            /* sfb+ memory writes on stdout.                 */
extern int primitiveCount;  /* # of test primitives for sfbtgen to produce   */
extern int silent;          /* silent sfbtgen/sfb+ memory comparison         */
extern int Seed;            /* sfbtgen random seed                           */
extern int planes;          /* for 8 or 32-plane testing with -nolog         */
extern int PCT_cmds;        /* batch file containst PCT specific commands    */

main(argc, argv)
char *argv[];
{
  char *scriptName;
  int traceOpened = 0;
  int scale;

  historyFlag = 0;

  InitReservedWords();
  WrapperInit();

  Init();

  if (argc > 1 && strcmp(argv[1], "-trace") == 0) {
    /*
     * Read a trace list
     */
    traceOpened = 1;
    --argc, ++argv;

    if ((argc > 1) && (argv[1][0] != '-')) {
      --argc;
      OpenTrace(*(++argv));
    }
    else
      OpenTrace("SigsToTrace");
  }

  if (argc > 1 && strcmp(argv[1], "-dtrace") == 0) {
    extern int firstDinoTick, lastDinoTick;
    /*
     * generate a dinotrace batch output 
     */
    --argc, ++argv;

    if (!traceOpened) OpenTrace("SigsToTrace");
    if ((argc > 1) && (argv[1][0] != '-')) {
      --argc;
      DinoTraceInit(*(++argv));
    }
    else
      DinoTraceInit("DinoOut");

    if ((argc > 1) && (argv[1][0] != '-')) {
      --argc;
      firstDinoTick = atoi(*(++argv));
    }
    if ((argc > 1) && (argv[1][0] != '-')) {
      --argc;
      lastDinoTick  = atoi(*(++argv));
    }
  }

  if (argc > 1 && strcmp(argv[1], "-ptrace") == 0) {
    /*
     * generate a dinotrace batch output 
     */
    --argc, ++argv;

    if (!traceOpened) OpenTrace("SigsToTrace");
    if ((argc > 1) && (argv[1][0] != '-')) {
      --argc;
      PrestoPatternInit(*(++argv));
    }
    else
      PrestoPatternInit("PresoOut");
  }

  if (argc > 1 && strcmp(argv[1], "-log") == 0) {
    /*
     * generate a log of commands and vram writes.
     */
    --argc, ++argv;

    historyFlag = 0;
    logFlag = 1;
    scriptName = (argc > 1 ? --argc, *(++argv) : "sfb");
    SetLogFlag (1, scriptName);
  }

  if (argc > 1 && strncmp(argv[1], "-batch", 6) == 0) {
    /*
     * run in batch mode.
     */
    historyFlag = 0;
    vramCompareFlag = 1;
    PCT_cmds = ( (strcmp(argv[1], "-batchPCT") == 0) ? 1 : 0 );
    if (PCT_cmds)  vramCompareFlag = 0;
    scriptName = (argc > 2 ? argv[2] : "regress");
    eventMask = (argc > 3 ? atoi(argv[3]) : 0);
    OpenLogFiles (scriptName);
    BatchMode (-1);
    exit (0);
  } else 
    /*
     * output vram log file.
     */
    if (argc > 1 && strcmp(argv[1], "-vramlog") == 0) { /* this is outmoded..*/
      historyFlag = 0;
      vramCompareFlag = 1;
      noHaltOnError = 1;
      scriptName = (argc>2 ? argv[2] : "regress");
      OpenLogFiles (scriptName);
      BatchMode (-1);
      exit (0);
    } else                             /* output vram differences on stdout */
    if (argc > 1 && strcmp(argv[1], "-difflog") == 0) {
      if (argc > 2) primitiveCount = atoi(argv[2]);
      if (argc > 3) Seed = atoi(argv[3]);
      historyFlag = 0;
      vramCompareFlag = 1;
      noLog = 1;
      OpenLogFiles (NULL);
      BatchMode (-1);
      exit (0);
    } else                             /* output sfb+ vram writes on stdout */
    if (argc > 1 && strcmp(argv[1], "-nolog") == 0) {   /* & popen on sfbtgen*/
      if (argc > 2) primitiveCount = atoi(argv[2]);
      if (argc > 3) Seed = atoi(argv[3]);
      if (argc > 4) planes = atoi(argv[4]);
      historyFlag = 0;
      vramCompareFlag = 1;
      noHaltOnError = 1;
      noLog = 1;
      OpenLogFiles (NULL);
      BatchMode (-1);
      exit (0);
    } else                              /* output sfb+ vram writes on stdout */
    if (argc > 1 && strcmp(argv[1], "-silent") == 0) {  /* silent comparison */
      if (argc > 2) primitiveCount = atoi(argv[2]);
      if (argc > 3) Seed = atoi(argv[3]);
      historyFlag = 0;
      vramCompareFlag = 1;
      noHaltOnError = 1;
      silent = 1;
      noLog = 1;
      OpenLogFiles (NULL);
      BatchMode (-1);
      exit (0);
    } else {
    /*
     * run interactively.
     */
    historyFlag = 1;
    vramCompareFlag = 0;
    interactiveFlag = 1;

    while (--argc > 0) {
      ++argv;
      if ((yyin = fopen(*argv, "r")) != NULL) {
	cmdFile = yyin;
	yyparse();
	if (vid) {
	  fprintf( vid, "f\n" ); 
	  fflush( vid );
	}
      } else {
	fprintf (stderr, "couldn't open %s\n", *argv);
      }
    }
  }

  do {
    yyin = stdin;
    yyout = stdout;
    cmdFile = stdin;
    yyparse();
  } while (yyin != stdin);
/*
  fprintf (stderr, "clocks = %d, changes = %d, assigns = %d\n",
	   ticks, nodeChanges, nodeEvals);
*/
  exit (0);
  return (0);
}


struct {
  char *symbol;
  int token;
} reservedWords[] = {
  "trace", TRACE,
  "t", TRACE,
  "del", DELETE,
  "delete", DELETE,
  "rm", DELETE,
  "display", DISPLAY,
  "d", DISPLAY,
  "debug", DEBUG,
  "history", HISTORY,
  "hist", HISTORY,
  "init", INIT,
  "dep", DEPOSIT,
  "de", DEPOSIT,
  "ex", EXAMINE,
  "e", EXAMINE,

/*********************************************
 *   add your new command/token pairs here   *
 *********************************************/

  "address1", REGISTER,
  "addrreg", REGISTER,
  "background", REGISTER,
  "bcont", REGISTER,
  "bg", REGISTER,
  "sim", SIM,
  "blockcolor0", REGISTER,
  "blockcolor1", REGISTER,
  "blockcolor2", REGISTER,
  "blockcolor3", REGISTER,
  "blockcolor4", REGISTER,
  "blockcolor5", REGISTER,
  "blockcolor6", REGISTER,
  "blockcolor7", REGISTER,

  "blueinc", REGISTER,
  "bluevalue", REGISTER,
  "boolOp", REGISTER,

  "bres1", REGISTER,
  "bres2", REGISTER,
  "bres3", REGISTER,
  "bresWidth", REGISTER,
  "breswidth", REGISTER,

  "commandstat", REGISTER,

  "copy64src", REGISTER,
  "copy64dst", REGISTER,
  "copy64src2", REGISTER,
  "copy64dst2", REGISTER,

  "cpybf0", REGISTER,
  "cpybf1", REGISTER,
  "cpybf2", REGISTER,
  "cpybf3", REGISTER,
  "cpybf4", REGISTER,
  "cpybf5", REGISTER,
  "cpybf6", REGISTER,
  "cpybf7", REGISTER,

  "cursorbase", REGISTER,
  "cursorxy", REGISTER,
  "dacsetup", REGISTER,
  "dacSetup", REGISTER,
  "data", REGISTER,
  "datareg", REGISTER,
  "deep", REGISTER,
  "dmabase", REGISTER,
  "eeprom", REGISTER,
  "fg", REGISTER,
  "foreground", REGISTER,
  "greeninc", REGISTER,
  "greenvalue", REGISTER,
  "hcontrol", REGISTER,
  "ics", REGISTER,
  "interrupt", REGISTER,

  "mode", REGISTER,
  "ppm", REGISTER,
  "perspixmask", REGISTER,
  "persPixMask", REGISTER,
  "pixelMask", REGISTER,
  "pixelmask", REGISTER,
  "pixelShift", REGISTER,
  "pixelshift", REGISTER,
  "planeMask", REGISTER,
  "planemask", REGISTER,
  "ramDac", REGISTER,
  "ramdac", REGISTER,
  "redinc", REGISTER,
  "redvalue", REGISTER,
  "rop", REGISTER,
  "slope0", REGISTER,
  "slope1", REGISTER,
  "slope2", REGISTER,
  "slope3", REGISTER,
  "slope4", REGISTER,
  "slope5", REGISTER,
  "slope6", REGISTER,
  "slope7", REGISTER,

  "slope000", REGISTER,
  "slope001", REGISTER,
  "slope010", REGISTER,
  "slope011", REGISTER,
  "slope100", REGISTER,
  "slope101", REGISTER,
  "slope110", REGISTER,
  "slope111", REGISTER,

  "slopen0", REGISTER,
  "slopen1", REGISTER,
  "slopen2", REGISTER,
  "slopen3", REGISTER,
  "slopen4", REGISTER,
  "slopen5", REGISTER,
  "slopen6", REGISTER,
  "slopen7", REGISTER,

  "slopen000", REGISTER,
  "slopen001", REGISTER,
  "slopen010", REGISTER,
  "slopen011", REGISTER,
  "slopen100", REGISTER,
  "slopen101", REGISTER,
  "slopen110", REGISTER,
  "slopen111", REGISTER,

  "slopenogo0", REGISTER,
  "slopenogo1", REGISTER,
  "slopenogo2", REGISTER,
  "slopenogo3", REGISTER,
  "slopenogo4", REGISTER,
  "slopenogo5", REGISTER,
  "slopenogo6", REGISTER,
  "slopenogo7", REGISTER,

  "spanwidth", REGISTER,
  "start", REGISTER,
  "stencil", REGISTER, /* replaced intstatus */
  "vcontrol", REGISTER,
  "videobase", REGISTER,
  "videoshift", REGISTER,
  "intStatus", REGISTER,
  "videovalid", REGISTER,
  "zbase", REGISTER,
  "zincf", REGISTER,
  "zincw", REGISTER,
  "zvaluef", REGISTER,
  "zvaluew", REGISTER,
  "eventmask", EVENTMASK,
  "scale", SCALE, 
  "queer", QUEER, 
  "dump", DUMP, 
  "vgawrite", VGAWRITE, 
  "iowrite", IOWRITE, 
  "write", WRITE, 
  "w", WRITE, 
  "aw", ABSWRITE, 
  "aiw", ABSIOWRITE, 
  "wb", WRITEBYTE,
  "r", READ, 
  "ar", ABSREAD, 
  "vgaread", VGAREAD, 
  "ioread", IOREAD, 
  "read", READ, 
  "line", LINE, 
  "oldline", OLDLINE, 
  "cline", CLINE, 
  "bc", BUSCYCLE, 
  "cpoint", CPOINT, 
  "cp", CPOINT, 
  "flush", FLUSH, 
  "fill", FILL, 
  "bfill", BFILL, 
  "fm", FILLMEMORY, 
  "dmawr", DMAWR, 
  "dmard", DMARD, 
  "copy", COPYAREA, 
  "ticks", TICKS, 
  "dino", DINO, 
  "v", VECTOR, 
  "presto", PRESTO, 
  "script", SCRIPT, 
  "scriptz", SCRIPTZ, 
  "width", WIDTH, 
  "video", VIDEO, 
  "batch", BATCH, 
  "idle", IDLE, 
  "log", LOG,
  "source", SOURCE,
  "cread", CREAD,
  "cwrite", CWRITE,
  "memsize", MEMSIZE,
  "checkInit", CHECKINIT,
  "reset", RESET,
/*********************************************
 *      end of custom commands/tokens        *
 *********************************************/

  0, 0
};

extern int StringToken(char *s)
{
  register HASHENTRY *p;

  p = FindOrEnter (s, (char *) IDENT, symTable);

  yylval.sval = p->Symbol;
  return((int) p->Data);
}

extern int NumberToken(char *s)
{
  yylval.ival = atoi(s);
  return(NUMBER);
}

extern int HexNumToken(char *s)
{
  yylval.ival = strtoul(s, 0, 16);
  return (NUMBER);
}

InitReservedWords()
{
  int StringHash(), strcmp();
  int i;

  symTable = NewHashTable(StringHash, strcmp, 0);

  for (i=0; reservedWords[i].symbol; ++i)
    Enter (reservedWords[i].symbol, (char *) reservedWords[i].token, symTable);
}


/*********************************************
 *       add your new procedures here	     *
 *********************************************/

struct {
  char *symbol;
  unsigned token;
} registerList[] = {

  "address1",  ADDRREG_ALIAS,
  "addrreg",  ADDRREG_ADDRESS,
  "background",  BG_ADDRESS,
  "bcont",  BCONT_ADDRESS,
  "bg",  BG_ADDRESS,

  "blockcolor0",  BLOCKCOLOR0,
  "blockcolor1",  BLOCKCOLOR1,
  "blockcolor2",  BLOCKCOLOR2,
  "blockcolor3",  BLOCKCOLOR3,
  "blockcolor4",  BLOCKCOLOR4,
  "blockcolor5",  BLOCKCOLOR5,
  "blockcolor6",  BLOCKCOLOR6,
  "blockcolor7",  BLOCKCOLOR7,

  "blueinc",  BINC_ADDRESS,
  "bluevalue",  BVAL_ADDRESS,
  "boolOp",  ROP_ADDRESS,

  "bres1",  BRES1_ADDRESS,
  "bres2",  BRES2_ADDRESS,
  "bres3",  BRES3_ADDRESS,
  "bresWidth",  BRESWIDTH_ADDRESS,
  "breswidth",  BRESWIDTH_ADDRESS,

  "commandstat",  COMMANDSTAT0,

  "copy64src",  COPY64SRC,
  "copy64dst",  COPY64DST,
  "copy64src2",  COPY64SRC2,
  "copy64dst2",  COPY64DST2,

  "cpybf0",  CPYBF0_ADDRESS,
  "cpybf1",  CPYBF1_ADDRESS,
  "cpybf2",  CPYBF2_ADDRESS,
  "cpybf3",  CPYBF3_ADDRESS,
  "cpybf4",  CPYBF4_ADDRESS,
  "cpybf5",  CPYBF5_ADDRESS,
  "cpybf6",  CPYBF6_ADDRESS,
  "cpybf7",  CPYBF7_ADDRESS,

  "cursorbase",  CURSORBASE,
  "cursorxy",  CURSORXY,
  "dacSetup", DACSETUP_ADDRESS,
  "dacsetup", DACSETUP_ADDRESS,
  "data",  DATAREG_ADDRESS,
  "datareg",  DATAREG_ADDRESS,
  "deep",  DEEP_ADDRESS,
  "dmabase",  DMABASE,
  "eeprom", EEPROM_WRITE,
  "fg",  FG_ADDRESS,
  "foreground",  FG_ADDRESS,
  "greeninc",  GINC_ADDRESS,
  "greenvalue",  GVAL_ADDRESS,
  "hcontrol",  HCONTROL,
  "ics", ICS_ADDRESS,
  "mode",  MODE_ADDRESS,
  "ppm", PERSISTENTPIXMASK,
  "perspixmask", PERSISTENTPIXMASK,
  "persPixMask", PERSISTENTPIXMASK,
  "pixelMask",  PIXMSK_ADDRESS,
  "pixelmask",  PIXMSK_ADDRESS,
  "pixelShift",  PIXSHFT_ADDRESS,
  "pixelshift",  PIXSHFT_ADDRESS,
  "planeMask",  PLANEMASK_ADDRESS,
  "planemask",  PLANEMASK_ADDRESS,
  "ramdac", RAMDAC_ADDRESS,
  "ramdac", RAMDAC_ADDRESS,
  "redinc",  RINC_ADDRESS,
  "redvalue",  RVAL_ADDRESS,
  "rop",  ROP_ADDRESS,
  "slope0",  SLOPE_000,
  "slope1",  SLOPE_001,
  "slope2",  SLOPE_010,
  "slope3",  SLOPE_011,
  "slope4",  SLOPE_100,
  "slope5",  SLOPE_101,
  "slope6",  SLOPE_110,
  "slope7",  SLOPE_111,

  "slope000",  SLOPE_000,
  "slope001",  SLOPE_001,
  "slope010",  SLOPE_010,
  "slope011",  SLOPE_011,
  "slope100",  SLOPE_100,
  "slope101",  SLOPE_101,
  "slope110",  SLOPE_110,
  "slope111",  SLOPE_111,

  "slopen0",  SLOPEN_000,
  "slopen1",  SLOPEN_001,
  "slopen2",  SLOPEN_010,
  "slopen3",   SLOPEN_011,
  "slopen4",   SLOPEN_100,
  "slopen5",   SLOPEN_101,
  "slopen6",  SLOPEN_110,
  "slopen7",  SLOPEN_111,

  "slopen000",  SLOPEN_000,
  "slopen001",  SLOPEN_001,
  "slopen010",  SLOPEN_010,
  "slopen011",  SLOPEN_011,
  "slopen100",  SLOPEN_100,
  "slopen101",  SLOPEN_101,
  "slopen110",  SLOPEN_110,
  "slopen111",  SLOPEN_111,

  "slopenogo0",  SLOPEN_000,
  "slopenogo1",  SLOPEN_001,
  "slopenogo2",  SLOPEN_010,
  "slopenogo3",  SLOPEN_011,
  "slopenogo4",  SLOPEN_100,
  "slopenogo5",  SLOPEN_101,
  "slopenogo6",  SLOPEN_110,
  "slopenogo7",  SLOPEN_111,

  "spanwidth",  SPANWIDTH,
  "stencil", STENCIL_ADDRESS, /* replaced int status */
  "start",  START_ADDRESS,
  "vcontrol",  VCONTROL,
  "videobase",  VIDEOBASE,
  "videoshift",  VIDEOSHIFT,
  "interrupt",  INTERRUPT,
  "videovalid",  VIDEOVALID,
  "zbase",  ZADDR_BASE_ADDRESS,
  "zincf",  ZINC_FRACT_ADDRESS,
  "zincw",  ZINC_WHOLE_ADDRESS,
  "zvaluef",  ZVAL_FRACT_ADDRESS,
  "zvaluew",  ZVAL_WHOLE_ADDRESS,

  0, NULL
};

HASHTAB *regSymTable;

InitRegisters()
{
  int StringHash(), strcmp();
  int i;

  regSymTable = NewHashTable(StringHash, strcmp, 0);

  for (i=0; registerList[i].symbol; ++i)
    Enter (registerList[i].symbol, (char *) registerList[i].token, regSymTable);
}

unsigned RegisterAddress( char *s )
{
  /*
   * initialize the table of register symbols
   * the first time this routine is called.
   */
  if( regSymTable == NULL ) InitRegisters();

  if (Find (s, regSymTable)) {
    register HASHENTRY *p;

    p = Lookup( s, regSymTable );
    return( (unsigned) p->Data );
  } else {
    fprintf( stderr, "register \"%s\" address not found in table\n", s );
    exit( 1 );
  }
}

unsigned ReadRegister( char *s )
{
  return( BusRead( RegisterAddress( s )));
}

void WriteRegister( char *s, unsigned data )
{
  unsigned addr = RegisterAddress( s );

  /*
   * if a byte value has been specified for foreground or background,
   * replicate the value 4 times across the word before doing the write.
   */
  if( (addr == FG_ADDRESS || addr == BG_ADDRESS) && data < 256 ) {
    COLORS reg;
    int i;

    for (i=0; i<4; ++i)
      reg.byte[i] = data;

    data = reg.data;
  }

  BusWrite( addr, data, LWMASK );
}

extern FILE *cmdOut;
extern FILE *vramOut;

void SetLogFlag( int n, char *s )
{
  char name[256];

  logFlag = n;

  if (n) {
    sprintf (name, "%s.cmds", s);
    if ((cmdOut = fopen(name, "w")) == NULL) {
      fprintf (stderr, "can't open %s\n", name);
      exit (1);
    }
    sprintf (name, "%s.vram", s);
    if ((vramOut = fopen(name, "w")) == NULL) {
      fprintf (stderr, "can't open %s\n", name);
      exit (1);
    }
  } else {
    if (cmdOut) fclose (cmdOut);
    if (vramOut) fclose (vramOut);
  }
}

Init( )
{
}

/*********************************************
 *      end of custom procedures	     *
 *********************************************/
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 76
# define YYLAST 341
short yyact[]={

  12, 218, 127,  65, 119, 116, 102, 101,  98,  97,
  63,  67,  69,  96,  61,  60,  73,  74,  75,  76,
  59,  79,  80,  81,  82,  83,  84,  86,  87,  88,
  89,  90,  91,  92,  93,  94,  58,  57, 238, 234,
 232, 100, 229, 228, 103, 104, 105, 106, 107, 226,
 109, 110, 111, 225, 115, 230, 117, 118, 113, 215,
 122, 214, 188, 125, 174, 158,  65, 152, 141, 210,
 209, 205, 201, 200, 199, 121,  65, 190, 184,  72,
 134, 135, 136, 137, 138, 139, 140, 142,  68, 182,
  65,  66,  62,  65, 149, 150, 151,  65, 153,  65,
  65, 181,  65,  65, 159, 160, 161, 162, 163, 164,
  65, 165, 166, 167,  65, 180, 217, 179, 178, 177,
 176, 173, 172,  65, 175, 171,  65,  65, 170, 169,
 168, 157, 156, 155, 154, 148, 147, 146, 145, 144,
 143, 133, 183, 132, 131, 130, 129, 128, 126, 124,
 123, 185, 186, 187, 120, 189, 112, 108,  99,  95,
  77,  71, 191, 192, 193, 194, 195, 196, 197, 198,
  70,   2,   1,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0, 202, 203, 204,
   0,   0,   0, 206, 207, 208,   0,   0, 211, 212,
 213,   0,   0,   0,   0,   0, 216,   0,   0,   0,
 220,   0,   0,   0,   0, 223,  64,   0, 224,   0,
   0,   0, 227,   0,   0, 231,   0,   0,   0, 233,
   0,   0,   0,   0,   0, 235,   0, 236, 237,   0,
   0,   0,   0,   0,   0,   0,  13,   0,   0,  11,
   3,   5,   8,   9,  10,   7,   6,  15,  16,  18,
  25,  19,  42,  48,  33,  51,  24,  52,  50,  29,
  22,  43,  44,  46,  47,  21,  20,  27,  26,  64,
  30,  45,  37,  41,  28,  23,  31,  32,  14,  64,
  34,  49,  53,  35,  36,   4,  17,  40,  38,  39,
  56,  54,  55,  64,   0, 114,  64,   0,   0,   0,
  64,   0,  64,  64,   0,  64,  64,   0,   0,   0,
  85,   0,   0,  64, 219,   0,   0,  64, 221, 222,
  78,   0,   0,   0,   0,   0,  64,   0,   0,  64,
  64 };
short yypact[]={

-1000, -10,-1000,-220,-221,-237,-242,-243,  82,  81,
  78, 160,-1000, 151,  69, -42, -42, -42, 150,  31,
 -42, -42, -42, -42, -42,  21, -42, -42, -42, -42,
 -42, -42, -42, -42, 149,-244,-248,-249, 148, -42,
-250,-251, -42, -42, -42, -42, -42, 147, -42, -42,
 -42, 146,  48,-252, -42, -42,-253, 144,  65, 140,
 139, -42,-1000, 138,-1000,-256,-1000, 137,-1000, 136,
-1000,-1000,-1000, 135, 134, 133, 131,-1000, -42, -42,
 -42, -42, -42, -42, -42,  58, 130, 129, 128, 127,
 126, 125, -42, -42, -42,-1000,  57, 124, 123,-1000,
 122, 121,  55, -42, -42, -42, -42, -42,-1000, -42,
 -42, -42,-1000,-1000, 120, 119, 118, 115, 112,  54,
-1000,-1000, -42,-1000,-1000, 110,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000, 109, 108, 107, 105,  91,  79,
 -42,-1000,  68,-1000,-1000,-1000,-1000,-1000,-1000, -42,
 -42, -42,-1000,  52,-1000,-1000,-1000,-1000,-1000,  67,
 -42, -42, -42, -42, -42, -42, -42, -42,-1000,-1000,
-1000,-1000,-1000,  64,-1000,  63,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,  62,-1000, -42, -42, -42,-1000,  61,
-1000, -42, -42, -42,  60,  59, -42, -42, -42,-1000,
-1000,-1000,  51,  49, -42,-1000, -42, -42, -42,-1000,
-1000, -42, -42, -42,-1000,-1000, -42,  43,-1000,  39,
 -42,  33,  32,  45,  30,-1000,-1000, -42,-1000,-1000,
-1000,  29,-1000, -42,-1000, -42, -42,  28,-1000 };
short yypgo[]={

   0, 116,   1, 172, 171 };
short yyr1[]={

   0,   3,   3,   4,   4,   4,   4,   4,   4,   4,
   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
   4,   4,   2,   2,   1,   1 };
short yyr2[]={

   0,   0,   2,   3,   3,   5,   3,   3,   4,   2,
   3,   2,   3,   2,   3,   2,   1,   2,   2,   3,
   3,   3,   3,   2,   4,   4,   4,   4,   4,   4,
   5,   3,   4,   3,   3,   3,   3,   3,   3,   6,
   6,   8,   2,   3,   4,   5,   3,   3,   2,   3,
   3,   3,   4,   7,   7,  12,   5,   5,   2,   7,
   7,   7,   8,   2,   2,   3,   3,   3,   3,   3,
   4,   3,   1,   2,   0,   1 };
short yychk[]={

-1000,  -3,  -4, 260, 305, 261, 266, 265, 262, 263,
 264, 259,  10, 256, 298, 267, 268, 306, 269, 271,
 286, 285, 280, 295, 276, 270, 288, 287, 294, 279,
 290, 296, 297, 274, 300, 303, 304, 292, 308, 309,
 307, 293, 272, 281, 282, 291, 283, 284, 273, 301,
 278, 275, 277, 302, 311, 312, 310, 257, 257, 257,
 257, 257,  10,  -2, 258,  45,  10,  -2,  10,  -2,
  10,  10,  10,  -2,  -2,  -2,  -2,  10, 299,  -2,
  -2,  -2,  -2,  -2,  -2, 299,  -2,  -2,  -2,  -2,
  -2,  -2,  -2,  -2,  -2,  10, 257, 257, 257,  10,
  -2, 257, 257,  -2,  -2,  -2,  -2,  -2,  10,  -2,
  -2,  -2,  10,  10, 257,  -2, 257,  -2,  -2, 257,
  10,  10,  -2,  10,  10,  -2,  10, 258,  10,  10,
  10,  10,  10,  10,  -2,  -2,  -2,  -2,  -2,  -2,
  -2,  10,  -2,  10,  10,  10,  10,  10,  10,  -2,
  -2,  -2,  10,  -2,  10,  10,  10,  10,  10,  -2,
  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  10,  10,
  10,  10,  10,  -2,  10,  -2,  10,  10,  10,  10,
  10,  10,  10,  -2,  10,  -2,  -2,  -2,  10,  -2,
  10,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  10,
  10,  10,  -2,  -2,  -2,  10,  -2,  -2,  -2,  10,
  10,  -2,  -2,  -2,  10,  10,  -2,  -1,  -2,  -1,
  -2,  -1,  -1,  -2,  -2,  10,  10,  -2,  10,  10,
  10,  -2,  10,  -2,  10,  -2,  -2,  -2,  10 };
short yydef[]={

   1,  -2,   2,   0,   0,   0,   0,   0,   0,   0,
   0,   0,  16,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   9,   0,  72,   0,  11,   0,  13,   0,
  15,  17,  18,   0,   0,   0,   0,  23,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  42,   0,   0,   0,  48,
   0,   0,   0,   0,   0,   0,   0,   0,  58,   0,
   0,   0,  63,  64,   0,   0,   0,   0,   0,   0,
   3,   4,   0,   6,   7,   0,  10,  73,  12,  14,
  19,  20,  21,  22,   0,   0,   0,   0,   0,   0,
   0,  31,   0,  33,  34,  35,  36,  37,  38,   0,
   0,   0,  43,   0,  46,  47,  49,  50,  51,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  65,  67,
  66,  68,  69,   0,  71,   0,   8,  24,  25,  26,
  27,  28,  29,   0,  32,   0,   0,   0,  44,   0,
  52,   0,   0,   0,   0,   0,   0,   0,   0,  70,
   5,  30,   0,   0,   0,  45,  74,  74,   0,  56,
  57,  74,  74,   0,  39,  40,   0,   0,  75,   0,
   0,   0,   0,   0,   0,  53,  54,   0,  59,  60,
  61,   0,  41,   0,  62,   0,   0,   0,  55 };
#ifndef lint
static char yaccpar_sccsid[] = "@(#)yaccpar	4.1	(Berkeley)	2/11/83";
#endif

#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

#ifdef YYDEBUG
int yydebug = 0; /* 1 for debugging */
#endif
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse() {

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

#ifdef YYDEBUG
	if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
#endif
		if( ++yyps> &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
		*yyps = yystate;
		++yypv;
		*yypv = yyval;

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
	if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
			}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

		case 0:   /* brand new error */

			yyerror( "syntax error" );
		yyerrlab:
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];  /* simulate a shift of "error" */
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			   /* the current yyps has no shift onn "error", pop stack */

#ifdef YYDEBUG
			   if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
#endif
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

#ifdef YYDEBUG
			if( yydebug ) printf( "error recovery discards char %d\n", yychar );
#endif

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

#ifdef YYDEBUG
		if( yydebug ) printf("reduce %d\n",yyn);
#endif
		yyps -= yyr2[yyn];
		yypvt = yypv;
		yypv -= yyr2[yyn];
		yyval = yypv[1];
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 2:
# line 308 "gram.dwgs.y"
{ 
#ifdef RANDEVENTS	    
	  GenerateRandomEvents(); 
#endif
	  if (vid && yyin == stdin) { fflush(vid); } 
	} break;
case 3:
# line 317 "gram.dwgs.y"
{ AddNet(yypvt[-1].sval); } break;
case 4:
# line 319 "gram.dwgs.y"
{ AddVector (yypvt[-1].sval, 31, 0); } break;
case 5:
# line 321 "gram.dwgs.y"
{ AddVector (yypvt[-3].sval, yypvt[-2].ival, yypvt[-1].ival); } break;
case 6:
# line 323 "gram.dwgs.y"
{ RemoveNet(yypvt[-1].sval); } break;
case 7:
# line 325 "gram.dwgs.y"
{ Examine(yypvt[-1].sval); } break;
case 8:
# line 327 "gram.dwgs.y"
{ DepositCommand(yypvt[-2].sval, yypvt[-1].ival); } break;
case 9:
# line 329 "gram.dwgs.y"
{ Display(displaySize); } break;
case 10:
# line 331 "gram.dwgs.y"
{ displaySize = yypvt[-1].ival;
                          printf ("displaySize = %d\n", displaySize);
			  Display(displaySize); } break;
case 11:
# line 335 "gram.dwgs.y"
{ printf ("debugFlag = %d\n", debugFlag); } break;
case 12:
# line 337 "gram.dwgs.y"
{ debugFlag = yypvt[-1].ival; } break;
case 13:
# line 339 "gram.dwgs.y"
{ printf ("historyFlag = %d\n", historyFlag); } break;
case 14:
# line 341 "gram.dwgs.y"
{ historyFlag = yypvt[-1].ival;
                          printf ("historyFlag = %d\n", historyFlag); } break;
case 15:
# line 344 "gram.dwgs.y"
{ Init(); } break;
case 16:
# line 346 "gram.dwgs.y"
{ Tick(); } break;
case 17:
# line 348 "gram.dwgs.y"
{ yyerrok; } break;
case 18:
# line 354 "gram.dwgs.y"
{ printf ("eventMask = %d\n", eventMask); } break;
case 19:
# line 356 "gram.dwgs.y"
{ eventMask = yypvt[-1].ival; } break;
case 20:
# line 358 "gram.dwgs.y"
{ SetScale (yypvt[-1].ival); } break;
case 21:
# line 360 "gram.dwgs.y"
{ extern int watchQueerValues; watchQueerValues = yypvt[-1].ival; } break;
case 22:
# line 362 "gram.dwgs.y"
{
			  int i;

			  for (i=yypvt[-1].ival; i>0; --i)
			    Tick();
			} break;
case 23:
# line 369 "gram.dwgs.y"
{
			  DumpRegisters();
			} break;
case 24:
# line 373 "gram.dwgs.y"
{
			  WriteRegister(yypvt[-2].sval, (unsigned) yypvt[-1].ival);
			} break;
case 25:
# line 377 "gram.dwgs.y"
{
			  /*
			   * assume that any addresses within the first
			   * megabyte are intended as frame buffer addresses
			   * (ordinarily this is the address space of the ROM).
			   */
			  if (yypvt[-2].ival >= 0x100000)
			    BusWrite((unsigned) yypvt[-2].ival, (unsigned) yypvt[-1].ival, 0xf);
			  else
			    BusWrite((unsigned) 0x800000 + yypvt[-2].ival, (unsigned) yypvt[-1].ival, 0xf);
			} break;
case 26:
# line 389 "gram.dwgs.y"
{
#ifdef PCI
			  /*
			   * assume that any addresses within the first
			   * megabyte are intended as frame buffer addresses
			   * (ordinarily this is the address space of the ROM).
			   */
			  if (yypvt[-2].ival >= 0x100000)
			    BusIOWrite((unsigned) yypvt[-2].ival, (unsigned) yypvt[-1].ival, 0xf);
			  else
			    BusIOWrite((unsigned) 0x800000 + yypvt[-2].ival, (unsigned) yypvt[-1].ival, 0xf);
#endif
			} break;
case 27:
# line 403 "gram.dwgs.y"
{
			  /*
			   * for vga PCT slaves are in ROM space, so pass thru addr
			   * unchanged.
			   */
			   BusWrite((unsigned) yypvt[-2].ival, (unsigned) yypvt[-1].ival, 0xf);
			} break;
case 28:
# line 411 "gram.dwgs.y"
{
			  BusWrite((unsigned) yypvt[-2].ival, (unsigned) yypvt[-1].ival, 0xf);
			} break;
case 29:
# line 415 "gram.dwgs.y"
{
#ifdef PCI
			  PCIConfigWrite((unsigned) yypvt[-2].ival, (unsigned) yypvt[-1].ival, 0xf);
#endif
			} break;
case 30:
# line 421 "gram.dwgs.y"
{
			  /*
			   * assume that any addresses within the first
			   * megabyte are intended as frame buffer addresses
			   * (ordinarily this is the address space of the ROM).
			   */
			  if (yypvt[-3].ival >= 0x100000)
			    BusWrite((unsigned) yypvt[-3].ival,
				     (unsigned) yypvt[-2].ival,
				     (unsigned) yypvt[-1].ival);
			  else
			    BusWrite((unsigned) 0x800000 + yypvt[-3].ival,
				     (unsigned) yypvt[-2].ival,
				     (unsigned) yypvt[-1].ival);
			} break;
case 31:
# line 437 "gram.dwgs.y"
{
			  unsigned d = ReadRegister(yypvt[-1].sval);
			  if (!logFlag) printf ("0x%08x\n", d);
			} break;
case 32:
# line 442 "gram.dwgs.y"
{
			  unsigned d = ReadRegister(yypvt[-2].sval);
			  if (d != yypvt[-1].ival)
			    if (!logFlag) printf ("expected = %08x, received = %08x\n",
				    yypvt[-1].ival, d);
			} break;
case 33:
# line 449 "gram.dwgs.y"
{
			  unsigned d;
			  /*
			   * assume that any addresses within the first
			   * megabyte are intended as frame buffer addresses
			   * (ordinarily this is the address space of the ROM).
			   */
			  if (yypvt[-1].ival >= 0x100000)
			    d = BusRead(yypvt[-1].ival);
			  else
			    d = BusRead((unsigned) 0x800000 + yypvt[-1].ival);

			  if (!logFlag) printf ("0x%08x\n", d);
                        } break;
case 34:
# line 464 "gram.dwgs.y"
{
#ifdef PCI
			  unsigned d = BusIORead((unsigned) 0x800000 + yypvt[-1].ival);
			  if (!logFlag) printf ("0x%08x\n", d);
#endif
                        } break;
case 35:
# line 471 "gram.dwgs.y"
{
			  unsigned d = BusRead((unsigned)  yypvt[-1].ival);
			  if (!logFlag) printf ("0x%08x\n", d);
			} break;
case 36:
# line 476 "gram.dwgs.y"
{
#ifdef PCI
			  unsigned d = PCIConfigRead((unsigned) yypvt[-1].ival);
			  if (!logFlag) printf ("0x%08x\n", d);
#endif
			} break;
case 37:
# line 483 "gram.dwgs.y"
{
			  printf ("0x%08x\n", BusRead((unsigned) yypvt[-1].ival));
			} break;
case 38:
# line 487 "gram.dwgs.y"
{
			  printf ("0x%08x\n", BusRead((unsigned) yypvt[-1].ival));
			} break;
case 39:
# line 491 "gram.dwgs.y"
{
			  DmaReadBlt(dmaBuff+yypvt[-4].ival, yypvt[-3].ival, yypvt[-2].ival, yypvt[-1].ival);
			} break;
case 40:
# line 495 "gram.dwgs.y"
{
			  DmaWriteBlt(dmaBuff+yypvt[-4].ival, yypvt[-3].ival, yypvt[-2].ival, yypvt[-1].ival);
			} break;
case 41:
# line 499 "gram.dwgs.y"
{
			  AreaCopy(yypvt[-6].ival, yypvt[-5].ival, yypvt[-4].ival, yypvt[-3].ival, yypvt[-2].ival, yypvt[-1].ival);
			} break;
case 42:
# line 503 "gram.dwgs.y"
{
			  extern int ticks;
			  printf ("ticks = %d\n", ticks);
			} break;
case 43:
# line 508 "gram.dwgs.y"
{
			  DinoTraceInit (yypvt[-1].sval);
			} break;
case 44:
# line 512 "gram.dwgs.y"
{
			  extern int firstDinoTick;

			  firstDinoTick = yypvt[-1].ival;
			  DinoTraceInit (yypvt[-2].sval);
			} break;
case 45:
# line 519 "gram.dwgs.y"
{
			  extern int firstDinoTick, lastDinoTick;

			  firstDinoTick = yypvt[-2].ival;
			  lastDinoTick  = yypvt[-1].ival;
			  DinoTraceInit (yypvt[-3].sval);
			} break;
case 46:
# line 527 "gram.dwgs.y"
{
			  PrestoPatternInit (yypvt[-1].sval);
			} break;
case 47:
# line 531 "gram.dwgs.y"
{
			  if ((cmdFile = fopen(yypvt[-1].sval, "r")) != NULL) {
			    BatchMode(-1);
			  } else {
			    fprintf (stderr, "can't open '%s'\n", yypvt[-1].sval);
			  }
			} break;
case 48:
# line 539 "gram.dwgs.y"
{
			  if ((rasterVid = popen ("video", "w")) == NULL) {
			    fprintf (stderr, "can't popen 'video'\n");
			  }
			} break;
case 49:
# line 545 "gram.dwgs.y"
{
			  extern int ramWidth;

			  ramWidth = yypvt[-1].ival;
			} break;
case 50:
# line 551 "gram.dwgs.y"
{
			  int tmp = interactiveFlag;
			  char s[256];

			  interactiveFlag = 0;
			  historyFlag = 0;
			  vramCompareFlag = 1;

			  OpenLogFiles (yypvt[-1].sval);
			  BatchMode (-1);

			  interactiveFlag = tmp;
			} break;
case 51:
# line 565 "gram.dwgs.y"
{
			  extern char *ScriptPath (char *name);
			  int tmp = interactiveFlag;
			  char s[256];
			  char *p;

			  interactiveFlag = 0;
			  if ((p = ScriptPath (yypvt[-1].sval)) != NULL) {
			    sprintf( s, "uncompress -c %s/%s.cmds.Z", p, yypvt[-1].sval );
			    if ((cmdFile = popen(s, "r")) != NULL) {
			      BatchMode(-1);
			    } else {
			      fprintf (stderr, "can't open '%s'\n", yypvt[-1].sval);
			    }
			  }
			  interactiveFlag = tmp;
			} break;
case 52:
# line 583 "gram.dwgs.y"
{
			  extern char *ScriptPath (char *name);
			  int tmp = interactiveFlag;
			  char s[256];
			  char *p;

			  interactiveFlag = 0;
			  if ((p = ScriptPath (yypvt[-2].sval)) != NULL) {
			    sprintf( s, "uncompress -c %s/%s.cmds.Z", p, yypvt[-2].sval );
			    if ((cmdFile = popen(s, "r")) != NULL) {
			      BatchMode(yypvt[-1].ival);
			    } else {
			      fprintf (stderr, "can't open '%s'\n", yypvt[-2].sval);
			    }
			  }
			  interactiveFlag = tmp;
			} break;
case 53:
# line 601 "gram.dwgs.y"
{
			  Line(yypvt[-5].ival, yypvt[-4].ival, yypvt[-3].ival, yypvt[-2].ival, (unsigned) yypvt[-1].ival, /* smooth? */ 0);
			} break;
case 54:
# line 605 "gram.dwgs.y"
{
			  OldLine(yypvt[-5].ival, yypvt[-4].ival, yypvt[-3].ival, yypvt[-2].ival, (unsigned) yypvt[-1].ival);
			} break;
case 55:
# line 609 "gram.dwgs.y"
{
			  SmoothShadedLine(yypvt[-10].ival, yypvt[-9].ival, yypvt[-8].ival, yypvt[-7].ival, yypvt[-6].ival, yypvt[-5].ival, yypvt[-4].ival, yypvt[-3].ival, yypvt[-2].ival, yypvt[-1].ival);
			} break;
case 56:
# line 613 "gram.dwgs.y"
{
#ifdef NEW_BUS_INTERFACE
			  printf("BusCycle is unavailable in this model\n");
#else
			  BusCycle(yypvt[-3].ival, yypvt[-2].ival, yypvt[-1].ival);
#endif
			} break;
case 57:
# line 621 "gram.dwgs.y"
{
			  SmoothShadedLine(yypvt[-3].ival, yypvt[-2].ival, yypvt[-3].ival+1, yypvt[-2].ival+1, 
					   yypvt[-1].ival & 0xff, (yypvt[-1].ival >> 8) & 0xff, (yypvt[-1].ival >> 16) & 0xff,
					   yypvt[-1].ival & 0xff, (yypvt[-1].ival >> 8) & 0xff, (yypvt[-1].ival >> 16) & 0xff);
			} break;
case 58:
# line 627 "gram.dwgs.y"
{
			  if (vid) {
			    fprintf( vid, "f\n" );
			    fflush( vid );
			  }
			} break;
case 59:
# line 634 "gram.dwgs.y"
{
			  Fill(yypvt[-5].ival, yypvt[-4].ival, yypvt[-3].ival, yypvt[-2].ival, (unsigned) yypvt[-1].ival);
			} break;
case 60:
# line 638 "gram.dwgs.y"
{
			  BlockFill(yypvt[-5].ival, yypvt[-4].ival, yypvt[-3].ival, yypvt[-2].ival, (unsigned) yypvt[-1].ival);
			} break;
case 61:
# line 642 "gram.dwgs.y"
{
			  FillMemory(yypvt[-5].ival, yypvt[-4].ival, yypvt[-3].ival, yypvt[-2].ival, (unsigned) yypvt[-1].ival, 0xffffffff);
			} break;
case 62:
# line 646 "gram.dwgs.y"
{
			  FillMemory(yypvt[-6].ival, yypvt[-5].ival, yypvt[-4].ival, yypvt[-3].ival, (unsigned) yypvt[-2].ival, yypvt[-1].ival);
			} break;
case 63:
# line 650 "gram.dwgs.y"
{
			  MakeIdle();
			} break;
case 64:
# line 654 "gram.dwgs.y"
{
			  SetLogFlag (!logFlag, "sfb");
			  printf( "logFlag = %d\n", logFlag );
			} break;
case 65:
# line 659 "gram.dwgs.y"
{
			  SetLogFlag (1, yypvt[-1].sval);
			  printf( "logFlag = %d\n", logFlag );
			} break;
case 66:
# line 664 "gram.dwgs.y"
{
			  FILE *fsrc;

			  if ((fsrc = fopen(yypvt[-1].sval, "r")) == NULL) {
			    fprintf (stderr, "couldn't open '%s'\n", yypvt[-1].sval);
			  } else {
			    yyin = fsrc;
			    yyparse();
			  }
			} break;
case 67:
# line 675 "gram.dwgs.y"
{
			  SetLogFlag (yypvt[-1].ival, "sfb");
			  printf( "logFlag = %d\n", logFlag );
			} break;
case 68:
# line 680 "gram.dwgs.y"
{
			  ChipReset(yypvt[-1].ival);
			} break;
case 69:
# line 684 "gram.dwgs.y"
{
			  extern FILE *cmdOut;
			  if (logFlag)
			    fprintf (cmdOut, "3 %x\n", yypvt[-1].ival);

			  printf( "changing memory size to %x bytes\n", yypvt[-1].ival);
			  InitVRAM (yypvt[-1].ival);
			  printf( "memory initialized pseudo-randomly.\n");
			} break;
case 70:
# line 694 "gram.dwgs.y"
{
			  FillNets(yypvt[-1].ival);
			  ChipInit(16);
			  FillNets(~yypvt[-1].ival);
			  ChipInit(16);
			  CompareHistory(yypvt[-2].sval);
			} break;
case 71:
# line 702 "gram.dwgs.y"
{
			  FillNets(0);
			  ChipInit(16);
			  FillNets(~0);
			  ChipInit(16);
			  CompareHistory(yypvt[-1].sval);
			} break;
case 72:
# line 716 "gram.dwgs.y"
{ yyval.ival = yypvt[-0].ival; } break;
case 73:
# line 717 "gram.dwgs.y"
{ yyval.ival = -yypvt[-0].ival; } break;
case 74:
# line 724 "gram.dwgs.y"
{ yyval.ival = 0xffffffff; } break;
case 75:
# line 725 "gram.dwgs.y"
{ yyval.ival = yypvt[-0].ival; } break; 
		}
		goto yystack;  /* stack new state and value */

	}
