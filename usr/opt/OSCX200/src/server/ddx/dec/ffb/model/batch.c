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
static char *rcsid = "@(#)$RCSfile: batch.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:21:26 $";
#endif
/*
 under RCS

 $Author: Robert_Lembree $
 $Date: 1993/11/19 21:21:26 $, $Revision: 1.1.2.2 $
 $Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/ffb/model/batch.c,v 1.1.2.2 1993/11/19 21:21:26 Robert_Lembree Exp $
 $Locker:  $
 $Log: batch.c,v $
 * Revision 1.1.2.2  1993/11/19  21:21:26  Robert_Lembree
 * 	SFB+ Initial version
 * 	[1993/11/19  16:32:48  Robert_Lembree]
 *
 * Revision 1.1.1.2  1993/11/19  16:32:48  Robert_Lembree
 * 	SFB+ Initial version
 *
 * Revision 2.43  1993/06/15  18:03:45  pcharles
 * swapped int/extern count variables decl in batch.c and
 * sfbTester.c to allow exclusion of batch.c from link
 *
 * Revision 2.42  1993/06/14  20:11:21  rsm
 * added check for mismatches occurring due to differences in copy buffer
 * initialization.
 *
 * Revision 2.41  1993/06/11  07:08:49  pcharles
 * indented RAMDAC messages
 *
 * Revision 2.40  1993/06/10  19:25:31  pcharles
 * #ifdef'd out GenerateRandEvents on RANDEVENTS
 *
 * Revision 2.39  1993/06/10  18:56:47  pcharles
 * put all random events in function GenerateRandomEvents() to
 * facilitate usage from gram.dwgs.y
 *
 * Revision 2.38  1993/06/10  16:38:47  pcharles
 * changed ramdac read/write counts to look at each ramdac separately
 *
 * Revision 2.37  1993/06/10  15:50:37  pcharles
 * added eventMask to control random event injection
 * RandomIcsWrite()
 *
 * Revision 2.36  1993/06/09  22:36:31  pcharles
 * put random parallel port events in compile-time conditional
 *
 * Revision 2.35  1993/06/09  22:07:42  pcharles
 * added random eeprom, parallel port, ramdac and console events
 * to module part models, to support calls from batch.c
 * InitRamdac() and InitEeprom()
 * and misc. bug fixes
 *
 * Revision 2.34  1993/06/08  15:18:38  rsm
 * changed order of search path, to give preference to scripts
 * in "." and the various inboxes.
 * added initialization for video timing and cursor.
 *
 * Revision 2.33  1993/06/07  12:37:41  mullaly
 * added conditional munging of addresses to keep them in PCI address range
 *
 * Revision 2.32  1993/06/04  15:57:17  rsm
 * removed blank lines from input stream
 * adjust bogus planemasks that some scripts write in 8-bit unpacked mode
 *
 * Revision 2.31  1993/06/03  15:25:31  siegrist
 * pct_seldlay to delay pci dev_sel
 *
 * Revision 2.30  1993/05/28  18:45:49  siegrist
 * modified trdy and disconnect logic
 *
 * Revision 2.29  1993/05/28  18:01:33  chris
 * added setrlimit call to prevent the model from dumping core
 *
 * Revision 2.28  1993/05/27  18:52:02  siegrist
 * fixed trdy logic
 *
 * Revision 2.26  1993/05/25  18:57:49  rsm
 * *always* call InitVRAM when a memory size command is parsed
 *
 * Revision 2.25  1993/05/25  18:18:24  rsm
 * use memory size command to change the amount of memory being simulated
 *
 * Revision 2.24  1993/05/25  01:28:21  rsm
 * added /disks/sfbScripts3 to search path
 *
 * Revision 2.23  1993/05/18  18:40:25  rsm
 * added a warning if the script file is found more than once along the searchpath
 *
 * Revision 2.22  1993/05/17  18:28:44  rsm
 * added heuristics for detecting truncated command files
 *
 * Revision 2.21  1993/05/14  22:58:32  rsm
 * added another disk to the search path
 *
 * Revision 2.20  1993/05/06  20:27:32  rsm
 * removed code that adjusted address for old traces
 *
 * Revision 2.19  1993/05/05  19:49:26  rsm
 * added /db/sfbScripts/inbox to search path
 *
 * Revision 2.18  1993/05/05  17:16:50  rsm
 * added command for memory size declaration
 *
 * Revision 2.17  1993/05/04  12:57:40  rsm
 * find batch scripts along a search path rather than a single directory
 * use shadow planemask for FillMemory calls when a planemask hasn't been
 * specified.
 *
 * Revision 2.16  1993/04/30  14:19:29  rsm
 * new FillMemory implementation
 *
 * Revision 2.15  1993/04/26  21:14:52  rsm
 * use shadowDeep rather than peeking into sfbReg
 *
 * Revision 2.14  1993/04/23  17:56:23  siegrist
 * added pct default master, and setbyte commands
 *
 * Revision 2.13  1993/04/13  14:24:11  pcharles
 * added parameter (planes) to effect 8/32 plane testing via testgen
 *
 * Revision 2.12  1993/04/13  14:21:06  chris
 * Fix virtual addresses that do not actually point to FB space
 *
 * Revision 2.11  1993/04/07  14:48:07  siegrist
 * added pct_message...able to pass messages during batch
 *
 * Revision 2.10  1993/04/07  00:23:31  rsm
 * removed code that was munging addresses from trace files
 * replace FillMemory routine with new implementation
 *
 * Revision 2.9  1993/04/02  18:46:56  siegrist
 * added set dma_data
 *
 * Revision 2.8  1993/03/29  16:02:12  rsm
 * hardcoded new/old addresses into FixLine() routine
 *
 * Revision 2.7  1993/03/23  23:01:18  rsm
 * print out clock tick for incorrect read data
 *
 * Revision 2.6  1993/03/23  17:50:22  rsm
 * added code to modify input from scripts according to recent register changes
 *
 * Revision 2.5  1993/03/17  20:45:35  mullaly
 * changed the startup write of the statcomm register to 7
 *
 * Revision 2.4  1993/03/03  15:07:30  rsm
 * moved storage declaration for cmdFile and dmaBuff to fix linking
 * problem for Jim Peacock.
 *
 * Revision 2.3  1993/02/25  17:46:52  chris
 * Removed OpenTrace...Moved to debug.c
 *
 * Revision 2.2  1993/02/22  20:20:00  siegrist
 * added dinotrace for pct batch.
 *
 * Revision 2.1  1993/02/22  15:06:42  rsm
 * set cmdFile and vramFile to NULL after closing.
 *
 * Revision 2.0  1993/02/15  22:41:55  rsm
 * rev 2.0
 *
 * Revision 1.21  1993/01/14  18:57:17  scott
 * fixed incorrect removal of popen vram script.
 *
 * Revision 1.20  1993/01/14  16:15:35  pcharles
 * added PCI conditional
 *
 * Revision 1.19  1993/01/12  23:28:00  scott
 * added pct patch specific mode
 *
 * Revision 1.18  1992/12/22  15:11:35  rsm
 * print out a different message when BatchMode ends with no VRAM writes.
 *
 * Revision 1.17  1992/12/17  18:17:49  chris
 * ifdefed out reads when testing cmdParser in isolation
 *
 * Revision 1.16  1992/12/08  21:40:09  scott
 * added PCI config writes during batch mode
 *
 * Revision 1.15  1992/12/08  15:55:30  pcharles
 * *** empty log message ***
 *
 * Revision 1.14  1992/12/04  13:26:28  rsm
 * added pseudo-random writes of the color registers
 *
 * Revision 1.13  1992/12/03  20:35:48  rsm
 * moved around data storage definitions again
 *
 * Revision 1.12  1992/12/01  16:57:22  rsm
 * moved storage definition for some global variables to simplify
 * building an X-server with our model.
 *
 * Revision 1.11  1992/11/24  14:10:43  scott
 * added PCI batch mode conditional flag
 *
 * Revision 1.10  1992/11/17  22:53:45  rsm
 * added command to execute compressed scripts interactively
 *
 * Revision 1.9  1992/11/17  14:59:31  rsm
 * changes for running logged scripts interactively
 *
 * Revision 1.8  1992/11/17  00:32:56  pcharles
 * Added MakeIdle() after commands finish.
 *
 * Revision 1.7  1992/11/16  19:06:34  pcharles
 * Moved dynamic comparison routines external (diffram.c)
 * and added -silent option for use with said routines.
 *
 * Revision 1.6  1992/11/16  13:50:17  rsm
 * changes for 32bpp
 *
 * Revision 1.5  1992/11/13  02:33:59  rsm
 * changes for generating traces
 *
 * Revision 1.4  1992/11/12  14:50:43  scott
 * added type def void to NextLine().
 *
 * Revision 1.3  1992/11/04  15:54:19  pcharles
 * Added bob's changes to pass log file names in through -batch
 *
 * Revision 1.2  1992/10/27  15:17:19  pcharles
 * Added support for -nolog, -difflog, -dynamic vram comparison.
 *

*/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "vram.h"         /* need RAM_WIDTH/DEPTH for sfbtgen popen control */
#include "defs.h"
#include "types.h"
#include "vars.h"
#include "Module.h"

extern int vramCompareFlag;
extern FILE *vramFile;
extern int vramLine;               
extern FILE *cmdFile;
extern int cmdLine;
extern DEEPREG shadowDeep;

#ifdef RANDEVENTS
extern eepromReadCount, eepromWriteCount, 
       ramdacReadCount [RAMDACS], ramdacWriteCount [RAMDACS],
       idleCycleCount, consoleEventCount,
       portWriteCount, icsWriteCount;
extern int eventMask;
#endif

int FirstPciCommand, next_command;    
int primitiveCount = 1; /* from gram.y - # of primitives sfbtgen will gen*/
int noLog = 0;          /*               popen logs, rather than file    */
extern int silent;      /*               Silent memory comparison        */
int Seed;           	/*               random seed                     */
int planes;             /*               8 or 32-plane testing           */
#define NBUF 512
extern char    vrambuf[NBUF];
char cmdbuf[NBUF];
int randomCycles = 1;
extern int ticks;
extern int lastDinoTick;

int PCT_cmds;           /* flag to indicate batch file contains PCT cmds */
FILE *fp;

ULONG64
do_rams(
	unsigned addr,
	ULONG64 data,
	unsigned planeMask,
	unsigned byteMask,
	unsigned we_L,
	SFBDepth depth
	);

void FillMemory(
		unsigned addr,		/* Byte address		    */
		int     width,		/* width in bytes	    */
		int     height,		/* height in bytes	    */
		int     scanlineWidth,	/* # bytes to next scanline */
		int     pixel,		/* 32-bit pixel value       */
		unsigned planemask	/* 32-bit planemask	    */
		);

#include <sys/types.h>
#include <sys/stat.h>

/*
 * returns TRUE if file exists and is a regular file.
 */
PlainFile (char *name)
{
  struct stat buf;

  return stat(name, &buf) == 0 && (buf.st_mode & S_IFMT) == S_IFREG;
}

char *searchPath[] = {
  "/disks/sfb/scripts",
  "/disks/sfbScripts/scripts",
  "/disks/sfbScripts2/scripts",
  "/disks/sfbScripts3/scripts",
  "/disks/sfb/inbox",
  "/disks/sfbScripts/inbox",
  "/disks/sfbScripts2/inbox",
  "/disks/sfbScripts3/inbox",
  ".",
  NULL
  };

/*
 * looks for the file along the search path and
 * returns the path
 */
char *ScriptPath (char *name)
{
  char **p = searchPath;
  char *path = NULL;
  int dupl = 0;
  char s[256];

  while (*p) {
    sprintf (s, "%s/%s.cmds.Z", *p, name);
    if (PlainFile (s)) {
      if (path) {
	if (dupl == 0)
	  fprintf (stderr, "WARNING! duplicate script file found\n");
	fprintf (stderr, "Ignoring '%s'\n", path);
	++dupl;
      }
      path = *p;
    }
    ++p;
  }

  if (dupl)
    fprintf (stderr, "Using '%s'\n", path);

  if (path) return path;

  fprintf (stderr, "couldn't find '%s' in search path\n", name);
  exit (1);
}

OpenLogFiles(char *scriptName)
{
  char s[256];

  if (noLog|silent) {                  /* pipes to command/memory generator */
    fprintf(stderr,"Opening command pipe from sfbtgen.\n");
    sprintf(s,"sfbtgen %d %d %d %d %d cmd",
	    RAM_WIDTH,RAM_DEPTH,primitiveCount,Seed,planes);
    if ((cmdFile=popen(s,"r")) == NULL) {
      fprintf (stderr, "can't open pipe for cmds\n"); exit (1); 
    }
    fprintf(stderr,"Opening VRAM pipe from sfbtgen.\n");
    sprintf(s,"sfbtgen %d %d %d %d %d mem",
	    RAM_WIDTH,RAM_DEPTH,primitiveCount,Seed,planes);
    if ((vramFile=popen(s,"r")) == NULL) {
      fprintf (stderr, "can't open pipe for vram\n"); exit (1); 
    } 
  } else {                              /* or read from files...             */
    if (PCT_cmds == 0) {
      char *scriptDirectory = ScriptPath (scriptName);
	
      sprintf( s, "uncompress -c %s/%s.cmds.Z", scriptDirectory, scriptName );
      if ((cmdFile = popen(s, "r")) == NULL) {
	fprintf (stderr, "can't popen '%s'\n", s);
	exit (1);
      }
      sprintf( s, "uncompress -c %s/%s.vram.Z", scriptDirectory, scriptName );
      if ((vramFile = popen(s, "r")) == NULL) {
	fprintf (stderr, "can't popen '%s'\n", s);
	exit (1);
      } 
    } else {
      if ((cmdFile = fopen(scriptName, "r")) == NULL) {
	fprintf (stderr, "can't fopen '%s'\n", scriptName);
	exit (1);
      }
    }
  }
}

#ifdef RANDEVENTS
void GenerateRandomEvents(void)
{
/* insert random EEPROM accesses. */
  if(eventMask & 0x01) RandomEepromRead(14, 15);  
  if(eventMask & 0x10) RandomEepromWrite(14, 15);
  
/* insert random RAMDAC accesses. */
  if(eventMask & 0x02) RandomRamdacRead(14, 15); 
  if(eventMask & 0x20) RandomRamdacWrite(14, 15); 
  
/* insert random ||port writes.   */
  if(eventMask & 0x04) RandomPortWrite(14,15);   
  
/* insert random ics writes.      */
  if(eventMask & 0x08) RandomIcsWrite(14,15);
  
/* insert 0..3 rand clk ticks between cycles.*/
  if(eventMask & 0x40) RandomIdle(64-8, 63, 3);
  
/* insert random console message sequences. */    
  if(eventMask & 0x80) RandomConsole(254,255);
}
#endif

BatchMode(int lastLine)
{
  unsigned addr, data;
  int i;
  int HoldPCT_cmds;
  int cmd_valid, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;
  char *temp, *pct_cmd;

#ifdef PCI_BLOCK_MODE
/* setup PCI default configuration */

HoldPCT_cmds = PCT_cmds;
  PCT_cmds = 0;
  Tick();
  Tick();
  Tick();
  Tick();
  PCIConfigWrite(0x10000 + 0x4, 0x00000007, 0xf);    /* write to cmd/status */
  PCIConfigWrite(0x10000 + 0xC, 0x00000c00, 0xf);    /* write to latency tmr */
  PCT_cmds = HoldPCT_cmds;

#endif

#if 0
  if ((fp = fopen("decsim-cmd.log", "w")) == NULL) {
    fprintf (stderr, "can't open decsim-cmd.log\n");
    exit (1);
  }
#endif
  /* initialize seed for random
  srandom (1);

  /* limit the size of our corefile to 0 */
  setrlimit (RLIMIT_CORE, 0);

  FirstPciCommand = 1;

  /* initialize video registers, turn on video, cursor, etc. */
  InitVideo();

  while (ticks <= lastDinoTick && 
	 (lastLine == -1 || lastLine >= cmdLine) && GetLine()) {

#ifdef RANDEVENTS
    GenerateRandomEvents();
#endif    

#ifdef PCI_BLOCK_MODE
    if (PCT_cmds == 1) {
    /* parse line of input, test if it has a pct_ command */
      if ( (pct_cmd = strchr(cmdbuf,'p')) != 0 ) {/* get start of command */
        if ( strncmp(pct_cmd, "pct_", 4) == 0 ) { /* test if real command */
          cmd_valid = 0;			  /* init flag */

          if ( (temp = strchr(cmdbuf,'*')) != 0 ) {/* pct_cmd inside comment */
            if (pct_cmd > temp) {               /* comment starts before cmd */
              cmd_valid = 1;
              printf("PCT read: threw out: %s",cmdbuf);
	    }
          }

          /*****************************************************************/
          /***********  pct_devseldelay(A,B) *******************************/
          if ( (strncmp(&pct_cmd[4],"seldlay",7) == 0) && 
               (cmd_valid == 0) ) {
            sscanf(&pct_cmd[11],"(%i, %i, %i)", &p1, &p2, &p3);
            if (p1 !=2)
	      printf("PCT_READ: ERROR - unable to parse setdevdelay():\n\t\t%s",
		      cmdbuf);
            else
              pct_devseldelay(p2, p3);
	    cmd_valid = 1;
            }

          /*****************************************************************/
          /***********  pct_message(A) *************************************/
          /* 0 - no devsel    1 - no trdy   other - anomaly               **/
          if ( (strncmp(&pct_cmd[4],"message",7) == 0) && 
               (cmd_valid == 0) ) {
            sscanf(&pct_cmd[11],"(%i)", &p1);
            pct_message(p1);
            cmd_valid = 1;
            }

          /*****************************************************************/
          /***********  pct_default_mas(A) ***********************************/
          /* 0 - no devsel    1 - no trdy   other - anomaly               **/
          if ( (strncmp(&pct_cmd[4],"def_mas",7) == 0) && 
               (cmd_valid == 0) ) {
            sscanf(&pct_cmd[11],"(%i)", &p1);
            pct_dflt_mas(p1);
            cmd_valid = 1;
            }

          /*****************************************************************/
          /************** set data  ****************************************/
          if ( (strncmp(&pct_cmd[4],"setdata",7) == 0) && /* setdata command?*/
               (cmd_valid == 0) ) {
            sscanf(&pct_cmd[11],"(%i, %i, %i, %i)", &p1, &p2, &p3, &p4);
            if (p1 !=3)
	      printf("PCT_READ: ERROR - unable to parse setdata():\n\t\t%s",
		      cmdbuf);
            else
              pct_setdata(p1, p2, p3, p4);
	    cmd_valid = 1;
	  }

          /*****************************************************************/
          /************** set dma   ****************************************/
          if ( (strncmp(&pct_cmd[4],"set_dma",7) == 0) && /* setdata command?*/
               (cmd_valid == 0) ) {
            sscanf(&pct_cmd[11],"(%i, %i, %i, %i)", &p1, &p2, &p3, &p4);
            if (p1 !=3)
	      printf("PCT_READ: ERROR - unable to parse setdata():\n\t\t%s",
		      cmdbuf);
            else
              pct_dma_data(p1, p2, p3, p4);
	    cmd_valid = 1;
	  }

          /*****************************************************************/
          /************** set command **************************************/
          if ( (strncmp(&pct_cmd[4],"command",7) == 0) &&  /* command? */
               (cmd_valid == 0) ) {

            /* if not the first command, then kick off BusInterface stuff */
            if (FirstPciCommand == 0) {
                /* call the proper BusRead...BusWrite etc...*/
                if ((next_command == 0x03) || (next_command == 0x07) || 
                    (next_command == 0x0F)) 
                   BusWrite(0,0,0);
                if ((next_command == 0x02) || (next_command == 0x06) || 
                    (next_command == 0x0C) || (next_command == 0x0E)) 
                   BusRead(0);

                if (next_command == 0xA) PCIConfigRead(0);
                if (next_command == 0xB) PCIConfigWrite(0,0,0);

                /* send the reserved commands as Buswrites...the module
                   should never acknowledge...                          */
                if ((next_command == 0x0) || (next_command == 0x1) ||
                    (next_command == 0x4) || (next_command == 0x5) ||
                    (next_command == 0x8) || (next_command == 0x9) ||
                    (next_command == 0xD))
                            BusWrite(0,0,0);

               }
            FirstPciCommand = 0;   /** reset flag for first command **/


            /*****************************************************************/
            /*   check for correct number arguments                          */
            sscanf(&pct_cmd[11],"(%i, %i, %i, %i, %i, %i, %i)",
                                &p1, &p2, &p3, &p4, &p5, &p6, &p7);

               /*********************************************************/
               /* store the command type associated with this command   */
               /* IACK	     0x00	Interrupt acknowledge   ignore  */
               /* SPCYC      0x01       Special cycle           ignore  */
               /* IORD       0x02       I/O Read cycle          read    */
               /* IOWR       0x03       I/O write cycle         write   */
               /* RESV1      0x04       Reserved                ignore  */
               /* RESV2      0x05       Reserved                ignore  */
               /* MEMRD      0x06       Memory read             read    */
               /* MEMWR      0x07       Memory write            write   */
               /* RESV3      0x08       Reserved                ignore  */
               /* RESV4      0x09       Reserved                ignore  */ 
               /* CNFRD      0x0A       Configuration read      read    */
               /* CNFWR      0x0B       Configuration write     write   */
               /* MEMRDM     0x0C       Memory read multiple    read    */
               /* RESV5      0x0D       Reserved                ignore  */ 
               /* MEMRDL     0x0E       Memory read line        read    */
               /* MEMWRI     0x0F       Memory write/invalidate write   */
               /*********************************************************/

            switch (p1) {
               case 1: pct_command(p1,p2);
                       next_command = 0xFF;
                       break;
               case 6:  /* printf("heres command %d %d %x %x %d %d %d \n",
                                                        p1,p2,p3,p4,p5,p6,p7); */
                       pct_command(p1, p2, p3, p4, p5, p6, p7);
                       next_command = p4;
                       break;
               default: printf("PCT_READ: ERROR - unable to parse command():\n\t\t%s",cmdbuf);
                        break;
                       }
            cmd_valid = 1;
           }

          /*****************************************************************/
          /*************** settrdy *****************************************/
          if ( (strncmp(&pct_cmd[4],"settrdy",7) == 0) && (cmd_valid == 0) ) 
             {
              sscanf(&pct_cmd[11],"(%x, %x, %x, %x, %x, %x, %x, %x, %x, %x)",
                         &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10);
              switch (p1) {
                case 3: pct_settrdy(p1, p2, p3, p4);
                        break;
                case 4: pct_settrdy(p1, p2, p3, p4, p5);
                        break;
                case 8: pct_settrdy(p1, p2, p3, p4, p5, p6, p7, p8, p9);
                        break;
                default: printf("PCT_READ: ERROR - unable to parse settrdy():\n\t\t%s",cmdbuf);
                         break;
                }
	      cmd_valid = 1;
             }

          /*****************************************************************/
          /*************** setirdy *****************************************/
          if ( (strncmp(&pct_cmd[4],"setirdy",7) == 0) && (cmd_valid == 0) ) 
             {
              sscanf(&pct_cmd[11],"(%x, %x, %x, %x )", &p1, &p2, &p3, &p4);
              if (p1 != 3)
   	         printf("PCT_READ: ERROR - unable to parse setirdy():\n\t\t%s",
	            cmdbuf);
               else
                 pct_setirdy(p1, p2, p3, p4);
	      cmd_valid = 1;
             }

          /*****************************************************************/
          /*************** setbyte *****************************************/
          if ( (strncmp(&pct_cmd[4],"setbyte",7) == 0) && (cmd_valid == 0) ) 
             {
              sscanf(&pct_cmd[11],"(%x, %x, %x, %x )", &p1, &p2, &p3, &p4);
              if (p1 != 3)
   	         printf("PCT_READ: ERROR - unable to parse setirdy():\n\t\t%s",
	            cmdbuf);
               else
                 pct_setbyte(p1, p2, p3, p4);
	      cmd_valid = 1;
             }


          if (cmd_valid == 0)
            printf("PCT_READ: ERROR - unknown pct_command:\n\t\t%s", pct_cmd);
        }
      } 
    }/* end of PCT parse test */
#endif
    if (PCT_cmds == 0) {
      if (cmdbuf[0] == '\n') {
        if (fp) fprintf(fp, "tick\n");
        Tick();
      } else if (cmdbuf[0] == '0') {
        /*
         * read cycle.
         */
        unsigned rd;

        if (sscanf(cmdbuf, "0 %x %x", &addr, &data) != 2) {
  	  if (sscanf(cmdbuf, "0 %x", &addr) != 1) {
	    fprintf (stderr, "can't parse read command '%s'\n", cmdbuf);
	    fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
		     cmdLine, vramLine);
	    exit (1);
	  } else {
	    rd = BusRead(addr);
	  }
        } else {
	  static unsigned copyBufferReadMask = 0;

	  rd = BusRead(addr);
#ifndef CMD_PARSER_ONLY
	  if (rd != data) {
	    extern int ticks;

	    if ((copyBufferReadMask & addr & 0x1c) == 0
		&& (addr == CPYBF0_ADDRESS || addr == CPYBF1_ADDRESS)) {
	      fprintf (stderr,
		       "**** copybuffer-init mismatch: cmdLine = %d, vramLine = %d\n",
		       cmdLine, vramLine);
	      fprintf (stderr, "**** clock tick %d\n", ticks);
	      fprintf (stderr, "**** addr = %x, expected = %x, received = %x\n",
		       addr, data, rd);
	    } else {
	      fprintf (stderr,
		       "**** incorrect readdata: cmdLine = %d, vramLine = %d\n",
		       cmdLine, vramLine);
	      fprintf (stderr, "**** clock tick %d\n", ticks);
	      fprintf (stderr, "**** addr = %x, expected = %x, received = %x\n",
		       addr, data, rd);
	      exit (1);
	    }
	  }
	  if (addr >= CPYBF0_ADDRESS && addr <= CPYBF7_ADDRESS)
	    copyBufferReadMask |= addr & 0x1c;
#endif
        }
      } else if (cmdbuf[0] == '1') {
        /*
         * write cycle.
         */
        unsigned mask, tmpData, tmpMask;
        int nwords;
#ifndef PCI_BLOCK_MODE
        struct {
	  unsigned addr;
	  unsigned data[8];
	  unsigned mask;
        } block;
#else
        int tmpAddr;
        struct {
	  unsigned addr;
	  unsigned data[10];
	  unsigned mask[10];
        } block;
#endif

        if (sscanf(cmdbuf, "1 %x %x %x", &addr, &data, &mask) != 3) {
	  fprintf (stderr, "can't parse write command '%s'\n", cmdbuf);
	  fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
		   cmdLine, vramLine);
	  exit (1);
        }
#ifndef PCI_BLOCK_MODE

        nwords = 1;
        block.addr = addr;
        block.data [(addr>>2) & 7] = data;
        block.mask = 1 << ((addr>>2) & 7);

        if (mask == 0xf) {
	  while (GetLine()) {
	    if (cmdbuf[0] != '1') {
	      UngetLine();
	      break;
	    } else 
	      if (sscanf(cmdbuf, "1 %x %x %x",&addr,&tmpData,&tmpMask)!=3) {
	      fprintf (stderr, "can't parse write command '%s'\n", cmdbuf);
	      fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
		       cmdLine, vramLine);
	      exit (1);
	    }
	    if (tmpMask == 0xf
	        && addr == block.addr+4
	        && (addr>>5) == (block.addr>>5)) {
	      block.addr = addr;
	      block.data [(addr>>2) & 7] = tmpData;
	      block.mask |= 1 << ((addr>>2) & 7);
	      ++nwords;
	    } else {
	      UngetLine();
	      break;
	    }
	  }
        }

        if (nwords > 1) {
	  BusWriteBlock (block.addr, block.data, block.mask);
        } else {
	  BusWrite (block.addr, data, mask);
        }

#else  /* do PCI linear address mode */

        nwords = 1;
        block.addr = addr;
        block.data [0] = data;
        block.mask [0] = mask;

	while (GetLine()) {
	  if (cmdbuf[0] != '1') {
	    UngetLine();
	    break;
	  } else if (sscanf(cmdbuf, "1 %x %x %x",&tmpAddr,&tmpData,&tmpMask)!=3) {
	    fprintf (stderr, "can't parse write command '%s'\n", cmdbuf);
	    fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
		     cmdLine, vramLine);
	    exit (1);
	  }
	  if (addr+4 == tmpAddr) {
	    addr = tmpAddr;
	    block.data [nwords] = tmpData;
	    block.mask [nwords] = tmpMask;
	    ++nwords;
	  } else {
	    UngetLine();
	    break;
	  }
	}

        if (nwords > 1) {
	  BusWriteBlock (block.addr, block.data, block.mask, nwords);
        } else {
	  BusWrite (block.addr, data, mask);
        }
#endif

      } else if (cmdbuf[0] == '2') {
        /*
         * fill memory.
         */
        unsigned width, height, scanlinewidth, pixel, planemask, n;

	n = sscanf(cmdbuf, "2 %x %x %x %x %x %x",
	  	   &addr, &width, &height, &scanlinewidth, &pixel, &planemask);
	if (n == 5) {
	  extern unsigned shadowPlanemask;
	  planemask = shadowPlanemask;
	} else if (n != 6) {
	  fprintf (stderr, "can't parse fill command '%s'\n", cmdbuf);
	  fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
		   cmdLine, vramLine);
	  exit (1);
        }

	/*
	 * mask these address just like other framebuffer addresses.
	 */
	addr &= (shadowDeep.reg.addrMask << 22) | 0x3fffff;

        FillMemory (addr, width, height, scanlinewidth, pixel, planemask);
      } else if (cmdbuf[0] == '3') {
	/*
	 * memory size declaration.
	 */
        unsigned bytes;

	if (cmdLine != 1) {
	  fprintf (stderr, "memory size command can only occur on the first line\n");
	  fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
		   cmdLine, vramLine);
	  exit (1);
	} else if (sscanf(cmdbuf, "3 %x", &bytes) != 1) {
	  fprintf (stderr, "can't parse memory size command '%s'\n", cmdbuf);
	  fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
		   cmdLine, vramLine);
	  exit (1);
	}

	if (bytes != 0x200000 && bytes != 0x800000)
	  fprintf (stderr, "WARNING! this trace is requesting %x bytes\n", bytes);
	InitVRAM (bytes);
      } 
      else if (cmdbuf[0] == '4') Tick();
      else if (cmdbuf[0] == '5') MakeIdle();
      else {
	fprintf (stderr, "unrecognized command line '%s'\n", cmdbuf);
	fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
		 cmdLine, vramLine);
	exit (1);
      }
    }
  }
  MakeIdle();
  if (vramLine == 0)
      fprintf (stderr, "**** NO VRAM ACTIVITY: ");
  else
    fprintf (stderr, "**** Finished: ");
  fprintf (stderr, "cmdLine = %d, vramLine = %d, ticks = %d\n",
	   cmdLine, vramLine, ticks); 
  
#ifdef RANDEVENTS
  { 
    int i;

    if(eepromReadCount || eepromWriteCount || 
       ramdacReadCount[0] || ramdacReadCount[1] || 
       ramdacWriteCount[0] || ramdacWriteCount[1] || idleCycleCount) {
      fprintf (stderr, "**** Including pseudorandom asynchronous events:\n");
      fprintf (stderr, "     EEPROM reads = %d, writes = %d\n",
	       eepromReadCount, eepromWriteCount); 
      for(i=0; i<RAMDACS; i++) 
	fprintf (stderr, "   RAMDAC%d reads = %d, writes = %d\n",
		 i, ramdacReadCount [i], ramdacWriteCount [i]);
      fprintf (stderr, "     Idle = %d\n", idleCycleCount);
      fprintf (stderr, "     Console = %d\n", consoleEventCount);
      fprintf (stderr, "     Parallel Port write = %d\n", portWriteCount);
    }
  }
#endif

  if (cmdFile) fclose (cmdFile);
  if (vramFile) fclose (vramFile);
  cmdFile = vramFile = NULL;
}

int
FixLine ()
{
  do {
    if (fgets (cmdbuf, NBUF, cmdFile) == NULL)
      return 0;
    if (cmdbuf [0] == '\n') ++cmdLine;
  } while (cmdbuf [0] == '\n');

  if (cmdbuf [strlen(cmdbuf) - 1] != '\n') {
    if (feof (cmdFile)) {
      fprintf (stderr, "cmd file appears truncated: '%s'\n", cmdbuf);
      return 0;
    }
    fprintf (stderr, "cmd file appears corrupted: '%s'\n", cmdbuf);
    fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
    exit (1);
  }

  if (cmdbuf[0] == '1') {
    unsigned addr, data, mask;

    if (sscanf(cmdbuf, "1 %x %x %x", &addr, &data, &mask) == 3) {
      static ROPREG shadowRop;

      /*
       * Fix virtual addresses that do not actually point to FB space
       */
      if (addr >= 0x200000) {     /* Must be a frame buffer address */
        if ((addr & (shadowDeep.reg.addrMask << 22)) < 0x200000)
          addr |= 0x200000;
#ifdef PCI_BLOCK_MODE
          addr &= 0x1ffffff;
#endif
      }

      /*
       * Chris changed the addresses of some registers, but we haven't
       * modified the traces yet.  Fix them here for the time being.
       */
      switch (addr) {
      case 0x1000c0:	/*COPY64SRC*/
	addr = 0x100160; break;
      case 0x1000c4:	/*COPY64DST*/
	addr = 0x100164; break;
      case 0x1000c8:	/*COPY64SRC2*/
	addr = 0x100168; break;
      case 0x1000cc:	/*COPY64DST2*/
	addr = 0x10016c; break;
      case 0x1000e0:	/*BLOCKCOLOR0*/
	addr = 0x100140; break;
      case 0x1000e4:	/*BLOCKCOLOR1*/
	addr = 0x100144; break;
      case 0x1000e8:	/*BLOCKCOLOR2*/
	addr = 0x100148; break;
      case 0x1000ec:	/*BLOCKCOLOR3*/
	addr = 0x10014c; break;
      case 0x1000f0:	/*BLOCKCOLOR4*/
	addr = 0x100150; break;
      case 0x1000f4:	/*BLOCKCOLOR5*/
	addr = 0x100154; break;
      case 0x1000f8:	/*BLOCKCOLOR6*/
	addr = 0x100158; break;
      case 0x1000fc:	/*BLOCKCOLOR7*/
	/*???bogus dma high address register???*/
	addr = 0x10015c; break;
      case MODE_ADDRESS:
	{
	  MODEREG m;
	  
	  m.u32 = data;
	  if (m.reg.mode == 9) {
	    /* transparent block fill used to be 9, now it's 2d */
	    m.reg.mode = 0x2d;
	    data = m.u32;
	  }
	  break;
	}
      case ROP_ADDRESS:
	shadowRop.u32 = data;
	break;
      case PLANEMASK_ADDRESS:
	if (shadowRop.reg.visual == VISUAL8U) {
	  unsigned m = data & 0xff;
	  m |= m << 8;
	  m |= m << 16;
	  if (data != m) {
	    fprintf (stderr, "WARNING! unsupported planemask value (%-.8x) for\n", data);
	    fprintf (stderr, "8-bit unpacked visual.  using %-.8x instead.\n", m);
	    data = m;
	  }
	}
	break;
      default:
	break;
      }
      
      sprintf (cmdbuf, "1 %x %x %x", addr, data, mask);
    }
  }

  return 1;
}

static int ungot;

GetLine()
{
  if (ungot) {
    ungot = 0;
    ++cmdLine;
    return (1);
  } else {
    ++cmdLine;
    return FixLine();
  }
}

UngetLine()
{
  --cmdLine;
  ungot = 1;
}


/*
 * FillMemory implementation courtesy of Joel McCormack.
 *
 * this routine is invoked by a single command in the trace file.
 * it serves to dramatically reduce the size of the trace files.
 */
#define SFBCOPYALIGNMENT    8
#define SFBCOPYALIGNMASK    (SFBCOPYALIGNMENT-1)
#define ALL1		0xff

void FillMemory(
		unsigned addr,		/* Byte address		    */
		int     width,		/* width in pixels	    */
		int     height,		/* height in scanlines	    */
		int     scanlineWidth,	/* # bytes to next scanline */
		int     pixel,		/* 32-bit pixel value       */
		unsigned planemask	/* 32-bit planemask	    */
		)
{
    extern FILE *cmdOut;
    extern int  logFlag;
    ULONG64     data;
    unsigned    p, a;
    unsigned    leftMask, rightMask, mask;
    int		savevramCompareFlag;
    int		saveLogFlag;
    int		m, align, w;
    int		SFBPIXELBYTES;
    unsigned int depth32;

    MakeIdle();
    if (logFlag)
      fprintf (cmdOut, "2 %x %x %x %x %x %x\n",
	       addr, width, height, scanlineWidth, pixel, planemask);

    depth32 = shadowDeep.reg.deep;

    data.lo = pixel;
    data.hi = pixel;
    saveLogFlag = logFlag;
    logFlag = FALSE;
    savevramCompareFlag = vramCompareFlag;
    vramCompareFlag = FALSE;
    if (depth32) width *= 4;

    do {
	/* Recompute alignment each time in case scanlineWidth mod 8 = 4 */
	align = addr & SFBCOPYALIGNMASK;
	a = addr - align;
	w = width + align;
	leftMask = (ALL1 << align) & ALL1;
	rightMask = ALL1 >> ((-w) & SFBCOPYALIGNMASK);
    
	if (w <= SFBCOPYALIGNMENT) {
	    mask = leftMask & rightMask;
	    do_rams(a/SFBCOPYALIGNMENT, data, planemask, mask, 0, depth32);
	} else {
	    p = a/SFBCOPYALIGNMENT;
	    do_rams(p, data, planemask, leftMask, 0, depth32);
	    for (m = w - 2*SFBCOPYALIGNMENT; m > 0; m -= SFBCOPYALIGNMENT) {
		p += 1;
		do_rams(p, data, planemask, ALL1, 0, depth32);
	    }
	    do_rams(p+1, data, planemask, rightMask, 0, depth32);
	}
	addr += scanlineWidth;
	height--;
    } while (height > 0);

    logFlag = saveLogFlag;
    vramCompareFlag = savevramCompareFlag;
}

/*
 * initialize video timing registers, turn on video and cursor.
 */
InitVideo()
{
  CURSORXYREG cursorxy;
  HORIZONTAL hcontrol;
  VERTICAL vcontrol;
  CURSORBASEREG cursorbase;
  unsigned vbase;

#ifdef M1284x1024
  hcontrol.u32        = 0;
  hcontrol.reg.active = 321;
  hcontrol.reg.fp     = 8;
  hcontrol.reg.sync   = 40;
  hcontrol.reg.bp     = 58;
  hcontrol.reg.odd    = 0;
#else
  hcontrol.u32        = 0;
  hcontrol.reg.active = 101;
  hcontrol.reg.fp     = 3;
  hcontrol.reg.sync   = 4;
  hcontrol.reg.bp     = 4;
  hcontrol.reg.odd    = 1;
#endif

  vcontrol.u32        = 0;
  vcontrol.reg.active = 100;
  vcontrol.reg.fp     = 3;
  vcontrol.reg.sync   = 3;
  vcontrol.reg.bp     = 3;

  cursorxy.u32   = 0;
#if 0
  /* position the cursor to the left of the screen, so it's not visible */
  cursorxy.reg.x = (hcontrol.reg.sync + hcontrol.reg.bp)*4 - 100;
#else
  cursorxy.reg.x = (hcontrol.reg.sync + hcontrol.reg.bp)*4 + 100;
#endif
  cursorxy.reg.y = (vcontrol.reg.sync + vcontrol.reg.bp) + 7;

  cursorbase.u32              = 0;
  cursorbase.reg.base         = 0;
  cursorbase.reg.rowsMinusOne = 64-1;

  vbase = 1;

#if 0
  hcontrol.u32 = 0x410460;
  vcontrol.u32 = 0x80821060;
  cursorxy.u32 = 0x000050C8;
  cursorbase.u32 = 0xFC00;
#endif

  BusWrite (CURSORXY, cursorxy.u32, 0xf);
  BusWrite (HCONTROL, hcontrol.u32, 0xf);
  BusWrite (VCONTROL, vcontrol.u32, 0xf);
  BusWrite (VIDEOVALID, 5, 0xf);
  BusWrite (CURSORBASE, cursorbase.u32, 0xf);
  BusWrite (VIDEOBASE, vbase, 0xf);
}
