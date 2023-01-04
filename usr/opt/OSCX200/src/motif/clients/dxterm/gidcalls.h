/* #module gidcalls.h "X0.0" */
/*
 *  Title:	gidcalls.h
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
 *	<short description of module contents>
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	<original author>
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 */

/**
*  The ReGIS and GID_PROCESS interface is:
*
*     GID_PROCESS(.data[.n], ..., .data[2], .data[1], .n)
*
*  where GIDIS words are supplied in reverse order.
* 
*  MODIFICATION:
*
*  20-Aug-1987	Bob Messenger
*  Strip out macros for procedure calls
*
*  001 Converted origional code into C.		\RFD
*
* Gidis v2   Andrew F. Vesper
*  30 June 1983
*
**/

		/* flag to say variable length argument list		     */
#define	TILL_END_LIST	255	
		/* amount to shift op-code byte for op-code word 	     */
#define	OP_CODE_SHIFT	8

#define		G17_SPECIAL_ALPHABET 	-1
#define		G18_NO_CURSOR		-1
#define		G19_DEFAULT_CURSOR	0
#define		G20_TRACKING_CROSS	1
#define		G21_CROSS_HAIR		2
#define		G22_RUBBER_LINE		3
#define		G23_RUBBER_RECTANGLE 	4

#define		G38_MODE_TRANSPARENT		0
#define		G39_MODE_TRANSPARENT_NEGATE	1
#define		G40_MODE_COMPLEMENT 		2
#define		G41_MODE_COMPLEMENT_NEGATE	3
#define		G42_MODE_OVERLAY		4
#define		G43_MODE_OVERLAY_NEGATE		5
#define		G44_MODE_REPLACE		6
#define		G45_MODE_REPLACE_NEGATE		7
#define		G46_MODE_ERASE			8
#define		G47_MODE_ERASE_NEGATE		9

/* values for rs->sc_current_opcode.  Must be unique and non-zero */

#define		G58_OP_DRAW_LINES		1
#define		G64_OP_BEGIN_FILLED_FIGURE	2
#define		G94_OP_DRAW_CHARACTERS		3

#define		G78_EXPLICIT_LOCAL	0
#define		G79_EXPLICIT_GLOBAL	1
#define		G80_IMPLICIT_LOCAL	2
#define		G81_IMPLICIT_GLOBAL	3

#define		G84_REND_REVERSE_ITALIC		1
#define		G85_REND_ITALIC			2
#define		G86_REND_UNDERLINE		4
#define		G87_REND_BOLD			8
#define		G88_REND_H_REFLECTION		16
#define		G89_REND_V_REFLECTION		32

#define		G102_STATUS_SUCCESS	1
#define		G103_STATUS_FAILURE	0

/* REPORT tags */

#define		G109_REPORT_CURRENT_POSITION	((1 << OP_CODE_SHIFT) + 2)
#define		G110_REPORT_OUTPUT_SIZE		((2 << OP_CODE_SHIFT) + 9)
#define		G111_REPORT_INPUT_SIZE		((3 << OP_CODE_SHIFT) + 9)
#define		G112_REPORT_STATUS		((4 << OP_CODE_SHIFT) + 1)
#define		G113_REPORT_CELL_STANDARD	((5 << OP_CODE_SHIFT) + 4)
#define		G114_REPORT_INPUT		((6 << OP_CODE_SHIFT) + 3)
#define		G115_REPORT_DEVICE_CHAR		((7 << OP_CODE_SHIFT) + 0)

/* INPUT COMMANDS */

#define		G117_TRIGGER_CLOCK		1
#define		G118_TRIGGER_STATE_CHANGE 	2
#define		G119_TRIGGER_BUTTON		3
#define		G120_TRIGGER_NONE			4

#define		G125_DEVICE_OFFLINE		-1
#define		G126_INPUT_WAS_REQUESTED 		-2
