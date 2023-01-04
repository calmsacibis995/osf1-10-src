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
/* $XConsortium: brfexmacro.h,v 5.2 91/04/04 13:32:56 gildea Exp $ */

/*--------------------------------------------------------------------*\
|
|  Copyright (C) 1989,1990, 1991, National Computer Graphics Association
|
|  Permission is granted to any individual or institution to use, copy, or
|  redistribute this software so long as it is not sold for profit, provided
|  this copyright notice is retained.
|
|                         Developed for the
|                National Computer Graphics Association
|                         2722 Merrilee Drive
|                         Fairfax, VA  22031
|                           (703) 698-9600
|
|                                by
|                 SimGraphics Engineering Corporation
|                    1137 Huntington Drive  Unit A
|                      South Pasadena, CA  91030
|                           (213) 255-0900
|---------------------------------------------------------------------
|
| Author        :	mfr / SimGraphics Engineering Corportation
|
| File          :	brfexception.h
| Date		:	Sun Feb 11 00:41:57 PST 1990
| Project       :	PLB
|
| Description	:	Exception handler macro defines
|
| Status        :	Version 1.0
|
| Revisions     :	
|
\*--------------------------------------------------------------------*/


#define INIT_LIST(item)\
{\
        brf_exception[item].brf_ex_type = NULL;\
        brf_exception[item].brf_ex_flag = BRF_OK;\
        brf_exception[item].brf_ex_optional[VCOLORT] = NULL;\
        brf_exception[item].brf_ex_optional[VCOLORI] = NULL;\
        brf_exception[item].brf_ex_optional[VNORM] = NULL;\
        brf_exception[item].brf_ex_optional[FCOLORT] = NULL;\
        brf_exception[item].brf_ex_optional[FCOLORI] = NULL;\
        brf_exception[item].brf_ex_optional[FNORM] = NULL;\
        brf_exception[item].brf_ex_optional[EDATA] = NULL;\
        brf_exception[item].brf_ex_range[RANGESTART] = NO_RANGE_DATA;\
        brf_exception[item].brf_ex_range[RANGESTOP] = NO_RANGE_DATA;\
        brf_exception[item].brf_ex_rrange[RANGESTART] = NO_RANGE_DATA;\
        brf_exception[item].brf_ex_rrange[RANGESTOP] = NO_RANGE_DATA;\
        brf_exception[item].brf_ex_custom_handler = NULL;\
        brf_exception[item].brf_ex_message = "This Space For Rent\n";\
        brf_exception[item].brf_ex_logic = TRUE;\
        brf_exception[item].brf_ex_numhits = 0;\
}


#define LIST_NONSUP(a,b,c)\
{\
INIT_LIST(listptr);\
brf_exception[listptr].brf_ex_type = a ;\
brf_exception[listptr].brf_ex_flag = b ;\
brf_exception[listptr].brf_ex_message = c ;\
        listptr++;\
}



#define LIST_OPTIONAL(a,b,c,d,e)\
{\
INIT_LIST(listptr);\
brf_exception[listptr].brf_ex_type = a ;\
brf_exception[listptr].brf_ex_flag = b ;\
brf_exception[listptr].brf_ex_optional[c] = TRUE ;\
brf_exception[listptr].brf_ex_custom_handler = d ;\
brf_exception[listptr].brf_ex_message = e ;\
        listptr++;\
}




#define LIST_RRANGE(a,b,c,d,e,f)\
{\
INIT_LIST(listptr);\
brf_exception[listptr].brf_ex_type = a ;\
brf_exception[listptr].brf_ex_flag = b ;\
brf_exception[listptr].brf_ex_rrange[RANGESTART] = c ;\
brf_exception[listptr].brf_ex_rrange[RANGESTOP] = d ;\
brf_exception[listptr].brf_ex_custom_handler = e ;\
brf_exception[listptr].brf_ex_message = f ;\
        listptr++;\
}




#define LIST_INVERT_RRANGE(a,b,c,d,e,f)\
{\
INIT_LIST(listptr);\
brf_exception[listptr].brf_ex_type = a ;\
brf_exception[listptr].brf_ex_flag = b ;\
brf_exception[listptr].brf_ex_logic = FALSE ;\
brf_exception[listptr].brf_ex_rrange[RANGESTART] = c ;\
brf_exception[listptr].brf_ex_rrange[RANGESTOP] = d ;\
brf_exception[listptr].brf_ex_custom_handler = e ;\
brf_exception[listptr].brf_ex_message = f ;\
        listptr++;\
}





#define LIST_IRANGE(a,b,c,d,e,f)\
{\
INIT_LIST(listptr);\
brf_exception[listptr].brf_ex_type = a ;\
brf_exception[listptr].brf_ex_flag = b ;\
brf_exception[listptr].brf_ex_range[RANGESTART] = c ;\
brf_exception[listptr].brf_ex_range[RANGESTOP] = d ;\
brf_exception[listptr].brf_ex_custom_handler = e ;\
brf_exception[listptr].brf_ex_message = f ;\
        listptr++;\
}




#define LIST_INVERT_IRANGE(a,b,c,d,e,f)\
{\
INIT_LIST(listptr);\
brf_exception[listptr].brf_ex_type = a ;\
brf_exception[listptr].brf_ex_flag = b ;\
brf_exception[listptr].brf_ex_logic = FALSE ;\
brf_exception[listptr].brf_ex_range[RANGESTART] = c ;\
brf_exception[listptr].brf_ex_range[RANGESTOP] = d ;\
brf_exception[listptr].brf_ex_custom_handler = e ;\
brf_exception[listptr].brf_ex_message = f ;\
        listptr++;\
}



#define LIST_COLORMODE(a,b,c,d)\
{\
INIT_LIST(listptr);\
brf_exception[listptr].brf_ex_type = a ;\
brf_exception[listptr].brf_ex_flag = b ;\
brf_exception[listptr].brf_ex_custom_handler = c ;\
brf_exception[listptr].brf_ex_message = d ;\
        listptr++;\
}


