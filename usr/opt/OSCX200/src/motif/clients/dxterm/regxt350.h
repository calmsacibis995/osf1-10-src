/* #module regxt350.h "X0.0" */
/*
 *  Title:	regxt350.h
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988, 1993                                                 |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
 *  | All Rights Reserved.                                                   |
 *  |                                                                        |
 *  | This software is furnished under a license and may be used and  copied |
 *  | only  in  accordance  with  the  terms  of  such  license and with the |
 *  | inclusion of the above copyright notice.  This software or  any  other |
 *  | copies  thereof may not be provided or otherwise made available to any |
 *  | other person.  No title to and ownership of  the  software  is  hereby |
 *  | transfered.                                                            |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
*
*	REGXT350.REQ		include file containing device-dependent 
*				literal declarations that customize the
*				REGIS parser to the XT100 environment 
*
*	Edit history:
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
*  Aston Chan		17-Dec-1991	V3.1
*      I18n code merge
*
*  Bob Messenger        17-Jul-1990     X3.0-5
*      Merge in Toshi Tanimoto's changes to support Asian terminals -
*	- max cell height/width of Kanji
*	- variable cell storage size support
*
* Bob Messenger		 7-Apr-1989	X2.0-6
*	- Removed TOTAL4_SIMULTANEOUS_COLORS, since the number of bit planes
*	  (and hence colors) is now variable.
*
* Bob Messenger		21-Mar-1989	X2.0-3
*	- Use macrographReportEnable resource to determine whether
*	  macrograph reporting is enabled.
*
* 015   12-Sep-84	/RDM	Enable RPRT_POSITION_INTERACTIVE
*
* 014	??-???-83	/RFD 	Translate origional Bliss into C.
*
* 013	25-Aug-83	/AFV	Change linkage to SCAN to a 
*				single parameter (contained in R1)
*				which is the pointer to an ASCIZ
*				string
*
* 012	23-Aug-83	/AFV	Added macro ADJUST_45_TEXT_Y 
*				to account for the 2.5:1 
*				pixel aspect ratio on PRO
*
* 011	23-Aug-83	/AFV	Change linkages to getl_regis, 
*				puts_regis and INCHQ_UNQ
*
* 010	19-Aug-83	/AFV	Add parameter MAX_ERRORS_ALLOWED
*
* 009	18-Aug-83	/AFV	Add parameter ASK_FOR_TEXT_EXCAPEMENT
*
* 008			/AFV	Change to support Gidis v2
*
* 007	6-Apr-83	/DCL	Implement macrograph report lockout hooks 
*				(macrograph report is always enabled on
*				XT100).  Also rip out max_skip_paren_count; 
*				it's no longer needed.
*
* 006	23-Mar-83	/DCL	Enable alternate monitor support (i.e. "A" 
*				color specifier handling)
*
* 005	3-Mar-83	/DCL	Add MAX6_TEXT_STANDARD_SIZE, 
*				MAX5_TEXT_HEIGHT_MULTIPLIER, and
*				MAX4_TEXT_CELL_MULTIPLIER 
*
* 004	20-Jan-83	/DCL	Add RPRT_POSITION_INTERACTIVE
*
* 003	14-Dec-82	/DCL	Add REVERSE_PATTERN_REGISTER
*
* 002	4-Nov-82	/DCL	Add OVERLAY_STYLE
*
* 001	15-Oct-82	/DCL	Original code stolen and consolidated from
*				REGIS.BLI and REGBLK.REQ; also add 
*				max_skip_paren_count
**/

#ifndef H_REGXT350_H
#define H_REGXT350_H 0

#define		GID_PASSWORD	9005

/**
*
*	Product dependent definitions:
*
*	There are four types of product dependencies in the parser:  hardware
*	dependencies, operating system dependencies, capacity dependencies, 
*	and feature switches.
*
**/
/**
*
*	Hardware dependencies:
*
**/

#define	ALTERNATE_MONITOR	 TRUE	/* Alternate monitor support */
#define	X_HARDWARE_RESOLUTION	 960	/* Logical resolution... */
#define	Y_HARDWARE_RESOLUTION	 600	/* ...of the device */
#define	REVERSE_PATTERN_REGISTER TRUE
					/* Displayed pattern is mirror image of */
					/*  stored pattern, i.e. low bit first */
#define	ASYNCHRONOUS_COLOR_MAP	 FALSE	/*  Regis is the only thing that can */
					/*  change the color map.  if set to */
					/*  TRUE there must be a routine  */
					/*  'update_color_map()'. */
#define	SCREEN_RESOLUTION_X     ( X_HARDWARE_RESOLUTION )
#define	ADJUST_45_TEXT_Y(y)	( (y) + ((y)/4) )

/**
*
*	Operating system/software environment dependencies:
*
**/

#define	GID_RPRT_ROUTINE	  TRUE	/* Is GID_REPORT, the GIDIS report */
					/*  mechanism, an external routine */
					/*  (TRUE) or a macro (FALSE)? */
#define	OVERLAY_STYLE		     0	/* 0 = No overlaying */
					/* 1 = All mapping code resides in a */
					/*     separate module (XT100 style) */
#define MACROGRAPH_RPRT_OK(dummy)  (rs->widget->common.macrographReportEnable)

/**
*
*	Capacity dependencies:
*
**/
#define	H_ALPH_CELL_HEIGHT_DEFAULT  10	/* Alphabet 0 cell storage height */
#define	W_ALPH_CELL_WIDTH_DEFAULT   8	/* Alphabet 0 cell storage width */
#define	ALPH_EXTENT_DEFAULT	    96	/* Alphabet 0 extent */
#define	MAX1_ALPH_CELL_HEIGHT	    10	/* Maximum cell height */
#define	MAX2_ALPH_CELL_WIDTH	    8	/* Maximum cell width */
#define	MAX1_ALPH_MAX_CELL_HEIGHT   24	/* Maximum cell height (max) */
#define	MAX2_ALPH_MAX_CELL_WIDTH    11	/* Maximum cell width (max) */
#define	MAX1_ALPH_WIDE_CELL_HEIGHT  24	/* Maximum cell height (wide) */
#define	MAX2_ALPH_WIDE_CELL_WIDTH   11	/* Maximum cell width (wide) */
#define	MAX3_ALPH_EXTENT		   191	/* Maximum alphabet extent */
#define	MAX4_TEXT_CELL_MULTIPLIER    16	/* Maximum T(M[n,n]) */
#define	MAX5_TEXT_HEIGHT_MULTIPLIER  16	/* Maximum T(Hn) */
#define	MAX6_TEXT_STANDARD_SIZE      16	/* Maximum T(Sn) */
#define	TOTAL1_CHARS_PER_ALPH_NAME   10	/* Alphabet name length */
#define	TOTAL2_MACROGRAPH_MEMORY   5026	/* Number of bytes for macrographs */
#define	TOTAL3_NUMBER_OF_ALPHABETS   4	/* Number of alphabets (including */
					/*  the zeroth, or permanent) */
#define	TOTAL_NUMBER_OF_MACROGRAPHS 26	/* Number of loadable macrographs */
#define	SIZE_OF_COLOR_SPECIFIER      3	/* Number of bits in 1 color specifier */
					/*  used to normalize color specifiers */
#define	POSITION_STACK_SIZE	    16	/* Number of locations in the stack */
#define	NCM_NUMBER_COLOR_MAPS	     1	/* Total number of color maps: */
					/*  primary = color + mono  */
					/*  alternate 0 = color part of primary */
					/*  other alternates = color + mono */
/**
*
*	Feature switches:
*
**/
#define	DEBUG_ENABLED		  FALSE	/* Debugging switch */
#define	DIRECT_GID_ENABLED	  FALSE	/* G instruction support */
#define	ERROR_RPRT_ENABLED        TRUE	/* R(E) support */
#define	RPRT_POSITION_INTERACTIVE TRUE
					/* R(P(I)) support */
#define	VARIABLE_CELL_STORAGE_SIZE TRUE /* L[,] support */
#define	SEPARATE_CURRENT_POSITION TRUE /* Regis & gidis never have separate */
					/*  current positions.  change to TRUE */
					/*  gidis does curves, arcs, etc. and */
					/*  regis does not know where c.p. ends */
#define	ASK_FOR_TEXT_ESCAPEMENT  FALSE  /* Should Regis request the text */
					/*  escapement from Gidis (via implicit */
					/*  movement and transparent mode)? */
#define	MAX_ERRORS_ALLOWED           2	/* Number of erroneous Gidis reports */
					/*  we should throw away before giving */
					/*  up. */
/**
*	End of include file REGXT350.H
**/
/**
*	CMS REPLACEMENT HISTORY
**/
/**
*	*1 A_VESPER 14-SEP-1983 14:03:11 ""
**/

#endif
