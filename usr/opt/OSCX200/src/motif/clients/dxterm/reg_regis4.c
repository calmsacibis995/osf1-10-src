/*****	REGIS4 (IDENT = 'V004') *****/			/* 2 */
/* #module <module name> "X0.0" */
/*
 *  Title:	regis4.c
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
* FACILITY:	Common ReGIS interpreter for VT200 and CT100
*
* ABSTRACT:	Main interpreter code
*
* ENVIRONMENT:	PDP-11
*
* AUTHOR:	Andrew Vesper	CREATION DATE:	24-May-1983
*
* MODIFICATION HISTORY (please update IDENT above):
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
*	Edit 004	 8-May-84	/RDM
* Make regis reentrant by including regstruct.h
*
*	Edit 003	??-???-83	/RFD
* Translated the origional Bliss code into C.
*
*	Edit 002	20-Oct-83	/AFV				   
* change load_alphabet to use END_LIST format and			   
* send only those words which are needed				   
*
*	Edit 001	24-May-83	/AFV				   
* initial code							   	   
**/


#include "gidcalls.h"
#include "regstruct.h"

#define STATE_SIZE	15

/*****			E X T E R N A L    F U N C T I O N S		 *****/
/*									     */
/*	FUNCTION		DESCRIPTION				FILE */
/*	--------		-----------				---- */
extern
	rgid_process();		/* Send gid commands to be processed  GIDISx */


/*****		F U N C T I O N S    I N    T H I S    F I L E		 *****/
/*									     */
/*	FUNCTION			DESCRIPTION		       VALUE */
/*	--------			-----------		       ----- */
/**
*	load_alphabet(v,a,v,v)						no
*	restore_state()							no
*	save_state()							no
*	scroll_screen(v,v)						no
*	reg_wait(v)							no
**/

load_alphabet(character, table, width, height)
int	character;
int	*table;
int	width;
int	height;
{
    /**
    *	EXPLICIT INPUTS:
    *	    character		index of character to define
    *	    table		REF VECTOR - bit pattern
    *	    width, height	width and height of storage cell
    *
    *	EXPLICIT OUTPUTS:	    none
    *
    *	IMPLICIT INPUTS:
    *	    ld_alphabet		the alphabet to be loaded		   2
    *	    gid_alphabet	the alphabet GIDIS is working on	   2
    *
    *	IMPLICIT OUTPUTS:	    none
    *
    *	SIDE EFFECTS:
    *	    character loaded using LOAD_CHARACTER_CELL
    **/

    int		i;						/* 2 */
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    if (rs->ld_alphabet != rs->gid_alphabet)
    {
	rs->gid_alphabet = rs->ld_alphabet;
	G66_SET_ALPHABET(rs->ld_alphabet);
    }
    G68_LOAD_CHARACTER_CELL( character, height, table );
}

restore_state()
{
    /**
    *	EXPLICIT INPUTS:	    none		
    *
    *	EXPLICIT OUTPUTS:	    none
    *
    *	IMPLICIT INPUTS:	    saved state
    *
    *	IMPLICIT OUTPUTS:	    current state
    *
    *	SIDE EFFECTS:
    *	    tell Gidis about those things Gidis needs to worry about
    **/

    int		*cur_state;
    int		*svd_state;
    int		i;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /*
    * Current state is the address of the first variable in the list of
    *  variables of current states declared in file reglob.c .  This is
    *  dangerous code and caution should be used in changing it and should
    *  eventually be corrected.		??
    */
    cur_state = &rs->md_writing_color;
    /*
    * Saved state is the address of the first variable in the list of
    *  variables of saved states declared in file reglob.c .  This is
    *  dangerous code and caution should be used in changing it and should
    *  eventually be corrected.		??
    */
    svd_state = &rs->s_mdwriting_color;

    if ((rs->md_mode != rs->s_mdmode) || (rs->md_negative_writing != rs->s_mdnegative_writing))
    {
	G37_SET_WRITING_MODE(rs->s_mdmode | rs->s_mdnegative_writing);
    }
    if (rs->md_plane_mask != rs->s_mdplane_mask)
    {
	G48_SET_PLANE_MASK(rs->s_mdplane_mask);
    }
    if (rs->md_writing_color != rs->s_mdwriting_color)
    {
	G35_SET_PRIMARY_COLOR(rs->s_mdwriting_color);
    }
    if ((rs->pt_register != rs->s_ptregister) || (rs->pt_multiplier != rs->s_ptmultiplier))
    {
	G49_SET_LINE_TEXTURE(16, rs->s_ptregister, rs->s_ptmultiplier);
    }
    if ((rs->sh_alphabet != rs->s_shalphabet) || (rs->sh_char != rs->s_shchar)
     || (rs->sh_width != rs->s_shwidth) || (rs->sh_height != rs->s_shheight))
    {
	G52_SET_AREA_TEXTURE(rs->s_shalphabet, rs->s_shchar);
	G53_SET_AREA_SIZE(rs->s_shwidth, rs->s_shheight);
	if (rs->s_shalphabet >= 0)
	{
	    if (rs->al_width[rs->s_shalphabet] == 8 && rs->al_height[rs->s_shalphabet] == 10)
	    {
		G54_SET_AREA_CELL_SIZE(8, 8);
	    }
	}
    }
    for (i = 0; i < STATE_SIZE; i++)
    {
	cur_state[i] = svd_state[i];
    }
}

save_state()
{
    /**
    *	EXPLICIT INPUTS:	none
    *
    *	EXPLICIT OUTPUTS:	none
    *
    *	IMPLICIT INPUTS:
    *	    current writing mode values
    *
    *	IMPLICIT OUTPUTS:
    *	    'saved' writing mode values
    *
    *	SIDE EFFECTS:
    *	    copies all writing mode values
    **/

    int		*cur_state;
    int		*svd_state;
    int		i;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /*
    * Current state is the address of the first variable in the list of
    *  variables of current states declared in file reglob.c .  This is
    *  dangerous code and caution should be used in changing it and should
    *  eventually be corrected.		??
    */
    cur_state = &rs->md_writing_color;
    /*
    * Saved state is the address of the first variable in the list of
    *  variables of saved states declared in file reglob.c .  This is
    *  dangerous code and caution should be used in changing it and should
    *  eventually be corrected.		??
    */
    svd_state = &rs->s_mdwriting_color;

    for (i = 0; i < STATE_SIZE; i++)
    {
	svd_state[i] = cur_state[i];
    }
}

scroll_screen(rx, ry)
int	rx;
int	ry;
{
    /**
    *	EXPLICIT INPUTS:
    *	    rx, ry		relative distance to move the screen
    *
    *	EXPLICIT OUTPUTS:	    none
    *
    *	IMPLICIT INPUTS:	    none
    *
    *	IMPLICIT OUTPUTS:	    none
    *
    *	SIDE EFFECTS:
    *	    will have to handle clipping region in future
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    G97_SCROLL_CLIPPING_REGION(rx, ry);
}

reg_wait(ticks)
int	ticks;
{
    /**
    *	EXPLICIT INPUTS:
    *	    ticks		number of ticks to wait
    *
    *	EXPLICIT OUTPUTS:	    none
    *
    *	IMPLICIT INPUTS:
    *	    WT_EFN		global literal for event flag to use
    *
    *	IMPLICIT OUTPUTS:	    none
    *
    *	SIDE EFFECTS:
    *	    execution is delayed for a period of time.
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (IS_TRUE(rs->sc_need_to_flush))
    {
	G13_FLUSH_BUFFER();
    }
    rs->sc_need_to_flush = FALSE;
    G139_WAIT( ticks );
}
/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*2 A_VESPER 21-OCT-1983 16:30:33 "reduced size of g$load characters"
*	*1 A_VESPER 14-SEP-1983 14:01:53 ""
**/
