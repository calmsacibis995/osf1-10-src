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
static char *rcsid = "@(#)$RCSfile: snmppe_text.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:34:22 $";
#endif
/*
 **  Copyright (c) Digital Equipment Corporation, 1992
 **  All Rights Reserved.  Unpublished rights reserved
 **  under the copyright laws of the United States.
 **  
 **  The software contained on this media is proprietary
 **  to and embodies the confidential technology of 
 **  Digital Equipment Corporation.  Possession, use,
 **  duplication or dissemination of the software and
 **  media is authorized only pursuant to a valid written
 **  license from Digital Equipment Corporation.
 **
 **  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 **  disclosure by the U.S. Government is subject to
 **  restrictions as set forth in Subparagraph (c)(1)(ii)
 **  of DFARS 252.227-7013, or in FAR 52.227-19, as
 **  applicable.
 **
 *
 * MODULE DESCRIPTION:
 *
 * Common Agent SNMP Protocol Engine
 *
 * Module SNMPPE_TEXT.C
 *      Contains 'text' functions (translatable text) messages for the SNMP 
 *      Protocol Engine for the Common Agent.
 *
 * WRITTEN BY:
 *    Enterprise Management Frameworks/Common Agent for ULTRIX
 *    D. McKenzie   October 1992
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines accepts requests over a network,
 *       converts incoming information into standard Common Agent format
 *       and calls the Common Agent for processing of that request.
 *
 *    Purpose:
 *       This module contains the translatable text messages (as functions)
 *       for all diagnostic output of the SNMP protocol engine.
 *
 * History
 *      V1.0    Oct 1992     D. McKenzie
 *
 * NOTES:
 *
 */

/*
File SNMPPE_TEXT.C contains all printable text output for the 'Mxxx' messages
used throughout the protocol engine for diagnostic and debugging use.  If you
modify the text of any message, or add new messages, or delete old messages,
you must:

   (a) Add the new message/modify the existing message/delete the old message
       in/in/from SNMPPE_TEXT.C.  Be sure to follow the format used in
       this module when creating a new text message 'function'.

   (b) Use the 'MSG' macro defined in SNMPPE.H; this macro invokes the
       appropriate msgXXX() function which you added to this file
       in steb (a) above.  Be sure to add/delete the 'extern' for the 
       new/deleted message 'function' at the end of SNMPPE.H.  Perform
       this step ONLY if you are adding a new message, or if you are
       deleting an existing message.  Modifications to existing messages
       require changes to the appropriate function in this file only.

*/

char *msg001() { "M001 - mutex lock failed in pe_log(): %m";}
char *msg002() { "M002 - mutex unlock failed in pe_log(): %m";}
char *msg003() { "M003 - RPC Receive Thread(s) creation failed";}
char *msg004() { "M004 - snmp_pe (V%s.%s) initialization complete";}
char *msg005() { "M005 - invalid -d usage\n";}
char *msg006() { "M006 - Missing debug logfile name\n";}
char *msg007() { "M007 - invalid -c usage\n";}
char *msg008() { "M008 - Missing configuration file name\n";}
char *msg009() { "M009 - invalid command line argument: '%s'\n";}
char *msg010() { "M010 - invalid debug class: '%s'\n";}
char *msg011() { "M011 - pthread mutex initialization failed: %s\n";}
char *msg012() { "M012 - attempt to open file '%s' failed: %s\n";}
char *msg013() { "M013 - attempt to open system log failed %d\n";}
char *msg014() { "M014 - open failed on config file: '%s'";}
char *msg015() { "M015 - Missing Community name, config file line %d";}
char *msg016() { "M016 - Missing Trap Community name, config file line %d";}
char *msg017() { "M017 - %s: License for Common Agent Base Kit is not loaded.\n";}
char *msg018() { "M018 - %s: not super user.\n";}
char *msg019() { "M019 - Malloc fail in init_config()";}
char *msg020() { "M020 - Malloc fail in init_config()";}
char *msg021() { "M021 - Malformed inet address '%s', config file line %d";}
char *msg022() { "M022 - invalid access mode '%s', config file line %d";}
char *msg023() { "M023 - Malloc fail in init_config()";}
char *msg024() { "M024 - Malloc fail in init_config()";}
char *msg025() { "M025 - Malformed inet address, config file line %d";}
char *msg026() { "M026 - Unrecognized entry, config file line %d";}
char *msg027() { "M027 - unable to create snmp variables shared mem, '%s', errno = %d";}
char *msg028() { "M028 - unable to attach to snmp variables shared memory, errno = %d";}
char *msg029() { "M029 - unable to init mutex for statistics block, errno = %d";}
char *msg030() { "M030 - invalid -p port value\n";}
char *msg031() { "M031 - Missing inbound port number\n";}
char *msg032() { "M032 - invalid outbound -t port value\n";}
char *msg033() { "M033 - Missing outbound port number\n";}
char *msg034() { "M034 - service for 'snmp' 'udp' not found by getservbyname()";}
char *msg035() { "M035 - unable to acquire internet socket. '%s' errno=%d";}
char *msg036() { "M036 - unable to bind socket in internet domain. '%s' errno=%d";}
char *msg037() { "M037 - service for 'snmp-trap' 'udp' not found by getservbyname()";}
char *msg038() { "M038 - unable to acquire internet socket. '%s' errno=%d";}
char *msg039() { "M039 - unable to bind socket in internet domain. '%s' errno=%d";}
char *msg040() { "M040 - acquisition of Service List mutex failed, errno = %d";}
char *msg041() { "M041 - release of Service List mutex failed, errno = %d";}
char *msg042() { "M042 - No memory for Service Block allocation";}
char *msg045() { "M045 - Error during recvfrom: '%s'\n";}
char *msg046() { "M046 - Internal RECVFROM Error limit exceeded";}
char *msg047() { "M047 - ASN.1 PDU parse failed from %s, size = %d, MCC = %d, PDU ignored";}
char *msg048() { "M048 - Memory exhausted during varbind list parse";}
char *msg049() { "M049 - moss error %d on moss_avl_free() call";}
char *msg050() { "M050 - moss error %d on moss_avl_free() call";}
char *msg051() { "M051 - No varbindlist in PDU from %s, size = %d (decimal), PDU ignored";}
char *msg052() { "M052 - ASN.1 varbindlist parse failed on PDU from %s, size = %d (dec), mcc(%d), moss(%d), PDU ignored";}
char *msg053() { "M053 - GET-RESPONSE PDU 'sendto %d' address(%s) failed, errno = %d, '%s'";}
char *msg054() { "M054 - ASN.1 varbindlist encode failed on PDU to %s, mcc(%d), moss(%d), response not generated";}
char *msg055() { "M055 - ASN.1 encode failed on PDU from %s, response not generated";}
char *msg056() { "M056 - Unable to serialize PDU: dropped";}
char *msg057() { "M057 - Unable to serialize PDU: dropped";}
char *msg058() { "M058 - Attempt to reset AVL failed (%s)";}
char *msg059() { "M059 - Attempt to point AVL failed (%s)";}
char *msg060() { "M060 - Error from AVL free, instance (%s), attribute (%s)";}
char *msg061() { "M061 - Error from AVL free: %d";}
char *msg062() { "M062 - anomalous Rtn (%s) from msi_get_attributes";}
char *msg063() { "M063 - Add of Element to Instance AVL element failed (%s)";}
char *msg064() { "M064 - No Arcs left: conversion fails on (%s)";}
char *msg065() { "M065 - Length Arc #%d invalid '%d': convert fails on (%%s)";}
char *msg066() { "M066 - Memory exhausted";}
char *msg067() { "M067 - Instance Arc #%d too large '%d': (%%s)";}
char *msg068() { "M068 - Error from AVL free: %d";}
char *msg069() { "M069 - Add of subelement to Instance AVL element failed (%s)";}
char *msg070() { "M070 - Not enough instance arcs left (%d) for IP Address(%%s)";}
char *msg071() { "M071 - Instance AVL initialization failed (%s)";}
char *msg072() { "M072 - No Arcs left: conversion fails/not done on (%s)";}
char *msg073() { "M073 - Unsuppted Instance Arc #%d translation for Tag %d, (%%s)";}
char *msg074() { "M074 - Invalid derived-status %d inbound argument";}
char *msg078() { "M078 - Attribute list AVL initialization failed (%s)";}
char *msg079() { "M079 - Temporary Object Id creation failed (%s)";}
char *msg080() { "M080 - Attribute Object Id creation failed (%s)";}
char *msg081() { "M081 - Temporary Object Id deletion failed (%s)";}
char *msg082() { "M082 - AVL Add failed (%s)";}
char *msg083() { "M083 - AVL Add failed (%s)";}
char *msg084() { "M084 - Attribute Object Id deletion failed (%s)";}
char *msg085() { "M085 - acquisition of Service List mutex failed, errno = %d";}
char *msg086() { "M086 - release of Service List mutex failed, errno = %d";}
char *msg087() { "M087 - release of Service List mutex failed, errno = %d";}
char *msg088() { "M088 - Object Class information for reply is missing";}
char *msg089() { "M089 - Attempt to reset obj_inst AVL failed: %d";}
char *msg090() { "M090 - Attempt to step forward failed %d %d";}
char *msg091() { "M091 - Point attempt on instance AVL failed %d";}
char *msg092() { "M092 - Free oid failed";}
char *msg093() { "M093 - MIR instance tag %d doesn't match AVL tag %d";}
char *msg094() { "M094 - Free oid failed";}
char *msg095() { "M095 - MIR instance OID (%s) doesn't match AVL OID (%s)";}
char *msg096() { "M096 - Creation of instance-frag OID failed %d";}
char *msg097() { "M097 - Creation of instance-frag OID failed %d";}
char *msg098() { "M098 - Creation of instance-frag OID failed %d";}
char *msg099() { "M099 - Unsupported SNMP Instance translation for Tag %d, Class (%s)";}
char *msg100() { "M100 - Free oid failed";}
char *msg101() { "M101 - Concatenation of instance oid fragment failed";}
char *msg102() { "M102 - Release of old new-arc oid failed";}
char *msg103() { "M103 - Release of old inst-frag oid failed";}
char *msg104() { "M104 - Ill-formed AVL for object class (%s) Tag %d Status %d";}
char *msg105() { "M105 - Instance OID free attempt failed";}
char *msg106() { "M106 - AVL reset on attribute list AVL failed %d";}
char *msg107() { "M107 - Point attempt on instance AVL failed %d";}
char *msg108() { "M108 - oid creation attempt failed";}
char *msg109() { "M109 - OID Concatenation failed %d";}
char *msg110() { "M110 - Instance OID free attempt failed";}
char *msg111() { "M111 - Init attempt for 'out_entry' AVL failed %d";}
char *msg112() { "M112 - ADD attempt for 'out_entry' AVL failed %d";}
char *msg113() { "M113 - Instance OID free attempt failed";}
char *msg114() { "M114 - More than 1 reply to pei_send_get_reply, timeout may occur";}
char *msg115() { "M115 - Improper GET reply code %d, object class (%%s)";}
char *msg116() { "M116 - attrlist AVL reset failed: %d, object class (%%s)";}
char *msg117() { "M117 - AVL point on attr-list failed: %d, object class (%%s)";}
char *msg118() { "M118 - Failed to find svc(%d) or vbe(%d) blocks for (%%s)";}
char *msg119() { "M119 - Improper Entry at reply entry point 'Create' for (%s)";}
char *msg120() { "M120 - Improper Entry at reply entry point 'Delete' for (%s)";}
char *msg121() { "M121 - MIR Database has not been loaded";}
char *msg122() { "M122 - MIR storage release error";}
char *msg123() { "M123 - MIR storage release error";}
char *msg124() { "M124 - MIR invalid mandle";}
char *msg125() { "M125 - MIR oid pointer invalid";}
char *msg126() { "M126 - MIR Tier 2 Initialization failed %d";}
char *msg127() { "M127 - '%s' returned, MIR corrupt or coding error %d";}
char *msg128() { "M128 - Coding Error, status=%s %d";}
char *msg129() { "M129 - Missing 'MIR_Contained_By' Relationship: %s %d";}
char *msg130() { "M130 - mandle-to-oid conversion failure %s %d";}
char *msg132() { "M132 - memory exhausted";}
char *msg133() { "M133 - Mandle class release failed";}
char *msg135() { "M135 - MIR corrupt or coding error, status= %s, masa=%d";}
char *msg136() { "M136 - Instance (%s) has no value";}
char *msg137() { "M137 - Mandle class release failed";}
char *msg138() { "M138 - Instance Attribute w/no SNMP OID: masa(%d)";}
char *msg139() { "M139 - mandle-to-oid conversion failure";}
char *msg140() { "M140 - Instance (%s) has constructed datatype";}
char *msg141() { "M141 - MIR Dissolve Failed (%s)";}
char *msg142() { "M142 - Fault from eval data construct";}
char *msg143() { "M143 - Instance (%s) is not exact";}
char *msg144() { "M144 - MIR initialization failed";}
char *msg145() { "M145 - (%d) Unrecognized mir_lookup_data_construct() error";}
char *msg146() { "M146 - MIR Dissolve Failed (%s)";}
char *msg147() { "M147 - memory exhausted";}
char *msg148() { "M148 - mutex lock failed in log_service(): %m";}
char *msg149() { "M149 - mutex unlock failed in log_service(): %m";}
char *msg150() { "M150 - NULL Service block address presented for log";}
char *msg151() { "M151 - Execution Anomaly: MIR GET_NEXT on (%%s) rtned %s";}
char *msg152() { "M152 - Execution Anomaly: mandle_to_oid returned %s, %d";}
char *msg153() { "M153 - %s returned, MSL invalid, MIR corrupt or coding error";}
char *msg154() { "M154 - %s returned, MSL invalid, MIR corrupt or coding error";}
char *msg155() { "M155 - OID Arc limit exceeded";}
char *msg156() { "M156 - Memory exhausted";}
char *msg157() { "M157 - MIR Database has not been loaded";}
char *msg158() { "M158 - MIR invalid mandle";}
char *msg159() { "M159 - MIR oid pointer invalid";}
char *msg160() { "M160 - High Class tag value(%x hex) in (%%s), defaulting to ASN1_C_NULL";}
char *msg161() { "M161 - Instance AVL initialization failed (%s)";}
char *msg162() { "M162 - Instance AVL start-construct failed (%s)";}
char *msg163() { "M163 - Error from AVL free (%s)";}
char *msg164() { "M164 - Instance AVL start-construct failed (%s)";}
char *msg165() { "M165 - Attempt to append current Class AVL Failed (%s)";}
char *msg166() { "M166 - Instance AVL end-construct failed (%s)";}
char *msg168() { "M168 - Error from AVL free (%s)";}
char *msg169() { "M169 - Instance AVL initialization failed (%s)";}
char *msg170() { "M170 - Instance AVL end-construct failed (%s)";}
char *msg171() { "M171 - Invalid processing status %d";}
char *msg172() { "M172 - Attempt to reset AVL failed (%s)";}
char *msg173() { "M173 - Attempt to point AVL failed (%s)";}
char *msg174() { "M174 - GET(%%s), AVL-bld(%s), /i_arc=%d, i_arcs_left=%d/";}
char *msg175() { "M175 - MIR Database has not been loaded";}
char *msg176() { "M176 - MIR invalid mandle";}
char *msg177() { "M177 - MIR oid pointer invalid";}
char *msg178() { "M178 - Anomalous return (%s) from MIR for (%%s)";}
char *msg179() { "M179 - OID Anomaly (%%s), extra arcs /i_arc=%d, i_arcs_left=%d/";}
char *msg180() { "M180 - OID Anomaly (%%s), Attr Arc invalid, extra arcs /i_arc=%d, i_arcs_left=%d/";}
char *msg181() { "M181 - OID Anomaly (%%s), Attr Arc bad, some conversion OK, junk:/i_arc=%d, i_arcs_left=%d/";}
char *msg182() { "M182 - OID Anomaly (%%s), some conversion OK, junk:/i_arc=%d, i_arcs_left=%d/";}
char *msg183() { "M183 - OID Anomaly (%%s), Attr Arc invalid, no conv. OK, junk:/i_arc=%d, i_arcs_left=%d/";}
char *msg184() { "M184 - OID Anomaly (%%s), no conv OK, junk:/i_arc=%d, i_arcs_left=%d/";}
char *msg185() { "M185 - Error from AVL free: %d";}
char *msg186() { "M186 - Error from AVL free, instance(%s), attribute(%s)";}
char *msg187() { "M187 - (%d)=(%s) return from (%%s) invoke-action: rolling";}
char *msg188() { "M188 - (%d)=(%s) return from (%%s) invoke-action: generr";}
char *msg190() { "M190 - Instance AVL init failed (%s)";}
char *msg191() { "M191 - Attribute AVL init failed (%s)";}
char *msg192() { "M192 - Error from AVL free, instance(%s), attribute(%s)";}
char *msg193() { "M193 - More than one reply to pei_send_get_reply";}
char *msg194() { "M194 - Improper GET-NEXT reply code (%d)=(%s), object class (%%s)";}
char *msg195() { "M195 - Failed to find svc(%d) or vbe(%d) blocks for (%%s)";}
char *msg196() { "M196 - Attempt to point backward failed %d";}
char *msg197() { "M197 - Attempt to point backward failed %d";}
char *msg198() { "M198 - Bad Version %d in PDU from %s, PDU ignored";}
char *msg199() { "M199 - Attempt to point backward failed %d";}
char *msg200() { "M200 - Attempt to reset AVL failed (%s)";}
char *msg201() { "M201 - Attempt to point AVL failed (%s)";}
char *msg202() { "M202 - Error from AVL free: %d";}
char *msg203() { "M203 - SET(%%s), AVL-bld(%s), /i_arc=%d, i_arcs_left=%d/";}
char *msg204() { "M204 - Error from AVL free: %d";}
char *msg205() { "M205 - Error from AVL free, instance (%s), attribute (%s)";}
char *msg206() { "M206 - anomalous Rtn (%s) from msi_SET_attributes";}
char *msg207() { "M207 - More than 1 reply to pei_send_get_reply, timeout may occur";}
char *msg208() { "M208 - Improper SET reply code %d, object class (%%s)";}
char *msg209() { "M209 - attrlist AVL reset failed: %d, object class (%%s)";}
char *msg210() { "M210 - AVL point on attr-list failed: %d, object class (%%s)";}
char *msg211() { "M211 - Failed to find svc(%d) or vbe(%d) blocks for (%%s)";}
char *msg212() { "M212 - mutex lock failed in netio_asn1_dump(): %m";}
char *msg213() { "M213 - mutex unlock failed in netio_asn1_dump(): %m";}
char *msg214() { "M214 - Unable to acquire host name '%s', errno = %d";}
char *msg215() { "M215 - Unable to acquire host information '%s', errno = %d";}
char *msg216() { "M216 - mutex lock failed in netio_send_trap(): %m";}
char *msg217() { "M217 - mutex unlock failed in netio_send_trap(): %m";}
char *msg218() { "M218 - Trap (type %d) PDU ASN.1 encode failed, MCC-code =%d, no trap sent";}
char *msg219() { "M219 - TRAP PDU 'sendto = %d' address(%s) failed, errno = %d, '%s'";}
char *msg220() { "M220 - Missing Trap INET Address, config file line %d";}
char *msg221() { "M221 - Controlled Exit Requested from (%s)";}
char *msg222() { "M222 - Protocol setup failure '%s', %d, %d";}
char *msg223() { "M223 - Interface registration failure '%s',%d, %d";}
char *msg224() { "M224 - Binding vector acqusition failed '%s',%d, %d";}
char *msg225() { "M225 - Binding vector string conversion failure '%s',%d, %d";}
char *msg226() { "M226 - Binding vector deallocation failure '%s',%d, %d";}
char *msg227() { "M227 - Binding string deallocation failure '%s',%d, %d";}
char *msg228() { "M228 - RPC listen returned w/code %d";}
char *msg229() { "M229 - Exception TRUE from 'rpc_server_register_if'";}
char *msg230() { "M230 - Exception TRUE from 'rpc_server_inq_bindings'";}
char *msg231() { "M231 - Exception TRUE from 'rpc_server_listen'";}
char *msg232() { "M232 - MIR Init Fail: %s";}
char *msg233() { "M233 - Unrecognized Return Status from MIR Subsystem";}
char *msg234() { "M234 - Unrecognized Return Status from MIR Subsystem";}
char *msg235() { "M235 - Unrecognized Return Status from MIR Subsystem";}
char *msg236() { "M236 - Unrecognized Return Status from MIR Subsystem";}
char *msg237() { "M237 - Instance Arc #%d too large '%d': (%%s)";}
char *msg238() { "M238 - AVL Add failed (%s)";}
char *msg239() { "M239 - mandle-to-oid conversion failure %s %d";}
char *msg240() { "M240 - Attempt to allocate memory failed";}
char *msg241() { "M241 - Attempt to allocate memory failed";}
char *msg242() { "M242 - Attempt to allocate memory failed";}
char *msg243() { "M243 - Attempt to move avl pointer forward failed %d";}
char *msg244() { "M244 - snmp_pe Debug Mode enabled: Fork suppressed";}
char *msg245() { "M245 - Extraneous no_auth_traps value, config file line %d";}
char *msg246() { "M246 - Missing Inet address, config file line %d";}
char *msg247() { "M247 - failure obtaining sysparms for sysid_octet, '%s', errno = %d";}
char *msg248() { "M248 - moss_oid_append for sysid_octet failed, '%s', errno = %d";}
char *msg249() { "M249 - moss_oid_to_octet for sysid_octet failed, '%s', errno = %d";}
char *msg250() { "M250 - Missing access mode, config file line %d";}
char *msg251() { "M251 - Error from AVL free: %d";}
char *msg252() { "M252 - Error from AVL free: %d";}
char *msg253() { "M253 - Error from AVL free: %d";}
char *msg254() { "M254 - Error from AVL free: %d";}
char *msg255() { "M255 - Error from AVL free: %d";}
char *msg256() { "M256 - Error from AVL free: %d";}
char *msg257() { "M257 - Error from AVL free: %d";}
char *msg258() { "M258 - Error from AVL free: %d";}
char *msg259() { "M259 - Error from AVL free: %d";}
char *msg260() { "M260 - Error from AVL free: %d";}
char *msg261() { "M261 - Error from AVL free: %d";}
char *msg262() { "M262 - Error from AVL free: %d";}
char *msg263() { "M263 - Error from AVL free: %d";}
char *msg264() { "M264 - Error from AVL free: %d";}
char *msg265() { "M265 - Error from AVL free: %d";}
char *msg266() { "M266 - Error from AVL free: %d";}
char *msg267() { "M267 - Error from AVL free: %d";}
char *msg268() { "M268 - Error from AVL free: %d";}
char *msg269() { "M269 - Error from AVL free: %d";}
char *msg270() { "M270 - Error from AVL free: %d";}
char *msg271() { "M271 - Error from AVL free: %d";}
char *msg272() { "M272 - Error from AVL free: %d";}
char *msg273() { "M273 - Error from AVL free: %d";}
char *msg274() { "M274 - Error from AVL free: %d";}
char *msg275() { "M275 - Error from AVL free: %d";}
char *msg276() { "M276 - Error from AVL free: %d";}
char *msg277() { "M277 - Error from AVL free: %d";}
char *msg278() { "M278 - Error from AVL free: %d";}
char *msg279() { "M279 - Error from AVL free: %d";}
char *msg280() { "M280 - Access Control AVL initialization failed %d";}
char *msg281() { "M281 - Access Control AVL add failed %d";}
char *msg282() { "M282 - No memory for Access control AVL community name";}
char *msg283() { "M283 - Access Control AVL add failed %d";}
char *msg284() { "M284 - Access Control AVL add failed %d";}
char *msg285() { "M285 - Error from AVL free (%s)";}
char *msg286() { "M286 - %s: License for Common Agent Base Kit (COM-AGNT-BAS) is not loaded, or \nlicense for Common Agent Developer's Toolkit (COM-AGNT-DEV) is not loaded.\n";}
char *msg287() { "M287 - mold_populate_from_mir() fails";}
char *msg288() { "M288 - OID Anomaly (%%s), Attr Arc invalid, extra arcs /i_arc=%d, i_arcs_left=%d/";}
char *msg289() { "M289 - OID Anomaly (%%s), Attr Arc bad, some conversion OK, junk:/i_arc=%d, i_arcs_left=%d/";}
char *msg290() { "M290 - OID Anomaly (%%s), some conversion OK, junk:/i_arc=%d, i_arcs_left=%d/";}
char *msg291() { "M291 - OID Anomaly (%%s), Attr Arc invalid, no conv. OK, junk:/i_arc=%d, i_arcs_left=%d/";}
char *msg292() { "M292 - OID Anomaly (%%s), no conv OK, junk:/i_arc=%d, i_arcs_left=%d/";}
char *msg293() { "M293 - Error from AVL free (%s)";}
char *msg294() { "M294 - OID Anomaly (%%s), extra arcs /i_arc=%d, i_arcs_left=%d/";}
char *msg295() { "M295 - OID Anomaly (%%s), instance arcs ignored/i_arc=%d, i_arcs_left=%d/";}
char *msg296() { "M296 - OID Anomaly (%%s), instance arcs ignored/i_arc=%d, i_arcs_left=%d/";}
char *msg297() { "M297 - memory exhausted";}
char *msg298() { "M298 - Add of subelement to Instance AVL element failed (%s)";}
char *msg299() { "M299 - Validation of octet string failed %d";}
char *msg300() { "M300 - memory exhausted";}
char *msg301() { "M301 - Invalid queue handle (%lx)";}
char *msg302() { "M302 - Invalid queue handle (%lx)";}
char *msg303() { "M303 - Invalid object class OID";}
char *msg304() { "M304 - Invalid event_type OID";}
char *msg305() { "M305 - NULL event_parameters";}
char *msg306() { "M306 - moss_avl_reset on event_parameters failed";}
char *msg307() { "M307 - No Specific trap number passed in event parms avl";}
char *msg308() { "M308 - Unsupported MOM-generated Generic trap (%d)";}
char *msg309() { "M309 - Object Class OID mismatch with event parm avl Enterprise oid";}
char *msg310() { "M310 - SEQUENCE occured in event parameters avl";}
char *msg311() { "M311 - Mal-formed event parameters avl has more after EOC";}
char *msg312() { "M312 - Mal-formed event parameters avl (NULL oid)";}
char *msg313() { "M313 - Mal-formed event parameters avl (Empty oid)";}
char *msg314() { "M314 - Unable to create temporary varbind arg avl";}
char *msg315() { "M315 - Mal-formed enterprise OID in event parameters varbind arg";}
char *msg316() { "M316 - Unable to moss_avl_add temporary varbind arg avl";}
char *msg317() { "M317 - Unable to moss_avl_free temporary varbind arg avl";}
char *msg318() { "M318 - Ignoring Unknown OID in event parameters avl";}
char *msg319() { "M319 - Missing or mismatching event_type/specific_trap in event parameters avl";}
char *msg320() { "M320 - Mal-formed enterprise OID in event parameters varbind arg";}
char *msg321() { "M321 - ASN.1 varbindlist encode failed on TRAP PDU; asn_status(%d), moss(%d)";}
char *msg322() { "M322 - Unable to moss_avl_add temporary varbind arg avl";}
char *msg323() { "M323 - ASN.1 varbindlist encode failed on TRAP PDU; asn_status(%d), moss(%d)";}
char *msg324() { "M324 - moss_avl_point FAILED on event parameters avl";}
char *msg325() { "M325 - Generic Trap event_type mismatch with event parm avl generic_trap_num";}
char *msg326() { "M326 - Invalid Specific trap number (%d) for generic trap (%d)";}
char *msg327() { "M327 - Instance AVL modifier cannot be updated; AVL is botched";}
char *msg328() { "M328 - Invalid Queue Access Mode (%d)";}
char *msg329() { "M329 - acquisition of Event UID mutex failed, errno = %d";}
char *msg330() { "M330 - release of Event UID mutex failed, errno = %d";}

