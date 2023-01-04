/*
 *  Title:	DT_regis.h - DECterm Widget ReGIS definitions
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
 *	Definitions for ReGIS emulation in DECterm Widget
 *
 *  Author:	Bob Messenger
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Eric Osman	   June 1993
 *  - Remove reference to color_names.  Let those that need it ref it
 *    in the appropriate .C module.
 *
 *  Bill Matthews  May 1990
 *  - Make color_name readwrite because it contains address data
 *    Make color_name globalref
 *
 *  Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Add definitions for pure color codes/names.
 */

typedef char *RegisData;	/* generic pointer.  ReGIS modules include
				   regstruct.h to get the real definition */

/* pure color codes - these need to be in the same order as the color
   attributes in dt_wv_hdr.h */

#define BLACK	0
#define RED	1
#define GREEN	2
#define YELLOW	3
#define BLUE	4
#define MAGENTA	5
#define CYAN	6
#define WHITE	7
