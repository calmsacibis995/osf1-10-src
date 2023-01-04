/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	< to be supplied >
**
**  ABSTRACT:
**
**	< to be supplied >
**
**  ENVIRONMENT:
**
**	< to be supplied >
**
**  MODIFICATION HISTORY:
**
**	< to be supplied >
**
**--
**/
/*
 *	d e s c . h - descriptor macros to aid in the definition and use
 *		of read only and write only string descriptors
 */

#ifndef DSC$K_DTYPE_T
#include <descrip.h>
#endif

#define CALLSYS(function)	\
	if( ((function) & 1) != 1 )

#define DCLDESC(name)	\
	struct dsc$descriptor_s name  = {0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0}

#define FILLDESC_R(name,str)	\
	name.dsc$w_length = strlen(str);	\
	name.dsc$a_pointer = str

#define FILLDESC_W(name,str)	\
	name.dsc$w_length = strlen(str);	\
	strcpy( name.dsc$a_pointer, str )

#define INITDESC_W(name,str)	\
	name.dsc$w_length = sizeof(str);	\
	name.dsc$a_pointer = str

#define FIXDESC(desc,length)	\
	desc.dsc$a_pointer[length] = 0;	\
	desc.dsc$w_length = length

#define	STRVAL(desc) desc.dsc$a_pointer

#define STRLEN(desc) desc.dsc$w_length

#define DTYPE(desc) desc.dsc$b_dtype

#define DCLASS(desc) desc.dsc$b_class

#define DUMPDESC(desc) \
	printf("------------- GENERAL VALUES --------------\n"); \
	printf("LENGTH = %d\n", desc.dsc$w_length ); \
	printf("TYPE   = %d\n", desc.dsc$b_dtype ); \
	printf("CLASS  = %d\n", desc.dsc$b_class ); \
	printf("-------------- STRING VALUES --------------\n"); \
	printf("ADDRESS OF STRING = %D\n", desc.dsc$a_pointer ); \
	printf("VALUE OF STRING = %s\n", desc.dsc$a_pointer ); \
	printf("--------------------------------------------\n")
