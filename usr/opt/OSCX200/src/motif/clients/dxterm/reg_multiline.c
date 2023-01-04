/*****	MULTILINE (IDENT = 'V1.03' )  *****/
/* #module <module name> "X0.0" */
/*
 *  Title:	multiline.c
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
* FACILITY:	Common ReGIS Parser
*
* ABSTRACT:	multi-draw-line routines -- handles shading
*
* ENVIRONMENT:	PDP-11
*
* AUTHOR:	Andrew F. Vesper
*
* CREATION DATE:  7 March 1983
*
* MODIFICATION HISTORY (Please update IDENT above):
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
*	Edit 003	 8-May-84	/RDM
* Make regis reentrant by including regstruct.h.
*
*	Edit 002	??-???-83	/RFD
* Translated the origional Bliss code into C.
*
*	Edit 001	10 Aug 83	/AFV
* Modify to include in the Regis interpreter
*
*	Edit 000	 7 Mar 83	/AFV
* Algorithm testing.
**/

#include "gidcalls.h"
#include "regstruct.h"

/*****			E X T E R N A L     F U N C T I O N S		 *****/
/*									     */
/*	FUNCTION		DESCRIPTION			       FILE  */
/*	--------		-----------			       ----  */
extern
    rgid_process();		/* Send gid commands to be processed  GIDISx */
#if VARIANT == 1
extern
    PutBreak(),			/* Flush all pending terminal output  REGIO  */
    PutChar(),			/* Queue character on terminal output REGIO  */
    PutDecimal(),		/* Output a signed decimal number     OPSYS  */
    Put_String();		/* Output an ASCIZ string	      OPSYS  */
#endif

/*****		F U N C T I O N S    I N    T H I S    F I L E		 *****/
/* 									     */
/*    FUNCTION			DESCRIPTION			      VALUE  */
/*    --------			-----------			      -----  */
/**
*  draw_multiline(v,a,a)						no
*  sign_mult_6(v,v,v,v,v,v)						yes
**/

#define		SIGN_MASK	0100000

#define		CRLF		{					\
		   Put_String( ("\n"), PutChar );			\
		   PutBreak(); 						\
				}

#define	mult_2_signed(a, b)	( ((a == 0) || (b == 0)) ? 0 :		\
	(((a & SIGN_MASK) == (b & SIGN_MASK)) ? 1 : -1) )


draw_multi_line(n, x, y)
int	n;
int	*x;
int	*y;
{
    int		i;
    int		saved_x;
    int		saved_y;
    int 		direction;
    int		new_direction;
    int		save_dir;
    int		new_saved_dir;
    int		start;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

#if VARIANT == 1
  Put_String( ("entering draw_multi_line, n = "), PutChar );
  PutDecimal( n, PutChar );
  CRLF;
  Put_String( ("shading position is ["), PutChar );
  PutDecimal( rs->sh_x, PutChar );
  Put_String( (","), PutChar );
  PutDecimal( rs->sh_y, PutChar );
  Put_String( ("]"), PutChar );
  CRLF;
  Put_String( ("current position is ["), PutChar );
  PutDecimal( rs->gid_x, PutChar );
  Put_String( (","), PutChar );
  PutDecimal( rs->gid_y, PutChar );
  Put_String( ("]"), PutChar );
  CRLF;

    for (i = 0; i < n; i++)
    {
	Put_String( (" point "), PutChar );
	PutDecimal( i, PutChar );
	Put_String( (" is ["), PutChar );
	PutDecimal( x[i], PutChar );
	Put_String( (","), PutChar );
	PutDecimal( y[i], PutChar );
	Put_String( ("]"), PutChar );
	CRLF;
    }
#endif

#if SEPARATE_CURRENT_POSITION
    if (IS_FALSE(rs->gid_xy_valid))
    {
	rpg_request_gidi_pos();
    }
#endif
    saved_x = rs->gid_x;
    saved_y = rs->gid_y;

    if ( rs->sh_mode == WANT_POLYGON_FILL )
	{
	rs->sh_mode = POLYGON_FILL;
	G56_SET_POSITION( x[0], y[0] );
	G64_BEGIN_FILLED_FIGURE();
	start = 1;
	}
    else
	start = 0;

    switch (rs->sh_mode)
    {
	case (SHADE_OFF) :
	case (WANT_POLYGON_FILL) :
	case (POLYGON_FILL) :
#if VARIANT == 1
	    Put_String( ("SHADE off"), PutChar );
	    CRLF;
#endif
	    G59_DRAW_LINES_END_LIST();
	    for (i = start; i < n; i++)
	    {
		rs->temp_gid_buffer[0] = x[i];
		rs->temp_gid_buffer[1] = y[i];
		rgid_process( rs->temp_gid_buffer, 2 );
	    }
	    G2_END_LIST();
	    rs->gid_x = x[n-1];
	    rs->gid_y = y[n-1];

#if SEPARATE_CURRENT_POSITION
	    rs->gid_xy_valid = TRUE;
#endif
	    break;
	case (Y_SHADE_TO) :
#if VARIANT == 1
	    Put_String( ("SHADE to y"), PutChar );
	    CRLF;
#endif
	    G64_BEGIN_FILLED_FIGURE();
	    G59_DRAW_LINES_END_LIST();

	    for (i = 0; i <= (n - 2); i++)
	    {
		rs->temp_gid_buffer[0] = x[i];
		rs->temp_gid_buffer[1] = y[i];
		rgid_process( rs->temp_gid_buffer, 2 );

		if (((x[i] - rs->gid_x) && SIGN_MASK) != ((x[i+1] - x[i]) &&
			 SIGN_MASK))
		{
		    rs->temp_gid_buffer[0] = x[i];
		    rs->temp_gid_buffer[1] = rs->sh_y;
		    rs->temp_gid_buffer[2] = saved_x;
		    rs->temp_gid_buffer[3] = rs->sh_y;
		    rgid_process( rs->temp_gid_buffer, 4 );
		    G2_END_LIST();
		    G65_END_FILLED_FIGURE();
		    saved_x = x[i];
		    saved_y = y[i];
		    G56_SET_POSITION(saved_x, saved_y);
		    rs->gid_x = saved_x;
		    rs->gid_y = saved_y;
		    G64_BEGIN_FILLED_FIGURE();
		    G59_DRAW_LINES_END_LIST();
		}
	    }	/* end of for loop */
	    rs->temp_gid_buffer[0] = x[n-1];
	    rs->temp_gid_buffer[1] = y[n-1];
	    rs->temp_gid_buffer[2] = x[n-1];
	    rs->temp_gid_buffer[3] = rs->sh_y;
	    rs->temp_gid_buffer[4] = saved_x;
	    rs->temp_gid_buffer[5] = rs->sh_y;
	    rgid_process( rs->temp_gid_buffer, 6 );
	    G2_END_LIST();
	    G65_END_FILLED_FIGURE();
	    G56_SET_POSITION(rs->gid_x = x[n-1], rs->gid_y = y[n-1]);

#if SEPARATE_CURRENT_POSITION
	    rs->gid_xy_valid = TRUE;
#endif
	    break;
	case (X_SHADE_TO) :
#if VARIANT == 1
	    Put_String( ("SHADE to x"), PutChar );
	    CRLF;
#endif
	    G64_BEGIN_FILLED_FIGURE();
	    G59_DRAW_LINES_END_LIST();

	    for (i = 0; i <= (n - 2); i++)
	    {
		rs->temp_gid_buffer[0] = x[i];
		rs->temp_gid_buffer[1] = y[i];
		rgid_process( rs->temp_gid_buffer, 2 );
		if (((y[i] - rs->gid_y) && SIGN_MASK) != 
	            ((y[i+1] - y[i]) && SIGN_MASK))
		{
		    rs->temp_gid_buffer[0] = rs->sh_x;
		    rs->temp_gid_buffer[1] = y[i];
		    rs->temp_gid_buffer[2] = rs->sh_x;
		    rs->temp_gid_buffer[3] = saved_y;
		    rgid_process( rs->temp_gid_buffer, 4 );
		    G2_END_LIST();
		    G65_END_FILLED_FIGURE();
		    saved_x = x[i];
		    saved_y = y[i];
		    G56_SET_POSITION(saved_x, saved_y);
		    rs->gid_x = saved_x;
		    rs->gid_y = saved_y;
		    G64_BEGIN_FILLED_FIGURE();
		    G59_DRAW_LINES_END_LIST();
		}
	    }			/* end of for loop */
	    rs->temp_gid_buffer[0] = x[n-1];
	    rs->temp_gid_buffer[1] = y[n-1];
	    rs->temp_gid_buffer[2] = rs->sh_x;
	    rs->temp_gid_buffer[3] = y[n-1];
	    rs->temp_gid_buffer[4] = rs->sh_x;
	    rs->temp_gid_buffer[5] = saved_y;
	    rgid_process( rs->temp_gid_buffer, 6 );
	    G2_END_LIST();
	    G65_END_FILLED_FIGURE();
	    G56_SET_POSITION( rs->gid_x = x[n-1], rs->gid_y = y[n-1] );
	    break;		/* End of X_SHADE_TO code */
	case (SHADE_TO_POINT) :
	{

#if VARIANT == 1
	    Put_String( ("SHADE to point"), PutChar );
	    CRLF;
#endif
	    G56_SET_POSITION(rs->sh_x, rs->sh_y);
	    G64_BEGIN_FILLED_FIGURE();
	    G59_DRAW_LINES_END_LIST();
	    rs->temp_gid_buffer[0] = saved_x;
	    rs->temp_gid_buffer[1] = saved_y;
	    rgid_process(rs->temp_gid_buffer, 2);

	    if (saved_x == rs->sh_x)
	    {
		direction = mult_2_signed(saved_y - rs->sh_y, x[0] - rs->sh_x);
#if VARIANT == 1
		Put_String( ("x == sh_x, dir = "), PutChar );
		PutDecimal(direction, PutChar);
		CRLF;
#endif
	    }
	    else
	    {
		direction = mult_6_signed(saved_x - x[0], rs->sh_y, x[0] - rs->sh_x,
		   saved_y, y[0], saved_x - rs->sh_x);
#if VARIANT == 1
		Put_String( ("x != sh_x, dir = "), PutChar );
		PutDecimal(direction, PutChar);
		CRLF;
#endif
	    }
	    save_dir = 0;

	    for (i = 0; i <= (n - 2); i++)
	    {
		rs->temp_gid_buffer[0] = x[i];
		rs->temp_gid_buffer[1] = y[i];
		rgid_process( rs->temp_gid_buffer, 2 );

		if (x[i] == rs->sh_x)
		{
		    new_direction = mult_2_signed(y[i] - rs->sh_y, x[i+1] - rs->sh_x);
#if VARIANT == 1
		    Put_String( ("x == sh_x, new dir = "), PutChar );
		    PutDecimal( new_direction, PutChar );
		    CRLF;
#endif
		}
		else
		{
		    new_direction = mult_6_signed(x[i] - x[i+1], rs->sh_y,
			x[i+1] - rs->sh_x,  y[i], y[i+1], x[i] - rs->sh_x);
#if VARIANT == 1
		    Put_String( ("x != sh_x, new dir = "), PutChar );
		    PutDecimal( new_direction, PutChar );
		    CRLF;
#endif
		}
		if (saved_x == rs->sh_x)
		{
		    new_saved_dir = mult_2_signed(saved_y - rs->sh_y, x[i+1] - rs->sh_x);
#if VARIANT == 1
		    Put_String( ("saved x == shading x, new saved dir = "),
					PutChar );
		    PutDecimal( new_saved_dir, PutChar );
		    CRLF;
#endif
		}
		else
		{
		    new_saved_dir = mult_6_signed(saved_x - x[i+1], rs->sh_y,
			x[i+1] - rs->sh_x, saved_y, y[i+1], saved_x - rs->sh_x);
#if VARIANT == 1
		    Put_String( ("saved x != shading x, new saved dir = "),
					PutChar );
		    PutDecimal(new_saved_dir, PutChar );
		    CRLF;
#endif
		}
		if ((mult_2_signed( direction, new_direction ) < 0)
		    || (mult_2_signed( save_dir, new_saved_dir ) < 0))
		{
		    G2_END_LIST();
		    G65_END_FILLED_FIGURE();
		    G56_SET_POSITION(rs->sh_x, rs->sh_y);
		    saved_x = x[i];
		    saved_y = y[i];
		    G64_BEGIN_FILLED_FIGURE();
		    G59_DRAW_LINES_END_LIST();
		    rs->temp_gid_buffer[0] = saved_x;
		    rs->temp_gid_buffer[1] = saved_y;
		    rgid_process( rs->temp_gid_buffer, 2 );
		    save_dir = 0;
		    new_saved_dir = 0;
		}
		if (new_direction != 0)
		{
		    direction = new_direction;
		}
		if (new_saved_dir != 0)
		    save_dir = new_saved_dir;
		}
	    }			/* End of INCR loop */
	    rs->temp_gid_buffer[0] = x[n-1];
	    rs->temp_gid_buffer[1] = y[n-1];
	    rgid_process( rs->temp_gid_buffer, 2 );
	    G2_END_LIST();
	    G65_END_FILLED_FIGURE();
	    G56_SET_POSITION(rs->gid_x = x[n-1], rs->gid_y = y[n-1]);

#if SEPARATE_CURRENT_POSITION
	    rs->gid_xy_valid = TRUE;
#endif
    }
}

mult_6_signed(f1, f2, f3, f4, f5, f6)
int	f1;
int	f2;
int	f3;
int	f4;
int	f5;
int	f6;
{
    /**
    * Function:  calculate sign of f1*f2 + f3*f4 - f5*f6
    *
    * Parameters: all pass by value.
    *
    **/

    long	int	 a;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    a = (f1 * f2) + (f3 * f4) - (f5 * f6);
    if (a == 0)
    {
	return(0);
    }
    else if (a < 0)
    {
	return(-1);
    }
    else
    {
	return(1);
    }
}
/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*1 A_VESPER 14-SEP-1983 14:00:36 ""
**/
