/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Where should the loader live on the Multimax?  In general, we'd
 * like for it to live as high and out of the way as possible.
 * However, on the Multimax there some restrictions.  For example,
 * crt0 initialize the global variable, _environ, as follows.
 *
 *                      movd r0,@_environ
 *
 * This instruction uses the absolute addressing mode.  When packed
 * into an instruction, the absolute displacement (e.g. the value
 * @_environ) must fit into a signed 30-bit field.  However, it is
 * invalid for the value to be negative.  So for all practical
 * purposes, the value of the symbol _environ must not be greater than
 * 29 bits in size.  Therefore all loader text, data and BSS must
 * reside below 0x1fffffff.  We currently select 0x1f000000, which
 * gives the loader 16 megabytes.
 */

SECTIONS
{
	.text 0x1f000000 : {}
	GROUP BIND ((ADDR(.text) + SIZEOF(.text) + 0x1fff) & ~0x1fff): {
		.data : {}
		.bss  : {}
	}
}

modstart = 0x2000;
