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
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#ifndef _A_OUT_H_
#define _A_OUT_H_
#include <nlist.h>	/* included for all machines */

 /*		COMMON OBJECT FILE FORMAT



 	File Organization:

 	_______________________________________________    INCLUDE FILE
 	|_______________HEADER_DATA___________________|
 	|					      |
 	|	File Header			      |    "filehdr.h"
 	|.............................................|
 	|					      |
 	|	Auxilliary Header Information	      |	   "aouthdr.h"
 	|					      |
 	|_____________________________________________|
 	|					      |
 	|	".text" section header		      |	   "scnhdr.h"
 	|					      |
 	|.............................................|
 	|					      |
 	|	".data" section header		      |	      ''
 	|					      |
 	|.............................................|
 	|					      |
 	|	".bss" section header		      |	      ''
 	|					      |
 	|_____________________________________________|
 	|______________RAW_DATA_______________________|
 	|					      |
 	|	".text" section data (rounded to 4    |
 	|				bytes)	      |
 	|.............................................|
 	|					      |
 	|	".data" section data (rounded to 4    |
 	|				bytes)	      |
 	|_____________________________________________|
 	|____________RELOCATION_DATA__________________|
 	|					      |
 	|	".text" section relocation data	      |    "reloc.h"
 	|					      |
 	|.............................................|
 	|					      |
 	|	".data" section relocation data	      |       ''
 	|					      |
 	|_____________________________________________|
 	|__________LINE_NUMBER_DATA_(SDB)_____________|
 	|					      |
 	|	".text" section line numbers	      |    "linenum.h"
 	|					      |
 	|.............................................|
 	|					      |
 	|	".data" section line numbers	      |	      ''
 	|					      |
 	|_____________________________________________|
 	|________________SYMBOL_TABLE_________________|
 	|					      |
 	|	".text", ".data" and ".bss" section   |    "syms.h"
 	|	symbols				      |	   "storclass.h"
 	|					      |
 	|_____________________________________________|
	|________________STRING_TABLE_________________|
	|					      |
	|	    long symbol names		      |
	|_____________________________________________|



 		OBJECT FILE COMPONENTS

 	HEADER FILES:
 			/usr/include/filehdr.h
			/usr/include/aouthdr.h
			/usr/include/scnhdr.h
			/usr/include/reloc.h
			/usr/include/linenum.h
			/usr/include/syms.h
			/usr/include/storclass.h

	STANDARD FILE:
			/usr/include/a.out.h    "object file" 
   */

#include "filehdr.h"
#include "aouthdr.h"
#include "scnhdr.h"
#include "reloc.h"
#include "syms.h"	/* includes storclass.h */

/*
 * Coff files produced by the mips loader are guaranteed to have the raw data
 * for the sections follow the headers in this order: .text, .rdata, .data and
 * .sdata the sum of the sizes of last three is the value in dsize in the
 * optional header.  This is all done for the benefit of the programs that
 * have to load these objects so only the file header and optional header
 * have to be inspected.  The macro N_TXTOFF() takes pointers to file header
 * and optional header and returns the file offset to the start of the raw
 * data for the .text section.  The raw data for the three data sections
 * follows the start of the .text section by the value of tsize in the optional
 * header.
 *
 * Object files produced by pre 0.23 versions of the compiler had their sections
 * rounded to 8 byte boundaries.  0.23 and later versions have their sections
 * rounded to 16 (SCNROUND in scnhdr.h) byte boundaries.
 */
#define N_TXTOFF(f, a) \
 ((a).magic == ZMAGIC || (a).magic == LIBMAGIC ? 0 : \
  ((a).vstamp < 23 ? \
   ((FILHSZ + AOUTHSZ + (f).f_nscns * SCNHSZ + 7) & ~7) : \
   ((FILHSZ + AOUTHSZ + (f).f_nscns * SCNHSZ + SCNROUND-1) & ~(SCNROUND-1)) ) )
#endif /* _A_OUT_H_ */
