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
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: %W% %E%";
#endif /* lint */
#endif /* REV_INFO */

/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/

/*
 * This file contains the structure and literal definitions required
 * by the WML processor
 */

#define TRUE		1
#define FALSE		0
#define SUCCESS		1
#define FAILURE		0

#if defined(DEC_MOTIF_BUG_FIX)
#if !defined(NULL)
#define	NULL		((void *) 0)
#endif
#else
#define	NULL		0
#endif


/*
 * Generic object pointer
 */
typedef	char	*ObjectPtr;


/*
 * True and False for attributes, so setting is explicit
 */
#define	WmlAttributeUnspecified		0
#define	WmlAttributeTrue		1
#define	WmlAttributeFalse		2

/*
 * Values of character set direction
 */
#define	WmlCharSetDirectionLtoR		1
#define	WmlCharSetDirectionRtoL		2

/*
 * Values of character set character size
 */
#define	WmlCharSizeOneByte		1
#define	WmlCharSizeTwoByte		2
#define	WmlCharSizeMixed1_2Byte		3


/*
 * Upper case and lower case converters
 */
#define _upper(c)	((c) >= 'a' && (c) <= 'z' ? (c) & 0x5F:(c))
#define _lower(c)	((c) >= 'A' && (c) <= 'Z' ? (c) | 0x20:(c))


/*
 * The Uil token classes which are dealt with WML. Matched to definitions
 * in UilKeyDef.h, although this isn't required.
 */
#define	WmlTokenClassArgument	1
#define	WmlTokenClassCharset	2
#define	WmlTokenClassColor	3
#define	WmlTokenClassEnumval	4
#define	WmlTokenClassFont	5
#define	WmlTokenClassIdentifier	6
#define	WmlTokenClassKeyword	7
#define	WmlTokenClassLiteral	8
#define	WmlTokenClassReason	9
#define	WmlTokenClassReserved	10
#define	WmlTokenClassSpecial	11
#define	WmlTokenClassUnused	12
#define	WmlTokenClassClass	13




/*
 * Structures for WML objects. Two kinds are created for each object
 * recognized and built as a WML description is read:
 *	- A syntactic descriptor, which captures the information parse
 *	  from the input in pretty much its raw form
 *	- A semantically resolved descriptor, in which pointers to
 *	  other descriptors have been resolved, ordering is done, superclass
 *	  superclass inheritance is complete, etc.
 */

/*
 * Syntactic structures. These are constructred during input parse.
 */


/*
 * A dummy holding only a header. Allows access to the validation field.
 */
typedef struct
    {
    int			validation;	/* a unique validation code */
    ObjectPtr		rslvdef;	/* the resolved object pointer */
    } WmlSynDef, *WmlSynDefPtr;



/*
 * A class resource descriptor. This a subset of a full resource descriptor,
 * which captures those elements which can be overridden in a class declaration
 */
#define WmlClassResDefValid	871253

typedef struct WmlSynClassResDefStruct
    {
    int			validation;	/* WmlClassResDefValid */
    struct WmlSynClassResDefStruct
			*next;		/* next descriptor in chain */
    char		*name;		/* resource name */
    char		*type;		/* override type name */
    char		*dflt;		/* override default value */
    short int		exclude;	/* WmlAttributeTrue if to be excluded */
    } WmlSynClassResDef, *WmlSynClassResDefPtr;


/*
 * A class controls descriptor. It contains elements which can be added
 * to a controls reference.
 */

#define WmlClassCtrlDefValid	7132320

typedef struct WmlSynClassCtrlDefStruct
    {
    int			validation;	/* WmlClassCtrlDefValid */
    struct WmlSynClassCtrlDefStruct
			*next;		/* next descriptor in chain */
    char		*name;		/* controlled class name */
    } WmlSynClassCtrlDef, *WmlSynClassCtrlDefPtr;


/*
 * A class structure. One exists for each Class statement in a WML
 * description.
 */
#define WmlClassDefValid	93741

#define WmlClassTypeMetaclass	1
#define	WmlClassTypeWidget	2
#define	WmlClassTypeGadget	3

typedef struct WmlSynClassDefStruct
    {
    int			validation;	/* WmlClassDefValid */
    struct WmlClassDefStruct
			*rslvdef;	/* resolved definition */
    short int		type;		/* from WmlClassType... */
    short int		dialog;		/* TRUE if a dialog class (has
					   window manager decoration, etc) */
    char		*name;		/* class name */
    char		*superclass;	/* superclass name */
    char		*widgetclass;	/* widget class name */
    char		*int_lit;	/* internal literal name */
    char		*convfunc;	/* convenience function name */
    char		*docname;	/* name for docs */
    char		*ctrlmapto;	/* resource controls map to */
    WmlSynClassCtrlDefPtr
			controls;	/* list of controlled classes */
    WmlSynClassResDefPtr
			resources;	/* list of resources from input */
    } WmlSynClassDef, *WmlSynClassDefPtr;


/*
 * A controls list structure. One is created for each ControlsList statement
 * in a WML description.
 */
#define WmlCtrlListDefValid	621298

typedef struct WmlSynCtrlListDefStruct
    {
    int			validation;	/* WmlCtrlListDefValid */
    struct WmlCtrlListDefStruct
			*rslvdef;	/* resolved definition */
    char		*name;		/* controls list name */
    WmlSynClassCtrlDefPtr
			controls;	/* list of controlled classes */
    } WmlSynCtrlListDef, *WmlSynCtrlListDefPtr;


/*
 * A resource descriptor. One is created for each Resource statement
 * in a WML description
 */
#define WmlResourceDefValid	310538

#define WmlResourceTypeArgument		1
#define	WmlResourceTypeReason		2
#define	WmlResourceTypeConstraint	3
#define	WmlResourceTypeSubResource	4

typedef struct WmlSynResourceDefStruct
    {
    int			validation;	/* WmlResourceDefValid */
    struct WmlResourceDefStruct
			*rslvdef;	/* resolved definition */
    short int		type;		/* from WmlResourceType... */
    char		*name;		/* resource name */
    char		*datatype;	/* resource data type */
    char		*int_lit;	/* internal literal name */
    char		*resliteral;	/* resource name literal */
    char		*enumset;	/* enumeration set name */
    char		*docname;	/* name for docs */
    char		*related;	/* names related resource */
    char		*dflt;		/* default value */
    short int		xrm_support;	/* WmlAttributeTrue if can be
					   Xrm resource */
    short int		alias_cnt;	/* alias count */
    char		**alias_list;	/* vector of aliass */
    } WmlSynResourceDef, *WmlSynResourceDefPtr;


/*
 * A datatype descriptor
 */
#define WmlDataTypeDefValid	714210

typedef struct WmlSynDataTypeDefStruct
    {
    int			validation;	/* WmlDataTypeDefValid */
    struct WmlDataTypeDefStruct
			*rslvdef;	/* resolved definition */
    char		*name;		/* data type name */
    char		*int_lit;	/* internal literal name */
    char		*docname;	/* name for docs */
    short int		xrm_support;	/* WmlAttributeTrue if can be
					   Xrm resource */
    } WmlSynDataTypeDef, *WmlSynDataTypeDefPtr;


/*
 * An enumeration set values descriptor, as it occurs in the list for
 * an enumeration set descriptor.
 */
typedef struct WmlSynEnumSetValDefStruct
    {
    int			validation;	/* WmlEnumValueDefValid */
    struct WmlSynEnumSetValDefStruct
			*next;		/* next descriptor in chain */
    char		*name;		/* enumeration value name */
    } WmlSynEnumSetValDef, *WmlSynEnumSetValDefPtr;


/*
 * An enumeration set descriptor
 */
#define	WmlEnumSetDefValid	931184

typedef struct WmlSynEnumSetDefStruct
    {
    int			validation;	/* WmlEnumSetDefValid */
    struct WmlEnumSetDefStruct
			*rslvdef;	/* resolved definition */
    char		*name;		/* enumeration set name */
    char		*datatype;	/* enumeration set data type */
    WmlSynEnumSetValDefPtr
			values;		/* list of values in set */
    } WmlSynEnumSetDef, *WmlSynEnumSetDefPtr;



/*
 * An enumeration value descriptor
 */
#define	WmlEnumValueDefValid	172938

typedef struct WmlSynEnumValueDefStruct
    {
    int			validation;	/* WmlEnumValueDefValid */
    struct WmlEnumValueDefStruct
			*rslvdef;	/* resolved definition */
    char		*name;		/* enumeration value name */
    char		*enumlit;	/* name of defining literal */
    } WmlSynEnumValueDef, *WmlSynEnumValueDefPtr;


/*
 * A charset descriptor
 */
#define WmlCharSetDefValid	110983

typedef struct WmlSynCharSetDefStruct
    {
    int			validation;	/* WmlCharSetDefValid */
    struct WmlCharSetDefStruct
			*rslvdef;	/* resolved definition */
    char		*name;		/* data type name */
    char		*int_lit;	/* internal literal name */
    char		*xms_name;	/* identifying XmString name */
    short int		direction;	/* WmlCharSetDirection... */
    short int		parsedirection;	/* WmlCharSetDirection... */
    short int		charsize;	/* WmlCharSize... */
    short int		alias_cnt;	/* alias count */
    char		**alias_list;	/* vector of aliases */
    } WmlSynCharSetDef, *WmlSynCharSetDefPtr;



/*
 * Data structures constructed during semantic validation. Each points
 * as required to syntactic data structures. These are typically accessed
 * from ordered pointer vectors.
 */


/*
 * A datatype descriptor. It extends the syntactic element by assigning
 * a code to the descriptor.
 */
typedef struct WmlDataTypeDefStruct
    {
    struct WmlSynDataTypeDefStruct
			*syndef;	/* syntactic definition */
    char		*tkname;	/* name for generating literals */
    } WmlDataTypeDef, *WmlDataTypeDefPtr;


/*
 * A charset descriptor. It extends the syntactic element by assigning
 * a code to the descriptor.
 */
typedef struct WmlCharSetDefStruct
    {
    struct WmlSynCharSetDefStruct
			*syndef;	/* syntactic definition */
    char		*tkname;	/* name for generating literals */
    short int		sym_code;	/* code value for literals */
    } WmlCharSetDef, *WmlCharSetDefPtr;


/*
 * An element in the values list of an enumeration set. These elements have
 * separate lists in order to deal with the possibility of an enumeration
 * value which is a member of more than one list.
 */
typedef struct WmlEnumSetValDefStruct
    {
    struct WmlEnumSetValDefStruct
			*next;		/* next value in list */
    struct WmlEnumValueDefStruct
			*value;		/* value descriptor for element */
    } WmlEnumSetValDef, *WmlEnumSetValDefPtr;


/*
 * A resolved enumeration set descriptor
 */
typedef struct WmlEnumSetDefStruct
    {
    struct WmlSynEnumSetDefStruct
			*syndef;	/* syntactic definition */
    char		*tkname;	/* name for generating literals */
    short int		sym_code;	/* code value for literals */
    WmlDataTypeDefPtr	dtype_def;	/* data type */
    short int		values_cnt;	/* count of # of values in set */
    WmlEnumSetValDefPtr
			values;		/* list of values in set */
    } WmlEnumSetDef, *WmlEnumSetDefPtr;


/*
 * A resolved enumeration value descriptor
 */
typedef struct WmlEnumValueDefStruct
    {
    struct WmlSynEnumValueDefStruct
			*syndef;	/* syntactic definition */
    short int		sym_code;	/* code value for literals */
    } WmlEnumValueDef, *WmlEnumValueDefPtr;


/*
 * A resource descriptor
 */
typedef struct WmlResourceDefStruct
    {
    struct WmlSynResourceDefStruct
			*syndef;	/* syntactic definition */
    char		*tkname;	/* name for generating literals */
    short int		sym_code;	/* code value for literals */
    WmlDataTypeDefPtr	dtype_def;	/* data type for base definition */
    WmlEnumSetDefPtr	enumset_def;	/* enumeration set if specified */
    short int		related_code;	/* if non-0, the sym_code for the
					   related (count) argument */
    short int		xrm_support;	/* if AttributeTrue, then the resource
					   can be set in Xrm. Resolved
					   from either explicit setting or
					   data type setting */
    struct WmlClassDefStruct
			*ref_class;	/* used for membership sort */
    struct WmlClassResDefStruct
			*ref_ptr;	/* used dynamically for search */
    } WmlResourceDef, *WmlResourceDefPtr;


/*
 * A resource descriptor with overrides which is a member of the resource
 * list of a class descriptor.
 */
typedef struct WmlClassResDefStruct
    {
    struct WmlClassResDefStruct
			*next;		/* next resource in list */
    WmlResourceDefPtr	act_resource;	/* actual resource descriptor */
    WmlDataTypeDefPtr	over_dtype;	/* overriding data type */
    char		*dflt;		/* overriding default */
    int			exclude;	/* WmlAttributeTrue if to be excluded */
    } WmlClassResDef, *WmlClassResDefPtr;


/*
 * An element for the controls list of a resolved class descriptor
 */
typedef struct WmlClassCtrlDefStruct
    {
    struct WmlClassCtrlDefStruct
			*next;		/* next control in list */
    struct WmlClassDefStruct
			*ctrl;		/* class being controlled */
    } WmlClassCtrlDef, *WmlClassCtrlDefPtr;
    
    

/*
 * A resolved class descriptor. It has a pointer to its superclass, and
 * a resource list consisting of its inherited resources followed by
 * its own resources.
 */
typedef struct WmlClassDefStruct
    {
    struct WmlSynClassDefStruct
			*syndef;	/* syntactic definition */
    struct WmlClassDefStruct
			*superclass;	/* superclass structure */
    char		*tkname;	/* name to be used in literals.
					   int_lit or name if no int_lit */
    short int		sym_code;	/* code value for literals */
    short int		inherit_done;	/* TRUE when inheritance complete */
    WmlClassResDefPtr	arguments;	/* linked argument list */
    WmlClassResDefPtr	reasons;	/* lined reason list */
    WmlClassCtrlDefPtr	controls;	/* list of controlled classes.
					   Controls list references will
					   be expanded into this list. */
    struct WmlClassDefStruct
			*variant;	/* the gadget class for a widget */
    struct WmlClassDefStruct
			*nondialog;	/* the non-dialog ancestor of a
					   dialog widget */
    WmlResourceDefPtr	ctrlmapto;	/* the resource controls map to */
    struct WmlClassCtrlDefStruct
			*ref_ptr;	/* used dynamically for search */
    } WmlClassDef, *WmlClassDefPtr;


/*
 * A resolved controls list descriptor.
 */
typedef struct WmlCtrlListDefStruct
   {
   struct WmlSynCtrlListDefStruct
			*syndef;	/* syntactic definition */
   WmlClassCtrlDefPtr	controls;	/* list of controlled classes */
   } WmlCtrlListDef, *WmlCtrlListDefPtr;



/*
 * Data structures used to locate and order objects in various ways.
 */

/*
 * Token structure used to create ordered token lists for generation of
 * UilKeyTab.h. The token string is in the order vector.
 */
typedef struct WmlKeyWTokenStruct
    {
    int			class;		/* token class, WmlTokenClass... */
    ObjectPtr		objdef;		/* object definition (resolved) */
    } WmlKeyWToken, *WmlKeyWTokenPtr;


/*
 * A grammar token as obtained from the UIL grammar file (Uil.y)
 */
typedef struct WmlGrammarTokenStruct
    {
    int			class;		/* token class, WmlTokenClass... */
    char		*token;		/* token name (literal) */
    int			val;		/* token id as value */
    } WmlGrammarToken, *WmlGrammarTokenPtr;


/*
 * A list element which allows association of a name with an object.
 * Typically used to construct ordered lists.
 */
typedef struct
    {
    char		*objname;	/* object name */
    ObjectPtr		objptr;		/* the object */
    } ObjectHandleDef, *ObjectHandleDefPtr;

/*
 * A dynamic handle element list, extensible by malloc'ing more space.
 */
typedef struct
    {
    int			cnt;		/* # entries in use */
    int			max;		/* max # entries available */
    int			ordered;	/* TRUE if list is lexicographically
					   ordered by object name */
    ObjectHandleDefPtr	hvec;		/* vector of handle entries */
    } DynamicHandleListDef, *DynamicHandleListDefPtr;



/*
 * Global declarations
 */

/*
 * Defined in wml.c
 */
extern int		wml_err_count;		/* total errors */
extern int		wml_line_count;		/* lines read from input */
extern DynamicHandleListDefPtr
			wml_synobj_ptr;		/* syntactic object list */

extern DynamicHandleListDefPtr	wml_obj_datatype_ptr;
extern DynamicHandleListDefPtr	wml_obj_enumval_ptr;
extern DynamicHandleListDefPtr	wml_obj_enumset_ptr;
extern DynamicHandleListDefPtr	wml_obj_reason_ptr;
extern DynamicHandleListDefPtr	wml_obj_arg_ptr;
extern DynamicHandleListDefPtr	wml_obj_allclass_ptr;
extern DynamicHandleListDefPtr	wml_obj_class_ptr;
extern DynamicHandleListDefPtr	wml_obj_ctrlist_ptr;
extern DynamicHandleListDefPtr	wml_obj_charset_ptr;

extern DynamicHandleListDefPtr	wml_tok_sens_ptr;
extern DynamicHandleListDefPtr	wml_tok_insens_ptr;




/*
 * Defined in wmlutils.c
 */
extern char *wmlAllocateString ();		/* dynamic string copy */
extern void wmlUpperCaseString ();		/* convert to upper case */
extern void wmlInitHList ();			/* init dynamic list */
extern void wmlResizeHList ();			/* resize a list */
extern void wmlClearHList ();			/* clear a list for reuse */
extern int wmlFindInHList ();			/* find name in list */
extern void wmlInsertInHList ();		/* generic list insert */
extern WmlClassResDefPtr wmlResolveResIsMember ();
						/* is resource in class? */


/*
 * Defined in wmlsynbld.c
 */
extern char		yystringval[];		/* any string value */
extern char		yynameval[];		/* any name (identifier) */
extern int		yytknval1;		/* terminal token value 1 */
extern int		yytknval2;		/* terminal token value 2 */
extern ObjectPtr	wml_cur_obj;		/* object being constructed */
extern ObjectPtr	wml_cur_subobj;		/* current subobject */
extern void wmlCreateClass ();
extern void wmlAddClassAttribute ();
extern void wmlAddClassResource ();
extern void wmlAddClassResourceAttribute ();
extern void wmlAddClassControl ();
extern void wmlAddCtrList ();
extern void wmlCreateResource ();
extern void wmlCreateDatatype ();
extern void wmlAddResourceAttribute ();
extern void wmlAddDatatypeAttribute ();
extern void wmlAddCtrListControl ();
extern void wmlCreateEnumSet ();
extern void wmlAddEnumSetValue ();
extern void wmlCreateEnumValue ();
extern void wmlAddEnumValueAttribute ();
extern void wmlCreateCharset ();
extern void wmlAddCharsetAttribute ();
extern void LexIssueError ();


/*
 * Defined in wmlresolve.c
 */
extern void wmlResolveDescriptors ();
extern void wmlMarkReferencePointers ();


/*
 * Defined in wmlouth.c
 */
extern void wmlOutputHFiles ();


/*
 * Defined in wmloutdat.c
 */
extern void wmlOutputDatFiles ();

/*
 * Define in wmloutp1 or wmloutp2
 */
extern void wmlOutput ();
