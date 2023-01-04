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
/*   $RCSfile: WmResParse.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:24:28 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#include <stdio.h>

#ifdef _NO_PROTO

extern void ProcessWmFile ();
extern void ProcessCommandLine ();
extern void ProcessMotifBindings ();
#ifdef DEC_MOTIF_EXTENSION
extern void WmConfigFileChec();
#endif
extern FILE          * FopenConfigFile ();
extern void            FreeMenuItem ();
extern unsigned char * GetNextLine ();
extern unsigned char * GetString ();

extern Boolean          ParseBtnEvent ();
extern Boolean          ParseKeyEvent ();
extern void            ParseButtonStr ();
extern void            ParseKeyStr ();
extern MenuItem      * ParseMwmMenuStr ();
 
extern int             ParseWmFunction ();
extern void            ProcessWmFile ();
extern void            PWarning ();
#ifdef DEC_MOTIF_EXTENSION
extern void            PWarningStr ();
#endif
extern void            SaveMenuAccelerators ();
extern void             ScanAlphanumeric ();
extern void            ScanWhitespace ();
#ifndef VMS
extern void            ToLower ();
#else
extern void            ToLower2 ();	/* otherwise conflicts with tolower with lower case "t" */
#endif
extern void		SyncModifierStrings();

#else /* _NO_PROTO */
       
extern void ProcessWmFile (WmScreenData *pSD);
extern void ProcessCommandLine (int argc,  char *argv[]);
extern void ProcessMotifBindings (void);
#ifdef DEC_MOTIF_EXTENSION
extern void WmConfigFileChec( char *fileName );
#endif /* DEC_MOTIF_EXTENSION */
extern FILE          * FopenConfigFile (void);
extern void            FreeMenuItem (MenuItem *menuItem);
extern unsigned char * GetNextLine (void);
extern unsigned char * GetString (unsigned char **linePP);
extern Boolean ParseBtnEvent (unsigned char  **linePP,
                              unsigned int *eventType,
                              unsigned int *button,
                              unsigned int *state,
                              Boolean      *fClick);

extern void            ParseButtonStr (WmScreenData *pSD, unsigned char *buttonStr);
extern void            ParseKeyStr (WmScreenData *pSD, unsigned char *keyStr);
extern Boolean ParseKeyEvent (unsigned char **linePP, unsigned int *eventType,
		       KeyCode *keyCode,  unsigned int *state);
extern MenuItem      * ParseMwmMenuStr (WmScreenData *pSD, unsigned char *menuStr);
extern int             ParseWmFunction (unsigned char **linePP, unsigned int res_spec, WmFunction *pWmFunction);
extern void            ProcessWmFile (WmScreenData *pSD);
extern void            PWarning (char *message);
#ifdef DEC_MOTIF_EXTENSION
extern void            PWarningStr( char *message, char *string );
#endif
extern void            SaveMenuAccelerators (WmScreenData *pSD, MenuSpec *newMenuSpec);
extern void            ScanAlphanumeric (unsigned char **linePP);
extern void            ScanWhitespace(unsigned char  **linePP);
#ifndef VMS
extern void            ToLower (unsigned char  *string);
#else 
/* otherwise conflicts with tolower with lower case "t" */
extern void            ToLower2( unsigned char *string );	
#endif                        
extern void		SyncModifierStrings(void);
#endif /* _NO_PROTO */
