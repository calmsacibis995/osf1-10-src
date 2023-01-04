/*****  MGMM ( IDENT = 'V1.10' )  *****/
/* #module <module name> "X0.0" */
/*
 *  Title:	mgmm.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1981,1993                                                  |
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
* ABSTRACT:	Macrograph memory manager.
*		This version allocates space for macrographs within
*		a static block of memory.
*
* ENVIRONMENT:	PDP-11
*
* AUTHOR:	J.A.Lomicka	CREATION DATE: 10-July-1981
*
* MODIFICATION HISTORY (please update IDENT above):
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
*	Edit	010	 8-May-84	/RDM
* Make regis reentrant by including regstruct.h.
*
*	Edit	009	??-???-83	/RFD
* Translated the origional Bliss code into C.
*
*	Edit	008	25-May-83	/AFV
*
*	Edit	007	27-Apr-83	/DCL
* fix a bug in macrograph space reporting.
*
*	Edit	006	24-Mar-83	/DCL
* Change PSECT allocation for VT240 overlays.
*
*	Edit	005	38-Jan-83	/DCL
* Rip out the GLOBAL LITERAL method of version number tracking.  The 
* version number is now in the IDENT modifier of the MODULE heading.
*
*	Edit	004	15-Oct-82	/DCL
* Don't declare regblk and namblk external; that stuff is in REGBLK.REQ now.
*
*	Edit	003	11-Jan-82	/DCL
* Implement MGM_REPORT
*
*	Edit	002	8-Jan-82	/DCL
* Modify to use reentrant data structure
*
*	Edit	001	14-July-1981	/JAL
* Tested and believed complete and correct for statically allocated memory.
*
*	Edit	000	8-July-1981	/JAL
* Skeletal definition
**/

#include "regstruct.h"

/*****		F U N C T I O N S    I N    T H I S    F I L E		 *****/
/*									     */
/*	FUNCTION			DESCRIPTION		       VALUE */
/*	--------			-----------		       ----- */
/**
*    mac_append_character(v)						no
*    mac_clear()							no
*    mac_redefine(v)							no
*    mg_delete(v)							no
*    mgm_report(a,a)							no
**/

mac_append_character(ch)
char	ch;
{
    /**
    * FUNCTIONAL DESCRIPTION:	The character given is appended to the
    *   currently selected macrograph.  A macrograph is selected with
    *   "mac_redefine".
    *
    *   if memory is full, no action is taken and no error is returned.
    *
    * FORMAL PARAMETERS:
    *   ch:	Ascii value of character to append.
    *
    * IMPLICIT INPUTS:	Macrograph memory state
    *
    * IMPLICIT OUTPUTS:	Macrgraph emeory state
    *
    * ROUTINE VALUE:	NOVALUE
    *
    * SIDE EFFECTS:		The mg_total_stored_length and the length of the
    *   selected macrograph are incremented.	    
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (rs->mg_total_stored_length < TOTAL2_MACROGRAPH_MEMORY)
    {			/* if there is room for the character in memory */
	rs->mg_memory[rs->mg_total_stored_length++] = ch;
	rs->mg_lengths[rs->mg_top_one]++;
    }
}

mac_clear_all()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Reset all macrographs to null and deallocate associated memory.
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:	Macrograph memory and associated pointers.
    *
    * IMPLICIT OUTPUTS:	Macrograph memory and associated pointers.
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		All macrographs will read as the null string
    **/

    int			mg_name;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    rs->mg_total_stored_length = 0;
    rs->mg_top_one = 0;

/*	For each possible macrograph */

    for (mg_name = 0; mg_name <= ('Z' - 'A'); mg_name++)
    {
	rs->mg_lengths[mg_name] = 0;	   /* Clear length  */
	rs->mg_begin[mg_name] = 0; /* Clear contents */
    }
}

mac_redefine(mg_name)
int	mg_name;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Clear a macrograph, and make it the top one in memory.
    *
    * FORMAL PARAMETERS:
    *   mg_name is a macrograph number [0:25]. It is not range checked.
    *
    * IMPLICIT INPUTS:	Macrograph memory and associated pointers.
    *
    * IMPLICIT OUTPUTS:	The macrograph memory may be moved around if
    *   it is neccessary to call "mg_delete" on the specified macrograph.
    *   "mg_name" becomes the new "top_one".
    *
    * ROUTINE VALUE:	NOVALUE
    *
    * SIDE EFFECTS:	Until "mac_append_character" is called, the macrograph will
    *   read as null.  Future calls to "mac_append_character" will append
    *   to this macrograph.
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (rs->mg_lengths[mg_name] != 0)
    {
	mg_delete(mg_name);
    }
    rs->mg_top_one = mg_name;
    rs->mg_begin[mg_name] = (rs->mg_memory + rs->mg_total_stored_length);
}

mg_delete(mg_name)
int	mg_name;
{
    /**
    * FUNCTIONAL DESCRIPTION:	An existing macrograph is removed from
    *   macrograph memory.  The rest of memory is moved to close up the
    *   hole left by the deleted macrograph.  The macrograph deleted
    *   should currently be defined for proper operation of this routine.
    *
    * FORMAL PARAMETERS:
    *   mg_name:	The macrograph number to delete [0:25].
    *
    * IMPLICIT INPUTS:	Macrograph memory and associated pointers.
    *
    * IMPLICIT OUTPUTS:	The starting_points vector is updated to take
    *   into account the moving of memory into the empty space.
    *   mg_total_stored_length is updated.
    *
    * ROUTINE VALUE:		NOVALUE
    *
    * SIDE EFFECTS:		The deleted macrograph will read null.
    **/

    int			moved_mg;
    int			n;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

/*	Re-arrange memory */
    for (n = 0; n < rs->mg_total_stored_length - (rs->mg_begin[mg_name] - rs->mg_memory
	    + rs->mg_lengths[mg_name]); n++)
    {
	rs->mg_memory[(rs->mg_begin[mg_name] - rs->mg_memory) + n] =
	 rs->mg_memory[rs->mg_begin[mg_name] - rs->mg_memory + n + rs->mg_lengths[mg_name] ];
    }

/*	update starting addresses of moved objects */

    for (moved_mg = 0; moved_mg <= ('Z' - 'A'); moved_mg++)
    {
	if ((rs->mg_begin[moved_mg] - rs->mg_begin[mg_name]) > 0)
	{
	    rs->mg_begin[moved_mg] -= rs->mg_lengths[mg_name];
	}
    }
/*	Update total length */

    rs->mg_total_stored_length -= rs->mg_lengths[mg_name];

/*	Zero deleted macrograph */

    rs->mg_lengths[mg_name] = 0;
}

mgm_report( free_space, total_space )
int	*free_space;
int	*total_space;
{
    /**
    * FUNCTIONAL DESCRIPTION:	Report on macrograph space utilization
    *
    * FORMAL PARAMETERS:
    *   free_space	= address in which to store number of bytes still
    *			  available for macrograph storage
    *   total_space	= address in which to store total number of bytes,
    *			  free and in use, allocated to macrograph storage
    *
    * IMPLICIT INPUTS:	mg_total_stored_length and the literal 
    *			TOTAL2_MACROGRAPH_MEMORY.
    *
    * IMPLICIT OUTPUTS:	none
    *
    * ROUTINE VALUE:	none
    *
    * SIDE EFFECTS:	none.
    **/

    /**
    * adjustments are made for the fact that 'mg_count_defined_mgs'
    * is really a count of NULs at the end of defined macrographs.
    * 'TOTAL2_MACROGRAPH_MEMORY' includes extra space for enough NULs 
    * for all possible macrographs; 'total_space' does not.
    * likewise for 'total_stored_length' and 'free_space'
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    *free_space = TOTAL2_MACROGRAPH_MEMORY - rs->mg_total_stored_length
	+ rs->mg_count_defined_mgs - TOTAL_NUMBER_OF_MACROGRAPHS;

    *total_space = TOTAL2_MACROGRAPH_MEMORY - TOTAL_NUMBER_OF_MACROGRAPHS;
}
/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*1 A_VESPER 14-SEP-1983 14:00:31 ""
**/
