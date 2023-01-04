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
static char *rcsid = "@(#)$RCSfile: mir_intercmp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:16:13 $";
#endif
/*
 * Copyright © (C) Digital Equipment Corporation 1990, 1991.
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * MODULE DESCRIPTION:
 *
 * Common Agent Management Information Repository
 *
 * Module MIR_INTERCMP.C
 *      Contains all the internationalizable message phrases for the
 *      compiler.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   October 1992
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *      This module is included into the link for the compiler to provide
 *      for internationalization of messages.
 *
 *    Purpose:
 *       This module contains all the message phrases that are subject
 *       to internationalization, called on a per-phrase basis.
 *
 * History
 *      V1.98   Oct 1992                Created
 */

/*
|==============================================================================
| Message Phrases called from file mir_frontend.c
|==============================================================================
*/
char *mp001() {return("mirc - Error: Failed to open MSL file\n       \'%s\'\n");}
char *mp002() {return("\nmirc - Info: Compiling MSL file\n       \'%s\'\n");}
char *mp003() {return("mirc - Info: Ending Compilation of MSL file (%d lines)\n");}
char *mp004() {return("\nmirc - Info: Generating Binary Output to \'%s\'\n");}
char *mp005() {return("mirc - Error: Binary output generation failed.\n");}
char *mp006() {return("mirc - Info: No binary output file produced\n");}
char *mp007() {return("\nmirc - Info: Generating Symbolic dump to \'%s\'\n");}
/* char *mp008() {return("Include nesting limit exceeded");} (not used) */
char *mp009() {return("Include Filename (plus -I Include Path string) is too long");}
char *mp010() {return("Failed to open include file");}
char *mp011() {return("mirc - Error at line %d:\n       %s\n");}
char *mp012() {return("mirc - Error at line %d in file:\n       \'%s\'\n       %s\n");}
char *mp013() {return("mirc - Warning at line %d:\n       %s\n");}
char *mp014() {return("mirc - Warning at line %d in file:\n       \'%s\'\n       %s\n");}
char *mp015() {return("mirc - Info at line %d:\n       %s\n");}
char *mp016() {return("mirc - Info at line %d in file:\n       \'%s\'\n       %s\n");}
char *mp017() {return("Usage for MIRC Version V%d.%d: \n\n");}
char *mp018() {return("     ..............................initial options........................\n");}
char *mp019() {return("mirc [-o[<out-filename>]  -d[<dmp-filename>]  -k<keyword> -I<include-path>]. . .\n\n");}
char *mp020() {return("                              Optional\n");}
char *mp021() {return("                   Input Binary MIR Database file\n");}
char *mp022() {return("     . . . . . . .     -b<input MIR Filename>   . . . . . . . .\n\n");}
char *mp023() {return("     ....MSL Files....  ..List of MSL files..   ...MSL files....\n");}
char *mp024() {return("     ....<filename>...   -f<filelist name>      ...<filename>...\n\n");}
char *mp025() {return("     .Remove E-C Specifier.  ....Include Path....   ....MSL Filenames....\n");}
char *mp026() {return("      -r<Entity-Class OID>   -I<New Include Path>   . . . <filenames> . .\n\n");}
char *mp027() {return("    -o               Suppress binary MIR output (diagnostics only)\n");}
char *mp028() {return("    -o<filename>     Direct error-free output to binary <filename>.\n");}
char *mp029() {return("    -d               Create symbolic dump upon error-free output to\n");}
char *mp030() {return("                     filename derived from output filename (\'.dmp\')\n");}
char *mp031() {return("    -d<filename>     Create symbolic dump on error-free output to\n");}
char *mp032() {return("                     specified <filename>.\n");}
char *mp033() {return("    -k<keyword>      Specifies keyword for INCLUDE statement.\n");}
char *mp034() {return("    -I<include-path> Specifies a path to a directory where include\n");}
char *mp035() {return("                     files are sought.  (Only one -I switch active at once.)\n");}
char *mp036() {return("    -b<MIR database> Specifies *one* input binary MIR Database file.\n");}
char *mp037() {return("    -f<filelist>     Supply the name of a file as <filelist> that\n");}
char *mp038() {return("                     contains the names of MSL files, one to a line.\n");}
char *mp039() {return("                     Wildcards are not allowed, \'-r\' and \'-I\' may\n");}
char *mp040() {return("                     be used one-to-a-line, \'#\' and \'!\' start\n");}
char *mp041() {return("                     comment lines within the file.\n");}
char *mp042() {return("    -r<E-C OID>      Specifies one Entity-Class & Children to be removed,\n");}
char *mp043() {return("                     Object Identifier must specify Entity-Class\n");}
char *mp044() {return("     The definition of each CHILD Entity (supplied in an ASCII MSL file)\n");}
char *mp045() {return("     must FOLLOW the definitions of all of it's parent entities.\n");}
char *mp046() {return("mirc - Error: No symbolic dump possible without binary output\n");}
char *mp047() {return("mirc - Error: -b switch requires filename\n");}
char *mp048() {return("mirc - Erroneous multiple '-b' option\n");}
char *mp049() {return("mirc - Erroneous multiple '-d' option\n");}
char *mp050() {return("mirc - Out of Memory during initialization\n");}
char *mp051() {return("mirc - Missing keyword for '-k' option\n");}
char *mp052() {return("mirc - Erroneous multiple '-o' option\n");}
char *mp053() {return("mirc - Only one -I Include Path switch may be active at once.\n");}
char *mp054() {return("mirc - Erroneous option: %s\n");}
char *mp055() {return("mirc - Error: missing input filename(s) on command line\n");}
char *mp056() {return("\nmirc - Info: Removing Entity-Class '%s'\n");}
char *mp057() {return("mirc - Warning: %s\n       No Remove done.\n");}
char *mp058() {return("mirc - Error: Unable to open filename list file '%s'\n");}
char *mp059() {return("\nmirc - Info: Removing Entity-Class '%s'\n");}
char *mp060() {return("mirc - Error: VFORK failure\n");}
char *mp061() {return("mirc - Error: WAITPID failure\n");}
char *mp062() {return("mirc - Error: MIB translation of %s failed\n");}
char *mp063() {return("mirc - Error: EXECVE failure on \'%s\'\n");}
char *mp064() {return("       Unable to process file \'%s\'\n");}
char *mp065() {return("\nmirc - Info: Merging MSL file\n       \'%s\'\n");}
char *mp066() {return("    -m<filename>     Specifies MSL <filename> to be 'merged' into compilation\n");}


/*
|==============================================================================
| Message Phrases called from file mir_backend.c
|==============================================================================
*/
char *mp100() {return("\'%s\' is used more than once at this level (duplicate name)");}
char *mp101() {return("Unable to generate MCC OID for '%s':\n       %s\n");}
char *mp102() {return("mirc - Info: Loading existing binary MIR database file\n       \'%s\'\n");}
char *mp103() {return("mirc - Error: Load failed for file\n       \'%s\'\n");}
char *mp104() {return("mirc - Error: Out of Memory during Initialization\n");}
char *mp105() {return("Couldn't open builtin-types file %s");}
char *mp106() {return("mirc - Info: Loading the Built-In Data Types file\n       \'%s\'\n");}
char *mp107() {return("mirc - Info: Checking the Built-In Data Types file version\n       \'%s\'\n");}
char *mp108() {return("Parse of compiler-internal builtin-types file failed.  Internal Code %d\n");}
char *mp109() {return("mirc - Error: Unable to parse Builtin-Types File version string\n");}
char *mp110() {return("mirc - Warning: Possible 'Builtin-Types' Version skew:\n");}
char *mp111() {return("       Current Compiler Builtin-Types File Version = \'%s\'\n");}
char *mp112() {return("       Compiled MIR Database Builtin-Types Version = \'%s\'\n");}
char *mp113() {return("mirc - Error: \'%s\' is not a valid keyword.\n");}
char *mp114() {return("Objects nested too deeply.  Internal stack size %d exeeeded at line %d\n");}
char *mp115() {return("mirc-- Unknown error code: %d.\n");}
char *mp116() {return("Ran out of memory during operation");}
char *mp117() {return("Operation completed successfully");}
char *mp118() {return("Operation failed in some manner");}
char *mp119() {return("Specified Object Id is already registered to another Object");}
char *mp120() {return("Specified Object Id is already registered to this Object");}
char *mp121() {return("An OID in this SMI has already been registered for this object");}
char *mp122() {return("Partial match found");}
char *mp123() {return("Exact match found");}
char *mp124() {return("Exact (but short) match found");}
char *mp125() {return("Exact (but long) match found");}
char *mp126() {return("Datatype is undefined");}
char *mp127() {return("Externalization (Compiler Pass 2) Failed");}
char *mp128() {return("No parent exists for USE_PARENT operation");}
char *mp129() {return("Error opening include file, continuing");}
char *mp130() {return("Too many nested include files");}
char *mp131() {return("End of Partition encountered reading MIR database file");}
char *mp132() {return("SMI Code %d from builtin_types file is out of range.\n");}
char *mp133() {return("SMI name '%s' already in used, code %d \n");}
char *mp134() {return("Maximum OID arc length %d exceeded.\n");}
char *mp135() {return("MIR Object named '%s' has no %s code assigned");}
char *mp136() {return("mirc - add of forward-reference rel. failed: \'%s\'\n");}
char *mp137() {return("forward reference to \'%s\' from line %d is unresolved");}
char *mp138() {return("mirc - internal compiler error: fwd-ref block value %d\n");}
char *mp139() {return("abort");}
char *mp140() {return("No such Characteristic Attribute for DEPENDS ON line %d\n");}
char *mp141() {return("Out of Memory");}
char *mp142() {return("Unable to add MIR_Variant_Sel relationship");}
/*   mp143 available */
char *mp144() {return("Unable to add MIR_List_Entry relationship to Depends-On Object");}
char *mp145() {return("Unable to add MIR_Depends_OP relationship to Depends-On Object");}
char *mp146() {return("Characteristic Attribute for DEPENDS-ON has no datatype.");}
char *mp147() {return("Error at line %d: Characteristic Attribute is not of type Enumeration");}
char *mp148() {return("Unable to add MIR_Enum_Code rel. to Depends-On Object");}
char *mp149() {return("Unable to add MIR_Enum_Text rel. to Depends-On Object");}
char *mp150() {return("Error at line %d: Not a legal enumeration value: %s\n");}
char *mp151() {return("Modification of Object not created by AUGMENT file is not allowed");}
char *mp152() {return("Augment Object Code specified (%d) doesn't match predefined code (%d)");}
char *mp153() {return("mirc - Info: Merging Object %s = %d\n");}
char *mp154() {return("mirc - Error: Nothing Merged from this file\n");}
char *mp155() {return("mirc - Error: Nothing Augmented from this file\n");}


/*
|==============================================================================
| Message Phrases called from file mir_intermediate.c
|==============================================================================
*/
char *mp200() {return("mirc - Internal Error, Improper call to I_Add_Table_Entry, bad ids_table_type = %d \n");}
char *mp201() {return("mirc - Internal Error in I_Create_IDS() for non-terminal:%d.\n");}


/*
|==============================================================================
| Message Phrases called from file mir_remove.c
|==============================================================================
*/
char *mp400() {return("mirc - Internal Error:\n       Out of Memory\n");}
char *mp401() {return("OID (%s) contains invalid character");}
char *mp402() {return("OID (%s) does not exactly specify a MIR object.");}
char *mp403() {return("MIR Object (%s) named '%s'\n       is not a valid Entity-Class.");}
char *mp404() {return("mirc - Internal Error:\n       Missing containing Entity-Class\n");}
char *mp405() {return("mirc - Internal Error:\n       Missing Char. Attribute\n");}
char *mp406() {return("mirc - Internal Error:\n       Missing ref. to Char. Attribute\n");}
/* mp407 is available */
char *mp408() {return("MIR Object (%s) named '%s'\n       is not properly contained by it's parent");}
char *mp409() {return("mirc - Internal Error:\n       invalid flavor to delete, code %d\n");}
char *mp410() {return("mirc - Internal Error:\n       MIR_Structured_As: Invalid flavor to delete\n");}
char *mp411() {return("mirc - Internal Error:\n        Invalid Relationhip to delete code %d\n");}
char *mp412() {return("mirc - Internal Error:\n       Subregister down pointer missing\n");}
char *mp413() {return("mirc - Internal Error:\n       OID backpointer not found\n");}
char *mp414() {return("mirc - Internal Error:\n       Non-Terminal OID backpointer not found\n");}
char *mp415() {return("mirc - Internal Error:\n       OID backpointer to slice not found\n");}


/*
|==============================================================================
| Message Phrases called from file mir_internal.c
|==============================================================================
*/
char *mp500() {return("mirc - Error:\n       Attempt to open binary database file \'%s\' failed. status = %d");}
char *mp501() {return("mirc - Error: The World MIR Object missing: (%s) found instead\n");}
char *mp502() {return("mirc - Error: I/O Error %d on Read\n       on binary MIR database file");}
char *mp503() {return("mirc - Error: Invalid \'Endian\' Indicator\n       Should have been \'1\': was \'%d\'\n");}
char *mp504() {return("mirc - Error: Invalid Binary File Format Version Indicator\n       Should have been \'2\': was \'%d\'\n");}
char *mp505() {return("mirc - Error: Out of Memory\n       while allocating MAS Preamble of size %d");}
char *mp506() {return("mirc - Error: \'ftell()\' Error %d\n       on binary MIR database file");}
char *mp507() {return("mirc - Error: Out of Memory\n       while allocating Mini-MAS  of size %d");}
char *mp508() {return("mirc - Error: Out of Memory\n       while allocating local MAS buffer of size %d");}
char *mp509() {return("mirc - Error: Out of Memory\n       while allocating xref list of size %d");}
char *mp510() {return("mirc - Error: Out of Memory\n       while allocating syn-to-IDS list of size %d");}
char *mp511() {return("mirc - Error: MIR Input file Object/Count Mismatch\n       while processing Type %d");}
char *mp512() {return("mirc - Error: Out of Memory\n       while allocating SLICE w/size %d");}
char *mp513() {return("mirc - Error: Out of Memory\n       while allocating SUBREGISTER");}
char *mp514() {return("mirc - Error: Out of Memory\n       while allocating SIGNED NUMBER");}
char *mp515() {return("mirc - Error: Out of Memory\n       while allocating UNSIGNED NUMBER");}
char *mp516() {return("mirc - Error: Out of Memory\n       while allocating STRING");}
char *mp517() {return("mirc - Error: Out of Memory\n       while allocating NT w/size %d");}
char *mp518() {return("mirc - Internal Error: Type Count %d not zero for type %d");}
char *mp519() {return("mirc - Error: Binary MIR File internal alignment #0 error\n       mo_size = %u  external = %d  mo_type code %d");}
char *mp520() {return("mirc - Error: Binary MIR File fseek error\n       fseek value %ld");}
char *mp521() {return("mirc - Error: Binary MIR File fread error\n       errno  %d");}
char *mp522() {return("mirc - Error: Binary MIR File internal alignment #1 error\n       mo_size = %u  external = %d  mo_type code %d status = %d");}
char *mp523() {return("mirc - Error: Binary MIR File fseek error\n       fseek value %ld");}
char *mp524() {return("mirc - Error: Binary MIR File fread error\n       errno  %d");}
char *mp525() {return("mirc - Error: Binary MIR File internal alignment #2 error\n       mo_size = %u  external = %d  mo_type code %d status = %d");}
char *mp526() {return("mirc - Error: Binary MIR File internal buffer too small\n       mo_size = %u  external = %d  mo_type code %d ");}
char *mp527() {return("mirc - Error: Binary xref lookup failed\n       external address = %d, MIR object type code %d\n");}
char *mp528() {return("mirc - Error: Internal Error - 'backarc' invalid\n       arc = %d, smi code = %d");}
char *mp529() {return("mirc - Error: Internal Error - Subregister 'backarc' invalid\n       arc = %d, smi code = %d");}
char *mp530() {return("mirc - Error: Internal Error - Slice fwd ptr missing\n       Obj @ External Addr %d, SMI code %d ");}
char *mp531() {return("mirc - Error: MIR Relationship %d has no name\n");}
char *mp532() {return("mirc - Error: Internal Limit %d exceeded for MIR Relationships\n");}
char *mp533() {return("mirc - Error: Create for Rebuild Count failed\n");}
char *mp534() {return("mirc - Error: Create for Keyword IDX failed\n");}
char *mp535() {return("mirc - Error: Invalid SMI Code %d read from MIR database file \n");}
char *mp536() {return("mirc - Error: Out of Memory for IDX structure\n");}


/*
|==============================================================================
| Message Phrases called from file mir_external.c
|==============================================================================
*/
char *mp600() {return("mirc - Internal Error: Malloc failure, entry count = %d.\n");}
char *mp601() {return("mirc - Internal Error: Attempt to open file %s failed.\n");}
char *mp602() {return("mirc - Internal Error: fwrite of endian-ness indicator to '%s' failed.\n");}
char *mp603() {return("mirc - Internal Error: fwrite of binary format indicator to '%s' failed.\n");}
char *mp604() {return("mirc - Internal Error: fwrite of output PREAMBLE to '%s'failed.\n");}
char *mp605() {return("mirc - Internal Error: fwrite of MAS to '%s' failed.\n");}
char *mp606() {return("mirc - Internal Error: fclose of file '%s'failed.\n");}
char *mp607() {return("mirc - Internal Error - Out of order: \'%s\' and \'%s\'\n");}
char *mp608() {return("mirc - Internal Error: Externalized address not zero: %d.\n");}
char *mp609() {return("mirc - Internal Error: Phase Error, ex_add=%d, cell=%d.\n");}


/*
|==============================================================================
| Message Phrases called from file mir_lex.l
|==============================================================================
*/
char *mp700() {return("EOF encountered inside comment");}
char *mp701() {return("EOF hit inside quoted string");}
char *mp702() {return("Unexpected character encountered");}


/*
|==============================================================================
| Message Phrases called from file mir_yacc.y
|==============================================================================
*/
char *mp800() {return("Specified OID already in use, oid indexing not done");}
char *mp801() {return("Specified OID already registered to this object");}
char *mp802() {return("Object already has an OID of this type assigned");}
char *mp803() {return("Unable to generate DNA OID:\n       %s");}
char *mp804() {return("Invalid object identifier syntax");}
char *mp805() {return("Invalid Parentheses");}
char *mp806() {return("Invalid KEYWORD for INCLUDE statement");}
char *mp807() {return("Invalid Number");}
char *mp808() {return("Datatype '%s' is undefined.");}
char *mp809() {return("DEPENDS-ON Clause not legal for Global Entity");}
char *mp810() {return("Out of Memory for DEPENDS-ON object");}
char *mp811() {return("Invalid CATEGORY value");}
char *mp812() {return("Unsupported PRIVATE keyword encountered.");}
char *mp813() {return("Invalid Version syntax");}
char *mp814() {return("Template's relationship does not exist (misspelled?)");}
char *mp815() {return("Unrecognized SMI: defaulting to \'Unknown\'");}
char *mp816() {return("Unrecognized SMI: defaulting to \'Unknown\'");}
char *mp817() {return("Invalid SPECIFICATION syntax");}
char *mp818() {return("Sub-Range Datatype Invalid, Legal: Integer, Enum, Latin1String");}
char *mp819() {return("Invalid Subrange Numeric value");}
char *mp820() {return("Nested CASE statements not allowed");}
char *mp821() {return("Unable to convert RECORD to VARIANT RECORD");}
char *mp822() {return("'%s': No such fixed-field in this record");}
char *mp823() {return("GLOBAL entity must be first in file");}
char *mp824() {return("First File Entity is not Global: Missing PARENT= clause");}
char *mp825() {return("PARENT= clause invalid for Global Entity");}
char *mp826() {return("PARENT= clause depth does not match actual depth");}
char *mp827() {return("Missing DNS Primary Name definition");}
char *mp828() {return("Beginning-END ENTITY mismatch");}
char *mp829() {return("Empty Attribute Group list");}
char *mp830() {return("Unable to find attribute name");}
char *mp831() {return("DNS IDENT valid only for IDENTIFIER partition attributes");}
char *mp832() {return("DNS Primary Identifiers needed only for Global Entities");}
char *mp833() {return("Duplicate Primary DNS Name found");}
char *mp834() {return("Unrecognized DNS Name type");}
char *mp835() {return("No events specified for event group");}
char *mp836() {return("Event name '%s' not defined.");}
char *mp837() {return("No Counter Attributes defined");}
char *mp838() {return("Numeric Conversion failure on character \'%c\', zero used");}
char *mp839() {return("Numeric Overflow, %ld used instead.");}
char *mp841() {return("Numeric Underflow, %ld used instead.");}
char *mp842() {return("Numeric ERANGE condition occurred");}
char *mp843() {return("Object is already defined: automatic assignment of DNA OID is not allowed");}
char *mp844() {return("OID assignment to an object not created by AUGMENT file is not allowed");}
char *mp845() {return("Merge not allowed on Augment File");}
char *mp846() {return("mirc - Info: Augmenting Entity-Class at end of following path:\n       ");}
char *mp847() {return("No Such Augment Entity-Class Name '%s'.");}
char *mp848() {return("Entity defined in line %d has no IDENTIFIER=() clause");}
char *mp849() {return("Invalid Parent Name '%s'.");}
char *mp850() {return("Directive defined in line %d has no DIRECTIVE_TYPE=() clause");}
char *mp851() {return("Specified Counter Attribute is not defined");}
