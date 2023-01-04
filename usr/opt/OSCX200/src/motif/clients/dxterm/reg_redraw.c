/*****	REDRAW (IDENT = 'V1.02')  *****/
/* #module <module name> "X0.0" */
/*
 *  Title:	redraw.c
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
* FACILITY:	Common Regis Interpreter
*
* ABSTRACT:	line drawing routines
*
* ENVIRONMENT:	PDP-11
*
* AUTHOR:	Andrew F. Vesper
*
* CREATION DATE:  3-May-1983
*
* MODIFICATION HISTORY (Please update IDENT above)
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
*	Edit 002	 8-May-84	/RDM
* Make regis reentrant by including regstruct.h
*
*	Edit 001	??-???-83	/RFD
* Translated the origional Bliss code into C.
*
*	Edit 000	03-May-83	/AFV
* Initial algorithm
**/

#include "gidcalls.h"
#include "regstruct.h"

/*****			E X T E R N A L     F U N C T I O N S		 *****/
/*									     */
/*	FUNCTION		DESCRIPTION			      FILE   */
/*	--------		-----------			      ----   */
#if VARIANT == 1 
extern
    PutBreak(),			/* Flush all pending terminal output  REGIO  */
    PutChar(),			/* Queue character on terminal output REGIO  */
    PutDecimal(),		/* Output a signed decimal number     OPSYS  */
    Put_String();		/* Output an ASCIZ string	      OPSYS  */
#endif

#if SEPARATE_CURRENT_POSITION 
extern
    rpg_request_gidi_pos();		/* Request and read current	      REPORT */
#endif				/*	GIDIS position			     */

/*****		F U N C T I O N S    I N    T H I S    F I L E		 *****/
/*									     */
/*	FUNCTION			DESCRIPTION		      VALUE  */
/*	--------			-----------		      -----  */
/**
*	draw_line(v,v)							no
**/

draw_line(x, y)
int	x;
int	y;
{
    /**
    *	EXPLICIT INPUTS:
    *	    x, y		coordinates of the end point of the line
    *
    *	EXPLICIT OUTPUTS:
    *	    none
    *
    *	IMPLICIT INPUTS:
    *	    gid_x, gid_y	coordinates of starting point
    *	    gid_xy_valid	flag to say if [gid_x,gid_y] is valid
    *	    sh_mode	one of (SHADE_OFF, X_SHADE_TO, Y_SHADE_TO or
    *				SHADE_TO_POINT)
    *
    *	IMPLICIT OUTPUTS:
    *	    gid_x, gid_y	set to end point [x,y]
    *	    gid_xy_valid	always set to TRUE
    *
    *	SIDE EFFECTS:
    *	    a line is drawn from the current position [gid_x,gid_y]
    *		to the given position [x,y]
    *	    depending on sh_mode, an area is SHADEd
    **/

#define		FOR_MACRO_G58_DRAW_LINES		(10)

    int 	array_of_parameters[FOR_MACRO_G58_DRAW_LINES];
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
#if SEPARATE_CURRENT_POSITION 
    if ((rs->sh_mode != SHADE_OFF) && (IS_FALSE(rs->gid_xy_valid)))
    {
	rpg_request_gidi_pos();
    }
#endif

#if VARIANT == 1 
    Put_String( (" [draw_line sh_mode = "), PutChar );
    PutDecimal( rs->sh_mode, PutChar );
    Put_String( ("] "), PutChar );
    PutBreak();
#endif

    switch (rs->sh_mode)
    {
	case SHADE_OFF :
	case POLYGON_FILL :
	    array_of_parameters[0] = x;
	    array_of_parameters[1] = y;
	    G58_DRAW_LINES(2, array_of_parameters);
	    break;
	case X_SHADE_TO :
	    G64_BEGIN_FILLED_FIGURE();
	    array_of_parameters[0] = rs->sh_x;
	    array_of_parameters[1] = rs->gid_y;
	    array_of_parameters[2] = rs->sh_x;
	    array_of_parameters[3] = y;
	    array_of_parameters[4] = x;
	    array_of_parameters[5] = y;
	    G58_DRAW_LINES(6, array_of_parameters);
	    G65_END_FILLED_FIGURE();
	    break;
	case Y_SHADE_TO :
	    G64_BEGIN_FILLED_FIGURE();
	    array_of_parameters[0] = rs->gid_x;
	    array_of_parameters[1] = rs->sh_y;
	    array_of_parameters[2] = x;
	    array_of_parameters[3] = rs->sh_y;
	    array_of_parameters[4] = x;
	    array_of_parameters[5] = y;
	    G58_DRAW_LINES(6, array_of_parameters);
	    G65_END_FILLED_FIGURE();
	    break;
	case SHADE_TO_POINT :
	    G64_BEGIN_FILLED_FIGURE();
	    array_of_parameters[0] = rs->sh_x;
	    array_of_parameters[1] = rs->sh_y;
	    array_of_parameters[2] = x;
	    array_of_parameters[3] = y;
	    G58_DRAW_LINES(4, array_of_parameters);
	    G65_END_FILLED_FIGURE();
	    break;
	case WANT_POLYGON_FILL:
	    G56_SET_POSITION( x, y );
	    G64_BEGIN_FILLED_FIGURE();
	    rs->sh_mode = POLYGON_FILL;
	    break;
	    
    }
    rs->gid_x = x;
    rs->gid_y = y;
#if SEPARATE_CURRENT_POSITION 
    rs->gid_xy_valid = TRUE;
#endif
}		/* of GLOBAL ROUTINE draw_line */
/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*1 A_VESPER 14-SEP-1983 14:01:18 ""
**/
