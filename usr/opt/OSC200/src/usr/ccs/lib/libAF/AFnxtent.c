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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: AFnxtent.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:41:29 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

/*
 * NAME: 	AFnxtent
 * FUNCTION: 	Get the next entry from an Attribute File. 
 *	     	Merge in any default entry that preceeds it.
 * RETURN VALUE DESCRIPTION: Returns a pointer to an Attribute structure.
 */

#include <stdio.h>
#include <string.h>
#include <AFdefs.h>

ENT_t
AFnxtent( AFILE_t af )
{       
    register ATTR_t cat;			/* Current attributes */
    register ATTR_t dat;			/* Default attributes */
    extern int AFread();

    while (AFread(af) != 0) {   
	/*
	 *	If current entry is a default entry then swap current with
	 *	and the default (Note: AF_dflt maybe NULL)
	 */
	if (af->AF_dflt && ! strcmp(af->AF_cent.EN_name,af->AF_dflt)) {
	    register char *tp;			/* Temp char pointer */
						    /* Copy dlft name */
	    af->AF_dent.EN_name  = af->AF_cent.EN_name;	
						    /* Swap input buffers */
	    tp = af->AF_cent.EN_cbuf;
	    af->AF_cent.EN_cbuf = af->AF_dent.EN_cbuf; 
	    af->AF_dent.EN_cbuf = tp;
						    /* Swap attr lists */
	    cat = af->AF_cent.EN_catr;
	    af->AF_cent.EN_catr = af->AF_dent.EN_catr; 
	    af->AF_dent.EN_catr = cat;
						    /* Swap next attr */
	    cat = af->AF_cent.EN_natr;
	    af->AF_cent.EN_natr = af->AF_dent.EN_natr; 
	    af->AF_dent.EN_natr = cat;
	} else {   
	/*
	 *	Normal entry, if default entry found then merge attributes
	 */
	    af->AF_cent.EN_natr = NULL;
	    if (af->AF_dent.EN_catr->AT_name != NULL) {

		for (dat=af->AF_dent.EN_catr; dat->AT_name; dat++) {   

		    for (cat=af->AF_cent.EN_catr;  ; cat++) {   
			if (cat->AT_name == NULL) {   
			    cat->AT_name = dat->AT_name;
			    cat->AT_value = dat->AT_value;
			    cat->AT_nvalue = dat->AT_nvalue;
			    if((cat -af->AF_cent.EN_catr) >= (af->AF_maxatr-1)){
				af->AF_errs |= AF_ERRCATR;
				return(&af->AF_cent);
			    }
			    cat++;
			    cat->AT_name = NULL;
			    cat->AT_value = NULL;
			    cat->AT_nvalue = NULL;
			    break;
			}
			if (strcmp(dat->AT_name, cat->AT_name) == 0) {
			    break;
			}
		    }
		}
	    } /* end if default exists */
	    return(&af->AF_cent);
	}
    }
    return(NULL);
}
