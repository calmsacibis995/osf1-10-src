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
 *	i2c.h:	Definitions associated with the I2C bus and its peripherals.
 */

#ifndef i2c_DEFINED
#define i2c_DEFINED 1

/*
 *
 *	I2C control bits and access macros:
 *
 */
#define I2C_CLOCK_OFF           (u_int)0xBF
#define I2C_CLOCK_OFF_DATA_ZERO (u_int)0x3F
#define I2C_CLOCK_ON            (u_int)0x40
#define I2C_CLOCK_ON_DATA_ONE   (u_int)0xC0
#define I2C_DATA_ONE            (u_int)0x80
#define I2C_DATA_ZERO           (u_int)0x7F

#define I2C_CLOCK_DELAY i2c_clock_delay()
#define I2C_DATA_DELAY i2c_data_delay()

#define I2C_CLOCK_DATA_LOW  {*port &= I2C_CLOCK_OFF_DATA_ZERO; I2C_CLOCK_DELAY;}
#define I2C_CLOCK_DATA_HIGH {*port |= I2C_CLOCK_ON_DATA_ONE; I2C_CLOCK_DELAY;}
#define I2C_CLOCK_LOW       {*port &= I2C_CLOCK_OFF; I2C_CLOCK_DELAY;}
#define I2C_CLOCK_HIGH      {*port |= I2C_CLOCK_ON; I2C_CLOCK_DELAY;}
#define I2C_DATA_LOW        {*port &= I2C_DATA_ZERO; I2C_DATA_DELAY;}
#define I2C_DATA_HIGH       {*port |= I2C_DATA_ONE; I2C_DATA_DELAY;}

#define I2C_READ_BIT        (u_int)0x01
 
/*
 *
 *	Definitions associated with the Phillips Digital Color Space Convertor(DCSC).
 *
 */
#define I2C_ADDR_DCSC          0xE0		/* Address of DCSC chip. */
#define I2C_SUBA_DCSC_CONTROL  0x00		/* Subaddress of DCSC control register. */
#define I2C_SUBA_DCSC_DATA     0x01		/* Subaddress of DCSC lut data register. */

/*
 *	Control register (subaddress 0) bit definitions:
 */
#define DCSC_VLUT_READ_ENABLED                0x40
#define DCSC_VLUT_WRITE_ENABLED 			  0x00
#define DCSC_VLUT_OUTPUT_DATA_TRISTATED_BY_OE 0x20
#define DCSC_VLUT_OUTPUT_DATA_TRISTATED       0x00
#define DCSC_INPUT_DATA_TO_FORMATTER          0x10
#define DCSC_INPUT_DATA_AT_FIXED_VALUES       0x00
#define DCSC_MATRIX_IN_USE                    0x08
#define DCSC_MATRIX_BYPASSED                  0x00
#define DCSC_INPUT_FORMATTER_TO_FORMAT_1      0x00
#define DCSC_INPUT_FORMATTER_TO_FORMAT_2      0x01
#define DCSC_INPUT_FORMATTER_TO_FORMAT_3      0x02
#define DCSC_INPUT_FORMATTER_TO_FORMAT_4      0x03
#define DCSC_INPUT_FORMATTER_TO_FORMAT_5      0x04

#define DCSC_LOAD_LUT (DCSC_VLUT_WRITE_ENABLED | \
                             DCSC_VLUT_OUTPUT_DATA_TRISTATED_BY_OE | \
                             DCSC_INPUT_DATA_TO_FORMATTER | \
                             DCSC_MATRIX_IN_USE | \
                             DCSC_INPUT_FORMATTER_TO_FORMAT_3 )

#define DCSC_COMPOSITE_LUT  (DCSC_VLUT_READ_ENABLED | \
                             DCSC_VLUT_OUTPUT_DATA_TRISTATED_BY_OE | \
                             DCSC_INPUT_DATA_TO_FORMATTER | \
                             DCSC_MATRIX_IN_USE | \
                             DCSC_INPUT_FORMATTER_TO_FORMAT_3)

#define DCSC_S_VIDEO_LUT    (DCSC_VLUT_READ_ENABLED | \
                             DCSC_VLUT_OUTPUT_DATA_TRISTATED_BY_OE | \
                             DCSC_INPUT_DATA_TO_FORMATTER | \
                             DCSC_MATRIX_IN_USE | \
                             DCSC_INPUT_FORMATTER_TO_FORMAT_3)

#define DCSC_RGB_LUT        (DCSC_VLUT_READ_ENABLED | \
                             DCSC_VLUT_OUTPUT_DATA_TRISTATED_BY_OE | \
                             DCSC_INPUT_DATA_TO_FORMATTER | \
                             DCSC_MATRIX_BYPASSED | \
                             DCSC_INPUT_FORMATTER_TO_FORMAT_4)

/*
 *
 *	Definitions associated with the Phillips Digital Multi-Signal Decoder (DMSD).
 * 
 */
#define I2C_ADDR_DMSD	0x8A	/* Address of DMSD chip. */

#endif /* !i2c_DEFINED */
