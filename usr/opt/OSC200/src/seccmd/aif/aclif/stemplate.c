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
static char	*sccsid = "@(#)$RCSfile: stemplate.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:38 $";
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


/* Template routine that will get a screen.
 * This file is meant to be included in a C file that has all
 * of the necessary support routines defined.
 * Following are the #defines which must be taken care of:
 *
 *	SCRNFUNC	The name of this function, appears in menu tables
 *	PARMTEMPLATE	Template scrn_parms for this screen
 *	STRUCTTEMPLATE	Template scrn_struct[] for the variables
 *	DESCTEMPLATE	Template scrn_desc[] for the screen
 *	FILLINSTRUCT	The structure that defines the screen values
 *	VALIDATE	Routine that validates arguments to SCRNFUNC
 *	BUILDFILLIN	Routine that fills in the fillin structure
 *	SCREENACTION	Once structure is validated, this does the action
 *	FREETABS	Routine that frees temporary storage from 3 functions:
 *	COPYSCREEN	Returns a scrn_parms pointer for this screen
 *	BUILDDESC	Returns a scrn_desc pointer for this screen
 *	BUILDSTRUCT	Returns a scrn_struct pointer for this screen
 *	FIRSTDESC	Table entry of first scrn_struct field to get
 *	NSCRNSTRUCT	number of scrn_structs for this screen.
 */

/* all screen functions take an argv-like argument.
 * argv[0] is the choice from the screen.
 * argv[1,2, . . .] are the menu prompt fields.
 */

SCRNFUNC (argv)
char	**argv;
{
	struct	FILLINSTRUCT	fill;

	memset((char *) &fill, '\0', sizeof(fill));
	return (scrnfunc (argv, (char *) &fill,
	  VALIDATE, BUILDSTRUCT, BUILDDESC, BUILDFILLIN,
	  COPYSCREEN, FREETABS, SCREENACTION, FIRSTDESC, NSCRNSTRUCT,
	  &PARMTEMPLATE, DESCTEMPLATE, STRUCTTEMPLATE)
	);
}

/* so that this template can be used multiple times in the same routine,
 * undefine everything.
 */
#undef SCRNFUNC
#undef PARMTEMPLATE
#undef STRUCTTEMPLATE
#undef DESCTEMPLATE
#undef FILLINSTRUCT
#undef VALIDATE
#undef BUILDFILLIN
#undef SCREENACTION
#undef FREETABS
#undef COPYSCREEN
#undef BUILDDESC
#undef BUILDSTRUCT
#undef FIRSTDESC
#undef NSCRNSTRUCT
