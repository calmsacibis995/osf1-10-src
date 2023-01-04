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
static char *rcsid = "@(#)$RCSfile: smodel.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:43:50 $";
#endif
#include <stdio.h>

main(int argc, char **argv)
{
  extern int vramCompareFlag;

  WrapperInit();

  if (argc >= 2) {
    OpenPipes(*++argv);
  } else {
    OpenPipes("/disks/sfb/scripts/dwtetris");
  }
  vramCompareFlag = 1;
  BatchMode(-1);
}

OpenPipes(char *scriptName)
{
  extern FILE *cmdFile, *vramFile;
  char s[256];

  sprintf( s, "uncompress -c %s.cmds.Z", scriptName );
  if ((cmdFile = popen(s, "r")) == NULL) {
    fprintf (stderr, "can't popen '%s'\n", s);
    exit (1);
  }
  sprintf( s, "uncompress -c %s.vram.Z", scriptName );
  if ((vramFile = popen(s, "r")) == NULL) {
    fprintf (stderr, "can't popen '%s'\n", s);
    exit (1);
  }
}

/*
 * this just defines the name "History".
 * it will never be called as long as "historyFlag" is 0.
 */
History()
{
}

#define NOEVENTS 0
int ticks;

Tick( )
{
  OneCycle( NOEVENTS );
  ++ticks;
}

/*
 * this just defines "DebugInit" to be a null procedure.
 * the interactive simulator uses "DebugInit" to initialize
 * symbol tables.
 */
DebugInit()
{
}

