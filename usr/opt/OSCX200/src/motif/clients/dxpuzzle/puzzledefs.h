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
  Copyright (c) Digital Equipment Corporation, 1987, 1988, 1989, 1990
  All Rights Reserved.  Unpublished rights reserved
  under the copyright laws of the United States.

  The software contained on this media is proprietary
  to and embodies the confidential technology of
  Digital Equipment Corporation.  Possession, use,
  duplication or dissemination of the software and
  media is authorized only pursuant to a valid written
  license from Digital Equipment Corporation.

  RESTRICTED RIGHTS LEGEND   Use, duplication, or
  disclosure by the U.S. Government is subject to
  restrictions as set forth in Subparagraph (c)(1)(ii)
  of DFARS 252.227-7013, or in FAR 52.227-19, as
  applicable.
*/
 


/*
**++
**  MODULE NAME:
**	puzzledefs.h
**
**  FACILITY:
**      OOTB Puzzle
**
**  ABSTRACT:
**	Global Defines and typedefs for the puzzle
**
**  AUTHORS:
**      Neal Finnegan
**
**  RELEASER:
**
**  CREATION DATE:     6-OCT-1987
**
**  MODIFICATION HISTORY:
**
**--
**/

#ifdef VMS
#include "stdio.h"
#else
#include <stdio.h>
#endif


#ifdef VMS
#include "signal.h"
#else
#include <signal.h>
#endif


#ifdef VMS
#include "math.h"
#else
#include <math.h>
#endif

#ifdef VMS
#include "time.h"
#else
#include <time.h>
#endif


#ifdef UNIX
#include <Mrm/MrmAppl.h>
#include <Xm/BulletinB.h>
#include <Xm/BulletinBP.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/RowColumnP.h>
#include <Xm/Scale.h>
#include <MessageB.h>
#include <DXm/DXmHelpB.h>
#else

#include "MrmAppl.h"
#include "BulletinB.h"
#include "BulletinBP.h"
#include "PushB.h"
#include "RowColumn.h"
#include "RowColumnP.h"
#include "Scale.h"
#include "DXmHelpB.h"
#include "MessageB.h"

#include "descrip.h"

#endif


/* Error strings that can't be done with UIL	*/
#define	NoPuzzleHierarchy	"Can't open hierarchy \n"
#define	NoPuzzleMain		"Can't fetch main window \n"
#define	NoPuzzlePopup		"Can't fetch popup menu \n"
#define	NoMessageWidget		"Can't fetch message widget \n"
#define	NoPuzzleHelp		"Can't fetch help message \n"
#define	NoPuzzleSettings	"Can't fetch settings \n"

#define	APPL_NAME		"Puzzle"
#ifdef VMS
#define	CLASS_NAME		"DECW$PUZZLE"
#define PUZZLE_HELP		"DECW$PUZZLE.DECW$BOOK"
#else
#define	CLASS_NAME		"DXpuzzle"
#endif

#define	Half(x)			((x) >> 1)
#define	Double(x)		((x) << 1)

#define	MAX_FILE_LEN		256
#define FACTOR_3D		13
#define MIN_INDENT		2
#define REDUCE_FACTOR		30
#define MAX_ACROSS		10
#define MAX_TILES		(MAX_ACROSS * MAX_ACROSS)
#define EMPTY			-1

#define SHFT_KEY  		174
#define CTRL_KEY  		175
#define LOCK_KEY  		176
#define COMP_KEY  		177

typedef struct {
	int 		row, col;
	Position	x, y;
} tiledef;


/*
  	Several OS specific defines
OK_STATUS - status to return for success
ERROR_STATUS - status to return for failure
*/                                                      

#ifdef VMS
#define	OK_STATUS	1
#define ERROR_STATUS	0
#else
#define	OK_STATUS	0
#define ERROR_STATUS	1
#endif

#define INFORMATION		0
#define WARNING			1
#define ERROR			2
#define FATAL			3


#ifdef VMS
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *                      VMSDSC (VMS descriptors)
 *                      ========================
 *
 *  These macros require the following header file:
 *
 *              descrip.h
 *
 * VMSDSC (typedef)
 * ------
 *
 * $INIT_VMSDSC (intialize a VMS static descriptor)
 * ------------
 *      dsc             : VMSDSC        : write : value
 *      len             : int           : read  : value
 *      addr            : char          : read  : ref
 *
 *    RETURNS: address of the descriptor (dsc)
 */

#define VMSDSC          struct dsc$descriptor

#define $INIT_VMSDSC(dsc, len, addr)            \
        ((dsc .dsc$b_class) = DSC$K_CLASS_S,    \
         (dsc .dsc$b_dtype) = DSC$K_DTYPE_T,    \
         (dsc .dsc$w_length) = (len),           \
         (dsc .dsc$a_pointer) = (addr),         \
         (& (dsc) )                             \
        )
#endif
