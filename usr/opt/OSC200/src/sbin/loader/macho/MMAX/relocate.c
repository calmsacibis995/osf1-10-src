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
static char	*sccsid = "@(#)$RCSfile: relocate.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:41:14 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *
 * relocate.c
 *
 * machine dependent relocation for a Mach-O file
 * for the NS32K (Multimax).
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <sys/param.h>
#include <loader.h>
#include <math.h>
#include <strings.h>

#include "ldr_types.h"
#include "ldr_sys_int.h"
#include "ldr_windows.h"
#include "ldr_malloc.h"
#include "ldr_region.h"
#include "ldr_package.h"
#include "ldr_symbol.h"
#include "ldr_errno.h"
#include "ldr_symval_std.h"
#include "ldr_hash.h"

#include <mach_o_header.h>
#include <mach_o_format.h>

#include "open_hash.h"
#include "mo_ldr.h"
#include "relocate.h"

/* 
 *	MACROS FOR MULTIMAX RELOCATION
 */

/* macros to access the sub-pieces of the mmax relocation ri_size_type field */
#define R_FORMAT_TYPE(r) (r->ri_size_type & R_FORMAT)
#define R_ADDRTYPE_TYPE(r)  (r->ri_size_type & R_ADDRTYPE)
#define R_ACTION_TYPE(r) (r->ri_size_type & R_RELTO)
#define R_PC_RELATIVE(r) (r->ri_size_type & R_PCREL)

#define RELOC_TARGET_BITSIZE(r)		32
#define RELOC_TARGET_BITPOS(r)		0
#define RELOC_TARGET_SIZE(r)		(((r)->ri_size_type & R_SIZESP) >> RELOC_TARGET_SIZE_RIGHT_SHIFT)
#define RELOC_TARGET_SIZE_RIGHT_SHIFT	12	/* shift out the 0 bits */

/* procedure declarations */

static void 
put_num(char *buf, long val, char n);

static void 
put_imm(char *buf, long val, char n);

static int 
sign_extend (int value, int bits);

static int
put_disp(char *buf, long val, char n);

static int 
get_num(char *buf, int n);

static int 
get_imm(char *buf, int n);

static int
get_disp(char *buffer, int n, int *disp);

/* #define DEBUG	1 */

#ifdef DEBUG
#define	dprintf(x)	ldr_msg x
#else
#define	dprintf(x)
#endif /* DEBUG */

/*
 * process a single relocation.  MMAX specific machine dependent code.
 * Patch the relocation target address with the relocation value.
 */
int
patch_reloc_addr(reloc_info_t *reloc_p, univ_t vaddr, univ_t mapaddr,
		 mo_long_t relocation)
{
	unsigned int mask;		/* relocation mask */
	char *loc;			/* location to relocate = target */
	int bytes;			/* 1, 2 or 4 byte relocation */
	int rc;				/* return code/status */
	int disp;			/* displacement */

	loc = (char *) mapaddr;

	if (R_PC_RELATIVE(reloc_p))
		relocation -= (mo_long_t)vaddr;

	bytes = 1 << RELOC_TARGET_SIZE(reloc_p);

	/* unshifted mask for relocation */
	mask = 1 << RELOC_TARGET_BITSIZE(reloc_p) - 1;
	mask |= mask - 1;

	relocation &= mask;

	dprintf(("patch_reloc_addr: vaddr : 0x%x mapaddr : 0x%x bytes : %d mask : 0x%x relocation : 0x%x\n",
		 vaddr, mapaddr, bytes, mask, relocation));

	rc = LDR_SUCCESS;

	switch(R_FORMAT_TYPE(reloc_p))
		{
		case R_IMMED:
			dprintf(("case R_IMMED - RELOC_MEMORY_ADD_P\n"));
			put_imm(loc, relocation + get_imm(loc, bytes), bytes);
			break;

		case R_DISPL:
			dprintf(("case R_DISPL - RELOC_MEMORY_ADD_P\n"));
			if ((rc = get_disp(loc, bytes, &disp)) != LDR_SUCCESS)
				break;
			rc = put_disp(loc, relocation + disp, bytes);
			break;

		case R_NUMBER:
			dprintf(("case R_NUMBER - RELOC_MEMORY_ADD_P\n"));
			put_num(loc, relocation + get_num(loc, bytes), bytes);
			break;

		case R_PROCDES :
			dprintf(("case R_PROCDES - not supported\n"));
			ldr_log("patch_reloc_addr: case R_PROCDES not supported\n");
			rc = LDR_ENOEXEC;
			break;

		default:
			dprintf(("case default - unknown relocation type: 0x%x\n",
				 R_FORMAT_TYPE(reloc_p)));
			rc = LDR_ENOEXEC;
			break;

		} /* switch */

	return rc;
}


static int 
get_num(buf, n)
     char *buf;
     int n;
{
  int val = 0;
  buf += (n - 1);
  for (; n > 0; n--)
    {
      val = val * 256 + (*buf-- & 0xff);
    }
  return val;
}

static int 
get_imm(buf, n)
     char *buf;
     int n;
{
  int val = 0;
  for (; n > 0; n--)
    {
      val = (val * 256) + (*buf++ & 0xff);
    }
  return val;
}

static int 
get_disp (buffer, n, disp)
     char *buffer;
     int n, *disp;
{
  int Ivalue;

  Ivalue = *buffer++ & 0xff;
  if (n == 0)
    if (Ivalue & 0x80)
    if (Ivalue & 0x40)
	n = 4;
    else
	n = 2;
    else
      n = 1;
  switch (n)
    {
    case 1:
      Ivalue = sign_extend (Ivalue, 7);
      break;
    case 2:
      Ivalue = sign_extend (Ivalue, 6);
      Ivalue = (Ivalue << 8) | (0xff & *buffer);
      break;
    case 4:
      Ivalue = sign_extend (Ivalue, 6);
      Ivalue = (Ivalue << 8) | (0xff & *buffer++);
      Ivalue = (Ivalue << 8) | (0xff & *buffer++);
      Ivalue = (Ivalue << 8) | (0xff & *buffer);
      break;
    default:
      dprintf(("get_disp: invalid argument\n"));
      ldr_log("get_disp: invalid argument\n");
      return LDR_ENOEXEC;
      break;
    }
  *disp = Ivalue; 
  return LDR_SUCCESS;
}


/* 
 * Just put out the twos complement value with arbitary alignment.
 * Could just do an assignment if we know we are on a little endian
 * machine.
 */
static void 
put_num(char *buf, long val, char n)
{
	for (; n > 0; n --) {
		*buf++ = val & 0xff;
		val >>= 8;
	}
}

/* Immediate operands are big endian */
static void 
put_imm(char *buf, long val, char n)
{
	int i;
	
	buf += (n -1);
	for (i = n-1; i >= 0; i--) {
		*buf -- = (val & 0xff);
		val >>= 8;
	}
}

static int 
sign_extend (value, bits)
     int value, bits;
{
  value = value & ((1 << bits) - 1);
  return (value & (1 << (bits-1))
	  ? value | (~((1 << bits) - 1))
	  : value);
}

/* This converts an integer "val" to a displacement. The reason for its'
   existence is the fact that ns32k uses Huffman coded displacements.
   This implies that the bit order is reversed in displacements and
   that they are prefixed with a size-tag.

   binary: msb -> lsb	0xxxxxxx				byte
   			10xxxxxx xxxxxxxx			word
   			11xxxxxx xxxxxxxx xxxxxxxx xxxxxxxx	double word
          
   This must be taken care of and we do it here! 		  
 */
static int
put_disp(char *buf, long val, char n)
{
	switch(n) 
	{
	case 1:
		if (val < -64 || val > 63) {
			dprintf(("put_disp: byte displacement 0x%x out of range\n",
			       val));
			ldr_log("put_disp: byte displacement 0x%x out of range\n",
			       val);
			return LDR_ENOEXEC;
		}
		val &= 0x7f;
		*buf++ = val;
		break;

	case 2:
		if (val < -8192 || val > 8191) {
			dprintf(("put_disp: word displacement 0x%x out of range\n",
			       val));
			ldr_log("put_disp: word displacement 0x%x out of range\n",
			       val);
			return LDR_ENOEXEC;
		}
		val &= 0x3fff;
		val |= 0x8000;
		*buf++ = (val >> 8);
		*buf++ = val;
		break;

	case 4 :
		if (val < -0x1f000000 || val >= 0x20000000) {
			dprintf(("put_disp: double word displacement 0x%x out of range\n",
			       val));
			ldr_log("put_disp: double word displacement 0x%x out of range\n",
			       val);
			return LDR_ENOEXEC;
		}
		val |= 0xc0000000;
		*buf++ = (val >> 24);
		*buf++ = (val >> 16);
		*buf++ = (val >> 8);
		*buf++ = val;
		break;
	
	default:
		dprintf(("put_disp: invalid argument\n"));
		ldr_log("put_disp: invalid argument\n");
		return LDR_ENOEXEC;
		break;
	} /* switch */

	return LDR_SUCCESS;
}
