/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * memory configuration for kmod1
 *
 * OSF/1 Release 1.0
 */

SECTIONS
{
	.text 0x1f00000 : {}
	GROUP BIND ((ADDR(.text) + SIZEOF(.text) + 0x1fff) & ~0x1fff): {
		.data : {}
		.bss  : {}
	}
}
