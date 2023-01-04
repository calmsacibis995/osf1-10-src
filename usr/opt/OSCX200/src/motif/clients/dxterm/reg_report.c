/*****	REPORT (IDENT = 'V1.05')  *****/
/* #module <module name> "X0.0" */
/*
 *  Title:	report.c
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
* ABSTRACT:	gidis to regis report handlers
*
* ENVIRONMENT:	PDP-11
*
* AUTHOR:	Andrew F. Vesper
*
* CREATION DATE:  3 May 1983
*
* MODIFICATION HISTORY (Please update IDENT above)
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
*	Edit 005	 8-May-84	/RDM
* Make regis reentrant by including regstruct.h
*
*	Edit 004	??-???-83	/RFD
* Translated the origional Bliss code into C.
*
*	Edit 003	19-Aug-83	/AFV
* Check for the right number of words returned in a Gidis 
* report.  Keep trying to get the right number of words 
* up to MAX_ERRORS_ALLOWED tries.
*
*	Edit 002	18-Aug-83	/AFV
* Include REQUEST_GIDI_POS if ASK_FOR_TEXT_ESCAPEMENT is TRUE
*
*	Edit 001	11-Aug-83	/AFV
* Change "send_gidis" to "G_SEND_GIDIS"
*
*	Edit 000	3 May 1983	/AFV
* Initial algorithm
**/

#include "gidcalls.h"
#include "regstruct.h"

/*****			E X T E R N A L    F U N C T I O N S		 *****/
/*									     */
/*	FUNCTION		DESCRIPTION			      MODULE */
/*	--------		-----------			      ------ */
extern
    gr_flush(),			/*				      GIDISx */
    rgid_process(),		/* Send gid commands off to buffer    GIDISx */
    gid_report();		/*				      GIDISx */
#if VARIANT == 1 
extern
    PutBreak(),			/* Flush all pending terminal output  REGIO  */
    PutChar(),			/* Queue character on terminal output REGIO  */
    PutDecimal(),		/* Output signed decimal number	      OPSYS  */
    Put_String();		/* Ouput an ASCIZ string	      OPSYS  */
#endif

/*****		F U N C T I O N S    I N    T H I S    F I L E		 *****/
/*									     */
/*	FUNCTION		DESCRIPTION			       VALUE */
/*	--------		-----------			       ----- */
/**
*	get_gidi_report(v,v,a,v)					yes
*	request_gidi_report()						no
**/

get_gireport(request_op, rprt_id, buffer, bufsize)
int	request_op;
int	rprt_id;
int	*buffer;
int	bufsize;
{
    /**
    *	EXPLICIT INPUTS:
    *	    request_op		opcode which evokes the desired report
    *	    rprt_id		tag which denotes the desired report
    *	    bufsize		size of buffer
    *
    *	EXPLICIT OUTPUTS:
    *	    buffer		REF VECTOR -- a place to put the report
    *	    return value	number of words placed in the buffer
    *
    *	IMPLICIT INPUTS:	    none
    *
    *	IMPLICIT OUTPUTS:	    none
    *
    *	SIDE EFFECTS:
    *	    reads from the gidis to regis data stream
    *	    searches for the desired report id
    **/


#define		MYBUFSIZE	1
#define		LENGTH_MASK	0377

    int		my_buffer[MYBUFSIZE];
    int		words_needed;
    int		length;
    int		rprt_byte;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* first flush the data stream */

#if VARIANT == 1 
    Put_String( ("\n [get_gidi_report]   [flushing]\n "), PutChar );
    PutBreak();
#endif

    gr_flush();

    /* debug *RFD* */
/**
#if VARIANT == 1
	PutString("get_gidi_report,report.c,debug1\n",PutChar);
#endif
**/
    /* now request the desired report */

    rgid_process( &request_op, 1 );

    /* debug *RFD* */
/**
#if VARIANT == 1
	PutString("get_gidi_report,report.c,debug2\n",PutChar);
#endif
**/
    G13_FLUSH_BUFFER();

    /* debug *RFD* */
/**
#if VARIANT == 1
	PutString("get_gidi_report,report.c,debug3\n",PutChar);
#endif
**/

    /* now wait for the desired report */

    rprt_byte = (rprt_id & ~LENGTH_MASK);

#if VARIANT == 1 
    Put_String( (" [waiting for report id "), PutChar );
    PutDecimal( rprt_id, PutChar );
    Put_String("]\n", PutChar );
    PutBreak();
#endif

    do
    {
	length = gid_report(my_buffer, 1);	/* get report id */

    /* debug *RFD* */
/**
#if VARIANT == 1
	PutString("get_gidi_report,report.c,debug3.55\n", PutChar);
#endif
**/
	if (length != 1) 			/* die */
	{
#if VARIANT == 1 
	    Put_String( (" [Failure: length is "), PutChar );
	    PutDecimal( length, PutChar );
	    Put_String( (", RETURNING 0]\n "), PutChar );
	    PutBreak();
#endif
	    return(0);
	}
    }		/* end of do while */
    while ((my_buffer[0] & ~LENGTH_MASK) != rprt_byte);

    words_needed = (my_buffer[0] & LENGTH_MASK);

#if VARIANT == 1 
    Put_String( (" [tag calls for "), PutChar );
    PutDecimal( words_needed, PutChar );
    Put_String( (" more words.]\n "), PutChar );
    PutBreak();
#endif

    if (words_needed == 255) 	/* This routine cannot handle it */
    {
#if VARIANT == 1 
	Put_String( (" [need endlist input, I quit]\n "), PutChar );
	PutBreak();
#endif
	return(0);
    }
    if (words_needed > bufsize)
    {
	words_needed = bufsize;
    }
#if VARIANT == 1 
    Put_String( (" [User wants "), PutChar );
    PutDecimal( words_needed, PutChar );
    Put_String( (" more words.]\n "), PutChar );
    PutBreak();
#endif

    length = gid_report(buffer, words_needed);

#if VARIANT == 1 
    Put_String( (" [gid_report returned "), PutChar );
    PutDecimal( length, PutChar );
    Put_String( (" words, first word is "), PutChar );
    PutDecimal( *buffer, PutChar );
    Put_String( ("]\n "), PutChar );
    PutBreak();
#endif

    return(length);
}			/* of routine get_gid_report */

#if SEPARATE_CURRENT_POSITION || ASK_FOR_TEXT_ESCAPEMENT 

rpg_request_gidi_pos()
{
    /**
    *	EXPLICIT INPUTS:	    none
    *
    *	EXPLICIT OUTPUTS:	    none
    *
    *	IMPLICIT INPUTS:	    none
    *
    *	IMPLICIT OUTPUTS:
    *	    gid_x, gid_y		current position as known by Gidis
    * 	    gid_xy_valid		always set to TRUE
    *
    *	SIDE EFFECTS:
    *	    calls get_gidi_report
    **/

    int		buffer[2];
    int		error_count;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    error_count = 0;

    request_current_position( &rs->gid_x, &rs->gid_y );

#if SEPARATE_CURRENT_POSITION 
    rs->gid_xy_valid = TRUE;
#endif

}
#endif
/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*1 A_VESPER 14-SEP-1983 14:02:05 ""
**/
